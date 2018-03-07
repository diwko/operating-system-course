#ifndef CONTACT_H
#define CONTACT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    FIRST_NAME,
    LAST_NAME,
    BIRTH_DATE,
    EMAIL,
    PHONE,
    ADDRESS
} Field;

typedef struct{
    char *first_name;
    char *last_name;
    char *birth_date;
    char *email;
    char *phone;
    char *address;
} Contact;

Contact *contact_create(
        char *first_name,
        char *last_name,
        char *birth_date,
        char *email,
        char *phone,
        char *address
);

void contact_delete(Contact *contact);

char *contact_get_field(Contact *contact, Field field);

int contact_equals(Contact *contact1, Contact *contact2);

int contact_compare(Contact *contact1, Contact *contact2, Field field);

void contact_print(Contact *contact);



#endif //CONTACT_H
