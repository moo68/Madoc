#pragma once

#include <vector>

#include <glm/glm.hpp>


// TODO: Pass a WorldInfo struct for all the specific generation settings
struct WorldInfo {
    int seed;
    int worldWidth;
    int worldHeight;
    float tempMult;
};


std::vector<float> generateBiomeColor(float x, float y, int seed);

float generateTemperature(float y, float worldHeight, float tempMult);

float generateElevation(std::array<int, 512>& permutationTable,
                        std::array<glm::vec2, 32>& gradientVectors, float x, float y);

float generatePrecipitation(std::array<int, 512>& permutationTable,
                            std::array<glm::vec2, 32>& gradientVectors, float x, float y);
