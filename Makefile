CC = g++
CFLAGS = -Wall -g
LDAP = -lldap -llber

all: server_main client_main

client_main: client_main.cpp client.cpp
	$(CC) -std=c++17 $(CFLAGS) -o client_main client_main.cpp client.cpp

Client: Client.hpp
	$(CC) -std=c++17 $(CFLAGS) -c client.cpp	

server_main: server_main.cpp server.cpp
	$(CC) -std=c++17 $(CFLAGS) -o server_main server_main.cpp server.cpp $(LDAP) 

Server: Server.hpp
	$(CC) -std=c++17 $(CFLAGS) -c server.cpp $(LDAP) 

clean:
	rm -f server_main client_main
