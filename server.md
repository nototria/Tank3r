# Server
### connection
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
5. send ```"exit,room_id\n"``` to exit the room
6. host can send ```"start,room_id\n"``` to start the game
7. server send ```"start,player_count\nclient_id,user_name\n...seed,seed_number\n"``` to every client

### game start
```obj_id``` : 4-digit number
```type``` : bullet, tank
```direction``` : 0, 1, 2, 3
```health``` : 0~20
```key``` : valid input key
#### server send
- create new object
```"n,obj_id,client_id,type,x,y,direction\n"```
- update object
```"u,obj_id,x,y,direction\n"```
- delete object
```"d,obj_id"```
- update health
```"h,obj_id,health"```
#### client send
- input
```"key,client_id"```
