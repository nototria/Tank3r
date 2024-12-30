#include"GameServer.hpp"
#include<iostream>
#include<cstring>
#include<vector>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<map>
#include"../shared/GameUtils.hpp"
#include"../shared/GameObject.h"
#include"../shared/map_generator.cpp"
struct StartParam{
    GameServer *obj_ptr;
    int room_id;
    StartParam(GameServer *_obj_ptr, const int _room_id): obj_ptr(_obj_ptr), room_id(_room_id){}
};

GameServer::GameServer(const int &server_fd, const int &udp_fd){
    this->client_count=0;
    //init pollfd list
    this->pollfd_list[MAX_CLIENTS].fd=server_fd;
    this->pollfd_list[MAX_CLIENTS].events=POLLRDNORM;
    for(int i=0;i<MAX_CLIENTS;++i) this->pollfd_list[i].fd=-1;

    //udp_sock_fd
    this->udp_sock_fd=udp_fd;

    //init mutex
    cli_mgr_mutex=PTHREAD_MUTEX_INITIALIZER;
    room_mgr_mutex=PTHREAD_MUTEX_INITIALIZER;
    for(int i=0;i<MAX_CLIENTS;++i){
        input_buffer_mutex[i]=PTHREAD_MUTEX_INITIALIZER;
        client_udp_addr_mutex[i]=PTHREAD_MUTEX_INITIALIZER;
    }
}

inline bool GameServer::is_full() const{
    return this->client_count>=MAX_CLIENTS;
}

void GameServer::add_client(const int client_fd){
    for(int i=0;i<MAX_CLIENTS;++i){
        if(pollfd_list[i].fd<0){//if this slot it empty
            //setting pollfd_list
            pollfd_list[i].fd=client_fd;
            pollfd_list[i].events=POLLRDNORM;
            client_buffer[i].clear();
            //call cli_mgr.add_client to init client data slot
            cli_mgr.add_client(i);
            //send client_id to client
            snprintf(send_buffer,1024,"%s\n",id2str(i));
            write(client_fd,send_buffer,5);
            //store cli addr
            socklen_t addrlen;
            getpeername(client_fd,(struct sockaddr*)this->client_udp_addr+i,&addrlen);
            this->client_udp_addr[i].sin_port=UDP_PORT;
            return;
        }
    }
    //if the server is full
    close(client_fd);
}

void GameServer::rm_client(const int idx){
    //if the client is in a room
    if(this->cli_mgr.get_room_id(idx)>=0){
        this->exit_room(idx);
    }
    //call cli_mgr.rm_client to release resource
    this->cli_mgr.rm_client(idx);
    //close connection
    close(this->pollfd_list[idx].fd);
    //reset pollfd_list
    this->pollfd_list[idx].fd=-1;
}

inline void GameServer::set_user_name(const int idx, const std::string &str){
    cli_mgr.set_user_name(idx,str);
    std::cout<<"set client "<<id2str(idx)<<" = "<<cli_mgr.get_user_name(idx)<<std::endl;
}

void GameServer::join_room(const int idx, int room_id){
    int client_id=idx;
    if(room_mgr.join_room(client_id,room_id)){
        //join successful
        cli_mgr.join_room(client_id,room_id);
        //send joined room id to client
        snprintf(send_buffer,1024,"join%c%s\n",COMMAND_SEP,id2str(room_id));
        write(this->pollfd_list[idx].fd,send_buffer,10);

        //if this is the first player of the room
        if(room_mgr.player_count(room_id)==1){
            //notify the new host
            snprintf(send_buffer,1024,"host%c%s\n",COMMAND_SEP,id2str(room_id));
            write(this->pollfd_list[idx].fd,send_buffer,10);
        }

        return;
    }
    //join fail
    //send fail to client
    write(this->pollfd_list[client_id].fd,"fail\n",5);
    return;
}

void GameServer::exit_room(const int idx){
    int client_id=idx;
    int room_id=cli_mgr.get_room_id(client_id);
    if(room_mgr.exit_room(client_id,room_id)){
        //if host change
        snprintf(send_buffer,1024,"host%c%s\n",COMMAND_SEP,id2str(room_id));
        write(this->pollfd_list[room_mgr.get_host_id(room_id)].fd,send_buffer,10);
    }
    cli_mgr.exit_room(client_id);
}

