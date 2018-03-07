#ifndef CONTACTTREE_H
#define CONTACTTREE_H

#include "contact.h"

typedef struct ContactTreeNode ContactTreeNode;

struct ContactTreeNode{
    Contact *contact;
    ContactTreeNode *parent;
    ContactTreeNode *left;
    ContactTreeNode *right;
};

typedef struct {
    ContactTreeNode *first;
    Field sorted_by;
} ContactTree;

ContactTree *contact_tree_create(Field sorted_by);

void contact_tree_delete(ContactTree *tree);

void contact_tree_add_contact(ContactTree *tree, Contact *contact);

void contact_tree_delete_contact(ContactTree *tree, Contact *contact);

ContactTree *contact_tree_find(ContactTree *tree, Field field, char *data);

void contact_tree_sort(ContactTree *tree, Field field);

void contact_tree_print(ContactTree *tree);

#endif //CONTACTTREE_H
