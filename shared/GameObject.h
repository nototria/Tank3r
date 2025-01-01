#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <ncurses.h>
#include <map>
#include "GameParameters.h"

// game timer
class GameTimer {
private:
    std::chrono::steady_clock::time_point lastTime;
    double accumulator;
    const double tickRate;

public:
    GameTimer(double tickRateInSeconds)
        : tickRate(tickRateInSeconds), accumulator(0) {
        lastTime = std::chrono::steady_clock::now();
    }

    bool shouldUpdate() {
        auto currentTime = std::chrono::steady_clock::now();
        double elapsedTime = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;
        accumulator += elapsedTime;
        if (accumulator >= tickRate) {
            accumulator -= tickRate;
            return true;
        }
        return false;
    }
};

// GameObject
class GameObject {
protected:
    int x;
    int y;
    MapObjectType type;

public:
    GameObject(int x = 0, int y = 0,MapObjectType type = MapObjectType::empty) : x(x), y(y), type(type) {}
    virtual ~GameObject() {}

    // function
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }

    int getX() const { return x; }
    int getY() const { return y; }

    void setPosition(int newX, int newY){
        x = newX;
        y = newY;
    }

    MapObjectType getType() const { return type; }

    virtual void render() const {
        std::cout << "Rendering GameObject at (" << x << ", " << y << ")" << std::endl;
    }
};

// MapObjects
class MapObject : public GameObject{
public:
    MapObject(int x, int y, MapObjectType type) : GameObject(x, y, type) {}

    int getX() const { return x; }
    int getY() const { return y; }
    MapObjectType getType() const { return type; }
    bool isBlocking() const {return type == MapObjectType::wall;}
    bool isWater() const {return type == MapObjectType::water;}
    bool isObstacle() const {return (type == MapObjectType::water || type == MapObjectType::wall);}
    std::wstring getSymbol() const {
        switch (type) {
            case MapObjectType::wall: return L"█"; // wall
            case MapObjectType::water: return L"▒"; // water
            default: return L" ";
        }
    }
};

// Bullet
class Bullet : public GameObject {
private:
    Direction direction;
    bool active;
    bool inWater;

public:
    Bullet(int x, int y, Direction dir) : GameObject(x, y, MapObjectType :: bullet), direction(dir), active(true),inWater(false){}

    static bool checkCollision(int x, int y, const std::vector<MapObject>& staticObjects) {
        for (const auto& obj : staticObjects) {
            if (obj.isBlocking() && obj.getX() == x && obj.getY() == y) {
                return true;
            }
        }
        return false;
    }

    static bool checkInWater(int x, int y, const std::vector<MapObject>& staticObjects) {
        for (const auto& obj : staticObjects) {
            if (obj.isWater() && obj.getX() == x && obj.getY() == y) {
                return true;
            }
        }
        return false;
    }
    
    bool getInWater() const {return inWater;}

    bool checkCollisionWithBullet(const Bullet& other) const {
        return active && other.isActive() && x == other.getX() && y == other.getY();
    }

    void move(int width, int height, const std::vector<MapObject>& staticObjects) {
        if (!active) return;

        int nextX = x;
        int nextY = y;

        switch (direction) {
            case Direction::Up:    nextY -= 1; break;
            case Direction::Down:  nextY += 1; break;
            case Direction::Left:  nextX -= 1; break;
            case Direction::Right: nextX += 1; break;
        }

        // Check for collisions or out-of-bounds
        if (nextX <= 0 || nextX >= width - 1 || 
            nextY <= 0 || nextY >= height - 1 || 
            checkCollision(nextX, nextY, staticObjects)) {
            active = false;
        } else {
            if(checkInWater(nextX, nextY, staticObjects)){
                inWater = true;
            }else{
                inWater = false;
            }
            x = nextX;
            y = nextY;
        }
    }

    bool isOutOfBounds(int width, int height) const {
        return x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1;
    }
    bool isActive() const { return active; }
    void setActive(bool isActive) { active = isActive; }
    wchar_t getSymbol() const {return L'*';}
};

// Tank
class Tank : public GameObject {
private:
    Direction direction;                                                            
    int color;
    int hp;
    int id;
    bool isAlive;
    std::string name;
    std::vector<Bullet> bullets;

public:
    wchar_t getDirectionSymbol() const {
        switch (direction) {
            case Direction::Right: return L'⊢';
            case Direction::Left:  return L'⊣';
            case Direction::Up:    return L'⊥';
            case Direction::Down:  return L'⊤';
            default: return L'?';
        }
    }

