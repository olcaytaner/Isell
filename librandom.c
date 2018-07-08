#include "interval.h"
#include "libarray.h"
#include "libmemory.h"
#include "librandom.h"

int seed = 0;

/**
 * In bootstrapping, one constructs a resample of the dataset (and of equal size to the dataset), which is obtained by random sampling with replacement from the original dataset. This function generates an array of indexes, which contains the indexes of the instances in the original dataset with replacement. That is, the array may contain duplicate elements.
 * @param[in] size Size of the original dataset
 * @return Array of indexes
 */
int* bootstrap_array(int size)
{
 /*!Last Changed 19.02.2005*/
 int i, *result = safemalloc(size * sizeof(int), "bootstrap_array", 1);
 for (i = 0; i < size; i++)
   result[i] = myrand() % size;
 return result;
}

int myrand()
{
 seed = (1664525 * seed + 1013904223) % RANDOM_MAX;
 return seed;
}

void mysrand(unsigned newseed)
{
 seed = newseed;
}

double produce_random_value(double start, double end)
{
 /*!Last Changed 26.01.2005*/
 double part;
 long int x;
 x = myrand() % 10001;
 part = (end - start)/10000.0;
 return start + part * x;
}

int *random_array(int size)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 31.10.2003*/
 int i,tmp,j;
 int *myarray;
 myarray = (int *) safemalloc(size * sizeof(int), "random_array", 3);
 for (i = 0; i < size; i++)
   myarray[i] = i;
 for (i = 0; i < size; i++)
  {
   j = myrand() % size;
   tmp = myarray[i];
   myarray[i] = myarray[j];
   myarray[j] = tmp;
  }
 return myarray;
}

double* random_array_double(int size, double start, double end)
{
 /*!Last Changed 27.03.2006*/
 double* myarray;
 int i;
 myarray = (double *) safemalloc(size * sizeof(double), "random_identity_array", 2);
 for (i = 0; i < size; i++)
   myarray[i] = produce_random_value(start, end);
 return myarray;
}

BOOLEAN random_combination(int* combination, int size, int range)
{
 /*!Last Changed 19.03.2008*/
 int i;
 for (i = 0; i < size; i++)
   combination[i] = myrand() % (range + 1);
 return BOOLEAN_TRUE;
}

double* random_normalized_array(int size)
{
 /*!Last Changed 09.07.2009 added normalize_array_of_doubles*/
 /*!Last Changed 27.03.2006*/
 double *myarray;
 myarray = random_array_double(size, 0.0, 1.0);
 normalize_array_of_doubles(myarray, size);
 return myarray;
}
