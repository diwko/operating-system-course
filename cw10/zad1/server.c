#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "commons.h"

#define MAX_CLIENTS 20

typedef struct {
    int sockfd;
    char name[NAME_MAX_SIZE];
} Client;

Client clients[MAX_CLIENTS];
int clients_connected = 0;

int operation_id = 0;
int sockfd_local;
int sockfd_net;
struct sockaddr_un local_addr;
struct sockaddr_in net_addr;
int epoll_fd;
pthread_mutex_t	clients_mutex = PTHREAD_MUTEX_INITIALIZER;


void parse_args(int argc, char *argv[], struct sockaddr_un *local_addr, struct sockaddr_in *net_addr) {
    if(argc != 3) {
        fprintf(stderr, "INCORRECT ARGUMENTS\n "
                "PORT PATH\n");
        exit(EXIT_FAILURE);
    }

    local_addr->sun_family = AF_UNIX;
    if(strlen(argv[2]) >= UNIX_PATH_MAX) {
        fprintf(stderr, "Incorrect PATH (<%d)\n", UNIX_PATH_MAX);
        exit(EXIT_FAILURE);
    }
    strcpy(local_addr->sun_path, argv[2]);

    net_addr->sin_family = AF_INET;
    net_addr->sin_port = htobe16(atoi(argv[1]));
    net_addr->sin_addr.s_addr = INADDR_ANY;
}

void free_resources() {
    pthread_mutex_lock_safe(&clients_mutex);

    int i;
    for(i = 0; i < MAX_CLIENTS; i++)
        if(clients[i].sockfd != -1)
            close_safe(clients[i].sockfd);

    shutdown_safe(sockfd_local, SHUT_RDWR);
    shutdown_safe(sockfd_net, SHUT_RDWR);
    close_safe(sockfd_local);
    close_safe(sockfd_net);
    unlink_safe(local_addr.sun_path);

    pthread_mutex_unlock_safe(&clients_mutex);
}

int epoll_create1_safe() {
    epoll_fd = epoll_create1(0);
    if(epoll_fd == -1) {
        perror("EPOLL_CREATE ERROR");
        exit(EXIT_FAILURE);
    }
    return epoll_fd;
}

void epoll_ctl_safe(int epfd, int op, int fd, struct epoll_event *event) {
    if(epoll_ctl(epfd, op, fd, event) == -1) {
        perror("EPOLL_CTL ERROR");
    }
}

void epoll_sockets(const int epoll_fd, const int sock_fd, int op) {
    struct epoll_event event;
    event.data.fd = sock_fd;
    event.events = EPOLLIN;
    epoll_ctl_safe(epoll_fd, op, sock_fd, &event);
}

int client_name_unique(const char *name) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].sockfd != -1) {
            if(strcmp(name, clients[i].name) == 0) {
                return 0;
            }
        }
    }
    return 1;
}

int find_client(int client_fd) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].sockfd == client_fd)
            return i;
    }
    return -1;
}

int random_client() {
    int x;
    while(1) {
        x = rand()%MAX_CLIENTS;
        if(clients[x].sockfd != -1)
            break;
    }
    return x;
}

int add_client(int client_fd, const char *name) {
    pthread_mutex_lock_safe(&clients_mutex);

    if(clients_connected >= MAX_CLIENTS || !client_name_unique(name)){
        pthread_mutex_unlock_safe(&clients_mutex);
        return 0;
    }

    int i;
    for(i = 0; i < MAX_CLIENTS && clients[i].sockfd != -1; i++);

    Client client;
    client.sockfd = client_fd;
    strcpy(client.name, name);
    clients[i] = client;
    clients_connected++;

    epoll_sockets(epoll_fd, client_fd, EPOLL_CTL_ADD);
    fprintf(stderr, "CONNECT - %s\n", name);

    pthread_mutex_unlock_safe(&clients_mutex);
    return 1;
}

void remove_client(int client_fd) {
    pthread_mutex_lock_safe(&clients_mutex);

    int x = find_client(client_fd);
    if(x != -1) {
        clients[x].sockfd = -1;
        clients_connected--;
        fprintf(stderr, "DISCONNECT - %s\n", clients[x].name);
    }
    close(client_fd);
    pthread_mutex_unlock_safe(&clients_mutex);
}

void handshake(const int sockfd) {
    int client_fd = accept_safe(sockfd, NULL, NULL);

    MsgConnect msg;
    if(recv_safe(client_fd, &msg, sizeof(MsgConnect), 0) == 0) {
        remove_client(client_fd);
        return;
    }

    MsgConnectStatus status;
    if(add_client(client_fd, msg.name)) {
        status.status = STATUS_OK;
    } else {
        status.status = STATUS_CONNECTION_REFUSED;
    }

    send_safe(client_fd, &status, sizeof(MsgConnectStatus), 0);
    if(status.status == STATUS_CONNECTION_REFUSED)
        close_safe(client_fd);
}

