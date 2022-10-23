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

// TODO: receiveMessage printer

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

         printf("<< %s\n", receiveMessage(create_socket, buffer));
      } else if(strcmp(command, "LIST\n") == 0) {
         printf("<Username> ");
         // Read Username
         fgets(buffer, BUF, stdin);
         sendMessage(buffer, create_socket);

         int mail_count = stoi(receiveMessage(create_socket, buffer));
         if(mail_count > 0) {
            for (int i = 0; i < mail_count; i++)
            {
               sendMessage((char*)"OK", create_socket);
               printf("<< %s\n", receiveMessage(create_socket, buffer));

            }
         }
      } else if(strcmp(command, "READ\n") == 0) {
         printf("<Username> ");
         // Read Username
         fgets(buffer, BUF, stdin);
         sendMessage(buffer, create_socket);

         printf("<Message-Number> ");
         // Read Message Number
         fgets(buffer, BUF, stdin);
         sendMessage(buffer, create_socket);

         char* ok = receiveMessage(create_socket, buffer);
         printf("<< %s\n", receiveMessage(create_socket, buffer));

         if(strcmp(ok, "OK") == 0) {
            do
            {
               sendMessage((char*)"OK", create_socket);
               char* mailLine = receiveMessage(create_socket, buffer);
               if(strcmp(mailLine, ".\n") == 0) break;
               printf("<< %s\n", mailLine);
            } while (true);
         }
      } else if(strcmp(command, "DEL\n") == 0) {
         printf("<Username> ");
         // Read Username
         fgets(buffer, BUF, stdin);
         sendMessage(buffer, create_socket);

         printf("<Message-Number> ");
         // Read Message Number
         fgets(buffer, BUF, stdin);
         sendMessage(buffer, create_socket);

         printf("<< %s\n", receiveMessage(create_socket, buffer));
      }

      // TODO: HANDLE UNWANTED INPUT
      
      isQuit = strcmp(command, "QUIT\n") == 0;
   
   } while (!isQuit);

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