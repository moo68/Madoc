#pragma once

#include <vector>

#include <madoc/voronoi.h>


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
