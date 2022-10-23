all: server client

server:server.cpp
	g++ -std=c++17 -Wall -g -o server server.cpp

client:client.cpp
	g++ -Wall -g -o client client.cpp
