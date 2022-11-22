#ifndef SERVER_HPP
#define SERVER_HPP
#define BUF 1024
#include <sys/stat.h>
#include <string>
#include <sys/socket.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
class Server
{

private:
    int abortRequested = 0;
    int create_socket = -1;
    int client_socket = -1;
    struct stat st = {0};
    int PORT;
    string mailDirectoryName;
    socklen_t addrlen;
    int reuseValue = 1;
    struct sockaddr_in address, cliaddress;
    pid_t pid;
    void createSocket();
    void handShake();
    void acceptConnection();
    void clientCommunication();
    void receivemessage(char *buffer);
    void handleCommands(string &command);


public:
    Server(int port, string mailName);
    void run();
};

#endif