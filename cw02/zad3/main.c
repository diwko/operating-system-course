#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

struct flock prepare_lock(short type, int offset, int len) {
    struct flock lock;
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = len;
    return lock;
}

struct flock check_lock(int file_descriptor, int char_number){
    struct flock lock;
    lock = prepare_lock(F_WRLCK, char_number, 1);
    fcntl(file_descriptor, F_GETLK, &lock);
    return lock;
}

struct flock set_lock(int file_descriptor, int char_number, int type) {
    struct flock lock = prepare_lock(type, char_number, 1);
    fcntl(file_descriptor, F_GETLK, &lock);
    if(lock.l_type == F_UNLCK) {
        lock = prepare_lock(type, char_number, 1);
        fcntl(file_descriptor, F_SETLKW, &lock);
        printf("Założono blokadę\n");
        return lock;
    } else {
        lock = prepare_lock(type, char_number, 1);
        printf("Blokada już istnieje\n"
                       "Wybierz opcję:\n"
                       "1 - wersja nieblokująca\n"
                       "2 - wersja blokująca\n"
                       "_ - zakończ\n");
        int option;
        scanf("%d", &option);
        switch(option) {
            case 1:
                if(fcntl(file_descriptor, F_SETLK, &lock) == -1){
                    printf("Nie udało się założyć blokady\n");
                    lock.l_type = F_UNLCK;
                    return lock;
                }
                return lock;
            case 2:
                printf("Oczekiwanie na założenie blokady...\n");
                if(fcntl(file_descriptor, F_SETLKW, &lock) == -1){
                    printf("Nie udało się założyć blokady\n");
                    lock.l_type = F_UNLCK;
                    return lock;
                }
                printf("Założono blokadę\n");
                return lock;
            default:
                break;
        }
    }
}

void print_lock(struct flock lock){
    if(lock.l_type != F_UNLCK) {
        char *type;
        if(lock.l_type == F_RDLCK)
            type = "F_RDLCK";
        else
            type = "F_WRLCK";

        printf("znak: %d; PID: %d; type: %s\n", (int)lock.l_start, lock.l_pid, type);
    }
}

void print_locks(int file_descriptor, struct flock *locks, int locks_number) {
    struct flock lock;
    long file_size = lseek(file_descriptor, 0, SEEK_END);

    for(int i = 0; i < file_size; i++) {
        lock = prepare_lock(F_WRLCK, i, 1);
        fcntl(file_descriptor, F_GETLK, &lock);
        print_lock(lock);
    }
    printf("Moje blokady:\n");
    for(int lock = 0; lock < locks_number; lock++) {
        print_lock(locks[lock]);
    }
}

void remove_lock(int file_descriptor, int char_number) {
    struct flock lock = prepare_lock(F_UNLCK, char_number, 1);
    fcntl(file_descriptor, F_SETLK, &lock);
}

char read_char(int file_descriptor, int char_number) {
    if(check_lock(file_descriptor, char_number).l_type == F_WRLCK){
        printf("Jest założona blokada F_WRLCK, nie można odczytać");
        return ' ';
    }

    char buffer;
    lseek(file_descriptor, char_number, SEEK_SET);
    read(file_descriptor, &buffer, 1);
    return buffer;
}

void replace_char(int file_descriptor, int char_number, char new_char) {
    if(check_lock(file_descriptor, char_number).l_type != F_UNLCK){
        printf("Jest założona blokada, nie można zmieniać");
        return;
    }
    lseek(file_descriptor, char_number, SEEK_SET);
    write(file_descriptor, &new_char, 1);
}

int get_int() {
    int x;
    scanf("%d", &x);
    return x;
}

char get_char() {
    printf("Podaj znak:\n");
    char x;
    scanf(" %c", &x);
    return x;
}

int get_char_number(int file_size) {
    printf("Podaj numer znaku:\n");
    int n = get_int();
    if(n >= 0 && n < file_size)
        return n;
    return get_char_number(file_size);
}

int get_main_option() {
    printf(""
                   "\n============\n"
                   "0 - wyjście\n"
                   "1 - ustawienie rygla do odczytu na wybrany znak pliku\n"
                   "2 - ustawienie rygla do zapisu na wybrany znak pliku\n"
                   "3 - wyświetlenie listy zaryglowanych znaków pliku\n"
                   "4 - zwolnienie wybranego rygla\n"
                   "5 - odczyt wybranego znaku pliku\n"
                   "6 - zmiana wybranego znaku pliku\n"
                   "============\n\n");
    int option = get_int();
    if(option >= 0 && option <= 6)
        return option;
    get_main_option();
}

char *get_path() {
    printf("Podaj ścieżkę do pliku:\n");
    char file_name[200];
    scanf("%s", file_name);
    char *name = malloc(sizeof(char) * (strlen(file_name) + 1));
    strcpy(name, file_name);
    return name;
}

int get_lock_type() {
    printf("1 - F_RDLCK\n2 - F_WRLCK\n");
    int x = get_int();
    if(x == 1)
        return F_RDLCK;
    if(x == 2)
        return F_WRLCK;
    get_lock_type();
}

void my_lock_add(struct flock locks[], int *locks_number, struct flock new_lock) {
    for(int i = 0; i < *locks_number; i++){
        if((int)locks[i].l_start == new_lock.l_start){
            locks[i].l_type = new_lock.l_type;
            return;
        }
    }
    locks[*locks_number] = new_lock;
    *locks_number = *locks_number + 1;
}

void my_lock_remove(struct flock locks[], int locks_number, int new_lock) {
    for(int i = 0; i < locks_number; i++){
        if((int)locks[i].l_start == new_lock)
            locks[i].l_type = F_UNLCK;
    }
}

int get_file_size(int fd) {
    struct stat stbuf;
    if((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
        printf("Błąd pliku");
        exit(1);
    }
    return (int)stbuf.st_size;
}

void user_interface() {
    struct flock locks[100];
    int locks_number = 0;

    int main_option;
    char *path;
    path = get_path();
    int file = open(path, O_RDWR);
    if(file < 0){
        printf("Nie mogę otworzyć plku");
        exit(1);
    }

    int file_size = get_file_size(file);
    printf("%d", file_size);
    do {
        main_option = get_main_option();
        int tmp;
        switch(main_option){
            case 1:
                my_lock_add(locks, &locks_number, set_lock(file, get_char_number(file_size), F_RDLCK));
                break;
            case 2:
                my_lock_add(locks, &locks_number, set_lock(file, get_char_number(file_size), F_WRLCK));
                break;
            case 3:
                print_locks(file, locks, locks_number);
                break;
            case 4:
                tmp = get_char_number(file_size);
                remove_lock(file, tmp);
                my_lock_remove(locks, locks_number, tmp);
                break;
            case 5:
                printf("%c\n", read_char(file, get_char_number(file_size)));
                break;
            case 6:
                replace_char(file, get_char_number(file_size), get_char());
                break;
            default:
                continue;
        }
    } while(main_option != 0);
    free(path);
    close(file);
}

int main() {
    user_interface();

    return 0;
}