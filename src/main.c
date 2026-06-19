#include <stdio.h>
#include <stdlib.h>   // getenv, exit, atoi, free
#include <string.h>   // strcmp, strncmp, strcspn, strtok, strdup
#include <unistd.h>   // access, X_OK
#include <limits.h>   // PATH_MAX

static const char *builtins[] = {"echo", "exit", "type"};
static const size_t builtin_count = sizeof(builtins) / sizeof(builtins[0]);

static int is_builtin(const char *name) {
    for (size_t i = 0; i < builtin_count; i++) {
        if (strcmp(name, builtins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// If an executable named `name` is found in PATH, write its full path into
// `out` and return 1. Otherwise return 0.
static int find_in_path(const char *name, char *out, size_t out_size) {
    const char *path = getenv("PATH");
    if (path == NULL) {
        return 0;
    }

    char *copy = strdup(path);          // strtok mutates its input, so copy first
    if (copy == NULL) {
        return 0;
    }

    int found = 0;
    for (char *dir = strtok(copy, ":"); dir != NULL; dir = strtok(NULL, ":")) {
        snprintf(out, out_size, "%s/%s", dir, name);
        if (access(out, X_OK) == 0) {   // exists AND is executable
            found = 1;
            break;                      // first match wins
        }
    }

    free(copy);
    return found;
}

static void execute(char *line) {
    // exit [code]
    if (strcmp(line, "exit") == 0 || strncmp(line, "exit ", 5) == 0) {
        exit(line[4] == ' ' ? atoi(line + 5) : 0);
    }
    // echo <text>
    if (strncmp(line, "echo ", 5) == 0) {
        printf("%s\n", line + 5);
        return;
    }
    // type <name>
    if (strncmp(line, "type ", 5) == 0) {
        const char *arg = line + 5;
        char path[PATH_MAX];
        if (is_builtin(arg)) {
            printf("%s is a shell builtin\n", arg);
        } else if (find_in_path(arg, path, sizeof(path))) {
            printf("%s is %s\n", arg, path);
        } else {
            printf("%s: not found\n", arg);
        }
        return;
    }
    // unknown command
    printf("%s: command not found\n", line);
}

int main(void) {
    setbuf(stdout, NULL);

    char line[128];
    while (1) {
        printf("$ ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        line[strcspn(line, "\n")] = '\0';
        execute(line);
    }
    return 0;
}
