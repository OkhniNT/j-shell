#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void jsh_loop(void);
char *jsh_read_line(void);
char **jsh_split_line(char *line);
int jsh_launch(char **args);
int jsh_execute(char **args);

int jsh_cd(char **args);
int jsh_help(char **args);
int jsh_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &jsh_cd,
    &jsh_help,
    &jsh_exit
};

int jsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int main(int argc, char **argv) {
    jsh_loop();

    return EXIT_SUCCESS;
}

/* Main shell functions */

void jsh_loop(void) {
    char *line;
    char **args;
    int status;

    printf("Welcome to J Shell\n");
    do {
        printf("jsh > ");
        line = jsh_read_line();
        args = jsh_split_line(line);
        status = jsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

// TODO: also implement with getline()
#define JSH_RL_BUFSIZE 1024
char *jsh_read_line(void) {
    int bufsize = JSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "jsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If hit EOF, replace with a null character and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If buffer is exceeded, reallocate
        if (position >= bufsize) {
            bufsize += JSH_RL_BUFSIZE;
            buffer = realloc(buffer, sizeof(char) * bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define JSH_TOK_BUFSIZE 64
#define JSH_TOK_DELIM " \t\r\n\a"
char **jsh_split_line(char *line) {
    int bufsize = JSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if (!tokens) {
        fprintf(stderr, "jsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, JSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += JSH_TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);
            if (!tokens) {
                fprintf(stderr, "jsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, JSH_TOK_DELIM);
    }
    tokens[position] = NULL;

    return tokens;
}

int jsh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) { // Child process 
        if (execvp(args[0], args) == -1) {
            perror("jsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) { // Forking error 
        perror("jsh");
    } else { // Parent process 
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int jsh_execute(char **args) {
    if (args[0] == NULL) { // Exit on empty command
        return 1;
    }

    for (int i = 0; i < jsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return jsh_launch(args);
}

/* Builtin Functions */

int jsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "jsh: expected argument to \"cd\"\n");
    } else if (chdir(args[1]) != 0) {
            perror("jsh");
    }
    return 1;
}

int jsh_help(char **args) {
    printf("J Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < jsh_num_builtins(); i++) {
        printf("\t%s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");

    return 1;
}

int jsh_exit(char **args) {
    return 0;
}
