#include "include/world/DungeonGeneratorMid.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <queue>

// Assuming these structs are in a separate header or defined here for simplicity.
// In a real project, they'd likely be in their own files (e.g., Floor.h, Rect.h, etc.)
struct Floor {
    static const int WIDTH = 128;
    static const int HEIGHT = 128;
    char tiles[WIDTH][HEIGHT];

    void fill(char c) {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                tiles[x][y] = c;
            }
        }
    }

    void print() {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                std::cout << tiles[x][y];
            }
            std::cout << "\n";
        }
    }
};

struct Rect {
    int x, y, w, h;
    Rect(int x = 0, int y = 0, int w = 0, int h = 0) : x(x), y(y), w(w), h(h) {}

    int centerX() const { return x + w / 2; }
    int centerY() const { return y + h / 2; }

    bool intersects(const Rect& other) const {
        return x < other.x + other.w && x + w > other.x &&
            y < other.y + other.h && y + h > other.y;
    }
};

struct BSPNode {
    Rect bounds;
    BSPNode* left;
    BSPNode* right;
    Rect room;
    bool hasRoom;

    BSPNode(Rect bounds) : bounds(bounds), left(nullptr), right(nullptr), hasRoom(false) {}
    ~BSPNode() { delete left; delete right; }

    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

// Class implementation
BSPDungeonGenerator::BSPDungeonGenerator() : rng(std::random_device{}()), floor(nullptr), root(nullptr),
    targetRoomCount(16), maxDepth(4) {}

BSPDungeonGenerator::~BSPDungeonGenerator() {
    delete root;
}

int BSPDungeonGenerator::calculateDepth(int roomCount) {
    if (roomCount <= 1) return 0;
    if (roomCount <= 4) return 3;
    if (roomCount <= 8) return 4;
    if (roomCount <= 16) return 5;
    if (roomCount <= 32) return 6;
    if (roomCount <= 64) return 7;
    if (roomCount <= 128) return 8;
    return 9;
}

void BSPDungeonGenerator::generate(Floor& f, int roomCount) {
    floor = &f;
    floor->fill('#');

    targetRoomCount = roomCount;
    maxDepth = calculateDepth(roomCount);

    delete root;
    rooms.clear();
    connections.clear();

    std::cout << "Generating dungeon with ~" << roomCount << " rooms (max depth: " << maxDepth << ")\n";

    root = new BSPNode(Rect(1, 1, Floor::WIDTH - 2, Floor::HEIGHT - 2));
    splitNode(root, maxDepth);
    createRooms(root);
    carveRooms();
    createConnections(root);
    carveCorridors();
    addDoors();

    std::cout << "Actually generated: " << rooms.size() << " rooms";
    float successRate = (float)rooms.size() / targetRoomCount * 100;
    std::cout << " (" << (int)successRate << "% of target)\n\n";
}

int BSPDungeonGenerator::getRoomCount() const {
    return rooms.size();
}

void BSPDungeonGenerator::splitNode(BSPNode* node, int depth) {
    if (depth <= 0) return;

    int minNodeSize = 18;
    if (targetRoomCount > 32) minNodeSize = 16;
    else if (targetRoomCount < 8) minNodeSize = 28;

    if (node->bounds.w < minNodeSize || node->bounds.h < minNodeSize) return;

    int currentLeafCount = countLeafNodes(root);
    if (currentLeafCount >= targetRoomCount * 2.5) return;

    bool splitHorizontal;
    float ratio = (float)node->bounds.w / node->bounds.h;
    if (ratio > 1.2) splitHorizontal = false;
    else if (ratio < 0.8) splitHorizontal = true;
    else splitHorizontal = (currentLeafCount < targetRoomCount * 0.5 && depth > 2) ? rng() % 2 : rng() % 2;

    int splitPos;
    int minSplitSize = minNodeSize / 2 + 2;

    if (splitHorizontal) {
        int minSplit = node->bounds.y + minSplitSize;
        int maxSplit = node->bounds.y + node->bounds.h - minSplitSize;
        if (minSplit >= maxSplit) return;
        int range = maxSplit - minSplit;
        int centerBias = range / 4;
        int splitRange = std::max(1, range - centerBias);
        splitPos = minSplit + centerBias / 2 + rng() % splitRange;
        node->left = new BSPNode(Rect(node->bounds.x, node->bounds.y, node->bounds.w, splitPos - node->bounds.y));
        node->right = new BSPNode(Rect(node->bounds.x, splitPos, node->bounds.w, node->bounds.y + node->bounds.h - splitPos));
    }
    else {
        int minSplit = node->bounds.x + minSplitSize;
        int maxSplit = node->bounds.x + node->bounds.w - minSplitSize;
        if (minSplit >= maxSplit) return;
        int range = maxSplit - minSplit;
        int centerBias = range / 4;
        int splitRange = std::max(1, range - centerBias);
        splitPos = minSplit + centerBias / 2 + rng() % splitRange;
        node->left = new BSPNode(Rect(node->bounds.x, node->bounds.y, splitPos - node->bounds.x, node->bounds.h));
        node->right = new BSPNode(Rect(splitPos, node->bounds.y, node->bounds.x + node->bounds.w - splitPos, node->bounds.h));
    }

    splitNode(node->left, depth - 1);
    splitNode(node->right, depth - 1);
}

int BSPDungeonGenerator::countLeafNodes(BSPNode* node) {
    if (!node) return 0;
    if (node->isLeaf()) return 1;
    return countLeafNodes(node->left) + countLeafNodes(node->right);
}

void BSPDungeonGenerator::createRooms(BSPNode* node) {
    if (!node) return;
    if (node->isLeaf()) {
        int minSize = 8;
        int maxSize = 20;
        int maxW = std::min(maxSize, node->bounds.w - 3);
        int maxH = std::min(maxSize, node->bounds.h - 3);
        int minW = std::min(minSize, maxW);
        int minH = std::min(minSize, maxH);

        if (maxW >= minW && maxH >= minH) {
            int roomW = minW + (rng() % (maxW - minW + 1));
            int roomH = minH + (rng() % (maxH - minH + 1));
            int maxOffsetX = node->bounds.w - roomW - 2;
            int maxOffsetY = node->bounds.h - roomH - 2;
            int offsetX = 1 + (maxOffsetX > 0 ? rng() % maxOffsetX : 0);
            int offsetY = 1 + (maxOffsetY > 0 ? rng() % maxOffsetY : 0);

            node->room = Rect(node->bounds.x + offsetX, node->bounds.y + offsetY, roomW, roomH);
            node->hasRoom = true;
            rooms.push_back(node->room);
        }
        else if (node->bounds.w >= 10 && node->bounds.h >= 10) {
            int roomW = std::min(8, node->bounds.w - 2);
            int roomH = std::min(8, node->bounds.h - 2);
            node->room = Rect(node->bounds.x + 1, node->bounds.y + 1, roomW, roomH);
            node->hasRoom = true;
            rooms.push_back(node->room);
        }
    }
    else {
        createRooms(node->left);
        createRooms(node->right);
    }
}

void BSPDungeonGenerator::carveRooms() {
    for (const Rect& room : rooms) {
        for (int y = room.y; y < room.y + room.h; ++y)
            for (int x = room.x; x < room.x + room.w; ++x)
                floor->tiles[x][y] = '.';
    }
}

void BSPDungeonGenerator::collectRooms(BSPNode* node, std::vector<Rect>& roomList) {
    if (!node) return;
    if (node->hasRoom) roomList.push_back(node->room);
    else {
        collectRooms(node->left, roomList);
        collectRooms(node->right, roomList);
    }
}

void BSPDungeonGenerator::createConnections(BSPNode* node) {
    if (!node || node->isLeaf()) return;

    createConnections(node->left);
    createConnections(node->right);

    if (node->left && node->right) {
        Rect leftRoom = getRandomRoom(node->left);
        Rect rightRoom = getRandomRoom(node->right);
        connections.push_back({ leftRoom, rightRoom });
    }
}

Rect BSPDungeonGenerator::getRandomRoom(BSPNode* node) {
    std::vector<Rect> roomList;
    collectRooms(node, roomList);
    if (roomList.empty()) return Rect(0, 0, 0, 0);
    std::uniform_int_distribution<int> dist(0, roomList.size() - 1);
    return roomList[dist(rng)];
}

void BSPDungeonGenerator::carveCorridors() {
    for (auto& pair : connections) {
        Rect a = pair.first;
        Rect b = pair.second;
        int x1 = a.centerX(), y1 = a.centerY();
        int x2 = b.centerX(), y2 = b.centerY();

        if (rng() % 2) {
            carveHorizontalCorridor(x1, x2, y1);
            carveVerticalCorridor(y1, y2, x2);
        }
        else {
            carveVerticalCorridor(y1, y2, x1);
            carveHorizontalCorridor(x1, x2, y2);
        }
    }
}

void BSPDungeonGenerator::carveHorizontalCorridor(int x1, int x2, int y) {
    if (x2 < x1) std::swap(x1, x2);
    for (int x = x1; x <= x2; ++x) floor->tiles[x][y] = '.';
}

void BSPDungeonGenerator::carveVerticalCorridor(int y1, int y2, int x) {
    if (y2 < y1) std::swap(y1, y2);
    for (int y = y1; y <= y2; ++y) floor->tiles[x][y] = '.';
}

void BSPDungeonGenerator::addDoors() {
    for (auto& pair : connections) {
        Rect a = pair.first;
        Rect b = pair.second;
        int doorX = (a.centerX() + b.centerX()) / 2;
        int doorY = (a.centerY() + b.centerY()) / 2;
        if (floor->tiles[doorX][doorY] == '.') floor->tiles[doorX][doorY] = '+';
    }
}