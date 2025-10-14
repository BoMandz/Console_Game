#include "include/world/VoronoiGenerator.h"

#include <algorithm>

VoronoiGenerator::VoronoiGenerator() : rng(std::random_device{}()) {}

void VoronoiGenerator::generateRegions(std::vector<std::vector<int>>& grid, int size, int numParts) {
    std::uniform_int_distribution<> distCoord(0, size - 1);
    
    // Random seed points for Voronoi
    std::vector<std::pair<double, double>> seeds(numParts);
    for (int i = 0; i < numParts; i++)
        seeds[i] = {distCoord(rng), distCoord(rng)};
    
    lloydsRelaxation(grid, seeds, size, numParts);
}

void VoronoiGenerator::lloydsRelaxation(std::vector<std::vector<int>>& grid, 
                                       std::vector<std::pair<double, double>>& seeds, 
                                       int size, int numParts) {
    const int MAX_ITERATIONS = 50;
    const double TARGET_DEVIATION = 0.20;
    
    for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
        // Assign each cell to nearest seed (Voronoi)
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int bestIdx = 0;
                double bestDist = std::numeric_limits<double>::max();
                for (int i = 0; i < numParts; i++) {
                    double dx = x - seeds[i].first;
                    double dy = y - seeds[i].second;
                    double distSq = dx * dx + dy * dy;
                    if (distSq < bestDist) {
                        bestDist = distSq;
                        bestIdx = i;
                    }
                }
                grid[y][x] = bestIdx;
            }
        }
        
        // Calculate zone sizes
        std::vector<int> zoneSizes(numParts, 0);
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                zoneSizes[grid[y][x]]++;
            }
        }
        
        // Check if zones are balanced enough
        int minSize = *std::min_element(zoneSizes.begin(), zoneSizes.end());
        int maxSize = *std::max_element(zoneSizes.begin(), zoneSizes.end());
        double currentDeviation = (double)(maxSize - minSize) / maxSize;
        
        if (currentDeviation <= TARGET_DEVIATION) {
            break;
        }
        
        // Calculate new centroids for each zone
        std::vector<std::pair<double, double>> newSeeds(numParts, {0.0, 0.0});
        std::vector<int> counts(numParts, 0);
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int zoneId = grid[y][x];
                newSeeds[zoneId].first += x;
                newSeeds[zoneId].second += y;
                counts[zoneId]++;
            }
        }
        
        // Update seed positions to centroids
        for (int i = 0; i < numParts; i++) {
            if (counts[i] > 0) {
                newSeeds[i].first /= counts[i];
                newSeeds[i].second /= counts[i];
                
                // Add small random perturbation
                std::uniform_real_distribution<> perturbation(-1.0, 1.0);
                newSeeds[i].first += perturbation(rng);
                newSeeds[i].second += perturbation(rng);
                
                // Clamp to bounds
                newSeeds[i].first = std::max(0.0, std::min((double)(size - 1), newSeeds[i].first));
                newSeeds[i].second = std::max(0.0, std::min((double)(size - 1), newSeeds[i].second));
            }
        }
        
        seeds = newSeeds;
    }
}