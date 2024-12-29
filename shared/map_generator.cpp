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
void fixupNotches(std::vector<std::vector<char>>& mapGrid, int width, int height, int corridorWidth);
void drawWaterBorder(std::vector<std::vector<char>>& mapGrid, int width, int height, int excludeAreaSize);
void cleanLonelyObjects(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize);

std::vector<MapObject> generateMap(int width, int height, unsigned seed) {
    std::vector<MapObject> mapObjects;
    std::srand(seed);

    // Initialize the map grid with empty spaces
    std::vector<std::vector<char>> mapGrid(width, std::vector<char>(height, ' '));

    // Place dense clusters of walls and water
    placeDenseClusters(mapGrid, width, height);

    // Ensure the four corners of the map are connected
    connectCorners(mapGrid, width, height);

    // draw random road
    for(int i=0 ;i<std::rand()%10+20; i++){
        int x1 = std::rand() % width;
        int y1 = std::rand() % height;
        int x2 = std::rand() % width;
        int y2 = std::rand() % height;
        connectRandomPoints(mapGrid, x1, y1, x2, y2, 3);
    }

    // Fix up notches in the map
    fixupNotches(mapGrid, width, height, 3);

    // draw water border
    drawWaterBorder(mapGrid, width, height, 3);

    // Fix isolated water and wall objects
    cleanLonelyObjects(mapGrid, width, height, 2);

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

void fixupNotches(std::vector<std::vector<char>>& mapGrid, int width, int height, int corridorWidth) {
    // Helper to check if a tile is a wall or border
    auto isWallOrBorder = [&](int x, int y) {
        return x < 1 || x >= width - 1 || y < 1 || y >= height - 1 || mapGrid[x][y] == 'W';
    };

    // Helper to detect if a space is a notch
    auto isNotch = [&](int x, int y) {
        if (mapGrid[x][y] != ' ') return false; // Only consider empty spaces

        // Count surrounding walls or borders
        int wallCount = 0;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue; // Skip the current tile
                if (isWallOrBorder(x + dx, y + dy)) {
                    ++wallCount;
                }
            }
        }
        return wallCount >= 6; // Consider it a notch if most of the surrounding tiles are walls or borders
    };

    // DFS to check depth of notch area
    auto dfs = [&](int startX, int startY, std::vector<std::vector<bool>>& visited) -> int {
        int depth = 0;
        std::stack<std::pair<int, int>> stack;
        stack.push({startX, startY});
        visited[startX][startY] = true;

        while (!stack.empty()) {
            auto [x, y] = stack.top();
            stack.pop();
            ++depth;

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1 && !visited[nx][ny] && mapGrid[nx][ny] == ' ') {
                        visited[nx][ny] = true;
                        stack.push({nx, ny});
                    }
                }
            }
        }
        return depth;
    };

    // Helper to fill the notch with nearby blocks
    auto fillNotch = [&](int x, int y) {
        std::map<char, int> nearbyCounts;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                char tile = mapGrid[x + dx][y + dy];
                if (tile != ' ') ++nearbyCounts[tile];
            }
        }

        // Choose the most common type
        char fillType = 'W'; // Default to wall
        int maxCount = 0;
        for (const auto& [type, count] : nearbyCounts) {
            if (count > maxCount) {
                maxCount = count;
                fillType = type;
            }
        }

        // Fill the notch (but not the borders)
        if (!isWallOrBorder(x, y)) {
            mapGrid[x][y] = fillType;
        }
    };

    // Helper to connect the notch to the road
    auto connectToRoad = [&](int x, int y) {
        // BFS to find nearest open area and connect to it
        std::queue<std::pair<int, int>> queue;
        std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
        queue.push({x, y});
        visited[x][y] = true;

        while (!queue.empty()) {
            auto [cx, cy] = queue.front();
            queue.pop();

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int nx = cx + dx;
                    int ny = cy + dy;

                    if (nx < 1 || nx >= width - 1 || ny < 1 || ny >= height - 1) continue; // Skip border tiles
                    if (visited[nx][ny]) continue;

                    if (mapGrid[nx][ny] == ' ') {
                        // Found an open area, carve a path
                        connectRandomPoints(mapGrid, x, y, nx, ny, corridorWidth);
                        return;
                    }

                    visited[nx][ny] = true;
                    queue.push({nx, ny});
                }
            }
        }
    };

    // Main loop to detect and fix notches
    for (int x = 1; x < width - 1; ++x) {
        for (int y = 1; y < height - 1; ++y) {
            if (isNotch(x, y)) {
                // DFS to calculate the depth of the notch area
                std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
                int depth = dfs(x, y, visited);

                // Small notch, fill it
                if (depth <= 3) {
                    fillNotch(x, y);
                } else if (depth <= 10){
                    // Larger notch, connect it to the road
                    connectToRoad(x, y);
                }
            }
        }
    }
}

void connectCorners(std::vector<std::vector<char>>& mapGrid, int width, int height) {
    // Define corner points
    std::vector<std::pair<int, int>> corners = {
        {1, 1}, {1, height - 2}, {width - 2, height - 2}, {width - 2, 1}};
    
    // Calculate the center point
    int centerX = width / 2;
    int centerY = height / 2;

    // Clear the center area to ensure all lines can intersect
    int corridorWidth = 2; // Minimum corridor width
    for (int x = centerX - corridorWidth / 2; x <= centerX + corridorWidth / 2; ++x) {
        for (int y = centerY - corridorWidth / 2; y <= centerY + corridorWidth / 2; ++y) {
            if (x >= 1 && x < width - 1 && y >= 1 && y < height - 1) {
                mapGrid[x][y] = ' '; // Ensure the center area is passable
            }
        }
    }

    // Draw wide slanted lines from each corner to the center
    for (const auto& corner : corners) {
        int x = corner.first;
        int y = corner.second;

        // Determine the direction to the center
        int dx = (centerX > x) ? 1 : -1;
        int dy = (centerY > y) ? 1 : -1;

        // Draw a wide diagonal corridor
        while (x != centerX || y != centerY) {
            // Create a corridor with a width of 'corridorWidth'
            for (int offsetX = -corridorWidth / 2; offsetX <= corridorWidth / 2; ++offsetX) {
                for (int offsetY = -corridorWidth / 2; offsetY <= corridorWidth / 2; ++offsetY) {
                    int nx = x + offsetX;
                    int ny = y + offsetY;
                    if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1) {
                        mapGrid[nx][ny] = ' '; // Carve the corridor
                    }
                }
            }

            // Move diagonally towards the center
            if (x != centerX) x += dx;
            if (y != centerY) y += dy;
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
    int minClusterSize = 5;                 
    int maxClusterSize = 30;                // Smaller cluster sizes

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