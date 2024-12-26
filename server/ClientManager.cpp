#include"ClientManager.hpp"
#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
inline char *client_id(int x){
    static char tmp[6]="0000\n";
    for(int i=3;i>=0;--i){
        tmp[i]='0'+x%10;
        x/=10;
    }
    return tmp;
}
inline bool is_int(const std::string &str){
    auto it=str.begin();
    if(*it=='-') ++it;
    if(it==str.end()) return false;
    for(;it!=str.end();++it) if(!isdigit(*it)) return false;
    return true;
}

ClientManager::ClientManager(const int &server_fd){
    this->client_count=0;
    //init pollfd list
    this->pollfd_list[MAX_CLIENTS].fd=server_fd;
    this->pollfd_list[MAX_CLIENTS].events=POLLRDNORM;
    for(int i=0;i<MAX_CLIENTS;++i) this->pollfd_list[i].fd=-1;

    //init client data list
    for(int i=0;i<MAX_CLIENTS;++i){
        this->client_data_list[i].state=ClientData::inactive;
        this->client_data_list[i].client_id=i;
    }
}

inline bool ClientManager::is_full() const{
    return this->client_count>=MAX_CLIENTS;
}

//return false if is_full
bool ClientManager::add_client(const int &client_fd){
    std::cout<<"call add_client"<<std::endl;
    for(int i=0;i<MAX_CLIENTS;++i){
        if(this->pollfd_list[i].fd<0){
            //setting pollfd_list
            this->pollfd_list[i].fd=client_fd;
            this->pollfd_list[i].events=POLLRDNORM;
            //init client_data_list
            this->client_data_list[i].state=ClientData::wait_id;
            this->client_data_list[i].room_id=-1;
            this->client_data_list[i].ss.clear();

            write(this->pollfd_list[i].fd,client_id(i),5);

            ++this->client_count;
            std::cout<<"client count: "<<this->client_count<<std::endl;
            return true;
        }
    }
    return false;
}

void ClientManager::rm_client(const int idx){
    std::cout<<"call rm client("<<idx<<")"<<std::endl;
    if(this->pollfd_list[idx].fd<0) return;
    if(close(this->pollfd_list[idx].fd)<0){
        std::cerr<<"close error\n";
        std::cerr<<"called by ClientManager\n";
        exit(1);
    }
    this->pollfd_list[idx].fd=-1;
    this->client_data_list[idx].state=ClientData::inactive;
    --this->client_count;
    std::cout<<"client count: "<<this->client_count<<std::endl;
}

void ClientManager::recv_connection(){
    std::cout<<"call recv_connection"<<std::endl;
    socklen_t sockaddr_len;
    struct sockaddr_in client_addr;
    int client_tcp_fd;

    if(this->pollfd_list[MAX_CLIENTS].revents & POLLRDNORM){
        sockaddr_len=sizeof(client_addr);
        client_tcp_fd=accept(this->pollfd_list[MAX_CLIENTS].fd,(struct sockaddr *)&client_addr, &sockaddr_len);
        //if no error occure and not reach client limit
        if(client_tcp_fd>0){
            if(!this->add_client(client_tcp_fd)){
                //close connection if add_client return false (server is full)
                close(client_tcp_fd);
            }
        }
    }
}

void ClientManager::recv_commands(){
    std::cout<<"call recv commands"<<std::endl;
    for(int i=0;i<MAX_CLIENTS;++i){
        if(this->pollfd_list[i].fd<0) continue;
        if(!(pollfd_list[i].revents & (POLLRDNORM|POLLERR))) continue;
        memset(recv_buffer,0,sizeof(recv_buffer));
        int n=read(pollfd_list[i].fd,this->recv_buffer,1024);
        bool flag=true;
        for(int a=0;a<1024&&recv_buffer[a];++a){
            client_data_list[i].ss<<recv_buffer[a];
            flag=false;
            if(recv_buffer[a]=='\n') this->process_commands(i);
        }
        //n<=0
        if(flag) this->rm_client(i);
    }
}

void ClientManager::process_commands(const int idx){
    auto &item=this->client_data_list[idx];
    auto &input=item.ss;
    std::string tmp0,tmp1;
    switch (this->client_data_list[idx].state){
    case ClientData::wait_id:
        input>>tmp0;
        if(is_int(tmp0) && std::stoi(tmp0)==item.client_id){
            input>>item.user_id;
            item.state=ClientData::idle;
            std::cout<<"set client "<<client_id(item.client_id);
        }
        else input>>tmp1;
        break;
    default: break;
    }
}

void ClientManager::listen(){
    std::cout<<"call listen"<<std::endl;
    while(true){
        int poll_result=poll(this->pollfd_list,1+MAX_CLIENTS,-1);
        std::cout<<"poll"<<std::endl;
        if(poll_result<0){
            //poll error
            std::cerr<<"poll error\n";
            exit(1);
        }
        this->recv_connection();
        this->recv_commands();
    }
}
