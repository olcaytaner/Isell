#include <math.h>
#include <stdlib.h>
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "perceptron.h"

/**
 * Sigmoid function. Given the x calculates \frac{1}{1 + e^{-x}}.
 * @param[in] r x
 * @return Sigmoid value for x.
 */
double sigmoid(double r)
{
 return 1 / (1 + safeexp(-r));
}

/**
 * Usually used to produce probability values from an array of elements. First, we take power of e to the array of elements. Second, each element of the array is divided to the sum of array of elements to normalize (sum of the elements of the array will be 1).
 * @param[out] myarray Array of elements
 * @param[in] size Size of the array
 */
void softmax(double *myarray, int size)
{
 int i;
 double sum = 0.0;
 for (i = 0; i < size; i++)
  {
   myarray[i] = safeexp(myarray[i]);
   sum += myarray[i];
  }
 for (i = 0; i < size; i++)
   myarray[i] /= sum;
}

/**
 * First allocates a two dimensional double array. Second, fills the array with random values between -constant and constant.
 * @param[in] dim1 First dimension of the two dimensional array.
 * @param[in] dim2 Second dimension of the two dimensional array.
 * @param[in] constant Range of the fill.
 * @return Two dimensional array allocated and filled with random values.
 */
double **two_dimensional_initialize(int dim1, int dim2, double constant)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 int i,j;
 double **myarray;
 myarray = (double **)safemalloc_2d(sizeof(double *), sizeof(double), dim1, dim2, "two_dimensional_initialize", 3);
 for (i = 0; i < dim1; i++)
   for (j = 0; j < dim2; j++)
     myarray[i][j] = constant * ((double) produce_random_value(-1, 1));
 return myarray;
}

/**
 * First allocates a one dimensional array. Second, fills the array with random values between -constant and constant.
 * @param[in] dim Size of the array.
 * @param[in] constant Range of the fill.
 * @return One dimensional array allocated and filled with random values.
 */
double *one_dimensional_initialize(int dim, double constant)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 int i;
 double *myarray;
 myarray = safemalloc(dim*sizeof(double), "one_dimensional_initialize", 3);
 for (i = 0; i < dim; i++)
   myarray[i] = constant * ((double) produce_random_value(-1, 1));
 return myarray;
}
