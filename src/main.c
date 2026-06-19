#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");


  // User Input
  char cmd[32];
  fgets(cmd, sizeof(cmd), stdin);

  
  // Remove the trailing newline
  cmd[strlen(cmd) - 1] = '\0';

  printf("%s: command not found\n", cmd);

  return 0;
}
