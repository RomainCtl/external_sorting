/**
 * @file
 * @brief V3 of the system project.
 */

#ifndef PROJECT_V3_H
#define PROJECT_V3_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>	/* time */
#include <unistd.h>	/* Unix std */

#include "system_utils.h"

/**
 * @brief Function to sort a file using an external sort and nb_split subfiles.
 * @param[in] i_file Name of the file to process.
 * @param[in] o_file Name of the output sorted file.
 * @param[in] nb_split Number of subfiles to use for the external sort.
 * @note No parallelisme.
 **/
void projectV3(const char * i_file, const char * o_file, unsigned long nb_split);

/**
 * @brief Function to sort each file in thread.
 * @param[in] nb_split Index of the subfile in the array of files.
 * @param[in] filenames Array of file to sort names.
 * @param[in] filenames_sort Array of sorted file names.
 **/
void projectV3_sortFiles(unsigned long nb_split,
			 const char ** filenames,
			 const char ** filenames_sort);

struct arg_sort_v3 {
	unsigned long cpt;
	const char *file;
	const char *file_sort;
};

/**
 * @brief Function to sort a temporary subfile and remove it.
 * @param[in] arg Struct arg_sort_v3.
 **/
void *v3_sortFiles(void *arg);

/**
 * @brief Function to split sort-merge a list of sorted subfiles in 2 thread and to do the last merge.
 * @param[in] nb_split Index of the subfile in the array of files.
 * @param[in] filenames_sort Array of sorted file names.
 * @param[in] o_file Nome of the output file where sorted data are written.
 * @note It work in stream. Files are not fully loaded in memory.
 **/
void projectV3_combMerge(unsigned long nb_split,
			 const char ** filenames_sort,
			 const char * o_file);

struct arg_merge_v3 {
	unsigned long id;
	unsigned long cpt;
	unsigned long nb_split;
	const char **filenames_sort;
	char *o_file;
};

/**
 * @brief Function to sort-merge a list of sorted subfiles.
 * @param[in] arg Struct arg_merge_v3.
 **/
void *v3_mergeFiles(void *arg);

#endif
