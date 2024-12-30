#ifndef GameSync_H
#define GameSync_H

#include<iostream>
#include<vector>
#include"../shared/GameObject.h"

class GameSync{
private:

public:
    void send_input(char key);
    void update_tank(std::vector<Tank> &tanklist);
};

#endif
