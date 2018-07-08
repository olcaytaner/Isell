#ifndef __libmisc_H
#define __libmisc_H

#include<string.h>
#include<stdarg.h>
#include "constants.h"
#include "options.h"

int      atof_check(char* st, double* value, double min, double max);
int      atoi_check(char* st, int* value, int min, int max);
void     check_and_put_in_between(int* x, int left, int right);
void     check_and_set_mean(double* mean, int count);
void     check_and_set_meani(int* mean, int count);
int      checkparams(int paramcount, int must, ...);
void     find_mse_and_sse(char* inname, char* outname1, char* outname2);
int*     findmincounts(char** names, int count);
int      in_list(int searchvalue, int count, ...);
BOOLEAN  is_letter(char ch);
int      is_line_comment(char* line);
int      isfloat(char* st);
int      isinteger(char* st);
int      kronecker_delta(int i, int j);
void     set_precision(char* precisionst, char* before, char* after);
int      sort_function_double_ascending(const void *a, const void *b);
int      sort_function_double_descending(const void *a, const void *b);
int      sort_function_image_object_ascending(const void *a, const void *b);
int      sort_function_image_object_descending(const void *a, const void *b);
int      sort_function_int_ascending(const void *a, const void *b);
int      sort_function_int_descending(const void *a, const void *b);
int      sort_function_observation_ascending(const void *a, const void *b);
int      sort_function_observation_descending(const void *a, const void *b);
int      sort_function_string_ascending(const void *a, const void *b);
int      sort_function_string_descending(const void *a, const void *b);
void     sort_two_arrays(double* array1, int* array2, int size);
void     swap_char(char* i, char* j);
void     swap_double(double* i, double* j);
void     swap_int(int* i, int* j);

#endif
