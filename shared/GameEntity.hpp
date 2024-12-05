#ifndef GameEntity_H
#define GameEntity_H
#include"uuid.h"
struct Position{
    int x,y;
};
struct GameEntity{
    std::string id;
    enum{player,bullet} type;
    Position pos;
    enum{up, right, down, left} rota;
    int health;

    int fire_cd;
    bool fire;
    std::string to_str();
    void from_str(std::string &);
    GameEntity(std::string &);
};
#endif
