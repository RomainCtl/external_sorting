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
  T_noeud **noeuds = (T_noeud**) malloc(sizeof(T_noeud*) * (nb_split *2 -1));

  for (cpt=0 ; cpt < nb_split ; cpt++) {
    /* Sort case */
    noeuds[cpt] = create_noeud(cpt, NULL, filenames[cpt], NULL, NULL);
  }

  for (cpt=0 ; cpt < nb_split-1 ; cpt++) {
    /* Merge case */
    char *file = (char*) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    if (cpt == nb_split-2) {
      strcpy(file, o_file);
    } else {
      nb_print = snprintf(file,
        PROJECT_FILENAME_MAX_SIZE,
        "/tmp/tmp_merge_%d_%lu.txt", getpid(), nb_split+cpt);
      if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
        err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
      }
    }
    noeuds[nb_split+cpt] = create_noeud(nb_split+cpt, NULL, file, noeuds[cpt*2], noeuds[cpt*2 +1]);
  }
  T_noeud *tree = noeuds[nb_split *2 -2];

  display_tree(tree);

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
    /* Sort case */
    int * values = NULL;
    unsigned long nb_elem = SU_loadFile(noeud->file, &values);
    SU_removeFile(noeud->file);
    fprintf(stderr, "Inner sort %lu : Array of %lu elem by %d\n", noeud->id, nb_elem, getpid());

    SORTALGO(nb_elem, values);

    // send value to parent noeud
    close(noeud->fd[0]);
    assert( write(noeud->fd[1], &nb_elem, sizeof(nb_elem)) > 0 );
    assert( write(noeud->fd[1], &values, sizeof(values)) > 0 );
    close(noeud->fd[1]);

    fprintf(stderr, "DEBUG : %lu: %d done\n", noeud->id, getpid());
    exit(0);
  }
  else {
    /* Merge case */
    pid_t fils_l=-1, fils_r=-1;
    noeud->left->fd = (int*) malloc(sizeof(int) * 2);

    assert( pipe(noeud->left->fd) != -1 );
    assert( (fils_l = fork()) != -1 );

    /* Left child */
    if (fils_l == 0) run_tree_v5(noeud->left, id_last);
    else {
      /* father */
      noeud->right->fd = (int*) malloc(sizeof(int) * 2);

      assert( pipe(noeud->right->fd) != -1 );
      assert( (fils_r = fork()) != -1 );

      /* Right child */
      if (fils_r == 0) run_tree_v5(noeud->right, id_last);
      else {
        /* father */
        int *values_l=NULL, *values_r=NULL;
        unsigned long nb_elem_l=0, nb_elem_r=0;

        /* close unneeded */
        close(noeud->left->fd[1]);
        close(noeud->right->fd[1]);

        if (count_noeud(noeud->left) == 1){
          /* Case : Left child is sort process */
          assert( read(noeud->left->fd[0], &nb_elem_l, sizeof(nb_elem_l)) != -1 );
          assert( read(noeud->left->fd[0], &values_l, sizeof(values_l)) != -1 );
          // FIXME why i can't access to values_l tab ?? ('/home/romain/Documents/School/system/external_sorting/test.c' work with tab of 10000000 value)
        } else {
          /* Case : Left child is merge process */
          waitpid(fils_l, NULL, 0);
          fprintf(stderr, "DEBUG : %d completed\n", fils_l);

          nb_elem_l = SU_loadFile(noeud->left->file, &values_l);
          SU_removeFile(noeud->left->file);
        }
        close(noeud->left->fd[0]);

        if (count_noeud(noeud->right) == 1){
          /* Case : Right child is sort process */
          assert( read(noeud->right->fd[0], &nb_elem_r, sizeof(nb_elem_r)) != -1 );
          assert( read(noeud->right->fd[0], &values_r, sizeof(values_r)) != -1 );
          // FIXME why i can't access to values_r tab ?? ('/home/romain/Documents/School/system/external_sorting/test.c' work with tab of 10000000 value)
        } else {
          /* Case : Right child is merge process */
          waitpid(fils_r, NULL, 0);
          fprintf(stderr, "DEBUG : %d completed\n", fils_r);

          nb_elem_r = SU_loadFile(noeud->right->file, &values_r);
          SU_removeFile(noeud->right->file);
        }
        close(noeud->right->fd[0]);

        /* Merge */
        fprintf(stderr, "Merge sort: %lu + %lu -> %lu (%s) by %d\n",
          noeud->right->id,
          noeud->left->id,
          noeud->id,
          noeud->file,
          getpid());
        merge_sorted_data_v5(nb_elem_l, values_l, nb_elem_r, values_r, noeud->file);
      }
    }
  }
  fprintf(stderr, "DEBUG : %lu: %d done\n", noeud->id, getpid());
  if (noeud->id != id_last) exit(0);
}

void merge_sorted_data_v5(unsigned long nb_elem_l, int* values_l, unsigned long nb_elem_r, int* values_r, const char * file_target) {
  int *values = (int*) malloc(sizeof(int) * (nb_elem_l + nb_elem_r));

  fprintf(stderr, "DEBUG : %lu : %lu\n", nb_elem_l, nb_elem_r);

  unsigned long l=0, r=0, i=0;
  while (l < nb_elem_l || r < nb_elem_r) {
    fprintf(stderr, "DEBUG : prout\n");
    if (l < nb_elem_l && r < nb_elem_r) {
      if (values_l[l] < values_r[r]) {
        values[i] = values_l[l];
        l++;
      } else {
        values[i] = values_r[r];
        r++;
      }
    }
    else if (l < nb_elem_l) {
      values[i] = values_l[l];
      l++;
    } else { // r < nb_elem_r
      values[i] = values_r[r];
      r++;
    }
    i++;
  }

  // save file
  fprintf(stderr, "DEBUG : %lu\n", i);
  SU_saveFile(file_target, i, values);
  // free(values);
}