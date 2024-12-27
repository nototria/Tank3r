# Server
### connection
1. client connect to server with tcp\
if server is full, **server** closes the connection\
else **server** sends ```client_id``` to the **client**\
and set the **client** state to ```wait_id```
2. **client** send ```"client_id user_name"``` to server\
if ```client_id``` not match
3. client has two options
    - random match\
        send ```"join\n"```\
        join a random **room** that is waiting to start\
        if no room is available, create the **room**
    - join with **room** number\
        send ```"%.4d\n"```\
        if the **room** doesn't exist, create the **room**\
        if the status of the **room** is waiting, join the **room**\
        if the status of the **room** is playing, refuse
4. **server** reply the **room** number ```"%.4d\n"``` or ```"fail\n"```
5. if there are at least two **players** in the **room**,\
the first **player** can start the game (send ```"start\n"``` to server)

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
