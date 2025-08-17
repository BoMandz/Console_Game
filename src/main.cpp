#include <iostream>
#include "include/world/DungeonGeneratorMid.h"

// Note: The structs Rect, Floor, and BSPNode need to be
// accessible to main.cpp. For this example, they can be
// placed in a separate header file (e.g., DungeonTypes.h)
// that is included by both dungeonGeneratorMid.cpp and main.cpp.
// Or, for simplicity in this specific context, they can be
// included directly in dungeonGeneratorMid.cpp and their
// definitions copied here. A more robust solution involves
// putting them in their own header file.

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

int main() {
    Floor floor;
    BSPDungeonGenerator generator;
    generator.generate(floor, 20);
    floor.print();
    std::cout << "\nTotal rooms: " << generator.getRoomCount() << "\n";

    while (true)
    {
        /* code */
    }
    
    return 0;
}