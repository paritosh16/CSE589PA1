#ifndef HELPER_H_
#define HELPER_H_

#include <vector>

/* Struct that mantains information for all the clients that 
  login to the server
  It has the following values:
  client_port : the port from which client is logged in
*/
struct client_data{
  int sock_decriptor;
  char client_name[100];
  char client_ip_address[100];
  int client_port;
  int message_sent;
  int message_recieved;
  int status;
};

int author_command(char *result_string);
int ip_command(char *device_hostname, char *device_ip_address);
int tokenize_command(std::vector<char*>* tokenized_command, char* cmd);
int serialize_client_data(std::vector<client_data>* client_data, char* serialized_data);
int deserialize_client_data(std::vector<client_data>* client_data, char* serialized_data);
std::string ToString(int value);

#endif