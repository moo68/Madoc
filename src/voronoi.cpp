#include <iostream>
#include <iomanip>
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

    // TODO: make sure feature points can't spawn on already existing feature points?
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            inputGrid.cells[(y * inputGrid.width) + x] = 0;
        }
    }

    // Randomly assign some grid cells as feature points
    u_int16_t voronoiCellID = 1;
    u_int16_t macroCellID = 0;
    for (int macroY = 0; macroY < (inputGrid.height / inputGrid.macroHeight); macroY++) {
        for (int macroX = 0; macroX < (inputGrid.width / inputGrid.macroWidth); macroX++) {
            for (int i = 0; i < 3; i++) {
                int featurePointX = randomX(generator) + (macroX * inputGrid.macroWidth);
                int featurePointY = randomY(generator) + (macroY * inputGrid.macroHeight);

                // Edit the grid to include the feature point and add the feature point to
                // the grid's featurePointList
                inputGrid.cells[(featurePointY * inputGrid.width) + featurePointX] = voronoiCellID;
                inputGrid.featurePointList.push_back({featurePointX, featurePointY,
                    voronoiCellID, macroCellID});
                voronoiCellID++;
            }
            macroCellID++;
        }
    }

    // Iterate through the grid, assigning each cell the ID of its closest feature point
}

void printGrid(const VoronoiGrid &inputGrid) {
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            std::cout << std::setw(2) << inputGrid.cells[(y * inputGrid.width) + x] << " ";
        }
        std::cout << "\n";
    }

    std::cout << std::endl;

    for (int i = 0; i < inputGrid.featurePointList.size(); i++) {
        std::cout << "(" << inputGrid.featurePointList[i].x << ", " <<
            inputGrid.featurePointList[i].y << ") : " <<
                inputGrid.featurePointList[i].macroCellID << "\n";
    }
}
