#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>

#include "communication_protocol.h"


mqd_t get_queue(const char *name, int flags, mode_t mode) {
    mqd_t queue_id;

    if(mode == 0000) {
        if((queue_id = mq_open(name, flags)) == -1) {
            perror("mqopen error");
            exit(EXIT_FAILURE);
        }
    } else {
        struct mq_attr attr;
        attr.mq_msgsize = MAX_MESSAGE_SIZE;
        attr.mq_maxmsg = MAX_MESSAGES;

        if((queue_id = mq_open(name, flags, mode, &attr)) == -1) {
            perror("mqopen error");
            exit(EXIT_FAILURE);
        }
    }

    return queue_id;
}

void close_queue(mqd_t queue_id) {
    if(mq_close(queue_id) == -1) {
        perror("mq_close error");
        exit(EXIT_FAILURE);
    }
}

void delete_queue(const char *name) {
    if(mq_unlink(name) == -1) {
        perror("mq_unlink error");
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

void send_message(mqd_t queue_id, Message *message) {
    if(mq_send(queue_id, (char *)message, sizeof(Message), 0) == -1) {
        perror("mq_send error");
    }
}

int receive_message(Message *message, mqd_t queue_id) {
    if(mq_receive(queue_id, (char *)message, sizeof(Message), NULL) < 0) {
        //perror("mq_receive");
        return -1;
    }

    return 1;
}

void get_server_name(char *name_buffer) {
    getcwd(name_buffer, FILENAME_MAX);
    strcat(name_buffer, "/server_queue");
}