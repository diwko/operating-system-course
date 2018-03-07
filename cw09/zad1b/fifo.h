#ifndef FIFO_H
#define FIFO_H

#include <fcntl.h>

#define READ_LOCK 1
#define WRITE_LOCK 2

typedef struct {
    int operation;
    pthread_cond_t *condition;
} Data;

typedef struct {
    int max_size;
    Data **data;

    int size;
    int first;
    int last;
} Fifo;

void fifo_init(Fifo *fifo, int size);
void fifo_delete(Fifo *fifo);
int fifo_add(Data *data, Fifo *fifo);
Data *fifo_remove(Fifo *fifo);
Data *fifo_next(Fifo *fifo);
int fifo_is_empty(Fifo *fifo);
int fifo_is_full(Fifo *fifo);



#endif //FIFO_H
