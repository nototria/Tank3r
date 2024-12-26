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

class GameObject {
protected:
    int x;
    int y;

public:
    GameObject(int x = 0, int y = 0) : x(x), y(y) {}
    virtual ~GameObject() {}

    int getX() const { return x; }
    int getY() const { return y; }

    // function
    void setPosition(int newX, int newY){
        x = newX;
        y = newY;
    }
    virtual void render() const {
        std::cout << "Rendering GameObject at (" << x << ", " << y << ")" << std::endl;
    }
};

// Derived class: Tank
class Tank : public GameObject {
private:
    Direction direction;
    std::string color;

    // Convert direction to ASCII character for rendering
    int getDirectionSymbol() const {
        switch (direction) {
            case Direction::Right: return '⊢';
            case Direction::Left:  return '⊣';
            case Direction::Up:    return '⊥';
            case Direction::Down:  return '⊤';
            default: return '?';
        }
    }

public:
    // Constructor
    Tank(int x, int y, Direction dir = Direction::Up, const std::string& color = RESET_COLOR)
        : GameObject(x, y), direction(dir), color(color) {}

    // Set tank direction
    void setDirection(Direction dir) {
        direction = dir;
    }

    // Get tank direction
    Direction getDirection() const { return direction; }

    // Set tank color
    void setColor(const std::string& newColor) {
        color = newColor;
    }

    // Render function
    void render() const override {
        std::cout << color
                  << "Rendering Tank at (" << x << ", " << y << ") facing "
                  << (direction == Direction::Right ? "right" :
                      direction == Direction::Left ? "left" :
                      direction == Direction::Up ? "up" : "down")
                  << " [" << getDirectionSymbol() << "]"
                  << RESET_COLOR << std::endl;
    }
};

#endif // GAMEOBJECT_H