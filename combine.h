#ifndef __combine_H
#define __combine_H
#include "matrix.h"

int max_combine(int* predicted, matrix posteriors);
int mean_combine(int* predicted, matrix posteriors);
int median_combine(int* predicted, matrix posteriors);
int min_combine(int* predicted, matrix posteriors);
int prod_combine(int* predicted, matrix posteriors);
int vote_combine(int* predicted, matrix posteriors);

#endif
