/**
 * @file
 * @brief Implementation of binary tree system.
 */
#include "tree.h"
#define C 4

T_noeud* create_noeud(unsigned long id, int fd[], char *file, T_noeud *l, T_noeud *r) {
    T_noeud *m;
    m = (T_noeud*) malloc(sizeof(T_noeud));

    m->id = id;
    m->fd = fd;
    m->file = file;
    m->left = l;
    m->right = r;

    return m;
}

int count_noeud(T_noeud *n) {
    if (n == NULL) return 0;
    else return 1 + count_noeud(n->left) + count_noeud(n->right);
}

void display_tree_by_col(T_noeud *t, int space, char *p) {
    space+=C;

    if (t->right != NULL) display_tree_by_col(t->right, space, "/");

    for (int i=C ; i<space-1 ; i++) printf(" ");
    printf("%s%ld\n", p, t->id);

    if (t->left != NULL) display_tree_by_col(t->left, space, "\\");
}

void display_tree(T_noeud *t) {
    display_tree_by_col(t, 1, "-");
}