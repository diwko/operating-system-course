#include <stdlib.h>
#include "fifo.h"

int fifo_init(Fifo *fifo, int size) {
    if(size > FIFO_MAX_SIZE)
        return 0;

    fifo->max_size = size;
    fifo->size = 0;
    fifo->first = 0;
    fifo->last = -1;
    return 1;
}

int fifo_add(pid_t pid, Fifo *fifo) {
    if(fifo_is_full(fifo))
        return -1;

    Client client;
    client.pid = pid;
    if(fifo->last == -1) {
        client.number = 0;
    } else {
        client.number = (fifo->clients[fifo->last].number + 1) % (fifo->max_size + 1);
    }
    fifo->size++;
    fifo->last = (fifo->last + 1) % fifo->max_size;
    fifo->clients[fifo->last] = client;

    return client.number;
}

Client fifo_remove(Fifo *fifo) {
    if(fifo_is_empty(fifo))
        return (Client){-1, -1};

    fifo->size--;
    Client client = fifo->clients[fifo->first];
    fifo->first = (fifo->first + 1) % fifo->max_size;
    return client;
}

int fifo_is_empty(Fifo *fifo) {
    return fifo->size == 0 ? 1:0;
}

int fifo_is_full(Fifo *fifo) {
    return fifo->size == fifo->max_size ? 1:0;
}


