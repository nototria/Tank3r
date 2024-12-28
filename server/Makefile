FLAGS = -O2 -std=c++14

main: GameServer ClientManager RoomManager
	g++ $(FLAGS) main.cpp GameServer.o  ClientManager.o RoomManager.o -o main.out

GameServer: 
	g++ $(FLAGS) -c GameServer.cpp

ClientManager:
	g++ $(FLAGS) -c ClientManager.cpp

RoomManager:
	g++ $(FLAGS) -c RoomManager.cpp

clean:
	rm *.out
	rm *.o
