#ifndef RoomManager_H
#define RoomManager_H

#include<set>
#include"../shared/GameParameters.h"
class RoomManager{
    struct RoomData{
        enum State{inactive, wait, play} state;
        std::set<int> client_id_set;
        int host_id;
        int id;
        unsigned int map_seed;
        inline int player_count() const;
        inline bool is_full() const;
    };
    RoomData room_data_list[MAX_ROOMS];
public:
    RoomManager();
    //return true if join succeful
    //room_id will set to one it joined
    bool join_room(const int client_id, int &room_id);
    //return true if host change
    bool exit_room(const int, const int);
    void set_map_seed(const int, const unsigned long);
    //start create threads for the game and change state
    void start_game(const int);
    int get_host_id(const int) const;
    const unsigned long get_map_seed(const int) const;
    int player_count(const int) const;
    const std::set<int>& get_clients(const int) const;
    void check_state();
};

#endif
