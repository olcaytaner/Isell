#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"

void add_arrays(double* list1, double* list2, int size)
{
 /*!Last Changed 14.03.2007 Oya*/
 int i;
 for (i = 0; i < size; i++)
   list1[i] = list1[i] + list2[i];
}

double* clone_array(double* source, int size)
{
 /*!Last Changed 15.03.2007 Oya*/
 double* result;
 result = safemalloc(size * sizeof(double), "clone_array", 3);
 memcpy(result, source, size * sizeof(double));
 return result;
}

void* clone_void_array(void* source, int size)
{
 void* result;
 result = safemalloc(size, "clone_void_array", 2);
 memcpy(result, source, size);
 return result;
}

double** clone_array_2d(double** source, int size1, int size2)
{
 /*!Last Changed 15.03.2007 Oya*/
 int i;
 double** result;
 result = (double **) safemalloc_2d(sizeof(double*), sizeof(double), size1, size2, "clone_array_2d", 3);
 for (i = 0; i < size1; i++)
   memcpy(result[i], source[i], size2 * sizeof(double));
 return result;
}

int sort_function_array_2d(double** array, int i, int j, int index, int sort_type)
{
 if (sort_type == ASCENDING)
  {
   if (array[i][index] < array[j][index])
     return -1;
   else
     if (array[i][index] > array[j][index])
       return 1;
     else
       return 0;
  }
 else
  {
   if (array[i][index] < array[j][index])
     return 1;
   else
     if (array[i][index] > array[j][index])
       return -1;
     else
       return 0;
  }
}

void exchange_array_2d(double** array, int i, int j, int size2)
{
 double* tmp;
 tmp = safemalloc(size2 * sizeof(double), "exchange_array_2d", 2);
 memcpy(tmp, array[i], size2 * sizeof(double));
 memcpy(array[i], array[j], size2 * sizeof(double));
 memcpy(array[j], tmp, size2 * sizeof(double));
 safe_free(tmp);
}

int quicksort_three_elements_array_2d(double** array, int size2, int index, int sort_type, int start, int end)
{
 int mid = (start + end) / 2;
 if (sort_function_array_2d(array, mid, start, index, sort_type) < 0)
   exchange_array_2d(array, start, mid, size2);
 if (sort_function_array_2d(array, end, mid, index, sort_type) < 0)
   exchange_array_2d(array, end, mid, size2);
 if (sort_function_array_2d(array, mid, start, index, sort_type) < 0)
   exchange_array_2d(array, mid, start, size2);
 exchange_array_2d(array, mid, end, size2);
 return end;
}

int quicksort_partition_array_2d(double** array, int size2, int index, int sort_type, int start, int end)
{
 int i = start - 1, j, pivot;
 pivot = quicksort_three_elements_array_2d(array, size2, index, sort_type, start, end);
 for (j = start; j < end; j++)
   if (sort_function_array_2d(array, j, pivot, index, sort_type) <= 0)
    {
     i++;
     exchange_array_2d(array, i, j, size2);
    }
 exchange_array_2d(array, i + 1, end, size2);
 return i + 1;
}

int quicksort_check_array_2d(double** array, int size2, int index, int sort_type, int start, int end)
{
 int i;
 for (i = start + 1; i <= end; i++)
   if (sort_function_array_2d(array, i - 1, i, index, sort_type) > 0)
     return 0;
 return 1;
}

void quicksort_array_2d(double** array, int size2, int index, int sort_type, int start, int end)
{
 int pivot;
 if (start < end)
  {
   if (quicksort_check_array_2d(array, size2, index, sort_type, start, end))
     return;
   pivot = quicksort_partition_array_2d(array, size2, index, sort_type, start, end);
   quicksort_array_2d(array, size2, index, sort_type, start, pivot - 1);
   quicksort_array_2d(array, size2, index, sort_type, pivot + 1, end);
  }
}

