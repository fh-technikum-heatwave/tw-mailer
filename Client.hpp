#ifndef Client_HPP
#define CLIENT_HPP
#define BUF 1024

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>

using namespace std;

class Client
{
public:
    Client(int port, string ip);
    void run();

private:
    int abortRequested = 0;
    int create_socket = -1;
    int client_socket = -1;
    int PORT;
    string ipAdress;
    struct sockaddr_in address;
    int isQuit;
    char buffer[BUF], command[BUF];

    void createSocket();
    void receiveMessage(char *buffer);
    void commandHandle();
    void sendMessage(char *message);
    void sendCommand();
    void listCommand();
    void deleteCommand();
    void readCommand();
};
#endif