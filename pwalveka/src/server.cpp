/**
 * @server
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <algorithm>

#include "../include/server.h"
#include "../include/logger.h"
#include "../include/global.h"
#include "../include/helper.h"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256

/**
 * server_starter function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

int server_starter_function(int argc, char **argv)
{
  if(argc != 3) {
    printf("Usage:%s [mode] [port]\n", argv[0]);
    exit(-1);
  }
  printf("Port is : %s",argv[2]);

	// Grab the port number that the client will listen for incoming connections on.
	char* port_number = argv[2];

  /*Init. Logger*/
	cse4589_init_log(argv[2]);

  int port, server_socket, head_socket, selret, sock_index, fdaccept=0;
  struct sockaddr_in server_addr, client_addr;
  fd_set master_list, watch_list;
  socklen_t caddr_len;
  char device_hostname[100];
  char device_ip_address[100];

  /* Data Structure for client*/
  std::vector<client_data> list_of_clients;

   /* Function that populates the IP address of the machine*/
  int res = ip_command(device_hostname,device_ip_address);
  printf("The Hostname of the device is : %s\n", device_hostname);
  printf("The IP address of the device is: %s\n", device_ip_address);

  /* Socket */
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket < 0)
    perror("Cannot create socket");

  /* Fill up sockaddr_in struct */
  port = atoi(argv[2]);
  bzero(&server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);

  /* Bind */
  if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    perror("Bind failed");

  /* Listen */
  if(listen(server_socket, BACKLOG) < 0)
    perror("Unable to listen on port");

  /* ---------------------------------------------------------------------------- */

  /* Zero select FD sets */
  FD_ZERO(&master_list);
  FD_ZERO(&watch_list);

  /* Register the listening socket */
  FD_SET(server_socket, &master_list);
  /* Register STDIN */
  FD_SET(STDIN, &master_list);

  head_socket = server_socket;

  while(TRUE){
    memcpy(&watch_list, &master_list, sizeof(master_list));
    printf("%d \n", head_socket);
    //printf("\n[PA1-Server@CSE489/589]$ ");
    //fflush(stdout);

    /* select() system call. This will BLOCK */
    selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
    if(selret < 0)
      perror("select failed.");

    printf("Select recieved something\n");

    /* Check if we have sockets/STDIN to process */
    if(selret > 0){
      /* Loop through socket descriptors to check which ones are ready */
      for(sock_index = 0; sock_index <= head_socket; sock_index += 1){

        if(FD_ISSET(sock_index, &watch_list)){

          /* Check if new command on STDIN */
          if (sock_index == STDIN){
            char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

          	// The result string that will be printed and logged.
	          char result_string[1024];

            memset(cmd, '\0', CMD_SIZE);
            if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
              exit(-1);

            // Get rid of the newline character if there is one.
            int len = strlen(cmd); //where buff is your char array fgets is using
            if(cmd[len-1]=='\n')
                cmd[len-1]='\0';

            //printf("\nI got: %s\n", cmd);

            //Process PA1 commands here ...
            if (strcmp(cmd, AUTHOR_COMMAND) == 0) {
            // Check for the author command.
              char author_command_result[1024];
				      int status = author_command(author_command_result);
              if (!status) {
                // Successful execution. 
                sprintf(result_string, "[%s:SUCCESS]\n", cmd);
                cse4589_print_and_log(result_string);
                cse4589_print_and_log(author_command_result);
                sprintf(result_string, "\n[%s:END]\n", cmd);
                cse4589_print_and_log(result_string);
              } else {
                // Error has occured.
                sprintf(result_string, "[%s:ERROR]\n", cmd);
                cse4589_print_and_log(result_string);
                sprintf(result_string, "[%s:END]\n", cmd);
                cse4589_print_and_log(result_string);
              }
            } else if (strcmp(cmd, IP_COMMAND) == 0) {
            // Check for the IP command.
              char device_hostname[100];
              char device_ip_address[100];
              int status = ip_command(device_hostname, device_ip_address);
              if(!status) {
                // Successful execution. 
                sprintf(result_string, "[%s:SUCCESS]\nIP:", cmd);
                cse4589_print_and_log(result_string);
                cse4589_print_and_log(device_ip_address);
                sprintf(result_string, "\n[%s:END]\n", cmd);
                cse4589_print_and_log(result_string);
              } else {
                // Error has occured.
                sprintf(result_string, "[%s:ERROR]\n", cmd);
                cse4589_print_and_log(result_string);
                sprintf(result_string, "[%s:END]\n", cmd);
                cse4589_print_and_log(result_string);
              }
            } else if (strcmp(cmd, PORT_COMMAND) == 0) {
            // Check for the PORT commad.
              sprintf(result_string, "[%s:SUCCESS]\nPORT:", cmd);
              cse4589_print_and_log(result_string);
              cse4589_print_and_log(port_number);
              sprintf(result_string, "\n[%s:END]\n", cmd);
              cse4589_print_and_log(result_string);
            } else if(strcmp(cmd, STATISTICS_COMMAND) == 0) {
                  char decoded_string[20];
                  char result_string[100];
                  int decode_status;
                  //std::sort(list_of_clients.begin(), list_of_clients.end(), comparator_client_data_port);
                  int size = static_cast<int>(list_of_clients.size());
                  for(int i=0; i < size; i++) {
                    int sr_no = i + 1;
                    decode_status = decode_client_status(list_of_clients[i].status, decoded_string);
                    sprintf(result_string, "%-5d%-35s%-8d%-8d%-8s\n", sr_no, list_of_clients[i].client_name, list_of_clients[i].message_sent, list_of_clients[i].message_recieved, decoded_string);
                    cse4589_print_and_log(result_string);
                    fflush(stdout);
                  }
            } else {
            // TODO: This is the wrong command. Need to check with the requorements to see if any exception has to ber raised for the auto grader.
            }
            free(cmd);
          }
          /* Check if new client is requesting connection */
          else if(sock_index == server_socket){
            printf("Registering the new client\n");
            fdaccept = register_client(server_socket,client_addr,master_list,head_socket);
            if (fdaccept > 0)
            {
              printf("New Host connected with file descriptor : %d\n", fdaccept); 
              struct client_data new_client = add_new_client(fdaccept,client_addr);
              list_of_clients.push_back(new_client); 
              char acknowledgement[] = "LOGIN Recieved";
              char *serialized_data = (char*) malloc(sizeof(char)*BUFFER_SIZE);
              int serialize_status = serialize_client_data(&list_of_clients, serialized_data);
              send(new_client.sock_decriptor, serialized_data, BUFFER_SIZE, 0);
            }
            /*caddr_len = sizeof(client_addr);
            fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
            if(fdaccept < 0)
              perror("Accept failed.");

            printf("\nRemote Host connected!\n");

            /* Add to watched socket list 
            FD_SET(fdaccept, &master_list);
            if(fdaccept > head_socket) head_socket = fdaccept;*/
          }
          /* Read from existing clients */
          else{
            printf("Reading from existing client.");
            /* Initialize buffer to receieve response */
            char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
            memset(buffer, '\0', BUFFER_SIZE);

            if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
              close(sock_index);
              printf("Remote Host terminated connection!\n");

              /* Remove from watched list */
              FD_CLR(sock_index, &master_list);
            }
            else {
              //Process incoming data from existing clients here ...
              printf("The size of sockets so far:%d\n",head_socket);  
              printf("\nClient sent me: %s\n", buffer);
              printf("Searching for the socket to send\n");
              int temp_socket = search_client(buffer,list_of_clients);
              if (temp_socket > 0)
              {
                if(send(temp_socket, buffer, strlen(buffer), 0) == strlen(buffer))
                  printf("Transmitted to client!\n");  
              }
              else
              {
                printf("Invalid IP\n");
              }
              printf("The socket found is : %d\n", temp_socket);
              printf("ECHOing it back to the remote host ... \n");
              if(send(sock_index, buffer, strlen(buffer), 0) == strlen(buffer))
                printf("Done!\n");
              fflush(stdout);
            }
            free(buffer);            
          }
        }
        else
        {
          //printf("Not a part of list\n");
        }
      }
    }
  }
  return 0;
}

