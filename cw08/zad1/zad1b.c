#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

#define RECORD_SIZE 1024

pthread_t *threads;
int threads_number;

typedef struct {
    int file;
    int records_number;
    char *word;
} ThreadArgs;

typedef struct {
    int id;
    char text[RECORD_SIZE];
} Record;

long gettid() {
    return syscall(SYS_gettid);
}

void parse_args(int argc, char *argv[], int *threads_number, char **file_name, int *records_number, char **word) {
    if(argc != 5) {
        fprintf(stderr, "Niepoprawna liczba argumentów\n threads_number - file_name - records_number - word\n");
        exit(EXIT_FAILURE);
    }

    *threads_number = atoi(argv[1]);
    *file_name = argv[2];
    *records_number = atoi(argv[3]);
    *word = argv[4];
}

int open_file(const char *file_name, int flag) {
    int fd = open(file_name, flag);
    if(fd == -1) {
        perror("open error");
        exit(EXIT_FAILURE);
    }
    return fd;
}

Record *parse_records(char *string, int records_number) {
    Record *records = malloc(sizeof(Record)*records_number);

    char *buffer;
    char *rest;

    buffer = strtok_r(string, "\n", &rest);
    if(buffer == NULL)
        return records;
    sscanf(buffer, "%d%[^\n]", &records[0].id, records[0].text);

    int i;
    for(i = 1; i < records_number; i++) {
        buffer = strtok_r(NULL, "\n", &rest);
        if(buffer == NULL)
            return records;

        sscanf(buffer, "%d%[^\n]", &records[i].id, records[i].text);
    }

    return records;
}

int find_record(const Record *records, int records_number, const char *word) {
    int i;
    for(i = 0; i < records_number; i++) {
        if(strstr(records[i].text, word) != NULL) {
            return i;
        }
    }
    return -1;
}

ssize_t read_records(int fd, int records_number, char *buffer) {
    size_t size = (size_t)records_number*RECORD_SIZE;
    ssize_t bytes_readed = read(fd, buffer, size);
    if(bytes_readed == -1) {
        perror("read error");
        exit(EXIT_FAILURE);
    }
    return bytes_readed;
}

void cancel_treads() {
    pthread_t my_tid = pthread_self();
    int i;
    for(i = 0; i < threads_number; i++) {
        if(!pthread_equal(my_tid, threads[i])) {
            pthread_cancel(threads[i]);
        }
    }
}

void* thread_action(void *arguments) {
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    long my_tid = gettid();
    printf("TID: %ld - START\n", my_tid);

    ThreadArgs *args = arguments;

    int size = args->records_number*RECORD_SIZE;
    char *records_to_parse = malloc(sizeof(char)*size);

    while(read_records(args->file, args->records_number, records_to_parse) != 0) {
        Record* records = parse_records(records_to_parse, args->records_number);
        int x = find_record(records, args->records_number, args->word);
        if(x != -1) {
            printf("TID: %ld - FOUND - line: %d\n", my_tid, records[x].id);
            free(records);
            free(records_to_parse);
            cancel_treads();
            break;
        }
        free(records);
        printf("TID: %ld - BEFORE BREAKPOINT\n", my_tid);
        pthread_testcancel();
        printf("TID: %ld - AFTER BREAKPOINT\n", my_tid);
    }
    printf("TID: %ld - END\n", my_tid);
    free(records_to_parse);
    return NULL;
}

int main(int argc, char *argv[]) {
    char *file_name = NULL;
    int records_number;
    char *word = NULL;
    parse_args(argc, argv, &threads_number, &file_name, &records_number, &word);
    printf("%s\n", file_name);

    int file = open_file(file_name, O_RDONLY);

    threads = malloc(sizeof(pthread_t)*threads_number);

    int i;
    for(i = 0; i < threads_number; i++) {
        ThreadArgs args;
        args.file = file;
        args.records_number = records_number;
        args.word = word;

        if(pthread_create(&threads[i], NULL, &thread_action, &args) != 0) {
            perror("pthread_create error");
        }
    }
    for(i = 0; i < threads_number; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    close(file);

    return 0;
}
