#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "commons.h"

int socket_safe(int domain, int type, int protocol) {
    int sockfd = socket(domain, type, protocol);
    if(sockfd == -1) {
        perror("SOCKET ERROR");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void close_safe(int sockfd) {
    if(close(sockfd) == -1) {
        perror("CLOSE ERROR");
    }
}

void unlink_safe(const char *pathname) {
    if(unlink(pathname) == -1)
        perror("UNLINK ERROR");
}

void pthread_create_safe(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
    int res = pthread_create(thread, attr, start_routine, arg);
    if(res != 0) {
        fprintf(stderr, "PTHREAD_CREATE ERROR %d", res);
        exit(EXIT_FAILURE);
    }
}

void pthread_mutex_lock_safe(pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) != 0) {
        fprintf(stderr, "PTHREAD_MUTEX_LOCK ERROR");
    }
}

void pthread_mutex_unlock_safe(pthread_mutex_t *mutex) {
    if(pthread_mutex_unlock(mutex) != 0) {
        fprintf(stderr, "PTHREAD_MUTEX_UNLOCK ERROR");
    }
}

void connect_safe(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if(connect(sockfd, addr, addrlen) == -1) {
        perror("CONNECT ERROR");
        exit(EXIT_FAILURE);
    }
}

void shutdown_safe(int sockfd, int how) {
    if(shutdown(sockfd, how) == -1) {
        perror("SHUTDOWN ERROR");
    }
}

void send_safe(int sockfd, const void *buf, size_t len, int flags) {
    if(send(sockfd, buf, len, flags) == -1)
        perror("SEND ERROR");
}

ssize_t recv_safe(int sockfd, void *buf, size_t len, int flags) {
    ssize_t size = recv(sockfd, buf, len, flags);
    if(size == -1) {
        perror("RECV ERROR");
    }
    return size;
}

void bind_safe(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if(bind(sockfd, addr, addrlen) == -1) {
        perror("BIND ERROR");
        exit(EXIT_FAILURE);
    }
}

void listen_safe(int sockfd, int backlog) {
    if(listen(sockfd, backlog) == -1) {
        perror("LISTEN ERROR");
        exit(EXIT_FAILURE);
    }
}

int accept_safe(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int clientfd = accept(sockfd, addr, addrlen);
    if(clientfd == -1) {
        perror("ACCEPT ERROR");
    }
    return clientfd;
}
