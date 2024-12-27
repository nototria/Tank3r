#include"GameEntity.hpp"
#include<string>
#include<iostream>
#include<nlohmann/json.hpp>
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
