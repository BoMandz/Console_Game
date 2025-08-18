#ifndef DUNGEON_GENERATOR_EASY_H
#define DUNGEON_GENERATOR_EASY_H

#include "DungeonCommon.h"

using EasyFloor = Floor<DungeonConfig::EASY_WIDTH, DungeonConfig::EASY_HEIGHT>;

class EasyDungeonGenerator {
public:
    void generate(EasyFloor& floor);
    void print(const EasyFloor& floor) const;
    
private:
    struct Room : public Rect {
        Room(int l, int r, int t, int b) 
            : Rect(l, t, r-l, b-t) {}
    };

    void initFloor(EasyFloor& floor);
    std::vector<Room> createRooms(EasyFloor& floor, int vWalls, int hWalls);
    void addDoors(EasyFloor& floor, const std::vector<Room>& rooms);
};

#endif