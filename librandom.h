#ifndef __librandom_H
#define __librandom_H

#include "constants.h"

int*     bootstrap_array(int size);
int      myrand();
void     mysrand(unsigned newseed);
double   produce_random_value(double start, double end);
int*     random_array(int size);
double*  random_array_double(int size, double start, double end);
BOOLEAN  random_combination(int* combination, int size, int range);
double*  random_normalized_array(int size);

#endif

