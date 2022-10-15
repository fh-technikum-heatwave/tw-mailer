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

using namespace std;

int main(int argc, char *argv[])
{

    int c;
    bool searchRecursive = false;
    bool searchCaseInsensitive = false;

    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {

        switch (c)
        {
        case 'R':
        {
            searchRecursive = true;
            break;
        }

        case 'i':
        {
            searchCaseInsensitive = true;
            break;
        }

        default:
            assert(0);
        }
    }
    return 0;
}