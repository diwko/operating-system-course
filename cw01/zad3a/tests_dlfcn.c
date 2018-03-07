#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "contact.h"
#include "contactList.h"
#include "contactTree.h"

typedef struct {
    struct rusage rusage_start;
    struct rusage rusage_end;
    struct timeval real_start;
    struct timeval real_end;
} TimeCounter;

void start_time(TimeCounter *counter) {
    getrusage(RUSAGE_SELF, &counter->rusage_start);
    gettimeofday(&counter->real_start, NULL);
}

void stop_time(TimeCounter *counter) {
    getrusage(RUSAGE_SELF, &counter->rusage_end);
    gettimeofday(&counter->real_end, NULL);
}

void time_counter_print(TimeCounter *counter) {
    printf("------------------------------------------------------------\n");
    printf("%*s%*s%*s\n", 20, "Real[us]", 20, "User[us]", 20, "System[us]");
    printf("%*lu%*lu%*lu\n\n",
           20,counter->real_end.tv_usec - counter->real_start.tv_usec,
           20,counter->rusage_end.ru_utime.tv_usec - counter->rusage_start.ru_utime.tv_usec,
           20,counter->rusage_end.ru_stime.tv_usec - counter->rusage_start.ru_stime.tv_usec);
}

 Contact *create_contact(char *line,  Contact* (*contact_create)(char *first_name, char *last_name, char *birth_date, char *email, char *phone, char *address)) {
    char *fields[6];
    int field = 0;

    while( line != NULL && field < 6){
        fields[field] = strtok(line, "|");
        line = strtok (NULL, "");
        field++;
    }
    return contact_create(fields[0], fields[1], fields[2], fields[3], fields[4], fields[5]);
}

void contact_tree_add_contacts( ContactTree *tree, FILE *data, int max_line_size, void (*contact_tree_add_contact)( ContactTree *tree,  Contact *contact),  Contact* (*contact_create)(char *first_name, char *last_name, char *birth_date, char *email, char *phone, char *address)) {
    char line[max_line_size];
    if(tree == NULL)
        return;

    //TimeCounter c;

    while(fgets(line, max_line_size, data) != NULL) {
        //start_time(&c);
        contact_tree_add_contact(tree, create_contact(line, contact_create));
        /*stop_time(&c);
        printf("Add tree\n");
        time_counter_print(&c);*/
    }
}

void contact_list_add_contacts( ContactList *list, FILE *data, int max_line_size, void (*contact_list_add)( ContactList *contact_list,  Contact *contact),  Contact* (*contact_create)(char *first_name, char *last_name, char *birth_date, char *email, char *phone, char *address)) {
    char line[max_line_size];
    if(list == NULL)
        return;

    //TimeCounter c;

    while(fgets(line, max_line_size, data) != NULL) {
        //start_time(&c);
        contact_list_add(list, create_contact(line, contact_create));
        /*stop_time(&c);
        printf("Add list\n");
        time_counter_print(&c);*/
   }
}

