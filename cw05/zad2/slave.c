#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MIN_RE -2
#define MAX_RE  1
#define MIN_IM -1
#define MAX_IM  1
#define MAX_ABS 2

typedef struct {
    long double re;
    long double im;
} ComplexNumber;

long double abs_complex(ComplexNumber x) {
    return sqrtl(x.re*x.re + x.im*x.im);
}

ComplexNumber add_complex(ComplexNumber a, ComplexNumber b) {
    ComplexNumber result;
    result.re = a.re + b.re;
    result.im = a.im + b.im;
    return result;
}

ComplexNumber sqr_complex(ComplexNumber x) {
    if(x.re == 0 && x.im == 0)
        return x;

    long double abs = abs_complex(x);
    long double cos_fi = x.re/abs;
    long double sin_fi = x.im/abs;

    ComplexNumber result;
    result.re = abs*abs*(cos_fi*cos_fi - sin_fi*sin_fi);
    result.im = abs*abs*(2*sin_fi*cos_fi);

    return result;
}

ComplexNumber random_complex(long double min_re, long double max_re, long double min_im, long double max_im) {
    ComplexNumber complex;
    complex.re = (long double)rand()/RAND_MAX*(max_re - min_re) + min_re;
    complex.im = (long double)rand()/RAND_MAX*(max_im - min_im) + min_im;
    return complex;
}

int calculate_iters(ComplexNumber c, int K, long double max_abs) {
    int iters;
    ComplexNumber zn;
    zn.re = zn.im = 0;

    for(iters = 0; abs_complex(zn) <= max_abs && iters < K; iters++){
        zn = add_complex(sqr_complex(zn), c);
    }
    return iters;
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Nieprawidłowa liczba argumentów\n");
        exit(EXIT_FAILURE);
    }
    char *pipe_name = argv[1];
    int N = atoi(argv[2]);
    int K = atoi(argv[3]);

    int fd;
    if((fd = open(pipe_name, O_WRONLY)) < 0){
        perror("Error open");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)*getpid());

    char *buffer = malloc(sizeof(char)*100);
    int n;
    for(n = 0; n < N; n++) {
        ComplexNumber complex_rand = random_complex(MIN_RE, MAX_RE, MIN_IM, MAX_IM);
        int iters = calculate_iters(complex_rand, K, MAX_ABS);
        sprintf(buffer, "%.32Lf %.32Lf %d\n",  complex_rand.re, complex_rand.im, iters);

        if(write(fd, buffer, strlen(buffer)) < 0)
            perror("Writing error: ");
    }
    free(buffer);

    close(fd);

    return 0;
}
