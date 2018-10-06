#include<stdio.h>

char* author_command() {
  // The result string that will be printed and logged.
	char result_string[1024];
  sprintf(result_string, "AUTHOR:I, pwalveka, have read and understood the course academic integrity policy.\n");
  return result_string;
}