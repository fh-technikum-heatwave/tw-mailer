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


// TODO: both functions require createSocket maybe put it in other scope so we dont have to pass it in all the time
void sendMessage(char *message, int create_socket)
{
   if ((send(create_socket, message, strlen(message), 0)) == -1) 
   {
      perror("send error");
   }
}

char* receiveMessage(int create_socket, char* buffer)
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
   return buffer;
}


int main(int argc, char **argv)
{
   int create_socket;
   char buffer[BUF], command[BUF];
   struct sockaddr_in address;
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


   printf("<< %s", receiveMessage(create_socket, buffer));

   do
   {
      printf(">> ");
      fgets(command, BUF, stdin);
      sendMessage(command, create_socket);

      if(strcmp(command, "SEND\n") == 0) {
         const char* send_fields[3] = { "<SENDER> ", "<RECEIVER> ", "<SUBJECT> "}; 
         const int send_fields_length = (sizeof(send_fields)/sizeof(*send_fields));
         for (int i = 0; i < send_fields_length; i++)
         {
            printf("%s", send_fields[i]);
            fgets(buffer, BUF, stdin);
            if(i == send_fields_length - 1 && strlen(buffer) > 80) {
               printf("Subject can not be longer than 80 chars");
               break;
            } 
            else 
            {
               sendMessage(buffer, create_socket);
            }
         }     

         printf("<MESSAGE> ");
         do
         {
            fgets(buffer, BUF, stdin);
            sendMessage(buffer, create_socket);
         } while (strcmp(buffer, ".\n") != 0);

         printf("<< %s", receiveMessage(create_socket, buffer));
      }


      
      isQuit = strcmp(command, "QUIT\n") == 0;
      // if (fgets(buffer, BUF, stdin) != NULL)
      // {
      //    int size = strlen(buffer);
      //    // remove new-line signs from string at the end
      //    if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
      //    {
      //       size -= 2;
      //       buffer[size] = 0;
      //    }
      //    else if (buffer[size - 1] == '\n')
      //    {
      //       --size;
      //       buffer[size] = 0;
      //    }
      //    isQuit = strcmp(buffer, "quit") == 0;
      // }
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