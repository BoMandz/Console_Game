#include "include/world/DungeonGeneratorEasy.h"

// Initialize floor with outer walls
void initFloor(Floor &floor) {
    for (int y = 0; y < Floor::HEIGHT; ++y) {
        for (int x = 0; x < Floor::WIDTH; ++x) {
            if (x == 0 || x == Floor::WIDTH - 1 || y == 0 || y == Floor::HEIGHT - 1)
                floor.tiles[x][y] = '#';
            else
                floor.tiles[x][y] = '.';
        }
    }
}

// Generate N unique random integers ensuring minimum room size
std::vector<int> generateUniqueRandoms(int count, int min, int max, int minSpacing) {
    std::vector<int> validPositions;
    
    for (int i = min; i <= max; ++i) {
        validPositions.push_back(i);
    }
    
    if (validPositions.empty()) return {};
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(validPositions.begin(), validPositions.end(), g);
    
    std::vector<int> selected;
    
    for (int pos : validPositions) {
        if (selected.empty()) {
            selected.push_back(pos);
            continue;
        }
        
        bool validSpacing = true;
        for (int existing : selected) {
            if (abs(pos - existing) < minSpacing) {
                validSpacing = false;
                break;
            }
        }
        
        if (validSpacing) {
            selected.push_back(pos);
            if ((int)selected.size() >= count) break;
        }
    }
    
    std::sort(selected.begin(), selected.end());
    return selected;
}

// Add vertical + horizontal walls and return room list
std::vector<Room> addWallsAndGetRooms(Floor &floor, int vCount, int hCount) {
    std::vector<int> vWalls = generateUniqueRandoms(vCount, 4, Floor::WIDTH - 5, 4);
    std::vector<int> hWalls = generateUniqueRandoms(hCount, 4, Floor::HEIGHT - 5, 4);

    for (int x : vWalls) {
        for (int y = 1; y < Floor::HEIGHT - 1; ++y)
            floor.tiles[x][y] = '#';
    }

    for (int y : hWalls) {
        for (int x = 1; x < Floor::WIDTH - 1; ++x)
            floor.tiles[x][y] = '#';
    }

    std::vector<int> vAll = {0};
    vAll.insert(vAll.end(), vWalls.begin(), vWalls.end());
    vAll.push_back(Floor::WIDTH - 1);

    std::vector<int> hAll = {0};
    hAll.insert(hAll.end(), hWalls.begin(), hWalls.end());
    hAll.push_back(Floor::HEIGHT - 1);

    std::vector<Room> rooms;
    for (size_t i = 0; i < vAll.size() - 1; ++i) {
        for (size_t j = 0; j < hAll.size() - 1; ++j) {
            int roomWidth = vAll[i + 1] - vAll[i] - 1;
            int roomHeight = hAll[j + 1] - hAll[j] - 1;
            
            if (roomWidth >= 3 && roomHeight >= 3) {
                rooms.emplace_back(vAll[i], vAll[i + 1], hAll[j], hAll[j + 1]);
            }
        }
    }

    return rooms;
}

// Add doors ensuring every room has at least one door, max 1 door per wall segment between rooms
void addDoorsToRooms(Floor &floor, const std::vector<Room> &rooms) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::set<std::string> usedWallSegments;
    
    auto getWallSegmentId = [](int wallPos, int start, int end, bool isVertical) -> std::string {
        return (isVertical ? "V" : "H") + std::to_string(wallPos) + "_" + 
               std::to_string(start) + "_" + std::to_string(end);
    };
    
    for (const Room &room : rooms) {
        std::vector<std::vector<std::pair<int, int>>> wallOptions(4);
        std::vector<std::string> wallSegmentIds(4);
        
        if (room.top > 0) {
            std::string segmentId = getWallSegmentId(room.top, room.left, room.right, false);
            if (usedWallSegments.find(segmentId) == usedWallSegments.end()) {
                for (int x = room.left + 1; x < room.right; ++x) {
                    if (floor.tiles[x][room.top] == '#') {
                        wallOptions[0].emplace_back(x, room.top);
                    }
                }
                wallSegmentIds[0] = segmentId;
            }
        }
        
        if (room.bottom < Floor::HEIGHT - 1) {
            std::string segmentId = getWallSegmentId(room.bottom, room.left, room.right, false);
            if (usedWallSegments.find(segmentId) == usedWallSegments.end()) {
                for (int x = room.left + 1; x < room.right; ++x) {
                    if (floor.tiles[x][room.bottom] == '#') {
                        wallOptions[1].emplace_back(x, room.bottom);
                    }
                }
                wallSegmentIds[1] = segmentId;
            }
        }
        
        if (room.left > 0) {
            std::string segmentId = getWallSegmentId(room.left, room.top, room.bottom, true);
            if (usedWallSegments.find(segmentId) == usedWallSegments.end()) {
                for (int y = room.top + 1; y < room.bottom; ++y) {
                    if (floor.tiles[room.left][y] == '#') {
                        wallOptions[2].emplace_back(room.left, y);
                    }
                }
                wallSegmentIds[2] = segmentId;
            }
        }
        
        if (room.right < Floor::WIDTH - 1) {
            std::string segmentId = getWallSegmentId(room.right, room.top, room.bottom, true);
            if (usedWallSegments.find(segmentId) == usedWallSegments.end()) {
                for (int y = room.top + 1; y < room.bottom; ++y) {
                    if (floor.tiles[room.right][y] == '#') {
                        wallOptions[3].emplace_back(room.right, y);
                    }
                }
                wallSegmentIds[3] = segmentId;
            }
        }
        
        std::vector<int> availableWalls;
        for (int i = 0; i < 4; ++i) {
            if (!wallOptions[i].empty()) {
                availableWalls.push_back(i);
            }
        }
        
        if (!availableWalls.empty()) {
            std::shuffle(availableWalls.begin(), availableWalls.end(), gen);
            int numWalls = 1 + (gen() % std::min(2, (int)availableWalls.size()));
            
            for (int i = 0; i < numWalls; ++i) {
                int wallIndex = availableWalls[i];
                if (!wallOptions[wallIndex].empty()) {
                    std::uniform_int_distribution<int> posDist(0, wallOptions[wallIndex].size() - 1);
                    int posIndex = posDist(gen);
                    
                    int x = wallOptions[wallIndex][posIndex].first;
                    int y = wallOptions[wallIndex][posIndex].second;
                    
                    floor.tiles[x][y] = '+';
                    
                    if (!wallSegmentIds[wallIndex].empty()) {
                        usedWallSegments.insert(wallSegmentIds[wallIndex]);
                    }
                }
            }
        }
    }
}

// Print the map
void printFloor(const Floor &floor) {
    for (int y = 0; y < Floor::HEIGHT; ++y) {
        for (int x = 0; x < Floor::WIDTH; ++x)
            std::cout << floor.tiles[x][y];
        std::cout << '\n';
    }
}

// Print room information for debugging
void printRoomInfo(const std::vector<Room> &rooms) {
    std::cout << "Generated " << rooms.size() << " rooms:\n";
    for (size_t i = 0; i < rooms.size(); ++i) {
        const Room &r = rooms[i];
        int width = r.right - r.left - 1;
        int height = r.bottom - r.top - 1;
        std::cout << "Room " << i + 1 << ": " << width << "x" << height 
                  << " at (" << r.left + 1 << "," << r.top + 1 << ")\n";
    }
    std::cout << "\n";
}
