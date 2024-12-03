# Server
### connection
1. client connect to server with tcp
2. client send user id to server, "id %s\n"
3. client send udp listen port, "udp %d\n"
4. server send uuid to client and udp listen port
"uuid %s\nudp %d\n"
5. client use uuid as identity

### game start
1. server send static game data to new client
"map_seed %d\n"
2. when a new player connect to server
"uuid %s id %s\n"
3. client generate map
4. server send game status
    - player: send whenever it updates
    game tick, uuid, position, direction, health, fire
    "p t %d uuid %s p %d %d d %d h %d f %b\n"
    - bullet: send whenever it updates
    game tick, from uuid, position, direction, health(only 1 or 0)
    "b t %d uuid %s p %d %d d %d h %b\n"
