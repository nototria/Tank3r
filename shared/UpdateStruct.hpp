#ifndef UpdateStruct_H
#define UpdateStruct_H
#include"GameObject.h"
#include"GameUtils.hpp"
#include"GameParameters.h"
struct UpdateStruct{
    char type;//u,f,h
    int client_id;
    int x,y;
    Direction dir;
    int value;
    bool valid;
    UpdateStruct(std::string str){
        std::string tmp1,tmp2;
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        this->valid=true;
        if(tmp1.size()!=1 || (tmp1[0]!='u' && tmp1[0]!='f' && tmp1[0]!='h')){
            valid=false;
            return;
        }
        this->type=tmp1[0];
        
        str=tmp2;
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        if(tmp1.size()!=4 || !is_int(tmp1) || std::stoi(tmp1)<0 || std::stoi(tmp1)>=MAX_CLIENTS){
            valid=false;
            return;
        }
        this->client_id=std::stoi(tmp1);

        str=tmp2;
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        if(this->type='h'){
            if(!is_int(tmp1) || std::stoi(tmp1)<0){
                valid=false;
                return;
            }
            this->value=std::stoi(tmp1);
            return;
        }
        if(!is_int(tmp1) || std::stoi(tmp1)<0 || std::stoi(tmp1)>=SCREEN_WIDTH){
            valid=false;
            return;
        }
        this->x=std::stoi(tmp1);

        str=tmp2;
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        if(!is_int(tmp1) || std::stoi(tmp1)<0 || std::stoi(tmp1)>=SCREEN_WIDTH){
            valid=false;
            return;
        }
        this->y=std::stoi(tmp1);

        str=tmp2;
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        if(!is_int(tmp1) || std::stoi(tmp1)<0 || std::stoi(tmp1)>=4){
            valid=false;
            return;
        }
        switch(std::stoi(tmp1)){
        case 0: this->dir=Direction::Right;break;
        case 1: this->dir=Direction::Left;break;
        case 2: this->dir=Direction::Up;break;
        case 3: this->dir=Direction::Down;break;
        }
    }
};
#endif
