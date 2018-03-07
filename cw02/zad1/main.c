#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/stat.h>

void generate(char *filename, int record_size, int records_number) {
    int new_file = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    int random_file = open("/dev/urandom", O_RDONLY);
    char *buffer = malloc(sizeof(char) * record_size);

    for(int record = 0; record < records_number; record++){
        if(read(random_file, buffer, record_size) != record_size){
            printf("Problem with reading /dev/urandom");
            return;
        }
        if(write(new_file, buffer, record_size) != record_size){
            printf("Problem with generating file");
            return;
        }
    }

    close(random_file);
    close(new_file);
    free(buffer);
    return;
}

void sort_sys(char *filename, int record_size, int records_number) {
    FILE *file = fopen(filename, "r+");
    char *buffer1 = malloc(sizeof(char) * record_size);
    char *buffer2 = malloc(sizeof(char) * record_size);

    int sorted = 0;
    while(!sorted && records_number > 1){
        sorted = 1;
        for(int record = 0; record < records_number - 1; record++) {
            fread(buffer1, record_size, 1, file);
            fread(buffer2, record_size, 1, file);
            if(buffer1[0] > buffer2[0]) {
                fseek(file, -2 * record_size, SEEK_CUR);
                fwrite(buffer2, record_size, 1, file);
                fwrite(buffer1, record_size, 1, file);
                sorted = 0;
            }
            fseek(file, -1 * record_size, SEEK_CUR);
        }
        rewind(file);
        records_number--;
    }
    free(buffer1);
    free(buffer2);
    fclose(file);
}

void sort_lib(char *filename, int record_size, int records_number) {
    int file = open(filename, O_RDWR);
    char *buffer1 = malloc(sizeof(char) * record_size);
    char *buffer2 = malloc(sizeof(char) * record_size);

    int sorted = 0;
    while(!sorted && records_number > 1){
        sorted = 1;
        for(int record = 0; record < records_number - 1; record++) {
            read(file, buffer1, record_size);
            read(file, buffer2, record_size);
            if(buffer1[0] > buffer2[0]) {
                lseek(file, -2 * record_size, SEEK_CUR);
                write(file, buffer2, record_size);
                write(file, buffer1, record_size);
                sorted = 0;
            }
            lseek(file, -1 * record_size, SEEK_CUR);
        }
        lseek(file, 0, SEEK_SET);
        records_number--;
    }
    free(buffer1);
    free(buffer2);
    close(file);
}

void shuffle_sys(char *filename, int record_size, int records_number) {
    FILE *file = fopen(filename, "r+");
    char *buffer1 = malloc(sizeof(char) * record_size);
    char *buffer2 = malloc(sizeof(char) * record_size);

    srand(time(NULL));
    for(int i = 0; i < 1000; i++){
	    for(int record = 0; record < records_number - 1; record++) {
		fread(buffer1, record_size, 1, file);

		int random_record = (rand() % (records_number - record)) + record;

		fseek(file, random_record * record_size, SEEK_SET);
		fread(buffer2, record_size, 1, file);

		fseek(file, -1 * record_size, SEEK_CUR);
		fwrite(buffer1, record_size, 1, file);

		fseek(file, record * record_size, SEEK_SET);
		fwrite(buffer2, record_size, 1, file);
	    }
    }
    free(buffer1);
    free(buffer2);
    fclose(file);
}

void shuffle_lib(char *filename, int record_size, int records_number) {
    int file = open(filename, O_RDWR);
    char *buffer1 = malloc(sizeof(char) * record_size);
    char *buffer2 = malloc(sizeof(char) * record_size);

    srand(time(NULL));
    for(int i = 0; i < 1000; i++){
	    for(int record = 0; record < records_number - 1; record++) {
		read(file, buffer1, record_size);

		int random_record = (rand() % (records_number - record)) + record;

		lseek(file, random_record * record_size, SEEK_SET);
		read(file, buffer2, record_size);

		lseek(file, -1 * record_size, SEEK_CUR);
		write(file, buffer1, record_size);

		lseek(file, record * record_size, SEEK_SET);
		write(file, buffer2, record_size);
	    }
    }
    free(buffer1);
    free(buffer2);
    close(file);
}

struct Arguments {
    char *file_name;
    int records_number;
    int record_size;
};

struct Arguments parse_arguments_args(int argc, char *argv[]) {
    struct Arguments args;
    if(argc == 6) {
        args.file_name = argv[3];
        args.records_number = atoi(argv[4]);
        args.record_size = atoi(argv[5]);
    } else if(argc == 5) {
        args.file_name = argv[2];
        args.records_number = atoi(argv[3]);
        args.record_size = atoi(argv[4]);
    } else {
        args.file_name = NULL;
        args.records_number = 0;
        args.record_size = 0;
        printf("Niepoprawne argumenty - argumenty\n");
    }
    return args;
}

typedef void (*fptr)(char*, int, int);

fptr parse_arguments_function(int argc, char *argv[]) {
    if(argc == 6) {
        if(strcmp(argv[1], "sys") == 0) {
            if(strcmp(argv[2], "shuffle") == 0) {
                return shuffle_sys;
            } else if(strcmp(argv[2], "sort") == 0) {
                return sort_sys;
            }
        }
        else if(strcmp(argv[1], "lib") == 0) {
            if(strcmp(argv[2], "shuffle") == 0) {
                return shuffle_lib;
            } else if(strcmp(argv[2], "sort") == 0) {
                return sort_lib;
            }
        }
    } else if(argc == 5) {
        if(strcmp(argv[1], "generate") == 0) {
            return generate;
        }
    }
    printf("Niepoprawne argumenty - funkcja\nnp. ./program sys shuffle datafile 100 512\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    struct Arguments args = parse_arguments_args(argc, argv);
    fptr fun = parse_arguments_function(argc, argv);
    if(args.file_name == NULL || fun == NULL)
        return 1;

    struct tms start;
    struct tms stop;

    times(&start);
    fun(args.file_name, args.record_size, args.records_number);
    times(&stop);

    double stime = (stop.tms_stime - start.tms_stime) / (double)sysconf(_SC_CLK_TCK);
    double utime = (stop.tms_utime - start.tms_utime) /(double)sysconf(_SC_CLK_TCK);

    printf("%*s%*s\n", 15, "user_t [s]", 15, "system_t [s]");
    printf("%*f%*f\n", 15, utime, 15, stime);

    return 0;
}