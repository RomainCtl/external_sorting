/**
 * @file
 * @brief Dynamic Tree struct of the system project.
 */

#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct noeud {
    unsigned long id;
    int *fd;
    char *file;
    struct noeud *left;
    struct noeud *right;
} T_noeud, Tree;

/** \brief creation d'un noeud d'arbre
 *
 * \param fd : tableau de taille 2 pour les tubes interprocessus
 * \param file : pointer de chaine de caractère, nom du fichier à créer
 * \param l : pointer T_noeud gauche du noeud à créer
 * \param r : pointer T_noeud droite du noeud à créer
 *
 * \return T_noeud
 *
 */
T_noeud* create_noeud(unsigned long id, int fd[], char *file, T_noeud *l, T_noeud *r);

/** \brief verif si est vide
 *
 * \param n : pointer T_noeud, noeud racine de l'arbre
 *
 * \return entier nombre de noeud dans l'arbre
 *
 */
int count_noeud(T_noeud *n);

/**
 * \brief display binary tree in column
 *
 * \param t: pointer T_noeud, noeud to display
 * \param space : int width between left console border and noeud
 * \param p : char prefix ('-' for racine, '\' for left child and '\' for right child)
 *
 **/
void display_tree_by_col(T_noeud *t, int space, char *p);

/**
 * \brief display binary tree
 *
 * \param t : T_noeud tree to display
 *
 **/
void display_tree(T_noeud *t);

#endif