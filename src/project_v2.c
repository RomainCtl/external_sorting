/**
 * @file
 * @brief Implementation of the V2 of the system project.
 */

#include "project_v2.h"

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
void projectV2(const char * i_file, const char * o_file, unsigned long nb_split){

  /* Get number of line to sort */
  int nb_print = 0;
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  unsigned long nb_lines_per_files = nb_lines/ (unsigned long) nb_split;
  fprintf(stderr,
	  "Projet1 version with %lu split of %lu lines\n",
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
  projectV2_sortFiles(nb_split, (const char **) filenames, (const char **) filenames_sort);

  /* 3 - Merge (two by two) */
  projectV2_combMerge(nb_split, (const char **) filenames_sort, (const char *) o_file);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]); // not needed :  clear in sort
    free(filenames_sort[cpt]);
  }

  free(filenames);
  free(filenames_sort);

}

void projectV2_sortFiles(unsigned long nb_split, const char ** filenames, const char ** filenames_sort){

  pid_t fils[nb_split];

  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){
    // init
    fils[cpt] = -1;
    fils[cpt] = fork();
    if (fils[cpt] == -1){
      perror("Error lors d'un fork\n");
      exit(1);
    }

    if (fils[cpt] == 0){
      int * values = NULL;
      unsigned long nb_elem = SU_loadFile(filenames[cpt], &values);
      SU_removeFile(filenames[cpt]);
      fprintf(stderr, "Inner sort %lu: Array of %lu elem by %d\n", cpt, nb_elem, getpid());

      SORTALGO(nb_elem, values);

      SU_saveFile(filenames_sort[cpt], nb_elem, values);
      free(values);
      exit(0);
    }
  }
  wait(NULL);
}

void projectV2_combMerge(unsigned long nb_split, const char ** filenames_sort, const char * o_file){

  int nb_print = 0;
  unsigned long cpt = 0;
  // il faut au minimum nb_split = 4 pour pouvoir faire les merges en 2 cascades
  int can_make_two_cascade = ((int) nb_split) - 4;

  pid_t fils = -1;
  if (can_make_two_cascade >= 0) {
    fils = fork();
    if (fils == -1){
      perror("Error lors d'un fork\n");
      exit(1);
    }

    if (fils == 0) nb_split = (unsigned long)floor( ((double)nb_split) /2);
    else cpt = (unsigned long)floor( ((double)nb_split) /2);
  }

  char previous_name [PROJECT_FILENAME_MAX_SIZE];
  nb_print = snprintf(previous_name,
          PROJECT_FILENAME_MAX_SIZE,
          "%s", filenames_sort[cpt]);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  char current_name [PROJECT_FILENAME_MAX_SIZE];
  nb_print = snprintf(current_name,
          PROJECT_FILENAME_MAX_SIZE,
          "/tmp/tmp_split_%d_merge_%d.txt", getpid(), 0);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  printf("DEBUG %d : cpt %ld  |  nb_split %ld\n", getpid(), cpt, nb_split);

  for(cpt = cpt + 1; cpt < nb_split - 1; ++cpt) {
    fprintf(stderr, "Merge sort %lu : %s + %s -> %s by %d\n",
      cpt,
      previous_name,
      filenames_sort[cpt],
      current_name,
      getpid());
    SU_mergeSortedFiles(previous_name,
      filenames_sort[cpt],
      current_name);
    SU_removeFile(previous_name);
    SU_removeFile(filenames_sort[cpt]);

    nb_print = snprintf(previous_name,
      PROJECT_FILENAME_MAX_SIZE,
      "%s", current_name);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    nb_print = snprintf(current_name,
      PROJECT_FILENAME_MAX_SIZE,
      "/tmp/tmp_split_%d_merge_%lu.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* Last merge for each cascade */
  char o_file_a[PROJECT_FILENAME_MAX_SIZE], d[PROJECT_FILENAME_MAX_SIZE];
  strcpy(o_file_a, o_file);

  if (can_make_two_cascade >= 0) {
    sprintf(d, "%d", fils);
    strcat(o_file_a, ".");
    strcat(o_file_a, d);
    strcat(o_file_a, ".txt");
  }

  fprintf(stderr, "Last merge sort of %d: %s + %s -> %s\n",
    getpid(),
	  previous_name,
	  filenames_sort[nb_split - 1],
	  o_file_a);
  SU_mergeSortedFiles(previous_name,
		      filenames_sort[nb_split - 1],
		      o_file_a);
  SU_removeFile(previous_name);
  SU_removeFile(filenames_sort[nb_split - 1]);

  if (can_make_two_cascade >= 0) {
    if (fils == 0) exit(0);
    else waitpid(fils, NULL, 0);

    char o_file_fils[PROJECT_FILENAME_MAX_SIZE];
    strcpy(o_file_fils, o_file);
    strcat(o_file_fils, ".0.txt");

    /* Last merge */
    fprintf(stderr, "Last merge sort : %s + %s -> %s\n",
      o_file_a,
      o_file_fils,
      o_file);
    SU_mergeSortedFiles(o_file_a,
            o_file_fils,
            o_file);
    SU_removeFile(o_file_a);
    SU_removeFile(o_file_fils);
  }

}
