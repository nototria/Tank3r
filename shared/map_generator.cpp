#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "GameObject.h"

struct Room {
    int x, y, width, height;
};

std::vector<Room> performBSP(int width, int height, int minRoomSize, int maxSplits);
void connectRooms(const std::vector<Room>& rooms, std::vector<std::vector<char>>& mapGrid);
void connectCorners(std::vector<std::vector<char>>& mapGrid, int width, int height);
void placeDenseClusters(std::vector<std::vector<char>>& mapGrid, int width, int height);

std::vector<MapObject> generateMap(int width, int height, unsigned seed) {
    std::vector<MapObject> mapObjects;
    std::srand(seed); // Seed for reproducibility

    // Initialize the map grid with empty spaces
    std::vector<std::vector<char>> mapGrid(width, std::vector<char>(height, ' '));

    // Perform BSP to divide the map into rooms
    int minRoomSize = 5;
    int maxSplits = 5;
    std::vector<Room> rooms = performBSP(width, height, minRoomSize, maxSplits);


    // Connect rooms with corridors
    connectRooms(rooms, mapGrid);

    // Place dense clusters of walls and water
    placeDenseClusters(mapGrid, width, height);

    // Ensure the four corners of the map are connected
    connectCorners(mapGrid, width, height);

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

std::vector<Room> performBSP(int width, int height, int minRoomSize, int maxSplits) {
    std::vector<Room> rooms;
    std::vector<Room> splits = {{1, 1, width - 2, height - 2}}; // Avoid borders

    for (int i = 0; i < maxSplits; ++i) {
        std::vector<Room> newSplits;
        for (const auto& room : splits) {
            bool splitHorizontally = std::rand() % 2;
            if (splitHorizontally && room.height > 2 * minRoomSize) {
                int split = std::rand() % (room.height - 2 * minRoomSize) + minRoomSize;
                newSplits.push_back({room.x, room.y, room.width, split});
                newSplits.push_back({room.x, room.y + split, room.width, room.height - split});
            } else if (!splitHorizontally && room.width > 2 * minRoomSize) {
                int split = std::rand() % (room.width - 2 * minRoomSize) + minRoomSize;
                newSplits.push_back({room.x, room.y, split, room.height});
                newSplits.push_back({room.x + split, room.y, room.width - split, room.height});
            } else {
                newSplits.push_back(room);
            }
        }
        splits = newSplits;
    }

    for (const auto& split : splits) {
        int roomWidth = std::max(minRoomSize, std::rand() % (split.width - 3) + 3);
        int roomHeight = std::max(minRoomSize, std::rand() % (split.height - 3) + 3);
        int roomX = split.x + std::rand() % (split.width - roomWidth + 1);
        int roomY = split.y + std::rand() % (split.height - roomHeight + 1);
        rooms.push_back({roomX, roomY, roomWidth, roomHeight});
    }

    return rooms;
}

void connectRooms(const std::vector<Room>& rooms, std::vector<std::vector<char>>& mapGrid) {
    for (size_t i = 1; i < rooms.size(); ++i) {
        Room a = rooms[i - 1];
        Room b = rooms[i];

        int ax = a.x + a.width / 2;
        int ay = a.y + a.height / 2;
        int bx = b.x + b.width / 2;
        int by = b.y + b.height / 2;

        if (std::rand() % 2 == 0) {
            for (int x = std::min(ax, bx); x <= std::max(ax, bx); ++x) {
                mapGrid[x][ay] = ' ';
            }
            for (int y = std::min(ay, by); y <= std::max(ay, by); ++y) {
                mapGrid[bx][y] = ' ';
            }
        } else {
            for (int y = std::min(ay, by); y <= std::max(ay, by); ++y) {
                mapGrid[ax][y] = ' ';
            }
            for (int x = std::min(ax, bx); x <= std::max(ax, bx); ++x) {
                mapGrid[x][by] = ' ';
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
    int corridorWidth = 3; // Minimum corridor width
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

void placeDenseClusters(std::vector<std::vector<char>>& mapGrid, int width, int height) {
    int maxClusters = 100 + std::rand() % 50;  // Reduced cluster count
    int minClusterSize = 10;                 
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