void GameServer::start_game(const int room_id){
    //generate message
    //start,player_count\nclient_id,user_name\nclient_id,user_name\n...
    snprintf(send_buffer,1024,"start,%d\n",room_mgr.player_count(room_id));
    int str_idx=strlen(send_buffer);
    for(const auto &client_id:room_mgr.get_clients(room_id)){
        const auto &user_name=cli_mgr.get_user_name(client_id);
        snprintf(send_buffer+str_idx,1024-str_idx,"%s,%s\n",id2str(client_id),user_name.c_str());
        str_idx+=(6+user_name.size());
    }
    srand(time(0));
    room_mgr.set_map_seed(room_id,(unsigned long)random());
    snprintf(send_buffer+str_idx,1024-str_idx,"seed,%lu\n",room_mgr.get_map_seed(room_id));
    for(const auto &client_id:room_mgr.get_clients(room_id)){
        write(pollfd_list[client_id].fd,send_buffer,strlen(send_buffer));
    }

    room_mgr.start_game(room_id);

    auto *ptr=new StartParam(this,room_id);
    pthread_t tid;
    pthread_create(&tid,NULL,GameServer::game_loop,(void*)ptr);
}

void GameServer::recv_connection(){
    std::cout<<"call recv_connection"<<std::endl;
    socklen_t sockaddr_len;
    struct sockaddr_in client_addr;
    int client_tcp_fd;

    if(this->pollfd_list[MAX_CLIENTS].revents & POLLRDNORM){
        sockaddr_len=sizeof(client_addr);
        client_tcp_fd=accept(this->pollfd_list[MAX_CLIENTS].fd,(struct sockaddr *)&client_addr, &sockaddr_len);
        //if no error occure and not reach client limit
        if(client_tcp_fd>0){
            //attempt to add a client
            this->add_client(client_tcp_fd);
        }
    }
}

void GameServer::recv_commands(){
    std::cout<<"call recv commands"<<std::endl;
    for(int i=0;i<MAX_CLIENTS;++i){
        if(this->pollfd_list[i].fd<0) continue;
        if(!(pollfd_list[i].revents & (POLLRDNORM|POLLERR))) continue;
        memset(recv_buffer,0,sizeof(recv_buffer));
        int n=read(pollfd_list[i].fd,this->recv_buffer,1024);
        bool flag=true;
        for(int a=0;a<n;++a){
            this->client_buffer[i]<<recv_buffer[a];
            flag=false;
            if(recv_buffer[a]=='\n') this->process_commands(i);
        }
        //n<=0
        //client disconnect
        if(flag) this->rm_client(i);
    }
}

void GameServer::process_commands(const int idx){
    std::cout<<"call process_commands("<<id2str(idx)<<")\n";
    std::string tmp0,tmp1, str;
    std::getline(client_buffer[idx],str);
    std::cout<<"command: "<<str<<'\n';
    sep_str(str,tmp0,tmp1,COMMAND_SEP);

    int tmp, cli_room_id;
    switch (this->cli_mgr.get_state(idx)){
    case ClientData::wait_name:
        //client_id,user_name
        if(is_int(tmp0) && std::stoi(tmp0)==idx) this->set_user_name(idx,tmp1);
        break;
    case ClientData::idle:
        //join,room_id
        if(tmp0=="join" && is_int(tmp1)) this->join_room(idx,std::stoi(tmp1));
        break;
    case ClientData::wait_start:
        cli_room_id=cli_mgr.get_room_id(idx);
        //exit,room_id
        if(tmp0=="exit" && is_int(tmp1) && std::stoi(tmp1)==cli_room_id) this->exit_room(idx);
        //host can start the game
        else if(
            tmp0=="start" && is_int(tmp1) &&
            std::stoi(tmp1)==cli_mgr.get_room_id(idx) &&
            room_mgr.player_count(cli_room_id)>1 &&
            room_mgr.get_host_id(cli_mgr.get_room_id(idx))==idx
        ) this->start_game(cli_room_id);
        break;
    default: break;
    }
}

void GameServer::tcp_listen(){
    std::cout<<"call tcp_listen"<<std::endl;
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
        cli_mgr.check_state();
        room_mgr.check_state();
    }
}

void* GameServer::udp_listen(void *obj_ptr){
    std::cout<<"call udp_listen"<<std::endl;
    pthread_detach(pthread_self());

    auto &self=*(GameServer*)obj_ptr;
    struct sockaddr_in udp_addr;
    socklen_t len;

    while(true){
        len=sizeof(udp_addr);
        int n=recvfrom(self.udp_sock_fd,self.udp_recv_buffer,1024,0,(struct sockaddr*)&udp_addr,&len);
        if(n<0){
            std::cerr<<"recvfrom error"<<std::endl;
            exit(1);
        }
        if(self.udp_recv_buffer[n]=='\n') self.udp_recv_buffer[n]='\0';//test

        std::cout<<"udp recv: {"<<self.udp_recv_buffer<<"}"<<std::endl;
        InputStruct tmp(self.udp_recv_buffer);
        
        if(tmp.valid && self.cli_mgr.get_state(tmp.client_id)==ClientData::play){
            pthread_mutex_lock(self.input_buffer_mutex+tmp.client_id);
            self.input_buffer[tmp.client_id].push(tmp);//need mutex
            pthread_mutex_unlock(self.input_buffer_mutex+tmp.client_id);
        }
    }
    return NULL;
}

