#ifndef UpdateStruct_H
#define UpdateStruct_H
#include"GameObject.h"
#include"GameUtils.hpp"
struct UpdateStruct{
    int client_id;
    int x,y;
    Direction dir;
    bool valid;
};
#endif
