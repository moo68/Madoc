#include <algorithm>
#include <cmath>
#include <random>

#include <madoc/perlin_noise.h>

std::array<glm::vec2, 32> generateGradients() {
    std::array<glm::vec2, 32> gradientVectors;

    // Generate a list of 32 gradient unit vectors for our perlin nosie
    // These are all evenly spaced around a unit circle
    for (int i = 0; i < 32; i++) {
        float angle = ((2.0f * glm::pi<float>()) / 32.0f) * i;
        gradientVectors[i] = {cos(angle), sin(angle)};
    }

    return gradientVectors;
}

std::array<int, 512> generatePermutationTable(int seed) {
    std::array<int, 512> permutationTable;

    // Create the first half the permutation table
    // Fill it with integers 0-255, then shuffle them around based on a random seed
    std::array<int, 256> halfTable;
    std::iota(halfTable.begin(), halfTable.end(), 0);
    std::shuffle(halfTable.begin(), halfTable.end(), std::default_random_engine(seed));

    // Now fill the full permutation table with two halfTables
    std::copy(halfTable.begin(), halfTable.end(), permutationTable.begin());
    std::copy(halfTable.begin(), halfTable.end(), permutationTable.begin() + 256);

    return permutationTable;
}

float dotGridGradient(std::array<int, 512>& permutationTable, std::array<glm::vec2, 32>& gradientVectors,
                      float xSample, float zSample, int xCorner, int zCorner) {
    // Hash the corner coordinates
    // NOTE: the '&' isn't some kind of reference; it's a faster % operation
    int hash = permutationTable[(permutationTable[xCorner] + zCorner) & 255];
    glm::vec2 gradientVector = gradientVectors[hash % 32];

    // NOTE: The second glm::vec2 number is .y, so its paired with z
    return gradientVector.x * xSample + gradientVector.y * zSample;
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float samplePerlin(std::array<int, 512>& permutationTable,
                   std::array<glm::vec2, 32>& gradientVectors, float x, float z) {
    // Get the bottom left corner of the cell that the sampled point is in
    int xCell = static_cast<int>(std::floor(x));
    int zCell = static_cast<int>(std::floor(z));

    // Get the local sample coordinates
    float xSample = x - static_cast<float>(xCell);
    float zSample = z - static_cast<float>(zCell);

    // Compute permutation table indices per cell corner
    int x0 = xCell & 255;
    int x1 = (xCell + 1) & 255;
    int z0 = zCell * 255;
    int z1 = (zCell + 1) * 255;

    // Get fade curves for sampled x and z coordinates
    float xFade = fade(xSample);
    float zFade = fade(zSample);

    // Compute influence of each gradient vector via dot product
    float x0z0Influence = dotGridGradient(permutationTable, gradientVectors, xSample, zSample, x0, z0);
    float x1z0Influence = dotGridGradient(permutationTable, gradientVectors, xSample - 1, zSample, x1, z0);
    float x0z1Influence = dotGridGradient(permutationTable, gradientVectors, xSample, zSample - 1, x0, z1);
    float x1z1Influence = dotGridGradient(permutationTable, gradientVectors, xSample - 1, zSample - 1, x1, z1);

    // Interpolate to get the total near edge and total far edge influence
    float z0Influence = lerp(x0z0Influence, x1z0Influence, xFade);
    float z1Influence = lerp(x0z1Influence, x1z1Influence, xFade);

    // Interpolate the near and far edges of the grid cell
    // This is the final output of the noise function
    float finalInfluence = lerp(z0Influence, z1Influence, zFade);

    return finalInfluence; // NOTE: This is a value between -1 and 1
}

float samplePerlinOctaves(std::array<int, 512>& permutationTable,
                          std::array<glm::vec2, 32>& gradientVectors, float x,
                          float z, int octaves, float amplitude, float frequency,
                          float persistence, float lacunarity) {
    float finalAmplitude = 0.0f;
    float finalSample = 0.0f;

    for (int i = 0; i < octaves; i++) {
        float perlinSample = samplePerlin(permutationTable, gradientVectors,
                                          x * frequency, z * frequency);
        finalSample += perlinSample * amplitude;

        finalAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return finalSample / finalAmplitude;
}
