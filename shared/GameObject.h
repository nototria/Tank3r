#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>
#include <chrono>
#include <unistd.h>
#include <ncurses.h>

#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 40
const int statusBlockWidth = 25;
const int statusBlockHeight = 10; 
enum class Direction {Right, Left, Up, Down};
enum class GameState {TitleScreen, UsernameInput, GameLoop, EndScreen};
enum class MapObjectType {Wall,Water};

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

// MapObjects
class MapObject {
private:
    int x;
    int y;
    MapObjectType type;

public:
    MapObject(int x, int y, MapObjectType type) : x(x), y(y), type(type) {}

    int getX() const { return x; }
    int getY() const { return y; }
    MapObjectType getType() const { return type; }
    bool isBlocking() const {return type == MapObjectType::Wall;}
    bool isObstacle() const {return (type == MapObjectType::Water || type == MapObjectType::Wall);}
    char getSymbol() const {
        switch (type) {
            case MapObjectType::Wall: return '#'; // wall
            case MapObjectType::Water: return '~'; // water
            default: return ' ';
        }
    }
};

// GameObject
class GameObject {
protected:
    int x;
    int y;

public:
    GameObject(int x = 0, int y = 0) : x(x), y(y) {}
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
    virtual void render() const {
        std::cout << "Rendering GameObject at (" << x << ", " << y << ")" << std::endl;
    }
};

// Bullet
class Bullet : public GameObject {
private:
    Direction direction;
    bool active;

public:
    Bullet(int x, int y, Direction dir) : GameObject(x, y), direction(dir), active(true){}

    static bool checkCollision(int x, int y, const std::vector<MapObject>& staticObjects) {
        for (const auto& obj : staticObjects) {
            if (obj.isBlocking() && obj.getX() == x && obj.getY() == y) {
                return true;
            }
        }
        return false;
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
            x = nextX;
            y = nextY;
        }
    }

    bool isOutOfBounds(int width, int height) const {
        return x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1;
    }
    bool isActive() const { return active; }
    wchar_t getSymbol() const {return L'*';}
};

// Tank
class Tank : public GameObject {
private:
    Direction direction;
    int color;
    int hp;
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
    // Tank own element
    Tank(int x, int y, Direction dir = Direction::Up, int color = COLOR_BLUE, int hp = 20, const std::string& name = "Player")
        : GameObject(x, y), direction(dir), color(color), hp(hp), name(name) {}
    static std::vector<Tank> createTanks(int playerNum, const std::string remotePlayerNames[], int gridWidth, int gridHeight) {
        std::vector<Tank> tanks;
        int startX = gridWidth / 2;
        int startY = gridHeight / 2;

        for (int i = 0; i < playerNum; ++i) {
            // Use player name from remotePlayerNames and create a tank for each player
            tanks.emplace_back(startX + (i * 2), startY + (i * 2), Direction::Right, COLOR_BLUE, 20, remotePlayerNames[i]);
        }

        return tanks;
    }

    void setDirection(Direction dir) {direction = dir;}
    Direction getDirection() const { return direction; }

    int getHP() const { return hp; }
    void setHP(int newHP) { hp = newHP; }

    void setName(const std::string& newName) {name = newName;}
    std::string getName() const {return name;}

    void setColor(const int& newColor) {color = newColor;}
    int getColor() {return color;}

    static bool checkTankCollision(int nextX, int nextY, const std::vector<MapObject>& staticObjects) {
        for (const auto& obj : staticObjects) {
            if (obj.isObstacle() && obj.getX() == nextX && obj.getY() == nextY) {return true;}
        }
        return false;
    }

    // bullet control
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

#endif // GAMEOBJECT_H