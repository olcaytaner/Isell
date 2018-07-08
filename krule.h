#ifndef __krule_H
#define __krule_H
#include "typedef.h"

int**                count_feature_combinations_one_vs_rest(Instanceptr instances, int* iter, int size, int* permsize, int positive);
Decisionconditionptr find_best_discrete_condition(Instanceptr growset, int depth, int positive, double* bestgain);
void                 find_best_discrete_condition_1(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain);
void                 find_best_discrete_condition_2(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain);
void                 find_best_discrete_condition_3(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain);
void                 find_best_discrete_condition_4(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain);
Decisionconditionptr find_best_discrete_condition_omnivariate(Instanceptr growset, int positive);
void                 update_best_discrete_condition(Korderingsplit* split, int* perm[], int* rangelist, int* iter, int** featurecounts, int partitioncounts[], double* bestgain);

#endif
