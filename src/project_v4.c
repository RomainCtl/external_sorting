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
	  "Projet3 version with %lu split of %lu lines\n",
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
			"/tmp/tmp_split_%d_%lu.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    filenames_sort[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames_sort[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.sort.txt",getpid(), cpt);
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

  /* 2 - Sort each file */
  projectV4_sortFiles(nb_split, (const char **) filenames, (const char **) filenames_sort);

  /* 3 - Merge (two by two) */
  projectV4_combMerge(nb_split, (const char **) filenames_sort, (const char *) o_file);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]); // not needed :  clear in sort
    free(filenames_sort[cpt]);
  }

  free(filenames);
  free(filenames_sort);

}

void *v4_sortFiles(void *arg) {
  struct arg_sort_v4 *args = arg;
  int * values = NULL;
  unsigned long nb_elem = SU_loadFile(args->file, &values);
  SU_removeFile(args->file);
  fprintf(stderr, "Inner sort %lu: Array of %lu elem by %d\n", args->cpt, nb_elem, getpid());

  SORTALGO(nb_elem, values);

  SU_saveFile(args->file_sort, nb_elem, values);
  free(values);
  pthread_exit(NULL);
}

void projectV4_sortFiles(unsigned long nb_split, const char ** filenames, const char ** filenames_sort){

  pthread_t thr[nb_split];
  struct arg_sort_v4 args[nb_split];

  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){

    args[cpt].cpt = cpt;
    args[cpt].file = filenames[cpt];
    args[cpt].file_sort = filenames_sort[cpt];

    thr[cpt] = 1;
    if (pthread_create(&thr[cpt], NULL, v4_sortFiles, (void *) &args[cpt]) != 0) {
        fprintf ( stderr , "Erreur dans pthread_create %ld\n", cpt);
        exit (EXIT_FAILURE);
    }
  }
  for(cpt = 0; cpt < nb_split; ++cpt)
    if (pthread_join(thr[cpt], NULL))
        fprintf( stderr, "pthread_join %ld\n", cpt);
}

void *v4_mergeFiles(void *arg) {
  struct arg_merge_v4 *args = arg;
  int nb_print = 0;

  char previous_name [PROJECT_FILENAME_MAX_SIZE];
  nb_print = snprintf(previous_name,
          PROJECT_FILENAME_MAX_SIZE,
          "%s", args->filenames_sort[args->cpt]);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  char current_name [PROJECT_FILENAME_MAX_SIZE];
  nb_print = snprintf(current_name,
          PROJECT_FILENAME_MAX_SIZE,
          "/tmp/tmp_split_%ld_merge_%d.txt", args->id, 0);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }


  for(args->cpt = args->cpt + 1; args->cpt < args->nb_split - 1; ++args->cpt) {
    fprintf(stderr, "Merge sort %lu : %s + %s -> %s by %ld\n",
      args->cpt,
      previous_name,
      args->filenames_sort[args->cpt],
      current_name,
      args->id);
    SU_mergeSortedFiles(previous_name,
      args->filenames_sort[args->cpt],
      current_name);
    SU_removeFile(previous_name);
    SU_removeFile(args->filenames_sort[args->cpt]);

    nb_print = snprintf(previous_name,
      PROJECT_FILENAME_MAX_SIZE,
      "%s", current_name);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    nb_print = snprintf(current_name,
      PROJECT_FILENAME_MAX_SIZE,
      "/tmp/tmp_split_%ld_merge_%lu.txt",args->id, args->cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* Last merge for each cascade */
  char d[PROJECT_FILENAME_MAX_SIZE];
  sprintf(d, "%ld", args->id);
  strcat(args->o_file, ".");
  strcat(args->o_file, d);
  strcat(args->o_file, ".txt");

  fprintf(stderr, "Last merge sort of %ld: %s + %s -> %s\n",
    args->id,
	  previous_name,
	  args->filenames_sort[args->nb_split - 1],
	  args->o_file);
  SU_mergeSortedFiles(previous_name,
		      args->filenames_sort[args->nb_split - 1],
		      args->o_file);
  SU_removeFile(previous_name);
  SU_removeFile(args->filenames_sort[args->nb_split - 1]);

  pthread_exit(NULL);
}

void projectV4_combMerge(unsigned long nb_split, const char ** filenames_sort, const char * o_file){
  if (nb_split < 4) {
    perror("Number of split too small !\n");
    exit(1);
  }
  unsigned long NB_THR = 2;
  pthread_t thr[NB_THR];
  struct arg_merge_v4 args[NB_THR];
  unsigned long nb_split_thr = (unsigned long)floor( ((double)nb_split) / ((int) NB_THR));

  for(unsigned long i = 0; i < NB_THR ; ++i){
    args[i].id = i;
    args[i].cpt = i*nb_split_thr;
    args[i].nb_split = i+1 == NB_THR ? (i+1) *nb_split_thr + nb_split%nb_split_thr : (i+1) *nb_split_thr;
    args[i].filenames_sort = (const char**) malloc(sizeof(char*) * (size_t) nb_split);
    args[i].o_file = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    args[i].filenames_sort = filenames_sort;

    strcpy(args[i].o_file, o_file);

    thr[i] = 1;
    if (pthread_create(&thr[i], NULL, v4_mergeFiles, (void *) &args[i]) != 0) {
        fprintf ( stderr , "Erreur dans pthread_create %ld\n", i);
        exit (EXIT_FAILURE);
    }
  }
  for(unsigned long i = 0; i < NB_THR; ++i)
    if (pthread_join(thr[i], NULL))
        fprintf( stderr, "pthread_join %ld\n", i);


  /* Last merge */
  fprintf(stderr, "Last merge sort : %s + %s -> %s\n",
    args[0].o_file,
    args[1].o_file,
    o_file);
  SU_mergeSortedFiles(args[0].o_file,
          args[1].o_file,
          o_file);
  SU_removeFile(args[0].o_file);
  SU_removeFile(args[1].o_file);
}
