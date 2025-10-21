#include <iostream>

#include <madoc/voronoi_mesh.h>


int getStartingCell(const VoronoiBitmask &bitmask) {
    for (int y = 0; y < bitmask.height; y++) {
        for (int x = 0; x < bitmask.width; x++) {
            int currentCell = (y * bitmask.width) + x;
            if (bitmask.mask[currentCell] == true) {
                // TODO: Figure out if we actually need something like this:
                /*// Check to make sure the starting cell has more than just
                // one neighboring filled cell
                int numAdjacentFilled = 0;
                Direction checkedDirection = EAST;
                for (int i = 0; i < 8; i++) {
                    int checkedCell = moveAcrossBitmask(bitmask, currentCell, checkedDirection);
                    if (bitmask.mask[checkedCell]) {
                        numAdjacentFilled++;
                    }
                    checkedDirection = static_cast<Direction>(checkedDirection + 1);
                }

                if (numAdjacentFilled > 1) {
                    return currentCell;
                }*/
                return currentCell;
            }
        }
    }

    return 0;
}

std::vector<float> getEdgeVertices(const VoronoiBitmask &bitmask) {
    std::vector<float> edgeVertices;
    // TODO: Reserve some amount for edgeVertices

    const int startingCell = getStartingCell(bitmask);
    const int startingX = startingCell % bitmask.width;
    const int startingY = startingCell / bitmask.width;
    std::vector<float> startingCoord = {static_cast<float>(startingX), static_cast<float>(startingY), 0.0f};
    edgeVertices.insert(edgeVertices.end(), startingCoord.begin(), startingCoord.end());

    int currentCell = startingCell;
    Direction currentDirection = NORTH;
    Direction edgeDirection = NORTH;
    int nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);

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
            Direction newEdgeDirection;
            if (!(currentDirection == EAST || currentDirection == SOUTHEAST)) {
                newEdgeDirection = static_cast<Direction>(currentDirection - 2);
            }
            else if (currentDirection == EAST) {
                newEdgeDirection = NORTH;
            }
            else {
                newEdgeDirection = NORTHEAST;
            }

            // If we're changing what direction we're moving in, add the coords
            // of the current cell to out list of vertices
            if (newEdgeDirection != edgeDirection && currentCell != startingCell) {
                int currentX = currentCell % bitmask.width;
                int currentY = currentCell / bitmask.width;
                std::vector<float> currentVertex = {static_cast<float>(currentX),
                    static_cast<float>(currentY), 0.0f};
                edgeVertices.insert(edgeVertices.end(), currentVertex.begin(), currentVertex.end());
            }
            currentCell = nextCell;

            edgeDirection = newEdgeDirection;
            currentDirection = edgeDirection;
            nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);
        }
    }

    // Add the final vertex coordinate
    int finalX = currentCell % bitmask.width;
    int finalY = currentCell / bitmask.width;
    std::vector<float> finalVertex = {static_cast<float>(finalX),
        static_cast<float>(finalY), 0.0f};
    edgeVertices.insert(edgeVertices.end(), finalVertex.begin(), finalVertex.end());

    /*for (int i = 0; i < edgeVertices.size(); i += 3) {
        std::cout << "(" << edgeVertices[i] << "," << edgeVertices[i + 1] << "," << edgeVertices[i + 2] << ")\n";
    }*/

    return edgeVertices;
}

std::vector<float> getCenterVertex(std::vector<float>& vertices) {
    float centerX = 0.0f;
    float centerY = 0.0f;
    int numVertices = vertices.size() / 3;
    for (int i = 0; i < vertices.size(); i += 3) {
        centerX += vertices[i];
        centerY += vertices[i + 1];
    }
    centerX /= static_cast<float>(numVertices);
    centerY /= static_cast<float>(numVertices);
    std::vector<float> completeVertices = {centerX, centerY, 0.0f};
    completeVertices.insert(completeVertices.end(), vertices.begin(), vertices.end());

    return completeVertices;
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
