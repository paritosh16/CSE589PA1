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
#include <strings.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <vector>


#include "../include/client.h"
#include "../include/logger.h"
#include "../include/global.h"
#include "../include/helper.h"

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

	// Grab the port number that the client will listen for incoming connections on.
	char* port_number = argv[2];

	/*Init. Logger*/
	cse4589_init_log(argv[2]);
        
	// The result string that will be printed and logged.
	char result_string[100];

	while(TRUE){
			printf("\n[PA1-Client@CSE489/589]$ ");
			fflush(stdout);

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
				exit(1);
			}

			// Get the command from the vector.
			const char* command = tokenized_command[0];

			// Check for the author command.
			if (strcmp(command, AUTHOR_COMMAND) == 0) {
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
			// Check for the IP command.
			} else if (strcmp(command, IP_COMMAND) == 0) {
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
			// Check for the PORT command.
			} else if (strcmp(command, PORT_COMMAND) == 0) {
				sprintf(result_string, "[%s:SUCCESS]\nPORT:", cmd);
				cse4589_print_and_log(result_string);
				cse4589_print_and_log(port_number);
				sprintf(result_string, "\n[%s:END]\n", cmd);
				cse4589_print_and_log(result_string);
			} else if(strcmp(command, LOGIN_COMMAND) == 0) {
				int server;
				server = connect_to_host(tokenized_command[1], atoi(tokenized_command[2]));

				printf("%d", server);
				printf("%s", tokenized_command[1]);
				printf("%s", tokenized_command[2]);
				
				printf("I got: %s(size:%d chars)", cmd, strlen(cmd));

				printf("\nSENDing it to the remote server ... ");
				if(send(server, cmd, strlen(cmd), 0) == strlen(cmd))
						printf("Done!\n");
				fflush(stdout);

				/* Initialize buffer to receieve response */
				char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
				memset(buffer, '\0', BUFFER_SIZE);

				if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
						printf("Server responded: %s", buffer);
						fflush(stdout);
				}
			} else {
			}   
	}
}

int connect_to_host(char *server_ip, int server_port)
{
    int fdsocket, len;
    struct sockaddr_in remote_server_addr;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0)
       perror("Failed to create socket");

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");

    return fdsocket;
}