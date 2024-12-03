#include<iostream>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#define SERV_PORT 9999
#define MAX_CLIENTS 1000

signed main(){
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        std::cerr<<"get socket error\n";
        return 1;
    }
    if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        std::cerr<<"bind error\n";
        return 1;
    }


    return 0;
}
