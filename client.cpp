#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>   
#include <string> 

#define BUF 1024
#define PORT 6543

using namespace std;

int main(int argc, char **argv)
{
   int create_socket;
   char buffer[BUF];
   struct sockaddr_in address;
   int size;
   int isQuit;

   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("Socket error");
      return EXIT_FAILURE;
   }

   memset(&address, 0, sizeof(address)); // init storage with 0
   address.sin_family = AF_INET;         // IPv4

   if (argc < 3)
   {
      printf("Missing Arguments! USAGE ==> client <IP> <PORT>");
      return EXIT_FAILURE;
   }
   else
   {
      inet_aton(argv[1], &address.sin_addr);
      address.sin_port = htons(stoi(argv[2]));
   }

   if (connect(create_socket,
               (struct sockaddr *)&address,
               sizeof(address)) == -1)
   {
      // https://man7.org/linux/man-pages/man3/perror.3.html
      perror("Connect error - no server available");
      return EXIT_FAILURE;
   }

   // ignore return value of printf
   printf("Connection with server (%s) established\n",
          inet_ntoa(address.sin_addr));

   ////////////////////////////////////////////////////////////////////////////
   // RECEIVE DATA
   // https://man7.org/linux/man-pages/man2/recv.2.html
   size = recv(create_socket, buffer, BUF - 1, 0);
   if (size == -1)
   {
      perror("recv error");
   }
   else if (size == 0)
   {
      printf("Server closed remote socket\n"); // ignore error
   }
   else
   {
      buffer[size] = '\0';
      printf("%s", buffer); // ignore error
   }

   do
   {
      printf(">> ");
      if (fgets(buffer, BUF, stdin) != NULL)
      {
         int size = strlen(buffer);
         // remove new-line signs from string at the end
         if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
         {
            size -= 2;
            buffer[size] = 0;
         }
         else if (buffer[size - 1] == '\n')
         {
            --size;
            buffer[size] = 0;
         }
         isQuit = strcmp(buffer, "quit") == 0;

         //////////////////////////////////////////////////////////////////////
         // SEND DATA
         // https://man7.org/linux/man-pages/man2/send.2.html
         // send will fail if connection is closed, but does not set
         // the error of send, but still the count of bytes sent
         if ((send(create_socket, buffer, size, 0)) == -1) 
         {


            perror("send error");
            break;
         }



         // size = recv(create_socket, buffer, BUF - 1, 0);
         // if (size == -1)
         // {
         //    perror("recv error");
         //    break;
         // }
         // else if (size == 0)
         // {
         //    printf("Server closed remote socket\n"); // ignore error
         //    break;
         // }
         // else
         // {
         //    buffer[size] = '\0';
         //    printf("<< %s\n", buffer); // ignore error
         //    if (strcmp("OK", buffer) != 0)
         //    {
         //       fprintf(stderr, "<< Server error occured, abort\n");
         //       break;
         //    }
         // }
      }
   } while (!isQuit);

   ////////////////////////////////////////////////////////////////////////////
   // CLOSES THE DESCRIPTOR
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

   return EXIT_SUCCESS;
}
