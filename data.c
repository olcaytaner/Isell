#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "data.h"
#include "dataset.h"
#include "matrix.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "parameter.h"

extern int mustrealize;
extern Datasetptr current_dataset;

/**
 * Z normalize the actual values of the instances using the mean and variance r' = (r - mu) / sigma.
 * @param myptr Link list of data
 * @param mean Mean value of the real outputs (mu)
 * @param variance Variance of the real outputs (sigma^2)
 */
void normalize_regression_values(Instanceptr myptr, double mean, double variance)
{
 /*!Last Changed 21.12.2005*/
 Instanceptr tmp;
 int classfno;
 NORMALIZATION_TYPE normtype = get_parameter_l("normalization");
 classfno = current_dataset->classdefno;
 tmp = myptr;
 while (tmp)
  {
   switch (normtype)
    {
     case        Z_NORMALIZATION:
     case   VECTOR_NORMALIZATION:tmp->values[classfno].floatvalue = (tmp->values[classfno].floatvalue - mean) / sqrt(variance);
                                 break;
     case STANDARD_NORMALIZATION:tmp->values[classfno].floatvalue = (tmp->values[classfno].floatvalue - current_dataset->features[classfno].min.floatvalue) / (current_dataset->features[classfno].max.floatvalue - current_dataset->features[classfno].min.floatvalue);
                                 break;
    }
   
   tmp = tmp->next;
  }
}

/**
 * Revert the normalization done on a single instance. X' = (X - mu) / sigma => X = X' * sigma + mu. If discrete features are converted to continuous features (mustrealize TRUE) only float values are processed. Otherwise only integer and real features are normalized.
 * @param myptr Instance (X')
 * @param mean Mean vector (mu)
 * @param variance Variance vector (sigma^2)
 */
void unnormalize_instance(Instanceptr myptr, Instance mean, Instance variance)
{
 int i;
 Instanceptr temp;
 temp = myptr;
 if (mustrealize)
  {
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       temp->values[i].floatvalue = (double) (temp->values[i].floatvalue * sqrt(variance.values[i].floatvalue) + mean.values[i].floatvalue);
  }
 else
  {
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:temp->values[i].intvalue = (int) (temp->values[i].floatvalue * sqrt(variance.values[i].floatvalue) + mean.values[i].floatvalue);
                    break;
       case    REAL:temp->values[i].floatvalue = (double) (temp->values[i].floatvalue * sqrt(variance.values[i].floatvalue) + mean.values[i].floatvalue);
                    break;
      }
  }
}

/**
 * Normalize single instance. X' = (X - mu) / sigma. If discrete features are converted to continuous features (mustrealize TRUE) only float values are processed. Otherwise only integer and real features are normalized. For both, the result is stored in the float part of the union.
 * @param myptr Instance (X)
 * @param mean Mean vector (mu)
 * @param variance Variance vector (sigma)
 */
void normalize_instance(Instanceptr myptr, Instance mean, Instance variance)
{
 int i;
 double sum;
 Instanceptr temp;
 NORMALIZATION_TYPE normtype = get_parameter_l("normalization"); 
 temp = myptr;
 if (mustrealize)
  {
   if (normtype == VECTOR_NORMALIZATION)
    {
     sum = 0.0;
     for (i = 0; i < current_dataset->multifeaturecount; i++)
       if (i != current_dataset->classdefno)
         sum += temp->values[i].floatvalue * temp->values[i].floatvalue;
    }
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       switch (normtype)
        {
         case        Z_NORMALIZATION:if (variance.values[i].floatvalue != 0)
                                       temp->values[i].floatvalue = (double) ((temp->values[i].floatvalue - mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue));
                                     break;
         case STANDARD_NORMALIZATION:temp->values[i].floatvalue = (temp->values[i].floatvalue - current_dataset->features[i].min.floatvalue) / (current_dataset->features[i].max.floatvalue - current_dataset->features[i].min.floatvalue);
                                     break;
         case   VECTOR_NORMALIZATION:if (sum != 0)
                                       temp->values[i].floatvalue = temp->values[i].floatvalue / sqrt(sum);
                                     break;
        } 
  }
 else
  {
   if (normtype == VECTOR_NORMALIZATION)
    {
     sum = 0.0;
     for (i = 0; i < current_dataset->featurecount; i++)
       switch (current_dataset->features[i].type)
        {     
         case INTEGER:sum += temp->values[i].intvalue * temp->values[i].intvalue;
                      break;
         case    REAL:sum += temp->values[i].floatvalue * temp->values[i].floatvalue;
                      break;
        }     
    }
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:switch (normtype)
                     {
                      case        Z_NORMALIZATION:if (variance.values[i].floatvalue != 0)
                                                    temp->values[i].floatvalue = (double) ((temp->values[i].intvalue - mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue));
                                                  else
                                                    temp->values[i].floatvalue = (double) (temp->values[i].intvalue);
                                                  break;
                      case STANDARD_NORMALIZATION:temp->values[i].floatvalue = (temp->values[i].intvalue - current_dataset->features[i].min.intvalue) / (current_dataset->features[i].max.intvalue - current_dataset->features[i].min.intvalue + 0.0);
                                                  break;
                      case   VECTOR_NORMALIZATION:if (sum != 0)
                                                    temp->values[i].floatvalue = temp->values[i].intvalue / sqrt(sum);
                                                  break;
                     }
                    break;
       case    REAL:switch (normtype)
                     {
                      case        Z_NORMALIZATION:if (variance.values[i].floatvalue != 0)
                                                    temp->values[i].floatvalue = (double) ((temp->values[i].floatvalue-mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue));
                                                  break;
                      case STANDARD_NORMALIZATION:temp->values[i].floatvalue = (temp->values[i].floatvalue - current_dataset->features[i].min.floatvalue) / (current_dataset->features[i].max.floatvalue - current_dataset->features[i].min.floatvalue);
                                                  break;
                      case   VECTOR_NORMALIZATION:if (sum != 0)
                                                    temp->values[i].floatvalue = temp->values[i].floatvalue / sqrt(sum);
                                                  break;
                     }
                    break;
      }
  }
}

