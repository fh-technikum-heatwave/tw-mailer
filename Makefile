CC = g++
CFLAGS = -Wall -g
LDAP = -lldap -llber

all: server_main client_main

client_main:client_main.cpp Client.cpp
	$(CC) -std=c++17 $(CFLAGS) -o client_main client_main.cpp Client.cpp

Client: Client.hpp
	$(CC) -std=c++17 $(CFLAGS) -c Client.cpp	

server_main:server_main.cpp Server.cpp
	$(CC) -std=c++17 $(CFLAGS) -o server_main server_main.cpp Server.cpp $(LDAP) 

Server: Server.hpp
	$(CC) -std=c++17 $(CFLAGS) -c Server.cpp $(LDAP) 

clean:
	rm -f server_main client_main
