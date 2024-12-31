#ifndef GameSync_H
#define GameSync_H

#include<iostream>
#include<vector>
#include<deque>
#include"../shared/GameObject.h"
#include"../shared/InputStruct.hpp"
#include"../shared/UpdateStruct.hpp"
#include<poll.h>
class GameSync{
private:
    std::deque<InputStruct> input_queue;
    std::deque<UpdateStruct> update_queue;
    pthread_mutex_t update_mutex;
    int udp_fd;
    int client_id;
    int seq;
    static void* udp_listen(void *obj_ptr);
    bool is_running;
public:
    GameSync(const int connected_udp_fd, int _client_id):
        udp_fd(connected_udp_fd),
        client_id(_client_id), seq(0) {
            update_mutex = PTHREAD_MUTEX_INITIALIZER;
        };
    void send_input(char key);
    void update_tank(std::vector<Tank> &tanks);
};

void GameSync::send_input(char key){
    input_queue.emplace_back(InputStruct{key,this->client_id,seq});
    write(udp_fd,input_queue.back().to_str().c_str(),input_queue.back().to_str().size());
}

void GameSync::update_tank(std::vector<Tank> &tanks){

}

void* GameSync::udp_listen(void *obj_ptr){
    pthread_detach(pthread_self());
    auto &self=*(GameSync*)obj_ptr;
    struct pollfd pollfd_list[1];
    pollfd_list[0].fd=self.udp_fd;
    pollfd_list[0].events=POLLRDNORM;
    char recv_buffer[1024];
    while(self.is_running){
        int nready=poll(pollfd_list,1,20);
        if(nready<=0) continue;
        if(pollfd_list[0].revents & (POLLRDNORM | POLLERR)){
            int n=read(pollfd_list[0].fd,recv_buffer,1024);
            if(n>0 && recv_buffer[n-1]=='\n') --n;//test
            recv_buffer[n]='\0';
            UpdateStruct tmp(recv_buffer);
            if(tmp.valid){
                pthread_mutex_lock(&self.update_mutex);
                self.update_queue.push_back(tmp);
                pthread_mutex_lock(&self.update_mutex);
            }
        }
    }
}

#endif