/**
 * Calls normalize_instance to normalize all instances in the data sample.
 * @param myptr Data sample 
 * @param mean Mean vector
 * @param variance Variance vector
 */
void normalize(Instanceptr myptr, Instance mean, Instance variance)
{
 /*!Last Changed 20.03.2002*/
 Instanceptr temp;
 temp = myptr;
 while (temp)
  {
   normalize_instance(temp, mean, variance);
   temp = temp->next;
  }
}

/**
 * Discrete to continuous conversion (using 1-of-L encoding) of a single instance. Since the number of features is increased due to this conversion, values array are reallocated to have multifeaturecount features. Integer features are also converted to floating point features.
 * @param myptr Instance
 */
void realize_instance(Instanceptr myptr)
{
 Instanceptr tmp = myptr;
 int i, j, k;
 i = current_dataset->featurecount - 1;
 j = current_dataset->multifeaturecount - 1;
 if (i != j)
  tmp->values = (union value *) alloc(tmp->values, current_dataset->multifeaturecount * sizeof(union value), current_dataset->multifeaturecount);
 while (i >= 0)
  {
   switch (current_dataset->features[i].type)
    {
     case INTEGER  :tmp->values[j].floatvalue = (double) tmp->values[i].intvalue;
                    j--;
                    break;
     case REGDEF   :
     case REAL     :tmp->values[j].floatvalue = tmp->values[i].floatvalue;
                    j--;
                    break;
     case CLASSDEF :tmp->values[j].stringindex = tmp->values[i].stringindex;
                    j--;
                    break;
     case STRING   :for (k = 0; k < current_dataset->features[i].valuecount; k++)
                      if (k == tmp->values[i].stringindex)
                        tmp->values[j-k].floatvalue = 1;
                      else
                        tmp->values[j-k].floatvalue = 0;
                    j -= current_dataset->features[i].valuecount;
                    break;
     default       :printf(m1238, current_dataset->features[i].type);
                    break;
    }
   i--;
  }
}

/**
 * Calls realize_instance for discrete to continuous conversion of all instances in the data sample.
 * @param myptr Data sample 
 */
void realize(Instanceptr myptr)
{
 /*!Last Changed 10.03.2004 extended to regression*/
 Instanceptr tmp = myptr;
 while(tmp)
  {
   realize_instance(tmp);
   tmp = tmp->next;
  }
}

/**
 * Given the eigenvectors of the covariance matrix, this function reduce the dimension of a data sample from d to k.
 * @param inst Data sample
 * @param howmany k
 * @param eigenvect Eigenvectors of the covariance matrix. Eigenvectors correpond to columns of the matrix. 0'th column corresponds to the eigenvector with the largest eigenvalue, 1'the column corresponds to the eigenvector with the second largest eigenvalue etc.
 * @pre Function assumes that, discrete-to-continuous conversion is done.
 */
void dimension_reduce_with_pca(Instanceptr inst, int howmany, matrix eigenvect)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 Instanceptr tmp = inst;
 int i, j, k;
 double* values, sum;
 values = (double *)safemalloc((current_dataset->multifeaturecount - 1) * sizeof(double), "dimension_reduce_with_pca", 4);
 while (tmp)
  {
   for (i = 0; i < howmany; i++)
    {
     sum = 0;
     for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
       sum += eigenvect.values[j][i] * real_feature(tmp, j);
     values[i] = sum;
    }
   k = 0;
   for (j = 0; j < current_dataset->multifeaturecount - 1 && k < howmany; j++)
     if (in_list(current_dataset->features[j].type, 2, REAL, INTEGER))
      {
       tmp->values[j].floatvalue = (double) values[k];
       k++;
      }
   tmp = tmp->next;
  }
 safe_free(values);
}

/**
 * Creates N by N distance matrix between instances of a data sample. Each distance is calculated using distance_between_instances function.
 * @param myptr Data sample
 * @return Distance matrix
 */
matrix distance_matrix(Instanceptr myptr)
{
 /*!Last Changed 17.01.2008*/
 int i, j, size;
 Instanceptr temp1, temp2;
 matrix result;
 size = data_size(myptr);
 result = matrix_alloc(size, size);
 for (i = 0, temp1 = myptr; i < size; i++, temp1 = temp1->next)
   for (j = 0, temp2 = myptr; j < size; j++, temp2 = temp2->next)
     result.values[i][j] = distance_between_instances(temp1, temp2);
 return result;
}

/**
 * Creates N by N distance matrix between instances of a data sample. Each distance is calculated using distance_between_instances function. If the distance is above a certain threshold, the distance is converted to infinity (+LONG_MAX).
 * @param myptr Data sample
 * @param epsilon Threshold
 * @return Distance matrix
 */
matrix distance_matrix_with_threshold(Instanceptr myptr, double epsilon)
{
 /*!Last Changed 17.01.2008*/
 int i, j, size;
 Instanceptr temp1, temp2;
 matrix result;
 double distance;
 size = data_size(myptr);
 result = matrix_alloc(size, size);
 for (i = 0, temp1 = myptr; i < size; i++, temp1 = temp1->next)
   for (j = 0, temp2 = myptr; j < size; j++, temp2 = temp2->next)    
    {
     distance = distance_between_instances(temp1, temp2);
     if (distance < epsilon)
       result.values[i][j] = distance;
     else
       result.values[i][j] = +LONG_MAX;
    }
 return result;
}

