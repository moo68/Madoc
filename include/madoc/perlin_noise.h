#pragma once


#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


std::array<glm::vec2, 32> generateGradients();

std::array<int, 512> generatePermutationTable(int seed);

float dotGridGradient(float xSample, float zSample, int xCorner, int yCorner);

float fade(float t);

float lerp(float a, float b, float t);

float samplePerlin(float x, float z);

float samplePerlinOctaves(float x, float z, int octaves, float amplitude,
                          float frequency, float persistence, float lacunarity);
