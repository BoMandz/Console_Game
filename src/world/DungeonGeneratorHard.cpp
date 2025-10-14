#include "include/world/DungeonGeneratorHard.h"
#include "include/world/VoronoiGenerator.h"
#include <functional>
#include <queue>
#include <algorithm>
#include <numeric>
#include <climits>
#include <iostream>


HardDungeonGenerator::HardDungeonGenerator() : rng(std::random_device{}()), numParts(8) {}

void HardDungeonGenerator::generate(HardFloor& floor, int numParts) {
    this->numParts = numParts;
    const int SIZE = DungeonConfig::HARD_WIDTH;
    
    // Initialize floor
    floor.fill(' ');
    
    // Generate Voronoi regions using VoronoiGenerator
    std::vector<std::vector<int>> grid(SIZE, std::vector<int>(SIZE));
    VoronoiGenerator voronoiGen;
    voronoiGen.generateRegions(grid, SIZE, numParts);
    
    // Generate BSP dungeons for each region
    std::vector<std::vector<std::vector<char>>> dungeons(numParts);
    std::vector<std::vector<Rect>> regionRooms(numParts);
    generateBSPDungeons(grid, dungeons, regionRooms);
    
    // Place markers in each region
    placeMarkers(grid, dungeons, regionRooms);
    
    // Combine all dungeons into the final floor
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int regionId = grid[y][x];
            
            // Find local coordinates within region
            int minX = SIZE, minY = SIZE;
            for (int sy = 0; sy < SIZE; sy++) {
                for (int sx = 0; sx < SIZE; sx++) {
                    if (grid[sy][sx] == regionId) {
                        minX = std::min(minX, sx);
                        minY = std::min(minY, sy);
                    }
                }
            }
            
            int dungeonX = x - minX;
            int dungeonY = y - minY;
            
            char dungeonChar = ' ';
            if (dungeonY >= 0 && dungeonY < dungeons[regionId].size() &&
                dungeonX >= 0 && dungeonX < dungeons[regionId][0].size()) {
                dungeonChar = dungeons[regionId][dungeonY][dungeonX];
            }
            
            floor.tiles[x][y] = dungeonChar;
        }
    }
    draw(grid,dungeons);
}

void HardDungeonGenerator::generateBSPDungeons(std::vector<std::vector<int>>& grid,
                                              std::vector<std::vector<std::vector<char>>>& dungeons,
                                              std::vector<std::vector<Rect>>& regionRooms) {
    const int SIZE = DungeonConfig::HARD_WIDTH;
    
    for (int i = 0; i < numParts; i++) {
        int minX = SIZE, minY = SIZE, maxX = -1, maxY = -1;
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                if (grid[y][x] == i) {
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                    minY = std::min(minY, y);
                    maxY = std::max(maxY, y);
                }
            }
        }

        if (maxX < minX || maxY < minY) {
            dungeons[i] = std::vector<std::vector<char>>{};
            continue;
        }

        int width = maxX - minX + 1;
        int height = maxY - minY + 1;

        dungeons[i] = std::vector<std::vector<char>>(height, std::vector<char>(width, ' '));

        if (width >= 20 && height >= 20) {
            BSPTree bspTree;
            Rect rootArea(0, 0, width, height);
            int targetRooms = std::max(4, std::min(12, (width * height) / 200));
            bspTree.buildTree(rootArea, 5, targetRooms);
            bspTree.createRooms(regionRooms[i]);

            for (const Rect& room : regionRooms[i]) {
                for (int y = room.top(); y < room.bottom(); y++) {
                    for (int x = room.left(); x < room.right(); x++) {
                        if (x >= 0 && x < width && y >= 0 && y < height) {
                            dungeons[i][y][x] = '#';
                        }
                    }
                }
            }

            connectRoomsRecursively(bspTree.getRoot(), dungeons[i]);
        }
    }
}

