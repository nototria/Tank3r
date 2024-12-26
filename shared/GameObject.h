#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define BLUE_COLOR "\033[34m"
#define YELLOW_COLOR "\033[33m"

enum class Direction {Right, Left, Up, Down};
enum class GameState {TitleScreen, UsernameInput, GameLoop, EndScreen};
enum class MapObjectType {Wall,Water};

// game timer
class GameTimer {
private:
    std::chrono::steady_clock::time_point lastTime;
    double accumulator; // Accumulated time for fixed updates
    const double tickRate; // Fixed update rate (in seconds)

public:
    GameTimer(double tickRateInSeconds)
        : tickRate(tickRateInSeconds), accumulator(0) {
        lastTime = std::chrono::steady_clock::now();
    }

    // Calculate elapsed time and update the accumulator
    bool shouldUpdate() {
        auto currentTime = std::chrono::steady_clock::now();
        double elapsedTime = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        accumulator += elapsedTime;
        if (accumulator >= tickRate) {
            accumulator -= tickRate;
            return true; // Perform an update
        }
        return false; // Not enough time has passed for an update
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

    char getSymbol() const {
        switch (type) {
            case MapObjectType::Wall: return '#'; // Wall symbol
            case MapObjectType::Water: return '~'; // Water symbol
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

    void move(int width, int height, const std::vector<MapObject>& staticObjects) {
        if (!active) return;

        int nextX = x;
        int nextY = y;

        // Determine next position
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
            // Update position
            x = nextX;
            y = nextY;
        }
    }

    bool isOutOfBounds(int width, int height) const {
        return x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1;
    }

    bool isActive() const { return active; }
    
    wchar_t getSymbol() const {
        return L'*'; // Symbol for the bullet
    }
};

// Tank
class Tank : public GameObject {
private:
    Direction direction;
    std::string color;
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
    Tank(int x, int y, Direction dir = Direction::Up, const std::string& color = RESET_COLOR)
        : GameObject(x, y), direction(dir), color(color) {}
    void setDirection(Direction dir) {direction = dir;}
    Direction getDirection() const { return direction; }
    void setColor(const std::string& newColor) {color = newColor;}

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
    void render(WINDOW* win) const {
        mvwaddch(win, y, x, getDirectionSymbol());
        for (const auto& bullet : bullets) {
            if (bullet.isActive()) {
                mvwaddch(win, bullet.getY(), bullet.getX(), bullet.getSymbol());
            }
        }
    }
};

#endif // GAMEOBJECT_H