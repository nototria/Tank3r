#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <string>

#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define BLUE_COLOR "\033[34m"
#define YELLOW_COLOR "\033[33m"

enum class Direction {Right, Left, Up, Down};
enum class GameState {TitleScreen, UsernameInput, GameLoop, EndScreen};

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

// Tank
class Tank : public GameObject {
private:
    Direction direction;
    std::string color;

    wchar_t getDirectionSymbol() const {
        switch (direction) {
            case Direction::Right: return L'⊢';
            case Direction::Left:  return L'⊣';
            case Direction::Up:    return L'⊥';
            case Direction::Down:  return L'⊤';
            default: return L'?';
        }
    }

public:
    Tank(int x, int y, Direction dir = Direction::Up, const std::string& color = RESET_COLOR)
        : GameObject(x, y), direction(dir), color(color) {}

    void setDirection(Direction dir) {direction = dir;}

    Direction getDirection() const { return direction; }

    void setColor(const std::string& newColor) {color = newColor;}
};

// Bullet
class Bullet : public GameObject {
private:
    Direction direction;

public:
    Bullet(int x, int y, Direction dir) : GameObject(x, y), direction(dir) {}

    void move() {
        switch (direction) {
            case Direction::Up:    y -= 1; break;
            case Direction::Down:  y += 1; break;
            case Direction::Left:  x -= 1; break;
            case Direction::Right: x += 1; break;
        }
    }

    bool isOutOfBounds(int width, int height) const {
        return x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1;
    }

    wchar_t getSymbol() const {
        return L'*'; // Symbol for the bullet
    }
};

#endif // GAMEOBJECT_H