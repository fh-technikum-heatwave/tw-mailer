#include "Server.hpp"

Server::Server(int port, string mailName)
{
    PORT = port;
    mailDirectoryName = mailName;
}

void Server::run()
{
    createSocket();
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    handShake(); // In dieser Methode finden das listen und binden statt

    while (1)
    {
        cout << "Waiting for connections...\n"
             << endl;

        acceptConnection();
        printf("Client connected from %s:%d...\n",
               inet_ntoa(cliaddress.sin_addr),
               ntohs(cliaddress.sin_port));

        if ((pid = fork()) == 0)
        {
            close(create_socket);

            // Code: was hier passiert
            clientCommunication();

            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        close(client_socket);
    }

    close(create_socket);
}

void Server::clientCommunication()
{

    char buffer[BUF];
    int current_socket = client_socket;

    strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
    if (send(current_socket, buffer, strlen(buffer), 0) == -1)
    {
        cout << "send failed\n";
        exit(EXIT_FAILURE);
        // return NULL;
    }

    int size;
    do
    {
        receivemessage(buffer);
        string command = buffer; // Die Message muss aus dem Buffer rausgeschrieben werden, denn sie kann überschrieben werden
        cout << command + "\n"
             << endl;

    } while (strcmp(buffer, "quit") != 0 && !abortRequested);
}

void Server::handleCommands(string &command)
{

    if (command == "SEND")
    {
        
        return;
    }

    if (command == "LIST")
    {

        return;
    }

    if (command == "READ")
    {

        return;
    }

    if (command == "DEL")
    {
        return;
    }

    cout << "ungültiger Befehl\n";
}

void Server::receivemessage(char *buffer)
{
    int size;

    size = recv(client_socket, buffer, BUF - 1, 0);

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
    }

    if (size == 0)
    {
        printf("Client closed remote socket\n"); // ignore error
        // return;
        return;
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
}

void Server::acceptConnection()
{
    addrlen = sizeof(struct sockaddr_in);
    if ((client_socket = accept(create_socket,
                                (struct sockaddr *)&cliaddress,
                                &addrlen)) == -1)
    {
        if (abortRequested)
        {
            cout << "accept error after aborted" << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << "accept error" << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void Server::handShake()
{
    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(create_socket, 5) == -1)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
}

void Server::createSocket()
{
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cout << "Socket error\n"
             << endl;
        perror("Socket error"); // errno set by socket()
        exit(EXIT_FAILURE);
    }

    if (setsockopt(create_socket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuseValue,
                   sizeof(reuseValue)) == -1)
    {
        perror("set socket options - reuseAddr");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(create_socket,
                   SOL_SOCKET,
                   SO_REUSEPORT,
                   &reuseValue,
                   sizeof(reuseValue)) == -1)

    {
        perror("set socket options - reusePort");
        exit(EXIT_FAILURE);
    }
}