int load_operation(MsgOperation *op) {
    op->type = MSG_OPERATION;
    op->id = ++operation_id;
    if(scanf("%lf %c %lf", &op->a, &op->op, &op->b) == 3)
        return 1;
    fprintf(stderr, "INVALID OPERATION\n");
    char tmp[20];
    scanf("%s", tmp);
    return 0;
}

void *terminal_service(void *arg) {
    srand(time(NULL));

    while(1) {
        MsgOperation op;
        if(load_operation(&op)) {
            pthread_mutex_lock_safe(&clients_mutex);
            if(clients_connected > 0) {
                int client_fd = clients[random_client()].sockfd;
                send_safe(client_fd, &op, sizeof(op), 0);
            } else {
                fprintf(stderr, "NO CLIENTS\n");
            }
            pthread_mutex_unlock_safe(&clients_mutex);
        }
    }
    return NULL;
}

void operation_action(int client_fd) {
    MsgResult result;
    if(recv_safe(client_fd, &result, sizeof(MsgResult), 0) == 0) {
        remove_client(client_fd);
        return;
    }
    fprintf(stderr, "RESULT (%d) = %lf - %s\n", result.id, result.result, clients[find_client(client_fd)].name);
}

void msg_action(int type, int client_fd) {
    switch(type) {
        case MSG_RESULT:
            operation_action(client_fd);
            break;
        case MSG_DISCONNECT:
            remove_client(client_fd);
            break;
        default:
            fprintf(stderr, "UNRECOGNIZED MESSAGE TYPE\n");
            break;
    }
}

void *sockets_service(void *arg) {
    struct epoll_event events[MAX_CLIENTS];

    while(1) {
        int n = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);

        int i;
        for(i = 0; i < n; i++) {
            if(events[i].events == EPOLLIN) {
                if (events[i].data.fd == sockfd_local || events[i].data.fd == sockfd_net) {
                    handshake(events[i].data.fd);
                } else {
                    MsgType type;
                    if (recv_safe(events[i].data.fd, &type, sizeof(type), MSG_PEEK) == 0) {
                        remove_client(events[i].data.fd);
                        continue;
                    }
                    msg_action(type.type, events[i].data.fd);
                }
            }
        }
    }
    return NULL;
}

void *ping_service(void *arg) {
    MsgType ping = {MSG_PING};
    while(1) {
        int i;
        for(i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i].sockfd != -1) {
                if(send(clients[i].sockfd, &ping, sizeof(ping), MSG_NOSIGNAL) == -1) {
                    remove_client(clients[i].sockfd);
                }
            }
        }
        sleep(1);
    }
    return NULL;
}



void sigint_handler(int sig) {
    fprintf(stderr, "SERVER EXITING...\n");
    exit(EXIT_SUCCESS);
}

void init_clients() {
    Client client;
    client.sockfd = -1;
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = client;
    }
}

int main(int argc, char *argv[]) {
    atexit(free_resources);
    signal(SIGINT, sigint_handler);

    init_clients();

    parse_args(argc, argv, &local_addr, &net_addr);

    sockfd_local = socket_safe(AF_LOCAL, SOCK_STREAM, 0);
    sockfd_net = socket_safe(AF_INET, SOCK_STREAM, 0);

    bind_safe(sockfd_local, (struct sockaddr*)&local_addr, sizeof(local_addr));
    bind_safe(sockfd_net, (struct sockaddr*)&net_addr, sizeof(net_addr));

    printf("Local: %s\n", local_addr.sun_path);
    printf("IPv4: %s\n", inet_ntoa(net_addr.sin_addr));
    printf("Port: %d\n", ntohs(net_addr.sin_port));

    listen_safe(sockfd_local, MAX_CLIENTS);
    listen_safe(sockfd_net, MAX_CLIENTS);

    epoll_fd = epoll_create1_safe();

    epoll_sockets(epoll_fd, sockfd_local, EPOLL_CTL_ADD);
    epoll_sockets(epoll_fd, sockfd_net, EPOLL_CTL_ADD);

    pthread_t terminal_thread, sockets_thread, ping_thread;

    pthread_create_safe(&terminal_thread, NULL, terminal_service, NULL);
    pthread_create_safe(&sockets_thread, NULL, sockets_service, NULL);
    pthread_create_safe(&ping_thread, NULL, ping_service, NULL);

    pthread_join(terminal_thread, NULL);
    pthread_join(sockets_thread, NULL);
    pthread_join(ping_thread, NULL);

    free_resources();

    return 0;
}
