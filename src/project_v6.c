/**
 * @file
 * @brief Implementation of the V4 of the system project.
 */

#include "project_v6.h"

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
void projectV6(const char * i_file, const char * o_file, unsigned long nb_split){

  /* Get number of line to sort */
  int nb_print = 0;
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  unsigned long nb_lines_per_files = nb_lines/ (unsigned long) nb_split;
  fprintf(stderr,
	  "Projet6 version with %lu split of %lu lines\n",
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

  /* 2.1 - Sort cases */
  for (cpt=0 ; cpt < nb_split ; cpt++)
    noeuds[cpt] = create_noeud(cpt, NULL, filenames[cpt], NULL, NULL);

  /* 2.2 - Merge cases */
  for (cpt=0 ; cpt < nb_split-1 ; cpt++)
    noeuds[nb_split+cpt] = create_noeud(nb_split+cpt, NULL, NULL, noeuds[cpt*2], noeuds[cpt*2 +1]);

  T_noeud *tree = noeuds[nb_split *2 -2];
  fprintf(stderr, "Display of Binary Tree\n");
  display_tree(tree);

  /* 3 - Run Tree */
  pid_t fils=-1;
  int *fd = (int*) malloc(sizeof(int) *2);

  /* 3.1 - Create child process to run algo */
  assert( pipe(fd) != -1 );
  assert( (fils = fork()) != -1 );

  if (fils == 0) run_tree_v6(tree, fd);

  /* 3.2 - Read Sorted values and save */
  int *values = NULL;
  unsigned long nb_elem = 0;

  close(fd[1]);
  assert( read(fd[0], &nb_elem, sizeof(nb_elem)) != -1 );
  assert( read(fd[0], &values, sizeof(values)) != -1 );
  close(fd[0]);

  SU_saveFile(o_file, nb_elem, values);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt)
    free(filenames[cpt]);
  free(filenames);

  for(cpt=0 ; cpt < nb_split*2 -1 ; cpt++)
    free(noeuds[cpt]);
  free(noeuds);
}

void run_tree_v6(T_noeud *noeud, int *fd) {
  int * values = NULL;
  unsigned long nb_elem = 0;
  if (count_noeud(noeud) < 2) {
    /* Sort case */
    nb_elem = SU_loadFile(noeud->file, &values);
    SU_removeFile(noeud->file);
    fprintf(stderr, "Inner sort %lu : Array of %lu elem by %d\n", noeud->id, nb_elem, getpid());

    SORTALGO(nb_elem, values);
  }
  else {
    /* Merge case */
    pid_t fils_l=-1;
    int *fd_l = (int*) malloc(sizeof(int) * 2);

    assert( pipe(fd_l) != -1 );
    assert( (fils_l = fork()) != -1 );

    /* Launch first child (left) */
    if (fils_l == 0)
      run_tree_v6(noeud->left, fd_l);
    else {
      /* Father */
      pid_t fils_r=-1;
      int *fd_r = (int*) malloc(sizeof(int) * 2);

      assert( pipe(fd_r) != -1 );
      assert( (fils_r = fork()) != -1 );

      /* Launch second child (right) */
      if (fils_r == 0)
        run_tree_v6(noeud->right, fd_r);
      else {
        /* Father */
        int *values_l=NULL, *values_r=NULL;
        unsigned long nb_elem_l=0, nb_elem_r=0;

        /* close unneeded */
        close(fd_l[1]);
        close(fd_r[1]);

        /* Get data from Left child */
        assert( read(fd_l[0], &nb_elem_l, sizeof(nb_elem_l)) != -1 );
        assert( read(fd_l[0], &values_l, sizeof(values_l)) != -1 );
        close(fd_l[0]);

        /* Get data from Right child */
        assert( read(fd_r[0], &nb_elem_r, sizeof(nb_elem_r)) != -1 );
        assert( read(fd_r[0], &values_r, sizeof(values_r)) != -1 );
        close(fd_r[0]);

        /* Merge */
        fprintf(stderr, "Merge sort: %lu + %lu -> %lu by %d\n", noeud->right->id, noeud->left->id, noeud->id, getpid());

        nb_elem = merge_sorted_data_v6(nb_elem_l, values_l, nb_elem_r, values_r, &values);
      }
    }
  }
  /* Send value to parent noeud */
  close(fd[0]);
  assert( write(fd[1], &nb_elem, sizeof(nb_elem)) > 0 );
  assert( write(fd[1], &values, sizeof(values)) > 0 );
  close(fd[1]);

  fprintf(stderr, "DEBUG : Process (%lu: %d) done successfully\n", noeud->id, getpid());
  exit(0);
}

long unsigned int merge_sorted_data_v6(unsigned long nb_elem_l, int* values_l, unsigned long nb_elem_r, int* values_r, int **values) {
  fprintf(stderr, "DEBUG : nb_elem_l : %lu | nb_elem_r : %lu\n", nb_elem_l, nb_elem_r);
  *values = (int*) malloc(sizeof(int) * (nb_elem_l + nb_elem_r));

  unsigned long l=0, r=0, i=0;
  while (l < nb_elem_l || r < nb_elem_r) {
    if (l < nb_elem_l && r < nb_elem_r) {
      if (values_l[l] < values_r[r]) {
        *values[i] = values_l[l];
        l++;
      } else {
        *values[i] = values_r[r];
        r++;
      }
    }
    else if (l < nb_elem_l) {
      *values[i] = values_l[l];
      l++;
    } else { // r < nb_elem_r
      *values[i] = values_r[r];
      r++;
    }
    i++;
  }
  fprintf(stderr, "DEBUG : merge done for %d (nb_elem=%lu)\n", getpid(), i);
  return i;
}