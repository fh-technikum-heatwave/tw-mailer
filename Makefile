CC = g++
CFLAGS = -Wall -g

all: server_main client_main

client_main:client_main.cpp Client.cpp
	$(CC) -std=c++17 $(CFLAGS) -o client_main client_main.cpp Client.cpp

Client: Client.hpp
	$(CC) -std=c++17 $(CFLAGS) -c Client.cpp	

server_main:server_main.cpp Server.cpp
	$(CC) -std=c++17 $(CFLAGS) -o server_main server_main.cpp Server.cpp

Server: Server.hpp
	$(CC) -std=c++17 $(CFLAGS) -c Server.cpp

clean:
	rm -f server_main client_main
