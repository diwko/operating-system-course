#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/resource.h>

void interpret_env(char *line){
    char *name = strtok(line, " \t\n");
    char *value = strtok(NULL, "\n");

    if(value != NULL)
        setenv(name, value, 1);
    else
        unsetenv(name);
}

void interpret_command(char *line, struct rlimit *cpu_limit, struct rlimit *as_limit){
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
        perror("Cannot create process");
        exit(-1);
    } else if(pid == 0) {
        if(setrlimit(RLIMIT_CPU, cpu_limit) == -1) {
            perror("Problem with setting cpu limit");
            exit(EXIT_FAILURE);
        }
        if(setrlimit(RLIMIT_AS, as_limit) == -1) {
            perror("Problem with setting virtual memory limit");
            exit(EXIT_FAILURE);
        }

        if(execvp(args[0], args) == -1){
            fprintf(stderr, "Cannot exec: %s\n", command_name);
        }
    } else {
        int status;
        struct rusage rusage_stats;
        if(wait4(pid, &status, 0, &rusage_stats) == -1){
            fprintf(stderr, "Something went wrong\n");
        }

        if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
            fprintf(stderr, "Error while executing: %s\n", command_name);
            exit(EXIT_FAILURE);
        } else if(WIFSIGNALED(status)) {
            fprintf(stderr, "Process: %s terminated by signal: %d\n", line, WTERMSIG(status));
        } else {
            fprintf(stderr, "Process complete: %s\n", line);
        }

        //getrusage(RUSAGE_CHILDREN, &rusage_stats);
        fprintf(stderr, "%s%lu%s%lu\t%s%lu%s%lu\t%s%lu\n",
                " user_time[s]: ", rusage_stats.ru_utime.tv_sec, ".", rusage_stats.ru_utime.tv_usec,
                " system_time[s]: ", rusage_stats.ru_stime.tv_sec, ".", rusage_stats.ru_stime.tv_usec,
                " max_memory[kB]: ", rusage_stats.ru_maxrss);
    }
}

void parse_line(char *line, struct rlimit *cpu_limit, struct rlimit *as_limit){
    if(line[0] == '#') {
        interpret_env(&line[1]);
    } else {
        interpret_command(line, cpu_limit, as_limit);
    }
}

void parse_limits(char *argv[], struct rlimit *cpu_limit, struct rlimit *as_limit) {
    cpu_limit->rlim_max = cpu_limit->rlim_cur =  (rlim_t)(atoi(argv[2]));
    as_limit->rlim_max = as_limit->rlim_cur = (rlim_t)(atoi(argv[3])*1024*1024);
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Wrong arguments\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    struct rlimit cpu_limit;
    struct rlimit as_limit;
    parse_limits(argv, &cpu_limit, &as_limit);

    size_t line_size = 0;
    char *line = NULL;
    while(getline(&line, &line_size, file) != -1){
        parse_line(line, &cpu_limit, &as_limit);
    }

    free(line);
    fclose(file);
    return 0;
}
