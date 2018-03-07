#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    if(argc != 6) {
        fprintf(stderr, "Nieprawidłowa liczba argumentów\n");
        fprintf(stderr, "nazwa_fifo N K R liczba_procesów \n");
        exit(EXIT_FAILURE);
    }

    char *fifo_name = argv[1];
    int N = atoi(argv[2]);
    int procs = atoi(argv[5]);

    char n_in_proc[20];
    sprintf(n_in_proc, "%d", N/procs);

    char n_in_last_proc[20];
    sprintf(n_in_last_proc, "%d", N/procs + N % procs);

    pid_t pid;
    pid_t pid_master;
    pid_master = fork();
    if(pid_master == 0) {
        execlp("./master.run", "./master.run", fifo_name, argv[4], NULL);
        exit(EXIT_SUCCESS);
    } else if(pid_master < 0) {
        perror("fork1 error");
        exit(EXIT_FAILURE);
    }

    sleep(1);
    int i;
    for(i = 0; i < procs - 1; i++) {
        pid = fork();
        if(pid == 0) {
            execlp("./slave.run", "./slave.run", fifo_name, n_in_proc, argv[3], NULL);
            exit(EXIT_SUCCESS);
        } else if(pid < 0){
            fprintf(stderr, "fork2 error");
            exit(EXIT_FAILURE);
        }
    }

    pid = fork();
    if(pid == 0) {
        execlp("./slave.run", "./slave.run", fifo_name, n_in_last_proc, argv[3], NULL);
        exit(EXIT_SUCCESS);
    } else if(pid < 0){
        fprintf(stderr, "fork3 error");
        exit(EXIT_FAILURE);
    }

    while (wait(NULL) > 0);
    return 0;
}