void HardDungeonGenerator::placeMarkers(std::vector<std::vector<int>>& grid,
                                       std::vector<std::vector<std::vector<char>>>& dungeons,
                                       std::vector<std::vector<Rect>>& regionRooms) {
    const int SIZE = DungeonConfig::HARD_WIDTH;
    
    // Store region bounds for each zone
    std::vector<std::tuple<int, int, int, int>> regionBounds(numParts);
    for (int i = 0; i < numParts; i++) {
        int minX = SIZE, minY = SIZE, maxX = -1, maxY = -1;
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                if (grid[y][x] == i) {
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                    minY = std::min(minY, y);
                    maxY = std::max(maxY, y);
                }
            }
        }
        regionBounds[i] = std::make_tuple(minX, minY, maxX, maxY);
    }

    for (int zoneIdx = 0; zoneIdx < numParts; zoneIdx++) {
        int regionId = zoneIdx;
        int numberToPlace = zoneIdx + 1;
        char marker = (numberToPlace <= 9) ? '0' + numberToPlace : 'A' + (numberToPlace - 10);

        bool markerPlaced = false;

        // Try to place in a room first
        if (!regionRooms[regionId].empty()) {
            auto [minX, minY, maxX, maxY] = regionBounds[regionId];
            
            std::vector<int> roomIndices(regionRooms[regionId].size());
            std::iota(roomIndices.begin(), roomIndices.end(), 0);
            std::shuffle(roomIndices.begin(), roomIndices.end(), rng);

            for (int roomIdx : roomIndices) {
                const Rect& room = regionRooms[regionId][roomIdx];
                
                // Check if room is cut by Voronoi border
                bool roomIsCutByBorder = false;
                int globalLeft = minX + room.left();
                int globalRight = minX + room.right() - 1;
                int globalTop = minY + room.top();
                int globalBottom = minY + room.bottom() - 1;
                
                for (int gy = globalTop; gy <= globalBottom && !roomIsCutByBorder; gy++) {
                    for (int gx = globalLeft; gx <= globalRight && !roomIsCutByBorder; gx++) {
                        if (gx >= 0 && gx < SIZE && gy >= 0 && gy < SIZE) {
                            bool isOnBorder = false;
                            if (gx > 0 && grid[gy][gx-1] != regionId) isOnBorder = true;
                            if (gx < SIZE-1 && grid[gy][gx+1] != regionId) isOnBorder = true;
                            if (gy > 0 && grid[gy-1][gx] != regionId) isOnBorder = true;
                            if (gy < SIZE-1 && grid[gy+1][gx] != regionId) isOnBorder = true;
                            
                            if (isOnBorder) roomIsCutByBorder = true;
                        }
                    }
                }
                
                if (roomIsCutByBorder) continue;
                
                for (int attempt = 0; attempt < 10 && !markerPlaced; attempt++) {
                    if (room.right() <= room.left() || room.bottom() <= room.top()) break;
                    
                    std::uniform_int_distribution<> xDist(room.left(), std::max(room.left(), room.right() - 1));
                    std::uniform_int_distribution<> yDist(room.top(), std::max(room.top(), room.bottom() - 1));

                    int localX = xDist(rng);
                    int localY = yDist(rng);

                    if (localY >= 0 && localY < dungeons[regionId].size() && 
                        localX >= 0 && localX < dungeons[regionId][0].size()) {
                        
                        if (dungeons[regionId][localY][localX] == '#' || dungeons[regionId][localY][localX] == '.') {
                            dungeons[regionId][localY][localX] = marker;
                            markerPlaced = true;
                            
                            int globalX = minX + localX;
                            int globalY = minY + localY;
                            
                            auto furthest = findFurthestInRegion(grid, dungeons, regionId, globalX, globalY);
                            auto [furthestGlobalX, furthestGlobalY] = furthest;
                            
                            int furthestLocalX = furthestGlobalX - minX;
                            int furthestLocalY = furthestGlobalY - minY;
                            
                            if (furthestLocalY >= 0 && furthestLocalY < dungeons[regionId].size() && 
                                furthestLocalX >= 0 && furthestLocalX < dungeons[regionId][0].size()) {
                                char furthestMarker;
                                int nextNumber = numberToPlace + 1;
                                if (nextNumber <= 9) furthestMarker = '0' + nextNumber;
                                else furthestMarker = 'A' + (nextNumber - 10);
                                
                                dungeons[regionId][furthestLocalY][furthestLocalX] = furthestMarker;
                            }
                            break;
                        }
                    }
                }
                if (markerPlaced) break;
            }
        }

        // Fallback placement
        if (!markerPlaced) {
            auto [minX, minY, maxX, maxY] = regionBounds[regionId];
            
            if (maxX >= minX && maxY >= minY) {
                int width = maxX - minX + 1;
                int height = maxY - minY + 1;
                
                for (int attempt = 0; attempt < 50 && !markerPlaced; attempt++) {
                    std::uniform_int_distribution<> xDist(0, width - 1);
                    std::uniform_int_distribution<> yDist(0, height - 1);
                    int localX = xDist(rng);
                    int localY = yDist(rng);
                    
                    if (localY >= 0 && localY < dungeons[regionId].size() && 
                        localX >= 0 && localX < dungeons[regionId][0].size()) {
                        
                        if (dungeons[regionId][localY][localX] == '#' || dungeons[regionId][localY][localX] == '.') {
                            dungeons[regionId][localY][localX] = marker;
                            markerPlaced = true;
                            
                            int globalX = minX + localX;
                            int globalY = minY + localY;
                            
                            auto furthest = findFurthestInRegion(grid, dungeons, regionId, globalX, globalY);
                            auto [furthestGlobalX, furthestGlobalY] = furthest;
                            
                            int furthestLocalX = furthestGlobalX - minX;
                            int furthestLocalY = furthestGlobalY - minY;
                            
                            if (furthestLocalY >= 0 && furthestLocalY < dungeons[regionId].size() && 
                                furthestLocalX >= 0 && furthestLocalX < dungeons[regionId][0].size()) {
                                char furthestMarker;
                                int nextNumber = numberToPlace + 1;
                                if (nextNumber <= 9) furthestMarker = '0' + nextNumber;
                                else furthestMarker = 'A' + (nextNumber - 10);
                                
                                dungeons[regionId][furthestLocalY][furthestLocalX] = furthestMarker;
                            }
                            break;
                        }
                    }
                }
                
                if (!markerPlaced && !dungeons[regionId].empty() && !dungeons[regionId][0].empty()) {
                    std::uniform_int_distribution<> xDist(0, dungeons[regionId][0].size() - 1);
                    std::uniform_int_distribution<> yDist(0, dungeons[regionId].size() - 1);
                    int localX = xDist(rng);
                    int localY = yDist(rng);
                    dungeons[regionId][localY][localX] = marker;
                }
            }
        }
    }
}

