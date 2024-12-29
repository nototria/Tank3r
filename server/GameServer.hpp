#ifndef SERVER_H
#define SERVER_H
#include"ClientManager.hpp"
#include"RoomManager.hpp"
#include"../shared/GameParameters.h"
#include"../shared/InputStruct.hpp"
#include<poll.h>
#include<sstream>
#include<pthread.h>
#include<queue>
#include<netinet/in.h>
class GameServer{
private:
    char recv_buffer[1024], send_buffer[1024];
    int client_count;
    ClientManager cli_mgr;  //access by tcp_listen, game_thread
    RoomManager room_mgr;   //access by tcp_listen, game_thread
    std::stringstream client_buffer[MAX_CLIENTS];//access by tcp_listen

    std::queue<InputStruct> input_buffer[MAX_CLIENTS];//access by udp_listen, game_thread
    pthread_mutex_t input_buffer_lock[MAX_CLIENTS];

    struct sockaddr_in client_udp_addr[MAX_CLIENTS];//access by udp_listen, game_thread
    pthread_mutex_t client_udp_addr_lock[MAX_CLIENTS];

    void recv_commands();
    void recv_connection();

    //process commands in the slot with the idx
    //parse the command and call the correspond function
    void process_commands(const int idx);
    
    //assume that client_fd is a active tcp fd
    //add a client when receving a new connection
    //close the connection if the server is full
    void add_client(const int idx);
    
    //rm the client with idx when it disconnected
    void rm_client(const int idx);

    //set user name and change client state
    void set_user_name(const int idx, const std::string&);
    
    //join a client to a room
    //if room_id<0 then random match
    //reply the result to client
    //notify the new host
    void join_room(const int idx, int room_id);
    
    //assume that the client is in a room
    //a client exit a room
    //notify the new host
    void exit_room(const int client_id);

    void start_game(const int room_id);
public:
    struct pollfd pollfd_list[1+MAX_CLIENTS];
    GameServer(const int&);
    inline bool is_full() const;

    void tcp_listen();
    void udp_listen();
};
#endif
