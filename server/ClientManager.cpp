#include"ClientManager.hpp"
#include<iostream>
#include<vector>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

ClientManager::ClientManager(){
    //init client data list
    for(int i=0;i<MAX_CLIENTS;++i){
        this->client_data_list[i].state=ClientData::inactive;
        this->client_data_list[i].id=i;
    }
}

void ClientManager::add_client(const int client_id){
    auto &client_obj=client_data_list[client_id];

    client_obj.state=ClientData::wait_name;
    client_obj.room_id=-1;
    client_obj.user_name="";
}

void ClientManager::rm_client(const int client_id){
    auto &client_obj=client_data_list[client_id];

    client_obj.state=ClientData::inactive;
}

void ClientManager::set_user_name(const int client_id, const std::string &str){
    auto &client_obj=client_data_list[client_id];

    client_obj.user_name=str;
    client_obj.state=ClientData::idle;
}

void ClientManager::join_room(const int client_id, int room_id){
    client_data_list[client_id].state=ClientData::wait_start;
    client_data_list[client_id].room_id=room_id;
}

void ClientManager::exit_room(const int client_id){
    client_data_list[client_id].state=ClientData::idle;
    client_data_list[client_id].room_id=-1;
}

void ClientManager::start_game(const int client_id){

}

const ClientData::State& ClientManager::get_state(const int client_id) const{
    return client_data_list[client_id].state;
}

const std::string& ClientManager::get_user_name(const int client_id) const{
    return client_data_list[client_id].user_name;
}

const int& ClientManager::get_room_id(const int client_id) const{
    return client_data_list[client_id].room_id;
}
