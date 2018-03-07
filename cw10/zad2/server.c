#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "commons.h"

#define MAX_CLIENTS 20

typedef struct {
    int sockfd;
    char name[NAME_MAX_SIZE];
    sa_family_t family;
    void *addr;
    socklen_t addrlen;
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
    strcpy(local_addr->sun_path, argv[2]);

    net_addr->sin_family = AF_INET;
    net_addr->sin_port = htobe16(atoi(argv[1]));
    net_addr->sin_addr.s_addr = INADDR_ANY;
}

int address_eq(void *a1, void *a2) {
    if(((struct sockaddr*)a1)->sa_family != ((struct sockaddr*)a2)->sa_family)
        return 0;

    if(((struct sockaddr*)a1)->sa_family == AF_LOCAL) {
        struct sockaddr_un *b1 = a1;
        struct sockaddr_un *b2 = a2;
        if(strcmp(b1->sun_path, b2->sun_path) != 0)
            return 0;
    } else {
        struct sockaddr_in *b1 = a1;
        struct sockaddr_in *b2 = a2;
        if(b1->sin_port != b2->sin_port)
            return 0;
        if(b1->sin_addr.s_addr != b2->sin_addr.s_addr)
            return 0;
    }
    return 1;
}

void free_resources() {
    pthread_mutex_lock_safe(&clients_mutex);

    MsgType msg = {MSG_DISCONNECT};
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].sockfd != -1) {
            sendto_safe(clients[i].sockfd, &msg, sizeof(msg), 0, clients[i].addr, clients[i].addrlen);
            free(clients[i].addr);
        }
    }

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

int find_client(void *addr, socklen_t addrlen) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].sockfd != -1 && address_eq(clients[i].addr, addr))
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

int add_client(int sock_fd, const char *name, void *addr, socklen_t addrlen, sa_family_t family) {
    pthread_mutex_lock_safe(&clients_mutex);

    if(clients_connected >= MAX_CLIENTS || !client_name_unique(name)){
        pthread_mutex_unlock_safe(&clients_mutex);
        return 0;
    }

    int i;
    for(i = 0; i < MAX_CLIENTS && clients[i].sockfd != -1; i++);
    clients[i].sockfd = sock_fd;
    strcpy(clients[i].name, name);
    clients[i].family = family;
    clients[i].addr = addr;
    clients[i].addrlen = addrlen;
    clients_connected++;

    fprintf(stderr, "CONNECT - %s\n", name);

    pthread_mutex_unlock_safe(&clients_mutex);
    return 1;
}

void remove_client(void *addr, socklen_t addrlen) {
    pthread_mutex_lock_safe(&clients_mutex);
    int x = find_client(addr, addrlen);
    if(x != -1) {
        clients[x].sockfd = -1;
        free(clients[x].addr);
        clients_connected--;
        fprintf(stderr, "DISCONNECT - %s\n", clients[x].name);
    }

    pthread_mutex_unlock_safe(&clients_mutex);
}

void handshake(const int sockfd, sa_family_t family) {
    void *addr;
    socklen_t addrlen;
    MsgConnect msg;

    if(family == AF_LOCAL) {
        addr = malloc(sizeof(struct sockaddr_un));
        addrlen = sizeof(struct sockaddr_un);
        if(recvfrom_safe(sockfd, &msg, sizeof(msg), 0, addr, &addrlen) <= 0)
            return;
    } else if(family == AF_INET) {
        addr = malloc(sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
        if(recvfrom_safe(sockfd, &msg, sizeof(msg), 0, addr, &addrlen) <= 0)
            return;
    } else {
        fprintf(stderr, "UNRECOGNIZED PROTOCOL\n");
        return;
    }

    MsgConnectStatus status;
    if(add_client(sockfd, msg.name, addr, addrlen, family)) {
        status.status = STATUS_OK;
    } else {
        status.status = STATUS_CONNECTION_REFUSED;
    }

    sendto_safe(sockfd, &status, sizeof(MsgConnectStatus), 0, addr, addrlen);
    if(status.status == STATUS_CONNECTION_REFUSED)
        free(addr);
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
                Client *client = &clients[random_client()];
                sendto_safe(client->sockfd, &op, sizeof(op), 0, client->addr, client->addrlen);
            } else {
                fprintf(stderr, "NO CLIENTS\n");
            }
            pthread_mutex_unlock_safe(&clients_mutex);
        }
    }
    return NULL;
}

void operation_action(int sockfd) {
    MsgResult result;
    struct sockaddr addr;
    socklen_t addrlen;
    if(recvfrom_safe(sockfd, &result, sizeof(MsgResult), 0, &addr, &addrlen) == 0) {
        remove_client(&addr, addrlen);
        return;
    }
    fprintf(stderr, "RESULT (%d) = %lf - %s\n", result.id, result.result, clients[find_client(&addr, addrlen)].name);
}

void disconnect_action(int sockfd, sa_family_t family) {
    void *addr;
    socklen_t addrlen;
    MsgConnect msg;

    if(family == AF_LOCAL) {
        addr = malloc(sizeof(struct sockaddr_un));
        addrlen = sizeof(struct sockaddr_un);
        if(recvfrom_safe(sockfd, &msg, sizeof(msg), 0, addr, &addrlen) <= 0)
            return;
    } else if(family == AF_INET) {
        addr = malloc(sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
        if(recvfrom_safe(sockfd, &msg, sizeof(msg), 0, addr, &addrlen) <= 0)
            return;
    } else {
        fprintf(stderr, "UNRECOGNIZED PROTOCOL\n");
        return;
    }

    remove_client(addr, addrlen);
    free(addr);
}

void msg_action(int type, int sockfd, sa_family_t family) {
    switch(type) {
        case MSG_RESULT:
            operation_action(sockfd);
            break;
        case MSG_DISCONNECT:
            disconnect_action(sockfd, family);
            break;
        case MSG_CONNECT:
            handshake(sockfd, family);
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
            if (events[i].events == EPOLLIN) {
                MsgType msg;
                struct sockaddr addr;
                socklen_t addrlen;

                if (recvfrom_safe(events[i].data.fd, &msg, sizeof(msg), MSG_PEEK, &addr, &addrlen) > 0) {
                    msg_action(msg.type, events[i].data.fd, addr.sa_family);
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
                if(sendto(clients[i].sockfd, &ping, sizeof(ping), MSG_NOSIGNAL, clients[i].addr, clients[i].addrlen) == -1) {
                    remove_client(clients[i].addr, clients[i].addrlen);
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

    sockfd_local = socket_safe(AF_LOCAL, SOCK_DGRAM, 0);
    sockfd_net = socket_safe(AF_INET, SOCK_DGRAM, 0);

    bind_safe(sockfd_local, (struct sockaddr*)&local_addr, sizeof(local_addr));
    bind_safe(sockfd_net, (struct sockaddr*)&net_addr, sizeof(net_addr));

    printf("Local: %s\n", local_addr.sun_path);
    printf("IPv4: %s\n", inet_ntoa(net_addr.sin_addr));
    printf("Port: %d\n", ntohs(net_addr.sin_port));

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