/**
 * Creates N by N distance matrix between instances of a data sample. The distances are calculated via distance_between_instances function. The distances between instance X and the nearest k neighbors of it are calculated. For other instances the distance from the instance X is infinity (+LONG_MAX). There is also a postprocessing step. To make the distance matrix symmetric (distance between X and Y must equal to the distance between Y and X), the minimum of those two distances are taken.
 * @param myptr Data sample
 * @param k Number of neighbors
 * @warning k neighbor also include the instance itself.
 * @return Distance matrix
 */
matrix distance_matrix_with_k(Instanceptr myptr, int k)
{
 /*!Last Changed 31.03.2008*/
 int i, j, size;
 Instanceptr temp1, temp2;
 matrix result;
 double distance, threshold;
 size = data_size(myptr);
 result = matrix_alloc(size, size);
 for (i = 0, temp1 = myptr; i < size; i++, temp1 = temp1->next)
  {
   threshold = find_k_nearest_neighbor_distance(myptr, temp1, k);
   for (j = 0, temp2 = myptr; j < size; j++, temp2 = temp2->next)    
    {
     distance = distance_between_instances(temp1, temp2);
     if (distance < threshold + DBL_DELTA)
       result.values[i][j] = distance;
     else
       result.values[i][j] = +LONG_MAX;
    }
  }
 /*Make distance matrix symmetric*/
 for (i = 0; i < size; i++)
   for (j = i + 1; j < size; j++)
     if (result.values[i][j] < result.values[j][i])
       result.values[j][i] = result.values[i][j];
     else
       result.values[i][j] = result.values[j][i];     
 return result;
}

/**
 * Covariance matrix for a specific class is calculated.
 * @param myptr Data sample (including elements from current class and other classes)
 * @param classno The class
 * @param classmean Mean vector of the class
 * @pre Data must have been discrete-to-continuous converted
 * @return Covariance matrix
 */
matrix class_covariance(Instanceptr myptr, int classno, Instanceptr classmean)
{
 /*!Last Changed 26.03.2002*/
 int i,j,size = 0;
 matrix temp;
 Instanceptr current = myptr;
 temp = matrix_alloc(current_dataset->multifeaturecount - 1, current_dataset->multifeaturecount - 1);
 while (current)
  {
   if (give_classno(current) == classno)
    {
     for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
       for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
         temp.values[i][j] += (real_feature(current, i) - real_feature(classmean, i)) * (real_feature(current, j) - real_feature(classmean, j));
     size++;
    }
   current = current->next;
  }
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
   for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
     if (size > 1)
       temp.values[i][j] /= (size - 1);
     else
       temp.values[i][j] = 0;
 return temp;
}

/**
 * The boundary values of the feature fno are calculated (minimum and maximum values of the feature fno for the current data sample).
 * @param d Dataset
 * @param myptr Data sample
 * @param fno Feature for which the boundary values will be calculated
 * @param min Minimum value of the feature (output)
 * @param max Maximum value of the feature (output)
 */
void find_bounds_of_feature(Datasetptr d, Instanceptr myptr, int fno, double* min, double* max)
{
 /*!Last Changed 16.09.2004*/
 Instanceptr tmp = myptr;
 double val;
 if (tmp)
  {
   if (d->features[fno].type == INTEGER)
     val = (double)tmp->values[fno].intvalue;
   else
     val = tmp->values[fno].floatvalue;
  }
 else
   val = 0;
 *min = val;
 *max = val;
 while (tmp)
  { 
   if (d->features[fno].type == INTEGER)
     val = (double)tmp->values[fno].intvalue;
   else
     val = tmp->values[fno].floatvalue;
   if (val > *max)
     *max = val;
   else
     if (val < *min)
       *min = val;
   tmp = tmp->next;
  }
}

/**
 * The boundary values of the feature fno are calculated for the instances with class cno (minimum and maximum values of the feature fno for the instances of the current data sample having class cno).
 * @param myptr Data sample
 * @param fno Feature for which the boundary values will be calculated
 * @param cno The class index
 * @param min Minimum value of the feature (output)
 * @param max Maximum value of the feature (output)
 */
void find_bounds_of_feature_class(Instanceptr myptr, int fno, int cno, double* min, double* max)
{
 /*oya -- Last Changed 21.4.2006*/
 Instanceptr tmp = myptr;
 double val;
 int i;
 if (tmp)
   val = tmp->values[fno].floatvalue;
 else
   val = 0;
 *min = val;
 *max = val;
 while (tmp)
  { 
   i = give_classno(tmp);
   if (i == cno)
    {
     val = tmp->values[fno].floatvalue;
     if (val > *max)
       *max = val;
     else
       if (val < *min)
         *min = val;
    }
   tmp = tmp->next;
  }
}

/**
 * Finds the number of instances of the class classno having feature value (for feature fno) smaller than fval (if dir = LOWER), larger than fval (if dir = UPPER).
 * @param myptr Data sample
 * @param classno The class index
 * @param fno The feature index
 * @param fval The threshold
 * @param dir Direction dir:LOWER val must be less than fval, dir:UPPER val must be greater than fval
 * @return Number of instances
 */
int find_thresholded_class_count(Instanceptr myptr, int classno, int fno, double fval, int dir)
{
 /*oya -- Last Changed 21.04.2006*/
 /*
 dir:LOWER val must be less than fval
 dir:UPPER val must be greater than fval
 */
 int result = 0;
 Instanceptr temp = myptr;
 double val;
 while (temp)
  {
   if (give_classno(temp) == classno)
    {
     val = temp->values[fno].floatvalue;
     if ((dir == LOWER) && (val < fval))
       result++;
     else 
       if ((dir == UPPER) && (val > fval))
         result++;
     }
   temp = temp->next;
  }
 return result;
}

