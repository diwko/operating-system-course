#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include "fifo.h"

int flag;

//MONITOR
int data_size;
int *data;
int working_readers = 0;
int working_writers = 0;
pthread_mutex_t monitor;
Fifo fifo;
//END MONITOR

long gettid() {
    return syscall(SYS_gettid);
}

void pthread_mutex_lock_safe(pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) != 0) {
        perror("pthread_mutex_lock errror");
        exit(EXIT_FAILURE);
    }
}

void pthread_mutex_unlock_safe(pthread_mutex_t *mutex) {
    if(pthread_mutex_unlock(mutex) != 0) {
        perror("pthread_mutex_unlock errror");
        exit(EXIT_FAILURE);
    }
}

void pthread_cond_signal_safe(pthread_cond_t *cond) {
    int e = pthread_cond_signal(cond);
    if(e != 0) {
        fprintf(stderr, "pthread_cond_signal error %d\n", e);
        exit(EXIT_FAILURE);
    }
}

void pthread_cond_wait_safe(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int e = pthread_cond_wait(cond, mutex);
    if(e != 0) {
        fprintf(stderr, "pthread_cond_wait error %d\n", e);
        exit(EXIT_FAILURE);
    }
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

void add_fifo(int operation, pthread_cond_t *cond) {
    Data *data = malloc(sizeof(data));
    data->operation = operation;
    data->condition = cond;

    fifo_add(data, &fifo);
}

void next_from_fifo() {
    if(fifo_is_empty(&fifo))
        return;

    Data *data = fifo_remove(&fifo);
    pthread_cond_signal(data->condition);

    if(data->operation == READ_LOCK) {
        while(!fifo_is_empty(&fifo) && fifo_next(&fifo)->operation == READ_LOCK) {
            data = fifo_remove(&fifo);
            pthread_cond_signal_safe(data->condition);

            pthread_cond_destroy(data->condition);
            free(data);
        }
    } else {
        pthread_cond_destroy(data->condition);
        free(data);
    }
}

void read_lock() {
    pthread_mutex_lock_safe(&monitor);
    if(working_writers != 0 || working_readers != 0) {
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
        add_fifo(READ_LOCK, &cond);
        pthread_cond_wait_safe(&cond, &monitor);
    }
    working_readers++;
    pthread_mutex_unlock_safe(&monitor);
}

void read_unlock() {
    pthread_mutex_lock_safe(&monitor);
    working_readers--;
    if(working_readers == 0 && working_writers == 0)
        next_from_fifo();
    pthread_mutex_unlock_safe(&monitor);
}

void write_lock() {
    pthread_mutex_lock_safe(&monitor);
    if(working_writers != 0 || working_readers != 0) {
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
        add_fifo(WRITE_LOCK, &cond);
        pthread_cond_wait_safe(&cond, &monitor);
    }
    working_writers++;
    pthread_mutex_unlock_safe(&monitor);
}

void write_unlock() {
    pthread_mutex_lock_safe(&monitor);
    working_writers--;
    next_from_fifo();
    pthread_mutex_unlock_safe(&monitor);
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

    fifo_init(&fifo, writers_number + readers_number);

    srand(time(NULL));
    data = init_data(data_size);

    pthread_t *writers = create_writers(writers_number);
    pthread_t *readers = create_readers(readers_number);
    while(1);

    fifo_delete(&fifo);
    free(readers);
    free(writers);
    free(data);


    return 0;
}