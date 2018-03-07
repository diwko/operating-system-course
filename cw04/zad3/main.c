#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

pid_t child_pid = -1;

void send_signals_kill(int pid, int sig, int times) {
    int i;
    for(i = 0; i < times; i++) {
        printf("Wysłano sygnał %d nr. %d\n", sig, i + 1);
        kill(pid, sig);
    }
}

void send_signals_queue(int pid, int sig, int times) {
    int i;
    for(i = 0; i < times; i++) {
        union sigval val;
        printf("Wysłano sygnał %d nr. %d\n", sig, i + 1);
        sigqueue(pid, sig, val);
    }
}

void send_signals(int type, int pid, int times) {
    switch(type) {
        case 1:
            send_signals_kill(pid, SIGUSR1, times);
            send_signals_kill(pid, SIGUSR2, 1);
            break;
        case 2:
            send_signals_queue(pid, SIGUSR1, times);
            send_signals_queue(pid, SIGUSR2, 1);
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
    if(child_pid > 0)
        kill(child_pid, SIGKILL);
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
    if(argc != 3){
        fprintf(stderr, "Nieprawidłowa liczba argumentów\n");
        exit(EXIT_FAILURE);
    }

    int L = atoi(args[1]);
    int type = atoi(args[2]);

    signal(SIGINT, handler_int);
    child_pid = fork();
    if(child_pid > 0) {
        sleep(1);
        signal(SIGUSR1, handler_usr1);
        signal(SIGRTMIN, handler_usr1);
        signal(SIGRTMAX, handler_usr1);
        send_signals(type, child_pid, L);
        int status;
        waitpid(child_pid, &status, 0);
        if(WIFEXITED(status))
            return 0;
    } else if(child_pid == 0) {
        execl("./child.run", "child", args[1], NULL);
    } else {
        fprintf(stderr, "Nie udało się utworzyć procesu\n");
        return EXIT_FAILURE;
    }


    return 0;
}