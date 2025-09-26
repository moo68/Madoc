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

    // TODO: Check that width/height is evenly divisible by macroWidth/macroHeight
    outputGrid.macroWidth = macroWidth;
    outputGrid.macroHeight = macroHeight;

    return outputGrid;
}

void generateVoronoiCells(VoronoiGrid &inputGrid, const int seed) {
    // Random integer generation
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> randomX(0, inputGrid.macroWidth - 1);
    std::uniform_int_distribution<int> randomY(0, inputGrid.macroHeight - 1);

    // TODO: make sure feature points can't spawn on already existing feature points?
    // Assign all cells in the grid to a default ID of 0 (we should eventually be able
    // to get rid of this)
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            inputGrid.cells[(y * inputGrid.width) + x] = 0;
        }
    }

    // Randomly assign some grid cells as feature points
    const int numMacroX = inputGrid.width / inputGrid.macroWidth;
    const int numMacroY = inputGrid.height / inputGrid.macroHeight;
    u_int16_t voronoiID = 1;
    //u_int16_t macroCellID = 0;
    for (int macroY = 0; macroY < numMacroY; macroY++) {
        for (int macroX = 0; macroX < numMacroX; macroX++) {
            std::vector<FeaturePoint> featurePoints;
            featurePoints.reserve(3);
            for (int i = 0; i < 3; i++) {
                int featurePointX = randomX(generator) + (macroX * inputGrid.macroWidth);
                int featurePointY = randomY(generator) + (macroY * inputGrid.macroHeight);

                // Edit the grid to include the feature point
                inputGrid.cells[(featurePointY * inputGrid.width) + featurePointX] = voronoiID;
                featurePoints.push_back({featurePointX, featurePointY, voronoiID});
                voronoiID++;
            }
            inputGrid.macroCells.push_back({macroX, macroY, featurePoints});
            featurePoints.clear();
            //std::pair<int, int> macroCoords = {macroX, macroY};
            //inputGrid.featurePointMap.insert(macroCoords, featurePointList);
            //macroCellID++;
        }
    }

    // Iterate through the grid, assigning each cell the ID of its closest feature point
    // This will be used for a bitmask:
    /*for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            int currentMacroCellX = x / inputGrid.macroWidth;
            int currentMacroCellY = y / inputGrid.macroWidth;

            int startingX = (currentMacroCellX - 1) * inputGrid.macroWidth;
            int startingY = (currentMacroCellY - 1) * inputGrid.macroHeight;
            if (startingX < 0) { startingX = 0; }
            if (startingY < 0) { startingY = 0; }

            int endingX = ((currentMacroCellX + 1) * inputGrid.macroWidth)
                + inputGrid.macroWidth - 1;
            int endingY = ((currentMacroCellY + 1) * inputGrid.macroHeight)
                + inputGrid.macroHeight - 1;
            if (endingX > inputGrid.width - 1) { endingX = inputGrid.width - 1; }
            if (endingY > inputGrid.height - 1) { endingY = inputGrid.height - 1; }


        }
    }*/

    /*int totalNumMacroCells = numMacroX * numMacroY;
    std::array<int, 9> macroCellsToCheck{};
    for (int i = 0; i < totalNumMacroCells; i++) {
        macroCellsToCheck = {
            (i - 1) - numMacroX, i - numMacroX, (i + 1) - numMacroX,
            i - 1,               i,             i + 1,
            (i - 1) + numMacroX, i + numMacroX, (i + 1) + numMacroX
        };

        // If the macro cell is in bounds, get its feature points
        for (int j = 0; j < macroCellsToCheck.size(); j++) {
            int currentMacroRow = (i / numMacroX) * numMacroX;
            if (!(macroCellsToCheck[j] < 0 || macroCellsToCheck[j] > currentMacroRow * numMacroX)) {

            }
        }
    }*/
}

void printGrid(const VoronoiGrid &inputGrid) {
    // Print out the grid itself
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            std::cout << std::setw(2) << inputGrid.cells[(y * inputGrid.width) + x] << " ";
        }
        std::cout << "\n";
    }

    std::cout << std::endl;

    // Print out the coordinates of each feature point and what macroCell they lie in
    for (int i = 0; i < inputGrid.macroCells.size(); i++) {
        std::cout << "(" << inputGrid.macroCells[i].x << ", " <<
            inputGrid.macroCells[i].y << ") : ";
        for (int j = 0; j < inputGrid.macroCells[i].featurePoints.size(); j++) {
            std::cout << inputGrid.macroCells[i].featurePoints[j].x <<
                ", " << inputGrid.macroCells[i].featurePoints[j].y << " : ";
        }
        std::cout << "\n";
    }
}
