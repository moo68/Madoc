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

    outputGrid.numFeaturePoints = 0;

    return outputGrid;
}

void generateVoronoiCells(VoronoiGrid &inputGrid, const int seed,
    const int minFeaturePoints, const int maxFeaturePoints) {
    // Random integer generation
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> randomX(0, inputGrid.macroWidth - 1);
    std::uniform_int_distribution<int> randomY(0, inputGrid.macroHeight - 1);
    std::uniform_int_distribution<int> randomFeaturePoints(minFeaturePoints, maxFeaturePoints);

    const int numMacroX = inputGrid.width / inputGrid.macroWidth;
    const int numMacroY = inputGrid.height / inputGrid.macroHeight;
    u_int16_t voronoiID = 0;
    std::vector<int> generatedPoints;

    // Randomly assign some grid cells as feature points
    for (int macroY = 0; macroY < numMacroY; macroY++) {
        for (int macroX = 0; macroX < numMacroX; macroX++) {
            std::vector<FeaturePoint> featurePoints;
            featurePoints.reserve(maxFeaturePoints);
            for (int i = 0; i < randomFeaturePoints(generator); i++) {
                int featurePointX = randomX(generator) +
                    (macroX * inputGrid.macroWidth);
                int featurePointY = randomY(generator) +
                    (macroY * inputGrid.macroHeight);

                // Check if the generated point is a duplicate
                bool duplicatePoint = false;
                for (int j = 0; j < generatedPoints.size(); j += 2) {
                    if (generatedPoints[j] == featurePointX &&
                        generatedPoints[j + 1] == featurePointY) {
                        duplicatePoint = true;
                        i--;
                    }
                }
                generatedPoints.push_back(featurePointX);
                generatedPoints.push_back(featurePointY);

                // Edit the grid to include the feature point
                if (!duplicatePoint) {
                    inputGrid.cells[(featurePointY * inputGrid.width) +
                        featurePointX] = voronoiID;
                    featurePoints.push_back({featurePointX, featurePointY,
                        voronoiID});
                    inputGrid.numFeaturePoints++;
                    voronoiID++;
                }
            }
            inputGrid.macroCells.push_back({featurePoints});
            featurePoints.clear();
        }
    }

    // Get a list of points to each voronoi cell in order by their voronoiID
    inputGrid.featurePointPointers.reserve(maxFeaturePoints * numMacroX *numMacroY);
    for (int i = 0; i < inputGrid.macroCells.size(); i++) {
        for (int j = 0; j < inputGrid.macroCells[i].featurePoints.size(); j++) {
            FeaturePoint* currentPointPointer =
                &inputGrid.macroCells[i].featurePoints[j];
            inputGrid.featurePointPointers.push_back(currentPointPointer);
        }
    }

    // Iterate through each grid cell, getting what macro cell it's in
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            int currentMacroX = x / inputGrid.macroWidth;
            int currentMacroY = y / inputGrid.macroHeight;

            int shortestDistance = std::numeric_limits<int>::max();
            u_int16_t cellID;

            // Get all valid adjacent macro cells and the feature points in those macro cells
            for (int checkedMacroY = currentMacroY - 1;
                checkedMacroY <= currentMacroY + 1; checkedMacroY++) {
                for (int checkedMacroX = currentMacroX - 1;
                    checkedMacroX <= currentMacroX + 1; checkedMacroX++) {
                    if (checkedMacroX >= 0 && checkedMacroX < numMacroX &&
                        checkedMacroY >= 0 && checkedMacroY < numMacroY) {
                        MacroCell& currentMacroCell =
                            inputGrid.macroCells[(checkedMacroY * numMacroX) + checkedMacroX];
                        std::vector<FeaturePoint>& currentFeaturePoints =
                            currentMacroCell.featurePoints;

                        // For each feature point in this macro cell, calculate the distance
                        // and check whether it's the shortest
                        for (int i = 0; i < currentFeaturePoints.size(); i++) {
                            int dx = currentFeaturePoints[i].x - x;
                            int dy = currentFeaturePoints[i].y - y;
                            int distance = (dx * dx) + (dy * dy);

                            if (distance < shortestDistance) {
                                cellID = currentFeaturePoints[i].voronoiID;
                                shortestDistance = distance;
                            }
                        }
                    }
                }
            }

            // Set the cell's final voronoiID
            inputGrid.cells[(y * inputGrid.width) + x] = cellID;
        }
    }
}

