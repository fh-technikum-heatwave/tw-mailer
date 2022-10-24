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
#include <string.h>

#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

using namespace std;

#define BUF 1024

int abortRequested = 0;
int create_socket = -1;
int new_socket = -1;
struct stat st = {0};

void clientCommunication(int &data);
void signalHandler(int sig);
void sendMessage(char *message, int socket);
string mailDirectoryName;

char *receiveMessage(char *buffer, int current_socket);

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

    cout << "Create Directory \n";
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
        clientCommunication(new_socket); // returnValue can be ignored
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

void createReceiverDirectory(string &directoryName)
{

    string temp = mailDirectoryName + "/" + directoryName;

    if (stat(temp.c_str(), &st) == -1)
    {
        int ok = mkdir(temp.c_str(), 0700);

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

void receiveMailData(char *buffer, int current_socket)
{
    senderInputData.sender = receiveMessage(buffer, current_socket);
    senderInputData.receiver = receiveMessage(buffer, current_socket);
    senderInputData.subject = receiveMessage(buffer, current_socket);
    while (true)
    {
        char *message = receiveMessage(buffer, current_socket);

        if (strcmp(message, ".") == 0)
        {
            break;
        }

        senderInputData.message += message;
        senderInputData.message += "\n";
    }

    saveMessage();
    sendMessage("OK", current_socket);
}

vector<string> getUserMessages(string username, int socket)
{
    string tempPath = mailDirectoryName + "/" + username + "/";
    cout << tempPath;
    vector<string> filenames;

    int foundPath = stat(tempPath.c_str(), &st);

    // cout << foundPath << "\n";
    if (foundPath == -1)
    {
        cout << " Line 260" << endl;
        sendMessage("0", socket);
    }
    else if (foundPath == 0)
    {
        cout << tempPath + "\n";

        for (const auto &entry : fs::directory_iterator(tempPath))
            filenames.push_back(entry.path().stem());
            //cout << entry.path().stem() << endl;
    }

    return filenames;
}

void handleCommands(char buffer[BUF], int current_socket)
{
    if (strcmp(buffer, "SEND") == 0)
    {
        receiveMailData(buffer, current_socket);
    }
    else if (strcmp(buffer, "LIST") == 0)
    {
        char *username = receiveMessage(buffer, current_socket);
        vector<string> filenames = getUserMessages(username, current_socket);
       
        char* count = (char*)to_string(filenames.size()).c_str();

        sendMessage(count, current_socket);
        for (size_t i = 0; i < filenames.size(); i++)
        {
            char* response = receiveMessage(buffer, current_socket);
            if(strcmp(response, "OK") == 0) {
                sendMessage((char *)filenames[i].c_str(), current_socket);
            }
        }
    }
    else if (strcmp(buffer, "READ") == 0)
    {
    }
    else if (strcmp(buffer, "DEL") == 0)
    {
        const char* username = receiveMessage(buffer, current_socket);
        cout << username << " <-- NAME\n" << endl; 
        // TODO: Change to mail number
        const char* mail_subject = receiveMessage(buffer, current_socket);

        printf("\nUsename: %s  Subject: %s\n", username, mail_subject);
        cout << username << " <-- NAME2\n" << endl; 

        string tempPath = mailDirectoryName + "/" + username + "/";
        cout << tempPath + "\n";
        vector<string> filenames;

        int foundPath = stat(tempPath.c_str(), &st);

        if (foundPath == -1)
        {
            sendMessage("ERR", current_socket);
        }
        else if (foundPath == 0)
        {
            for (const auto &entry : fs::directory_iterator(tempPath)) {
                if (strcmp(mail_subject, entry.path().stem().c_str()) == 0)
                {
                    delete(&entry);
                    sendMessage("OK", current_socket);
                    break;
                }
                
            }
        }
    }
    else
    {
        cout << "ungültiger Befehl\n";
    }
}

char *receiveMessage(char *buffer, int current_socket)
{

    int size;

    cout << "Current Socket " + current_socket;

    size = recv(current_socket, buffer, BUF - 1, 0);

    if (size == -1)
    {
        if (abortRequested)
        {
            perror("recv error after aborted");
        }
        else
        {
            cout << " Line 293 \n";
            perror("recv error");
        }

        // return;
    }

    if (size == 0)
    {
        printf("Client closed remote socket\n"); // ignore error
        // return;
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

    return buffer;
}

void sendMessage(char *buffer, int socket)
{
    cout << "Line 388" << endl;

    if (send(socket, buffer, strlen(buffer), 0) == -1)
    {
        perror("send answer failed");
        // return NULL;
    }
}

void clientCommunication(int &data)
{
    char buffer[BUF];
    int current_socket = data;

    strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
    if (send(current_socket, buffer, strlen(buffer), 0) == -1)
    {
        perror("send failed");
        // return NULL;
    }

    int size;
    do
    {

        char *command = receiveMessage(buffer, current_socket);

        printf("Message received: %s\n", buffer);

        cout << command;

        handleCommands(command, current_socket);

        // cout<<"Buffer " << buffer << "\n";

    } while (strcmp(buffer, "quit") != 0 && !abortRequested);

    if (current_socket != -1)
    {
        if (shutdown(current_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown new_socket");
        }
        if (close(current_socket) == -1)
        {
            perror("close new_socket");
        }
        current_socket = -1;
    }

    // return NULL;
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
