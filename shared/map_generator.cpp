#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <queue>
#include <utility>
#include <algorithm>
#include <stack>
#include <random>
#include "GameObject.h"

void connectCorners(std::mt19937 &rng, std::vector<std::vector<char>>& mapGrid, int width, int height);
void placeDenseClusters(std::mt19937 &rng, std::vector<std::vector<char>>& mapGrid, int width, int height, int maxClusters, int minClusterSize, int maxClusterSize);
void connectRandomPoints(std::mt19937 &rng, std::vector<std::vector<char>>& mapGrid, int x1, int y1, int x2, int y2, int corridorWidth);
void drawWaterBorder(std::vector<std::vector<char>>& mapGrid, int width, int height, int excludeAreaSize);
void cleanLonelyObjects(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize);
void fillHolesWithWalls(std::vector<std::vector<char>>& mapGrid, int width, int height);
void connectLargeHolesToCorners(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize);

std::vector<MapObject> generateMap(int width, int height, unsigned seed) {
    std::vector<MapObject> mapObjects;
    std::mt19937 rng(seed);
    std::vector<std::vector<char>> mapGrid(width, std::vector<char>(height, ' '));

    // TBD: need to discuss the parameters
    placeDenseClusters(rng, mapGrid, width, height, 300, 20, 200); 

    // draw random road
    for(int i=0 ;i<rng()%10+5; i++){
        int x1 = rng() % width;
        int y1 = rng() % height;
        int x2 = rng() % width;
        int y2 = rng() % height;
        connectRandomPoints(rng, mapGrid, x1, y1, x2, y2, 3);
    }

    connectCorners(rng, mapGrid, width, height);
    drawWaterBorder(mapGrid, width, height, 3);
    connectLargeHolesToCorners(mapGrid, width, height, 10); // >= 10
    cleanLonelyObjects(mapGrid, width, height, 6); // < 6
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

void connectCorners(std::mt19937 &rng, std::vector<std::vector<char>>& mapGrid, int width, int height) {
    std::vector<std::pair<int, int>> corners = {
        {2, 2}, {1, height - 2}, {width - 2, height - 2}, {width - 2, 1}};
    
    int centerX = (rng() % (width-20))+10;
    int centerY = (rng() % (height-8))+4;
    int corridorWidth = 2;
    float slopePreference = 0.4f;

    for (int x = centerX - corridorWidth / 2; x <= centerX + corridorWidth / 2; ++x) {
        for (int y = centerY - corridorWidth / 2; y <= centerY + corridorWidth / 2; ++y) {
            if (x >= 1 && x < width - 1 && y >= 1 && y < height - 1) {
                mapGrid[x][y] = ' ';
            }
        }
    }

    for (const auto& corner : corners) {
        int x = corner.first;
        int y = corner.second;
        int dx = (centerX > x) ? 1 : -1;
        int dy = (centerY > y) ? 1 : -1;

        while (x != centerX || y != centerY) {
            for (int offsetX = -corridorWidth / 2; offsetX <= corridorWidth / 2; ++offsetX) {
                for (int offsetY = -corridorWidth / 2; offsetY <= corridorWidth / 2; ++offsetY) {
                    int nx = x + offsetX;
                    int ny = y + offsetY;
                    if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1) {
                        mapGrid[nx][ny] = ' ';
                    }
                }
            }

            if (rng() % 100 < (int)(slopePreference * 50)) {
                if (x != centerX && y != centerY) {
                    x += dx;
                    y += dy;
                } else if (x != centerX) {
                    x += dx;
                } else if (y != centerY) {
                    y += dy;
                }
            } else {
                if (x != centerX) {
                    x += dx;
                } else if (y != centerY) {
                    y += dy;
                }
            }
        }
    }
}

