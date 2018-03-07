#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong arguments\n");
        exit(EXIT_FAILURE);
    }

    char *var = getenv(argv[1]);
    if(var == NULL)
        fprintf(stderr, "%s: NOT EXIST\n", argv[1]);
    else
        printf("%s = %s\n", argv[1], var);

    return 0;
}

