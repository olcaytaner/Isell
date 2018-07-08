#ifndef __ktree_H
#define __ktree_H
#include "univariatetree.h"

Korderingsplit    copy_of_korderingsplit(Korderingsplit ksplit);
Decisioncondition create_kordering_condition(Korderingsplit ksplit, char comparison);
int               create_ktreechildren(Decisionnodeptr node, C45_parameterptr p);
Korderingsplit    find_best_discrete_split(Decisionnodeptr node, int depth, double* gain);
void              find_best_discrete_split_1(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain);
void              find_best_discrete_split_2(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain);
void              find_best_discrete_split_3(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain);
void              find_best_discrete_split_4(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain);
int               make_children_discrete(Decisionnodeptr node);
void              update_best_discrete_split(Korderingsplit* split, int* perm[], int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain);


#endif
