#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // access, X_OK, fork, execv, _exit
#include <limits.h>     // PATH_MAX
#include <sys/wait.h>   // waitpid

#define MAX_ARGS 64

static const char *builtins[] = {"echo", "exit", "type", "pwd", "cd"};
static const size_t builtin_count = sizeof(builtins) / sizeof(builtins[0]);

static int is_builtin(const char *name) {
    for (size_t i = 0; i < builtin_count; i++) {
        if (strcmp(name, builtins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Find an executable named `name` in PATH. On success, fill `out` and return 1.
static int find_in_path(const char *name, char *out, size_t out_size) {
    const char *path = getenv("PATH");
    if (path == NULL) {
        return 0;
    }
    char *copy = strdup(path);           // strtok_r mutates its input
    if (copy == NULL) {
        return 0;
    }
    int found = 0;
    char *saveptr;
    for (char *dir = strtok_r(copy, ":", &saveptr);
         dir != NULL;
         dir = strtok_r(NULL, ":", &saveptr)) {
        snprintf(out, out_size, "%s/%s", dir, name);
        if (access(out, X_OK) == 0) {    // exists AND executable
            found = 1;
            break;
        }
    }
    free(copy);
    return found;
}

// Split `line` into a NULL-terminated argv. Mutates `line`. Returns argc.
static int tokenize(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *saveptr;
    for (char *tok = strtok_r(line, " ", &saveptr);
         tok != NULL && argc < max_args - 1;
         tok = strtok_r(NULL, " ", &saveptr)) {
        argv[argc++] = tok;
    }
    argv[argc] = NULL;
    return argc;
}

static void run_external(char *line) {
    char *argv[MAX_ARGS];
    if (tokenize(line, argv, MAX_ARGS) == 0) {
        return;                          // empty line
    }

    char path[PATH_MAX];
    if (!find_in_path(argv[0], path, sizeof(path))) {
        printf("%s: command not found\n", argv[0]);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {                      // child
        execv(path, argv);
        perror("execv");                 // reached only if execv fails
        _exit(127);
    } else if (pid > 0) {                // parent
        int status;
        waitpid(pid, &status, 0);        // wait so output stays ordered
    } else {
        perror("fork");
    }
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
    // pwd
    if (strcmp(line, "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        return;
    }
    // cd <path>
    if (strncmp(line, "cd ", 3) == 0) {
        const char *dir = line + 3;
        if (strcmp(dir, "~") == 0) {
            const char *home = getenv("HOME");
            if (home != NULL) {
                dir = home;
            }
        }
        if (chdir(dir) != 0) {
            printf("cd: %s: No such file or directory\n", dir);
        }
        return;
    }
    // external command
    run_external(line);
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