/* Function that registers a new client to the currently listening clients
  This is done by adding the clients to the list of master list
*/

int register_client(int& server_socket,struct sockaddr_in& client_addr,fd_set& master_list,int& head_socket)
{
  printf("Inside register new client\n");
  int new_socket_descriptor = 0;
  socklen_t caddr_len = sizeof(client_addr);
  new_socket_descriptor = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
  printf("New socket is just after accept %d\n",new_socket_descriptor );
  if(new_socket_descriptor < 0)
  {
    perror("Accept failed.");
    return new_socket_descriptor;
  }
      

  printf("\nRemote Host connected!\n");
      
  /* Add to watched socket list */
  FD_SET(new_socket_descriptor, &master_list);
  printf("New sock_decriptor is : %d\n",new_socket_descriptor);
  if(new_socket_descriptor > head_socket) head_socket = new_socket_descriptor;

  return new_socket_descriptor;

}

struct client_data add_new_client(int &fdsocket,struct sockaddr_in& client_addr)
{
  client_data new_client;
  int len=100; 
  new_client.sock_decriptor = fdsocket;
  inet_ntop(AF_INET, &(client_addr.sin_addr), new_client.client_ip_address, len);
  // DEBUG BLOG
  int res = getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), new_client.client_ip_address, sizeof(new_client.client_ip_address), NULL, 0, NI_NUMERICHOST);
  int hostname_from_addr = getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), new_client.client_name, sizeof(new_client.client_name), NULL, 0, NI_NAMEREQD);
  printf("The ip address of the client is : %s\n", new_client.client_ip_address);
  printf("The domain address of the client is : %s\n", new_client.client_name);
  printf("Port to be entered in the vector:%d\n", ntohs(client_addr.sin_port));
  new_client.client_port = ntohs(client_addr.sin_port);
  new_client.message_sent = 0;
  new_client.message_recieved = 0;
  new_client.status = 1;
  return new_client;
}

/* Function that takes domain address & port and looks for the corresponding file descriptor*/
int search_client(char *client_ip_address,std::vector<client_data>& list_of_clients)
{
  for(int i = 0; i < list_of_clients.size();i++)
  {
    
    if (strcmp(client_ip_address,list_of_clients[i].client_ip_address) == 0)
    {
      return list_of_clients[i].sock_decriptor;
    }
  }

  return -1;
}


/*Function that sends message to the given client  */
/*
int send_to_client(char *client_addr,int *client_socket,char *message)
{
  if(send(client_socket, message, strlen(message), 0) == strlen(message))
  {
    printf("Done!\n");
  }
  else
  {
    printf("Unable to send the message\n");
    return -1;
  }

  return 1;
}*/