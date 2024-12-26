#include <vector>
#include <cstdlib>
#include <ctime>
#include "GameObject.h"
std::vector<MapObject> generateStaticObjects(int width, int height, int numWalls, int numWaters) {
    std::vector<MapObject> staticObjects;
    std::srand(std::time(0)); // Seed for randomness

    auto isPositionOccupied = [&](int x, int y) {
        for (const auto& obj : staticObjects) {
            if (obj.getX() == x && obj.getY() == y) return true;
        }
        return false;
    };

    // Generate walls
    for (int i = 0; i < numWalls; ++i) {
        int x, y;
        do {
            x = std::rand() % (width - 2) + 1;  // Avoid border
            y = std::rand() % (height - 2) + 1; // Avoid border
        } while (isPositionOccupied(x, y));   // Ensure no overlap

        staticObjects.emplace_back(x, y, MapObjectType::Wall);
    }

    // Generate water tiles
    for (int i = 0; i < numWaters; ++i) {
        int x, y;
        do {
            x = std::rand() % (width - 2) + 1;
            y = std::rand() % (height - 2) + 1;
        } while (isPositionOccupied(x, y));

        staticObjects.emplace_back(x, y, MapObjectType::Water);
    }

    return staticObjects;
}