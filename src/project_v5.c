/**
 * @file
 * @brief Implementation of the V4 of the system project.
 */

#include "project_v5.h"

/**
 * @brief Maximum length (in character) for a file name.
 **/
#define PROJECT_FILENAME_MAX_SIZE 1024

/**
 * @brief Type of the sort algorithm used in this version.
 **/
//#define SORTALGO(nb_elem, values) SU_ISort(nb_elem, values)
//#define SORTALGO(nb_elem, values) SU_HSort(nb_elem, values)
#define SORTALGO(nb_elem, values) SU_QSort(nb_elem, values)

/**********************************/

//gcc -W -Wall -Wextra -Wconversion -Werror -O -g -std=c99 src/project_v2.c -o src/project_v2
void projectV5(const char * i_file, const char * o_file, unsigned long nb_split){

  /* Get number of line to sort */
  int nb_print = 0;
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  unsigned long nb_lines_per_files = nb_lines/ (unsigned long) nb_split;
  fprintf(stderr,
	  "Projet5 version with %lu split of %lu lines\n",
	  nb_split,
	  nb_lines_per_files);

  /* 0 - Deal with case nb_split = 1 */
  if(nb_split < 2){
    int * values = NULL;
    unsigned long nb_elem = SU_loadFile(i_file, &values);

    SORTALGO(nb_elem, values);

    SU_saveFile(o_file, nb_elem, values);

    free(values);
    return;
  }

  /* 1 - Split the source file */

  /* 1.1 - Create a vector of target filenames for the split */
  char ** filenames = (char**) malloc(sizeof(char*) * (size_t) nb_split);
  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){
    filenames[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.txt", getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* 1.2 - Split the source file */
  SU_splitFile2(i_file,
		nb_lines_per_files,
		nb_split,
		(const char **) filenames
	);

  /* 2 - Create Binary Tree */
  int **tubes = (int**) malloc(sizeof(int*) * nb_split);

  T_noeud **noeuds = (T_noeud**) malloc(sizeof(T_noeud*) * (nb_split *2 -1));

  for (cpt=0 ; cpt < nb_split ; cpt++) {
    // create sort noeud
    tubes[cpt] = (int*) malloc(sizeof(int) * 2);
    pipe(tubes[cpt]);
    noeuds[cpt] = create_noeud(cpt, tubes[cpt], filenames[cpt], NULL, NULL);
  }
  for (cpt=0 ; cpt < nb_split-1 ; cpt++) {
    // create merde noeud
    char *file = (char*) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    if (cpt == nb_split -2) {
      strcpy(file, o_file);
    } else {
      nb_print = snprintf(file,
        PROJECT_FILENAME_MAX_SIZE,
        "/tmp/tmp_merge_%d_%lu.txt", getpid(), nb_split+cpt);
      if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
        err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
      }
    }
    noeuds[nb_split + cpt] = create_noeud(nb_split+cpt, NULL, file, noeuds[cpt*2], noeuds[cpt*2 +1]);
  }
  T_noeud *tree = noeuds[nb_split *2 -2];

  /* 3 - Run Trees */
  run_tree_v5(tree, nb_split *2 -2);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]);
  }
  free(filenames);

  for(cpt=0 ; cpt < nb_split*2 -1 ; cpt++) {
    free(noeuds[cpt]);
  }
  free(noeuds);
}

