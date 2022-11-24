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
        handleCommands(buffer);

    } while (strcmp(buffer, "quit") != 0 && !abortRequested);
}

void Server::handleCommands(char *buffer)
{

    string command = buffer;
    if (authenticated == false)
    {
        if (command == "LOGIN")
        {
            sendMessage("OK");
            login(buffer);
        }
        else
        {
            sendMessage("401"); // unauthorized User
        }

        return;
    }

    if (command == "SEND")
    {
        sendMessage("OK");
        receiveMail(buffer);

        return;
    }

    if (command == "LIST")
    {
        sendMessage("OK");
        allMails(buffer);
        return;
    }

    if (command == "READ")
    {
        sendMessage("OK");
        readMail(buffer);
        return;
    }

    if (command == "DEL")
    {
        sendMessage("OK");
        deleteMail(buffer);
        return;
    }

    cout << "ungültiger Befehl\n";
    sendMessage("invalid Command");
}

void Server::login(char *buffer)
{

    if (loginAttempts == 0)
    {

        return;
    }
    receivemessage(buffer);
    string user = buffer;

    receivemessage(buffer);
    string password = buffer;

    bool isAuth = ldapAuth(user, password);

    if (!isAuth)
    {
        loginAttempts--;
        sendMessage("ERR");
    }
    else
    {
        loginAttempts = 3;
        authenticated = true;
        username = user;
        sendMessage("OK");
    }
}

bool Server::isBlackListed()
{
    string ipAdress = inet_ntoa(cliaddress.sin_addr);
}

void Server::deleteMail(char *buffer)
{

    string tempPath = mailDirectoryName + "/" + username + "/";

    // TODO: Change to mail number
    receivemessage(buffer);
    string mail_subject = buffer;

    vector<string> filenames;

    int foundPath = stat(tempPath.c_str(), &st);

    if (foundPath == -1)
    {
        sendMessage("ERR");
    }
    else if (foundPath == 0)
    {
        for (const auto &entry : fs::directory_iterator(tempPath))
        {
            if (mail_subject == entry.path().stem().c_str())
            {
                remove(entry.path());
                sendMessage("OK");
                break;
            }
        }
    }
}

void Server::allMails(char *buffer)
{

    vector<string> filenames = getUserMessages();
    char *count = (char *)to_string(filenames.size()).c_str();
    sendMessage(count);

    for (size_t i = 0; i < filenames.size(); i++)
    {
        receivemessage(buffer);
        string response = buffer;

        cout << response;

        if (response == "OK")
        {
            sendMessage((char *)filenames[i].c_str());
        }
    }
}

void Server::readMail(char *buffer)
{
    string tempPath = mailDirectoryName + "/" + username + "/";

    // TODO: Change to mail number
    receivemessage(buffer);
    string mail_subject = buffer;

    int foundPath = stat(tempPath.c_str(), &st);
    cout << tempPath << endl;
    if (foundPath == -1)
    {
        cout << "ISNDIE" << endl;
        sendMessage("ERR");
    }
    else if (foundPath == 0)
    {
        tempPath += mail_subject;

        string output = read_file(tempPath.c_str());

        if (output.length() == 0)
        {
            sendMessage("ERR");
            return;
        }

        sendMessage((char *)"OK");
        receivemessage(buffer);
        string ok = buffer;

        if (ok == "OK")
        {
            sendMessage((char *)output.c_str());
        }
        else
        {
            sendMessage("ERR");
        }
    }
}

void Server::receiveMail(char *buffer)
{

    receivemessage(buffer);
    string receiver = buffer;

    receivemessage(buffer);
    string subject = buffer;

    string message = "";

    message = "Nachricht von: " + username + "\n";
    message += "SUBJECT: " + subject + "\n";

    while (true)
    {
        receivemessage(buffer);
        string msg = buffer;

        if (msg == ".")
        {
            break;
        }

        message += msg;
        message += "\n";
    }

    saveMessage(receiver, subject, message);
    sendMessage("OK");
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

void Server::saveMessage(string &receiver, string &subject, string &message)
{
    FILE *fp;
    mutex mtx;

    createReceiverDirectory(receiver);

    std::time_t result = std::time(nullptr);
    string temp = mailDirectoryName + "/" + receiver + "/" + subject;

    cout << "in File Save";

    mtx.lock();
    fp = fopen(temp.c_str(), "w");

    for (int i = 0; i < message.size(); i++)
    {
        /* write to file using fputc() function */
        fputc(message[i], fp);
    }

    fclose(fp);
    mtx.unlock();
}

void Server::createReceiverDirectory(string &directoryName)
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

void Server::sendMessage(char *buffer)
{

    if (send(client_socket, buffer, strlen(buffer), 0) == -1)
    {
        perror("send answer failed");
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

vector<string> Server::getUserMessages()
{
    string tempPath = mailDirectoryName + "/" + username + "/";
    vector<string> filenames;

    int foundPath = stat(tempPath.c_str(), &st);

    if (foundPath == -1)
    {
        sendMessage("0");
    }
    else if (foundPath == 0)
    {
        for (const auto &entry : fs::directory_iterator(tempPath))
            filenames.push_back(entry.path().stem());
    }

    return filenames;
}

bool Server::ldapAuth(string username, string password)
{
    LDAP *ld;
    char *dn;
    int version, rc;

    string tempRoot = "uid=" + username + ",ou=People,dc=technikum-wien,dc=at";
    const char *root_dn = tempRoot.c_str();

    string HOSTNAME = "ldap://ldap.technikum-wien.at:389";
    int ldapPort = 389;

    BerValue *servercredp;
    BerValue cred;

    cred.bv_val = (char *)password.c_str();
    cred.bv_len = strlen(password.c_str());

    version = LDAP_VERSION3;

    printf("Connecting %s in port %d...\n\n", HOSTNAME.c_str(), ldapPort);

    rc = ldap_initialize(&ld, HOSTNAME.c_str());

    if ((rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version)) != LDAP_SUCCESS)
    {
        printf("ldap_set_option(PROTOCOL_VERSION): %s", ldap_err2string(rc));
        fflush(stdout);
        ldap_unbind_ext_s(ld, NULL, NULL);
        exit(EXIT_FAILURE);
    }

    // if ((rc = ldap_start_tls_s(ld,NULL,NULL)) != LDAP_SUCCESS)
    // {
    //     printf("ldap_start_tls_s(): %s", ldap_err2string(rc));
    //     fflush(stdout);
    //     ldap_unbind_ext_s(ld, NULL, NULL);
    //     return false;
    // }

    if (rc != LDAP_SUCCESS)
    {
        printf("Error !");
    }

    rc = ldap_sasl_bind_s(ld, root_dn, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &servercredp);

    if (rc != LDAP_SUCCESS)
    {

        fprintf(stderr, "Error: %s\n", ldap_err2string(rc));
        ldap_unbind_ext(ld, NULL, NULL);
        return false;
    }
    else
    {
        printf("bind successful\n");
        return true;
    }

    ldap_unbind_ext(ld, NULL, NULL);
}

auto Server::read_file(std::string_view path) -> std::string
{
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size))
    {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}