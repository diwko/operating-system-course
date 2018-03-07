#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int direction = 1;

void sigint_handler(int sig) {
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

void sigtstp_handler(int sig) {
    direction *= -1;
}

void print_letter(int *letter, int direction) {
    printf("%c\n", *letter);
    if(*letter == 65 && direction == 1){
        *letter += 1;
    } else if(*letter == 90 && direction == -1){
        *letter -= 1;
    } else if(*letter > 65 && *letter < 90) {
        *letter += direction;
    }
    sleep(1);
}

int main() {
    int letter = 65;

    struct sigaction s_action;
    s_action.sa_handler = &sigtstp_handler;
    if(sigaction(SIGTSTP, &s_action, NULL) == -1)
        fprintf(stderr, "Błąd ustawienia funkcji przechwytującej sygnał SIGTSTP");

    if(signal(SIGINT, sigint_handler) == SIG_ERR)
        fprintf(stderr, "Błąd ustawienia funkcji przechwytującej sygnał SIGINT");

    while(1){
        print_letter(&letter, direction);
    }

    return 0;
}