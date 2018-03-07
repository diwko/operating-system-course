#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>

#include "server.h"


Client clients[MAX_SERVER_CLIENTS];
int connected_clients;
mqd_t queue_receive;


int main() {
    atexit(&close_server);
    connected_clients = 0;

    queue_receive = get_queue(server_queue_name, O_CREAT | O_NONBLOCK | O_RDONLY, QUEUE_PERMISSIONS);

    while(1) {
        Message message;
        if(receive_message(&message, queue_receive) == 1)
            parse_message(&message);
    }
}

Client* get_client(pid_t pid) {
    int i;
    for(i = 0; i < connected_clients; i++) {
        if(clients[i].pid == pid)
            return &clients[i];
    }
    return NULL;
}

void to_upper_case(char *text) {
    if(text == NULL)
        return;

    int letter;
    for(letter = 0; text[letter] != '\0'; letter++)
        text[letter] = (char)toupper(text[letter]);
}

void get_time_text(char *buffer) {
    time_t current_time = time(NULL);

    if(current_time == ((time_t)(-1)))
        perror("time error");

    ctime_r(&current_time, buffer);
}

void action_echo(Message *message) {
    printf("PID: %d - ECHO\n", message->pid);

    Message msg = create_message(TEXT, message->text);

    Client *client = get_client(message->pid);
    send_message(client->queue_id, &msg);
}

void action_upper_case(Message *message) {
    printf("PID: %d - UPPER_CASE\n", message->pid);

    Message msg = create_message(TEXT, message->text);
    to_upper_case(msg.text);

    Client *client = get_client(message->pid);
    send_message(client->queue_id, &msg);
}

void action_time(Message *message) {
    printf("PID: %d - TIME\n", message->pid);

    Message msg = create_message(TEXT, "");
    get_time_text(msg.text);

    Client *client = get_client(message->pid);
    send_message(client->queue_id, &msg);
}

void action_terminate(Message *message) {
    printf("PID: %d - TERMINATE\n", message->pid);

    while(1) {
        Message msg;
        if(receive_message(&msg, queue_receive) == 1)
            parse_message(&msg);
        else
            break;
    }

    exit(EXIT_SUCCESS);
}

void action_connect(Message *message) {
    printf("PID: %d - CONNECT\n", message->pid);

    mqd_t client_queue = get_queue(message->text, O_WRONLY, 0000);

    if(connected_clients >= MAX_SERVER_CLIENTS) {
        Message msg = create_message(STATUS, "-1");
        send_message(client_queue, &msg);
        return;
    }

    Client client;
    client.pid = message->pid;
    client.queue_id = client_queue;
    clients[connected_clients++] = client;

    char text[MAX_TEXT_SIZE];
    sprintf(text, "%d", connected_clients - 1);
    Message msg = create_message(STATUS, text);
    send_message(client_queue, &msg);
}

void parse_message(Message *message) {
    switch (message->type) {
        case ECHO:
            action_echo(message);
            break;
        case UPPER_CASE:
            action_upper_case(message);
            break;
        case TIME:
            action_time(message);
            break;
        case TERMINATE:
            action_terminate(message);
            break;
        case CONNECT:
            action_connect(message);
            break;
        default:
            fprintf(stderr, "Błędny typ komunikatu\n");
            break;
    }
}

void close_server() {
    int client;
    for(client = 0; client < connected_clients; client++) {
        Message msg = create_message(DISCONNECT, "");
        send_message(clients[client].queue_id, &msg);
        close_queue(clients[client].queue_id);
    }

    close_queue(queue_receive);
    delete_queue(server_queue_name);
}