void divide_array(double* list1, double div, int size)
{
 /*!Last Changed 13.03.2007 Oya*/
 int i;
 for (i = 0; i < size; i++)
   if (div != 0)
     list1[i] /= div;
   else
    {
     printf(m1377, "divide_array");
     list1[i] = 0.0;
    }
}

int find_bin(double* bins, int bincount, double value)
{
 /*!Last Changed 26.01.2005*/
 int i;
 double sum = 0;
 for (i = 0; i < bincount; i++)
  {
   sum += bins[i];
   if (value < sum)
     return i;
  }
 return bincount - 1;
}

int find_max_in_list(int *list,int size)
{
/*!Last Changed 14.07.2001*/
 int max = -INT_MAX, i, res = -1;
 for (i = 0; i < size; i++)
   if (list[i] > max)
    {
     max = list[i];
     res = i;
    }
 return res;
}

int find_max_in_list_double(double* list, int size)
{ 
 double max = 0;
 int i, res = -1;
 for (i = 0; i < size; i++)
   if (list[i] != -1)
     if (list[i] > max)
      {
       max = list[i];
       res = i;
      }
 return res;
}

int* find_max_occured(int *list, int size, int* newsize)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 09.05.2003*/
 int *result, i, *tmplist, j = 0, tmp;
 (*newsize) = 0;
 for (i = 0; i < size; i++)
   if (list[i] > 0)
     (*newsize)++;
 result = safemalloc((*newsize) * sizeof(int), "find_max_occured", 6);
 tmplist = safemalloc((*newsize) * sizeof(int), "find_max_occured", 7);
 for (i = 0; i < size; i++)
   if (list[i] > 0)
    {
     result[j] = i;
     tmplist[j] = list[i];
     j++;
    }
 for (i = 1; i < (*newsize); i++)
  {
   j = i;
   while (j > 0 && tmplist[j] >= tmplist[j-1])
    {
     tmp = tmplist[j];
     tmplist[j] = tmplist[j-1];
     tmplist[j-1] = tmp;
     tmp = result[j];
     result[j] = result[j-1];
     result[j-1] = tmp;
     j--;
    }
  }
 safe_free(tmplist);
 return result;
}

double find_mean_stdev_of_list(double* list, int size, double* stdev)
{
 /*!Last Changed 26.01.2005*/
 int i;
 double sum = 0, mean;
 for (i = 0; i < size; i++)
   sum += list[i];
 mean = sum / size;
 sum = 0;
 for (i = 0; i < size; i++)
   sum += (list[i] - mean) * (list[i] - mean);
 *stdev = sqrt(sum / (size - 1));
 return mean;
}

double find_median_in_list_double(double* list, int size)
{
 /*!Last Changed 06.05.2008*/ 
 qsort(list, size, sizeof(double), sort_function_double_ascending);
 return (list[size / 2]); 
}

int find_min_in_list_double(double* list, int size)
{
 /*!Last Changed 01.02.2005*/
 double min = +INT_MAX;
 int i, res = -1;
 for (i = 0; i < size; i++)
   if (list[i] != -1)
     if (list[i] < min)
      {
       min = list[i];
       res = i;
      }
 return res;
}

int find_mod_of_list(int *list, int size, int newlistsize)
{
 /*!Last Changed 03.03.2006 added by aydin*/
 int mod, *newlist, i;
 newlist = safecalloc(newlistsize, sizeof(int), "find_mod_of_list", 2);
 for (i = 0 ; i < size ; i++)
   if (list[i] >= 0 && list[i] < newlistsize)
     newlist[list[i]]++;
   else
     printf(m1322, "find_mod_of_list");
 mod = find_max_in_list(newlist, newlistsize);
 safe_free(newlist);
 return mod;
}

int findMaxOutput(double *Y, int length)
{
 int max = 0;
 int c;
 for (c = 0; c < length; c++)
  {
   if (Y[max] <= Y[c])
     max = c;
  }
 return max;
}

void init_array(double* list1, int size)
{
 /*!Last Changed 15.03.2007 Oya*/
 int i;
 for (i = 0; i < size; i++)
   list1[i] = 0.0;
}

