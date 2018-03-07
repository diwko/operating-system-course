#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>
#include "commons.h"

#define _BSD_SOURCE

int sockfd;

int parse_args(int argc, char *argv[], char *name, struct sockaddr_un *local_addr, struct sockaddr_in *net_addr) {
    if(argc <  4 || argc > 5) {
        fprintf(stderr, "Incorrect args\n"
                "NAME local PATH\n"
                "NAME net IPv4 PORT\n");
        exit(EXIT_FAILURE);
    }

    if(strlen(argv[1]) >= NAME_MAX_SIZE) {
        fprintf(stderr, "Incorrect NAME (<%d)\n", NAME_MAX_SIZE);
        exit(EXIT_FAILURE);
    }
    strcpy(name, argv[1]);

    int domain;
    if(strcmp(argv[2], "local") == 0 && argc == 4) {
        domain = AF_LOCAL;
        local_addr->sun_family = domain;

        if(strlen(argv[1]) >= UNIX_PATH_MAX) {
            fprintf(stderr, "Incorrect PATH (<%d)\n", UNIX_PATH_MAX);
            exit(EXIT_FAILURE);
        }
        strcpy(local_addr->sun_path, argv[3]);

    } else if(strcmp(argv[2], "net") == 0 && argc == 5) {
        domain = AF_INET;
        net_addr->sin_family = domain;

        if(inet_aton(argv[3], &net_addr->sin_addr) == 0) {
             fprintf(stderr, "Incorrect IPv4\n");
             exit(EXIT_FAILURE);
         }

        net_addr->sin_port = htobe16(atoi(argv[4]));
    } else {
        fprintf(stderr, "Incorrect args\n"
                "NAME local PATH\n"
                "NAME net IPv4 PORT\n");
        exit(EXIT_FAILURE);
    }
    return domain;
}

void handshake(int sockfd, const char *name) {
    MsgConnect msg;
    strcpy(msg.name, name);

    send_safe(sockfd, &msg, sizeof(MsgConnect), 0);

    MsgConnectStatus status;
    recv_safe(sockfd, &status, sizeof(MsgConnectStatus), 0);
    if(status.status != STATUS_OK) {
        fprintf(stderr, "WRONG NAME (name already exist)\n");
        exit(EXIT_FAILURE);
    }

    printf("CONNECTED TO SERVER\n");
}

double operation_result(const MsgOperation *op) {
    switch (op->op) {
        case '+':
            return op->a + op->b;
        case '-':
            return op->a - op->b;
        case '*':
            return op->a * op->b;
        case '/':
            return op->a / op->b;
        default:
            return 0;
    }
}

void operation_action(int sockfd) {
    MsgOperation operation;
    recv_safe(sockfd, &operation, sizeof(operation), 0);

    MsgResult result = {MSG_RESULT, operation.id, operation_result(&operation)};
    send_safe(sockfd, &result, sizeof(result), 0);
    printf("id: %d - COMPLETED\n", operation.id);
}

void free_resources() {
    shutdown_safe(sockfd, SHUT_RDWR);
    close_safe(sockfd);
}

void sigint_handler(int sig) {
    MsgType msg = {MSG_DISCONNECT};
    send_safe(sockfd, &msg, sizeof(msg), 0);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    atexit(free_resources);
    signal(SIGINT, sigint_handler);

    char name[NAME_MAX_SIZE];
    struct sockaddr_un local_addr;
    struct sockaddr_in net_addr;
    int domain = parse_args(argc, argv, name, &local_addr, &net_addr);

    sockfd = socket_safe(domain, SOCK_STREAM, 0);

    if(domain == AF_LOCAL) {
        connect_safe(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));
    } else if(domain == AF_INET) {
        connect_safe(sockfd, (struct sockaddr*)&net_addr, sizeof(net_addr));
    } else {
        fprintf(stderr, "Incorrect domain\n");
        close_safe(sockfd);
        exit(EXIT_FAILURE);
    }

    handshake(sockfd, name);

    while(1) {
        MsgType type;
        if(recv_safe(sockfd, &type, sizeof(type), MSG_PEEK) == 0)
            exit(EXIT_FAILURE);
        if(type.type == MSG_OPERATION) {
            operation_action(sockfd);
        } else if(type.type == MSG_PING) {
            if(recv_safe(sockfd, &type, sizeof(type), 0) == 0)
                exit(EXIT_FAILURE);
            fprintf(stderr, "PING\n");
        } else {
            fprintf(stderr, "UNRECOGNIZED TYPE: %d\n", type.type);
        }
    }


    return 0;
}
