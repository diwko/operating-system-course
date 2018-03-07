#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include "fifo.h"
#include "common_data.h"

Fifo *waiting_room;
int sem_number;
sem_t **semaphores;

void parse_args(int *clients, int *cuts, const int *argc, char **argv) {
    if(*argc != 3) {
        fprintf(stderr, "Nieprawdiłowa liczba argumentów");
        exit(EXIT_FAILURE);
    }
    *clients = atoi(argv[1]);
    *cuts = atoi(argv[2]);
}

Fifo *get_waiting_room() {
    int shm_id = shm_open(SHM_NAME, O_RDWR, 0666);
    if(shm_id == -1) {
        perror("shm_open error");
        exit(EXIT_FAILURE);
    }

    Fifo *fifo_shm = mmap(NULL, sizeof(Fifo), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if((void *)fifo_shm == (void *)(-1)) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }
    return fifo_shm;
}

void create_sem_name(char *buffer, int b) {
    sprintf(buffer, "%s_%d", SEM_NAME, b);
}

sem_t **get_semaphore(int *sem_number, int places) {
    *sem_number = 3 + places;
    sem_t **sems = malloc(sizeof(sem_t*)*(*sem_number));

    char name_buf[30];

    create_sem_name(name_buf, CLIENT_LEFT_SEM);
    sems[CLIENT_LEFT_SEM] = sem_open(name_buf, O_RDWR);
    if(sems[CLIENT_LEFT_SEM] == SEM_FAILED) {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }
    create_sem_name(name_buf, WAITING_ROOM_SEM);
    sems[WAITING_ROOM_SEM] = sem_open(name_buf, O_RDWR);
    if(sems[WAITING_ROOM_SEM] == SEM_FAILED) {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }
    create_sem_name(name_buf, BARBER_SLEEPING_SEM);
    sems[BARBER_SLEEPING_SEM] = sem_open(name_buf,O_RDWR);
    if(sems[BARBER_SLEEPING_SEM] == SEM_FAILED) {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }

    int c;
    for(c = 3; c < *sem_number; c++) {
        create_sem_name(name_buf, c);
        sems[c] = sem_open(name_buf, O_RDWR);
        if(sems[3] == SEM_FAILED) {
            perror("sem_open error");
            exit(EXIT_FAILURE);
        }
    }
    return sems;
}

void lock(unsigned short sem_num) {
    if(sem_wait(semaphores[sem_num]) == -1) {
        perror("sem_wait error");
        exit(EXIT_FAILURE);
    }
}

void unlock(unsigned short sem_num) {
    if(sem_post(semaphores[sem_num]) == -1) {
        perror("sem_post error");
        exit(EXIT_FAILURE);
    }
}

int get_sem_value(unsigned short sem_num) {
    int val;
    if(sem_getvalue(semaphores[sem_num], &val) == -1) {
        perror("sem_getvalue error");
        exit(EXIT_FAILURE);
    }
    return val;
}

void print_info(char *text) {
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    fprintf(stderr, "%s - %d - %ld.%ld\n", text, getpid(), tspec.tv_sec, tspec.tv_nsec);
}

void close_waiting_room() {
    if(munmap(waiting_room, sizeof(Fifo)) == -1)
        perror("munmap error");
}

void close_semaphore(short sem_id) {
    if(sem_close(semaphores[sem_id]) == -1)
        perror("sem_close error");
}

void close_semaphores() {
    int s;
    for(s = 0; s < sem_number; s++) {
        close_semaphore(s);
    }
}

void close_all() {
    close_waiting_room();
    close_semaphores();
    free(semaphores);
}

int main(int argc, char *argv[]) {
    if (atexit(close_all) != 0) {
        fprintf(stderr, "atexit error\n");
        exit(EXIT_FAILURE);
    }

    int clients;
    int cuts;
    parse_args(&clients, &cuts, &argc, argv);

    waiting_room = get_waiting_room();
    semaphores = get_semaphore(&sem_number, waiting_room->max_size + 1);

    int client;
    for(client = 0; client < clients; client++) {
        pid_t pid = fork();
        if(pid == 0) {
            int visits;
            for(visits = 0; visits < cuts; visits++) {
                lock(WAITING_ROOM_SEM);
                if(fifo_is_full(waiting_room)) {
                    unlock(WAITING_ROOM_SEM);
                    print_info("opuszczenie zakładu z powodu braku wolnych miejsc w poczekalni");
                } else {
                    int client_number = fifo_add(getpid(), waiting_room);
                    if(get_sem_value(BARBER_SLEEPING_SEM) == 1) {
                        lock(BARBER_SLEEPING_SEM);
                        print_info("obudzenie golibrody");
                    } else {
                        print_info("zajęcie miejsca w poczekalni");
                    }
                    unlock(WAITING_ROOM_SEM);
                    lock(3 + client_number);
                    unlock(CLIENT_LEFT_SEM);
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