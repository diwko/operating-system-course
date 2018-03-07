#include "contact.h"

char *_contact_copy_text(char *text) {
    char *allocated = malloc(sizeof(char) * (strlen(text) + 1));
    strcpy(allocated, text);
    return allocated;
}

Contact *contact_create(char *first_name, char *last_name, char *birth_date, char *email, char *phone, char *address) {
    Contact *contact = malloc(sizeof(Contact));
    if(contact != NULL){
        contact->first_name = _contact_copy_text(first_name);
        contact->last_name = _contact_copy_text(last_name);
        contact->birth_date = _contact_copy_text(birth_date);
        contact->email = _contact_copy_text(email);
        contact->phone = _contact_copy_text(phone);
        contact->address = _contact_copy_text(address);
    }
    return contact;
}

void contact_delete(Contact *contact){
    if(contact != NULL){
        free(contact->first_name);
        free(contact->last_name);
        free(contact->birth_date);
        free(contact->email);
        free(contact->phone);
        free(contact->address);
        free(contact);
    }
}

char *contact_get_field(Contact *contact, Field field) {
    if(contact == NULL)
        return NULL;

    switch(field){
        case FIRST_NAME:
            return contact->first_name;
        case LAST_NAME:
            return contact->last_name;
        case BIRTH_DATE:
            return contact->birth_date;
        case EMAIL:
            return contact->email;
        case PHONE:
            return contact->phone;
        case ADDRESS:
            return contact->address;
        default:
            return NULL;
    }
}

int contact_equals(Contact *contact1, Contact *contact2) {
    if(contact1 == NULL) {
        if (contact2 == NULL)
            return 0;
        else
            return -1;
    }
    if(contact2 == NULL)
        return 1;

    if(strcmp(contact1->address, contact2->address))
        return 0;
    if(strcmp(contact1->birth_date, contact2->birth_date))
        return 0;
    if(strcmp(contact1->email, contact2->email))
        return 0;
    if(strcmp(contact1->first_name, contact2->first_name))
        return 0;
    if(strcmp(contact1->last_name, contact2->last_name))
        return 0;
    if(strcmp(contact1->phone, contact2->phone))
        return 0;
    return 1;
}

int contact_compare(Contact *contact1, Contact *contact2, Field field) {
    if(contact1 == NULL) {
        if (contact2 == NULL)
            return 0;
        else
            return -1;
    }
    if(contact2 == NULL)
        return 1;

    return strcmp(contact_get_field(contact1, field), contact_get_field(contact2, field));
}

void contact_print(Contact *contact){
    printf("%s\n%s\n%s\n%s\n%s\n%s\n",
           contact->first_name,
           contact->last_name,
           contact->birth_date,
           contact->email,
           contact->phone,
           contact->address);
}




















