#include<iostream>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<poll.h>
#include<sstream>
#include"../shared/GameParameters.h"
using namespace std;
class ClientData{
public:
    enum state{inactive, wait_id, idle, wait_start, play} state;
    string user_id;
    int client_id;
    int room_id;
    stringstream ss;
};
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
    //using poll
    int client_count=0, poll_result;
    struct pollfd pollfd_list[1+MAX_CLIENTS];
    //put server tcp fd at [MAX_CLIENTS]
    pollfd_list[MAX_CLIENTS].fd=listen_fd;
    pollfd_list[MAX_CLIENTS].events=POLLRDNORM;
    for(int i=0;i<MAX_CLIENTS;++i) pollfd_list[i].fd=-1;
    
    //client state list
    ClientData client_data_list[MAX_CLIENTS];
    for(int i=0;i<MAX_CLIENTS;++i){
        client_data_list[i].state=ClientData::inactive;
        client_data_list[i].client_id=i;
    }

    //vars for handling new connection
    socklen_t sockaddr_len;
    struct sockaddr_in client_addr;
    int client_tcp_fd;

    while(true){
        poll_result=poll(pollfd_list,1+MAX_CLIENTS,-1);
        //timeout set to -1; block forever if nothing is ready
        if(poll_result<0){
            //poll error
            std::cerr<<"poll error\n";
            return 1;
        }
        //handle new connection
        if(pollfd_list[MAX_CLIENTS].revents & POLLRDNORM){
            sockaddr_len=sizeof(client_addr);
            client_tcp_fd=accept(listen_fd,(struct sockaddr *)&client_addr, &sockaddr_len);
            //if no error occure and not reach client limit
            if(client_tcp_fd>0 && client_count!=MAX_CLIENTS){
                for(int i=0;i<MAX_CLIENTS;++i){
                    if(pollfd_list[i].fd<0){
                        //setting pollfd_list
                        pollfd_list[i].fd=client_tcp_fd;
                        pollfd_list[i].events=POLLRDNORM;
                        //setting client_data_list
                        client_data_list[i].state=ClientData::wait_id;
                        client_data_list[i].room_id=-1;
                        ++client_count;
                        break;
                    }
                }
            }
        }
        //handle client command
        for(int i=0;i<MAX_CLIENTS;++i){
            if(pollfd_list[i].fd<0) continue;
            if((pollfd_list[i].revents & (POLLRDNORM|POLLERR))){
                
            }
            else{

            }
        }
    }

    return 0;
}