double* log_of_array(double* list, int size)
{
 /*!Last Changed 22.01.2007*/
 int i;
 double* result;
 result = safemalloc(size * sizeof(double), "log_of_array", 3);
 for (i = 0; i < size; i++)
   result[i] = safelog(list[i]);
 return result;
}

void multiply_array(double* list1, double mul, int size)
{
 /*!Last Changed 13.03.2007 Oya*/
 int i;
 for (i = 0; i < size; i++)
   list1[i] *= mul;
}

double multiply_two_arrays(double* list1, double* list2, int size)
{
 /*!Last Changed 13.03.2007 Oya*/
 int i;
 double result = 0.0;
 for (i = 0; i < size; i++)
   result += list1[i] * list2[i];
 return result;
}

double* multiply_two_arrays_by_value(double* list1, double* list2, int size)
{
 /*!Last Changed 13.03.2007 Oya*/
 int i;
 double* result;
 result = safemalloc(size * sizeof(double), "multiply_two_arrays_by_value", 3);
 for (i = 0; i < size; i++)
   result[i] = list1[i] * list2[i];
 return result;
}

void search_and_add_into_list(int* list, int* itemcount, int newitem)
{
 /*!Last Changed 17.07.2005*/
 int i, j;
 for (i = 0; i < *itemcount; i++)
   if (list[i] > newitem)
     break;
   else
     if (list[i] == newitem)
       return;
 for (j = *itemcount - 1; j >= i; j--)
   list[j + 1] = list[j];
 list[i] = newitem;
 (*itemcount)++;
}

int search_list(int* list, int itemcount, int item)
{
 /*!Last Changed 18.07.2005*/
 int i;
 for (i = 0; i < itemcount; i++)
   if (list[i] == item)
     return i;
 return -1;
}

int sum_of_list(int* list, int size)
{
 /*!Last Changed 28.05.2005*/
 int i, sum = 0;
 for (i = 0; i < size; i++)
   sum += list[i];
 return sum;
}

double euclidian_distance(double* list1, double* list2, int p)
{
 /*!Last Changed 01.10.2010*/
 int i;
 double sum = 0;
 for (i = 0; i < p; i++)
   sum += (list1[i] - list2[i]) * (list1[i] - list2[i]);
 return sqrt(sum);
}

void normalize_array(int* counts, double* posterior, int size)
{
 /*!Last Changed 05.01.2009*/
 int i, sum;
 sum = sum_of_list(counts, size);
 if (sum == 0)
   for (i = 0; i < size; i++)
     posterior[i] = 1.0 / size;
 else
   for (i = 0; i < size; i++)
     posterior[i] = counts[i] / (sum + 0.0);
}

void normalize_array_laplace(int* counts, double* posterior, int size)
{
 int i, sum;
 sum = sum_of_list(counts, size);
 for (i = 0; i < size; i++)
   posterior[i] = (counts[i] + 1) / (sum + size + 0.0);
}

double sum_of_list_double(double* list, int size)
{
 /*!Last Changed 28.05.2005*/
 int i;
 double sum = 0;
 for (i = 0; i < size; i++)
   sum += list[i];
 return sum;
}

void normalize_array_of_doubles(double* list, int size)
{
 /*!Last Changed 09.07.2009*/
 int i, sum;
 sum = sum_of_list_double(list, size);
 if (sum == 0.0)
   for (i = 0; i < size; i++)
     list[i] = 1.0 / size;
 else
   for (i = 0; i < size; i++)
     list[i] = list[i] / (sum + 0.0);
}

double* sum_of_two_arrays(double* list1, double* list2, int size)
{
 /*!Last Changed 22.01.2007*/
 int i;
 double* result;
 result = safemalloc(size * sizeof(double), "sum_of_two_arrays", 3);
 for (i = 0; i < size; i++)
   result[i] = list1[i] + list2[i];
 return result;
}