/**
 * Generates covariance matrix as if the data is quadratic (x_1^2 + x_1x_2 + ... + x_1x_d + x_2^2 + x_2x_3 + ... + x_2x_d + ... + x_(d-1)x_d + x_d^2 + x_1 + x_2 + ... + x_d). The quadratic data are calculated ad hoc.
 * @param myptr Data sample
 * @param currentmean Mean vector (stored as a matrix with one row)
 * @pre Data must have been discrete-to-continuous converted.
 * @return Covariance matrix
 */
matrix covariance_2d(Instanceptr myptr, matrix currentmean)
{
 /*!Last Changed 30.01.2005*/
 matrix result;
 Instanceptr tmp;
 int dim, matrixdim, i, j, k, m, size = 0, x, y;
 double fx, fy, meanx, meany;
 dim  = current_dataset->multifeaturecount - 1;
 matrixdim = multifeaturecount_2d(current_dataset);
 result = matrix_alloc(matrixdim, matrixdim);
 tmp = myptr;
 while (tmp)
  {
   x = 0;
   /*For dimensions x_ix_j*/
   for (i = 0; i < dim; i++)
     for (j = i; j < dim; j++)
      {
       y = 0;
       fx = real_feature(tmp, i) * real_feature(tmp, j);
       meanx = currentmean.values[0][x];
       /*with dimensions x_kx_m*/
       for (k = 0; k < dim; k++)
         for (m = k; m < dim; m++)
          {           
           fy = real_feature(tmp, k) * real_feature(tmp, m);
           meany = currentmean.values[0][y];
           result.values[x][y] += (fx - meanx) * (fy - meany);
           y++;
          }
       /*with dimensions x_k*/
       for (k = 0; k < dim; k++)
        {
         fy = real_feature(tmp, k);
         meany = currentmean.values[0][y];
         result.values[x][y] += (fx - meanx) * (fy - meany);
         y++;
        }
       x++;
      }
   /*For dimensions x_i*/
   for (i = 0; i < dim; i++)
    {
     y = 0;
     fx = real_feature(tmp, i);
     meanx = currentmean.values[0][x];
     /*with dimensions x_kx_m*/
     for (k = 0; k < dim; k++)
       for (m = k; m < dim; m++)
        {           
         fy = real_feature(tmp, k) * real_feature(tmp, m);
         meany = currentmean.values[0][y];
         result.values[x][y] += (fx - meanx) * (fy - meany);
         y++;
        }
     /*with dimensions x_k*/
     for (k = 0; k < dim; k++)
      {
       fy = real_feature(tmp, k);
       meany = currentmean.values[0][y];
       result.values[x][y] += (fx - meanx) * (fy - meany);
       y++;
      }
     x++;
    }
   tmp = tmp->next;
   size++;
  }
 for (i = 0; i < matrixdim; i++)
   for (j = 0; j < matrixdim; j++)
     result.values[i][j] /= (size - 1);
 return result;
}

/**
 * Calculates the covariance matrix of a data sample
 * @param myptr Data sample
 * @param currentmean Sample mean vector
 * @pre Data must have been discrete-to-continuous converted.
 * @return Covariance matrix
 */
matrix covariance(Instanceptr myptr, Instance currentmean)
{
 int i, j, size = data_size(myptr);
 matrix temp;
 Instanceptr current = myptr;
 temp = matrix_alloc(current_dataset->multifeaturecount - 1, current_dataset->multifeaturecount - 1);
 while (current)
  {
   for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
     for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
       temp.values[i][j] += (real_feature(current, i) - real_feature(&currentmean, i)) * (real_feature(current, j) - real_feature(&currentmean, j));
   current = current->next;
  }
 for (i = 0; i < current_dataset->multifeaturecount -1 ; i++)
   for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
     temp.values[i][j] /= (size - 1);
 return temp;
}

/**
 * Finds number of instances with the class classno 
 * @param myptr Data sample
 * @param classno Class index
 * @return Number of instances
 */
int find_class_count(Instanceptr myptr, int classno)
{
 /*!Last Changed 03.12.2003*/
 int result = 0;
 Instanceptr temp = myptr;
 while (temp)
  {
   if (give_classno(temp) == classno)
     result++;
   temp = temp->next;
  }
 return result;
}

/**
 * Checks if there is at least one instance with class classno in the data sample
 * @param myptr Data sample
 * @param classno Class index
 * @return 1 if there is at least one instance with class classno in the data sample, otherwise returns 0
 */
BOOLEAN class_exists_in_list(Instanceptr myptr, int classno)
{
 /*!Last Changed 09.05.2003*/
 Instanceptr temp = myptr;
 while (temp)
  {
   if (give_classno(temp) == classno)
     return BOOLEAN_TRUE;
   temp = temp->next;
  }
 return BOOLEAN_FALSE;
}

/**
 * Finds the error count as if the data sample is in a leaf node. Error = \sum_i N_i - N_max, where N_max is the number of instances in the class having maximum number of instances.
 * @param myptr Data sample
 * @return Error count
 */
int find_error_as_leaf(Instanceptr myptr)
{
 /*!Last Changed 23.05.2006*/
 int i, *result, maxclass = 0, sum = 0;
 result = find_class_counts(myptr);
 for (i = 0; i < current_dataset->classno; i++)
  {
   sum += result[i];
   if (result[i] > maxclass)
     maxclass = result[i];
  } 
 safe_free(result);
 return sum - maxclass;
}

/**
 * Does two jobs together. First finds the class that occurs maximum. Second checks if there is only single class in the sample.
 * @param myptr Data sample 
 * @param maxcount Number of instances for the class having maximum number of instances (output)
 * @return Last digit is 0 if there is only single class in the sample, 1 otherwise. The index of the class that occurs maximum can be found by dividing the return value to 10.
 */
