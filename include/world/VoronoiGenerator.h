#ifndef VORONOIGENERATOR_H
#define VORONOIGENERATOR_H

#include <vector>
#include <random>
#include <utility>

class VoronoiGenerator {
public:
    VoronoiGenerator();
    void generateRegions(std::vector<std::vector<int>>& grid, int size, int numParts);
    
private:
    std::mt19937 rng;
    
    void lloydsRelaxation(std::vector<std::vector<int>>& grid, 
                         std::vector<std::pair<double, double>>& seeds, 
                         int size, int numParts);
};

#endif