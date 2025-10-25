#pragma once

#include <vector>

#include <madoc/voronoi.h>


/*
 * Lists of all possible directions that a bitmask (or really any grid)
 * can be iterated over.
 */
enum Direction {
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
    NORTH,
    NORTHEAST
};

/*
 * Returns the top left filled in cell of the bitmask. This is where the
 * edge-scanning algorithm will start from.
 */
int getStartingCell(const VoronoiBitmask& bitmask);

/*
 * Return a series of vertices that are on the edges of the voronoi cell
 * in clockwise order
 */
std::vector<float> getEdgeVertices(const VoronoiBitmask& bitmask);

/*
 * Get the center of the shape based on its edge vertices and add
 * its center vertex to the beginning of its list of vertices
 */
std::vector<float> getCenterVertex(const VoronoiBitmask& bitmask, std::vector<float>& vertices);

/*
 * Return the integer of the cell that was moved to based on the current cell and
 * the direction of movement
 */
int moveAcrossBitmask(const VoronoiBitmask& bitmask, int currentCell, Direction direction);
