#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <dirent.h>
#include <errno.h>
#include <mutex>

using namespace std;

#define BUF 1024

int abortRequested = 0;
int create_socket = -1;
int new_socket = -1;
struct stat st = {0};

void *clientCommunication(void *data);
void signalHandler(int sig);
string mailDirectoryName;

int main(int argc, char *argv[])
{

    string port;
    int PORT;

    if (optind < argc)
    {
        port = argv[optind++];
        PORT = stoi(port);
        if (optind < argc)
            mailDirectoryName = argv[optind++];
        else
        {
            cout << "<mail-spool-directoryname> wurde nicht eingeben\n";
            return -1;
        }
    }
    else
    {
        cout << "Es wurden keine Parameter eingegeben\n";
        return -1;
    }

    cout << "CreateDirectory";
    // int result = mkdir(mailDirectoryName.c_str(), 0700);

    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;
    int reuseValue = 1;

    if (signal(SIGINT, signalHandler) == SIG_ERR)
    {
        perror("signal can not be registered");
        return EXIT_FAILURE;
    }

    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error"); // errno set by socket()
        return EXIT_FAILURE;
    }

    if (setsockopt(create_socket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuseValue,
                   sizeof(reuseValue)) == -1)
    {
        perror("set socket options - reuseAddr");
        return EXIT_FAILURE;
    }

    if (setsockopt(create_socket,
                   SOL_SOCKET,
                   SO_REUSEPORT,
                   &reuseValue,
                   sizeof(reuseValue)) == -1)
    {
        perror("set socket options - reusePort");
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

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
        clientCommunication(&new_socket); // returnValue can be ignored
        new_socket = -1;
    }

    if (create_socket != -1)
    {
        if (shutdown(create_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown create_socket");
        }
        if (close(create_socket) == -1)
        {
            perror("close create_socket");
        }
        create_socket = -1;
    }

    return EXIT_SUCCESS;
}

bool senderInputs = false;
int senderInputCounter = 0;

struct
{
    string sender;
    string receiver;
    string subject;
    string message;

} senderInputData;

void Send(int &inputCounter, char buffer[BUF])
{

    if (inputCounter == 1)
    {
        senderInputData.sender = buffer;
    }
    else if (inputCounter == 2)
    {
        senderInputData.receiver = buffer;
    }
    else if (inputCounter == 3)
    {
        senderInputData.subject = buffer;
    }
    else if (inputCounter > 3)
    {
        senderInputData.message += buffer;
    }
}

void createReceiverDirectory(string &directoryName)
{

    string temp = mailDirectoryName + "/" + directoryName;

    if (stat(temp.c_str(), &st) == -1)
    {
        int ok = mkdir(temp.c_str(), 0777);

        if (!ok)
        {
            cout << "created\n";
        }
    }
}

mutex mtx;

void saveMessage()
{
    FILE *fp;

    createReceiverDirectory(senderInputData.receiver);

    string temp = mailDirectoryName + "/" + senderInputData.receiver + "/" + senderInputData.subject;

    cout << "in File Save";


    mtx.lock();
    fp = fopen(temp.c_str(), "w");

    for (int i = 0; i < senderInputData.message.size(); i++)
    {
        /* write to file using fputc() function */
        fputc(senderInputData.message[i], fp);
    }

    fclose(fp);
    mtx.unlock();

    // fclose(fp);
}

void handleCommands(char buffer[BUF], int current_socket)
{

    if (senderInputs)
    {
        if (strcmp(buffer, ".") == 0)
        {
            senderInputs = false;
            senderInputCounter = 0;
            cout << senderInputData.message << " \n";

            saveMessage();

            if (send(current_socket, "OK", 3, 0) == -1)
            {
                perror("send answer failed");
                return;
            }
            return;
        }

        senderInputCounter++;

        Send(senderInputCounter, buffer);

        return;
    }

    // cout << buffer;

    if (strcmp(buffer, "SEND") == 0)
    {
        senderInputData.message = "";
        senderInputs = true;
    }
    else if (strcmp(buffer, "LIST") == 0)
    {
    }
    else if (strcmp(buffer, "READ") == 0)
    {
    }
    else if (strcmp(buffer, "DEL") == 0)
    {
    }
    else
    {
        cout << "ungültiger Befehl\n";
    }
}

void *clientCommunication(void *data)
{
    char buffer[BUF];
    int size;
    int *current_socket = (int *)data;

    strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
    if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
    {

        perror("send failed");
        return NULL;
    }

    do
    {

        size = recv(*current_socket, buffer, BUF - 1, 0);

        if (size == -1)
        {
            if (abortRequested)
            {
                perror("recv error after aborted");
            }
            else
            {
                perror("recv error");
            }
            break;
        }

        if (size == 0)
        {
            printf("Client closed remote socket\n"); // ignore error
            break;
        }

        // remove ugly debug message, because of the sent newline of client
        if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
        {
            size -= 2;
        }
        else if (buffer[size - 1] == '\n')
        {
            --size;
        }

        buffer[size] = '\0';
        printf("Message received: %s\n", buffer);

        handleCommands(buffer, *current_socket);

        // cout<<"Buffer " << buffer << "\n";

        // if (send(*current_socket, "OK", 3, 0) == -1)
        // {
        //     perror("send answer failed");
        //     return NULL;
        // }
    } while (strcmp(buffer, "quit") != 0 && !abortRequested);

    if (*current_socket != -1)
    {
        if (shutdown(*current_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown new_socket");
        }
        if (close(*current_socket) == -1)
        {
            perror("close new_socket");
        }
        *current_socket = -1;
    }

    return NULL;
}

void signalHandler(int sig)
{
    if (sig == SIGINT)
    {
        printf("abort Requested... \n"); // ignore error
        abortRequested = 1;

        if (new_socket != -1)
        {
            if (shutdown(new_socket, SHUT_RDWR) == -1)
            {
                perror("shutdown new_socket");
            }
            if (close(new_socket) == -1)
            {
                perror("close new_socket");
            }
            new_socket = -1;
        }

        if (create_socket != -1)
        {
            if (shutdown(create_socket, SHUT_RDWR) == -1)
            {
                perror("shutdown create_socket");
            }
            if (close(create_socket) == -1)
            {
                perror("close create_socket");
            }
            create_socket = -1;
        }
    }
    else
    {
        exit(sig);
    }
}