VoronoiBitmask generateVoronoiBitmask(const VoronoiGrid& inputGrid, const u_int16_t voronoiID) {
    // Get the corresponding feature point to the given voronoiID and related coordinates
    const FeaturePoint* currentFeaturePoint = inputGrid.featurePointPointers[voronoiID];
    int featureX = currentFeaturePoint->x;
    int featureY = currentFeaturePoint->y;
    int macroX = featureX / inputGrid.macroWidth;
    int macroY = featureY / inputGrid.macroHeight;

    // startingX/Y and endingX/Y are voronoiGrid coords, not bitmask
    int startingX = (macroX - 1) * inputGrid.macroWidth;
    int startingY = (macroY - 1) * inputGrid.macroHeight;
    if (startingX < 0) { startingX = 0; }
    if (startingY < 0) { startingY = 0; }

    int endingX = ((macroX + 1) * inputGrid.macroWidth)
                + inputGrid.macroWidth - 1;
    int endingY = ((macroY + 1) * inputGrid.macroHeight)
        + inputGrid.macroHeight - 1;
    if (endingX > inputGrid.width - 1) { endingX = inputGrid.width - 1; }

    bool yCorrection = false;
    if (endingY > inputGrid.height - 1) {
        endingY = inputGrid.height - 1;
        yCorrection = true;
    }

    // Interior dimensions (actual cells we want to copy)
    int interiorWidth = (endingX - startingX) + 1;
    int interiorHeight = (endingY - startingY) + 1;

    // Padded dimensions (+1 border each side)
    int paddedWidth = interiorWidth + 2;
    int paddedHeight = interiorHeight + 2;

    VoronoiBitmask bitmask;
    bitmask.width = interiorWidth;
    bitmask.height = interiorHeight;
    bitmask.xOffset = startingX;
    bitmask.yOffset = startingY;
    bitmask.mask.resize(paddedWidth * paddedHeight, false);

    // Iterate through the inputGrid to fill the bitmask
    for (int y = startingY; y <= endingY; y++) {
        for (int x = startingX; x <= endingX; x++) {
            int currentCell = (y * inputGrid.width) + x;
            int bitmaskX = x - startingX + 0;
            int bitmaskY = y - startingY + 1;
            /*if (yCorrection) {
                bitmaskY -= 1;
            }*/
            int currentBitmaskCell = ((bitmaskY + 0) * interiorWidth) + (bitmaskX + 0);

            if (inputGrid.cells[currentCell] == voronoiID) {
                bitmask.mask[currentBitmaskCell] = true;
            }
        }
    }
    /*for (int y = 1; y < interiorHeight; y++) {
        for (int x = 1; x < interiorWidth; x++) {
            int voronoiX = startingX + x - 1;
            int voronoiY = startingY + y - 1;
            int voronoiCell = (voronoiX * (endingX - startingX)) + voronoiY;

            if (inputGrid.cells[voronoiCell] == voronoiID) {
                bitmask.mask[(x * interiorWidth) + y] = true;
            }
            else {
                bitmask.mask[(x * interiorWidth) + y] = false;
            }
        }
    }*/

    return bitmask;
}

void printVoronoiGrid(const VoronoiGrid& inputGrid) {
    for (int y = 0; y < inputGrid.height; y++) {
        for (int x = 0; x < inputGrid.width; x++) {
            std::cout << std::setw(3) << inputGrid.cells[(y * inputGrid.width) + x] << " ";
        }
        std::cout << "\n";
    }

    std::cout << std::endl;
}

void printBitmask(const VoronoiBitmask& inputGrid, const u_int16_t voronoiID) {
    std::cout << "Voronoi cell " << voronoiID << ":\n";
    for (int y = 0; y < inputGrid.height + 2; y++) {
        for (int x = 0; x < inputGrid.width + 2; x++) {
            std::cout << inputGrid.mask[(y * inputGrid.width) + x] << " ";
        }
        std::cout << "\n";
    }

    std::cout << std::endl;
}
