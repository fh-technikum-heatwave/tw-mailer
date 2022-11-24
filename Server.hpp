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
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <mutex>
#include <signal.h>
#include <vector>
#include <signal.h>
#include <ldap.h>
#include <ldap_cdefs.h>
#include <ctime>
 

namespace fs = std::filesystem;
using namespace std;
class Server
{

private:
    int abortRequested = 0;
    int create_socket = -1;
    int client_socket = -1;
    struct stat st = {0};
    int PORT;
    string mailDirectoryName = "";
    socklen_t addrlen;
    string username = "";
    int reuseValue = 1;
    struct sockaddr_in address, cliaddress;
    pid_t pid;
    bool authenticated = false;
    int loginAttempts = 3;

    void createSocket();
    void handShake();
    void acceptConnection();
    void clientCommunication();
    void receivemessage(char *buffer);
    void handleCommands(char *command);
    void receiveMail(char *buffer);
    void saveMessage(string &receiver, string &subject, string &message);
    void sendMessage(char *buffer);
    void createReceiverDirectory(string &directoryName);
    void allMails(char *buffer);
    vector<string> getUserMessages();
    void readMail(char *buffer);
    auto read_file(std::string_view path) -> std::string;
    void deleteMail(char *buffer);
    void login(char *buffer);
    bool ldapAuth(string user, const string password);
    bool isBlackListed();

public:
    Server(int port, string mailName);
    void run();
};

#endif