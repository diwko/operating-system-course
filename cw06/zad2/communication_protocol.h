#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#define MAX_TEXT_SIZE 100
#define MAX_SERVER_CLIENTS 5
#define MAX_MESSAGES 10
#define MAX_MESSAGE_SIZE sizeof(Message)
#define QUEUE_PERMISSIONS 0666

static const char server_queue_name[] = "/server_queue";

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


mqd_t get_queue(const char *name, int flags, mode_t mode);

void close_queue(mqd_t queue_id);

void delete_queue(const char *name);

Message create_message(MessageType type, char *text);

void send_message(mqd_t queue_id, Message *message);

int receive_message(Message *message, mqd_t queue_id);

void get_server_name(char *name_buffer);



#endif //COMMUNICATION_PROTOCOL_H
