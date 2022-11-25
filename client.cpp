#include "Client.hpp"

Client::Client(int port, string ip)
{
    PORT = port;
    ipAdress = ip;
}

void Client::run()
{
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address)); // init storage with 0
    address.sin_family = AF_INET;         // IPv4
    inet_aton(ipAdress.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);

    if (connect(create_socket,
                (struct sockaddr *)&address,
                sizeof(address)) == -1)
    {
        // https://man7.org/linux/man-pages/man3/perror.3.html
        perror("Connect error - no server available");
        exit(EXIT_FAILURE);
    }

    // ignore return value of printf
    printf("Connection with server (%s) established\n",
           inet_ntoa(address.sin_addr));

    receiveMessage(buffer);
    printf(buffer);

    commandHandle();

    if (create_socket != -1)
    {
        if (shutdown(create_socket, SHUT_RDWR) == -1)
        {
            // invalid in case the server is gone already
            perror("shutdown create_socket");
        }
        if (close(create_socket) == -1)
        {
            perror("close create_socket");
        }
        create_socket = -1;
    }

    exit(EXIT_SUCCESS);
}

void Client::commandHandle()
{
    do
    {
        printf(">> ");
        fgets(command, BUF, stdin);
        isQuit = strcmp(command, "QUIT\n") == 0;
        if (isQuit)
        {
            break;
        }
        sendMessage(command);
        // this receiveMessage is neccessary, it tells if the user is authenticated or not
        receiveMessage(buffer);
        string response = buffer;

        if (response == "401")
        {
            cout << ">> You are not authenticated, please use Command: LOGIN\n";
            continue;
        }

        if (strcmp(command, "SEND\n") == 0)
        {
            sendCommand();
        }
        else if (strcmp(command, "LIST\n") == 0)
        {
            listCommand();
        }
        else if (strcmp(command, "READ\n") == 0)
        {
            readCommand();
        }
        else if (strcmp(command, "DEL\n") == 0)
        {
            deleteCommand();
        }
        else if (strcmp(command, "LOGIN\n") == 0)
        {
            login();
        }

        // TODO: HANDLE UNWANTED INPUT

        isQuit = strcmp(command, "QUIT\n") == 0;

    } while (!isQuit);
}

void Client::deleteCommand()
{
    printf("<Message-Number> ");
    // Read Message Number
    fgets(buffer, BUF, stdin);
    sendMessage(buffer);

    receiveMessage(buffer);
    printf("<< %s\n", buffer);
}

void Client::login()
{
    printf("<Username> ");
    fgets(buffer, BUF, stdin);
    sendMessage(buffer);
    char *password = getpass("<Password> ");
    sendMessage(password);
    receiveMessage(buffer);
    string message = buffer;

    printf("<< %s\n", buffer);
}

void Client::listCommand()
{
    receiveMessage(buffer);
    char *mcs = buffer;
    int mail_count = stoi(mcs);
    printf("<< %d\n", mail_count);
    if (mail_count > 0)
    {
        for (int i = 0; i < mail_count; i++)
        {
            sendMessage((char *)"OK");
            receiveMessage(buffer);
            printf("<< %s\n", buffer);
        }
    }
}

void Client::sendCommand()
{
    const char *send_fields[2] = {"<RECEIVER> ", "<SUBJECT> "};
    const int send_fields_length = (sizeof(send_fields) / sizeof(*send_fields));
    for (int i = 0; i < send_fields_length; i++)
    {
        printf("%s", send_fields[i]);
        fgets(buffer, BUF, stdin);
        if (i == send_fields_length - 1 && strlen(buffer) > 80)
        {
            printf("Subject can not be longer than 80 chars");
            return;
        }
        else
        {
            sendMessage(buffer);
        }
    }

    printf("<MESSAGE> ");
    do
    {
        fgets(buffer, BUF, stdin);
        sendMessage(buffer);
    } while (strcmp(buffer, ".\n") != 0);

    receiveMessage(buffer);
    printf("<< %s\n", buffer);
}

void Client::readCommand()
{

    printf("<Message-Number> ");
    // Read Message Number
    fgets(buffer, BUF, stdin);
    sendMessage(buffer);

    receiveMessage(buffer);
    string ok = buffer;
    printf("<< %s\n", buffer);
    if (ok == "OK")
    {
        sendMessage("OK");
        receiveMessage(buffer);

        printf("<< %s\n", buffer);
    }
}

void Client::receiveMessage(char *buffer)
{
    int size = recv(create_socket, buffer, BUF - 1, 0);
    if (size == -1)
    {
        perror("recv error");
    }
    else if (size == 0)
    {
        printf("Server closed remote socket\n"); // ignore error
    }

    buffer[size] = '\0';
}

void Client::sendMessage(char *message)
{
    if ((send(create_socket, message, strlen(message), 0)) == -1)
    {
        perror("send error");
    }
}
