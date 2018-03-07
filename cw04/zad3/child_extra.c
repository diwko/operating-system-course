#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int L;

void send_signals_kill(int pid, int sig, int times) {
    int i;
    for(i = 0; i < times; i++) {
        kill(pid, sig);
    }
}

void handler_usr1(int sig) {
    static int sig_number = 0;
    printf("Potomek PID: %d odebrał sygnał %d nr. %d\n", getpid(), sig, ++sig_number);

    if(sig_number == L){
        send_signals_kill(getppid(), SIGUSR1, L);
    }
}

void handler_usr2(int sig) {
    static int sig_number = 0;
    printf("Potomek PID: %d odebrał sygnał %d nr. %d\n", getpid(), sig, ++sig_number);
    exit(0);
}

void handler_rtmin(int sig) {
    static int sig_number = 0;
    printf("Potomek PID: %d odebrał sygnał %d nr. %d\n", getpid(), sig, ++sig_number);
    if(sig_number == L){
        send_signals_kill(getppid(), SIGRTMIN, L);
    }
}

void handler_rtmax(int sig) {
    static int sig_number = 0;
    printf("Potomek PID: %d odebrał sygnał %d nr. %d\n", getpid(), sig, ++sig_number);
    if(sig_number == L){
        send_signals_kill(getppid(), SIGRTMIN, L);
    }
}

int main(int argc, char *args[]) {
    if(argc != 2) {
        fprintf(stderr, "Nieprawidłowa liczba argumentów");
        exit(EXIT_FAILURE);
    }
    L = atoi(args[1]);

    signal(SIGUSR1, handler_usr1);
    signal(SIGUSR2, handler_usr2);
    signal(SIGRTMIN, handler_rtmin);
    signal(SIGRTMAX, handler_rtmax);

    while(1){

    }


    return 0;
}