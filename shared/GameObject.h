#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>

class GameObject {
protected:
    int x; // X-coordinate of the object
    int y; // Y-coordinate of the object

public:
    // Constructor
    GameObject(int x = 0, int y = 0) : x(x), y(y) {}

    // Destructor
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

    // Render function (virtual so it can be overridden by derived classes)
    virtual void render() const {
        std::cout << "Rendering GameObject at (" << x << ", " << y << ")" << std::endl;
    }
};

#endif // GAMEOBJECT_H