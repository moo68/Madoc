#include <random>

#include <madoc/biome_generator.h>
#include <madoc/perlin_noise.h>


std::vector<float> generateBiomeColor(float x, float y, int seed) {
    std::array<glm::vec2, 32> gradientVectors = generateGradients();
    std::array<int, 512> elevationPermutationTable = generatePermutationTable(seed);

    std::mt19937 generator(seed);
    int precipSeed = generator();
    std::array<int, 512> precipPermutationTable = generatePermutationTable(precipSeed);

    float worldHeight = 600;
    float temperature = generateTemperature(y, worldHeight, 1.0f);
    float elevation = generateElevation(elevationPermutationTable, gradientVectors, x, y);
    float precipitation = generatePrecipitation(precipPermutationTable, gradientVectors, x, y);

    // Impassible mountain
    if (elevation >= 0.67) {
        return {0.392f, 0.392f, 0.392f};
    }
    // Mountain
    if (elevation >= 0.62f) {
        return {0.588f, 0.588f, 0.588f};
    }
    // Lower land
    if (elevation >= 0.50f) {
        // Arctic
        if (temperature <= 0.10f) {
            return {0.921f, 0.921f, 0.921f};
        }
        // Tundra
        if (temperature <= 0.33f) {
            return {0.0f, 0.392f, 0.0f};
        }
        // Forest
        if (temperature <= 0.66f) {
            return {0.0f, 0.588f, 0.0f};
        }
        // Savannah
        if (temperature <= 0.85f) {
            return {0.784f, 0.725f, 0.0f};
        }
        // Hot
        else {
            // Rainforest
            if (precipitation >= 0.55f) {
                return {0.0f, 0.784f, 0.0f};
            }
            // Desert
            else {
                return {1.0f, 0.784f, 0.0f};
            }
        }
    }
    // Shallow Sea
    if (elevation >= 0.45f) {
        return {0.392f, 0.588f, 0.784f};
    }
    // Sea
    if (elevation >= 0.40f) {
        return {0.392f, 0.392f, 0.784f};
    }
    // Deep Sea
    else {
        return {0.196f, 0.196f, 0.784f};
    }

}

float generateTemperature(float y, float worldHeight, float tempMult) {
    float equatorValue = worldHeight / 2.0f;
    float distanceFromEquator = abs(equatorValue + y); // y is -, so add it

    return 1 - ((distanceFromEquator * tempMult) / equatorValue);
}

float generateElevation(std::array<int, 512>& permutationTable,
                        std::array<glm::vec2, 32>& gradientVectors, float x, float y) {
    float perlinSample = samplePerlinOctaves(permutationTable, gradientVectors,
                                             x, y, 4, 1.0f, 0.01f, 0.5f, 2.0f);
    return (perlinSample + 1) / 2;
}

float generatePrecipitation(std::array<int, 512>& permutationTable,
                            std::array<glm::vec2, 32>& gradientVectors, float x, float y) {
    float perlinSample = samplePerlinOctaves(permutationTable, gradientVectors,
                                             x, y, 4, 1.0f, 0.01f, 0.5f, 2.0f);
    return (perlinSample + 1) / 2;
}
