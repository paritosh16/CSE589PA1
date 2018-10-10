	/**
	 * @client
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
	 * This file contains the client.
	 */
	#include <stdio.h>
	#include <iostream>
	#include <stdlib.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <strings.h>
	#include <string.h>
	#include <string>
	#include <arpa/inet.h>
	#include <vector>
	//#include <algorithm>


	#include "../include/client.h"
	#include "../include/logger.h"
	#include "../include/global.h"
	#include "../include/helper.h"

	#define STDIN 0
	#define TRUE 1
	#define cmd_SIZE 256
	#define BUFFER_SIZE 256

	 /**
	 * main function
	 *
	 * @param  argc Number of arguments
	 * @param  argv The argument list
	 * @return 0 EXIT_SUCCESS
	 */
	int client_starter_function(int argc, char **argv)
	{
		if(argc != 3) {
			printf("Usage:%s [mode] [port]\n", argv[0]);
			exit(-1);
		}
		
		int head_socket, selret, server, sock_index = 0;
		fd_set master_list, watch_list;

		// Grab the port number that the client will listen for incoming connections on.
		char* port_number = argv[2];

		// Maintain the list of all the clients.
		std::vector<client_data> all_clients;

		/*Init. Logger*/
		cse4589_init_log(argv[2]);

		/* Zero select FD sets */
	  FD_ZERO(&master_list);
	  FD_ZERO(&watch_list);

	  /* Register STDIN */
	  FD_SET(STDIN, &master_list);

		// Define the head socket to stdin. Socket will be created on Login.
		head_socket = STDIN;
	        
		// The result string that will be printed and logged.
		char result_string[100];

		while(TRUE){
			// Copy the master list into watch list. Play on the watchlist.
			memcpy(&watch_list, &master_list, sizeof(master_list));

			// Client Prompt.
			printf("\n[PA1-Client@CSE489/589]$ ");
			fflush(stdout);

			/* select() system call. This will BLOCK */
	    selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
	    if(selret < 0)
	      perror("select failed.");

			printf("Selret: %d\n", selret);
			 /* Check if we have sockets/STDIN to process */
	    if(selret > 0){
	      /* Loop through socket descriptors to check which ones are ready */
	      for(sock_index = 0; sock_index <= head_socket; sock_index += 1){
					
					if(FD_ISSET(sock_index, &watch_list)){

	          /* Check if new command on STDIN */
	          if (sock_index == STDIN){
							char *cmd = (char*) malloc(sizeof(char)*cmd_SIZE);
							memset(cmd, '\0', cmd_SIZE);
							if(fgets(cmd, cmd_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
									exit(-1);

							// Get rid of the newline character if there is one.
							int len = strlen(cmd); //where buff is your char array fgets is using
							if(cmd[len-1]=='\n')
									cmd[len-1]= NULL;
							
							// The array that holds the tokenized client command.
							std::vector<char*> tokenized_command;
											
							// Tokenize the command.
							int tokenize_status = tokenize_command(&tokenized_command, cmd);
							if(tokenize_status) {
								// Some error occured. 
								printf("Tokenization error\n");
								exit(1);
							}

							// Get the command from the vector.
							const char* command = tokenized_command[0];

							if (strcmp(command, AUTHOR_COMMAND) == 0) {
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
							} else if (strcmp(command, IP_COMMAND) == 0) {
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
							} else if (strcmp(command, PORT_COMMAND) == 0) {
							// Check for the PORT command.				
								sprintf(result_string, "[%s:SUCCESS]\nPORT:", cmd);
								cse4589_print_and_log(result_string);
								cse4589_print_and_log(port_number);
								sprintf(result_string, "\n[%s:END]\n", cmd);
								cse4589_print_and_log(result_string);
							} else if(strcmp(command, LOGIN_COMMAND) == 0) {
							// Check for the LOGIN command. 
								char* server_ip = tokenized_command[1];
								int server_port = atoi(tokenized_command[2]);

								server = connect_to_host(server_ip, server_port, atoi(port_number));

								/* Adding the new socket to the currently watched sockets.*/
								FD_SET(server, &master_list);
	            	if(server > head_socket) head_socket = server;

								printf("Server Object:%d\n", server);
								printf("Server IP:%s\n", server_ip);
								printf("Server Port:%d\n", server_port);
								
								printf("I got: %s(size:%d chars)\n", cmd, strlen(cmd));

								printf("\nSENDing it to the remote server ... \n");
								if(send(server, cmd, strlen(cmd), 0) == strlen(cmd))
										printf("Done!\n");
								fflush(stdout);
								printf("stdout flushed.");

								/* Initialize buffer to receieve response */
								char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
								printf("ALlocated buffer.");
								memset(buffer, '\0', BUFFER_SIZE);

								printf("Just before recieve: ");
								if(recv(server, buffer, sizeof(client_data) * BUFFER_SIZE, 0) >= 0){
									int deserialize_status = deserialize_client_data(&all_clients, buffer);
									// printf("Server responded:\n");
									//print_client_data_vector(&all_clients);
									fflush(stdout);
								}
							} else if(strcmp(command, REFRESH_COMMAND) == 0){
							// Check for the REFRESH command.
							} else if(strcmp(command, SEND_COMMAND) == 0){
							// Check for the SEND command.
								printf("Head socket is :%d\n", head_socket);
								if(send(server, cmd, strlen(cmd), 0) == strlen(cmd))
									printf("The socket descriptor for socket is: %d\n", server);
									printf("Buffer: %s\n", cmd);
									printf("Done!\n");
								fflush(stdout);
							} else if(strcmp(command, LIST_COMMAND) == 0) {
							// Check for the LIST command.
								//std::sort(all_clients.begin(), all_clients.end(), comparator_client_data_port);
								int size = static_cast<int>(all_clients.size());
								for(int i=0; i < size; i++) {
									int sr_no = i + 1;
									char result_string[100];
									sprintf(result_string, "%-5d%-35s%-20s%-8d\n", sr_no, all_clients[i].client_name, all_clients[i].client_ip_address, all_clients[i].client_port);
									cse4589_print_and_log(result_string);
									fflush(stdout);
								}		
							} else if(strcmp(command, BROADCAST_COMMAND) == 0){
							// Check for the BROADCAST command.
							} else if(strcmp(command, BLOCK_COMMAND) == 0){
							// Check for the BLOCK command.
							} else if(strcmp(command, UNBLOCK_COMMAND) == 0){
							// Check for the UNBLOCK command.
							} else if(strcmp(command, BLOCKED_COMMAND) == 0){
							// Check for the BLOCKED command.
							} else if(strcmp(command, LOGOUT_COMMAND)== 0){
							// Check for the LOGOUT command.
							} else if(strcmp(command, EXIT_COMMAND) == 0){
							// Check for the EXIT command.
							} else {
							// TODO: This is the wrong command. Need to check with the requorements to see if any exception has to ber raised for the auto grader.
								printf("Head socket is :%d\n", head_socket);
								if(send(server, cmd, strlen(cmd), 0) == strlen(cmd))
									printf("The socket descriptor for socket is: %d\n", server);
									printf("Buffer: %s\n", cmd);
									printf("Done!\n");
								fflush(stdout);
							}
				} else {
							// Receive from socket.

							/* Initialize buffer to receieve response */
								char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
								memset(buffer, '\0', BUFFER_SIZE);

								if(recv(server, buffer, sizeof(client_data) * BUFFER_SIZE, 0) >= 0){
									printf("Server responded:%s\n", buffer);
									//print_client_data_vector(&all_clients);
									fflush(stdout);
								}
						}
					}
				}
			}
		}
	}

	int connect_to_host(char *server_ip, int server_port, int port_number)
	{
			printf("This is the port number we got:\n");
			printf("%d", port_number);
			printf("%d", htons(port_number));
			
	    int fdsocket, len;
	    struct sockaddr_in remote_server_addr;

	    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	    if(fdsocket < 0)
	       perror("Failed to create socket");

			/*Binding the client to a specific port*/
			struct sockaddr_in device_address;
			device_address.sin_family = AF_INET;
			device_address.sin_addr.s_addr = INADDR_ANY;
			device_address.sin_port = htons(port_number);
			if (bind(fdsocket, (struct sockaddr*) &device_address, sizeof(struct sockaddr_in)) == 0) 
					printf("Binded Correctly\n"); 
			else
					printf("Unable to bind\n"); 

	    bzero(&remote_server_addr, sizeof(remote_server_addr));
	    remote_server_addr.sin_family = AF_INET;
	    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
	    remote_server_addr.sin_port = htons(server_port);

	    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
	        perror("Connect failed");

			printf("Connected to host.\n");

	    return fdsocket;
	}
