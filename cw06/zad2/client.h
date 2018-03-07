#ifndef CLIENT_H
#define CLIENT_H

#include "communication_protocol.h"

MessageType load_message_type();

void load_message_text(char *buffer, size_t size);

Message load_message();

void close_client();

void action_text(Message *message);

void action_status(Message *message);

void action_disconnect(Message *message);

void parse_message(Message *message);

int get_server_queue(const char *name);

void get_rand_name(char *name_buffer);


#endif //CLIENT_H
