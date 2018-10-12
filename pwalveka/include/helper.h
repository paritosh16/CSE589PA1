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

/* Struct that mantains info for all the info for all the buffer data on the server*/
struct buffered_data{
  char client_send_ip_address[100];
  char client_recieving_ip_address[100];
  char buffered_message[256];
};

int author_command(char *result_string);
int ip_command(char *device_hostname, char *device_ip_address);
int tokenize_command(std::vector<char*>* tokenized_command, char* cmd);
int serialize_client_data(std::vector<client_data>* client_data, char* serialized_data);
int deserialize_client_data(std::vector<client_data>* client_data, char* serialized_data);
std::string ToString(int value);
int print_client_data_vector(std::vector<client_data>* client_details);
bool comparator_client_data_port(const client_data &record_a, const client_data &record_b);
int print_statistics(std::vector<client_data>* client_details);
int decode_client_status(int status, char* decoded_status);
int get_client_data_from_sock(int sock_desc, std::vector<client_data>* list_of_clients, int* index);
int get_client_data_from_ip(const char ip_address[100], std::vector<client_data>* list_of_clients, int* index);
int is_ip_address_valid(char ip_address[100]);
#endif