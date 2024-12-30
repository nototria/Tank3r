FLAGS = -O2 -std=c++14

main.out: GameServer.o ClientManager.o RoomManager.o
	g++ $(FLAGS) main.cpp GameServer.o  ClientManager.o RoomManager.o -o main.out

GameServer.o: GameServer.cpp GameServer.hpp
	g++ $(FLAGS) -c GameServer.cpp

ClientManager.o: ClientManager.cpp ClientManager.hpp
	g++ $(FLAGS) -c ClientManager.cpp

RoomManager.o: RoomManager.cpp RoomManager.hpp
	g++ $(FLAGS) -c RoomManager.cpp

.PHONY: clean
clean:
	-rm *.out
	-rm *.o
