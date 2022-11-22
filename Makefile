CC = g++
CFLAGS = -Wall -g

all: main client

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

main:Main.cpp Server.cpp
	$(CC) $(CFLAGS) -o main main.cpp Server.cpp

Server: Server.hpp
	$(CC) $(CFLAGS) -c Server.cpp

clean:
	rm -f main
