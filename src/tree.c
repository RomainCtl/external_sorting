/**
 * @file
 * @brief Implementation of binary tree system.
 */
#include "tree.h"


T_noeud* creerNoeud(char *e, T_noeud *l, T_noeud *r) {
    T_noeud *m;
    m = (T_noeud*) malloc(sizeof(T_noeud));

    m->file = e;
    m->left = l;
    m->right = r;

    return m;
}

int count_noeud(T_noeud *n) {
    if (n == NULL) return 0;
    else return 1 + count_noeud(n->l) + count_noeud(n->r);
}