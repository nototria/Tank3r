#ifndef ClientManager_H
#define ClientManager_H

#include<string>
#include<sstream>
#include"../shared/GameParameters.h"
#include<poll.h>
#include<set>

class ClientManager{
    struct ClientData{
        enum State{inactive, wait_id, idle, wait_start, play} state;
        std::string user_id;
        int id;
        int room_id;
        std::stringstream ss;
    };
    struct RoomData{
        enum State{inactive, wait, play} state;
        std::set<int> client_id_set;
        int host_id;
        int id;
        inline int player_count() const;
        inline bool is_full() const;
    };
    char recv_buffer[1024], send_buffer[1024];
    int client_count;
    ClientData client_data_list[MAX_CLIENTS];
    RoomData room_data_list[MAX_CLIENTS];
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
    void join_room(const int, const int);
    void exit_room(const int, const int);
    void start_game(const int);
    void check_state();
};

#endif
