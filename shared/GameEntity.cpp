#include"GameEntity.hpp"
#include<string>
#include<iostream>
#include<nlohmann/json.hpp>
//================================================================
//Vector2

Vector2::Vector2(): x(0), y(0){}
Vector2::Vector2(int _x, int _y): x(_x), y(_y){}
Vector2& Vector2::operator+=(Vector2 other){
    this->x+=other.x;
    this->y+=other.y;
    return *this;
}
Vector2& Vector2::operator-=(Vector2 other){
    this->x-=other.x;
    this->y-=other.y;
    return *this;
}
Vector2 Vector2::operator+(Vector2 other) const{
    return Vector2(this->x+other.x,this->y+other.y);
}
Vector2 Vector2::operator-(Vector2 other) const{
    return Vector2(this->x-other.x,this->y-other.y);
}
Vector2 Vector2::operator-() const{
    return Vector2(-this->x,-this->y);
}
std::ostream& operator<<(std::ostream& out, const Vector2 &obj){
    out<<'('<<obj.x<<", "<<obj.y<<')';
    return out;
}
//================================================================
//GameEntity
GameEntity::GameEntity(){
    this->type=none;
}
GameEntity::GameEntity(const std::string &str){
    nlohmann::json tmp;
    try{tmp = nlohmann::json::parse(str);}
    catch(nlohmann::json::parse_error err){
        this->type=none;
        std::cerr<<err.what()<<'\n';
        return;
    }
    try{*this = tmp;}
    catch(nlohmann::detail::out_of_range err){
        this->type=none;
        std::cerr<<err.what()<<" when converting json str to GameEntity\n";
        std::cerr<<str<<std::endl;
        return;
    }
}
std::string GameEntity::to_json_str() const{
    nlohmann::json tmp = *this;
    return tmp.dump();
}
