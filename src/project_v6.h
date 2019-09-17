/**
 * @file
 * @brief V6 of the system project.
 */

#ifndef PROJECT_V6_H
#define PROJECT_V6_H

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
void projectV6(const char * i_file, const char * o_file, unsigned long nb_split);

/**
 * @brief Function to run each processus to sort and merge
 * @param[in] noeud T_noeud to sort or merge
 * @param[in] fd int* tube
 **/
void run_tree_v6(T_noeud *noeud, int *fd);

/**
 * @brief Function to merge to array of data
 * @param[in] nb_elem_l unsigned long number of elem in values_l
 * @param[in] values_l int* left part to merge
 * @param[in] nb_elem_r unsigned long number of elem in values_r
 * @param[in] values_r int right part to merge
 * @param[in] values int** array to stock merged data
 *
 * @return long unsigned int number of elem in merged array
 **/
long unsigned int merge_sorted_data_v6(unsigned long nb_elem_l, int* values_l, unsigned long nb_elem_r, int* values_r, int **values);


#endif
