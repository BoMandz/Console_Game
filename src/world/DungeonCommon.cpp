#include "include/world/DungeonCommon.h"
#include <algorithm>

namespace RandomUtils {
    std::vector<int> generateUniquePositions(int count, int min, int max, int minSpacing) {
        std::vector<int> positions;
        if (max - min < minSpacing * (count - 1)) return {};
        
        std::vector<int> validPositions;
        for (int i = min; i <= max; i++) {
            validPositions.push_back(i);
        }
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(validPositions.begin(), validPositions.end(), g);
        
        positions.push_back(validPositions[0]);
        for (size_t i = 1; i < validPositions.size() && positions.size() < static_cast<size_t>(count); i++) {
            bool valid = true;
            for (int pos : positions) {
                if (abs(validPositions[i] - pos) < minSpacing) {
                    valid = false;
                    break;
                }
            }
            if (valid) positions.push_back(validPositions[i]);
        }
        
        std::sort(positions.begin(), positions.end());
        return positions;
    }
}