void GameServer::start_server(){
    pthread_t tid;
    pthread_create(&tid,NULL,GameServer::udp_listen,(void*)this);
    this->tcp_listen();
}

void *GameServer::game_loop(void *obj_ptr){
    pthread_detach(pthread_self());
    auto &self=*((StartParam*)obj_ptr)->obj_ptr;
    int room_id=((StartParam*)obj_ptr)->room_id;
    delete (StartParam*)obj_ptr;

    int player_count=self.room_mgr.player_count(room_id);
    int client_id_list[4], idx;
    idx=0;
    std::map<int,int> client_id2idx;
    for(auto &client_id:self.room_mgr.get_clients(room_id)){
        client_id_list[idx]=client_id;
        client_id2idx[client_id]=idx;
        ++idx;
    }

    //generate map
    std::vector<MapObject> staticObjects = generateMap(
        SCREEN_WIDTH,SCREEN_HEIGHT,
        self.room_mgr.get_map_seed(room_id)
    );
    std::vector<std::vector<MapObject>> grid(
        SCREEN_WIDTH,std::vector<MapObject>(SCREEN_HEIGHT)
    );
    for(auto &item:staticObjects){
        grid[item.getX()][item.getY()]=item;
    }

    std::string client_id_list_str[4];
    for(idx=0;idx<player_count;++idx){
        client_id_list_str[idx++]=id2str(client_id_list[idx]);
    }
    auto tanks=Tank::createTanks(
        self.room_mgr.player_count(room_id),
        client_id_list_str,
        SCREEN_WIDTH,SCREEN_HEIGHT
    );

    GameTimer timer(0.128);
    char udp_send_buffer[1024];
    
    bool loopRunning=true;
    while(loopRunning){
        if(!timer.shouldUpdate()) continue;
        for(idx=0;idx<player_count;++idx){
            int client_id=client_id_list[idx];
            //handle client input and update objects
            pthread_mutex_lock(self.input_buffer_mutex+client_id);
            auto &in_buffer=self.input_buffer[client_id];
            while(!in_buffer.empty()){
                switch(in_buffer.front().key){
                case 'w':
                    tanks[idx].setDirection(Direction::Up);
                    int nextY = tanks[idx].getY() - 1;
                    if (nextY > 0 && !tanks[idx].checkTankCollision(tanks[idx].getX(), nextY, staticObjects)){
                        tanks[idx].setY(nextY);
                    }
                    break;
                case 'a':
                    tanks[idx].setDirection(Direction::Left);
                    int nextX = tanks[idx].getX() - 1;
                    if (nextX > 0 && !tanks[idx].checkTankCollision(nextX, tanks[idx].getY(), staticObjects)){
                        tanks[idx].setX(nextX);
                    }
                    break;
                case 's':
                    tanks[idx].setDirection(Direction::Down);
                    int nextY = tanks[idx].getY() + 1;
                    if (nextY < SCREEN_HEIGHT - 1 && !tanks[idx].checkTankCollision(tanks[idx].getX(), nextY, staticObjects)){
                        tanks[idx].setY(nextY);
                    }
                    break;
                case 'd':
                    tanks[idx].setDirection(Direction::Right);
                    int nextX = tanks[idx].getX() + 1;
                    if (nextX < SCREEN_WIDTH - 1 && !tanks[idx].checkTankCollision(nextX, tanks[idx].getY(), staticObjects)){
                        tanks[idx].setX(nextX);
                    }
                    break;
                case ' ':
                    tanks[idx].fireBullet();
                    break;
                default: break;
                }
                //generate msg
                //[uf],client_id,x,y,direction,seq
                snprintf(udp_send_buffer,1024,"u,%s,%d,%d,%d,%d",
                    id2str(client_id),tanks[idx].getX(),tanks[idx].getY(),
                    tanks[idx].getDirection(),in_buffer.front().seq
                );
                if(in_buffer.front().key==' ') udp_send_buffer[0]='f';
                //send update to every client
                for(int i=0, len=strlen(udp_send_buffer);i<player_count;++i){
                    sendto(
                        self.udp_sock_fd,
                        udp_send_buffer,len,
                        0,(struct sockaddr*)(self.client_udp_addr+client_id_list[i]),
                        sizeof(self.client_udp_addr[client_id_list[i]])
                    );
                }
                in_buffer.pop();
            }
            pthread_mutex_unlock(self.input_buffer_mutex+client_id);
            
        }
    }

    return NULL;
}