    // Constructor
    Tank() : GameObject(0, 0, MapObjectType::tank), direction(Direction::Up), color(COLOR_GREEN), hp(20), isAlive(false), id(-1) {}
    Tank(int x, int y, Direction dir = Direction::Up, int color = COLOR_GREEN, int hp = 20, int id = 0)
        : GameObject(x, y, MapObjectType::tank), direction(dir), color(color), hp(hp), isAlive(true), id(id) {}

    static std::map<int, Tank> createTank(int playerNum, const std::vector<int> tankIds, int gridWidth, int gridHeight) {
        std::map<int, Tank> tankMap;
        std::vector<std::pair<int, int>> corners = {
            {1, 1}, {gridWidth - 2, 1},{1, gridHeight - 2}, {gridWidth - 2, gridHeight - 2}};

        for (int i = 0; i < playerNum; ++i) {
            // Create a Tank for each clientId and add to the map
            tankMap[tankIds[i]] = Tank(corners[i].first, corners[i].second, Direction::Down, COLOR_GREEN, 20, tankIds[i]);
        }

        return tankMap;
    }

    // Tank own functions
    void setDirection(Direction dir) { direction = dir; }
    Direction getDirection() const { return direction; }

    int getHP() const { return hp; }
    void setHP(int newHP) {
        hp = newHP;
        if (hp <= 0) {
            hp = 0;
            isAlive = false;
        }
    }

    bool IsAlive() const { return isAlive; }
    void revive(int newHP = 20) {
        hp = newHP;
        isAlive = true; // Revive the tank
    }

    void setName(const std::string& newName) { name = newName; }
    std::string getName() const { return name; }

    void setId(const int& newId) { id = newId; }

    int getId() const { return id; }

    void setColor(const int& newColor) { color = newColor; }
    int getColor() { return color; }

    static bool checkTankCollision(int nextX, int nextY, const std::vector<MapObject>& staticObjects) {
        for (const auto& obj : staticObjects) {
            if (obj.isObstacle() && obj.getX() == nextX && obj.getY() == nextY) {
                return true;
            }
        }
        return false;
    }

    // Bullet control
    void fireBullet() { bullets.emplace_back(x, y, direction); }
    std::vector<Bullet>& getBullets() { return bullets; }
    void updateBullets(int width, int height, const std::vector<MapObject>& staticObjects) {
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->move(width, height, staticObjects);
            if (!it->isActive()) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }
};

// server side
std::vector<int> checkBulletTankCollisions(std::map<int,Tank>& tanksMap) {
    std::vector<int> getHitTankIds;
    for (auto& [id, tank] : tanksMap) {
        std::vector<Bullet>& bullets = tank.getBullets();
        for (auto it = bullets.begin(); it != bullets.end();) {
            bool collisionDetected = false;
            if (!it->isActive()) {
                it = bullets.erase(it);
                continue;
            }

            int x = it->getX();
            int y = it->getY();
            for (auto& [id2, tank2] : tanksMap) {
                if (id == id2 || !tank2.IsAlive()) continue;
                if (tank2.getX() == x && tank2.getY() == y) {
                    collisionDetected = true;
                    getHitTankIds.push_back(id2);
                    //tank2.setHP(tank2.getHP() - 1);
                    it = bullets.erase(it);
                    break;
                }
            }
            if (!collisionDetected) {
                ++it;
            }
        }
    }
    return getHitTankIds;
}

void cleanupInactiveBullets(std::map<int, Tank>& tanksMap) {
    for (auto& [id, tank] : tanksMap) {
        std::vector<Bullet>& bullets = tank.getBullets();
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                           [](const Bullet& b) { return !b.isActive(); }),
            bullets.end());
    }
}

void handleBulletCollisions(std::map<int, Tank>& tanksMap) {
    std::vector<Bullet*> activeBullets;

    // Collect all active bullets
    for (auto& [id, tank] : tanksMap) {
        for (Bullet& bullet : tank.getBullets()) {
            if (bullet.isActive()) {
                activeBullets.push_back(&bullet);
            }
        }
    }

    // Check collisions between bullets
    for (size_t i = 0; i < activeBullets.size(); ++i) {
        for (size_t j = i + 1; j < activeBullets.size(); ++j) {
            if (activeBullets[i]->getX() == activeBullets[j]->getX() &&
                activeBullets[i]->getY() == activeBullets[j]->getY()) {
                activeBullets[i]->setActive(false);
                activeBullets[j]->setActive(false);
            }
        }
    }

    // Clean up inactive bullets
    cleanupInactiveBullets(tanksMap);
}

#endif // GAMEOBJECT_H
