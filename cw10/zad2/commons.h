#ifndef CW10_COMMONS_H
#define CW10_COMMONS_H

#define UNIX_PATH_MAX 108
#define NAME_MAX_SIZE 128
#define STATUS_OK 200
#define STATUS_CONNECTION_REFUSED 111

#define MSG_PING 1
#define MSG_OPERATION 2
#define MSG_RESULT 3
#define MSG_DISCONNECT 4
#define MSG_CONNECT 5

typedef struct {
    int type;
} MsgType;

typedef struct {
    int type;
    char name[NAME_MAX_SIZE];
} MsgConnect;

typedef struct {
    int status;
} MsgConnectStatus;

typedef struct {
    int type;
    int id;
    char op;
    double a;
    double b;
} MsgOperation;

typedef struct {
    int type;
    int id;
    double result;
} MsgResult;

int socket_safe(int domain, int type, int protocol);
void close_safe(int sockfd);
void unlink_safe(const char *pathname);
void pthread_create_safe(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
void pthread_mutex_lock_safe(pthread_mutex_t *mutex);
void pthread_mutex_unlock_safe(pthread_mutex_t *mutex);
void connect_safe(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void shutdown_safe(int sockfd, int how);
void send_safe(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv_safe(int sockfd, void *buf, size_t len, int flags);
void bind_safe(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void listen_safe(int sockfd, int backlog);
int accept_safe(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void sendto_safe(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom_safe(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);



#endif //CW10_COMMONS_H
