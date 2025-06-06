#ifndef PARAMETERS_H
#define PARAMETERS_H

#define SERV_PORT 9999
#define UDP_PORT 10000
#define SERV_IP "140.113.66.206"
#define MAX_CLIENTS 1000
#define MAX_ROOMS 10000
#define ROOM_MAX_CLIENTS 4
#define COMMAND_SEP ','

#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 40

#define COLOR_GRAY 8

const int statusBlockWidth = 25;
const int statusBlockHeight = 10;

enum class Direction {Right, Left, Up, Down};
enum class GameState {TitleScreen, UsernameInput, RoomMenu, InRoom, GameLoop, WinScreen, LoseScreen, TieScreen};
enum class MapObjectType {empty, wall,water,bullet,tank};

#endif
