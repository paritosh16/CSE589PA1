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
#include <map>

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

  int port, server_socket, head_socket, selret, sock_index, fdaccept=0,search_status,send_result,sending_client_index,ptr;
  struct sockaddr_in server_addr, client_addr;
  fd_set master_list, watch_list;
  socklen_t caddr_len;
  char device_hostname[100];
  char device_ip_address[100];
  char result_string[100];
  char end_of_message[] = "end_of_message";
  /* Data Structure for client*/
  std::vector<client_data> list_of_clients;


  /* Block list.*/
  std::map<std::string , std::vector<std::string> > block_list;

  /*Data Structure to mantain the list of the buffers*/
  std::vector<buffered_data> buffered_messages;

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

    //printf("\n[PA1-Server@CSE489/589]$ ");
    //fflush(stdout);

    /* select() system call. This will BLOCK */
    selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
    if(selret < 0)
      perror("select failed.");

    /* Check if we have sockets/STDIN to process */
    if(selret > 0){
      /* Loop through socket descriptors to check which ones are ready */
      for(sock_index = 0; sock_index <= head_socket; sock_index += 1){

        if(FD_ISSET(sock_index, &watch_list)){
          if (sock_index == STDIN){
          /* Check if new command on STDIN */
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

            // The array that holds the tokenized client command.
            std::vector<char*> tokenized_command;
                      
            // Tokenize the command.
            int tokenize_status = tokenize_command(&tokenized_command, cmd);
            if(tokenize_status) {
              // Some error occured. 
              exit(1);
            }

            // Get the command from the vector.
            const char* command = tokenized_command[0];

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
              strcpy(result_string, "[STATISTICS:SUCCESS]\n");
							cse4589_print_and_log(result_string);
              char decoded_string[20];
              char result_string[100];
              int decode_status;
              std::sort(list_of_clients.begin(), list_of_clients.end(), comparator_client_data_port);
              int size = static_cast<int>(list_of_clients.size());
              for(int i=0; i < size; i++) {
                int sr_no = i + 1;
                decode_status = decode_client_status(list_of_clients[i].status, decoded_string);
                sprintf(result_string, "%-5d%-35s%-8d%-8d%-8s\n", sr_no, list_of_clients[i].client_name, list_of_clients[i].message_sent, list_of_clients[i].message_recieved, decoded_string);
                cse4589_print_and_log(result_string);
                fflush(stdout);
              }
              strcpy(result_string, "[STATISTICS:END]\n");
							cse4589_print_and_log(result_string);
            // Check for the STATISTICS command. 
            } else if(strcmp(cmd, LIST_COMMAND)== 0) {
            // Check for the LIST command.
							strcpy(result_string, "[LIST:SUCCESS]\n");
							cse4589_print_and_log(result_string);
							std::sort(list_of_clients.begin(), list_of_clients.end(), comparator_client_data_port);
							int size = static_cast<int>(list_of_clients.size());
              int sr_no = 1;
							for(int i=0; i < size; i++) {
								char result_string[100];
								if(list_of_clients[i].status == 1) {
									sprintf(result_string, "%-5d%-35s%-20s%-8d\n", sr_no, list_of_clients[i].client_name,list_of_clients[i].client_ip_address, list_of_clients[i].client_port);
									cse4589_print_and_log(result_string);
                  sr_no++;
								}
								fflush(stdout);
							}	
							strcpy(result_string, "[LIST:END]\n");
							cse4589_print_and_log(result_string);	              
            } else if(strcmp(command, BLOCKED_COMMAND) == 0) {
            // Check for the blocked command.
              int is_address_valid = is_ip_address_valid(tokenized_command[1]);
              if (!is_address_valid) {
                // Address not valid. Print error.
                strcpy(result_string, "[BLOCKED:ERROR]\n[BLOCKED:END]\n");
							  cse4589_print_and_log(result_string);
              } else {
                std::map<std::string, std::vector<std::string> >::iterator iter = block_list.find(tokenized_command[1]);
                if (iter != block_list.end()) {
                  // Found the key. Now print the list.
                  strcpy(result_string, "[BLOCKED:SUCCESS]\n");
                  cse4589_print_and_log(result_string);
                  int sr_no = 1;
                  for(int i = 0; i < block_list[tokenized_command[1]].size(); i++) {
                    int index;
                    // Get the client details
                    int status = get_client_data_from_ip(block_list[tokenized_command[1]].at(i).c_str(), &list_of_clients, &index);
                    sprintf(result_string, "%-5d%-35s%-20s%-8d\n", sr_no, list_of_clients[index].client_name,list_of_clients[index].client_ip_address, list_of_clients[index].client_port);
                      cse4589_print_and_log(result_string);
                      sr_no++;
                  }
                  strcpy(result_string, "[BLOCKED:END]\n");
                  cse4589_print_and_log(result_string);
                  fflush(stdout);                  
                } else {
                  // IP address was valid, but cannot find it in the list. Print error.
                  strcpy(result_string, "[BLOCKED:ERROR]\n[BLOCKED:END]\n");
							    cse4589_print_and_log(result_string);
                }
              }
            } else {
              // TODO: This is the wrong command. Need to check with the requorements to see if any exception has to ber raised for the auto grader.
            }
            free(cmd);
          } else if (sock_index == server_socket) {
          /* Check if new client is requesting connection */
            printf("Registering the new client");
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
              printf("Done sending the serialized data\n");

              // print the buffer so far
              ptr = 0;
              while(ptr < buffered_messages.size())
              {
                
                if (strcmp(buffered_messages[ptr].client_recieving_ip_address,new_client.client_ip_address) == 0)
                {
                  printf("The pending messages of the new client  is:%s\n", buffered_messages[ptr].buffered_message);
                  //send(new_client.sock_decriptor, buffered_messages[i].buffered_message, BUFFER_SIZE, 0); 
                  send_result = send_message_to_client(new_client.sock_decriptor,buffered_messages[ptr].client_send_ip_address,buffered_messages[ptr].client_recieving_ip_address,buffered_messages[ptr].buffered_message,result_string);
                  log_send_message_event(new_client.sock_decriptor,buffered_messages[ptr].client_send_ip_address,buffered_messages[ptr].client_recieving_ip_address,buffered_messages[ptr].buffered_message,result_string);
                  new_client.message_recieved++;
                  buffered_messages.erase(buffered_messages.begin() + ptr);
                }
                else
                {
                  ptr = ptr + 1;
                }

              } // end of while loop looking for the buffer messages
              printf("End of checking the buffer\n");
              printf("%s\n",end_of_message );
              send(fdaccept,end_of_message,strlen(end_of_message), 0);
              printf("Sent confirmation to client\n");
            }
            
            fflush(stdout);
          } else {
          /* Read from existing clients */
            /* Initialize buffer to receieve response */
            char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
            memset(buffer, '\0', BUFFER_SIZE);

            if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
              close(sock_index);
              printf("Remote Host terminated connection!\n");
              /* Remove from watched list */
              FD_CLR(sock_index, &master_list);
            } else {
              //Process incoming data from existing clients here ...
              //printf("The size of sockets so far:%d\n", head_socket);  
              printf("\nClient sent me: %s\n", buffer);
              //printf("ECHOing it back to the remote host ... \n");

              // The array that holds the tokenized client command.
              std::vector<char*> tokenized_command;
                      
              // Tokenize the command.
              int tokenize_status = tokenize_command(&tokenized_command, buffer);
              if(tokenize_status) {
                // Some error occured. 
                exit(1);
              }

              // Get the command from the vector.
              const char* command = tokenized_command[0];

              if(strcmp(command, LOGIN_COMMAND) == 0) {
              // Check for the LOGIN command.
                int index;
                // Get the client details
                int status = get_client_data_from_sock(sock_index, &list_of_clients, &index);
                // Set the status of client to logged in.
                list_of_clients[index].status = 1;
                // Serialize the data.
                char *serialized_data = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                int serialize_status = serialize_client_data(&list_of_clients, serialized_data);
                if(send(sock_index, serialized_data, BUFFER_SIZE, 0) == BUFFER_SIZE)
                  printf("LOGIN (for already logged-in client) done!\n");
                  /*Add block for sending message to logged in commands*/
                  ptr = 0;
                  while(ptr < buffered_messages.size())
                  {
                  
                  if (strcmp(buffered_messages[ptr].client_recieving_ip_address,list_of_clients[index].client_ip_address) == 0)
                  {
                    printf("The pending messages of the re-loggen in client  is:%s\n", buffered_messages[ptr].buffered_message);
                    //send(new_client.sock_decriptor, buffered_messages[i].buffered_message, BUFFER_SIZE, 0); 
                    send_result = send_message_to_client(sock_index,buffered_messages[ptr].client_send_ip_address,buffered_messages[ptr].client_recieving_ip_address,buffered_messages[ptr].buffered_message,result_string);
                    log_send_message_event(sock_index,buffered_messages[ptr].client_send_ip_address,buffered_messages[ptr].client_recieving_ip_address,buffered_messages[ptr].buffered_message,result_string);
                    list_of_clients[index].message_recieved++;
                    buffered_messages.erase(buffered_messages.begin() + ptr);
                  }
                  else
                  {
                    ptr = ptr + 1;
                  }

                } // end of loop looking for the messages in the buffer
                send(sock_index, "end_of_message", 14, 0);


                fflush(stdout);
              } else if (strcmp(command, SEND_COMMAND) == 0) {
              // Check for the SEND command.
                int socket_to_send = search_client(tokenized_command[1],list_of_clients);
                bool cacheFlag = true;
                if (socket_to_send > 0 )
                {
                  search_status = get_client_data_from_sock(sock_index, &list_of_clients, &sending_client_index); 
                  int index_to_send = -1;
                  search_status = get_client_data_from_sock(socket_to_send, &list_of_clients, &index_to_send); 
                  // updating stastics to send
                  list_of_clients[sending_client_index].message_sent++;
                  bool is_blocked = false;
                  std::map<std::string, std::vector<std::string> >::iterator iter = block_list.find(std::string(list_of_clients[sending_client_index].client_ip_address));
                  if (iter != block_list.end()) {
                    // Found the block list of the sending client.
                    if (std::find(block_list[list_of_clients[sending_client_index].client_ip_address].begin(), block_list[list_of_clients[sending_client_index].client_ip_address].end(), list_of_clients[index_to_send].client_ip_address) != block_list[list_of_clients[sending_client_index].client_ip_address].end()) {
                      is_blocked = true;
                    }
                  }
                  if (list_of_clients[index_to_send].status > 0 && !is_blocked)
                  {
                  send_result = send_message_to_client(socket_to_send,list_of_clients[sending_client_index].client_ip_address,tokenized_command[1],tokenized_command[2],result_string);
                  log_send_message_event(socket_to_send,list_of_clients[sending_client_index].client_ip_address,tokenized_command[1],tokenized_command[2],result_string); 
                  list_of_clients[index_to_send].message_recieved++;
                  cacheFlag = false;
                  }
                  if(is_blocked) {
                    cacheFlag = false;
                  }
                }
                /*Case when message is not trasmitted and is cached*/
                if(cacheFlag)
                {
                  /*Case to cache the stuff in the buffer */
                  sending_client_index = 0;
                  search_status = get_client_data_from_sock(sock_index, &list_of_clients, &sending_client_index); 
                  buffered_data new_message;
                  strcpy(new_message.client_send_ip_address,list_of_clients[sending_client_index].client_ip_address);
                  strcpy(new_message.client_recieving_ip_address,tokenized_command[1]);
                  strcpy(new_message.buffered_message,tokenized_command[2]);
                  buffered_messages.push_back(new_message);
                  printf("Added the new message to the buffer\n");
                  printf("Buffer size is : %d\n",buffered_messages.size() );

                }
              } else if(strcmp(command, BROADCAST_COMMAND) == 0) {
              // Check for BROADCAST command.
                printf("Recieved Broadcast\n");
                search_status = get_client_data_from_sock(sock_index, &list_of_clients, &sending_client_index); 
                printf("The details of the client ip address is:%s \n",list_of_clients[sock_index].client_ip_address);
                // updating statstics for sendig client
                list_of_clients[sending_client_index].message_sent++;
                bool is_blocked = false;
                std::map<std::string, std::vector<std::string> >::iterator iter = block_list.find(std::string(list_of_clients[sending_client_index].client_ip_address));
                for(int i = 0; i < list_of_clients.size();i++)
                {
                  is_blocked = false;
                  if (iter != block_list.end()) {
                    // Found the block list of the sending client.
                    if (std::find(block_list[list_of_clients[sending_client_index].client_ip_address].begin(), block_list[list_of_clients[sending_client_index].client_ip_address].end(), list_of_clients[i].client_ip_address) != block_list[list_of_clients[sending_client_index].client_ip_address].end()) {
                      is_blocked = true;
                    }
                  }
                  if ((list_of_clients[i].sock_decriptor != sock_index) && !is_blocked)
                  {
                    if (list_of_clients[i].status > 0)
                    {
                      send_result = send_message_to_client(list_of_clients[i].sock_decriptor,list_of_clients[sending_client_index].client_ip_address,"255.255.255.255",tokenized_command[1],result_string);
                      list_of_clients[i].message_recieved++;                      
                    }
                    else
                    {
                      /*Case to cache the stuff in the buffer */
                    sending_client_index = 0;
                    search_status = get_client_data_from_sock(sock_index, &list_of_clients, &sending_client_index); 
                    buffered_data new_message;
                    strcpy(new_message.client_send_ip_address,list_of_clients[sending_client_index].client_ip_address);
                    strcpy(new_message.client_recieving_ip_address,list_of_clients[i].client_ip_address);
                    strcpy(new_message.buffered_message,tokenized_command[1]);
                    buffered_messages.push_back(new_message);
                    printf("Added the new message to the buffer\n");
                    printf("Buffer size is : %d\n",buffered_messages.size() );
                    printf("IP message of the new message added is : %s\n", new_message.client_recieving_ip_address);
                    }
                    
                  }
                  
                  /*if(send(list_of_clients[i].sock_decriptor, tokenized_command[1], strlen(tokenized_command[1]), 0) == strlen(tokenized_command[1]))
                  {
                    printf("Done sending to %s\n",list_of_clients[i].client_name);
                  }*/
                }

                log_send_message_event(list_of_clients[0].sock_decriptor,list_of_clients[sending_client_index].client_ip_address,"255.255.255.255",tokenized_command[1],result_string);

              } else if(strcmp(command, LOGOUT_COMMAND) == 0) {
              // Check for LOGOUT command.
                int index;
                // Get the client details
                int status = get_client_data_from_sock(sock_index, &list_of_clients, &index);
                // Log out the client.
                list_of_clients[index].status = 0;
                char* logout_response = "LOGOUT";
                // Send confirmation back to client.
                if(send(sock_index, logout_response, strlen(logout_response), 0) == strlen(logout_response))
                  printf("LOGOUT done!\n");
                fflush(stdout);
              } else if (strcmp(command, EXIT_COMMAND) == 0) {
              // Check for EXIT command.
                int index;
                // Get the client details
                int status = get_client_data_from_sock(sock_index, &list_of_clients, &index);
                // Log out the client.
                list_of_clients[index].status = 0;
                // Delete all the state of the client.
                list_of_clients.erase(list_of_clients.begin() + index);
                // EXIT confirmation.
                char* exit_response = "EXIT";
                if(send(sock_index, exit_response, strlen(exit_response), 0) == strlen(exit_response))
                  printf("EXIT done!.\n");
                fflush(stdout);
              } else if(strcmp(command, REFRESH_COMMAND) == 0) {
              // Check for REFRESH command.
                int index;
                // Get the client details
                int status = get_client_data_from_sock(sock_index, &list_of_clients, &index);
                // Serialize the data.
                char *serialized_data = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                int serialize_status = serialize_client_data(&list_of_clients, serialized_data);
                if(send(sock_index, serialized_data, BUFFER_SIZE, 0) == BUFFER_SIZE)
                  printf("REFRESH done!\n");
                fflush(stdout);
              } else if(strcmp(command, BLOCK_COMMAND) == 0) {
              // Check for BLOCK command.
                int index;
                char ip_address[100];
                char block_response[100];
                strcpy(ip_address, tokenized_command[1]);
                // Get the client details
                int status = get_client_data_from_sock(sock_index, &list_of_clients, &index);
                // Check if key is present in the map.
                if (block_list.find(list_of_clients[index].client_ip_address) != block_list.end()) {
                  // Contains the key.
                  // Find if the IP address is already blocked.
                  if (std::find(block_list[list_of_clients[index].client_ip_address].begin(), block_list[list_of_clients[index].client_ip_address].end(), tokenized_command[1]) != block_list[list_of_clients[index].client_ip_address].end()) {
                    // IP address exists, send an error message.
                    strcpy(block_response, "ERROR");
                  } else {
                    // Need to push the IP address.
                    block_list[list_of_clients[index].client_ip_address].push_back(std::string(tokenized_command[1]));
                    strcpy(block_response, "BLOCK");
                  }
                } else {
                  // First ever block operation, need to create a key.
                  block_list[list_of_clients[index].client_ip_address] = std::vector<std::string>();
                  block_list[list_of_clients[index].client_ip_address].push_back(std::string(tokenized_command[1]));
                  strcpy(block_response, "BLOCK");
                }
                if(send(sock_index, block_response, strlen(block_response), 0) == strlen(block_response))
                  printf("BLOCK done!\n");
                fflush(stdout);
              } else if(strcmp(command, UNBLOCK_COMMAND) == 0) {
              // Check for UNBLOCK command.
                int index;
                char unblock_response[100];
                // Get the client details
                int status = get_client_data_from_sock(sock_index, &list_of_clients, &index);
                // Delete the ip to be unblocked from the vector.
                std::vector<std::string>::iterator iter = std::find(block_list[std::string(list_of_clients[index].client_ip_address)].begin(), block_list[std::string(list_of_clients[index].client_ip_address)].end(), std::string(tokenized_command[1]));
                if(iter != block_list[std::string(list_of_clients[index].client_ip_address)].end()) {
                  // Item found. Need to delete this now.
                  block_list[std::string(list_of_clients[index].client_ip_address)].erase(iter);
                  strcpy(unblock_response, "UNBLOCK");
                } else {
                  // Item not found implying the IP to unblock was never blocked.
                  strcpy(unblock_response, "ERROR");
                }
                if(send(sock_index, unblock_response, strlen(unblock_response), 0) == strlen(unblock_response))
                  printf("UNBLOCK done!\n");
                fflush(stdout);
              } else {
                // Not a valid command.
                if(send(sock_index, buffer, strlen(buffer), 0) == strlen(buffer))
                  printf("Not a valid command. Please read the manual.\n");
                fflush(stdout);
              }
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
  int new_socket_descriptor = 0;
  socklen_t caddr_len = sizeof(client_addr);
  new_socket_descriptor = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
  if(new_socket_descriptor < 0) {
      perror("Accept failed.");
      return new_socket_descriptor;
  }

  printf("\nRemote Host connected!\n");
      
  /* Add to watched socket list */
  FD_SET(new_socket_descriptor, &master_list);
  if(new_socket_descriptor > head_socket) head_socket = new_socket_descriptor;

  return new_socket_descriptor;

}

struct client_data add_new_client(int &fdsocket,struct sockaddr_in& client_addr)
{
  client_data new_client;
  int len=100; 
  new_client.sock_decriptor = fdsocket;
  inet_ntop(AF_INET, &(client_addr.sin_addr), new_client.client_ip_address, len);
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


/* Function that sends message to a client*/
int send_message_to_client(int socket_to_send,char *from_client_ip,char *to_client_ip,char *message,char *result_string)
{
  std::string transmit_string = std::string("");
  transmit_string.append(from_client_ip);
  transmit_string.append(std::string(" "));
  transmit_string.append(message);
  char transmitting_string[BUFFER_SIZE] ;
  strcpy(transmitting_string, transmit_string.c_str());
  //printf("%s\n",transmitting_string);
  if(send(socket_to_send, transmitting_string, strlen(transmitting_string), 0) == strlen(transmitting_string))
  {
    
    /* Update the statstics fore receving client*/


    return 1;
  }


  return -1;
}




/*Function that logs the event message on succeful transmit*/
void log_send_message_event(int socket_to_send,char *from_client_ip,char *to_client_ip,char *message,char *result_string)
{
    sprintf(result_string, "[RELAYED:SUCCESS]\n");
    cse4589_print_and_log(result_string);
    sprintf(result_string,"msg from:%s, to:%s\n[msg]:%s\n", from_client_ip, to_client_ip, message);
    cse4589_print_and_log(result_string);
    sprintf(result_string,"[RELAYED:END]\n");
    cse4589_print_and_log(result_string);

}