#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MIN_RE -2
#define MAX_RE  1
#define MIN_IM -1
#define MAX_IM  1

int **calloc_2d_array(int x, int y) {
    int **array = malloc(x*sizeof(int*));
    int i;
    for(i = 0; i < x; i++)
        array[i] = calloc(y, sizeof(int));
    return array;
}

void free_2d_array(int **array, int x) {
    int i;
    for(i = 0; i < x; i++)
        free(array[i]);
    free(array);
}

void read_fifo(int **T, int R, char *fifo_name) {
    FILE *fifo = fopen(fifo_name, "r");
    if(fifo == NULL){
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }
    long double re, im;
    int iters;

    while(feof(fifo) == 0) {
        fscanf(fifo, "%Lf %Lf %d", &re, &im, &iters);
        T[(int)((re - MIN_RE)/(MAX_RE - MIN_RE)*(R-1))][(int)((im - MIN_IM)/(MAX_IM - MIN_IM)*(R-1))] = iters;
    }
    fclose(fifo);
}

void prepare_data_for_gnuplot(char *file_name, int **array, int x, int y) {
    FILE *file = fopen(file_name, "w");
    if(file == NULL){
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }
    int x_a, y_a;
    for(x_a = 0; x_a < x; x_a++) {
        for(y_a = 0; y_a < y; y_a++) {
            fprintf(file, "%d %d %d\n", x_a, y_a, array[x_a][y_a]);
        }
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Nieprawidłowa liczba argumentów\n");
        exit(EXIT_FAILURE);
    }
    char *pipe_name = argv[1];
    int R = atoi(argv[2]);

    if(mkfifo(pipe_name, 0666) == -1){
        perror("Error mkfifo");
        exit(EXIT_FAILURE);
    }

    int **T = calloc_2d_array(R, R);

    read_fifo(T, R, pipe_name);

    char data_file[] = "data.txt";
    prepare_data_for_gnuplot(data_file, T, R, R);

    free_2d_array(T, R);

    FILE *gnuplot_pipe = popen("gnuplot", "w");
    fprintf(gnuplot_pipe, "set view map\n");
    fprintf(gnuplot_pipe, "set xrange [0:%d]\n", R);
    fprintf(gnuplot_pipe, "set yrange [0:%d]\n", R);
    fprintf(gnuplot_pipe, "plot '%s' with image\n", data_file);
    fflush(gnuplot_pipe);
    printf("Nasiśnij ENTER, aby zakończyć ");
    getc(stdin);
    pclose(gnuplot_pipe);

    unlink(pipe_name);

    return 0;
}