#ifndef GameSync_H
#define GameSync_H

#include<iostream>
#include<vector>
#include<deque>
#include"../shared/GameObject.h"
#include"../shared/InputStruct.hpp"
#include"../shared/UpdateStruct.hpp"
#include<poll.h>
#include<fstream>
class GameSync{
private:
    std::deque<InputStruct> input_queue;
    
    pthread_mutex_t update_mutex;
    std::deque<UpdateStruct> update_queue;
    UpdateStruct last_ack;
    
    int udp_fd;
    int my_client_id;
    int seq;
    static void* udp_listen(void *obj_ptr);
    bool is_running;
    std::ofstream log_file;
public:
    GameSync(const int connected_udp_fd, int _client_id, const Tank &this_tank):
        udp_fd(connected_udp_fd),
        my_client_id(_client_id), seq(1),
        last_ack(_client_id,this_tank), is_running(false) {
            std::string file_name=id2str(_client_id);
            file_name+=".log";
            log_file.open(file_name);
            update_mutex = PTHREAD_MUTEX_INITIALIZER;
        };
    void send_input(char key);
    void update_tank(std::map<int,Tank> &tanks, std::vector<MapObject> &staticObjects);
    void start_inGame_listen();
    void stop_inGame_listen(){
        this->is_running=false;
    }
};

void GameSync::send_input(char key){
    input_queue.emplace_back(key,this->my_client_id,seq++);
    write(udp_fd,input_queue.back().to_str().c_str(),input_queue.back().to_str().size());
}

void GameSync::update_tank(std::map<int,Tank> &tanks, std::vector<MapObject> &staticObjects){
    pthread_mutex_lock(&this->update_mutex);
    while(!update_queue.empty()){
        switch(update_queue.front().type){
        case 'u':
            if(update_queue.front().client_id==this->my_client_id){
                //get last ack
                if(this->last_ack.seq<update_queue.front().seq){
                    this->last_ack=update_queue.front();
                }
            }
            else{
                //update other tanks
                if(tanks.find(update_queue.front().client_id)!=tanks.end()){
                    tanks[update_queue.front().client_id].setX(update_queue.front().x);
                    tanks[update_queue.front().client_id].setY(update_queue.front().y);
                    tanks[update_queue.front().client_id].setDirection(update_queue.front().dir);
                }
            }
            break;
        case 'f':
            //remote tank fire
            if(update_queue.front().client_id!=this->my_client_id && tanks.find(update_queue.front().client_id)!=tanks.end()){
                tanks[update_queue.front().client_id].fireBullet();
            }
            break;
        case 'h':
            //tank hp update
            if(tanks.find(update_queue.front().client_id)!=tanks.end()){
                tanks[update_queue.front().client_id].setHP(update_queue.front().value);
            }
            break;
        default: break;
        }
        update_queue.pop_front();
    }
    pthread_mutex_unlock(&this->update_mutex);

    //apply client side prediction
    auto &this_tank=tanks[this->my_client_id];
    //discard old inputs
    while(!input_queue.empty() && input_queue.front().seq<=last_ack.seq){
        input_queue.pop_front();
    }

    int preX = last_ack.x;
    int preY = last_ack.y;
    int nextX, nextY;
    for(auto &item:input_queue){
        switch(item.key){
        case 'w':
            this_tank.setDirection(Direction::Up);
            nextY=preY-1;
            if(nextY>0 && !this_tank.checkTankCollision(preX,nextY,staticObjects)){
                this_tank.setY(nextY);
            }
            break;
        case 'a':
            this_tank.setDirection(Direction::Left);
            nextX=preX-1;
            if(nextX>0 && !this_tank.checkTankCollision(nextX,preY,staticObjects)){
                this_tank.setX(nextX);
            }
            break;
        case 's':
            this_tank.setDirection(Direction::Down);
            nextY=preY+1;
            if(nextY<SCREEN_HEIGHT-1 && !this_tank.checkTankCollision(preX,nextY,staticObjects)){
                this_tank.setY(nextY);
            }
            break;
        case 'd':
            this_tank.setDirection(Direction::Right);
            nextX=preX+1;
            if(nextX<SCREEN_WIDTH-1 && !this_tank.checkTankCollision(nextX,preY,staticObjects)){
                this_tank.setX(nextX);
            }
            break;
        }
        preX=this_tank.getX();
        preY=this_tank.getY();
    }
}

void* GameSync::udp_listen(void *obj_ptr){
    pthread_detach(pthread_self());
    auto &self=*(GameSync*)obj_ptr;
    struct pollfd pollfd_list[1];
    pollfd_list[0].fd=self.udp_fd;
    pollfd_list[0].events=POLLRDNORM;
    char recv_buffer[1024];
    while(self.is_running){
        int nready=poll(pollfd_list,1,10);
        if(nready<=0) continue;
        if(pollfd_list[0].revents & (POLLRDNORM | POLLERR)){
            int n=read(pollfd_list[0].fd,recv_buffer,1024);
            recv_buffer[n]='\0';
            self.log_file<<"recv "<<recv_buffer<<std::endl;
            UpdateStruct tmp(recv_buffer);
            if(tmp.valid){
                pthread_mutex_lock(&self.update_mutex);
                self.update_queue.push_back(tmp);
                self.log_file<<"store "<<tmp.to_str()<<std::endl;
                pthread_mutex_unlock(&self.update_mutex);
            }
        }
    }
    return NULL;
}

void GameSync::start_inGame_listen(){
    write(udp_fd,id2str(this->my_client_id),4);
    this->is_running=true;
    pthread_t tid;
    pthread_create(&tid,NULL,GameSync::udp_listen,this);
}
#endif
