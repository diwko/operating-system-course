#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

void handler_usr1(int sig) {
    int sig_rand = rand() % 32;
    kill(getppid(), SIGRTMIN + sig_rand);
}

int main() {
    srand(time(NULL));
    sleep(rand() % 11);

    signal(SIGUSR1, handler_usr1);

    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    kill(getppid(), SIGUSR1);
    pause();
    clock_gettime(CLOCK_REALTIME, &stop);

    return (int)(stop.tv_nsec - start.tv_nsec);
}