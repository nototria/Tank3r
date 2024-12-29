#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <queue>
#include <utility>
#include <algorithm>
#include <stack>
#include "GameObject.h"

void connectCorners(std::vector<std::vector<char>>& mapGrid, int width, int height);
void placeDenseClusters(std::vector<std::vector<char>>& mapGrid, int width, int height);
void connectRandomPoints(std::vector<std::vector<char>>& mapGrid, int x1, int y1, int x2, int y2, int corridorWidth);
void drawWaterBorder(std::vector<std::vector<char>>& mapGrid, int width, int height, int excludeAreaSize);
void cleanLonelyObjects(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize);
void fillHolesWithWalls(std::vector<std::vector<char>>& mapGrid, int width, int height);

std::vector<MapObject> generateMap(int width, int height, unsigned seed) {
    std::vector<MapObject> mapObjects;
    std::srand(seed);
    std::vector<std::vector<char>> mapGrid(width, std::vector<char>(height, ' '));

    placeDenseClusters(mapGrid, width, height);
    
    // draw random road
    for(int i=0 ;i<std::rand()%10+5; i++){
        int x1 = std::rand() % width;
        int y1 = std::rand() % height;
        int x2 = std::rand() % width;
        int y2 = std::rand() % height;
        connectRandomPoints(mapGrid, x1, y1, x2, y2, 3);
    }

    connectCorners(mapGrid, width, height);
    drawWaterBorder(mapGrid, width, height, 3);
    cleanLonelyObjects(mapGrid, width, height, 3);
    fillHolesWithWalls(mapGrid, width, height);

    // Convert mapGrid to mapObjects
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (mapGrid[x][y] == 'W') {
                mapObjects.emplace_back(x, y, MapObjectType::wall);
            } else if (mapGrid[x][y] == 'A') {
                mapObjects.emplace_back(x, y, MapObjectType::water);
            }
        }
    }

    return mapObjects;
}

void connectCorners(std::vector<std::vector<char>>& mapGrid, int width, int height) {
    // Define corner points
    std::vector<std::pair<int, int>> corners = {
        {1, 1}, {1, height - 2}, {width - 2, height - 2}, {width - 2, 1}};
    
    // Calculate the center point
    int centerX = std::rand() % (width-1);
    int centerY = std::rand() % (height-1);
    int corridorWidth = 2;

    // Clear the exact center area to ensure all lines intersect here
    for (int x = centerX - corridorWidth / 2; x <= centerX + corridorWidth / 2; ++x) {
        for (int y = centerY - corridorWidth / 2; y <= centerY + corridorWidth / 2; ++y) {
            if (x >= 1 && x < width - 1 && y >= 1 && y < height - 1) {
                mapGrid[x][y] = ' ';
            }
        }
    }

    // Internal slope control (cannot be changed outside the function)
    float slopePreference = 1.0f; // Controls the slope behavior

    // Draw slanted corridors from each corner to the center
    for (const auto& corner : corners) {
        int x = corner.first;
        int y = corner.second;

        // Determine the direction to the center
        int dx = (centerX > x) ? 1 : -1;
        int dy = (centerY > y) ? 1 : -1;

        // Draw a corridor from the corner to the exact center
        while (x != centerX || y != centerY) {
            // Create a wide corridor using the specified width
            for (int offsetX = -corridorWidth / 2; offsetX <= corridorWidth / 2; ++offsetX) {
                for (int offsetY = -corridorWidth / 2; offsetY <= corridorWidth / 2; ++offsetY) {
                    int nx = x + offsetX;
                    int ny = y + offsetY;
                    if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1) {
                        mapGrid[nx][ny] = ' '; // Carve the corridor
                    }
                }
            }

            // Adjust slope dynamically within the function
            if (std::rand() % 100 < (int)(slopePreference * 50)) {
                // Prefer diagonal movement
                if (x != centerX && y != centerY) {
                    x += dx;
                    y += dy;
                } else if (x != centerX) {
                    x += dx; // Horizontal fallback
                } else if (y != centerY) {
                    y += dy; // Vertical fallback
                }
            } else {
                // Prefer horizontal or vertical movement
                if (x != centerX) {
                    x += dx; // Horizontal movement
                } else if (y != centerY) {
                    y += dy; // Vertical movement
                }
            }
        }
    }
}

void connectRandomPoints(std::vector<std::vector<char>>& mapGrid, int x1, int y1, int x2, int y2, int corridorWidth) {
    int currentX = x1;
    int currentY = y1;

    // Carve the starting point
    for (int offsetX = -corridorWidth / 2; offsetX <= corridorWidth / 2; ++offsetX) {
        for (int offsetY = -corridorWidth / 2; offsetY <= corridorWidth / 2; ++offsetY) {
            int nx = currentX + offsetX;
            int ny = currentY + offsetY;
            if (nx >= 1 && nx < mapGrid.size() - 1 && ny >= 1 && ny < mapGrid[0].size() - 1) {
                mapGrid[nx][ny] = ' ';
            }
        }
    }

    // Randomly carve a tortuous path to the destination
    while (currentX != x2 || currentY != y2) {
        // Randomly decide whether to move horizontally or vertically
        bool moveHorizontally = (std::rand() % 2 == 0);

        if (moveHorizontally && currentX != x2) {
            // Move horizontally towards the target
            currentX += (x2 > currentX) ? 1 : -1;
        } else if (!moveHorizontally && currentY != y2) {
            // Move vertically towards the target
            currentY += (y2 > currentY) ? 1 : -1;
        }

        // Occasionally deviate from the direct path to make it tortuous
        if (std::rand() % 4 == 0) {
            currentX += (std::rand() % 3 - 1); // Move -1, 0, or 1 randomly
            currentY += (std::rand() % 3 - 1);
        }

        // Carve the current position as part of the corridor
        for (int offsetX = -corridorWidth / 2; offsetX <= corridorWidth / 2; ++offsetX) {
            for (int offsetY = -corridorWidth / 2; offsetY <= corridorWidth / 2; ++offsetY) {
                int nx = currentX + offsetX;
                int ny = currentY + offsetY;
                if (nx >= 1 && nx < mapGrid.size() - 1 && ny >= 1 && ny < mapGrid[0].size() - 1) {
                    mapGrid[nx][ny] = ' ';
                }
            }
        }
    }
}