void HardDungeonGenerator::drawLCorridor(std::vector<std::vector<char>>& dungeon, int x1, int y1, int x2, int y2) {
    for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
        if (x >= 0 && x < dungeon[0].size() && y1 >= 0 && y1 < dungeon.size()) {
            if (dungeon[y1][x] == ' ') dungeon[y1][x] = '.';
        }
    }
    for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
        if (x2 >= 0 && x2 < dungeon[0].size() && y >= 0 && y < dungeon.size()) {
            if (dungeon[y][x2] == ' ') dungeon[y][x2] = '.';
        }
    }
}

void HardDungeonGenerator::connectRoomsRecursively(BSPNode* node, std::vector<std::vector<char>>& dungeon) {
    if (!node || node->isLeaf()) return;

    connectRoomsRecursively(node->left, dungeon);
    connectRoomsRecursively(node->right, dungeon);

    std::function<Rect*(BSPNode*)> getRoom = [&](BSPNode* n) -> Rect* {
        if (!n) return nullptr;
        if (n->hasRoom) return &n->room;
        Rect* leftRoom = getRoom(n->left);
        if (leftRoom) return leftRoom;
        return getRoom(n->right);
    };

    Rect* leftRoom = getRoom(node->left);
    Rect* rightRoom = getRoom(node->right);

    if (leftRoom && rightRoom) {
        drawLCorridor(dungeon, leftRoom->centerX(), leftRoom->centerY(),
                     rightRoom->centerX(), rightRoom->centerY());
    }
}