int all_in_one_class(Instanceptr myptr, int* maxcount)
{
 /*!Last Changed 01.11.2005*/
 /*!Last Changed 23.03.2002*/
 int i, *result, maxindex = -1, maxclass = 0, alone, sum = 0;
 result = find_class_counts(myptr);
 for (i = 0; i < current_dataset->classno; i++)
  {
   sum += result[i];
   if (result[i] > maxclass)
    {
     maxclass = result[i];
     maxindex = i;
    }
  }
 safe_free(result);
 if (sum != maxclass)
   alone = 1;
 else
   alone = 0;
 *maxcount = maxclass;
 if (maxindex == -1)
   return 0;
 else
   return maxindex * 10 + alone;
}

/**
 * Generates an array of indexes, where each element refers to the class index of the corresponding instance.
 * @param myptr Data sample
 * @return Array of class indexes.
 */
int* find_classes(Instanceptr myptr)
{
 /*!Last Changed 28.10.2005*/
 int i = 0;
 Instanceptr temp = myptr;
 int *result;
 result = (int *) safemalloc(data_size(myptr) * sizeof(int), "find_classes", 4);
 while (temp)
  {
   result[i] = give_classno(temp);
   i++;
   temp = temp->next;
  }
 return result;
}

/**
 * Generates an array of class counts, where each element refers to the number of instances of the corresponding class.
 * @param myptr Data sample
 * @return Array of class counts.
 */
int* find_class_counts(Instanceptr myptr)
{
 /*!Last Changed 02.02.2004 added safealloc*/
 /*!Last Changed 16.03.2002*/
 int i;
 Instanceptr temp = myptr;
 int *result;
 result = (int *) safecalloc(current_dataset->classno, sizeof(int), "find_class_counts", 4);
 while (temp)
  {
   i = give_classno(temp);
   result[i]++;
   temp = temp->next;
  }
 return result;
}

/**
 * Generates a link list of instances (number of instances is count), where each instance has fcount number of features.
 * @param fcount The number of features
 * @param count The number of instance
 * @return Link list of instances
 */
Instanceptr empty_link_list_of_instances(int fcount, int count)
{
 /*!Last Changed 26.11.2005*/
 /*!Last Changed 24.03.2005*/
 /*!Last Changed 22.03.2005*/
 int i;
 Instanceptr result, tmpbefore, tmp;
 result = empty_instance(fcount);
 tmpbefore = result;
 for (i = 1; i < count; i++)
  {
   tmp = empty_instance(fcount);
   tmpbefore->next = tmp;
   tmpbefore = tmp;
  }
 return result;
}

/**
 * Creates an empty three dimensional array.\n
 * First dimension: Number of classes (fixed) \n
 * Second dimension: Number of features (fixed) \n
 * Third dimension: Number of different values for each feature (dynamic) \n
 * The array will store the number of possible values of each feature value for each class.
 * @return Empty array.
 */
int*** allocate_feature_counts()
{
 /*!Last Changed 11.08.2007*/
 int i, j;
 int*** feature_counts;
 feature_counts = (int ***)safemalloc(current_dataset->classno * sizeof(int **), "allocate_feature_counts", 3);
 for (i = 0;i < current_dataset->classno; i++)
   {
    feature_counts[i] = (int **)safemalloc(current_dataset->featurecount * sizeof(int *), "allocate_feature_counts", 6);
    for (j = 0;j < current_dataset->featurecount; j++)
      if (current_dataset->features[j].type == STRING)
        feature_counts[i][j] = (int *)safecalloc(current_dataset->features[j].valuecount, sizeof(int), "allocate_feature_counts", 9);
   }
 return feature_counts;
}

/**
 * Frees the three dimensional array allocated in the function allocate_feature_counts.
 * @pre The array must have been allocated using allocate_feature_counts.
 * @param feature_counts The array
 */
void free_feature_counts(int*** feature_counts)
{
 /*!Last Changed 11.08.2007*/
 int i, j;
 for (i = 0; i < current_dataset->classno; i++)
  {
   for (j = 0; j < current_dataset->featurecount; j++)
     if (current_dataset->features[j].type == STRING)
       safe_free(feature_counts[i][j]);
   safe_free(feature_counts[i]);
  }
 safe_free(feature_counts);
}

/**
 * The function will fill the three dimensional array allocated in the function allocate_feature_counts. The array will store the number of possible values of each feature value for each class. \n
 * array[i][j][k]: The number of instances where the instance with class i has the string value of the feature j has the index k.
 * @param myptr Data sample
 * @return Feature count array
 */
int*** find_feature_counts(Instanceptr myptr)
{
 /*!Last Changed 11.08.2007*/
 int i, j;
 Instanceptr tmp;
 int*** feature_counts = allocate_feature_counts();
 tmp = myptr;
 while (tmp)
  {
   i = give_classno(tmp);
   for (j = 0; j < current_dataset->featurecount; j++)
     if (current_dataset->features[j].type == STRING)
       feature_counts[i][j][tmp->values[j].stringindex]++;
   tmp = tmp->next;
  }
 return feature_counts;
}

/**
 * Finds the mean vector of each class. (i) If discrete to continuous conversion has been done, the mean vector has multifeaturecount number of features. (ii) If discrete to continuoud conversion has not been done, the mean vector has featurecount number of features. The mean value of a discrete feature is the value which occurs maximum number of times.
 * @param myptr Data sample
 * @param counts Number of instances in each class (stored as an array). \n
 * counts[i]: Number of instances in the class i.
 * @return Link list of class mean vectors.
 */
