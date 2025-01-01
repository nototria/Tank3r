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
    char recv_buffer[1024], send_buffer[1024], udp_recv_buffer[1024];
    std::stringstream client_buffer[MAX_CLIENTS];//access by tcp_listen
    
    int client_count;
    ClientManager cli_mgr;  //access by tcp_listen, game_thread
    pthread_mutex_t cli_mgr_mutex;
    
    RoomManager room_mgr;   //access by tcp_listen, game_thread
    pthread_mutex_t room_mgr_mutex;

    int udp_sock_fd;
    std::queue<InputStruct> input_buffer[MAX_CLIENTS];//access by udp_listen, game_thread
    pthread_mutex_t input_buffer_mutex[MAX_CLIENTS];

    struct sockaddr_in client_udp_addr[MAX_CLIENTS];//access by udp_listen, game_thread
    pthread_mutex_t client_udp_addr_mutex[MAX_CLIENTS];

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
    
    void tcp_listen();
    static void* udp_listen(void *obj_ptr);
    static void* game_loop(void *obj_ptr);

    int last_client_id;
public:
    struct pollfd pollfd_list[1+MAX_CLIENTS];
    GameServer(const int& server_fd, const int& udp_fd);
    inline bool is_full() const;

    void start_server();
};
#endif
/*
Heap-dynamic data       | Stack-dynamic data
(thread-specific data)  | (no special treatment)
--------------------------------------------------
Static Data             | 
(used with mutex)       | 
*/