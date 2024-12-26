#include<iostream>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include"ClientManager.hpp"
signed main(){
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        //get socket error
        std::cerr<<"get socket error\n";
        return 1;
    }
    if(bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        //bind error
        std::cerr<<"bind error\n";
        return 1;
    }
    listen(listen_fd,1024);
    
    ClientManager client_mgr(listen_fd);

    client_mgr.listen();

    return 0;
}
