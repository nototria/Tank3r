#ifndef ClientManager_H
#define ClientManager_H

#include<string>
#include<sstream>
#include"../shared/GameParameters.h"
#include<poll.h>
#include<set>
struct ClientData{
    enum State{inactive, wait_name, idle, wait_start, play} state;
    std::string user_name;
    int id;
    int room_id;
};
class ClientManager{
    ClientData client_data_list[MAX_CLIENTS];
public:
    ClientManager();
    //init client data slot for the given client_id
    void add_client(const int client_id);
    //reset client data slot for the given client_id
    void rm_client(const int client_id);

    const ClientData::State& get_state(const int) const;
    
    //set user_name and change client state
    void set_user_name(const int, const std::string &);
    const std::string& get_user_name(const int) const;

    //join room and change client state
    void join_room(const int client_id, const int room_id);
    //exit room and change client state
    void exit_room(const int client_id);
    const int& get_room_id(const int client_id) const;

    void start_game(const int);

    void check_state() const;
};

#endif
