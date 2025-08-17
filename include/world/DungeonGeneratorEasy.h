#ifndef DUNGEONGENERATOREASY_H
#define DUNGEONGENERATOREASY_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <set>
#include <string>

struct Floor {
    static const int WIDTH = 64;
    static const int HEIGHT = 64;
    char tiles[WIDTH][HEIGHT];
};

struct Room {
    int left, right, top, bottom;
    Room(int l, int r, int t, int b) : left(l), right(r), top(t), bottom(b) {}
};

// Initialization
void initFloor(Floor &floor);

// Utility
std::vector<int> generateUniqueRandoms(int count, int min, int max, int minSpacing = 4);

// Wall + room generation
std::vector<Room> addWallsAndGetRooms(Floor &floor, int vCount, int hCount);

// Doors
void addDoorsToRooms(Floor &floor, const std::vector<Room> &rooms);

// Debug / printing
void printFloor(const Floor &floor);
void printRoomInfo(const std::vector<Room> &rooms);

#endif // DUNGEON_H
