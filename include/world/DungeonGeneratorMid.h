#ifndef DUNGEON_GENERATOR_MID_H
#define DUNGEON_GENERATOR_MID_H

#include "DungeonCommon.h"
#include "BSPTree.h"

using MidFloor = Floor<DungeonConfig::MID_WIDTH, DungeonConfig::MID_HEIGHT>;

class MidDungeonGenerator {
public:
    MidDungeonGenerator();
    void generate(MidFloor& floor, int roomCount = 16);
    int getRoomCount() const { return rooms.size(); }
    int calculateDepth(int roomCount) const;
    void collectRooms(BSPNode* node, std::vector<Rect>& roomList);
    Rect getRandomRoom(BSPNode* node);
    void createConnections(BSPNode* node);
    
private:
    BSPTree bspTree;
    std::vector<Rect> rooms;
    std::vector<std::pair<Rect, Rect>> connections;
    std::mt19937 rng;

    void carveRooms(MidFloor& floor);
    void createConnections();
    void carveCorridors(MidFloor& floor);
    void addDoors(MidFloor& floor);
};

#endif