void run_tree_v5(T_noeud *noeud, unsigned long id_last) {
  if (count_noeud(noeud) < 2) {
    // Sort case
    int * values = NULL;
    unsigned long nb_elem = SU_loadFile(noeud->file, &values);
    SU_removeFile(noeud->file);
    fprintf(stderr, "Inner sort %lu : Array of %lu elem by %d\n", noeud->id, nb_elem, getpid());

    SORTALGO(nb_elem, values);

    data_structure_v5 data; // = (data_structure_v5*) malloc(sizeof(data_structure_v5));
    data.nb_elem = nb_elem;
    data.values = values;

    // return value to parent noeud
    close(noeud->fd[0]); // closing read part
    write(noeud->fd[1], &data, sizeof(data));
  }
  else {
    // merge case
    pid_t *fils = (pid_t*) malloc(sizeof(pid_t) * 2);

    fils[0] = fork();
    if (fils[0] == -1){
      perror("Echec du fork\n");
      exit(1);
    } else if (fils[0] == 0) { //fils gauche
      run_tree_v5(noeud->left, id_last); // lance le traitement du fils gauche
    }
    else {
      // pere
      fils[1] = fork();
      if (fils[1] == -1){
        perror("Echec du fork\n");
        exit(1);
      } else if (fils[1] == 0) { //fils droite
        run_tree_v5(noeud->right, id_last); // lance le traitement du fils droite
      }
      else {
        //pere
        data_structure_v5 value_r;// = (data_structure_v5*) malloc(sizeof(data_structure_v5));
        data_structure_v5 value_l;// = (data_structure_v5*) malloc(sizeof(data_structure_v5));

        if (count_noeud(noeud->left) == 1){ // Le fils gauche est un sort
          close(noeud->left->fd[1]); // closing write part
          ssize_t t = read(noeud->left->fd[0], &value_l, sizeof(value_l));
          if (t == -1) {
            perror("Echec de la lecture (pipe)");
            exit(1);
          }
          close(noeud->left->fd[0]);
          fflush(NULL);
        } else { // c'est un merge, on récup le contenu du fichier
          waitpid(fils[0], NULL, 0); // on attend que le fils termine
          fprintf(stderr, "DEBUG : %d completed\n", fils[0]);
          value_l.values = NULL;
          value_l.nb_elem = SU_loadFile(noeud->left->file, &(value_l.values));
          SU_removeFile(noeud->left->file);
        }

        if (count_noeud(noeud->right) == 1){ // Le fils droite est un sort
          close(noeud->right->fd[1]); // closing write part
          ssize_t t = read(noeud->right->fd[0], &value_r, sizeof(value_r));
          if (t == -1) {
            perror("Echec de la lecture (pipe)");
            exit(1);
          }
          close(noeud->left->fd[1]);
          fflush(NULL);
        } else { // c'est un merge, on récup le contenu du fichier
          waitpid(fils[1], NULL, 0); // on attend que le fils termine
          fprintf(stderr, "DEBUG : %d completed\n", fils[1]);
          value_r.values = NULL;
          value_r.nb_elem = SU_loadFile(noeud->right->file, &(value_r.values));
          SU_removeFile(noeud->right->file);
        }

        // Do the merge
        fprintf(stderr, "Merge sort: %lu + %lu -> %lu (%s) by %d\n",
          noeud->right->id,
          noeud->left->id,
          noeud->id,
          noeud->file,
          getpid());
        merge_sorted_data_v5(value_l,
                value_r,
                noeud->file);
      }
    }
  }
  fprintf(stderr, "DEBUG : %lu: %d done\n", noeud->id, getpid());
  if (noeud->id != id_last) exit(0);
}

void merge_sorted_data_v5(const data_structure_v5 values_l, const data_structure_v5 values_r, const char * file_target) {
  int *values = (int*) malloc(sizeof(int) * values_l.nb_elem + values_r.nb_elem);

  fprintf(stderr, "DEBUG : %lu : %lu\n", values_l.nb_elem, values_r.nb_elem);

  fprintf(stderr, "DEBUG : %d\n", values_l.values[0]);

  unsigned long l=0, r=0, i=0;
  while (l < values_l.nb_elem || r < values_r.nb_elem) {
    fprintf(stderr, "DEBUG : prout\n");
    if (l < values_l.nb_elem && r < values_r.nb_elem) {
      if (values_l.values[l] < values_r.values[r]) {
        values[i] = values_l.values[l];
        l++;
      } else {
        values[i] = values_r.values[r];
        r++;
      }
    }
    else if (l < values_l.nb_elem) {
      values[i] = values_l.values[l];
      l++;
    } else { // r < values_r.nb_elem
      values[i] = values_r.values[r];
      r++;
    }
    i++;
  }

  // save file
  fprintf(stderr, "DEBUG : %lu\n", i);
  SU_saveFile(file_target, i, values);
  // free(values);
}