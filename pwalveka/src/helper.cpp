#include <stdio.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <vector>

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
