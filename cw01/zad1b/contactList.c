#include "contactList.h"

ContactListNode *_contact_list_create_node(Contact *contact) {
    ContactListNode *node = malloc(sizeof(ContactListNode));
    if(node != NULL) {
        node->contact = contact;
        node->prev = node;
        node->next = node;
    }
    return node;
}

void _contact_list_delete_node(ContactListNode *node) {
    if(node == NULL || node->contact == NULL)
        return;
    node->prev->next = node->next;
    node->next->prev = node->prev;
    contact_delete(node->contact);
    free(node);
}

ContactListNode *_contact_list_remove_first(ContactList *list) {
    if(list == NULL || list->first->next == list->first)
        return NULL;

    ContactListNode *to_remove = list->first->next;
    list->first->next = to_remove->next;
    to_remove->next->prev = list->first;

    to_remove->prev = to_remove;
    to_remove->next = to_remove;

    return to_remove;
}

void _contact_list_add_node(ContactList *list, ContactListNode *node) {
    if(list == NULL || node == NULL)
        return;

    node->next = list->first;
    node->prev = list->first->prev;
    list->first->prev->next = node;
    list->first->prev = node;
}

ContactList *contact_list_create() {
    ContactList *contact_list = malloc(sizeof(ContactList));
    if(contact_list != NULL) {
        ContactListNode *sll_node = _contact_list_create_node(NULL);
        if(sll_node != NULL)
            contact_list->first = sll_node;
        else
            return NULL;
    }
    return contact_list;
}

void contact_list_delete(ContactList *contact_list) {
    if(contact_list == NULL)
        return;

    while(contact_list->first->next != contact_list->first) {
        _contact_list_delete_node(contact_list->first->next);
    }
    free(contact_list->first);
    free(contact_list);
}

void contact_list_add(ContactList *contact_list, Contact *contact) {
    if(contact_list == NULL || contact == NULL)
        return;

    ContactListNode *node = _contact_list_create_node(contact);
    if(node != NULL)
        _contact_list_add_node(contact_list, node);
}

void contact_list_delete_contact(ContactList *contact_list, Contact *contact) {
    if(contact_list == NULL || contact == NULL || contact_list->first->next == contact_list->first)
        return;

    for(ContactListNode *node = contact_list->first->next; node->contact != NULL; node = node->next) {
        if(contact_equals(node->contact, contact)){
            _contact_list_delete_node(node);
            break;
        }
    }
}

ContactList *contact_list_find(ContactList *contact_list, Field field, char *data) {
    if(contact_list == NULL || contact_list->first->next == contact_list->first)
        return NULL;

    ContactList *find_list = contact_list_create();

    for(ContactListNode *node = contact_list->first->next; node->contact != NULL; node = node->next){
        if(!strcmp(contact_get_field(node->contact, field), data))
            contact_list_add(find_list, node->contact);
    }
    return find_list;
}

void _contact_list_extend(ContactList *list_to_extend, ContactList *list) {
    if(list_to_extend == NULL || list == NULL || list->first->next == list->first)
        return;

    list_to_extend->first->prev->next = list->first->next;
    list->first->next->prev = list_to_extend->first->prev;
    list_to_extend->first->prev = list->first->prev;
    list->first->prev->next = list_to_extend->first;
    list->first->next = list->first;
    list->first->prev = list->first;
    contact_list_delete(list);
}

ContactList *_contact_list_merge(ContactList *list1, ContactList *list2, Field field) {
    if(list1 == NULL || list1->first->next == list1->first)
        return list2;
    if(list2 == NULL || list2->first->next == list2->first)
        return  list1;

    ContactList *merged_list = contact_list_create();

    while(list1->first->next != list1->first && list2->first->next != list2->first) {
        if(contact_compare(list1->first->next->contact, list2->first->next->contact, field) < 0) {
            _contact_list_add_node(merged_list, _contact_list_remove_first(list1));
        } else {
            _contact_list_add_node(merged_list, _contact_list_remove_first(list2));
        }
    }

    _contact_list_extend(merged_list, list1);
    _contact_list_extend(merged_list, list2);
    return merged_list;
}

ContactList *_contact_list_remove_increasing_sequence(ContactList *list, Field field) {
    if(list == NULL || list->first->next == list->first)
        return NULL;

    ContactList *sequence = contact_list_create();
    _contact_list_add_node(sequence, _contact_list_remove_first(list));

    while(list->first->next != list->first &&
            contact_compare(list->first->next->contact, sequence->first->prev->contact, field) >= 0) {
        _contact_list_add_node(sequence, _contact_list_remove_first(list));
    }
    return sequence;
}

void contact_list_sort(ContactList *contact_list, Field field) {
    int is_sorted = 0;
    do {
        ContactList *sorted;
        ContactList *sequence1 = _contact_list_remove_increasing_sequence(contact_list, field);
        ContactList *sequence2 = _contact_list_remove_increasing_sequence(contact_list, field);
        sorted = _contact_list_merge(sequence1, sequence2, field);

        if(contact_list->first->next == contact_list->first)
            is_sorted = 1;
        else {
            while(contact_list->first->next != contact_list->first) {
                sequence1 = _contact_list_remove_increasing_sequence(contact_list, field);
                sequence2 = _contact_list_remove_increasing_sequence(contact_list, field);
                ContactList *merged = _contact_list_merge(sequence1, sequence2, field);
                _contact_list_extend(sorted, merged);
            }
        }
        free(contact_list->first);
        contact_list->first = sorted->first;
        free(sorted);
    } while(!is_sorted);

}

void contact_list_print(ContactList *contact_list) {
    if(contact_list == NULL || contact_list->first->next == contact_list->first)
        return;

    for(ContactListNode *node = contact_list->first->next; node->contact != NULL; node = node->next)
        contact_print(node->contact);
}