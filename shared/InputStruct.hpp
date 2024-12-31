#ifndef InputStruct_H
#define InputStruct_H
#include"GameUtils.hpp"
#include"GameParameters.h"
struct InputStruct{
    char key;
    int client_id;
    int seq;
    bool valid;
    InputStruct(char _key, int _client_id, int _seq): key(_key), client_id(_client_id), seq(_seq), valid(true) {}
    //key,client_id,seq_number
    InputStruct(std::string str){
        std::string tmp1,tmp2;
        this->valid=true;
        
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        if(tmp1.size()!=1){
            this->valid=false;
            return;
        }
        this->key=tmp1[0];

        str=tmp2;
        sep_str(str,tmp1,tmp2,COMMAND_SEP);
        if(tmp1.size()!=4||!is_int(tmp1)|| std::stoi(tmp1)<0 ||std::stoi(tmp1)>=MAX_CLIENTS){
            this->valid=false;
            return;
        }
        this->client_id=std::stoi(tmp1);
        
        if(!is_int(tmp2)){
            this->valid=false;
            return;
        }
        this->seq=std::stoi(tmp2);
    }
    std::string to_str() const{
        std::string tmp;
        tmp.reserve(10);
        tmp+=this->key;
        tmp+=',';

        tmp+=id2str(this->client_id);
        tmp+=',';
        tmp+=std::to_string(this->seq);
        return tmp;
    }
};
#endif
