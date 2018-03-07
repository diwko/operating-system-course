#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

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
int sem_number;
sem_t **semaphores;

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

Fifo* create_waiting_room(int *shm_id, int size) {
    if(size > FIFO_MAX_SIZE) {
        fprintf(stderr, "size error");
        exit(EXIT_FAILURE);
    }

    *shm_id = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
    if(*shm_id == -1) {
        perror("shm_open error");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(*shm_id, sizeof(Fifo))) {
        perror("ftruncate error");
        exit(EXIT_FAILURE);
    }

    Fifo *fifo_shm = mmap(NULL, sizeof(Fifo), PROT_READ | PROT_WRITE, MAP_SHARED, *shm_id, 0);
    if((void *)fifo_shm == (void *)(-1)) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }
    fifo_init(fifo_shm, size);

    return fifo_shm;
}

void create_sem_name(char *buffer, int b) {
    sprintf(buffer, "%s_%d", SEM_NAME, b);
}

sem_t **create_semaphore(int *sem_number, int places) {
    *sem_number = 3 + places;
    sem_t **sems = malloc(sizeof(sem_t*)*(*sem_number));

    char name_buf[30];

    create_sem_name(name_buf, CLIENT_LEFT_SEM);
    sems[CLIENT_LEFT_SEM] = sem_open(name_buf, O_CREAT | O_RDWR, 0666, 0);
    if(sems[CLIENT_LEFT_SEM] == SEM_FAILED) {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }
    create_sem_name(name_buf, WAITING_ROOM_SEM);
    sems[WAITING_ROOM_SEM] = sem_open(name_buf, O_CREAT | O_RDWR, 0666, 1);
    if(sems[WAITING_ROOM_SEM] == SEM_FAILED) {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }    
    create_sem_name(name_buf, BARBER_SLEEPING_SEM);
    sems[BARBER_SLEEPING_SEM] = sem_open(name_buf, O_CREAT | O_RDWR, 0666, 0);
    if(sems[BARBER_SLEEPING_SEM] == SEM_FAILED) {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }

    int c;
    for(c = 3; c < *sem_number; c++) {
        create_sem_name(name_buf, c);
        sems[c] = sem_open(name_buf, O_CREAT | O_RDWR, 0666, 0);
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

void print_info(char *text, pid_t pid) {
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    printf("%s - %d - %ld.%ld\n", text, pid, tspec.tv_sec, tspec.tv_nsec);
    fflush(stdout);
}

void close_waiting_room() {
    if(munmap(waiting_room, sizeof(Fifo)) == -1)
        perror("munmap error");
    if(shm_unlink(SHM_NAME) == -1)
        perror("shm_unlink error");
}

void close_semaphore(short sem_id) {
    if(sem_close(semaphores[sem_id]) == -1)
        perror("sem_close error");

    char name_buf[30];
    create_sem_name(name_buf, sem_id);
    if(sem_unlink(name_buf) == -1)
        perror("sem_unlink error");
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

    if (signal(SIGINT, &sig_int_handler) == SIG_ERR) {
        fprintf(stderr, "signal error\n");
        exit(EXIT_FAILURE);
    }

    int places;
    parse_args(&places, &argc, argv);

    waiting_room = create_waiting_room(&waiting_room_id, places);
    semaphores = create_semaphore(&sem_number, places + 1);

    while (1) {
        lock(WAITING_ROOM_SEM);
        if (fifo_is_empty(waiting_room)) {
            unlock(WAITING_ROOM_SEM);
            unlock(BARBER_SLEEPING_SEM);
            print_info("zaśnięcie golibrody", 0);
            while(get_sem_value(BARBER_SLEEPING_SEM) == 1);
        } else {
            Client client = fifo_remove(waiting_room);
            unlock(WAITING_ROOM_SEM);

            print_info("rozpoczęcie strzyżenia", client.pid);
            unlock(3 + client.number);
            lock(CLIENT_LEFT_SEM);
            print_info("zakończenie strzyżenia", client.pid);
        }
    }

    return 0;
}