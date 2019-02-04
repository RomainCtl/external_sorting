/**
 * @file
 * @brief Dynamique List (FIFO) struct of the system project.
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct elem{
    char *file;
    struct elem *next;
} element;

typedef struct{
    element *Queue;
    element *Tete;
} fileD;

/** \brief creation d'une file
 *
 * \param pointer de file
 *
 */
void creation(fileD *f);

/** \brief verif si est vide
 *
 * \param pointer de fileD
 * \return bool
 *
 */
bool estVide(fileD f);

/** \brief pour ajouter le produit a la file
 *
 * \param le file
 * \param un produit a ajouter
 *
 */
void ajout(fileD *f, char* p);

/** \brief retirer la tete de file
 *
 * \param pointer de file
 *
 */
void retrait(fileD *f);

/** \brief pour obtenir la tete de file
 *
 * \param la file
 * \return un char*
 *
 */
char* obtenirTete(fileD f);

#endif