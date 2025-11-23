#include <iostream>
#include <cmath>

#include <madoc/voronoi_mesh.h>


int getStartingCell(const VoronoiBitmask &bitmask) {
    for (int y = 1; y < bitmask.height; y++) {
        for (int x = 1; x < bitmask.width; x++) {
            int currentCell = (y * bitmask.width) + x;

            // If the starting cell has at least one neighbor, it's valid
            if (bitmask.mask[currentCell]) {
                Direction checkedDirection = EAST;
                for (int i = 0; i < 8; i++) {
                    int checkedCell = moveAcrossBitmask(bitmask, currentCell, checkedDirection);
                    if (bitmask.mask[checkedCell]) {
                        return currentCell;
                    }
                    checkedDirection = static_cast<Direction>(checkedDirection + 1);
                }
            }
        }
    }

    return -1;
}

bool isValidCell(int cell, int totalCells) {
    return (cell >= 0 && cell < totalCells);
}


std::vector<float> getEdgeVertices(const VoronoiBitmask &bitmask) {
    std::vector<float> edgeVertices;
    // TODO: Reserve some amount for edgeVertices

    const int startingCell = getStartingCell(bitmask);
    // Check if no valid cell is found
    if (startingCell < 0) {
        return edgeVertices;
    }

    const int totalCells = bitmask.width * bitmask.height;
    const int startingX = startingCell % bitmask.width;
    const int startingY = startingCell / bitmask.width;
    std::vector<float> startingCoord = {static_cast<float>(startingX - 1) +
        static_cast<float>(bitmask.xOffset) + 0.5f,
        (static_cast<float>(startingY - 1) + static_cast<float>(bitmask.yOffset) + 0.5f) * -1, 0.0f};
    edgeVertices.insert(edgeVertices.end(), startingCoord.begin(), startingCoord.end());

    int currentCell = startingCell;
    Direction currentDirection = NORTH;
    Direction edgeDirection = NORTH;
    int nextCell = moveAcrossBitmask(bitmask, currentCell, currentDirection);

    // While the cell's edges haven't been fully traversed
    while (isValidCell(currentCell, totalCells) &&
        isValidCell(nextCell, totalCells) && nextCell != startingCell) {
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
                std::vector<float> currentVertex = {static_cast<float>(currentX - 1) +
                    static_cast<float>(bitmask.xOffset) + 0.5f,
                    (static_cast<float>(currentY - 1) + static_cast<float>(bitmask.yOffset)
                    + 0.5f) * -1, 0.0f};
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
    std::vector<float> finalVertex = {static_cast<float>(finalX - 1) +
        static_cast<float>(bitmask.xOffset) + 0.5f,
        (static_cast<float>(finalY - 1) + static_cast<float>(bitmask.yOffset) + 0.5f) * -1, 0.0f};
    edgeVertices.insert(edgeVertices.end(), finalVertex.begin(), finalVertex.end());

    return edgeVertices;
}

std::vector<float> getCenterVertex(const VoronoiBitmask &bitmask, std::vector<float>& vertices) {
    float centerX = 0.0f;
    float centerY = 0.0f;
    int numVertices = vertices.size() / 3;
    for (int i = 0; i < vertices.size(); i += 3) {
        centerX += vertices[i];
        centerY += vertices[i + 1];
    }
    centerX /= static_cast<float>(numVertices);
    centerY /= static_cast<float>(numVertices);
    centerX += 0.5f;
    centerY += 0.5f;
    std::vector<float> completeVertices = {centerX, centerY, 0.0f};

    completeVertices.insert(completeVertices.end(), vertices.begin(), vertices.end());

    return completeVertices;
}

std::vector<unsigned int> getEarClippedIndices(const std::vector<float>& inputVertices) {
    // Make the list of vertices (not just the raw floats that OpenGL uses)
    std::vector<float> rawVertices = inputVertices;
    std::vector<glm::vec2> vertices;
    vertices.resize(rawVertices.size() / 3);
    for (int i = 0; i < rawVertices.size(); i += 3) {
        vertices[i / 3] = glm::vec2(rawVertices[i], rawVertices[i + 1]);
    }

    // Make the list of indices, where each index corresponds to a vertex
    std::vector<unsigned int> indices;
    indices.resize(vertices.size());
    for (int i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }
    // Also the final, empty list of indices that will form triangles
    std::vector<unsigned> triangles;

    // Create the list of booleans that represent convexity of the corresponding vertex
    std::vector<bool> isConvex;

    // Ear clip!
    while (indices.size() != 3) {
        isConvex.resize(indices.size());

        // Get the winding order of the polygon this iteration
        float signedArea = 0.0f;
        for (int i = 0; i < indices.size(); i++) {
            // Get proper wrapping indices
            int next = (i + 1) % indices.size();

            // Compute part of the signed area
            signedArea += (vertices[indices[i]].x * vertices[indices[next]].y) -
                (vertices[indices[next]].x * vertices[indices[i]].y);
        }

        // Get whether each vertex is convex or not
        for (int i = 0; i < indices.size(); i++) {
            // Get proper wrapping indices
            int prev = (i - 1 + indices.size()) % indices.size();
            int next = (i + 1) % indices.size();

            // Get the vectors formed by the 3 vertices
            glm::vec3 vectorA = {
                vertices[indices[i]].x - vertices[indices[prev]].x,
                vertices[indices[i]].y - vertices[indices[prev]].y,
                0.0f
            };
            glm::vec3 vectorB = {
                vertices[indices[next]].x - vertices[indices[i]].x,
                vertices[indices[next]].y - vertices[indices[i]].y,
                0.0f
            };

            // Check if the vertex is convex
            if (signedArea < 0 && glm::cross(vectorA, vectorB).z < 0) {
                isConvex[i] = true;
            }
            else if (signedArea > 0 && glm::cross(vectorA, vectorB).z > 0) {
                isConvex[i] = true;
            }
            else {
                isConvex[i] = false;
            }
        }

        // Clip the ear if its convex
        for (int i = 0; i < indices.size(); i++) {
            int prev = (i - 1 + indices.size()) % indices.size();
            int next = (i + 1) % indices.size();

            if (isConvex[i]) {
                triangles.push_back(indices[prev]);
                triangles.push_back(indices[i]);
                triangles.push_back(indices[next]);

                indices.erase(indices.begin() + i);
                break;
            }
        }

        bool anyConvex = false;
        for (int i = 0; i < isConvex.size(); i++) {
            if (isConvex[i]) {
                anyConvex = true;
            }
        }

        // If the remaining points won't work for ear-clipping, just manually
        // connect them into triangles
        if (!anyConvex) {
            for (int i = 0; i < indices.size() - 2; i++) {
                triangles.push_back(indices[i]);
                triangles.push_back(indices[i + 1]);
                triangles.push_back(indices[i + 2]);
            }

            return triangles;
        }
    }

    // Add the final triangle
    triangles.push_back(indices[0]);
    triangles.push_back(indices[1]);
    triangles.push_back(indices[2]);
    return triangles;
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

float Cross(const glm::vec2& p, const glm::vec2& v, const glm::vec2& n)
{
    return (v.x - p.x) * (n.y - v.y) - (v.y - p.y) * (n.x - v.x);
}

bool InTriangle(const glm::vec2& pt, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
    float c1 = Cross(a, b, pt);
    float c2 = Cross(b, c, pt);
    float c3 = Cross(c, a, pt);

    bool has_neg = (c1 < 0.f) || (c2 < 0.f) || (c3 < 0.f);
    bool has_pos = (c1 > 0.f) || (c2 > 0.f) || (c3 > 0.f);

    return !(has_neg && has_pos);
}