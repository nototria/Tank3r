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
enum class GameState {
    TitleScreen,
    UsernameInput,
    GameLoop,
    EndScreen
};

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

// Tank class derived from GameObject
class Tank : public GameObject {
private:
    Direction direction;
    std::string color;

public:
    // Constructor
    Tank(int x, int y, Direction dir = Direction::Up, const std::string& color = "\033[0m")
        : GameObject(x, y), direction(dir), color(color) {}

    // Set tank direction
    void setDirection(Direction dir) {direction = dir;}
    // Get tank direction
    Direction getDirection() const { return direction; }
};

#endif // GAMEOBJECT_H