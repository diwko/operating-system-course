#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <stdlib.h>
#include "contact.h"

typedef struct ContactListNode ContactListNode;
struct ContactListNode {
    Contact *contact;
    ContactListNode *prev;
    ContactListNode *next;
};

typedef struct {
    ContactListNode *first;
} ContactList;

//Return empty ContactList
ContactList *contact_list_create();

//Delete contact_list with all contacts
void contact_list_delete(ContactList *contact_list);

//Add contact at the end of contact_list
void contact_list_add(ContactList *contact_list, Contact *contact);

//Delete contact from contact_list
void contact_list_delete_contact(ContactList *contact_list, Contact *contact);

//Return ContactList with all Contacts which field (FIRST_NAME, LAST_NAME, BIRTH_DATE, EMAIL, PHONE, ADDRESS) = data
ContactList *contact_list_find(ContactList *contact_list, Field field, char *data);

//Sort contact_list by field
void contact_list_sort(ContactList *contact_list, Field field);

//Print list
void contact_list_print(ContactList *contact_list);

#endif //CONTACTLIST_H
