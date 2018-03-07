#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/sem.h>
#include <time.h>

#include "fifo.h"
#include "common_data.h"




#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
#else
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};
#endif


int waiting_room_id;
Fifo *waiting_room;
int sem_id;

void parse_args(int *places, const int *argc, char **argv) {
    if(*argc != 2) {
        fprintf(stderr, "Nieprawdiłowa liczba argumentów");
        exit(EXIT_FAILURE);
    }
    *places = atoi(argv[1]);
}

void sig_int_handler(int sig) {
    printf("Zamykam zakład\n");
    exit(EXIT_SUCCESS);
}

Fifo* create_waiting_room(int *wr_id, int size) {
    if(size > FIFO_MAX_SIZE) {
        fprintf(stderr, "size error");
        exit(EXIT_FAILURE);
    }

    key_t fifo_key = ftok(PATH, SHARED_MEMORY_ID);
    if(fifo_key == -1) {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    *wr_id = shmget(fifo_key, sizeof(Fifo), IPC_CREAT | 0666);
    if(*wr_id == -1) {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    Fifo *wr = (Fifo*)shmat(*wr_id, NULL, 0);
    if((void *)wr == (void *)(-1)) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }
    fifo_init(wr, size);
    return wr;
}

void delete_waiting_room() {
    shmdt(waiting_room);
    shmctl(waiting_room_id, IPC_RMID, NULL);
}

void delete_semaphore() {
    semctl(sem_id, -1, IPC_RMID);
}

void delete_all() {
    delete_waiting_room();
    delete_semaphore();
}

void sem_set_val(int sem_id, short sem_num, int val) {
    union semun semopts;
    semopts.val = val;
    if(semctl(sem_id, sem_num, SETVAL, semopts) == -1) {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}

int create_semaphore(int places) {
    key_t sem_key = ftok(PATH, SEMAPHORE_ID);
    if(sem_key == -1) {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(sem_key, 3 + places, IPC_CREAT | 0666);
    if(sem_id == -1) {
        perror("semget error");
        exit(EXIT_FAILURE);
    }

    sem_set_val(sem_id, CLIENT_LEFT_SEM, 0);
    sem_set_val(sem_id, WAITING_ROOM_SEM, 1);
    sem_set_val(sem_id, BARBER_SLEEPING_SEM, 0);

    int c;
    for(c = 0; c < places; c++) {
        sem_set_val(sem_id, 3 + c, 0);
    }

    return sem_id;
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

int get_sem_value(int sem_id, unsigned short sem_num) {
    int val = semctl(sem_id, sem_num, GETVAL, 0);
    if(val == -1) {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
    return val;
}

void print_info(char *text, pid_t pid) {
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    printf("%s - %d - %ld.%ld\n", text, pid, tspec.tv_sec, tspec.tv_nsec);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (atexit(delete_all) != 0) {
        fprintf(stderr, "atexit error\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, &sig_int_handler) == SIG_ERR) {
        fprintf(stderr, "signal error\n");
        exit(EXIT_FAILURE);
    }

    int places;
    parse_args(&places, &argc, argv);

    waiting_room = create_waiting_room(&waiting_room_id, places);
    sem_id = create_semaphore(places + 1);

    while (1) {
        lock(sem_id, WAITING_ROOM_SEM);
        if (fifo_is_empty(waiting_room)) {
            unlock(sem_id, WAITING_ROOM_SEM);
            unlock(sem_id, BARBER_SLEEPING_SEM);
            print_info("zaśnięcie golibrody", 0);
            sem_operation(sem_id, BARBER_SLEEPING_SEM, 0);
        } else {
            Client client = fifo_remove(waiting_room);
            unlock(sem_id, WAITING_ROOM_SEM);

            print_info("rozpoczęcie strzyżenia", client.pid);
            unlock(sem_id, 3 + client.number);
            lock(sem_id, CLIENT_LEFT_SEM);
            print_info("zakończenie strzyżenia", client.pid);
        }
    }

    return 0;
}