#ifndef FIFO_H
#define FIFO_H

#include <fcntl.h>

#define FIFO_MAX_SIZE 1000

typedef struct {
    pid_t pid;
    int number;
} Client;

typedef struct {
    int max_size;
    Client clients[FIFO_MAX_SIZE];

    int size;
    int first;
    int last;
} Fifo;

int fifo_init(Fifo *fifo, int size);
int fifo_add(pid_t pid, Fifo *fifo);
Client fifo_remove(Fifo *fifo);
int fifo_is_empty(Fifo *fifo);
int fifo_is_full(Fifo *fifo);



#endif //FIFO_H
