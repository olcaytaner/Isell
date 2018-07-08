#ifndef __libarray_H
#define __libarray_H

void     add_arrays(double* list1, double* list2, int size);
double*  clone_array(double* source, int size);
double** clone_array_2d(double** source, int size1, int size2);
void*    clone_void_array(void* source, int size);
void     divide_array(double* list1, double div, int size);
double   euclidian_distance(double* list1, double* list2, int p);
void     exchange_array_2d(double** array, int i, int j, int size2);
int      find_bin(double* bins, int bincount, double value);
int      find_max_in_list(int *list, int size);
int      find_max_in_list_double(double* list, int size);
int*     find_max_occured(int *list, int size, int* newsize);
double   find_mean_stdev_of_list(double* list, int size, double* stdev);
double   find_median_in_list_double(double* list, int size);
int      find_min_in_list_double(double* list, int size);
int      find_mod_of_list(int *list, int size, int newlistsize);
int      findMaxOutput(double *Y, int length);
void     init_array(double* list1, int size);
double*  log_of_array(double* list, int size);
void     multiply_array(double* list1, double mul, int size);
double   multiply_two_arrays(double* list1, double* list2, int size);
double*  multiply_two_arrays_by_value(double* list1, double* list2, int size);
void     normalize_array(int* counts, double* posterior, int size);
void     normalize_array_laplace(int* counts, double* posterior, int size);
void     normalize_array_of_doubles(double* list, int size);
void     quicksort_array_2d(double** array, int size2, int index, int sort_type, int start, int end);
int      quicksort_check_array_2d(double** array, int size2, int index, int sort_type, int start, int end);
int      quicksort_partition_array_2d(double** array, int size2, int index, int sort_type, int start, int end);
int      quicksort_three_elements_array_2d(double** array, int size2, int index, int sort_type, int start, int end);
void     search_and_add_into_list(int* list, int* itemcount, int newitem);
int      search_list(int* list, int itemcount, int item);
int      sort_function_array_2d(double** array, int i, int j, int index, int sort_type);
int      sum_of_list(int* list, int size);
double   sum_of_list_double(double* list, int size);
double*  sum_of_two_arrays(double* list1, double* list2, int size);

#endif

