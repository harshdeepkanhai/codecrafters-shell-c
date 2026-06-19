#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  while (1) {
    printf("$ ");


    // User Input
    char cmd[128];
    if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
      return 0;
    }


  
    // Remove the trailing newline
    cmd[strcspn(cmd, "\n")] = '\0';

    if (strcmp(cmd, "exit") == 0) {
      break;
    } else if (strncmp(cmd, "echo ", 5) == 0) {
      printf("%s\n", cmd + 5);
    } else {
      printf("%s: command not found\n", cmd);
    }
  }

  return 0;
}
