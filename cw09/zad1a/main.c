#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>

int data_size;
int *data;
int flag;

sem_t writing;
sem_t data_sem;

int working_readers;

long gettid() {
    return syscall(SYS_gettid);
}

void sem_init_safe(sem_t *sem, int pshared, unsigned int value) {
    if(sem_init(sem, pshared, value) == -1) {
        perror("sem_init error");
        exit(EXIT_FAILURE);
    }
}

void sem_post_safe(sem_t *sem) {
    if(sem_post(sem) == -1) {
        perror("sem_post error");
        exit(EXIT_FAILURE);
    }
}

void sem_wait_safe(sem_t *sem) {
    if(sem_wait(sem) == -1) {
        perror("sem_wait error");
        exit(EXIT_FAILURE);
    }
}

void block(sem_t *sem) {
    sem_wait_safe(sem);
}

void unblock(sem_t *sem) {
    sem_post_safe(sem);
}

void pthread_create_safe(pthread_t *thread, const pthread_attr_t *attr,
                         void *(*start_routine) (void *), void *arg) {
    if(pthread_create(thread, attr, start_routine, arg) != 0) {
        fprintf(stderr, "pthread create error\n");
    }

}

int *init_data(int data_size) {
    int *data = calloc((size_t)data_size, sizeof(int));
    if(data == NULL) {
        perror("calloc error");
        exit(EXIT_FAILURE);
    }
    return data;
}

void print_data(int *data, int size) {
    int i;
    for(i = 0; i < size; i++) {
        printf("|%d", data[i]);
    }
    printf("|\n");
}

void write_numbers(int *data, int size, int max_replaced, int flag) {
    int number = rand()%max_replaced + 1;

    printf("TID: %ld - START WRITING\n", gettid());

    for(; number > 0; number--) {
        int index = rand()%size;
        int value = rand()%data_size;
        data[index] = value;

        if(flag)
            printf("TID: %ld - WRITE %d -> %d\n", gettid(), index, value);
    }

    printf("TID: %ld - END WRITING\n", gettid());
    //print_data(data, size);
}

void find_numbers(int *data, int size, int divider, int flag) {
    printf("TID: %ld - START READING (divider: %d)\n", gettid(), divider);

    int found = 0;
    int i;
    for(i = 0; i < size; i++) {
        if(data[i]%divider == 0) {
            found++;
            if(flag)
                printf("TID: %ld - FOUND %d -> %d\n", gettid(), i, data[i]);
        }
    }

    printf("TID: %ld - END READING (found: %d)\n", gettid(), found);
}


void read_lock() {
    block(&data_sem);
    working_readers++;
    if(working_readers == 1)
        block(&writing);
    unblock(&data_sem);
}

void read_unlock() {
    block(&data_sem);
    working_readers--;
    if(working_readers == 0)
        unblock(&writing);
    unblock(&data_sem);
}

void write_lock() {
    block(&writing);
}

void write_unlock() {
    unblock(&writing);
}

void *writer_action(void *arg) {
    while(1) {
        write_lock();
        write_numbers(data, data_size, data_size, flag);
        write_unlock();
        //sleep(1);
    }
}

void *reader_action(void *arg) {
    while(1) {
        read_lock();
        find_numbers(data, data_size, rand()%data_size + 1, flag);
        read_unlock();
        //sleep(1);
    }
}

pthread_t *create_writers(int number) {
    pthread_t *writers = malloc(sizeof(pthread_t)*number);
    if(writers == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    int i;
    for(i = 0; i < number; i++) {
        pthread_create_safe(&writers[i], NULL, writer_action, NULL);
    }
    return writers;
}

pthread_t *create_readers(int number) {
    pthread_t *readers = malloc(sizeof(pthread_t)*number);
    if(readers == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    int i;
    for(i = 0; i < number; i++) {
        pthread_create_safe(&readers[i], NULL, reader_action, NULL);
    }
    return readers;
}

void parse_args(int argc, char *argv[], int *writers, int *readers, int *data_size, int *flag) {
    if(argc == 4 || argc == 5) {
        *writers = atoi(argv[1]);
        *readers = atoi(argv[2]);
        *data_size = atoi(argv[3]);
        *flag = 0;
        if (argc == 5 && strcmp(argv[4], "-i") == 0)
            *flag = 1;
    } else  {
        fprintf(stderr, "Incorrect arguments: WRITERS READERS DATA_SIZE [-i]\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int writers_number;
    int readers_number;

    parse_args(argc, argv, &writers_number, &readers_number, &data_size, &flag);

    sem_init_safe(&writing, 0, 1);
    sem_init_safe(&data_sem, 0, 1);
    working_readers = 0;

    srand(time(NULL));
    data = init_data(data_size);

    pthread_t *writers = create_writers(writers_number);
    pthread_t *readers = create_readers(readers_number);
    while(1);

    free(readers);
    free(writers);
    free(data);

    sem_destroy(&data_sem);
    sem_destroy(&writing);

    return 0;
}