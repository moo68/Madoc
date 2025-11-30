#include <madoc/biome_generator.h>
#include <madoc/perlin_noise.h>


std::vector<float> generateBiomeColor(float x, float y, int seed) {
    std::array<glm::vec2, 32> gradientVectors = generateGradients();
    std::array<int, 512> permutationTable = generatePermutationTable(seed);

    float worldHeight = 600;
    float temperature = generateTemperature(y, worldHeight, 1.0f);
}

float generateTemperature(float y, float worldHeight, float tempMult) {
    float equatorValue = worldHeight / 2.0f;
    float distanceFromEquator = abs(equatorValue + y); // y is -, so add it

    return 1 - ((distanceFromEquator * tempMult) / equatorValue);
}
