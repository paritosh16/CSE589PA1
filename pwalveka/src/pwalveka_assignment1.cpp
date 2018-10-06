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
	//cse4589_init_log(argv[2]);

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
        int result = client_starter_function(argc, argv);
        if(!result) {
            // Server function succesful.
            return 0;
        } else {
            // Server function failed!
            exit(-1);
        }        
    }

    // Server mode.
    else if (strcmp(type_of_app, "s") == 0)
    {
    	//cout << "It is a server \n";
        int result = server_starter_function(argc, argv);
        if(!result) {
            // Server function succesful.
            return 0;
        } else {
            // Server function failed!
            exit(-1);
        }

    }

    // Not a valid type choice.
    else
    {
    	cout << "Not a valid input for type of application \n";
        return 0;
    }

    // Exit successful.	
	//return 0;
}
