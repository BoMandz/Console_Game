#ifndef DUNGEON_GENERATOR_MID_H
#define DUNGEON_GENERATOR_MID_H

#include <vector>
#include <random>
#include <utility>

// Forward declarations
struct Floor;
struct Rect;
struct BSPNode;

class BSPDungeonGenerator {
private:
    std::mt19937 rng;
    Floor* floor;
    BSPNode* root;
    std::vector<Rect> rooms;
    std::vector<std::pair<Rect, Rect>> connections;
    int targetRoomCount;
    int maxDepth;

    // Private member functions
    int calculateDepth(int roomCount);
    void splitNode(BSPNode* node, int depth);
    int countLeafNodes(BSPNode* node);
    void createRooms(BSPNode* node);
    void carveRooms();
    void collectRooms(BSPNode* node, std::vector<Rect>& roomList);
    void createConnections(BSPNode* node);
    Rect getRandomRoom(BSPNode* node);
    void carveCorridors();
    void carveHorizontalCorridor(int x1, int x2, int y);
    void carveVerticalCorridor(int y1, int y2, int x);
    void addDoors();

public:
    BSPDungeonGenerator();
    ~BSPDungeonGenerator();

    void generate(Floor& f, int roomCount = 16);
    int getRoomCount() const;
};

#endif // DUNGEON_GENERATOR_MID_H