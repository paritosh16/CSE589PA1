#ifndef HELPER_H_
#define HELPER_H_

#include <vector>

int author_command(char *result_string);
int ip_command(char *device_hostname, char *device_ip_address);
int tokenize_command(std::vector<char*>* tokenized_command, char* cmd);

#endif