#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#define LINE_SIZE 1000
#define MAX_COMMANDS_NUMBER 30
#define MAX_ARGS_NUMBER 5

char **split_line(int max_size, char *line, char *delim) {
    char **splitted = malloc(sizeof(char*)*max_size);

    int i = 0;
    splitted[i++] = strtok(line, delim);
    for(; i < max_size; i++) {
        splitted[i] = strtok(NULL, delim);
        if(splitted[i] == NULL)
            break;
    }
    for(; i < max_size; i++)
        splitted[i] = NULL;
    return splitted;
}

char ***parse_line(char *line, char *command_delim, char *arg_delim) {
    char ***output = malloc(sizeof(char*)*MAX_COMMANDS_NUMBER);
    char **splitted_line = split_line(MAX_COMMANDS_NUMBER, line, command_delim);

    int i;
    for(i = 0; i < MAX_COMMANDS_NUMBER; i++) {
        if(splitted_line[i] != NULL)
            output[i] = split_line(MAX_ARGS_NUMBER, splitted_line[i], arg_delim);
        else
            output[i] = NULL;
    }
    free(splitted_line);

    return output;
}

void child_task(int fd_in[2], int fd_out[2], char *command[]) {
    if(fd_in[0] >= 0) {
        close(fd_in[1]);
        dup2(fd_in[0], STDIN_FILENO);
    }

    if(fd_out[0] >= 0) {
        close(fd_out[0]);
        dup2(fd_out[1], STDOUT_FILENO);
    }

    execvp(command[0], command);    
    
    if(fd_in[0] >= 0)
        close(fd_in[0]);

    if(fd_out[0] >= 0)
        close(fd_out[1]);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    char *command_delim = "|\n";
    char *args_delim = " \n";

    char line[LINE_SIZE];

    printf("\nZAD1 INTERPRETER, 'Q' - exit\n\n");

    while(1) {
        printf("Podaj polecenie $$ ");
        if (fgets(line, LINE_SIZE, stdin) == NULL) {
            fprintf(stderr, "Błąd odczytu polecenia");
            exit(EXIT_FAILURE);
        }

        if(line[0] == 'Q')
            exit(EXIT_SUCCESS);

        char ***commands = parse_line(line, command_delim, args_delim);

        int command = 0;
        int fds[MAX_COMMANDS_NUMBER][2];
        fds[command][0] = fds[command][1] = -1;
        command++;

        pid_t pid;
        for (; command < MAX_COMMANDS_NUMBER; command++) {
            if (commands[command] == NULL) {
                fds[command][0] = fds[command][1] = -1;
            } else {
                if (pipe(fds[command]) == -1) {
                    perror("Pipe error: ");
                    exit(EXIT_FAILURE);
                }
            }

            if ((pid = fork()) > 0) {
                close(fds[command - 1][1]);
                waitpid(pid, NULL, WUNTRACED);
            } else if (pid == 0) {
                child_task(fds[command - 1], fds[command], commands[command - 1]);
            } else {
                perror("Fork error: ");
                exit(EXIT_FAILURE);
            }
            if(commands[command] == NULL)
                break;
        }
    }
}