void placeDenseClusters(std::vector<std::vector<char>>& mapGrid, int width, int height) {
    int maxClusters = 400 + std::rand() % 200;  // Reduced cluster count
    int minClusterSize = 1;                 
    int maxClusterSize = 100;                // Smaller cluster sizes

    for (int i = 0; i < maxClusters; ++i) {
        char clusterType = (std::rand() % 2 == 0) ? 'W' : 'A';

        int startX = std::rand() % width;
        int startY = std::rand() % height;

        if (mapGrid[startX][startY] != ' ') continue;

        std::vector<std::pair<int, int>> clusterTiles = {{startX, startY}};
        mapGrid[startX][startY] = clusterType;

        for (int j = 0; j < maxClusterSize; ++j) {
            auto [currentX, currentY] = clusterTiles[std::rand() % clusterTiles.size()];
            int newX = currentX + (std::rand() % 3 - 1);
            int newY = currentY + (std::rand() % 3 - 1);

            if (newX > 1 && newX < width - 2 && newY > 1 && newY < height - 2 &&
                mapGrid[newX][newY] == ' ') {
                mapGrid[newX][newY] = clusterType;
                clusterTiles.push_back({newX, newY});
            }
        }

        if (clusterTiles.size() < minClusterSize) {
            for (const auto& [x, y] : clusterTiles) {
                mapGrid[x][y] = ' ';
            }
        }
    }
}

void drawWaterBorder(std::vector<std::vector<char>>& mapGrid, int width, int height, int excludeAreaSize) {
    // Define the corner coordinates
    std::vector<std::pair<int, int>> corners = {
        {1, 1}, {1, height - 2}, {width - 2, height - 2}, {width - 2, 1}
    };

    // Helper function to check if a tile is near any of the four corners
    auto isNearCorner = [&](int x, int y) {
        for (const auto& corner : corners) {
            if (std::abs(x - corner.first) < excludeAreaSize && std::abs(y - corner.second) < excludeAreaSize) {
                return true;
            }
        }
        return false;
    };

    // Draw the water border
    for (int x = 1; x < width - 1; ++x) {
        // Top border (excluding the area near the corners)
        if (!isNearCorner(x, 1)) {
            mapGrid[x][1] = 'A'; // Water
        }

        // Bottom border (excluding the area near the corners)
        if (!isNearCorner(x, height - 2)) {
            mapGrid[x][height - 2] = 'A'; // Water
        }
    }

    for (int y = 1; y < height - 1; ++y) {
        // Left border (excluding the area near the corners)
        if (!isNearCorner(1, y)) {
            mapGrid[1][y] = 'A'; // Water
        }

        // Right border (excluding the area near the corners)
        if (!isNearCorner(width - 2, y)) {
            mapGrid[width - 2][y] = 'A'; // Water
        }
    }
}

void bfs(std::vector<std::vector<char>>& mapGrid, std::vector<std::vector<bool>>& visited, int startX, int startY, char objectType, std::vector<std::pair<int, int>>& cluster) {
    int width = mapGrid.size();
    int height = mapGrid[0].size();
    
    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});
    visited[startX][startY] = true; // Mark as visited
    cluster.push_back({startX, startY});

    // Directions for up, down, left, and right movement
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        // Explore the neighbors (up, down, left, right)
        for (auto& dir : directions) {
            int nx = x + dir[0];
            int ny = y + dir[1];

            // Ensure within bounds and matching the object type, and not visited yet
            if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1 && !visited[nx][ny] && mapGrid[nx][ny] == objectType) {
                visited[nx][ny] = true; // Mark as visited
                q.push({nx, ny});
                cluster.push_back({nx, ny});
            }
        }
    }
}

void cleanLonelyObjects(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize) {
    // Create a visited map to track visited tiles
    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));

    // Loop through the map and apply BFS to detect clusters
    for (int x = 1; x < width - 1; ++x) {
        for (int y = 1; y < height - 1; ++y) {
            char current = mapGrid[x][y];

            // Skip empty spaces or already visited tiles
            if (current == ' ' || visited[x][y]) continue;

            // Use BFS to find and count the size of the cluster
            std::vector<std::pair<int, int>> cluster;
            bfs(mapGrid, visited, x, y, current, cluster);

            // If the cluster size is smaller than the minimum size, clean up the cluster
            if (cluster.size() < minClusterSize) {
                for (const auto& [cx, cy] : cluster) {
                    mapGrid[cx][cy] = ' '; // Clean up lonely cluster
                }
            }
        }
    }
}

void fillHolesWithWalls(std::vector<std::vector<char>>& mapGrid, int width, int height) {
    // A helper function to check if a cell is within bounds
    auto isValidCell = [&](int x, int y) {
        return x >= 0 && x < height && y >= 0 && y < width;
    };

    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
    std::vector<std::pair<int, int>> road;

    bfs(mapGrid, visited, 1, 1, ' ', road);

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (!visited[x][y] && mapGrid[x][y] == ' ') {
                mapGrid[x][y] = 'A'; // Replace with wall
            }
        }
    }
}