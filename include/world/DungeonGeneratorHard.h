#ifndef DUNGEON_GENERATOR_HARD_H
#define DUNGEON_GENERATOR_HARD_H

#include "include/world/DungeonCommon.h"
#include "include/world/algorithm/BSPTree.h"

using HardFloor = Floor<DungeonConfig::HARD_WIDTH, DungeonConfig::HARD_HEIGHT>;

class HardDungeonGenerator {
public:
    HardDungeonGenerator();
    void generate(HardFloor& floor, int numParts = 8);
    int getRegionCount() const { return numParts; }
    void draw(const std::vector<std::vector<int>>& grid,
                               const std::vector<std::vector<std::vector<char>>>& dungeons);
private:
    int numParts;
    std::mt19937 rng;
    
    void generateVoronoiRegions(std::vector<std::vector<int>>& grid, int size);
    void generateBSPDungeons(std::vector<std::vector<int>>& grid, 
                            std::vector<std::vector<std::vector<char>>>& dungeons,
                            std::vector<std::vector<Rect>>& regionRooms);
    void placeMarkers(std::vector<std::vector<int>>& grid,
                     std::vector<std::vector<std::vector<char>>>& dungeons,
                     std::vector<std::vector<Rect>>& regionRooms);
    void drawLCorridor(std::vector<std::vector<char>>& dungeon, int x1, int y1, int x2, int y2);
    void connectRoomsRecursively(BSPNode* node, std::vector<std::vector<char>>& dungeon);
    std::pair<int, int> findFurthestInRegion(const std::vector<std::vector<int>>& grid,
                                            const std::vector<std::vector<std::vector<char>>>& dungeons,
                                            int regionId, int gx, int gy);
    
};

#endif