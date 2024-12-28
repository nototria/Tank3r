# Server
### connection
```*_id``` is a 4-digit number
1. client connect to server with tcp\
if server is full, **server** closes the connection\
else **server** sends ```client_id``` to the **client**\
and set the **client** state to ```wait_name```
2. **client** send ```"client_id,user_name"``` to server\
**user_name** cannot contain ```','```
3. **client** has two options
    - random match\
        send ```"join,-1\n"```\
        join a random **room**
    - join with **room** number\
        send ```"join,room_id\n"```\
        join with **room** number
4. **server** reply ```"join,room_id\n"``` or ```"fail\n"```
5. **server** send ```"host,room_id"``` to **host**\
if the host exits the room, new host will receive this message
6. send ```"exit\n"``` to exit the room
7. host can send ```"start,room_id\n"``` to start the game
8. server send ```"start,player_count\nclient_id,user_name\n..."``` to every client

### game start
1. server send static game data to new client
"map_seed %d\n"
2. when a new player connect to server
"uuid %s id %s\n"
3. client generate map
4. server send game status
    - player:
        - uuid, position, direction, health, fire, fire_cd
        - "p u %s p %d %d d %d h %d f %b %d"
    - bullet: 
        - uuid, owner_id, position, direction, health(only 1 or 0)
        - "b u %s p %d %d d %d h %b o %s"
