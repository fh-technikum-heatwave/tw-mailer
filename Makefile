CC = g++
CFLAGS = -Wall -g
LDAP = -lldap -llber

all: server_main client_main

client_main: client_main.cpp client.cpp
	$(CC) -std=c++17 $(CFLAGS) -o client_main client_main.cpp Client.cpp

Client: client.hpp
	$(CC) -std=c++17 $(CFLAGS) -c Client.cpp	

server_main: server_main.cpp server.cpp
	$(CC) -std=c++17 $(CFLAGS) -o server_main server_main.cpp Server.cpp $(LDAP) 

Server: server.hpp
	$(CC) -std=c++17 $(CFLAGS) -c Server.cpp $(LDAP) 

clean:
	rm -f server_main client_main
