#include"RoomManager.hpp"
#include"../shared/GameUtils.hpp"
#include<iostream>
#include<vector>

inline int RoomManager::RoomData::player_count() const{
    return this->client_id_set.size();
}

inline bool RoomManager::RoomData::is_full() const{
    return this->client_id_set.size()>=ROOM_MAX_CLIENTS;
}

RoomManager::RoomManager(){
    //init room data list
    for(int i=0;i<MAX_ROOMS;++i){
        this->room_data_list[i].id=i;
        this->room_data_list[i].state=RoomData::inactive;
        this->room_data_list[i].host_id=-1;
    }
}

bool RoomManager::join_room(const int client_id, int &room_id){
    std::cout<<"call join room"<<std::endl;
    if(room_id>=MAX_ROOMS) return false;
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
        else return false;
    }

    //get room_obj
    auto &room_obj=room_data_list[room_id];
    
    //if room is full
    if(room_obj.is_full()) return false;
    
    room_obj.client_id_set.insert(client_id);

    //if this is the first player of the room
    if(room_obj.player_count()==1){
        room_obj.state=RoomData::wait;
        room_obj.host_id=client_id;
    }
    return true;
}

bool RoomManager::exit_room(const int client_id, const int room_id){
    if(room_id<0) return 0;
    auto &room_obj=room_data_list[room_id];

    room_obj.client_id_set.erase(client_id);

    //if the room become empty
    if(room_obj.player_count()==0){
        room_obj.state=RoomData::inactive;
        room_obj.host_id=-1;
        return 0;
    }
    //if the room host exit
    if(room_obj.host_id==client_id){
        room_obj.host_id=*(room_obj.client_id_set.begin());
        return 1;
    }
    return 0;
}

void RoomManager::start_game(const int room_id){
    auto &room_obj=this->room_data_list[room_id];
    room_obj.state=RoomData::play;
    //TODO : create threads to simulate the game and listen to UDP message
}

int RoomManager::get_host_id(const int room_id){
    return room_data_list[room_id].host_id;
}

int RoomManager::player_count(const int room_id){
    return room_data_list[room_id].player_count();
}

const std::set<int>& RoomManager::get_clients(const int room_id){
    return room_data_list[room_id].client_id_set;
}

void RoomManager::check_state(){
    std::cout<<"RoomManager{\n";
    bool flag=false;
    for(int i=0;i<MAX_ROOMS;++i){
        auto &room_obj=room_data_list[i];
        if(room_obj.state==RoomData::inactive) continue;
        if(flag) std::cout<<",\n";
        flag=true;
        std::cout<<"\t{";
        std::cout<<"room_id : "<<id2str(room_obj.id);
        std::cout<<", state : ";
        switch (room_obj.state){
        case RoomData::wait: std::cout<<"wait";break;
        case RoomData::play: std::cout<<"play";break;
        default: break;
        }

        std::cout<<", host_id : "<<id2str(room_obj.host_id);
        
        std::cout<<", clients: {";
        bool flag0=false;
        for(auto &item:room_obj.client_id_set){
            if(flag0) std::cout<<", ";
            flag0=true;
            std::cout<<id2str(item);
        }
        std::cout<<"}";
        
        std::cout<<"}";
    }
    std::cout<<"\n}\n";
    std::cout.flush();
}
