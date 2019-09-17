/**
 * @file
 * @brief Implementation of the V4 of the system project.
 */

#include "project_v4.h"

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
void projectV4(const char * i_file, const char * o_file, unsigned long nb_split){

  /* Get number of line to sort */
  int nb_print = 0;
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  unsigned long nb_lines_per_files = nb_lines/ (unsigned long) nb_split;
  fprintf(stderr,
	  "Projet4 version with %lu split of %lu lines\n",
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
  char ** filenames_sort = (char**) malloc(sizeof(char*) * (size_t) nb_split);
  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){
    filenames[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.txt", getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    filenames_sort[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames_sort[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.sort.txt", getpid(), cpt);
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

  fileD * filenames_sorted = (fileD*) malloc(sizeof(fileD));
  creation(filenames_sorted);
  pthread_t thr[nb_split *2 -1];

  /* 2 - Sort each file */
  projectV4_sortFiles(thr, nb_split, filenames, filenames_sort, filenames_sorted);

  /* 3 - Merge (two by two) */
  projectV4_combMerge(thr, nb_split, filenames_sorted, (const char *) o_file);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]); // not needed :  clear in sort
    free(filenames_sort[cpt]);
  }
  while (!estVide(*filenames_sorted)) retrait(filenames_sorted);

  free(filenames);
  free(filenames_sort);
  free(filenames_sorted);

}

void *v4_sortFiles(void *arg) {
  Arg_Sort_v4 *args = arg;

  int * values = NULL;
  unsigned long nb_elem = SU_loadFile(args->file, &values);
  SU_removeFile(args->file);
  fprintf(stderr, "Inner sort %lu: Array of %lu elem by %d\n", args->cpt, nb_elem, getpid());

  SORTALGO(nb_elem, values);

  SU_saveFile(args->file_sort, nb_elem, values);

  ajout(args->f, args->file_sort);

  free(values);
  pthread_exit(NULL);
}

void projectV4_sortFiles(pthread_t *thr, unsigned long nb_split, char ** filenames, char ** filenames_sort, fileD *filenames_sorted){

  Arg_Sort_v4 *args = (Arg_Sort_v4 *) malloc(sizeof(Arg_Sort_v4) * nb_split);

  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){

    args[cpt].cpt = cpt;
    args[cpt].file = (char*) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    args[cpt].file_sort = (char*) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    args[cpt].file = filenames[cpt];
    args[cpt].file_sort = filenames_sort[cpt];
    args[cpt].f = filenames_sorted;

    thr[cpt] = 1;
    if (pthread_create(&thr[cpt], NULL, v4_sortFiles, (void *) &args[cpt]) != 0) {
        fprintf ( stderr , "Erreur dans pthread_create %ld\n", cpt);
        exit (EXIT_FAILURE);
    }
  }
}

void *v4_mergeFiles(void *arg) {
  Arg_Merge_v4 *args = arg;

  // Wait
  if (pthread_join(args->thr[args->thr_to_wait], NULL) || pthread_join(args->thr[args->thr_to_wait+1], NULL))
      fprintf( stderr, "pthread_join %ld or %ld\n", args->thr_to_wait, args->thr_to_wait+1);

  char *source1 = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
  char *source2 = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
  source1 = obtenirTete(*(args)->f);
  retrait(args->f);
  source2 = obtenirTete(*(args)->f);
  retrait(args->f);

  fprintf(stderr, "Merge sort: %s + %s -> %s\n",
    source1,
    source2,
    args->dest);
  SU_mergeSortedFiles(source1,
		      source2,
		      args->dest);
  SU_removeFile(source1);
  SU_removeFile(source2);

  ajout(args->f, args->dest); // Add in FIFO list

  pthread_exit(NULL);
}

void projectV4_combMerge(pthread_t *thr, unsigned long nb_split, fileD *f, const char * o_file){
  unsigned long nb_merge = 0;
  unsigned long nb_sort_finished = 0;
  int nb_print = 0;

  Arg_Merge_v4 *args = (Arg_Merge_v4*) malloc(sizeof(Arg_Merge_v4) * (nb_split-1));

  // Pour N split, il faut N-1 Thread de merge
  for (nb_merge = 0 ; nb_merge < nb_split -1 ; nb_merge++) {
    args[nb_merge].dest = (char*) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    args[nb_merge].f = f;
    args[nb_merge].thr = thr;
    args[nb_merge].thr_to_wait = nb_sort_finished;
    nb_sort_finished+=2;

    if (nb_merge == nb_split -2) {
      strcpy(args[nb_merge].dest, o_file);
    } else {
      nb_print = snprintf(args[nb_merge].dest,
        PROJECT_FILENAME_MAX_SIZE,
        "/tmp/tmp_split_%d_merge_%lu.txt", getpid(), nb_merge);
      if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
        err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
      }
    }

    thr[nb_split+nb_merge] = 1;
    if (pthread_create(&thr[nb_split+nb_merge], NULL, v4_mergeFiles, (void *) &args[nb_merge]) != 0) {
        fprintf ( stderr , "Erreur dans pthread_create %ld\n", nb_merge);
        exit (EXIT_FAILURE);
    }
  }
  if (pthread_join(thr[nb_split *2 -2], NULL))
    fprintf( stderr, "pthread_join last\n");
}