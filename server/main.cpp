#include<iostream>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include"GameServer.hpp"
signed main(){
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        //get socket error
        std::cerr<<"get tcp socket error\n";
        return 1;
    }
    if(bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        //bind error
        std::cerr<<"tcp bind error\n";
        return 1;
    }
    listen(listen_fd,1024);

    struct sockaddr_in udp_addr;
    memset(&udp_addr,0,sizeof(udp_addr));
    udp_addr.sin_family=AF_INET;
    udp_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    udp_addr.sin_port=htons(UDP_PORT);
    int udp_fd=socket(AF_INET,SOCK_DGRAM,0);
    if(udp_fd<0){
        std::cerr<<"get udp socket error\n";
        return 1;
    }
    if(bind(udp_fd,(struct sockaddr*)&udp_addr,sizeof(udp_addr))<0){
        std::cerr<<"udp bind error\n";
        return 1;
    }
    
    GameServer server(listen_fd,udp_fd);
    server.start_server();

    return 0;
}
