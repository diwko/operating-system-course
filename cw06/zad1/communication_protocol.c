#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "communication_protocol.h"


int get_queue(key_t key, int flags) {
    int queue_id;
    if((queue_id = msgget(key, flags)) == -1) {
        perror("msgget error");
        exit(EXIT_FAILURE);
    }
    return queue_id;
}

void close_queue(int queue_id) {
    if(msgctl(queue_id, IPC_RMID, NULL) == -1) {
        perror("msgctl - ipc_rmid error");
        exit(EXIT_FAILURE);
    }
}

Message create_message(MessageType type, char *text) {
    Message message;
    message.type = type;
    message.pid = getpid();

    if(strlen(text) + 1 > MAX_TEXT_SIZE) {
        fprintf(stderr, "message text error: text to long");
        message.text[MAX_TEXT_SIZE - 1] = '\0';
    }
    strcpy(message.text, text);

    return message;
}

void send_message(int queue_id, Message *message) {
    if(msgsnd(queue_id, message, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("client msgsnd error");
    }
}

int receive_message(Message *message, int queue_id, int message_type, int flags) {
    if(msgrcv(queue_id, message, sizeof(Message) - sizeof(long), message_type, flags) < 0)
        return -1;
    return 1;
}