#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <typeinfo>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// socket includes
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{

    string port;
    string mailDirectoryName;
    if (optind < argc)
    {
        port = argv[optind++];
        if (optind < argc)
            mailDirectoryName = argv[optind++];
        else
            cout << "<mail-spool-directoryname> wurde nicht eingeben\n";
    }
    else
    {
        cout << "Es wurden keine Parameter eingegeben\n";
    }

    int create_socket;

    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error"); // errno set by socket()
        return EXIT_FAILURE;
    }

    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;

    // memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind error");
        return EXIT_FAILURE;
    }

    if (listen(create_socket, 5) == -1)
    {
        perror("listen error");
        return EXIT_FAILURE;
    }

    int abortRequested = 0;
    // int create_socket = -1; 
    int new_socket = -1;

    while (!abortRequested)
    {

        printf("Waiting for connections...\n");

        addrlen = sizeof(struct sockaddr_in);
        if ((new_socket = accept(create_socket,
                                 (struct sockaddr *)&cliaddress,
                                 &addrlen)) == -1)
        {
            if (abortRequested)
            {
                perror("accept error after aborted");
            }
            else
            {
                perror("accept error");
            }
            break;
        }


        printf("Client connected from %s:%d...\n",
               inet_ntoa(cliaddress.sin_addr),
               ntohs(cliaddress.sin_port));
        // clientCommunication(&new_socket); // returnValue can be ignored
        new_socket = -1;
    }
    return 0;
}