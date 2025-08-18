#ifndef DUNGEON_COMMON_H
#define DUNGEON_COMMON_H

#include <vector>
#include <random>

// Common dungeon dimensions
namespace DungeonConfig {
    constexpr int EASY_WIDTH = 64;
    constexpr int EASY_HEIGHT = 64;
    constexpr int MID_WIDTH = 128;
    constexpr int MID_HEIGHT = 128;
}

// Unified rectangle structure
struct Rect {
    int x, y, w, h;
    
    Rect(int x = 0, int y = 0, int w = 0, int h = 0)
        : x(x), y(y), w(w), h(h) {}
    
    int left()   const { return x; }
    int right()  const { return x + w; }
    int top()    const { return y; }
    int bottom() const { return y + h; }
    int centerX() const { return x + w/2; }
    int centerY() const { return y + h/2; }
    
    bool intersects(const Rect& other) const {
        return x < other.x + other.w &&
               x + w > other.x &&
               y < other.y + other.h &&
               y + h > other.y;
    }
};

// Floor template for different sizes
template <int WIDTH, int HEIGHT>
struct Floor {
    static constexpr int Width = WIDTH;
    static constexpr int Height = HEIGHT;
    char tiles[WIDTH][HEIGHT] = {};
    
    void fill(char c) {
        for (int y = 0; y < HEIGHT; ++y)
            for (int x = 0; x < WIDTH; ++x)
                tiles[x][y] = c;
    }
};

// Random number utilities
namespace RandomUtils {
    std::vector<int> generateUniquePositions(int count, int min, int max, int minSpacing = 4);
}

#endif