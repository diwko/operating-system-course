#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

void interpret_env(char *line){
    char *name = strtok(line, " \t\n");
    char *value = strtok(NULL, "\n");

    if(value != NULL)
        setenv(name, value, 1);
    else
        unsetenv(name);
}

void interpret_command(char *line){
    const int MAX_ARGS = 50;
    char *args[MAX_ARGS + 2];
    char command_name[256];
    strcpy(command_name, line);

    args[0] = strtok(line, " \t\n");
    int current_arg = 1;
    while((args[current_arg] = strtok(NULL, " \t\n")) != NULL) {
        current_arg++;
        if(current_arg >= MAX_ARGS - 2) {
            fprintf(stderr, "Too many arguments for command: %s", command_name);
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid = fork();
    if(pid < 0) {
        fprintf(stderr, "Cannot create process\n");
        exit(-1);
    } else if(pid == 0) {
        if(execvp(args[0], args) == -1){
            fprintf(stderr, "Cannot exec: %s\n", command_name);
        }
    } else {
        int status;
        if(waitpid(pid, &status, 0) == -1){
            fprintf(stderr, "Something went wrong\n");
        }
        if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
            fprintf(stderr, "Error while executing: %s\n", command_name);
            exit(EXIT_FAILURE);
        } else if(WIFSIGNALED(status)) {
            fprintf(stderr, "Process: %s terminated by signal: %d\n", line, WTERMSIG(status));
            exit(EXIT_FAILURE);
        } else {
            fprintf(stderr, "Process complete: %s\n", line);
        }
    }
}

void parse_line(char *line){
    if(line[0] == '#') {
        interpret_env(&line[1]);
    } else {
        interpret_command(line);
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong arguments\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    size_t line_size = 0;
    char *line = NULL;
    while(getline(&line, &line_size, file) != -1){
        parse_line(line);
    }

    free(line);
    fclose(file);
    return 0;
}
