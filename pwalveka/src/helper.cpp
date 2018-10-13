#include <stdio.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include "./helper.h"

/* Function that returns the string with the UBIT name agreeing to the academic integrity. Project will not be graded without the AUTHOR command. */
int author_command(char *result_string) {
  sprintf(result_string, "I, pwalveka, have read and understood the course academic integrity policy.");
  return 0;
}

/* Function that gets the hostname of the device first
    Then it resolves the hostname to corresponding IP Address*/
int ip_command(char *device_hostname, char *device_ip_address)
{
    struct hostent *host_info;
    struct in_addr **addr_list;
    int i;  
    int result = gethostname(device_hostname,2048);
    host_info = gethostbyname(device_hostname);
    addr_list = (struct in_addr **) host_info->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {
        // Return the first one;
        strcpy(device_ip_address , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return result;
}

/* Function to tokenize the command by splitting it over spaces.*/
int tokenize_command(std::vector<char*>* tokenized_command, char* cmd){
  char *token = strtok(cmd, " ");
  while(token) {
    tokenized_command->push_back(token);
    token = strtok(NULL, " ");
  }
  return 0;
}

/* Function to serialize client data structure into char* so that it can be sent over socket.*/
int serialize_client_data(std::vector<client_data>* client_data, char* serialized_data) {
  int client_data_size = (*client_data).size();
  std::string temporary_string = std::string("");
  for(int i = 0; i < client_data_size; i++ ){
    // Add client name.
    std::string retrieve_char_array = std::string((*client_data)[i].client_name);
    temporary_string.append(retrieve_char_array);
    // Add a delimeter.
    temporary_string.append(std::string(","));
    // Add client name.
    std::string retrieve_char_array2 = std::string((*client_data)[i].client_ip_address);
    temporary_string.append(retrieve_char_array2);
    // Add a delimeter.
    temporary_string.append(std::string(","));
    // Add the client port.
    temporary_string.append(ToString((*client_data)[i].client_port));
    // Add a delimieter.
    temporary_string.append(std::string(","));
    // Add the client status.
    temporary_string.append(ToString((*client_data)[i].status));
    // Add the end of record delimieter.
    temporary_string.append(std::string(";"));
  }
  strcpy(serialized_data, temporary_string.c_str());
  return 0;
}

/* Function to deserializa the data structure from char* into the client_data structure that is recieved over the socket.*/
int deserialize_client_data(std::vector<client_data>* client_data, char* serialized_data) {
  std::vector<char*> temporary_vector;
  char* token = strtok((char*)serialized_data, ";");
  while(token) {
    temporary_vector.push_back(token);
    token = strtok(NULL, ";");
  }
  int size = temporary_vector.size();
  for(int i = 0; i < size; i++ ){
    struct client_data temp_data;
    char* client_details = temporary_vector[i];
    char* item = strtok(client_details, ",");
    strcpy(temp_data.client_name,item);
    item = strtok(NULL, ",");
    strcpy(temp_data.client_ip_address, item);
    item = strtok(NULL, ",");
    temp_data.client_port = atoi(item);
    item = strtok(NULL, ",");
    temp_data.status = atoi(item);
    (*client_data).push_back(temp_data);
  }
  return 0;
}

/* Convert integer to std::string because CSE server doesn not have std:string.to_string*/
std::string ToString(int value) {
  std::stringstream stream;
  stream << value;
  return stream.str();
}

/* Function to print the client_data vector.*/
int print_client_data_vector(std::vector<client_data>* client_details) {
  int size = (*client_details).size();
  for(int i=0; i < size; i++) {
    // printf("Client No %d:\n", i);
    // printf("Hostname:%s\n", (*client_details)[i].client_name);
    // printf("IP Address:%s\n", (*client_details)[i].client_ip_address);
    // printf("Port:%d\n", (*client_details)[i].client_port);
    // printf("Status:%d\n", (*client_details)[i].status);
    printf("%-5d%-35s%-20s%-8d\n", i+1, (*client_details)[i].client_name, (*client_details)[i].client_ip_address, (*client_details)[i].client_port);
  }
  return 0;
}

/* Comparator function to sort the vector based on the port number.*/
bool comparator_client_data_port(const client_data &record_a, const client_data &record_b) {
  if(record_a.client_port == record_b.client_port) {
    return false;
  }
  return record_a.client_port < record_b.client_port;
}

/* Function to print the output of the statistics command from the vector.*/
int print_statistics(std::vector<client_data>* client_details) {
  int size = (*client_details).size();
  for(int i=0; i < size; i++) {
    printf("%-5d%-35s%-8d%-8d%-8s\n", i+1, (*client_details)[i].client_name, (*client_details)[i].message_sent, (*client_details)[i].message_recieved, (*client_details)[i].status);
  }
  return 0;
}

/* Function to decode the string status from the int in the vector.*/
int decode_client_status(int status, char* decoded_string) {
  if(status == 0) {
    strcpy(decoded_string, "logged-out");
  } else if(status == 1){
    strcpy(decoded_string, "logged-in");
  }
  return 0;
}

/* Given the socket descriptor, find the index of the client in the vector of all clients.*/
int get_client_data_from_sock(int sock_desc, std::vector<client_data>* list_of_clients, int* index) {
  int size = static_cast<int>((*list_of_clients).size());
  for (int i = 0; i < size; i++) {
    if(sock_desc == (*list_of_clients)[i].sock_decriptor) {
      // Got the client.
      *index = i;
      printf("Got the index:%d\n", *index);
      break;
    }
  }
  return 0;
}

/* Given the ip address, find the index of the client in the vector of all clients. */
int get_client_data_from_ip(const char ip_address[100], std::vector<client_data>* list_of_clients, int* index) {
  printf("Finding function got this ip address:%s\n", ip_address);
  int size = static_cast<int>((*list_of_clients).size());
  for (int i = 0; i < size; i++) {
    if(strcmp(ip_address, (*list_of_clients)[i].client_ip_address) == 0) {
      // Got the client.
      *index = i;
      break;
    }
  }
  return 0;
}

/* Function that tells if the ip address is valid.*/
int is_ip_address_valid(char ip_address[100]) {
  char dummy_address[100];
  return inet_pton(AF_INET, ip_address, dummy_address);;
}

/* Function to tell if the port number is valid.*/
int is_port_number_valid(char port_number[100]) {
  char *marker;
  int status = strtol(port_number, &marker, 10);
  if (*marker == 0) {
    // Marker not set to a string character, is a valid integer.
    if (status > 0 && status < 65536) {
      return 1;
    } else {
      return 0;
    }
  } else {
    // Has a character that is not valid int. 
    return 0;
  }
}

/* Function that will create a string of a message containing spaces.*/
int build_message_string(char* message, std::vector<char*>* tokenized_command) {
  int size = static_cast<int>((*tokenized_command).size());
  for(int i = 2; i < size; i++) {
    strcat(message, (*tokenized_command)[i]);
  }
  return 0;
}

/* Function that will get the message out of the tokenized array sent to the client.*/
int break_message_string(char* message, std::vector<char*>* tokenized_command) {
  int size = static_cast<int>((*tokenized_command).size());
  for(int i = 1; i < size; i++) {
    strcat(message, (*tokenized_command)[i]);
  }
  return 0;
}