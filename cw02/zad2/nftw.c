#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <time.h>

int max_size;

void print_file_stats(const char *path, const struct stat *stats) {
    printf( (stats->st_mode & S_IRUSR) ? "r" : "-");
    printf( (stats->st_mode & S_IWUSR) ? "w" : "-");
    printf( (stats->st_mode & S_IXUSR) ? "x" : "-");
    printf( (stats->st_mode & S_IRGRP) ? "r" : "-");
    printf( (stats->st_mode & S_IWGRP) ? "w" : "-");
    printf( (stats->st_mode & S_IXGRP) ? "x" : "-");
    printf( (stats->st_mode & S_IROTH) ? "r" : "-");
    printf( (stats->st_mode & S_IWOTH) ? "w" : "-");
    printf( (stats->st_mode & S_IXOTH) ? "x " : "- ");
    printf("%*ld ", 13, stats->st_size);
    char date[20];
    strftime(date, 20, "%Y.%m.%d %H:%M:%S", localtime(&stats->st_ctime));
    printf("%s ", date);
    printf("%s\n", path);
}

int find_files(const char *fpath, const struct stat *sb,
                int tflag, struct FTW *ftwbuf) {

    if(tflag == FTW_F && sb->st_size < max_size)
        print_file_stats(fpath, sb);
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        printf("Niepoprawana liczba argumentów\n");
        return 1;
    }
    char *path = argv[1];
    max_size = atoi(argv[2]);

    char real_path[1000];
    realpath(path, real_path);

    nftw(real_path, find_files, 20, FTW_DEPTH);

    return 0;
}