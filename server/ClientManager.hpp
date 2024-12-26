#ifndef ClientManager_H
#define ClientManager_H

#include<string>
#include<sstream>
#include"../shared/GameParameters.h"
#include<poll.h>
#include<vector>

class ClientManager{
    struct ClientData{
        enum State{inactive, wait_id, idle, wait_start, play} state;
        std::string user_id;
        int client_id;
        int room_id;
        std::stringstream ss;
    };
    struct RoomData{
        enum State{inactive, wait, play};


    };
    char recv_buffer[1024], send_buffer[1024];
    int client_count;
    ClientData client_data_list[MAX_CLIENTS];
public:
    struct pollfd pollfd_list[1+MAX_CLIENTS];
    ClientManager(const int&);
    inline bool is_full() const;
    bool add_client(const int&);
    void rm_client(const int);
    void recv_commands();
    void recv_connection();
    void process_commands(const int);
    void listen();
};

#endif
