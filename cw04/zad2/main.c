#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>

int n;
int k;
pid_t *waiting_pids;
pid_t *child_procs;

void handler_int(int sig){
    int i;
    for(i = 0; i < n; i++){
        if(child_procs[i] > 0 ){
            kill(child_procs[i], SIGKILL);
        }
    }
    free(child_procs);
    free(waiting_pids);
    exit(0);
}

void handler_usr1(int sig, siginfo_t *info, void *context) {
    static int waiting_procs = 0;
    printf("Otrzymano sygnał: %d od PID: %d\n", sig, info->si_pid);
    waiting_procs++;
    if(waiting_procs <= k) {
        waiting_pids[waiting_procs - 1] = info->si_pid;
    }
    if(waiting_procs == k) {
        int i;
        for(i = 0; i < k; i++) {
            printf("Wysyłam pozwolenie do PID: %d\n", waiting_pids[i]);
            kill(waiting_pids[i], SIGUSR1);
        }
    } else if(waiting_procs > k) {
        printf("Wysyłam pozwolenie do PID: %d\n", info->si_pid);
        kill(info->si_pid, SIGUSR1);
    }
}

void handler_rt(int sig, siginfo_t *info, void *context) {
    printf("Otrzymano sygnał: %d od PID: %d\n", sig, info->si_pid);

    int status;
    waitpid(info->si_pid, &status, 0);

    if(WIFEXITED(status)) {
        printf("Finished PID: %d time: %d ns\n", info->si_pid, WEXITSTATUS(status));
	int i;
        for(i = 0; i < n; i++){
            if(child_procs[i] == info->si_pid){
                child_procs[i] = -1;
                break;
            }
        }
    }
}

void parse_args(int argc, char *args[]) {
    if(argc != 3) {
        fprintf(stderr, "Nieprawidłowa liczba argumenów\n");
        exit(EXIT_FAILURE);
    }
    n = atoi(args[1]);
    k = atoi(args[2]);
}

int main(int argc, char *args[]) {
    srand(time(NULL));
    parse_args(argc, args);

    child_procs = calloc(n, sizeof(pid_t));
    waiting_pids = calloc(k, sizeof(pid_t));

    signal(SIGINT, handler_int);

    struct sigaction action_parent;
    action_parent.sa_flags = SA_SIGINFO;
    action_parent.sa_sigaction = handler_usr1;
    sigaction(SIGUSR1, &action_parent, NULL);
    
    int i;
    for(i = 0; i < 32; i++){
        struct sigaction action;
        action.sa_flags = SA_SIGINFO;
        action.sa_sigaction = handler_rt;
        sigaction(SIGRTMIN + i, &action, NULL);
    }

    pid_t pid;
    for(i = 0; i < n; i++){
        pid = fork();
        if(pid > 0) {
            child_procs[i] = pid;
            sleep(1);
        } else if(pid == 0) {
            execl("./child.run", "child", NULL);
        } else {
            perror("fork");
        }
    }

    int end = 0;
    while(end == 0){
        end = 1;
        for(i = 0; i < n; i++){
            if(child_procs[i] > 0)
                end = 0;
        }
    }

    free(child_procs);
    free(waiting_pids);
    return 0;
}