
#include <iostream>
#include <string>
#include "Client.hpp"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Missing Arguments! USAGE ==> client <IP> <PORT>");
        return EXIT_FAILURE;
    }

    int port = stoi(argv[2]);
    string ipAdress = argv[1];
    cout<<argv[1]<<endl;
    cout<<argv[2]<<endl;

    Client client{port, ipAdress};

    client.run();


    return 0;
}