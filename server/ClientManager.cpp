#include"ClientManager.hpp"
#include<iostream>
#include<vector>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
inline char *id2str(int x){
    static char tmp[6]="0000";
    for(int i=3;i>=0;--i){
        tmp[i]='0'+x%10;
        x/=10;
    }
    tmp[4]=0;
    return tmp;
}
inline bool is_int(const std::string &str){
    auto it=str.begin();
    if(*it=='-') ++it;
    if(it==str.end()) return false;
    for(;it!=str.end();++it) if(!isdigit(*it)) return false;
    return true;
}
inline void sep_str(const std::string &str, std::string &s1, std::string &s2, const char c){
    s1.clear();
    s2.clear();
    bool flag=true;
    for(const auto &a:str){
        if(a==c) flag=false;
        else{
            if(flag) s1+=a;
            else s2+=a;
        }
    }
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
        this->client_data_list[i].id=i;
    }

    //init room data list
    for(int i=0;i<MAX_ROOMS;++i){
        this->room_data_list[i].id=i;
        this->room_data_list[i].state=RoomData::inactive;
        this->room_data_list[i].host_id=-1;
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
            this->client_data_list[i].user_id.clear();
            this->client_data_list[i].room_id=-1;
            this->client_data_list[i].ss.clear();
            //send client id
            snprintf(this->send_buffer,1024,"%s\n",id2str(i));
            write(this->pollfd_list[i].fd,send_buffer,5);

            ++this->client_count;
            std::cout<<"client count: "<<this->client_count<<std::endl;
            return true;
        }
    }
    return false;
}

void ClientManager::rm_client(const int idx){
    std::cout<<"call rm client("<<id2str(idx)<<")"<<std::endl;
    if(this->pollfd_list[idx].fd<0) return;
    //close connection
    if(close(this->pollfd_list[idx].fd)<0){
        std::cerr<<"close error\n";
        std::cerr<<"called by ClientManager\n";
        exit(1);
    }
    this->pollfd_list[idx].fd=-1;

    auto &client_obj=this->client_data_list[idx];
    //if client is in a room
    if(client_obj.room_id>=0){
        this->exit_room(client_obj.id,client_obj.room_id);        
    }
    
    client_obj.state=ClientData::inactive;
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
        for(int a=0;a<n;++a){
            client_data_list[i].ss<<recv_buffer[a];
            flag=false;
            if(recv_buffer[a]=='\n') this->process_commands(i);
        }
        //n<=0
        //client disconnect
        if(flag) this->rm_client(i);
    }
}

void ClientManager::process_commands(const int idx){
    std::cout<<"call process_commands("<<id2str(idx)<<")\n";
    auto &client_obj=this->client_data_list[idx];
    std::string tmp0,tmp1, str;
    std::getline(client_obj.ss,str);
    std::cout<<"command: "<<str<<'\n';
    sep_str(str,tmp0,tmp1,COMMAND_SEP);

    int tmp;
    switch (this->client_data_list[idx].state){
    case ClientData::wait_id:
        if(is_int(tmp0) && std::stoi(tmp0)==client_obj.id){
            client_obj.user_id=tmp1;
            client_obj.state=ClientData::idle;
            std::cout<<"set client "<<id2str(client_obj.id)<<" = "<<client_obj.user_id<<std::endl;
        }
        break;
    case ClientData::idle:
        if(tmp0=="join" && is_int(tmp1)){
            this->join_room(client_obj.id,std::stoi(tmp1));
        }
        break;
    case ClientData::wait_start:
        //exit the room
        if(tmp0=="exit" && is_int(tmp1) && std::stoi(tmp1)==client_obj.room_id){
            this->exit_room(client_obj.id,client_obj.room_id);
        }
        //host can start the game
        else if(
            tmp0=="start" && is_int(tmp1) &&
            std::stoi(tmp1)==client_obj.room_id &&
            this->room_data_list[client_obj.room_id].host_id==client_obj.id
        ) this->start_game(client_obj.room_id);
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
        this->check_state();
    }
}

inline int ClientManager::RoomData::player_count() const{
    return this->client_id_set.size();
}

inline bool ClientManager::RoomData::is_full() const{
    return this->client_id_set.size()>=ROOM_MAX_CLIENTS;
}