void connectRandomPoints(std::mt19937 &rng, std::vector<std::vector<char>>& mapGrid, int x1, int y1, int x2, int y2, int corridorWidth) {
    int currentX = x1;
    int currentY = y1;

    for (int offsetX = -corridorWidth / 2; offsetX <= corridorWidth / 2; ++offsetX) {
        for (int offsetY = -corridorWidth / 2; offsetY <= corridorWidth / 2; ++offsetY) {
            int nx = currentX + offsetX;
            int ny = currentY + offsetY;
            if (nx >= 1 && nx < mapGrid.size() - 1 && ny >= 1 && ny < mapGrid[0].size() - 1) {
                mapGrid[nx][ny] = ' ';
            }
        }
    }

    while (currentX != x2 || currentY != y2) {
        bool moveHorizontally = (rng() % 2 == 0);
        if (moveHorizontally && currentX != x2) {
            currentX += (x2 > currentX) ? 1 : -1;
        } else if (!moveHorizontally && currentY != y2) {
            currentY += (y2 > currentY) ? 1 : -1;
        }

        // make it tortuous
        if (rng() % 4 == 0) {
            currentX += (rng() % 3 - 1);
            currentY += (rng() % 3 - 1);
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

void placeDenseClusters(std::mt19937 &rng, std::vector<std::vector<char>>& mapGrid, int width, int height, int maxClusters, int minClusterSize, int maxClusterSize) {
    maxClusters = maxClusters + rng() % maxClusters;
    
    for (int i = 0; i < maxClusters; ++i) {
        char clusterType = (rng() % 2 == 0) ? 'W' : 'A';

        int startX = rng() % width;
        int startY = rng() % height;

        if (mapGrid[startX][startY] != ' ') continue;

        std::vector<std::pair<int, int>> clusterTiles = {{startX, startY}};
        mapGrid[startX][startY] = clusterType;

        for (int j = 0; j < maxClusterSize; ++j) {
            auto [currentX, currentY] = clusterTiles[rng() % clusterTiles.size()];
            int newX = currentX + (rng() % 3 - 1);
            int newY = currentY + (rng() % 3 - 1);

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


// helper
void drawWaterBorder(std::vector<std::vector<char>>& mapGrid, int width, int height, int excludeAreaSize) {
    // Define the corner coordinates
    std::vector<std::pair<int, int>> corners = {
        {1, 1}, {1, height - 2}, {width - 2, height - 2}, {width - 2, 1}
    };
    auto isNearCorner = [&](int x, int y) {
        for (const auto& corner : corners) {
            if (std::abs(x - corner.first) < excludeAreaSize && std::abs(y - corner.second) < excludeAreaSize) {
                return true;
            }
        }
        return false;
    };
    for (int x = 1; x < width - 1; ++x) {
        // Top border
        if (!isNearCorner(x, 1)) {
            mapGrid[x][1] = 'A';
        }
        // Bottom border
        if (!isNearCorner(x, height - 2)) {
            mapGrid[x][height - 2] = 'A';
        }
    }
    for (int y = 1; y < height - 1; ++y) {
        // Left border
        if (!isNearCorner(1, y)) {
            mapGrid[1][y] = 'A';
        }
        // Right border
        if (!isNearCorner(width - 2, y)) {
            mapGrid[width - 2][y] = 'A';
        }
    }
}

void bfs(std::vector<std::vector<char>>& mapGrid, std::vector<std::vector<bool>>& visited, int startX, int startY, char objectType, std::vector<std::pair<int, int>>& cluster) {
    int width = mapGrid.size();
    int height = mapGrid[0].size();
    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});
    visited[startX][startY] = true;
    cluster.push_back({startX, startY});
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        // visit neighbors
        for (auto& dir : directions) {
            int nx = x + dir[0];
            int ny = y + dir[1];

            if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1 && !visited[nx][ny] && mapGrid[nx][ny] == objectType) {
                visited[nx][ny] = true;
                q.push({nx, ny});
                cluster.push_back({nx, ny});
            }
        }
    }
}

void cleanLonelyObjects(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize) {
    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
    for (int x = 1; x < width - 1; ++x) {
        for (int y = 1; y < height - 1; ++y) {
            char current = mapGrid[x][y];

            // empty or visited tiles
            if (current == ' ' || visited[x][y]) continue;

            // BFS to find the cluster
            std::vector<std::pair<int, int>> cluster;
            bfs(mapGrid, visited, x, y, current, cluster);
            if (cluster.size() < minClusterSize) {
                for (const auto& [cx, cy] : cluster) {
                    mapGrid[cx][cy] = ' ';
                }
            }
        }
    }
}

void fillHolesWithWalls(std::vector<std::vector<char>>& mapGrid, int width, int height) {
    auto isValidCell = [&](int x, int y) {
        return x >= 0 && x < height && y >= 0 && y < width;
    };

    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
    std::vector<std::pair<int, int>> road;

    bfs(mapGrid, visited, 1, 1, ' ', road);

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (!visited[x][y] && mapGrid[x][y] == ' ') {
                mapGrid[x][y] = 'W'; // Replace with wall
            }
        }
    }
}

void connectLargeHolesToCorners(std::vector<std::vector<char>>& mapGrid, int width, int height, int minClusterSize) {
    std::vector<std::pair<int, int>> corners = {
        {1, 1}, {1, height - 2}, {width - 2, height - 2}, {width - 2, 1}};
    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
    auto manhattanDistance = [](int x1, int y1, int x2, int y2) {
        return abs(x1 - x2) + abs(y1 - y2);
    };

    //  find holes
    for (int x = 1; x < width - 1; ++x) {
        for (int y = 1; y < height - 1; ++y) {
            // visited or non-empty
            if (visited[x][y] || mapGrid[x][y] != ' ') continue;

            // BFS to find the cluster
            std::vector<std::pair<int, int>> cluster;
            bfs(mapGrid, visited, x, y, ' ', cluster);
            if (cluster.size() >= minClusterSize) {
                // find the closest corner
                std::pair<int, int> nearestCorner = corners[0];
                int minDistance = manhattanDistance(cluster[0].first, cluster[0].second, nearestCorner.first, nearestCorner.second);
                for (const auto& corner : corners) {
                    int distance = manhattanDistance(cluster[0].first, cluster[0].second, corner.first, corner.second);
                    if (distance < minDistance) {
                        minDistance = distance;
                        nearestCorner = corner;
                    }
                }

                // connect to the nearest corner
                int x1 = cluster[0].first, y1 = cluster[0].second;
                int x2 = nearestCorner.first, y2 = nearestCorner.second;

                // create a Manhattan path
                while (x1 != x2 || y1 != y2) {
                    mapGrid[x1][y1] = ' ';
                    if (x1 < x2) x1++;
                    else if (x1 > x2) x1--;
                    if (y1 < y2) y1++;
                    else if (y1 > y2) y1--;
                }
            }
        }
    }
}