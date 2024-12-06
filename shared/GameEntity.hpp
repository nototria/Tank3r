#ifndef GameEntity_H
#define GameEntity_H
#include<string>
#include<iostream>
#include<nlohmann/json.hpp>
struct Vector2{
    int x,y;
    Vector2();
    Vector2(int, int);
    Vector2& operator+=(const Vector2);
    Vector2& operator-=(const Vector2);
    Vector2 operator+(const Vector2) const;
    Vector2 operator-(const Vector2) const;
    Vector2 operator-() const;
    friend std::ostream& operator<<(std::ostream&, const Vector2 &);
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Vector2,x,y)
};
const Vector2 unit_move[4]={
    {-1,0},{0,1},{1,0},{0,-1}
};
struct GameEntity{
    std::string id;
    enum EntityType{none,player,bullet} type;
    Vector2 pos;
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
