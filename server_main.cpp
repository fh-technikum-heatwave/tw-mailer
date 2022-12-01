#include "Server.hpp"
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <ldap.h>
#include <ldap_cdefs.h>

namespace fs = std::filesystem;

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

    fs::path directoryName{mailDirectoryName};

    fs::create_directory(directoryName);

    Server server(stoi(port), mailDirectoryName);

    server.run();

    return 0;
}
