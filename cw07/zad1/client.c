#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include "fifo.h"
#include "common_data.h"

void parse_args(int *clients, int *cuts, const int *argc, char **argv) {
    if(*argc != 3) {
        fprintf(stderr, "Nieprawdiłowa liczba argumentów");
        exit(EXIT_FAILURE);
    }
    *clients = atoi(argv[1]);
    *cuts = atoi(argv[2]);
}

Fifo *get_waiting_room() {
    key_t fifo_key = ftok(PATH, SHARED_MEMORY_ID);
    if(fifo_key == -1) {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    int shm_id = shmget(fifo_key, sizeof(Fifo), 0);
    if(shm_id == -1) {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    Fifo *fifo_shm = shmat(shm_id, NULL, 0);
    if((void *)fifo_shm == (void *)(-1)) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }
    return fifo_shm;
}

int get_semaphore() {
    key_t sem_key = ftok(PATH, SEMAPHORE_ID);
    if(sem_key == -1) {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(sem_key, 0, 0);
    if(sem_id == -1) {
        perror("semget error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

int get_sem_value(int sem_id, unsigned short sem_num) {
    int val = semctl(sem_id, sem_num, GETVAL, 0);
    if(val == -1) {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
    return val;
}

void sem_operation(int sem_id, unsigned short sem_num, short sem_op) {
    struct sembuf buf;
    buf.sem_num = sem_num;
    buf.sem_op = sem_op;
    buf.sem_flg = 0;
    if(semop(sem_id, &buf, 1) == -1) {
        perror("semop error");
        exit(EXIT_FAILURE);
    }
}

void lock(int sem_id, unsigned short sem_num) {
    sem_operation(sem_id, sem_num, -1);
}

void unlock(int sem_id, unsigned short sem_num) {
    sem_operation(sem_id, sem_num, 1);
}

void print_info(char *text) {
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    fprintf(stderr, "%s - %d - %ld.%ld\n", text, getpid(), tspec.tv_sec, tspec.tv_nsec);
}

int main(int argc, char *argv[]) {
    int clients;
    int cuts;
    parse_args(&clients, &cuts, &argc, argv);

    Fifo *waiting_room = get_waiting_room();
    int sem_id = get_semaphore();

    int client;
    for(client = 0; client < clients; client++) {
        pid_t pid = fork();
        if(pid == 0) {
            int visits;
            for(visits = 0; visits < cuts; visits++) {
                lock(sem_id, WAITING_ROOM_SEM);
                if(fifo_is_full(waiting_room)) {
                    unlock(sem_id, WAITING_ROOM_SEM);
                    print_info("opuszczenie zakładu z powodu braku wolnych miejsc w poczekalni");
                } else {
                    int client_number = fifo_add(getpid(), waiting_room);
                    if(get_sem_value(sem_id, BARBER_SLEEPING_SEM) == 1) {
                        lock(sem_id, BARBER_SLEEPING_SEM);
                        print_info("obudzenie golibrody");
                    } else {
                        print_info("zajęcie miejsca w poczekalni");
                    }
                    unlock(sem_id, WAITING_ROOM_SEM);
                    lock(sem_id, 3 + client_number);
                    unlock(sem_id, CLIENT_LEFT_SEM);
                    print_info("opuszczenie zakładu po zakończeniu strzyżenia");
                }
            }
            exit(EXIT_SUCCESS);
        } else if(pid < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}