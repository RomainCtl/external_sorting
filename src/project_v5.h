/**
 * @file
 * @brief V5 of the system project.
 */

#ifndef PROJECT_V5_H
#define PROJECT_V5_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>	/* time */
#include <unistd.h>	/* Unix std */

#include "system_utils.h"
#include "fifo.h"

/**
 * @brief Function to sort a file using an external sort and nb_split subfiles.
 * @param[in] i_file Name of the file to process.
 * @param[in] o_file Name of the output sorted file.
 * @param[in] nb_split Number of subfiles to use for the external sort.
 * @note No parallelisme.
 **/
void projectV5(const char * i_file, const char * o_file, unsigned long nb_split);

/**
 * @brief Function to sort each file in thread.
 * @param[in] nb_split Index of the subfile in the array of files.
 * @param[in] filenames Array of file to sort names.
 * @param[in] filenames_sort Array of sorted file names.
 **/
void projectV5_sortFiles(pthread_t *thr,
			 unsigned long nb_split,
			 char ** filenames,
			 char ** filenames_sort,
			 fileD *filenames_sorted);

typedef struct arg_sort_v5 {
	unsigned long cpt;
	char *file;
	char *file_sort;
	fileD *f;
} Arg_Sort_v5;

/**
 * @brief Function to sort a temporary subfile and remove it.
 * @param[in] arg Struct arg_sort_v3.
 **/
void *v5_sortFiles(void *arg);

/**
 * @brief Function to split sort-merge a list of sorted subfiles in 2 thread and to do the last merge.
 * @param[in] nb_split Index of the subfile in the array of files.
 * @param[in] filenames_sort Array of sorted file names.
 * @param[in] o_file Nome of the output file where sorted data are written.
 * @note It work in stream. Files are not fully loaded in memory.
 **/
void projectV5_combMerge(pthread_t *thr,
			 unsigned long nb_split,
			 fileD *filenames_sorted,
			 const char * o_file);


typedef struct arg_merge_v5 {
	char *dest;
	unsigned long thr_to_wait;
	pthread_t *thr;
	fileD *f;
} Arg_Merge_v5;

/**
 * @brief Function to sort-merge a list of sorted subfiles.
 * @param[in] arg Struct arg_merge_v3.
 **/
void *v5_mergeFiles(void *arg);



#endif
