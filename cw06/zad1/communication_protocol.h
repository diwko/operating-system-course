#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#define MAX_TEXT_SIZE 1000
#define MAX_SERVER_CLIENTS 5
#define KEY_PATH getenv("HOME")

static const int server_number = 'z';

typedef enum {
    ECHO = 1,
    UPPER_CASE,
    TIME,
    TERMINATE,
    CONNECT,
    DISCONNECT,
    STATUS,
    TEXT
} MessageType;


typedef struct {
    long type;
    pid_t pid;
    char text[MAX_TEXT_SIZE];
} Message;


int get_queue(key_t key, int flags);

void close_queue(int queue_id);

Message create_message(MessageType type, char *text);

void send_message(int queue_id, Message *message);

int receive_message(Message *message, int queue_id, int message_type, int flags);



#endif //COMMUNICATION_PROTOCOL_H
