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
#include <sstream>

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
        // Mode of the application is client.
        
        // The result string that will be printed and logged.
        char result_string[1024];

        while(TRUE){
            printf("\n[PA1-Client@CSE489/589]$ ");
            fflush(stdout);

            char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
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
                    sprintf(result_string, "AUTHOR:I, pwalveka, have read and understood the course academic integrity policy.\n");
                    cse4589_print_and_log(result_string);
                    sprintf(result_string, "[%s:END]\n", msg);
                    cse4589_print_and_log(result_string);

            } else {
                int server;
	            server = connect_to_host(argv[1], atoi(argv[2]));

                printf("I got: %s(size:%d chars)", msg, strlen(msg));

                printf("\nSENDing it to the remote server ... ");
                if(send(server, msg, strlen(msg), 0) == strlen(msg))
                    printf("Done!\n");
                fflush(stdout);

                /* Initialize buffer to receieve response */
                char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                memset(buffer, '\0', BUFFER_SIZE);

                if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
                    printf("Server responded: %s", buffer);
                    fflush(stdout);
                }
            }    
        }
        
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
