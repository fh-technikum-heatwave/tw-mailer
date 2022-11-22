#include "Server.hpp"
#include <unistd.h>

int main(int argc, char *argv[])
{

    string mailDirectoryName;
    string port;

    if (optind < argc)
    {
        port = argv[optind++];
        // PORT = stoi(port);
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


    Server server(stoi(port), mailDirectoryName);
    server.run();

    return 0;
}