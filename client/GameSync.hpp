#ifndef GameSync_H
#define GameSync_H

#include<iostream>
#include<vector>
#include<deque>
#include"../shared/GameObject.h"
#include"../shared/InputStruct.hpp"
class GameSync{
private:
    std::deque<InputStruct> input_queue;
public:
    void send_input(char key);
    void update_tank(std::vector<Tank> &tanklist);
};

#endif