int main() {
    void *lib_list = dlopen("libcontactList_shared.so", RTLD_NOW);
    void *lib_tree = dlopen("libcontactTree_shared.so", RTLD_NOW);

    if (!lib_tree | !lib_list) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }

    Contact* (*contact_create)(char *first_name, char *last_name, char *birth_date, char *email, char *phone, char *address);
    contact_create = dlsym(lib_list, "contact_create");

    void (*contact_delete)(Contact *contact);
    contact_delete = dlsym(lib_list, "contact_delete");

    ContactList * (*contact_list_create)();
    contact_list_create = dlsym(lib_list, "contact_list_create");

    void (*contact_list_delete)( ContactList *contact_list);
    contact_list_delete = dlsym(lib_list, "contact_list_delete");

    void (*contact_list_add)( ContactList *contact_list,  Contact *contact);
    contact_list_add = dlsym(lib_list, "contact_list_add");

    void (*contact_list_delete_contact)( ContactList *contact_list,  Contact *contact);
    contact_list_delete_contact = dlsym(lib_list, "contact_list_delete_contact");

    ContactList* (*contact_list_find)( ContactList *contact_list, Field field, char *data);
    contact_list_find = dlsym(lib_list, "contact_list_find");

    void (*contact_list_sort)( ContactList *contact_list, Field field);
    contact_list_sort = dlsym(lib_list, "contact_list_sort");

    ContactTree* (*contact_tree_create)(Field sorted_by);
    contact_tree_create = dlsym(lib_tree, "contact_tree_create");

    void (*contact_tree_delete)( ContactTree *tree);
    contact_tree_delete = dlsym(lib_tree, "contact_tree_delete");

    void (*contact_tree_add_contact)( ContactTree *tree,  Contact *contact);
    contact_tree_add_contact = dlsym(lib_tree, "contact_tree_add_contact");

    void (*contact_tree_delete_contact)( ContactTree *tree,  Contact *contact);
    contact_tree_delete_contact = dlsym(lib_tree, "contact_tree_delete_contact");

     ContactTree *(*contact_tree_find)( ContactTree *tree, Field field, char *data);
    contact_tree_find = dlsym(lib_tree, "contact_tree_find");

    void (*contact_tree_sort)( ContactTree *tree, Field field);
    contact_tree_sort = dlsym(lib_tree, "contact_tree_sort");


    char data_file_name[] = "data.txt";
    const int MAX_LINE_SIZE = 150;
    FILE *data_file = fopen(data_file_name, "rt");

    if (data_file == NULL)
    {
        perror("Nie udalo sie otworzyc pliku data.txt");
        return 1;
    }

    TimeCounter c;

    start_time(&c);
    ContactTree *contact_tree = contact_tree_create(LAST_NAME);
    for(int i = 0; i < 5; i++) {
        contact_tree_add_contacts(contact_tree, data_file, MAX_LINE_SIZE, contact_tree_add_contact, contact_create);
	rewind(data_file);
    }
    stop_time(&c);
    printf("Create tree\n");
    time_counter_print(&c);

    rewind(data_file);

    start_time(&c);
    ContactList *contact_list = contact_list_create();
    for(int i = 0; i < 5; i++) {
        contact_list_add_contacts(contact_list, data_file, MAX_LINE_SIZE, contact_list_add, contact_create);
        rewind(data_file);
    }
    stop_time(&c);
    printf("Create list\n");
    time_counter_print(&c);

    start_time(&c);
    contact_tree_find(contact_tree, FIRST_NAME, "Joan");
    stop_time(&c);
    printf("Find contacts tree\n");
    time_counter_print(&c);

    start_time(&c);
    contact_list_find(contact_list, FIRST_NAME, "Joan");
    stop_time(&c);
    printf("Find contacts list\n");
    time_counter_print(&c);

    Contact *contact = contact_create("Joan","Perez","1994/06/26","jperezrr@yahoo.co.jp","377-03-0144","Veracruz");

    start_time(&c);
    contact_tree_delete_contact(contact_tree, contact);
    stop_time(&c);
    printf("Delete contact tree\n");
    time_counter_print(&c);

    start_time(&c);
    contact_list_delete_contact(contact_list, contact);
    stop_time(&c);
    printf("Delete contact list\n");
    time_counter_print(&c);

    contact_delete(contact);
    contact = NULL;

    start_time(&c);
    contact_tree_sort(contact_tree, FIRST_NAME);
    stop_time(&c);
    printf("Sort tree\n");
    time_counter_print(&c);

    start_time(&c);
    contact_list_sort(contact_list, FIRST_NAME);
    stop_time(&c);
    printf("Sort list\n");
    time_counter_print(&c);

    contact_tree_delete(contact_tree);
    contact_list_delete(contact_list);

    fclose(data_file);
    dlclose(lib_list);
    dlclose(lib_tree);
    return 0;
}

