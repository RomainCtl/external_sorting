/**
 * @file
 * @brief Implementation of fifo list system.
 */
#include "fifo.h"

/**
 * @brief Maximum length (in character) for a file name.
 **/
#define PROJECT_FILENAME_MAX_SIZE 1024

void creation(fileD *f){
    f->Queue=NULL;
    f->Tete=NULL;
}

bool estVide(fileD f){
    return (f.Queue==NULL && f.Tete==NULL);
}

void ajout(fileD *f, char* filename){
    element *p;
    p = (element*)malloc(sizeof(element));
    p->file = (char*) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    strcpy(p->file, filename);
    p->next=NULL;
    if (estVide(*f)){
        f->Queue = (element*) malloc(sizeof(element));
        f->Tete = (element*) malloc(sizeof(element));
        f->Queue=p;
        f->Tete=p;
    } else {
        f->Queue->next=p;
        f->Queue=p;
    }
}

void retrait(fileD *f){
    element *p;
    if (f->Tete==f->Queue){
        p=f->Tete;
        f->Tete=NULL;
        f->Queue=NULL;
        free(p);
    } else {
        p=f->Tete;
        f->Tete=f->Tete->next;
        free(p);
    }
}

char* obtenirTete(fileD f){
    return f.Tete->file;
}
