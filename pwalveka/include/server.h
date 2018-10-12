#ifndef SERVER_H_
#define SERVER_H_

#include "./helper.h"

#define _BSD_SOURCE

int server_starter_function(int argc, char **argv);
int register_client(int& server_socket,struct sockaddr_in& client_addr,fd_set& master_list,int& head_socket);
struct client_data add_new_client(int &fdsocket,struct sockaddr_in& client_addr);
int search_client(char *client_ip_address,std::vector<client_data>& list_of_clients);
int send_message_to_client(int socket_to_send,char *from_client_ip,char *to_client_ip,char *message,char *result_string);
void log_send_message_event(int socket_to_send,char *from_client_ip,char *to_client_ip,char *message,char *result_string);
#endif