void ClientManager::join_room(const int client_id, int room_id){
    std::cout<<"call join room"<<std::endl;
    auto &client_obj=client_data_list[client_id];
    if(room_id>=MAX_ROOMS) return;
    //if room_id < 0, then random match
    if(room_id<0){
        std::vector<int> wait_rooms, empty_rooms;
        for(int i=0;i<MAX_CLIENTS;++i){
            if(this->room_data_list[i].player_count()==0){
                empty_rooms.push_back(i);
            }
            else if(
                this->room_data_list[i].state==RoomData::wait &&
                !this->room_data_list[i].is_full()
            ) wait_rooms.push_back(i);
        }
        //try to enter a room that already has some player
        //then try to enter a empty room
        //otherwise, no room is available
        if(!wait_rooms.empty()) room_id=wait_rooms[random()%wait_rooms.size()];
        else if(!empty_rooms.empty()) room_id=empty_rooms[random()%empty_rooms.size()];
        else{
            write(this->pollfd_list[client_obj.id].fd,"fail\n",5);
            return;
        }
    }

    //join with room_id
    auto &room_obj=room_data_list[room_id];
    
    //if room is full
    if(room_obj.is_full()){
        write(this->pollfd_list[client_obj.id].fd,"fail\n",5);
        return;
    }
    
    room_obj.client_id_set.insert(client_id);

    //send joined room id to client
    snprintf(send_buffer,1024,"join%c%s\n",COMMAND_SEP,id2str(room_obj.id));
    write(this->pollfd_list[client_obj.id].fd,send_buffer,10);

    //if this is the first player of the room
    if(room_obj.player_count()==1){
        room_obj.state=RoomData::wait;
        room_obj.host_id=client_obj.id;
        //notify the host
        snprintf(send_buffer,1024,"host%c%s\n",COMMAND_SEP,id2str(room_obj.id));
        write(this->pollfd_list[room_obj.host_id].fd,send_buffer,10);
    }

    client_obj.state=ClientData::wait_start;
    client_obj.room_id=room_obj.id;
    
    std::cout<<"client "<<id2str(client_id)<<" joined room "<<id2str(room_id)<<std::endl;
}

void ClientManager::exit_room(const int client_id, const int room_id){
    if(room_id<0) return;
    auto &room_obj=room_data_list[room_id];
    auto &client_obj=client_data_list[client_id];

    room_obj.client_id_set.erase(client_id);

    //if the room become empty
    if(room_obj.player_count()==0){
        room_obj.state=RoomData::inactive;
        room_obj.host_id=-1;
    }
    //if the room host exit
    else if(room_obj.host_id==client_id){
        room_obj.host_id=*(room_obj.client_id_set.begin());
        snprintf(send_buffer,1024,"host%c%s\n",COMMAND_SEP,id2str(room_obj.id));
        write(this->pollfd_list[room_obj.host_id].fd,send_buffer,10);
    }

    client_obj.state=ClientData::idle;
    client_obj.room_id=-1;
}

void ClientManager::start_game(const int room_id){
    auto &room_obj=this->room_data_list[room_id];
    room_obj.state=RoomData::play;
    for(auto &item:room_obj.client_id_set){
        this->client_data_list[item].state=ClientData::play;
        write(this->pollfd_list[item].fd,"start\n",6);
    }
    
}

void ClientManager::check_state(){
    std::cout<<"check state\n";
    std::cout<<"connected clients\n";
    bool flag=false;
    for(int i=0;i<MAX_CLIENTS;++i){
        if(this->pollfd_list[i].fd<0) continue;
        if(flag) std::cout<<'\n';
        flag=true;
        std::cout<<"\tclient_id "<<id2str(client_data_list[i].id)<<'\n';
        std::cout<<"\tuser_id "<<client_data_list[i].user_id<<'\n';
        std::cout<<"\tstate ";
        switch(this->client_data_list[i].state){
            case ClientData::inactive: std::cout<<"inactive";break;
            case ClientData::wait_id: std::cout<<"wait_id";break;
            case ClientData::idle: std::cout<<"idle";break;
            case ClientData::wait_start: std::cout<<"wait_start";break;
            case ClientData::play: std::cout<<"play";break;
        }
        std::cout<<"\n";
        std::cout<<"\troom_id ";
        if(this->client_data_list[i].room_id>=0) std::cout<<id2str(this->client_data_list[i].room_id);
        std::cout<<'\n';
    }
    std::cout<<"active rooms\n";
    flag=false;
    for(int i=0;i<MAX_ROOMS;++i){
        if(this->room_data_list[i].state==RoomData::inactive) continue;
        if(flag) std::cout<<'\n';
        flag=true;
        std::cout<<"\troom_id "<<id2str(this->room_data_list[i].id)<<'\n';
        std::cout<<"\tstate ";
        switch(this->room_data_list[i].state){
            case RoomData::inactive: std::cout<<"inactive";break;
            case RoomData::wait: std::cout<<"wait";break;
            case RoomData::play: std::cout<<"play";break;
        }
        std::cout<<'\n';
        std::cout<<"\tclient_id_list ";
        bool flag0=false;
        for(const auto &item:this->room_data_list[i].client_id_set){
            if(flag0) std::cout<<", ";
            std::cout<<id2str(item);
            flag0=true;
        }
        std::cout<<'\n';
        std::cout<<"\thost_id "<<id2str(this->room_data_list[i].host_id)<<'\n';
    }
    std::cout.flush();
}
