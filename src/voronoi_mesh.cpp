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
    std::vector<float> edgeVertices;
    // TODO: Reserve some amount for edgeVertices

    int startingCell = getStartingCell(bitmask);
    int startingX = startingCell % bitmask.width;
    int startingY = startingCell / bitmask.width;
    std::vector<float> startingCoord = {static_cast<float>(startingX), static_cast<float>(startingY), 0.0f};
    edgeVertices.insert(edgeVertices.end(), startingCoord.begin(), startingCoord.end());

    int currentCell = startingCell;
    Direction currentDirection = NORTH;
    Direction edgeDirection = NORTH;
    int nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);

    std::cout << "(" << startingX << ", " << startingY << ")\n";
    // While the cell's edges haven't been fully traversed
    while (nextCell != startingCell) {
        // If the next cell we look at is empty, check the next clockwise cell
        if (!bitmask.mask[nextCell]) {
            if (currentDirection != NORTHEAST) {
                currentDirection = static_cast<Direction>(currentDirection + 1);
                nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);
            }
            else {
                currentDirection = EAST;
                nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);
            }
        }
        // If the next cell we look at is *not* empty, move there and update
        // positions and directions accordingly
        else {
            currentCell = nextCell;
            std::cout << "(" << currentCell % bitmask.width << ", " << currentCell / bitmask.width << ")\n";
            if (!(currentDirection == EAST || currentDirection == SOUTHEAST)) {
                edgeDirection = static_cast<Direction>(currentDirection - 2);
            }
            else if (currentDirection == EAST) {
                edgeDirection = NORTH;
            }
            else {
                edgeDirection = NORTHEAST;
            }

            currentDirection = edgeDirection;
            nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);
        }
    }

    return edgeVertices;
}

int moveAcrossBitmask(const VoronoiBitmask &bitmask, int currentCell, Direction direction) {
    if (direction == EAST) { return (currentCell + 1); }
    if (direction == SOUTHEAST) { return (currentCell + bitmask.width + 1); }
    if (direction == SOUTH) { return (currentCell + bitmask.width); }
    if (direction == SOUTHWEST) { return (currentCell + bitmask.width - 1); }
    if (direction == WEST) { return (currentCell - 1); }
    if (direction == NORTHWEST) { return (currentCell - bitmask.width - 1); }
    if (direction == NORTH) { return (currentCell - bitmask.width); }
    if (direction == NORTHEAST) { return (currentCell - bitmask.width + 1); }

    return -1;
}