Instanceptr find_class_means(Instanceptr myptr,int *counts)
{
 /*!Last Changed 11.08.2007 added allocate_feature_counts and free_feature_counts*/
 /*!Last Changed 22.03.2005 added empty_instance*/
 /*!Last Changed 02.02.2004 added safemalloc and safecalloc*/
 /*!Last Changed 20.03.2002*/
 int i,j;
 int*** feature_counts = NULL;
 Instanceptr temp, temp1, result;
 if (!mustrealize)
   result = empty_link_list_of_instances(current_dataset->featurecount, current_dataset->classno);
 else
   result = empty_link_list_of_instances(current_dataset->multifeaturecount, current_dataset->classno);
 if (!mustrealize)
   feature_counts = allocate_feature_counts();
 temp1 = result;
 while (temp1)/*Initializing class means*/
  {
   if (mustrealize)
    {
     for (i = 0; i < current_dataset->multifeaturecount; i++)
       if (i != current_dataset->classdefno)
         temp1->values[i].floatvalue = 0;
    }
   else
    {
     for (i = 0; i < current_dataset->featurecount;i++)
       switch (current_dataset->features[i].type)
        {
         case INTEGER:temp1->values[i].floatvalue = 0;
                      break;
         case    REAL:temp1->values[i].floatvalue = 0;
                      break;
        }
    }
   temp1 = temp1->next;
  }
 temp = myptr;
 while (temp)/*Adding values to total*/
  {
   i = give_classno(temp);
   temp1 = data_x(result, i);
   if (mustrealize)
    {
     for (i = 0; i < current_dataset->multifeaturecount; i++)
       if (i != current_dataset->classdefno)
         temp1->values[i].floatvalue += temp->values[i].floatvalue;
    }
   else
    {
     for (j = 0; j < current_dataset->featurecount; j++)
       switch (current_dataset->features[j].type)
        {
         case INTEGER:temp1->values[j].floatvalue += temp->values[j].intvalue;
                      break;
         case    REAL:temp1->values[j].floatvalue += temp->values[j].floatvalue;
                      break;
         case  STRING:feature_counts[i][j][temp->values[j].stringindex]++;
                      break;
        }
    }
   temp = temp->next;
  }
 temp1 = result;
 j = 0;
 while (temp1)
  {
   if (mustrealize)
    {
     for (i = 0; i < current_dataset->multifeaturecount; i++)
       if (i != current_dataset->classdefno)
        {
         if (counts[j] != 0)
           temp1->values[i].floatvalue /= counts[j];
         else
           temp1->values[i].floatvalue = 0;         
        }
    }
   else
    {
     for (i = 0; i < current_dataset->featurecount; i++)
       switch (current_dataset->features[i].type)
        {
         case INTEGER:if (counts[j] != 0)
                        temp1->values[i].floatvalue /= counts[j];
                      else
                        temp1->values[i].floatvalue = 0;
                      break;
         case    REAL:if (counts[j]!=0)
                        temp1->values[i].floatvalue /= counts[j];
                      else
                        temp1->values[i].floatvalue = 0;
                      break;
         case  STRING:temp1->values[i].stringindex = find_max_in_list(feature_counts[j][i], current_dataset->features[i].valuecount);
                      break;
        }
    }
   temp1 = temp1->next;
   j++;
  }
 if (!mustrealize)
   free_feature_counts(feature_counts);
 return result;
}

/**
 * Finds the variance of each class. 
 * @param myptr Data sample
 * @param classmeans The mean vector for each class (Stored as a link list of instances).
 * @param counts The number of instances in each class (Stored as an array).
 * @warning If counts[i] == 1 or 0, variance will be 0.
 * @return Class variances.
 */
double* find_class_variance(Instanceptr myptr, Instanceptr classmeans, int *counts)
{
 /*!Last Changed 11.07.2006*/
 /*!Last Changed 19.12.2005*/
 int i, j;
 double* result;
 Instanceptr temp, m_i;
 result = safecalloc(current_dataset->classno, sizeof(double), "find_class_variance", 3);
 temp = myptr;
 while (temp)/*Adding values to total*/
  {
   i = give_classno(temp);
   m_i = data_x(classmeans, i);
   for (j = 0; j < current_dataset->multifeaturecount; j++)
     if (j != current_dataset->classdefno)
       result[i] += (temp->values[j].floatvalue - m_i->values[j].floatvalue) * (temp->values[j].floatvalue - m_i->values[j].floatvalue);
   temp = temp->next;
  }
 for (i = 0; i < current_dataset->classno; i++)
   if (counts[i] > 1)
     result[i] /= (counts[i] - 1);
   else
     result[i] = 0.0;
 return result;
}

/**
 * Finds within class scatter matrix S_w. S_w = \sum_{i = 1}^K S_i, where S_i is the covariance matrix for class i.
 * @param myptr Data sample.
 * @return S_w (within class scatter matrix).
 */
matrix within_class_matrix(Instanceptr myptr)
{
 /*!Last Changed 26.01.2002*/
 int i,j,k;
 Instanceptr temp;
 int *c_counts;
 Instanceptr c_means; 
 matrix result, current_matrix;
 temp = myptr;
 result = matrix_alloc(current_dataset->multifeaturecount - 1, current_dataset->multifeaturecount - 1);
 current_matrix = matrix_alloc(current_dataset->multifeaturecount - 1, current_dataset->multifeaturecount - 1);
 c_counts = find_class_counts(temp);
 c_means = find_class_means(temp, c_counts);
 for (i = 0; i < current_dataset->classno; i++)
  {
   temp = myptr;
   while (temp)
    {
     if (give_classno(temp) == i)
       for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
         for (k = 0; k < current_dataset->multifeaturecount - 1; k++)
           current_matrix.values[j][k] += (real_feature(temp, j) - real_feature(data_x(c_means, i), j)) * (real_feature(temp, k) - real_feature(data_x(c_means, i), k));
     temp = temp->next;
    }
   matrix_add(&result, current_matrix);
   for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
     for (k = 0; k < current_dataset->multifeaturecount - 1; k++)
       current_matrix.values[j][k] = 0;
  }
 matrix_free(current_matrix);
 safe_free(c_counts);
 deallocate(c_means);
 return result;
}

