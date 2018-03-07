#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <time.h>

#include "client.h"


int queue_receive;

int main() {
    atexit(&close_client);

    srand(time(NULL));
    key_t key = ftok(KEY_PATH, rand()%127+1);
    queue_receive = get_queue(key, IPC_CREAT | 0666);

    key_t server_key = ftok(KEY_PATH, server_number);
    int queue_send = get_server_queue(server_key);

    while(1) {
        Message message = load_message();
        send_message(queue_send, &message);

        if(receive_message(&message, queue_receive, 0, 0) == 1)
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

int get_server_queue(key_t server_key) {
    int queue_send = get_queue(server_key, 0);

    char text[MAX_TEXT_SIZE];
    sprintf(text, "%d", queue_receive);
    Message message = create_message(CONNECT, text);
    send_message(queue_send, &message);

    if(receive_message(&message, queue_receive, STATUS, 0) == 1)
        parse_message(&message);

    return queue_send;
}