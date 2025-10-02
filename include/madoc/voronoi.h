#pragma once

#include <vector>


/*
 * Represents a feature point on a 2D plane with an x and y coordinate,
 * and a unique ID for its voronoi cell.
 */
struct FeaturePoint {
    int x, y;
    u_int16_t voronoiID;
};

/*
 * Represents a macro cell on the grid, which contains different feature points.
 */
struct MacroCell {
    std::vector<FeaturePoint> featurePoints;
};

/*
 * All the data necessary to create a grid of cells which are then used to
 * create voronoi cells. Note that the vector 'cells' is 1D and stores
 * voronoi cell IDs--we shouldn't ever need more than 65k voronoi cells.
 *
 * The fields macroWidth and macroHeight refer to the width and height of each
 * 'macro cell' in the grid. Macro cells are just larger areas on the grid made
 * up of individual cells.
 */
struct VoronoiGrid {
    int width, height;
    std::vector<u_int16_t> cells;
    int macroWidth, macroHeight;
    std::vector<MacroCell> macroCells;
    std::vector<FeaturePoint*> featurePointPointers;
};

/*
 * Another grid, but this time just a bitmask with either 0 or 1 as values.
 */
struct VoronoiBitmask {
    int width, height;
    std::vector<bool> mask;
};

/*
 * Instantiates a "blank" VoronoiGrid by assigning its width, height,
 * macroWidth, and macroHeight.
 */
VoronoiGrid createVoronoiGrid(int width, int height, int macroWidth, int macroHeight);

/*
 * Takes a reference to an already existing VoronoiGrid to actually create
 * voronoi cells pseudorandomly using a given seed. minFeaturePoints and
 * maxFeaturePoints refer to the min and max per macro cell, not the whole grid.
 */
void generateVoronoiCells(VoronoiGrid& inputGrid, int seed, int minFeaturePoints,
    int maxFeaturePoints);

/*
 * based on a given grid and ID, return a bitmask of only that specific voronoi cell
 */
VoronoiBitmask generateVoronoiBitmask(const VoronoiGrid& inputGrid, u_int16_t voronoiID);

/*
 * Prints out the given voronoi grid into the terminal.
 */
void printVoronoiGrid(const VoronoiGrid& inputGrid);

/*
 * Prints out the given bitmask into the terminal.
 */
void printBitmask(const VoronoiBitmask& inputGrid, u_int16_t voronoiID);
