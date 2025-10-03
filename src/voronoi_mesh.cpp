#include <iostream>

#include <madoc/voronoi_mesh.h>


int getStartingCell(const VoronoiBitmask &bitmask) {
    for (int y = 0; y < bitmask.height; y++) {
        for (int x = 0; x < bitmask.width; x++) {
            int currentCell = (y * bitmask.width) + x;
            if (bitmask.mask[currentCell] == true) {
                return currentCell;
            }
        }
    }

    return 0;
}

std::vector<float> getEdgeVertices(const VoronoiBitmask &bitmask) {
    int startingCell = getStartingCell(bitmask);

}