void HardDungeonGenerator::draw(const std::vector<std::vector<int>>& grid,
                               const std::vector<std::vector<std::vector<char>>>& dungeons) {
    const int SIZE = DungeonConfig::HARD_WIDTH;
    
    // Color schemes
    std::vector<std::vector<std::string>> colors = {
        {"\033[1;32m", "\033[0;32m", "\033[1;92m"},
        {"\033[1;31m", "\033[0;31m", "\033[1;91m"},
        {"\033[1;34m", "\033[0;34m", "\033[1;94m"},
        {"\033[1;35m", "\033[0;35m", "\033[1;95m"},
        {"\033[1;95m", "\033[0;95m", "\033[1;35m"},
        {"\033[1;33m", "\033[0;33m", "\033[1;93m"},
        {"\033[0;33m", "\033[1;33m", "\033[0;93m"},
        {"\033[38;5;208m", "\033[38;5;202m", "\033[38;5;214m"}
    };

    // Print final dungeon
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int regionId = grid[y][x];
            bool voronoiBorder = false;
            if (x > 0 && grid[y][x-1] != regionId) voronoiBorder = true;
            if (x < SIZE-1 && grid[y][x+1] != regionId) voronoiBorder = true;
            if (y > 0 && grid[y-1][x] != regionId) voronoiBorder = true;
            if (y < SIZE-1 && grid[y+1][x] != regionId) voronoiBorder = true;

            if (voronoiBorder) {
                std::cout << "\033[1;37m#\033[0m";
            } else {
                int minX = SIZE, minY = SIZE;
                for (int sy = 0; sy < SIZE; sy++) {
                    for (int sx = 0; sx < SIZE; sx++) {
                        if (grid[sy][sx] == regionId) {
                            minX = std::min(minX, sx);
                            minY = std::min(minY, sy);
                        }
                    }
                }

                int dungeonX = x - minX;
                int dungeonY = y - minY;

                char dungeonChar = ' ';
                if (dungeonY >= 0 && dungeonY < dungeons[regionId].size() &&
                    dungeonX >= 0 && dungeonX < dungeons[regionId][0].size()) {
                    dungeonChar = dungeons[regionId][dungeonY][dungeonX];
                }

                if (dungeonChar >= '1' && dungeonChar <= '9') {
                    std::cout << "\033[1;37m" << dungeonChar << "\033[0m";
                } else if (dungeonChar >= 'A' && dungeonChar <= 'Z') {
                    std::cout << "\033[1;37m" << dungeonChar << "\033[0m";
                } else if (dungeonChar == '#') {
                    std::cout << colors[regionId][0] << ".\033[0m";
                } else if (dungeonChar == '.') {
                    std::cout << colors[regionId][1] << ".\033[0m";
                } else {
                    std::cout << colors[regionId][2] << '#' << "\033[0m";
                }
            }
        }
        std::cout << "\n";
    }
}

std::pair<int, int> HardDungeonGenerator::findFurthestInRegion(const std::vector<std::vector<int>>& grid,
                                                              const std::vector<std::vector<std::vector<char>>>& dungeons,
                                                              int regionId, int gx, int gy) {
    const int SIZE = DungeonConfig::HARD_WIDTH;
    int h = dungeons[regionId].size();
    int w = (h > 0) ? dungeons[regionId][0].size() : 0;

    int minX = INT_MAX, minY = INT_MAX;
    for (int y = 0; y < grid.size(); y++)
        for (int x = 0; x < grid[0].size(); x++)
            if (grid[y][x] == regionId) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
            }

    int lx = gx - minX;
    int ly = gy - minY;

    if (ly < 0 || ly >= h || lx < 0 || lx >= w) {
        return {gx, gy};
    }

    std::vector<std::vector<int>> dist(h, std::vector<int>(w, -1));
    std::queue<std::pair<int, int>> q;
    q.push({lx, ly});
    dist[ly][lx] = 0;

    std::pair<int, int> furthest = {gx, gy};
    int maxDist = 0;

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();

        if (dist[y][x] > maxDist) {
            maxDist = dist[y][x];
            furthest = {x + minX, y + minY};
        }

        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                int globalNx = nx + minX;
                int globalNy = ny + minY;
                
                if (globalNx >= 0 && globalNx < grid[0].size() && 
                    globalNy >= 0 && globalNy < grid.size()) {
                    
                    if (grid[globalNy][globalNx] == regionId) {
                        if (dist[ny][nx] == -1 && 
                           (dungeons[regionId][ny][nx] == '.' || dungeons[regionId][ny][nx] == '#')) {
                            dist[ny][nx] = dist[y][x] + 1;
                            q.push({nx, ny});
                        }
                    }
                }
            }
        }
    }
    return furthest;
}