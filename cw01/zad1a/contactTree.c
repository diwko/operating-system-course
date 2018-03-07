#include "contactTree.h"

ContactTreeNode *_contact_tree_create_node(Contact *contact) {
    ContactTreeNode *node = malloc(sizeof(ContactTreeNode));
    if(node != NULL) {
        node->contact = contact;
        node->parent = node;
        node->left = node;
        node->right = node;
    }
    return  node;
}

ContactTreeNode *_contact_tree_remove_node(ContactTreeNode *node) {
    if(node == NULL || node->contact == NULL)
        return NULL;

    if(node->left->contact != NULL && node->right->contact != NULL) {
        ContactTreeNode *succ = node->left;
        while(succ->right->contact != NULL)
            succ = succ->right;

        ContactTreeNode *tmp = node->left;
        node->left = succ->left;
        succ->left = tmp;
        tmp = node->right;
        node->right = succ->right;
        succ->right = tmp;
        if(node->parent->left == node)
            node->parent->left = succ;
        else
            node->parent->right = succ;
        if(succ->parent->left == succ)
            succ->parent->left = node;
        else
            succ->parent->right = node;
        tmp = node->parent;
        node->parent = succ->parent;
        succ->parent = node->parent;

        return _contact_tree_remove_node(node);

    } else if (node->left->contact != NULL) {
        if(node->parent->left == node) {
            node->parent->left = node->left;
            node->left->parent = node->parent;
        } else {
            node->parent->right = node->left;
            node->left->parent = node->parent;
        }
    } else if (node->right->contact != NULL) {
        if(node->parent->left == node) {
            node->parent->left = node->right;
            node->right->parent = node->parent;
        } else {
            node->parent->right = node->right;
            node->right->parent = node->parent;
        }
    } else {
        if(node->parent->left == node)
            node->parent->left = node->right;
        else
            node->parent->right = node->right;
    }
    node->parent = node;
    node->left = node;
    node->right = node;
    return node;
}

void _contact_tree_delete_node(ContactTreeNode *node) {
    if(node == NULL)
        return;
    ContactTreeNode *to_delete = _contact_tree_remove_node(node);
    contact_delete(to_delete->contact);
    free(to_delete);
}

ContactTree *contact_tree_create(Field sorted_by) {
    ContactTree *contact_tree = malloc(sizeof(ContactTree));
    if(contact_tree != NULL) {
        contact_tree->sorted_by = sorted_by;
        ContactTreeNode *sll_node = _contact_tree_create_node(NULL);
        if(sll_node != NULL)
            contact_tree->first = sll_node;
        else
            return NULL;
    }
    return contact_tree;
}

void _contact_tree_free_all(ContactTreeNode *node) {
    if(node->contact == NULL)
        return;
    _contact_tree_free_all(node->left);
    _contact_tree_free_all(node->right);
    contact_delete(node->contact);
    free(node);
}

void contact_tree_delete(ContactTree *tree) {
    if(tree == NULL)
        return;
    _contact_tree_free_all(tree->first->left);
    free(tree->first);
    free(tree);
}

void _contact_tree_add_node(ContactTree *tree, ContactTreeNode *node) {
    if(tree == NULL || node == NULL || node->contact == NULL)
        return;

    node->left = tree->first;
    node->right = tree->first;

    if(tree->first->left == tree->first) {
        tree->first->left = node;
        tree->first->right = node;
        node->parent = tree->first;
        return;
    }

    ContactTreeNode *finder = tree->first->left;
    while(finder != tree->first) {
        if(contact_compare(node->contact, finder->contact, tree->sorted_by) <= 0) {
            if(finder->left == tree->first) {
                finder->left = node;
                node->parent = finder;
                return;
            } else
                finder = finder->left;
        } else {
            if(finder->right == tree->first) {
                finder->right = node;
                node->parent = finder;
                return;
            } else
                finder = finder->right;
        }
    }
}

void contact_tree_add_contact(ContactTree *tree, Contact *contact) {
    if(tree == NULL || contact == NULL)
        return;

    ContactTreeNode *to_add = _contact_tree_create_node(contact);
    _contact_tree_add_node(tree, to_add);
}

ContactTreeNode *_contact_tree_find_node(ContactTree *tree, ContactTreeNode *start_node, Contact *contact) {
    if(start_node == NULL || start_node->contact == NULL || contact == NULL)
        return NULL;

    if(contact_equals(contact, start_node->contact))
        return start_node;

    if(contact_compare(contact, start_node->contact, tree->sorted_by) <= 0)
        return _contact_tree_find_node(tree, start_node->left, contact);
    else
        return _contact_tree_find_node(tree, start_node->right, contact);
}

void contact_tree_delete_contact(ContactTree *tree, Contact *contact) {
    if(tree == NULL || contact == NULL)
        return;

    ContactTreeNode *to_delete = _contact_tree_find_node(tree, tree->first->left, contact);
    _contact_tree_delete_node(to_delete);
}

void _contact_tree_find_all(ContactTreeNode *start_node, Field field, char *data, ContactTree *result){
    if(start_node == NULL || start_node->contact == NULL || data == NULL || result == NULL)
        return;

    if(strcmp(data, contact_get_field(start_node->contact, field)) == 0)
        contact_tree_add_contact(result, start_node->contact);

    _contact_tree_find_all(start_node->left, field, data, result);
    _contact_tree_find_all(start_node->right, field, data, result);
}

ContactTree *contact_tree_find(ContactTree *tree, Field field, char *data) {
    if(tree == NULL || data == NULL)
        return NULL;

    ContactTree *result = contact_tree_create(tree->sorted_by);

    ContactTreeNode *finder = tree->first->left;

    if(field == tree->sorted_by) {
        while(finder->contact != NULL) {
            int comparing_result = strcmp(data, contact_get_field(finder->contact, field));
            if(comparing_result == 0) {
                contact_tree_add_contact(result, finder->contact);
                finder = finder->left;
            } else {
                if(comparing_result < 0)
                    finder = finder->left;
                else if(comparing_result > 0)
                    finder = finder->right;
            }
        }
    } else
        _contact_tree_find_all(tree->first->left, field, data, result);

    return result;
}

void _contact_tree_repin(ContactTreeNode *start_node, ContactTree *new_tree) {
    if(start_node == NULL || start_node->contact == NULL)
        return;

    if(start_node->left->contact == NULL && start_node->right->contact == NULL) {
        ContactTreeNode *removed = _contact_tree_remove_node(start_node);
        _contact_tree_add_node(new_tree, removed);
    } else {
        _contact_tree_repin(start_node->left, new_tree);
        _contact_tree_repin(start_node->right, new_tree);

        ContactTreeNode *removed = _contact_tree_remove_node(start_node);
        _contact_tree_add_node(new_tree, removed);
    }
}

void contact_tree_sort(ContactTree *tree, Field field) {
    if(tree == NULL || tree->sorted_by == field)
        return;

    ContactTree *new_tree = contact_tree_create(field);
    _contact_tree_repin(tree->first->left, new_tree);

    tree->first = new_tree->first;
    tree->sorted_by = field;
    free(new_tree);
}

void _contact_tree_print_from(ContactTreeNode *start_node) {
    if(start_node == NULL || start_node->contact == NULL)
        return;

    _contact_tree_print_from(start_node->left);
    contact_print(start_node->contact);
    _contact_tree_print_from(start_node->right);
}

void contact_tree_print(ContactTree *tree) {
    if(tree == NULL)
        return;
    _contact_tree_print_from(tree->first->left);
}