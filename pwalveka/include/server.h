#ifndef SERVER_H_
#define SERVER_H_

#include "./helper.h"

#define _BSD_SOURCE

int server_starter_function(int argc, char **argv);
int register_client(int& server_socket,struct sockaddr_in& client_addr,fd_set& master_list,int& head_socket);
struct client_data add_new_client(int &fdsocket,struct sockaddr_in& client_addr);

#endif