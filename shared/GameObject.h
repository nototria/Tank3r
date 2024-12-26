#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <string>

// ANSI escape codes for colors
#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define BLUE_COLOR "\033[34m"
#define YELLOW_COLOR "\033[33m"

// Enum for tank directions
enum class Direction {
    Right, // Corresponds to '⊢'
    Left,  // Corresponds to '⊣'
    Up,    // Corresponds to '⊥'
    Down   // Corresponds to '⊤'
};

// Base class: GameObject
class GameObject {
protected:
    int x; // X-coordinate of the object
    int y; // Y-coordinate of the object

public:
    // Constructor
    GameObject(int x = 0, int y = 0) : x(x), y(y) {}

    // Virtual destructor
    virtual ~GameObject() {}

    // Getters for position
    int getX() const { return x; }
    int getY() const { return y; }

    // Setters for position
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    void setPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

    // Virtual render function
    virtual void render() const {
        std::cout << "Rendering GameObject at (" << x << ", " << y << ")" << std::endl;
    }
};

// Derived class: Tank
class Tank : public GameObject {
private:
    Direction direction; // Direction of the tank
    std::string color;   // Color code using ANSI escape sequences

    // Convert direction to ASCII character for rendering
    char getDirectionSymbol() const {
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