/**
 * @ubitname_assignment1
 * @author  Fullname <ubitname@buffalo.edu>
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
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <assert.h>

#include "../include/server.h"
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/client.h"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
#define MSG_SIZE 256

using namespace std;

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	//TODO: fix dependecy for init_log and fclose
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    //fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
    /* Reading the input arguments for server vs client application decision*/
    /* Input Param : type_of_application port_number*/
    assert (argc==3);

    char *type_of_app = argv[1];
    char *port_num = argv[2];

    if (strcmp(type_of_app, "c") == 0)
    {
    	cout << "It is a client \n";
        // Create a socket for the client to listen on the client.
        int port, server_socket, head_socket, selret, sock_index, fdaccept=0;
        struct sockaddr_in server_addr, client_addr;
        fd_set master_list, watch_list;
        socklen_t caddr_len;

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
        if(::bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
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

        do{
            // Print the prompt for the client.
            printf("\n[PA1-Client@CSE489/589]$ ");
		    fflush(stdout);

            // The result string that will be printed and logged.
            char result_string[1024];

            // Accept the input from the user. 
            char *msg = (char*) malloc(sizeof(char)* MSG_SIZE);
            memset(msg, '\0', MSG_SIZE);
            if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
                exit(-1);

            // Get rid of the newline character if there is one.
            int len = strlen(msg); //where buff is your char array fgets is using
            if(msg[len-1]=='\n')
                msg[len-1]='\0';
            
            // Check for the author command.
            if (strcmp(msg, AUTHOR_COMMAND) == 0) {
                    sprintf(result_string, "[%s:SUCCESS]\n", msg);
                    cse4589_print_and_log(result_string);
                    sprintf(result_string, "I, pwalveka, have read and understood the course academic integrity policy.\n");
                    cse4589_print_and_log(result_string);
                    sprintf(result_string, "[%s:END]\n", msg);
                    cse4589_print_and_log(result_string);

            }

            memcpy(&watch_list, &master_list, sizeof(master_list));

            /* select() system call. This will BLOCK */
            selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
            if(selret < 0)
            perror("select failed.");

            /* Check if we have sockets/STDIN to process */
            if(selret > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index = 0; sock_index <= head_socket; sock_index += 1){

                if(FD_ISSET(sock_index, &watch_list)){

                /* Check if new command on STDIN */
                if (sock_index == STDIN){
                    char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

                    memset(cmd, '\0', CMD_SIZE);
                    if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
                    exit(-1);

                    printf("\nI got: %s\n", cmd);

                    //Process PA1 commands here ...

                    free(cmd);
                }
                /* Check if new client is requesting connection */
                else if(sock_index == server_socket){
                    caddr_len = sizeof(client_addr);
                    fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                    if(fdaccept < 0)
                    perror("Accept failed.");

                    printf("\nRemote Host connected!\n");

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_socket) head_socket = fdaccept;
                }
                /* Read from existing clients */
                else{
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

                    printf("\nClient sent me: %s\n", buffer);
                    printf("ECHOing it back to the remote host ... ");
                    if(send(sock_index, buffer, strlen(buffer), 0) == strlen(buffer))
                        printf("Done!\n");
                    fflush(stdout);
                    }

                    free(buffer);
                }
                }
            }
            }
        } while(TRUE);
        
    }

    // Server mode.
    else if (strcmp(type_of_app, "s") == 0)
    {
    	cout << "It is a server \n";
    }

    // Not a valid type choice.
    else
    {
    	cout << "Not a valid input for type of application \n";
    }

    // Exit successful.	
	return 0;
}
