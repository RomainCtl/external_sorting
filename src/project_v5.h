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
#include "tree.h"

/**
 * @brief Function to sort a file using an external sort and nb_split subfiles.
 * @param[in] i_file Name of the file to process.
 * @param[in] o_file Name of the output sorted file.
 * @param[in] nb_split Number of subfiles to use for the external sort.
 * @note No parallelisme.
 **/
void projectV5(const char * i_file, const char * o_file, unsigned long nb_split);

/**
 * @brief Function to run each processus to sort and merge
 * @param[in] noeud T_noeud to sort or merge
 **/
void run_tree_v5(T_noeud *noeud, unsigned long id_last);

typedef struct data_v5 {
	unsigned long nb_elem;
	int *values;
} data_structure_v5;

/**
 * @brief Function to merge to array of data
 * @param[in] values_l data_structure_v5 left part to merge
 * @param[in] values_r data_structure_v5 right part to merge
 * @param[in] file_target Name of the output created file
 **/
void merge_sorted_data_v5(const data_structure_v5 values_l, const data_structure_v5 values_r, const char * file_target);


#endif
