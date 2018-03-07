#include <stdlib.h>
#include "fifo.h"

void fifo_init(Fifo *fifo, int size) {
    fifo->data = malloc(sizeof(Data*)*size);

    fifo->max_size = size;
    fifo->size = 0;
    fifo->first = 0;
    fifo->last = -1;
}

void fifo_delete(Fifo *fifo) {
    free(fifo->data);
}


int fifo_add(Data *data, Fifo *fifo) {
    if(fifo_is_full(fifo))
        return -1;

    fifo->size++;
    fifo->last = (fifo->last + 1) % fifo->max_size;
    fifo->data[fifo->last] = data;

    return 1;
}

Data *fifo_remove(Fifo *fifo) {
    if(fifo_is_empty(fifo))
        return NULL;

    fifo->size--;
    Data *data = fifo->data[fifo->first];
    fifo->first = (fifo->first + 1) % fifo->max_size;
    return data;
}

Data *fifo_next(Fifo *fifo) {
    if(fifo_is_empty(fifo))
        return NULL;

    return fifo->data[fifo->first];
}

int fifo_is_empty(Fifo *fifo) {
    return fifo->size == 0 ? 1:0;
}

int fifo_is_full(Fifo *fifo) {
    return fifo->size == fifo->max_size ? 1:0;
}


