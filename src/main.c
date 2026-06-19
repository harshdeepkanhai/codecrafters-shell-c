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
    } else if (strncmp(cmd, "type ", 5) == 0) {
        char *arg = cmd + 5;
      if (strcmp(arg, "echo") == 0 ||
          strcmp(arg, "exit") == 0 ||
          strcmp(arg, "type") == 0) {
          printf("%s is a shell builtin\n", arg);
      } else {
            printf("%s: not found\n", arg);
      }
    } else {
        printf("%s: command not found\n", cmd);
    }
  }

  return 0;
}
