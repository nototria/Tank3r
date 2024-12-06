#ifndef GameEntity_H
#define GameEntity_H
#include<string>
#include<iostream>
#include<nlohmann/json.hpp>
struct Position{
    int x,y;
    Position();
    Position(int, int);
    Position& operator+=(const Position);
    Position& operator-=(const Position);
    Position operator+(const Position) const;
    Position operator-(const Position) const;
    Position operator-() const;
    friend std::ostream& operator<<(std::ostream&, const Position &);
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Position,x,y)
};
const Position unit_move[4]={
    {-1,0},{0,1},{1,0},{0,-1}
};
struct GameEntity{
    std::string id;
    enum EntityType{none,player,bullet} type;
    Position pos;
    enum EntityRota{up, right, down, left} rota;
    int health;

    int fire_cd;
    bool fire;

    std::string owner;

    GameEntity();
    GameEntity(const std::string&);
    std::string to_json_str() const;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameEntity,type,id,pos,rota,health,fire,fire_cd,owner)
};
#endif
