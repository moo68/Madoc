#pragma once

#include <vector>


struct WorldInfo {
    int seed;
    int worldWidth;
    int worldHeight;
    float tempMult;
};


std::vector<float> generateBiomeColor(float x, float y, int seed);

float generateTemperature(float y, float worldHeight, float tempMult);

float generateElevation();

float generatePrecipitation();
