#ifndef SERVER_H
#define SERVER_H

#include "communication_protocol.h"


typedef struct {
    pid_t pid;
    mqd_t queue_id;
} Client;


Client* get_client(pid_t pid);

void to_upper_case(char *text);

void get_time_text(char *buffer);

void action_echo(Message *message);

void action_upper_case(Message *message);

void action_time(Message *message);

void action_terminate(Message *message);

void action_connect(Message *message);

void parse_message(Message *message);

void close_server();

#endif //SERVER_H