/**
 * Finds the mean vector of the data sample. The data sample may contain missing values. Missing values are not added to the sum. The mean value of a discrete feature is the value which occurs maximum number of times.
 * @param myptr Data sample.
 * @warning Missing values must have the value ISELL.
 * @return The mean vector.
 */
Instance find_mean_with_missing(Instanceptr myptr)
{
 /*!Last Changed 02.02.2004 Added safemalloc and safecalloc*/
 int i, *counts;
 Instanceptr temp;
 Instance result;
 int** featurevalues;
 featurevalues = (int **)safemalloc(current_dataset->featurecount * sizeof(int*), "find_mean_with_missing", 5);
 counts = (int *)safecalloc(current_dataset->featurecount, sizeof(int), "find_mean_with_missing", 6);
 temp = myptr;
 result.values = (union value *)safemalloc(current_dataset->featurecount * sizeof(union value), "find_mean_with_missing", 7);
 for (i = 0; i < current_dataset->featurecount; i++)
   switch (current_dataset->features[i].type)
    {
     case INTEGER:result.values[i].intvalue = 0;
                  break;
     case    REAL:result.values[i].floatvalue = 0;
                  break;
     case  STRING:featurevalues[i] = (int *)safecalloc(current_dataset->features[i].valuecount, sizeof(int), "find_mean_with_missing", 15);
                  break;
    }
 while (temp)
  {
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:if (temp->values[i].intvalue != ISELL)
                     {
                      result.values[i].intvalue += temp->values[i].intvalue;
                      counts[i]++;
                     }
                    break;
       case    REAL:if (temp->values[i].floatvalue != ISELL)
                     {
                      result.values[i].floatvalue += temp->values[i].floatvalue;
                      counts[i]++;
                     }
                    break;
       case  STRING:if (temp->values[i].stringindex != -1)
                      featurevalues[i][temp->values[i].stringindex]++;
                    break;
      }
   temp = temp->next;
  }
 for (i = 0; i < current_dataset->featurecount; i++)
   switch (current_dataset->features[i].type)
    {
     case INTEGER:if (counts[i] != 0)
                    result.values[i].intvalue /= counts[i];
                  break;
     case    REAL:if (counts[i] != 0)
                    result.values[i].floatvalue /= counts[i];
                  break;
     case  STRING:result.values[i].stringindex = find_max_in_list(featurevalues[i], current_dataset->features[i].valuecount);
                  break;
    }
 for (i = 0; i < current_dataset->featurecount; i++)
   if (current_dataset->features[i].type == STRING)
     safe_free(featurevalues[i]);
 safe_free(featurevalues);
 safe_free(counts);
 return result;
}

/**
 * Generates the mean vector matrix as if the data is quadratic (x_1^2 + x_1x_2 + ... + x_1x_d + x_2^2 + x_2x_3 + ... + x_2x_d + ... + x_(d-1)x_d + x_d^2 + x_1 + x_2 + ... + x_d). The quadratic data are calculated ad hoc. The mean vector is stored as the first row.
 * @param myptr Data sample.
 * @return The matrix where the first row contains the mean vector
 */
matrix find_mean_2d(Instanceptr myptr)
{
 /*!Last Changed 30.01.2005*/
 matrix result;
 Instanceptr tmp;
 int i, j, count, dim, matrixdim, x;
 dim = current_dataset->multifeaturecount - 1;
 matrixdim = multifeaturecount_2d(current_dataset);
 result = matrix_alloc(1, matrixdim);
 tmp = myptr;
 count = 0;
 while (tmp)
  {
   x = 0;
   for (i = 0; i < dim; i++)
     for (j = i; j < dim; j++)
      {
       result.values[0][x] += real_feature(tmp, i) * real_feature(tmp, j);
       x++;
      }
   for (i = 0; i < dim; i++)
    {
     result.values[0][x] += real_feature(tmp, i);
     x++;
    }
   count++;
   tmp = tmp->next;
  }
 for (i = 0; i < matrixdim; i++)
   result.values[0][i] /= count;
 return result;
}

Instanceptr find_linear_interpolation_data(Instanceptr data1, Instanceptr data2, double param)
{
 /*!Last Changed 01.05.2006 currently for classification*/
 int i;
 Instanceptr result;
 if (mustrealize)
  {
   result = empty_instance(current_dataset->multifeaturecount);
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       result->values[i].floatvalue = data1->values[i].floatvalue * param + data2->values[i].floatvalue * (1 - param);
   result->values[current_dataset->classdefno].stringindex = data1->values[current_dataset->classdefno].stringindex;
  }
 else
  {
   result = empty_instance(current_dataset->featurecount);
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case  INTEGER:result->values[i].intvalue = (int) (data1->values[i].intvalue * param + data2->values[i].intvalue * (1 - param));
                     break;
       case   REGDEF:
       case     REAL:result->values[i].floatvalue = data1->values[i].floatvalue * param + data2->values[i].floatvalue * (1 - param);
                     break;   
       case CLASSDEF:
       case   STRING:result->values[i].stringindex = data1->values[i].stringindex;
                     break;
      }
  }
 return result;
}

