#include <stdlib.h>
#include "combine.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "parameter.h"
#include "typedef.h"

/**
 * Combines learners outputs using simple voting scheme.
 * @param[in] predicted Class predictions of the learners.
 * @param[in] posteriors Estimated posterior probabilities of the learners.
 * @return The class having maximum number of votes
 */
int vote_combine(int* predicted, matrix posteriors)
{
 return find_mod_of_list(predicted, posteriors.row, posteriors.col);
}

/**
 * Combines learners outputs using mean voting scheme.
 * @param[in] predicted Class predictions of the learners.
 * @param[in] posteriors Estimated posterior probabilities of the learners.
 * @return The class having maximum total posterior probability summed over folds
 */
int mean_combine(int* predicted, matrix posteriors)
{
 int i,j;
 int numclass = posteriors.col;
 int numlearner = posteriors.row;
 int res = -1;
 double *classtotal;
 classtotal = (double *) safecalloc(numclass, sizeof(double), "mean_combine", 5);
 for (i = 0 ; i < numclass ; i++)
	 {
   for (j = 0 ; j < numlearner ; j++)
			 {
     classtotal[i] += posteriors.values[j][i];
			 }
	 }
 res = find_max_in_list_double(classtotal, numclass);
 safe_free(classtotal);
 return (res);
}

/**
 * Combines learners outputs using max voting scheme.
 * @param[in] predicted Class predictions of the learners.
 * @param[in] posteriors Estimated posterior probabilities of the learners.
 * @return The class having maximum of maximum fold posterior probabilities
 */
int max_combine(int* predicted, matrix posteriors)
{
 int i,j;
 int numclass = posteriors.col;
 int numlearner = posteriors.row;
 int res = -1;
 double *classmax, max;
 classmax = (double *) safecalloc(numclass, sizeof(double), "max_combine", 5);
 for (i = 0 ; i < numclass ; i++)
	 {
   max = -1.0;
   for (j = 0 ; j < numlearner ; j++)
    {
     if (posteriors.values[j][i] > max)
					 {
	      classmax[i] = posteriors.values[j][i];
					 }	  
			 }
	 }
 res = find_max_in_list_double(classmax, numclass);
 safe_free(classmax);
 return (res);
}

/**
 * Combines learners outputs using min voting scheme.
 * @param[in] predicted Class predictions of the learners.
 * @param[in] posteriors Estimated posterior probabilities of the learners.
 * @return The class having maximum of minimum fold posterior probabilities
 */
int min_combine(int* predicted, matrix posteriors)
{
 int i,j;
 int numclass = posteriors.col;
 int numlearner = posteriors.row;
 int res = -1;
 double *classmin, min;
 classmin = (double *) safecalloc(numclass, sizeof(double), "min_combine", 5);
 for (i = 0 ; i < numclass ; i++)
	 {
   min = 2.0;
   for (j = 0 ; j < numlearner ; j++)
			 {
     if (posteriors.values[j][i] < min)
					 {
	      classmin[i] = posteriors.values[j][i];
					 }
			 }
	 }
 res = find_max_in_list_double(classmin, numclass);
 safe_free(classmin);
 return (res);
}

/**
 * Combines learners outputs using product voting scheme.
 * @param[in] predicted Class predictions of the learners.
 * @param[in] posteriors Estimated posterior probabilities of the learners.
 * @return The class having maximum of product of fold posterior probabilities
 */
int prod_combine(int* predicted, matrix posteriors)
{
 int i,j;
 int numclass = posteriors.col;
 int numlearner = posteriors.row;
 int res = -1;
 double *classprod;
 classprod = (double *) safecalloc(numclass, sizeof(double), "prod_combine", 5);
 for (i = 0 ; i < numclass ; i++)
	 {
   classprod[i] = 1.0;
   for (j = 0 ; j < numlearner ; j++)
    {
     classprod[i] *= posteriors.values[j][i];
			 }
	 }
 res = find_max_in_list_double(classprod, numclass);
 safe_free(classprod);
 return (res);
}

/**
 * Combines learners outputs using median voting scheme.
 * @param[in] predicted Class predictions of the learners.
 * @param[in] posteriors Estimated posterior probabilities of the learners.
 * @return The class having maximum of median of product of fold posterior probabilities
 */
int median_combine(int* predicted, matrix posteriors)
{
 int i,j;
 int numclass = posteriors.col;
 int numlearner = posteriors.row;
 int res = -1;
 double *classmedian;
 double *foldmedian;
 classmedian = (double *) safecalloc(numclass, sizeof(double), "median_combine", 5);
 foldmedian = (double *) safecalloc(numlearner, sizeof(double), "median_combine", 6);
 for (i = 0; i < numclass; i++)
	 { 
   foldmedian[i] = 1.0;
   for (j = 0 ; j < numlearner ; j++)
			 {
     foldmedian[i] *= posteriors.values[j][i];
			 }
   classmedian[i] = find_median_in_list_double(foldmedian, numlearner);
	 }
 res = find_max_in_list_double(classmedian, numclass);
 safe_free(classmedian);
 return (res);
}
