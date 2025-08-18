#include "include/world/DungeonGeneratorEasy.h"
#include <set>
#include <string>
#include <iostream>
#include <cstdlib>

void EasyDungeonGenerator::initFloor(EasyFloor& floor) {
    for (int y = 0; y < EasyFloor::Height; y++) {
        for (int x = 0; x < EasyFloor::Width; x++) {
            floor.tiles[x][y] = (x == 0 || x == EasyFloor::Width - 1 ||
                                y == 0 || y == EasyFloor::Height - 1) ? '#' : '.';
        }
    }
}

std::vector<EasyDungeonGenerator::Room> 
EasyDungeonGenerator::createRooms(EasyFloor& floor, int vWalls, int hWalls) {
    using namespace DungeonConfig;
    
    auto vPositions = RandomUtils::generateUniquePositions(
        vWalls, 4, EASY_WIDTH - 5, 4
    );
    auto hPositions = RandomUtils::generateUniquePositions(
        hWalls, 4, EASY_HEIGHT - 5, 4
    );
    
    for (int x : vPositions) {
        for (int y = 1; y < EASY_HEIGHT - 1; y++) {
            floor.tiles[x][y] = '#';
        }
    }
    
    for (int y : hPositions) {
        for (int x = 1; x < EASY_WIDTH - 1; x++) {
            floor.tiles[x][y] = '#';
        }
    }
    
    std::vector<int> xBounds = {0};
    xBounds.insert(xBounds.end(), vPositions.begin(), vPositions.end());
    xBounds.push_back(EASY_WIDTH - 1);
    
    std::vector<int> yBounds = {0};
    yBounds.insert(yBounds.end(), hPositions.begin(), hPositions.end());
    yBounds.push_back(EASY_HEIGHT - 1);
    
    std::vector<Room> rooms;
    for (size_t i = 0; i < xBounds.size() - 1; i++) {
        for (size_t j = 0; j < yBounds.size() - 1; j++) {
            int left = xBounds[i] + 1;
            int right = xBounds[i + 1];
            int top = yBounds[j] + 1;
            int bottom = yBounds[j + 1];
            
            if (right - left >= 3 && bottom - top >= 3) {
                rooms.emplace_back(left, right, top, bottom);
            }
        }
    }
    
    return rooms;
}

void EasyDungeonGenerator::addDoors(EasyFloor& floor, const std::vector<Room>& rooms) {
    std::set<std::string> usedWalls;
    auto wallID = [](bool vertical, int pos, int start, int end) {
        return (vertical ? "V" : "H") + 
               std::to_string(pos) + "_" + 
               std::to_string(start) + "_" + 
               std::to_string(end);
    };
    
    for (const Room& room : rooms) {
        std::vector<std::vector<std::pair<int, int>>> doorOptions = {
            {}, {}, {}, {}  // top, bottom, left, right
        };
        std::vector<std::string> wallIDs(4);
        
        // Top wall
        if (room.top() > 1) {
            wallIDs[0] = wallID(false, room.top() - 1, room.left(), room.right());
            if (!usedWalls.count(wallIDs[0])) {
                for (int x = room.left() + 1; x < room.right() - 1; x++) {
                    if (floor.tiles[x][room.top() - 1] == '#') {
                        doorOptions[0].emplace_back(x, room.top() - 1);
                    }
                }
            }
        }
        
        // Bottom wall
        if (room.bottom() < EasyFloor::Height - 1) {
            wallIDs[1] = wallID(false, room.bottom(), room.left(), room.right());
            if (!usedWalls.count(wallIDs[1])) {
                for (int x = room.left() + 1; x < room.right() - 1; x++) {
                    if (floor.tiles[x][room.bottom()] == '#') {
                        doorOptions[1].emplace_back(x, room.bottom());
                    }
                }
            }
        }
        
        // Left wall
        if (room.left() > 1) {
            wallIDs[2] = wallID(true, room.left() - 1, room.top(), room.bottom());
            if (!usedWalls.count(wallIDs[2])) {
                for (int y = room.top() + 1; y < room.bottom() - 1; y++) {
                    if (floor.tiles[room.left() - 1][y] == '#') {
                        doorOptions[2].emplace_back(room.left() - 1, y);
                    }
                }
            }
        }
        
        // Right wall
        if (room.right() < EasyFloor::Width - 1) {
            wallIDs[3] = wallID(true, room.right(), room.top(), room.bottom());
            if (!usedWalls.count(wallIDs[3])) {
                for (int y = room.top() + 1; y < room.bottom() - 1; y++) {
                    if (floor.tiles[room.right()][y] == '#') {
                        doorOptions[3].emplace_back(room.right(), y);
                    }
                }
            }
        }
        
        // Select door positions
        int doorsAdded = 0;
        for (int i = 0; i < 4 && doorsAdded < 2; i++) {
            if (!doorOptions[i].empty()) {
                auto& options = doorOptions[i];
                auto [x, y] = options[rand() % options.size()];
                floor.tiles[x][y] = '+';
                usedWalls.insert(wallIDs[i]);
                doorsAdded++;
            }
        }
    }
}

void EasyDungeonGenerator::generate(EasyFloor& floor) {
    initFloor(floor);
    auto rooms = createRooms(floor, 5, 5);
    addDoors(floor, rooms);
}

void EasyDungeonGenerator::print(const EasyFloor& floor) const {
    for (int y = 0; y < EasyFloor::Height; y++) {
        for (int x = 0; x < EasyFloor::Width; x++) {
            std::cout << floor.tiles[x][y];
        }
        std::cout << '\n';
    }
}