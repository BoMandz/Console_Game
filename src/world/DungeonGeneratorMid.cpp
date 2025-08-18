#include "include/world/DungeonGeneratorMid.h"
#include <functional>
#include <queue>

MidDungeonGenerator::MidDungeonGenerator() : rng(std::random_device{}()) {}

int MidDungeonGenerator::calculateDepth(int roomCount) const {
    if (roomCount <= 1) return 0;
    if (roomCount <= 4) return 3;
    if (roomCount <= 8) return 4;
    if (roomCount <= 16) return 5;
    if (roomCount <= 32) return 6;
    return 7;  // Max depth for larger dungeons
}

void MidDungeonGenerator::generate(MidFloor& floor, int roomCount) {
    // Setup
    floor.fill('#');
    rooms.clear();
    connections.clear();
    
    // Create BSP tree
    Rect rootArea(2, 2, MidFloor::Width - 4, MidFloor::Height - 4);
    bspTree.buildTree(rootArea, calculateDepth(roomCount), roomCount);
    bspTree.createRooms(rooms);
    
    // Carve rooms into floor
    carveRooms(floor);
    
    // Create connections between rooms
    createConnections(bspTree.getRoot());
    carveCorridors(floor);
    addDoors(floor);
}

void MidDungeonGenerator::carveRooms(MidFloor& floor) {
    for (const Rect& room : rooms) {
        for (int y = room.y; y < room.y + room.h; y++) {
            for (int x = room.x; x < room.x + room.w; x++) {
                floor.tiles[x][y] = '.';
            }
        }
    }
}

void MidDungeonGenerator::collectRooms(BSPNode* node, std::vector<Rect>& roomList) {
    if (!node) return;
    if (node->hasRoom) {
        roomList.push_back(node->room);
    } else {
        collectRooms(node->left, roomList);
        collectRooms(node->right, roomList);
    }
}

Rect MidDungeonGenerator::getRandomRoom(BSPNode* node) {
    std::vector<Rect> roomList;
    collectRooms(node, roomList);
    if (roomList.empty()) return Rect();
    std::uniform_int_distribution<size_t> dist(0, roomList.size() - 1);
    return roomList[dist(rng)];
}

void MidDungeonGenerator::createConnections(BSPNode* node) {
    if (!node || node->isLeaf()) return;
    
    createConnections(node->left);
    createConnections(node->right);
    
    if (node->left && node->right) {
        Rect leftRoom = getRandomRoom(node->left);
        Rect rightRoom = getRandomRoom(node->right);
        if (leftRoom.w > 0 && rightRoom.w > 0) {
            connections.emplace_back(leftRoom, rightRoom);
        }
    }
}

void MidDungeonGenerator::carveCorridors(MidFloor& floor) {
    auto hCorridor = [&](int x1, int x2, int y) {
        if (x1 > x2) std::swap(x1, x2);
        for (int x = x1; x <= x2; x++) floor.tiles[x][y] = '.';
    };
    
    auto vCorridor = [&](int y1, int y2, int x) {
        if (y1 > y2) std::swap(y1, y2);
        for (int y = y1; y <= y2; y++) floor.tiles[x][y] = '.';
    };
    
    for (auto& [roomA, roomB] : connections) {
        int ax = roomA.centerX(), ay = roomA.centerY();
        int bx = roomB.centerX(), by = roomB.centerY();
        
        if (rand() % 2) {
            hCorridor(ax, bx, ay);
            vCorridor(ay, by, bx);
        } else {
            vCorridor(ay, by, ax);
            hCorridor(ax, bx, by);
        }
    }
}

void MidDungeonGenerator::addDoors(MidFloor& floor) {
    for (auto& [roomA, roomB] : connections) {
        int doorX = (roomA.centerX() + roomB.centerX()) / 2;
        int doorY = (roomA.centerY() + roomB.centerY()) / 2;
        if (floor.tiles[doorX][doorY] == '.') {
            floor.tiles[doorX][doorY] = '+';
        }
    }
}