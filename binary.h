#ifndef __binary_H
#define __binary_H

#include "constants.h"

/*!Structure containing AND'ed binary variables. The variables may also be in the complement form. For example, x1x2'x4 contains three variables x1, x2, and x4 where x2 is complemented.
*/
struct binaryterm{
                  /*!Indices of the variables. For the example above, variables[0] = 1, variables[1] = 2 and variables[2] = 4*/
                  int* variables;
                  /*!Are the variables complemented in the term. If the value is zero not complemented, 1 if complemented. For the example above, complement[0] = 0, complement[1] = 1, complement[2] = 0*/                  
                  BOOLEAN* complement;
                  /*!Number of variables in the term. For the example above, count = 3.*/                  
                  int count;
};

typedef struct binaryterm Binaryterm;
typedef Binaryterm* Binarytermptr;

/*! Structure containing OR'ed binary terms. For example, x1x2'+x1'x2 is a binary function*/
struct binaryfunction{
                      /*!AND'ed binary terms stored as an array.*/
                      Binarytermptr terms;
                      /*!Number of OR'ed binary terms. Number of elements in the terms array*/
                      int termcount;
};

typedef struct binaryfunction Binaryfunction;
typedef Binaryfunction* Binaryfunctionptr;

void              add_binary_term(Binaryfunctionptr function, Binaryterm term);
Binaryfunctionptr empty_binary_function();
BOOLEAN           evaluate_binary_function(Binaryfunctionptr bf, int* values);
void              free_binary_function(Binaryfunctionptr bf);
void              generate_random_data_from_binary_function(Binaryfunctionptr bf, int d, char* fname);
void              generate_random_noisy_data_from_binary_function(Binaryfunctionptr bf, int d, double p, char* fname);
Binaryfunctionptr read_binary_function(char* st);

#endif
