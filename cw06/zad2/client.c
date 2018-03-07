#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>

#include "client.h"

mqd_t queue_receive;
mqd_t queue_send;
char queue_receive_name[FILENAME_MAX];

int main() {
    atexit(&close_client);

    get_rand_name(queue_receive_name);
    queue_receive = get_queue(queue_receive_name, O_CREAT | O_RDONLY, QUEUE_PERMISSIONS);

    queue_send = get_server_queue(server_queue_name);

    while(1) {
        Message message = load_message();
        send_message(queue_send, &message);

        if(receive_message(&message, queue_receive) == 1)
            parse_message(&message);
    }
}

MessageType load_message_type() {
    MessageType types[] = {ECHO, UPPER_CASE, TIME, TERMINATE};
    printf("Podaj typ komunikatu: 0-ECHO, 1-UPPER_CASE, 2-TIME, 3-TERMINATE:\n");
    int option;
    scanf("%d", &option);
    if(option >= 0 && option <= 3)
        return types[option];
    else
        return load_message_type();
}

void load_message_text(char *buffer, size_t size) {
    printf("Podaj tekst:\n");
    getchar();
    if((getline(&buffer, &size, stdin)) == -1)
        perror("getline error");
}

Message load_message() {
    MessageType type = load_message_type();
    char text[MAX_TEXT_SIZE];

    if(type == ECHO || type == UPPER_CASE)
        load_message_text(text, MAX_TEXT_SIZE);

    return create_message(type, text);
}

void close_client() {
    close_queue(queue_receive);
    delete_queue(queue_receive_name);
}

void action_text(Message *message) {
    printf("%s\n", message->text);
}

void action_status(Message *message) {
    int client_id = atoi(message->text);
    if (client_id < 0) {
        fprintf(stderr, "Nie udało połączyć się z serwerem\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Połączono z serweremn\n");
}

void action_disconnect(Message *message) {
    close_queue(queue_send);
    printf("Rozłączono z serwerem\n");
    exit(EXIT_SUCCESS);
}

void parse_message(Message *message) {
    switch (message->type) {
        case TEXT:
            action_text(message);
            break;
        case STATUS:
            action_status(message);
            break;
        case DISCONNECT:
            action_disconnect(message);
            break;
        default:
            fprintf(stderr, "Błędny typ komunikatu\n");
            break;
    }
}

mqd_t get_server_queue(const char *name) {
    mqd_t queue_send = get_queue(name, O_WRONLY, 0000);

    Message message = create_message(CONNECT, queue_receive_name);
    send_message(queue_send, &message);

    if(receive_message(&message, queue_receive) == 1)
        parse_message(&message);

    return queue_send;
}

void get_rand_name(char *name_buffer) {
    strcpy(name_buffer, "/client_queue_");
    char number[10];
    sprintf(number, "%d", getpid());
    strcat(name_buffer, number);
}