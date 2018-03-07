#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

char *path_cat(char *s1, char *s2) {
    char *str = malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 2));
    strcpy(str, s1);
    if(str[strlen(s1)-1] != '/')
        strcat(str, "/");
    strcat(str, s2);
    return str;
}

void print_file_stats(char *path, struct stat stats) {
    printf( (stats.st_mode & S_IRUSR) ? "r" : "-");
    printf( (stats.st_mode & S_IWUSR) ? "w" : "-");
    printf( (stats.st_mode & S_IXUSR) ? "x" : "-");
    printf( (stats.st_mode & S_IRGRP) ? "r" : "-");
    printf( (stats.st_mode & S_IWGRP) ? "w" : "-");
    printf( (stats.st_mode & S_IXGRP) ? "x" : "-");
    printf( (stats.st_mode & S_IROTH) ? "r" : "-");
    printf( (stats.st_mode & S_IWOTH) ? "w" : "-");
    printf( (stats.st_mode & S_IXOTH) ? "x " : "- ");
    printf("%*ld ", 13, stats.st_size);
    char date[20];
    strftime(date, 20, "%Y.%m.%d %H:%M:%S", localtime(&stats.st_mtim.tv_sec));
    printf("%s ", date);
    printf("%s\n", path);
}

void find_files(char *path, int max_size) {
    DIR *directory = opendir(path);
    if(!directory){
        return;
    }

    struct dirent *dir;

    while(dir = readdir(directory)) {
        if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            continue;

        struct stat file_stat;
        char *full_path = path_cat(path, dir->d_name);
        stat(full_path, &file_stat);
        if(S_ISREG(file_stat.st_mode) && file_stat.st_size < max_size){
            print_file_stats(full_path, file_stat);

        } else if(S_ISDIR(file_stat.st_mode)) {
            find_files(full_path, max_size);
        }
        free(full_path);
    }
    closedir(directory);
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        printf("Niepoprawana liczba argumentów\n");
        return 1;
    }
    char *path = argv[1];
    int max_size = atoi(argv[2]);

    char real_path[1000];
    realpath(path, real_path);
    find_files(path, max_size);

    return 0;
}