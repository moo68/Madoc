#include <iostream>
#include <random>

#include <madoc/voronoi.h>


VoronoiGrid createVoronoiGrid(const int width, const int height,
    const int macroWidth, const int macroHeight) {
    VoronoiGrid outputGrid;

    outputGrid.width = width;
    outputGrid.height = height;

    outputGrid.cells.resize(width * height);

    // TODO: Check that width/height is evenly divisible by macroWIdth/macroHeight
    outputGrid.macroWidth = macroWidth;
    outputGrid.macroHeight = macroHeight;

    return outputGrid;
}

void generateVoronoiCells(VoronoiGrid &inputGrid, const int seed) {
    // Random integer generation
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> randomX(0, inputGrid.macroWidth);
    std::uniform_int_distribution<int> randomY(0, inputGrid.macroHeight);

    // Randomly assign some grid cells as feature points
    // TODO: make sure feature points can't spawn on already existing feature points?
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            inputGrid.cells[(y * inputGrid.width) + x] = 0;
        }
    }

    int voronoiCellID = 1;
    for (int macroY = 0; macroY < (inputGrid.height / inputGrid.macroHeight); macroY++) {
        for (int macroX = 0; macroX < (inputGrid.width / inputGrid.macroWidth); macroX++) {
            for (int i = 0; i < 3; i++) {
                int featurePointX = randomX(generator) + (macroX * inputGrid.macroWidth);
                int featurePointY = randomY(generator) + (macroY * inputGrid.macroHeight);

                inputGrid.cells[(featurePointY * inputGrid.width) + featurePointX] = voronoiCellID;
                voronoiCellID++;
            }
        }
    }
}

void printGrid(const VoronoiGrid &inputGrid) {
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            std::cout << inputGrid.cells[(y * inputGrid.width) + x] << " ";
        }
        std::cout << "\n";
    }
}
