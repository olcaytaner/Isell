#ifndef __libiteration_H
#define __libiteration_H

#include "constants.h"

BOOLEAN  bypass_iterator(int* iterator, int size, int range, int bypassno);
BOOLEAN  bypass_iterator_from_list(int* iterator, int* list, int* indexlist, int size, int range, int bypassno);
int*     first_combination(int size);
int*     first_combination_from_list(int size, int* list, int** indexlist);
int*     first_iterator(int size);
int*     first_iterator_from_list(int size, int* list, int** indexlist);
int*     first_mixed_iterator(int size);
int*     first_permutation(int size);
int**    generate_permutations(int howmany, int*total);
int**    generate_subsets(int howmany, int *total, int** counts);
BOOLEAN  is_subset(int* set1, int* set2, int size1, int size2);
BOOLEAN  next_combination(int* combination, int size, int range);
BOOLEAN  next_combination_from_list(int* combination, int* list, int* indexlist, int size, int range);
BOOLEAN  next_iterator(int* iterator, int size, int range);
BOOLEAN  next_iterator_from_list(int* iterator, int* list, int* indexlist, int size, int range);
BOOLEAN  next_mixed_iterator(int* iterator, int size, int* rangelist);
BOOLEAN  next_permutation(int* permutation, int size);

#endif