Instance find_mean(Instanceptr myptr)
{
 /*!Last Changed 02.02.2004 added safemalloc and safecalloc*/
 /*!Last Changed 19.03.2002*/
 int i,count=0;
 Instanceptr temp=myptr;
 int** feature_counts;
 Instance result;
 if (mustrealize)
  {
   result.values = (union value *)safemalloc(current_dataset->multifeaturecount*sizeof(union value), "find_mean", 7);
   for (i = 0;i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       result.values[i].floatvalue = 0;
   while (temp)
    {
     for (i = 0;i < current_dataset->multifeaturecount; i++)
       if (i != current_dataset->classdefno)
         result.values[i].floatvalue += temp->values[i].floatvalue;
     count++;
     temp = temp->next;
    }
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       result.values[i].floatvalue /= count;
  }
 else
  {
   feature_counts = (int **)safemalloc(current_dataset->featurecount * sizeof(int *), "find_mean", 25);
   for (i = 0; i < current_dataset->featurecount; i++)
     if (current_dataset->features[i].type == STRING)
       feature_counts[i] = (int *)safecalloc(current_dataset->features[i].valuecount, sizeof(int), "find_mean", 28);
   result.values = (union value *)safemalloc(current_dataset->featurecount * sizeof(union value), "find_mean", 29);
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:result.values[i].floatvalue = 0;
                    break;
       case    REAL:result.values[i].floatvalue = 0;
                    break;
      }
   while (temp)/*Adding values to total*/
    {
     for (i = 0; i < current_dataset->featurecount; i++)
       switch (current_dataset->features[i].type)
        {
         case INTEGER:result.values[i].floatvalue += temp->values[i].intvalue;
                      break;
         case    REAL:result.values[i].floatvalue += temp->values[i].floatvalue;
                      break;
         case  STRING:feature_counts[i][temp->values[i].stringindex]++;
                      break;
        }
     count++;
     temp = temp->next;
    }
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:result.values[i].floatvalue /= count;
                    break;
       case    REAL:result.values[i].floatvalue /= count;
                    break;
       case  STRING:result.values[i].stringindex = find_max_in_list(feature_counts[i], current_dataset->features[i].valuecount);
                    break;
      }
   for (i = 0; i < current_dataset->featurecount; i++)
     if (current_dataset->features[i].type == STRING)     
       safe_free(feature_counts[i]);
   safe_free(feature_counts);
  }
 return result;
}

Instance find_variance(Instanceptr myptr)
{
 /*!Last Changed 02.02.2004 added safemalloc and safecalloc*/
 /*!Last Changed 18.03.2002*/
 int i, count = data_size(myptr);
 Instanceptr temp = myptr;
 Instance mean;
 Instance result;
 mean=find_mean(myptr);
 if (mustrealize)
  {
   result.values = (union value *)safemalloc(current_dataset->multifeaturecount * sizeof(union value), "find_variance", 8);
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       result.values[i].floatvalue = 0;
   while (temp)
    {
     for (i = 0; i < current_dataset->multifeaturecount; i++)
       if (i != current_dataset->classdefno)
         result.values[i].floatvalue += (temp->values[i].floatvalue - mean.values[i].floatvalue) * (temp->values[i].floatvalue-mean.values[i].floatvalue);
     temp = temp->next;
    }
   for (i =0 ; i < current_dataset->multifeaturecount; i++)
     if (i != current_dataset->classdefno)
       result.values[i].floatvalue /= (count - 1);
  }
 else
  {
   result.values = (union value *)safemalloc(current_dataset->featurecount * sizeof(union value), "find_variance", 25);
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:result.values[i].floatvalue = 0;
                    break;
       case    REAL:result.values[i].floatvalue = 0;
                    break;
      }
   while (temp)
    {
     for (i = 0; i < current_dataset->featurecount; i++)
       switch (current_dataset->features[i].type)
        {
         case INTEGER:result.values[i].floatvalue += (temp->values[i].intvalue-mean.values[i].floatvalue) * (temp->values[i].intvalue - mean.values[i].floatvalue);
                      break;
         case    REAL:result.values[i].floatvalue += (temp->values[i].floatvalue-mean.values[i].floatvalue) * (temp->values[i].floatvalue - mean.values[i].floatvalue);
                      break;
        }
     temp=temp->next;
    }
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:result.values[i].floatvalue /= (count - 1);
                    break;
       case    REAL:result.values[i].floatvalue /= (count - 1);
                    break;
      }
  }
 safe_free(mean.values);
 return result;
}

matrix between_class_matrix(Instanceptr myptr)
{
 int i, j, k;
 Instanceptr temp = myptr;
 int *c_counts;
 Instanceptr c_means;
 Instance mean;
 matrix result = matrix_alloc(current_dataset->multifeaturecount - 1, current_dataset->multifeaturecount - 1);
 c_counts = find_class_counts(temp);
 c_means = find_class_means(temp, c_counts);
 mean = find_mean(temp);
 for (i = 0; i < current_dataset->classno; i++)
   for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
     for (k = 0; k < current_dataset->multifeaturecount - 1; k++)
       result.values[j][k] += c_counts[i] * ((real_feature(data_x(c_means, i), j) - real_feature(&mean, j))) * ((real_feature(data_x(c_means, i), k) - real_feature(&mean, k)));
 safe_free(mean.values);
 safe_free(c_counts);
 deallocate(c_means);
 return result;
}

int count_exceptions(Instanceptr list)
{
 /*!Last Changed 08.05.2004*/
 int excount = 0;
 Instanceptr tmp = list;
 while (tmp)
  {
   if (tmp->isexception)
     excount++;
   tmp = tmp->next;
  }
 return excount;
}

double exception_performance(Instanceptr list)
{
 /*!Last Changed 08.05.2004*/
 int excount = 0, error = 0;
 Instanceptr tmp = list;
 while (tmp)
  {
   if (tmp->isexception)
    {
     excount++;
     if (!(tmp->classified))
       error++;
    }
   tmp = tmp->next;
  }
 if (excount != 0)
   return (100 * (error + 0.0)) / excount;
 return 0;
}
