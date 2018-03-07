#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>

int N;
pid_t *child_procs;


void send_signals_kill(int pid, int sig, int times) {
    int i;
    for(i = 0; i < times; i++) {
        printf("kill - Wysłano sygnał %d nr. %d do PID: %d\n", sig, i + 1, pid);
        kill(pid, sig);
    }
}

void send_signals_queue(int pid, int sig, int times) {
    int i;
    for(i = 0; i < times; i++) {
        union sigval val;
        printf("queue - Wysłano sygnał %d nr. %d do PID: %d\n", sig, i + 1, pid);
        sigqueue(pid, sig, val);
    }
}

void send_signals(int type, int pid, int times) {
    switch(type) {
        case 1:
            send_signals_kill(pid, SIGUSR1, times);
            break;
        case 2:
            send_signals_queue(pid, SIGUSR1, times);
            break;
        case 3:
            send_signals_kill(pid, SIGRTMIN, times);
            send_signals_kill(pid, SIGRTMAX, times);
            break;
        default:
            fprintf(stderr, "Nie rozpoznano typu\n");
            exit(EXIT_FAILURE);
    }
}

void handler_int(int sig){
    int i;
    for(i = 0; i < N; i++){
        if(child_procs[i] > 0)
            kill(child_procs[i], SIGKILL);
    }
    free(child_procs);
    exit(0);
}

void handler_usr1(int sig){
    static int sig_number = 0;
    printf("Rodzic odebrał sygnał %d nr. %d\n", sig, ++sig_number);
}

void handler_rtmin(int sig){
    static int sig_number = 0;
    printf("Rodzic odebrał sygnał %d nr. %d\n", sig, ++sig_number);
}

void handler_rtmax(int sig){
    static int sig_number = 0;
    printf("Rodzic odebrał sygnał %d nr. %d\n", sig, ++sig_number);
}

int main(int argc, char *args[]) {
    int L;
    int T;
    if(argc >= 3){
        N = atoi(args[1]);
        L = atoi(args[2]);
        T = 0;
        if(argc == 4)
            T = atoi(args[3]);
    } else {
        fprintf(stderr, "Nieprawidłowa liczba argumentów\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handler_int);

    child_procs = calloc(N, sizeof(pid_t));

    int i;
    for(i = 0; i < N; i++) {
        pid_t pid = fork();
        if(pid > 0) {
            signal(SIGUSR1, handler_usr1);
            signal(SIGRTMIN, handler_usr1);
            signal(SIGRTMAX, handler_usr1);
            child_procs[i] = pid;
            sleep(1);
        } else if(pid == 0) {
            execl("./child_extra.run", "child", args[1], NULL);
        } else {
            fprintf(stderr, "Nie udało się utworzyć procesu\n");
            return EXIT_FAILURE;
        }
    }

    srand(time(NULL));
    for(i = 0; i < N; i++) {
        if(T != 0)
            sleep(T);
        int random_child = rand() % N;
        while(child_procs[random_child] == -1) {
            random_child = rand() % N;
        }
        send_signals(rand() % 3 + 1, child_procs[random_child], L);

        child_procs[random_child] = -1;
    }

    while(1){

    }

    free(child_procs);

    return 0;
}
