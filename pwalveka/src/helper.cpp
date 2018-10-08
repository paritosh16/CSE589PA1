#include <stdio.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <string>
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
    printf("%s",(*client_data)[i].client_name);
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
    temp_data.client_port = atoi(item);
    item = strtok(NULL, ",");
    temp_data.status = atoi(item);
    (*client_data).push_back(temp_data);
  }
  printf("Deserialization done.");
  return 0;
}

/* Convert integer to std::string because CSE server doesn not have std:string.to_string*/
std::string ToString(int value) {
  std::stringstream stream;
  stream << value;
  return stream.str();
}