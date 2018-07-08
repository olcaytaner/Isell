#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "datastructure.h"
#include "dimreduction.h"
#include "distributions.h"
#include "eps.h"
#include "lang.h"
#include "libarray.h"
#include "libfile.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"
#include "parameter.h"
#include "pstricks.h"
#include "tests.h"

void assign_ranks_to_observations(Obsptr obs, int size)
{
 /*!Last Changed 13.01.2009*/
 int i, sum, j, l;
 i = 0;
 while (i < size)
  {
   j = i;
   while (i + 1 < size && obs[i].observation == obs[i + 1].observation)
     i++;
   if (i != j)
    {
     sum = 0;
     for (l = j + 1; l <= i + 1; l++)
       sum += l;
     for (l = j; l <= i; l++)
       obs[l].rij = (sum + 0.0) / (i - j + 1);
    }
   else
     obs[i].rij = i + 1;
   i++;
  }
}

void assign_ranks_to_equal_size_observations(Obsptr* obs, int k, int N)
{
	int i, j, t, l, sum;
 Obs tmp, *obsarray;
 obsarray = safemalloc(k * sizeof(Obs), "assign_ranks_to_equal_size_observations", 3); 
	for (i = 0; i < N; i++)
	 {
			for (j = 0; j < k; j++)
				 for (t = j + 1; t < k; t++)
							if ((obs[t][i].observation < obs[j][i].observation && !get_parameter_o("accuracy")) || (obs[t][i].observation > obs[j][i].observation && get_parameter_o("accuracy")))
        {
         tmp = obs[t][i];
         obs[t][i] = obs[j][i];
         obs[j][i] = tmp;
        }
   j = 0;
   while (j < k)
    {
     t = j;
     while (j + 1 < k && obs[j][i].observation == obs[j + 1][i].observation)
       j++;
     if (j != t)
      {
       sum = 0;
       for (l = t + 1; l <= j + 1; l++)
         sum += l;
       for (l = t; l <= j; l++)
         obs[l][i].rij = (sum + 0.0) / (j - t + 1);
      }
     else
       obs[j][i].rij = j + 1;
     j++;
    }
   for (j = 0; j < k; j++)
     obsarray[obs[j][i].index] = obs[j][i];
   for (j = 0; j < k; j++)
     obs[j][i] = obsarray[j];
		}
 safe_free(obsarray);
}

int read_multivariate_observations_for_multiple_file(char** files, matrix observations[], int L)
{
 /*!Last Changed 06.06.2010*/
 int i, j, k, colcount, rowcount;
 FILE* myfile;
 colcount = number_of_floats_in_file(files[0]);
 rowcount = file_length(files[0]);
 for (i = 1; i < L; i++)
  {
   if (number_of_floats_in_file(files[i]) != colcount)
    {
     printf(m1411);
     return -1;
    }
   if (file_length(files[i]) != rowcount)
    {
     printf(m1438);
     return -1;
    }
  }
 for (i = 0; i < L; i++)
  {
   observations[i] = matrix_alloc(rowcount, colcount);
   if ((myfile = fopen(files[i], "r")) == NULL)
    {
     printf(m1274, files[i]);
     return -1;
    }
   for (j = 0; j < observations[i].row; j++)
     for (k = 0; k < colcount; k++)
       fscanf(myfile, "%lf", &(observations[i].values[j][k]));
   fclose(myfile);
  }
 return colcount;
}

Obs* read_observations_for_multiple_file(char** files, int** counts, int* size, int* filecount)
{
 /*!Last Changed 13.01.2009*/
 int treatments = 0, total = 0, k = 0, i;
 Obs *obs = NULL;
 FILE* myfile;
 while (files[treatments])
  {
   treatments++;
   *counts = alloc(*counts, treatments * sizeof(int), treatments);
   *counts[treatments - 1] = file_length(files[treatments - 1]);
   total += *counts[treatments - 1];
   obs = alloc(obs, total * sizeof(Obs), total);
   if ((myfile = fopen(files[treatments - 1], "r")) == NULL)
    {
     printf(m1274, files[treatments - 1]);
     return 0;
    }
   for (i = 0; i < *counts[treatments - 1]; i++)
    {
     fscanf(myfile, "%lf", &obs[k + i].observation);
     obs[k + i].observation_no = treatments - 1;
     obs[k + i].rij = 0;
     obs[k + i].index = k + i;
    }
   k = total;
   fclose(myfile);
  }
 *size = k;
 *filecount = treatments;
 return obs;
}

int read_multivariate_observations_for_two_file(char** files, matrix observations[2])
{
 /*!Last Changed 12.01.2009*/
 int i, j, k, colcount, rowcount;
 FILE* myfile;
 colcount = number_of_floats_in_file(files[0]);
 if (number_of_floats_in_file(files[1]) != colcount)
  {
   printf(m1411);
   return -1;
  }
 for (i = 0; i < 2; i++)
  {
   rowcount = file_length(files[i]);
   observations[i] = matrix_alloc(rowcount, colcount);
   if ((myfile = fopen(files[i], "r")) == NULL)
    {
     printf(m1274, files[i]);
     return -1;
    }
   for (j = 0; j < observations[i].row; j++)
     for (k = 0; k < colcount; k++)
       fscanf(myfile, "%lf", &(observations[i].values[j][k]));
   fclose(myfile);
  }
 return colcount;
}

matrix read_multivariate_observations_for_file(char* filename)
{
 int j, k, colcount, rowcount;
 FILE* myfile;
 matrix observations;
 colcount = number_of_floats_in_file(filename);
 if (colcount == -1)
   return matrix_alloc(1, 1);
 rowcount = file_length(filename);
 observations = matrix_alloc(rowcount, colcount);
 if ((myfile = fopen(filename, "r")) == NULL)
  {
   printf(m1274, filename);
   return matrix_alloc(1, 1);
  }
 for (j = 0; j < observations.row; j++)
   for (k = 0; k < colcount; k++)
     fscanf(myfile, "%lf", &(observations.values[j][k]));
 fclose(myfile);
 return observations;
}

double** read_observations_for_two_file(char** files, int* counts)
{
 /*Last Changed 06.02.2004*/
 double** observations;
 int i, j;
 FILE* myfile;
 observations = (double **)safemalloc(2 * sizeof(double*), "find_observations_for_two_file", 4);
 for (i = 0;i < 2;i++)
  {
   counts[i] = file_length(files[i]);
   observations[i] = (double *)safemalloc(counts[i] * sizeof(double), "find_observations_for_two_file", 8);
   if ((myfile = fopen(files[i], "r")) == NULL)
    {
     printf(m1274, files[i]);
     return NULL;
    }
   for (j = 0; j < counts[i]; j++)
     fscanf(myfile, "%lf", &observations[i][j]);
   fclose(myfile);
  }
 return observations;
}

double manova(char **files)
{
 int i, j, p, L = 0, k, t, v, df1, df2, df3;
 matrix* observations;
 matrix H, E, EH, eigenvectors;
 double **averages;
 double *overallaverage, *eigenvalues, statistic, ratio, res;
 while (files[L])
   L++;
 observations = safemalloc(L * sizeof(matrix), "manova", 5);
 p = read_multivariate_observations_for_multiple_file(files, observations, L);
 k = observations[0].row;
 if (p == -1)
  {
   for (i = 0; i < L; i++)
     matrix_free(observations[i]);
   safe_free(observations);
   return 0;
  }
 averages = safemalloc(L * sizeof(double*), "manova", 11);
 overallaverage = safecalloc(p, sizeof(double), "manova", 12);
 for (i = 0; i < L; i++)
   averages[i] = matrix_average_columns(observations[i]);
 for (i = 0; i < L; i++)
   for (j = 0; j < p; j++)
     overallaverage[j] += averages[i][j];
 for (i = 0; i < p; i++)
   overallaverage[i] /= L;
 H = matrix_alloc(p, p);
 E = matrix_alloc(p, p); 
 for (i = 0; i < L; i++)
   for (j = 0; j < p; j++)
     for (t = 0; t < p; t++)
       H.values[j][t] += (averages[i][j] - overallaverage[j]) * (averages[i][t] - overallaverage[t]);
 matrix_multiply_constant(&H, k);
 safe_free(overallaverage);
 for (i = 0; i < L; i++)
   for (j = 0; j < k; j++)
     for (t = 0; t < p; t++)
       for (v = 0; v < p; v++)
         E.values[t][v] += (observations[i].values[j][t] - averages[i][t]) *  (observations[i].values[j][v] - averages[i][v]);
 for (i = 0; i < L; i++)
  {
   safe_free(averages[i]);
   matrix_free(observations[i]);
  }
 safe_free(observations);
 safe_free(averages);
 EH = matrix_sum(E, H);
 ratio = matrix_determinant(E) / matrix_determinant(EH);
 matrix_inverse(&E);
 matrix_product(&E, H);
 eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, E);
 if (eigenvalues != NULL)
  {
   for (i = 1; i < E.col + 1; i++)
     write_array_variables("aout", i, 1, "f", eigenvalues[i - 1]);
   for (i = 0; i < E.col; i++)
     for (j = 0; j < E.row; j++)
       write_array_variables("aout", j + 1, 2 + i, "f", eigenvectors.values[j][i]);
   safe_free(eigenvalues);
   matrix_free(eigenvectors);
  }
 matrix_free(H);
 matrix_free(E);
 matrix_free(EH);
 df1 = p;
 df2 = L * (k - 1);
 df3 = L - 1;
 statistic = ((df1 + df3 + 1) / 2.0 - (df2 + df3)) * log(ratio);
 res = chi_square(df1 * df3, statistic);
 return 1 - res;
}

double anova(char **files)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 /*Last Changed 23.04.2001*/
 FILE *myfile;
 int treatments = 0, df1, df2;
 int *counts = NULL, i, j, total = 0;
 double **observations = NULL;
 double *means, overall_mean = 0.0;
 double res, mst, mse, sst = 0.0, sse = 0.0;
 while (files[treatments])
  {
   treatments++;
   counts = alloc(counts, treatments * sizeof(int), treatments);
   observations = alloc(observations, treatments * sizeof(double *), treatments);
   counts[treatments - 1] = file_length(files[treatments - 1]);
   observations[treatments - 1] = (double *)safemalloc(counts[treatments - 1] * sizeof(double), "anova", 13);
   if ((myfile = fopen(files[treatments - 1], "r")) == NULL)
    {
     printf(m1274, files[treatments - 1]);
     return 0;
    }
   for (i = 0; i < counts[treatments - 1]; i++)
     fscanf(myfile, "%lf", &observations[treatments - 1][i]);
   fclose(myfile);
  }
 means = (double *)safecalloc(treatments, sizeof(double), "anova", 23);
 for (i = 0; i < treatments; i++)
  {
   for (j = 0; j < counts[i]; j++)
     means[i] += observations[i][j];
   overall_mean += means[i];
   total += counts[i];
   means[i] /= counts[i];
  }
 overall_mean /= total;
 for (i = 0; i < treatments; i++)
  {
   sst += counts[i] * pow(means[i] - overall_mean, 2);
   for (j = 0; j < counts[i]; j++)
     sse += pow(observations[i][j] - means[i], 2);
  }
 df1 = treatments - 1;
 df2 = total - treatments;
 mst=sst / df1;
 mse=sse / df2;
 free_2d((void**)observations, treatments);
 safe_free(counts);
 safe_free(means);
 res = f_distribution(mst / mse, df1, df2);
 return 1-res;
}

int bonferroni(char **files,int *which,double confidence,double *real_confidence)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 /*Last Changed 07.05.2001*/
 int i, j, counts[2], df;
 double means[2], **observations, mse, sse = 0.0, variance, res, upper, lower;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 for (i = 0; i < 2; i++)
  {
   means[i] = 0.0;
   for (j = 0; j < counts[i]; j++)
     means[i] += observations[i][j];
   means[i] /= counts[i];
  }
 if (means[0] > means[1])
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which); 
 for (i = 0; i < 2; i++)
   for (j = 0; j < counts[i]; j++)
     sse += pow(observations[i][j] - means[i], 2);
 df = counts[0] + counts[1] - 2;
 mse = sse / df;
 free_2d((void**)observations, 2);
 variance = sqrt(mse * (1.0 / counts[0] + 1.0 / counts[1]));
 (*real_confidence) = 1 - t_distribution((means[0] - means[1]) / variance, df);
 if (get_parameter_i("tailed") == 1)
  {
   res = t_distribution_inverse(1 - confidence, df);
   if ((means[0] - means[1]) < res * variance)
     return 0;
   else
     return 1;
  }
 else
  {
   res = t_distribution_inverse((1 - confidence) / 2, df);
   lower = -variance * res;
   upper = variance * res;
   if ((means[0] - means[1] > lower) && (means[0] - means[1] < upper))
     return 0;
   else
     return 1;
  }
}

int combined5x2t(char **files, int *which, double confidence, double *real_confidence)
{
 /*Last Changed 06.10.2004*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int counts[2], i, df;
 double **observations, *difference, *variance, *means, num, denom, sum, upper, lower;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 difference = (double *)safemalloc(counts[0] * sizeof(double), "combined5x2t", 15);
 means = (double *)safemalloc((counts[0] / 2) * sizeof(double), "combined5x2t", 16);
 variance = (double *)safemalloc((counts[0] / 2) * sizeof(double), "combined5x2t", 17);
 sum = 0;
 for (i = 0; i < counts[0]; i++)
  {
   difference[i] = observations[0][i] - observations[1][i];
   write_array_variables("aout", i + 1, 1, "f", difference[i]);
   sum += difference[i];
  }
 if (sum > 0)
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);    
 denom = 0;
 num = 0;
 for (i = 0; i < (counts[0] / 2); i++)
  {
   means[i] = (difference[2 * i] + difference[2 * i + 1]) / 2;
   num += means[i];
   variance[i] = (difference[2 * i] - means[i]) * (difference[2 * i] - means[i]) + (difference[2 * i + 1] - means[i]) * (difference[2 * i + 1] - means[i]);
   denom += variance[i];
  }
 num *= sqrt(10);
 num /= 5;
 denom /= 5;
 denom = sqrt(denom);
 free_2d((void**)observations, 2);
 safe_free(difference);
 safe_free(means);
 safe_free(variance);
 df = counts[0] / 2;
 if (denom != 0)
  {
   sum = t_distribution(num / denom, df);
   (*real_confidence) = 1 - sum;
  }
 else
   return 0;
 if (get_parameter_i("tailed") == 1)
  {
   upper = t_distribution_inverse(1 - confidence, df);
   if ((num / denom) < upper)
     return 0;
   else
     return 1;
  }
 else
  {
   upper = t_distribution_inverse((1 - confidence) / 2, df);
   lower = -t_distribution_inverse((1 - confidence) / 2, df);
   if ((num / denom) > lower && (num / denom) < upper)
     return 0;
   else
     return 1;
  }
}

int ftest5x2(char **files, int *which, double confidence, double *real_confidence)
{
 /*Add divide by 0 check to num/denom */
 /*Last Changed 02.02.2004 added safemalloc*/
 /*Last Changed 16.05.2002*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int counts[2], i, df1, df2;
 double **observations, *difference, *variance, *means, num, denom, sum;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 difference = (double *)safemalloc(counts[0] * sizeof(double), "ftest5x2", 16);
 means = (double *)safemalloc((counts[0] / 2) * sizeof(double), "ftest5x2", 17);
 variance = (double *)safemalloc((counts[0] / 2) * sizeof(double), "ftest5x2", 18);
 sum = 0;
 num = 0;
 for (i = 0; i < counts[0]; i++)
  {
   difference[i] = observations[0][i] - observations[1][i];
   write_array_variables("aout", i + 1, 1, "f", difference[i]);
   num += difference[i] * difference[i];
   sum += difference[i];
  }
 if (sum > 0)
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);    
 denom = 0;
 for (i = 0; i < (counts[0] / 2); i++)
  {
   means[i] = (difference[2 * i] + difference[2 * i + 1]) / 2;
   variance[i] = (difference[2 * i] - means[i]) * (difference[2 * i] - means[i]) + (difference[2 * i + 1] - means[i]) * (difference[2 * i + 1] - means[i]);
   denom += variance[i];
  }
 denom *= 2;
 free_2d((void**)observations, 2);
 safe_free(difference);
 safe_free(means);
 safe_free(variance);
 df1 = counts[0];
 df2 = counts[0] / 2;
 if (denom != 0)
   (*real_confidence) = 1 - f_distribution(num / denom, df1, df2);
 else
   return 0;
 if (get_parameter_i("tailed") == 2)
  {
   if ((num / denom) < f_distribution_inverse(1 - confidence, df1, df2))
     return 0;
   else
     return 1;
  }
 else
	 {
   if ((num / denom) < f_distribution_inverse(1 - confidence, df1, df2))
     return 0;
   else
     if (sum < 0)
      {
       (*real_confidence) = 1 - (*real_confidence);
       return 0;      
      }
     else
       return 1;
		}
 return 0;
}

int index_in_multiple_sample(int index, int* pos, matrix observations[], int L)
{
 int i, sum = 0;
 for (i = 0; i < L; i++)
   if (index < sum + observations[i].row)
    {
     *pos = index - sum;
     return i;
    }
   else
     sum += observations[i].row;
 return -1;
}

matrix distance_matrix_for_multiple_multivariate_sample(matrix observations[], int L, int N)
{
 int i, j, p, treatmentno, pos;
 double *row1, *row2;
 matrix result;
 p = observations[0].col;
 result = matrix_alloc(N, N);
 for (i = 0; i < N; i++)
   result.values[i][i] = -1;
 for (i = 0; i < N; i++)
  {
   treatmentno = index_in_multiple_sample(i, &pos, observations, L);
   row1 = observations[treatmentno].values[pos];
   for (j = i + 1; j < N; j++)
    {
     treatmentno = index_in_multiple_sample(j, &pos, observations, L);
     row2 = observations[treatmentno].values[pos];
     result.values[i][j] = euclidian_distance(row1, row2, p);
     result.values[j][i] = result.values[i][j];
    }
  }
 return result;
}

void save_mst_to_array_variables(Graphptr mst, matrix observations[], int L)
{
 int i, j, k, fromindex, toindex, from, to;
 Point p1, p2;
 k = 1;
 for (i = 0; i < mst->nvertices; i++)
   for (j = 0; j < mst->degree[i]; j++)
     if (i < mst->edges[i][j].v)
      {
       fromindex = index_in_multiple_sample(i, &from, observations, L);
       toindex = index_in_multiple_sample(mst->edges[i][j].v, &to, observations, L);
       p1.x = observations[fromindex].values[from][0];
       p1.y = observations[fromindex].values[from][1];
       p2.x = observations[toindex].values[to][0];
       p2.y = observations[toindex].values[to][1];
       write_array_variables("aout", k, 1, "ffff", p1.x, p1.y, p2.x, p2.y); 
       k++;
      }
}

int waldwolfowitz(char **files, int *which, double confidence, double *real_confidence)
{
 int p, m, n, i, j, R, N, C;
 double expectation, variance, statistic;
 matrix observations[2], distancematrix;
 Graphptr mst;
 p = read_multivariate_observations_for_two_file(files, observations);
 if (p == -1)
  {
   matrix_free(observations[0]);
   matrix_free(observations[1]);
   return 0;
  }
 m = observations[0].row;
 n = observations[1].row;
 N = m + n;
 *which = 3;
 distancematrix = distance_matrix_for_multiple_multivariate_sample(observations, 2, N); 
 mst = minimum_spanning_tree_from_observations(observations, distancematrix, 2, N);
 matrix_free(distancematrix);
 if (mst == NULL)
   return 0;
 if (p == 2)
   save_mst_to_array_variables(mst, observations, 2);
 matrix_free(observations[0]);
 matrix_free(observations[1]);
 R = 1;
 for (i = 0; i < mst->nvertices; i++)
   for (j = 0; j < mst->degree[i]; j++)
     if (i < mst->edges[i][j].v && ((i < m && mst->edges[i][j].v >= m) || (i >= m && mst->edges[i][j].v < m)))
       R++;
 expectation = 2 * m * n / (N + 1.0) + 1;
 C = 0;
 for (i = 0; i < mst->nvertices; i++)
   if (mst->degree[i] > 1)
     C += binom(mst->degree[i], 2);
 free_graph(mst);
 set_default_variable("out3", R);
 variance = ((2 * m * n) / (N * (N - 1.0))) * ((2 * m * n - N) / (N + 0.0) + ((C - N + 2) / ((N - 2) * (N - 3))) * (N * (N - 1) - 4 * m * n + 2));
 statistic = (R - expectation) / sqrt(variance);
 if (get_parameter_i("tailed") == 2)
  {
   (*real_confidence) = normal(statistic, 1);
   if ((*real_confidence) < confidence)
     return 0;
   else
     return 1;
  }
 else
   printf(m1316);
 return 0;
}

void height_directed_preorder_traversal(Graphptr mst, int* visit_count, int current, matrix distancematrix)
{
 int i, to, minheight, selected;
 double mindistance;
 mst->visitorder[current] = *visit_count;
 while (1)
  {
   selected = -1;
   minheight = +INT_MAX;
   mindistance = +LONG_MAX;
   for (i = 0; i < mst->degree[current]; i++)
    {
     to = mst->edges[current][i].v;
     if (mst->visitorder[to] == 0)
       if (mst->height[to] < minheight || (mst->height[to] == minheight && distancematrix.values[current][to] < mindistance))
        {
         selected = to;
         minheight = mst->height[to];
         mindistance = distancematrix.values[current][to];
        }
    }
   if (selected != -1)
    {
     (*visit_count)++;
     height_directed_preorder_traversal(mst, visit_count, selected, distancematrix);
    }
   else 
     break;
  }
}

Graphptr minimum_spanning_tree_from_observations(matrix observations[], matrix distancematrix, int L, int N)
{
 Graphptr g, mst;
 g = graph_from_distancematrix(distancematrix);
 mst = minimum_spanning_tree_prim(g);
 if (get_parameter_o("displayresulton") == ON)
  {
   printf("MST\n");
   print_graph(mst);
  }
 if (mst->nedges != (2 * (N - 1)))
  {
   printf("Minimum spanning tree can not be found\n");
   return NULL;
  }
 free_graph(g); 
 return mst;
}

void ranks_from_minimum_spanning_tree(Graphptr mst, matrix distancematrix)
{
 double *path_lengths;
 int j, maxecc, ecc, i, visitorder, root, N;
 maxecc = 0;
 N = mst->nvertices;
 for (i = 0; i < N; i++)
  {
   ecc = eccentricity(mst, i);
   if (ecc > maxecc)
    {
     root = i;
     maxecc = ecc;
    }
  }
 path_lengths = shortest_path(mst, root);
 for (i = 0; i < N; i++)
  {
   if (mst->height[i] < path_lengths[i])
     mst->height[i] = path_lengths[i];
   j = mst->parents[i];
   while (j != -1)
    {
     if (mst->height[j] < path_lengths[i])
       mst->height[j] = path_lengths[i];
     j = mst->parents[j];
    }
  }
 safe_free(path_lengths);
 visitorder = 1;
 height_directed_preorder_traversal(mst, &visitorder, root, distancematrix);
 if (get_parameter_o("displayresulton") == ON)
  {
   printf("Visit Order\n");
   for (i = 0; i < N; i++)
     printf("%d: %d\n", i, mst->visitorder[i]);
  } 
}

int smirnov(char **files, int *which, double confidence, double *real_confidence)
{
 matrix distancematrix;
 int p, m, n, i, j, r, s, N;
 double D, d_i, statistic;
 Graphptr mst;
 matrix observations[2];
 p = read_multivariate_observations_for_two_file(files, observations);
 if (p == -1)
  {
   matrix_free(observations[0]);
   matrix_free(observations[1]);
   return 0;
  }
 multivariate_normality_test(files[0], confidence, &statistic);
 set_default_variable("out4",statistic + 0.0);
 multivariate_normality_test(files[1], confidence, &statistic);
 set_default_variable("out5",statistic + 0.0);
 m = observations[0].row;
 n = observations[1].row;
 N = m + n;
 *which = 3;
 distancematrix = distance_matrix_for_multiple_multivariate_sample(observations, 2, N);
 mst = minimum_spanning_tree_from_observations(observations, distancematrix, 2, N);
 if (mst == NULL)
   return 0;
 ranks_from_minimum_spanning_tree(mst, distancematrix);
 for (i = 0; i < m; i++)
   write_array_variables("aout", i + 1, 5, "d", mst->visitorder[i]);
 for (i = m; i < N; i++)
   write_array_variables("aout", i - m + 1, 6, "d", mst->visitorder[i]);
 if (p == 2)
   save_mst_to_array_variables(mst, observations, 2);
 matrix_free(observations[0]);
 matrix_free(observations[1]);
 D = 0;
 for (i = 1; i <= N; i++)
  {
   r = 0;
   s = 0;
   for (j = 0; j < N; j++)
     if (mst->visitorder[j] <= i)
      {
       if (j < m)
         r++;
       else 
         s++;
      }
   d_i = fabs(r / (m + 0.0) - s / (n + 0.0));
   if (d_i > D)
     D = d_i;
  }
 if (get_parameter_o("displayresulton") == ON)
   printf("D: %.2f\n", D);
 set_default_variable("out3", D);
 matrix_free(distancematrix);
 free_graph(mst);
 if (get_parameter_i("tailed") == 2)
  {
   *real_confidence = 1 - kolmogorov_two_sample(m, n, D);
   if (*real_confidence < confidence)
     return 0;
   else 
     return 1;
  }
 else
   printf(m1316); 
 return 0;
}

int hotellingt(char **files, int *which, double confidence, double *real_confidence)
{
 /*!Last Changed 12.01.2009*/
 int i, k, p, m, df1, df2;
 double *samplemean, num, denom, statistic;
 matrix observations[2], difference, covariance, mean, eigenvec, dummy, result, tmp, meant, weights;
 p = read_multivariate_observations_for_two_file(files, observations);
 if (p == -1)
  {
   matrix_free(observations[0]);
   matrix_free(observations[1]);
   return 0;
  }
 if (observations[0].row != observations[1].row)
  {
   printf(m1426);
   return 0;
  }
 multivariate_normality_test(files[0], confidence, &statistic);
 set_default_variable("out4", statistic + 0.0);
 multivariate_normality_test(files[1], confidence, &statistic);
 set_default_variable("out5", statistic + 0.0);
 k = observations[0].row;
 *which = 3;
 difference = matrix_difference(observations[0], observations[1]);
 covariance = matrix_covariance(difference, &samplemean);
 mean = matrix_alloc(1, p);
 memcpy(mean.values[0], samplemean, p * sizeof(double));
 safe_free(samplemean);
 dummy = matrix_copy(covariance);
 if (!matrix_inverse(&dummy))
  {
   find_eigenvalues_eigenvectors(&eigenvec, covariance, &p, 1.0 - DBL_DELTA);
   reduce_dimension_of_covariance_matrix(&covariance, p, eigenvec);
   reduce_dimension_of_vector(&mean, p, eigenvec);
   matrix_free(eigenvec);
   p++;
  }
 set_default_variable("out3", p + 0.0);
 matrix_free(dummy);
 m = k - 1;
 matrix_inverse(&covariance);
 tmp = matrix_multiply(mean, covariance);
 meant = matrix_transpose(mean);
 result = matrix_multiply(tmp, meant);
 weights = matrix_multiply(covariance, meant);
 for (i = 0; i < p; i++)
   write_array_variables("aout", i + 1, 1, "f", weights.values[i][0]); 
 matrix_free(weights);
 num = (m - p + 1) * k * result.values[0][0];
 matrix_free(meant);
 matrix_free(tmp);
 matrix_free(result);
 matrix_free(covariance);
 matrix_free(mean);
 for (i = 0; i < 2; i++)
   matrix_free(observations[i]);
 denom = p * m;
 df1 = p;
 df2 = m - p + 1;
 if (denom != 0)
   (*real_confidence) = 1 - f_distribution(num / denom, df1, df2);
 else
   return 0;
 if (get_parameter_i("tailed") == 2)
  {
   if ((num / denom) < f_distribution_inverse(1 - confidence, df1, df2))
     return 0;
   else
     return 1;
  }
 else
   printf(m1316);
 return 0;  
}

int multivariate_normality_test(char* filename, double confidence, double* test_statistic)
{
 double* averages;
 matrix observations, S, g, observations_transpose, tmp;
 int i, j, p, n, freedom;
 double sum, res;
 observations = read_multivariate_observations_for_file(filename);
 p = observations.col;
 n = observations.row;
 if (n == 1)
  {
   matrix_free(observations);
   return 1;
  }
 S = matrix_covariance(observations, &averages);
 if (matrix_determinant(S) == 0)
  {
   matrix_free(observations);
   matrix_free(S);
   safe_free(averages);
   return 1;
  }
 matrix_inverse(&S);
 for (i = 0; i < n; i++)
   for (j = 0; j < p; j++)
     observations.values[i][j] -= averages[j];
 observations_transpose = matrix_transpose(observations);
 tmp = matrix_multiply(observations, S);
 g = matrix_multiply(tmp, observations_transpose);
 matrix_free(tmp);
 matrix_free(observations_transpose);
 matrix_free(observations);
 matrix_free(S);
 safe_free(averages);
 sum = 0.0;
 for (i = 0; i < n; i++)
   for (j = 0; j < n; j++)
     sum += g.values[i][j] * g.values[i][j] * g.values[i][j];
 matrix_free(g);
 sum /= (n * n);
 *test_statistic = (sum * n) / 6.0;
 freedom = (p * (p + 1) * (p + 2)) / 6;
 res = 1 - chi_square(freedom, *test_statistic);
 if (res > confidence)
   return 1;
 else
   return 0;
}

double kruskalwallis_multivariate(char **files)
{
 Graphptr mst;
 matrix* observations, distancematrix;
 int treatmentno, pos, i, p, n, treatments = 0, df, N = 0;
 double *T, S, sumd, res;
 while (files[treatments])
   treatments++;
 observations = safemalloc(treatments * sizeof(matrix), "kruskalwallis_multivariate", 5);
 p = read_multivariate_observations_for_multiple_file(files, observations, treatments);
 if (p == -1)
  {
   for (i = 0; i < treatments; i++)
     matrix_free(observations[i]);
   safe_free(observations);
   return 0;
  } 
 n = observations[0].row; 
 for (i = 0; i < treatments; i++) 
   N += observations[i].row; 
 T = (double *)safecalloc(treatments, sizeof(double), "kruskalwallis_multivariate", 15);
 distancematrix = distance_matrix_for_multiple_multivariate_sample(observations, treatments, N);
 mst = minimum_spanning_tree_from_observations(observations, distancematrix, treatments, N);
 if (mst == NULL)
   return 0;
 ranks_from_minimum_spanning_tree(mst, distancematrix);
 matrix_free(distancematrix);
 for (i = 0; i < N; i++)
  {
   treatmentno = index_in_multiple_sample(i, &pos, observations, treatments);
   write_array_variables("aout", pos + 1, 5 + treatmentno, "d", mst->visitorder[i]);
  }
 if (p == 2)
   save_mst_to_array_variables(mst, observations, treatments);
 for (i = 0; i < N; i++)
  {
   treatmentno = index_in_multiple_sample(i, &pos, observations, treatments);
   T[treatmentno] += mst->visitorder[i];
  }
 free_graph(mst);
 sumd = 0;
 for (i = 0; i < treatments; i++)
   sumd += T[i] * T[i] / observations[i].row;
 for (i = 0; i < treatments; i++)
   matrix_free(observations[i]);
 safe_free(observations); 
 safe_free(T);
 S = sumd * ((12 + 0.0) / (treatments * n * (treatments * n + 1))) - 3 * (treatments * n + 1);
 df = treatments - 1;
 res = chi_square(df, S);
 return 1 - res;
}

double kruskalwallis(char **files)
{
 /*!Last Changed 13.01.2009 added find_observations_for_multiple_file, qsort, assign_ranks_to_observations*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 30.01.2002*/
 int treatments, df;
 int *counts = NULL, i, k = 0, n;
 Obs *obs;
 double *T, S, sumd, res;
 obs = read_observations_for_multiple_file(files, &counts, &k, &treatments);
 T = (double *)safecalloc(treatments, sizeof(double), "kruskalwallis", 27);
 qsort(obs, k, sizeof(Obs), sort_function_observation_ascending);
 assign_ranks_to_observations(obs, k);
 for (i = 0; i < k; i++)
   T[obs[i].observation_no] += obs[i].rij;
 sumd = 0;
 for (i = 0; i < treatments; i++)
   sumd += T[i] * T[i] / counts[i];
	safe_free(counts);
 safe_free(T);
 n = counts[0];
 S = sumd * ((12 + 0.0) / (treatments * n * (treatments * n + 1))) - 3 * (treatments * n + 1);
 safe_free(obs);
 df = treatments - 1;
 res = chi_square(df, S);
 return 1 - res;
}

Obsptr* read_observations_for_multiple_equal_size_file(char **files, int* size, int* count)
{
 int i, j;
 Obsptr *obs = NULL;
 FILE* myfile;
	*count = 0;
	*size = file_length(files[*count]);
	(*count)++;
 while (files[*count])
	 {
			if (file_length(files[*count]) != *size)
				 return NULL;
			(*count)++;
		}
	obs = safemalloc((*count) * sizeof(Obsptr), "find_observations_for_multiple_equal_file", 12);
 for (i = 0; i < *count; i++)
	 {
			if ((myfile = fopen(files[i], "r")) == NULL)
 			{
	 			printf(m1274, files[i]);
		 		return NULL;
		 	}
			obs[i] = safemalloc((*size) * sizeof(Obs), "find_observations_for_multiple_equal_file", 20);
			for (j = 0; j < *size; j++)
			 {
					fscanf(myfile, "%lf", &(obs[i][j].observation));
					obs[i][j].index = i;
				}	
			fclose(myfile);
	 }
 return obs;
}

double friedman(char **files)
{
 /*!Last Changed 25.04.2010*/
 int i, j, N, k, df1, df2;
 Obsptr *obs;
 double *R, S, F, sumri2, res;
 obs = read_observations_for_multiple_equal_size_file(files, &N, &k);
 assign_ranks_to_equal_size_observations(obs, k, N);
 R = (double *)safecalloc(k, sizeof(double), "friedman", 6);
 for (i = 0; i < k; i++)
   for (j = 0; j < N; j++)
	 	  R[i] += obs[i][j].rij;
	sumri2 = 0.0;
 for (i = 0; i < k; i++)
	 { 
		 R[i] /= N;
   write_array_variables("aout", i + 1, 3, "f", R[i]);
   if (get_parameter_o("displayresulton") == ON)
     printf("%.6f ", R[i]);
			sumri2 += R[i] * R[i];
		}
 if (get_parameter_o("displayresulton") == ON)
   printf("\n");
 safe_free(R);
 safe_free(obs);
 S = ((12.0 * N) / (k * (k + 1))) * (sumri2 - ((k * (k + 1) * (k + 1)) / 4.0));
 if (N * (k - 1) - S != 0)
  {
	  F = ((N - 1) * S) / (N * (k - 1) - S);
   df1 = k - 1;
   df2 = (k - 1) * (N - 1);
	  res = f_distribution(F, df1, df2);
   return 1 - res;
  }
 else
   return 1.0;
}

int nemenyi_two_sample(char **files, int *which, double confidence, double *real_confidence)
{
 int i, j, counts[2], N, k = get_parameter_i("nearcount");
 double **observations, sum[2] = {0.0, 0.0}, cd;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**) observations, 2);
   return 0;
  }
 N = counts[0];
 for (i = 0; i < 2; i++)
  {
   for (j = 0; j < N; j++)
     sum[i] += observations[i][j];
   sum[i] /= N;
  }
 free_2d((void**)observations, 2);
 cd = studentized_range_inverse(confidence, 12000, k) * sqrt((k * (k + 1)) / (12.0 * N));
 if (get_parameter_i("tailed") == 1)
  {
   *real_confidence = studentized_range((sum[0] - sum[1]) / sqrt((k * (k + 1)) / (12.0 * N)), 12000, k);
   if (sum[0] - sum[1] <= cd)
     return 0;
   else
     return 1;
  }
 else
  {
   *real_confidence = studentized_range(fabs(sum[0] - sum[1]) / sqrt((k * (k + 1)) / (12.0 * N)), 12000, k);
   if (fabs(sum[0] - sum[1]) <= cd)
     return 0;
   else
     return 1;
  }
}

void nemenyi(char **files, double confidence)
{
 /*!Last Changed 26.04.2010*/
 int i, j, N, k, t;
 Obsptr *obs;
 double *R, cd;
 double critical99[49] = {2.576, 2.913, 3.113, 3.255, 3.364, 3.452, 3.526, 3.590, 3.646, 3.696, 3.741, 3.781, 3.818, 3.853, 3.884, 3.914, 3.941, 3.967, 3.992, 4.015, 4.037, 4.057, 4.077,4.096, 4.114, 4.132, 4.148, 4.164, 4.179, 4.194, 4.208, 4.222, 4.236, 4.249, 4.261, 4.273, 4.285, 4.296, 4.307, 4.318, 4.329, 4.339, 4.349, 4.359, 4.368, 4.378, 4.387, 4.395, 4.404};
 double critical95[49] = {1.960, 2.344, 2.569, 2.728, 2.850, 2.948, 3.031, 3.102, 3.164, 3.219, 3.268, 3.313, 3.354, 3.391, 3.426, 3.458, 3.489, 3.517, 3.544, 3.569, 3.593, 3.616, 3.637, 3.658, 3.678, 3.696, 3.714, 3.732, 3.749, 3.765, 3.780, 3.795, 3.810, 3.824, 3.837, 3.850, 3.863, 3.876, 3.888, 3.899, 3.911, 3.922, 3.933, 3.943, 3.954, 3.964, 3.973, 3.983, 3.992};
 double critical90[49] = {1.645, 2.052, 2.291, 2.460, 2.589, 2.693, 2.780, 2.855, 2.920, 2.978, 3.030, 3.077, 3.120, 3.159, 3.196, 3.230, 3.261, 3.291, 3.319, 3.346, 3.371, 3.394, 3.417, 3.439, 3.459, 3.479, 3.498, 3.516, 3.533, 3.550, 3.567, 3.582, 3.597, 3.612, 3.626, 3.640, 3.653, 3.666, 3.679, 3.691, 3.703, 3.714, 3.726, 3.737, 3.747, 3.758, 3.768, 3.778, 3.788};
 obs = read_observations_for_multiple_equal_size_file(files, &N, &k);
 assign_ranks_to_equal_size_observations(obs, k, N);
 R = (double *)safecalloc(k, sizeof(double), "friedman", 6);
 for (i = 0; i < k; i++)
   for (j = 0; j < N; j++)
     R[i] += obs[i][j].rij;
 for (i = 0; i < k; i++)
  {
   R[i] /= N;
   if (get_parameter_o("displayresulton") == ON)
     printf("%.6f ", R[i]);
  }
 if (get_parameter_o("displayresulton") == ON)
   printf("\n");
 t = 0;
 if (confidence == 0.99)
   cd = critical99[k - 2] * sqrt((k * (k + 1)) / (6.0 * N));
 else
   if (confidence == 0.95)
     cd = critical95[k - 2] * sqrt((k * (k + 1)) / (6.0 * N));
   else
     if (confidence == 0.90)
       cd = critical90[k - 2] * sqrt((k * (k + 1)) / (6.0 * N));
     else
       cd = critical95[k - 2] * sqrt((k * (k + 1)) / (6.0 * N));
 if (get_parameter_o("displayresulton") == ON)
   printf("CD: %.6f\n", cd);
 for (i = 0; i < k; i++)
   for (j = i + 1; j < k; j++)
    {
     if (fabs(R[i] - R[j]) > cd)
      {
       t++;
       write_array_variables("aout", t, 1, "d", i + 1);
       write_array_variables("aout", t, 2, "d", j + 1);
      }
    }
 set_default_variable("out1", t + 0.0);
 switch (get_parameter_l("imagetype"))
  {
        case IMAGE_EPS:rangetest_plot(R, k, files, cd);
                       break;
   case IMAGE_PSTRICKS:rangetest_plot_pstricks(R, k, files, cd);
                       break;
  } 
 safe_free(R);
}

int no_test(char **files, int *which, double confidence, double *real_confidence)
{
 /*Last Changed 07.04.2006*/
 int j, counts[2], plus, minus;
 double **observations;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   return 0;
  }
 if (counts[0] != counts[1])
   return 0;
 plus = 0;
 minus = 0;
 for (j = 0; j < counts[0]; j++)
  {
   if (observations[0][j] > observations[1][j])
     plus++;
   if (observations[0][j] < observations[1][j])
     minus++;
  }
 free_2d((void**)observations, 2);
 if (plus > minus)
  {
   (*which) = 1;
   if (!get_parameter_o("accuracy")) 
     (*which) = 3 - (*which); 
   (*real_confidence) = 1;
   return 1;
  }
 else
  {
   (*which) = 2; 
   if (!get_parameter_o("accuracy")) 
     (*which) = 3 - (*which); 
   (*real_confidence) = 0;
   return 0;
  }
}

char* range_tests(char **files, int filecount, TEST_TYPE testtype, int* bestindex, int* caseno, char* newst)
{
 /*Last Changed 25.10.2004 updated the best search*/
 /*Last Changed 21.10.2004 added sort_two_arrays*/
 /*Last Changed 02.02.2004 added safemalloc*/
 /*Last Changed 05.05.2002*/
 /*Last Changed 14.08.2003 Added bestindex to represent best algorithm*/
 FILE *myfile;
 int treatments=0,*counts=NULL, i, j, k, m, p, *indexes, **lines,linecount = 0, found, **significance;
 double **observations=NULL, *means, multiplier, mse, sse=0.0, diff, confidence, n, test_result;
 char* st=NULL, ch;
 char tempst1[100],tst[100];
 switch (testtype)
  {
   case NEWMANKEULS:confidence= 1 - get_parameter_f("confidencelevel");
                    break;
   case DUNCAN     :confidence= 1 - pow(get_parameter_f("confidencelevel"), filecount-1);
                    break;
   default         :printf("This test type is not supported for range tests\n");
                    break;
  }
 while (files[treatments])
  {
   treatments++;
   counts=alloc(counts,treatments*sizeof(int),treatments);
   observations=alloc(observations,treatments*sizeof(double *),treatments);
   counts[treatments-1]=file_length(files[treatments-1]);
   observations[treatments-1]=(double *)safemalloc(counts[treatments-1]*sizeof(double), "range_tests", 19);
   if ((myfile=fopen(files[treatments-1],"r"))==NULL)
    {
     printf(m1274, files[treatments-1]);
     return 0;
    }
   for (i=0;i<counts[treatments-1];i++)
     fscanf(myfile,"%lf",&observations[treatments-1][i]);
   fclose(myfile);
  }
 significance = (int **)safecalloc_2d(sizeof(int *), sizeof(int), treatments, treatments, "range_tests", 29);
 for (i = 0; i < treatments; i++)
   for (j = i + 1; j < treatments; j++)
     significance[i][j] = 1;
 means = (double *)safemalloc(treatments*sizeof(double), "range_tests", 33);
 indexes = (int *)safemalloc(treatments*sizeof(int), "range_tests", 34);
 for (i=0;i<treatments;i++)
  {
   indexes[i]=i;
   means[i]=0.0;
   for (j=0;j<counts[i];j++)
     means[i]+=observations[i][j];
   means[i]/=counts[i];
  }
 for (i=0;i<treatments;i++)
   for (j=0;j<counts[i];j++)
     sse+=pow(observations[i][j]-means[i],2);
 sort_two_arrays(means, indexes, treatments);
 if (!get_parameter_o("texoutput"))
  {
   strcpy(tempst1, "");
   for (i = 0; i < treatments; i++)
     sprintf(tempst1, "%s%c", tempst1, indexes[i] + '1');
   sprintf(tempst1, "%s:", tempst1);
  }
 mse=sse/(treatments*(counts[0]-1));
 free_2d((void**)observations, treatments);
 n=counts[0];
 safe_free(counts);
 multiplier=sqrt(mse/n);
 for (i=0;i<filecount-1;i++)
   for (j=0;j<=i;j++)
    {
     found = 0;
     for (k=0; k<linecount; k++)
       if (j >= lines[k][0] && (filecount-1+j-i) <= lines[k][1])
        {
         found = 1;
         break;
        }
     if (found)
       continue;
     diff=means[filecount-1+j-i]-means[j];
     switch (testtype)
      {
       case NEWMANKEULS:
       case DUNCAN     :test_result=studentized_range_inverse(confidence,filecount*(n-1),filecount-i);
                        break;
       default         :printf("This test type is not supported for range tests\n");
                        break;
      }
     if (diff<(multiplier*test_result))
      {
       linecount++;
       lines = (int **) alloc(lines, linecount * sizeof(int *), linecount);
       lines[linecount-1] = (int *)safemalloc(2*sizeof(int), "range_tests", 85);
       lines[linecount-1][0] = j;
       lines[linecount-1][1] = filecount-1+j-i;
       for (k = j; k <= filecount-1+j-i; k++)
         for (m = k + 1; m <= filecount-1+j-i; m++)
           if (indexes[k] < indexes[m])
             significance[indexes[k]][indexes[m]] = 0;
           else
             significance[indexes[m]][indexes[k]] = 0;
      }
    }
 /*Find the best*/
 found = -1;
 for (k = 0; k < linecount; k++)
   if (lines[k][0] == 0)
    {
     found = k;
     break;
    }
 if (found == -1)
  {
   *bestindex = indexes[0];
   *caseno = 1;
  }
 else
  {
   *caseno = 2;
   *bestindex = indexes[lines[found][0]];
   for (k = lines[found][0] + 1; k <= lines[found][1]; k++)
     if (indexes[k] < *bestindex)
       *bestindex = indexes[k];
   for (k = 0; k < linecount; k++)
    {
     if (k == found)
       continue;
     j = 0;
     for (m = lines[k][0]; m <= lines[k][1]; m++)
       if (indexes[m] == *bestindex)
        {
         j = 1;
         break;
        }
     if (j)
      {
       for (m = lines[k][0]; m <= lines[k][1]; m++)
         if (indexes[m] < *bestindex)
          {
           *bestindex = -1;
           break;
          }
       *caseno = 4;
      }
     if (*bestindex == -1)
      {
       *caseno = 5;
       break;
      }
    }
  }
 if (*caseno == 2)
   for (k = lines[found][0]; k <= lines[found][1]; k++)
     if (*caseno != 3 && indexes[k] != *bestindex)
       for (p = 0; p < linecount; p++)
         if (*caseno != 3 && p != found)
           for (m = lines[p][0]; m <= lines[p][1]; m++)         
             if (indexes[m] == indexes[k])
               *caseno = 3;
 if (*bestindex != -1)
   (*bestindex)++;
 for (i = 0; i < linecount; i++)
  {
   sprintf(tst, "%s", "");
   for (j = lines[i][0]; j <= lines[i][1]; j++)
    {
     sprintf(tst,"%s%c",tst,'1'+indexes[j]);
    }
   for (k = 0; k < lines[i][1]-lines[i][0]+1; k++)
     for (m = k + 1; m < lines[i][1]-lines[i][0]+1; m++)
       if (tst[k]>tst[m])
        {
         ch = tst[k];
         tst[k] = tst[m];
         tst[m] = ch;
        }
   strcat(tempst1,tst);
   if (i != linecount-1)
     strcat(tempst1,"-");
  }
 for (k = 0; k < treatments; k++)
   for (m = k + 1; m < treatments; m++)
     if (significance[k][m])
       sprintf(newst,"%s%c%c-",newst,k+'1',m+'1');
 free_2d((void**)significance, treatments);
 if (linecount > 0)
   free_2d((void**)lines, linecount);
 safe_free(means);
 safe_free(indexes);
 st=strcopy(st,tempst1);
 return st;
}

int regressionftest(char **files,int *which,double confidence,double *real_confidence)
{
 /*Last Changed 13.12.2004*/
 int counts[2], i, df1, df2;
 long int datacount;
 double **observations, sum1, sum2;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 datacount = counts[0];
 sum1 = 0;
 sum2 = 0;
 for (i = 0; i < datacount; i++)
  {
   sum1 += observations[0][i];
   sum2 += observations[1][i];
  }
 if (sum1 > sum2)
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);    
 free_2d((void**)observations, 2);
 if (sum2 == 0)
   return 0;
 df1 = datacount;
 df2 = datacount;
 *real_confidence = 1 - f_distribution((sum1 / sum2), df1, df2);
 if (get_parameter_i("tailed") == 2)
  {
   if ((sum1 / sum2) < f_distribution_inverse(1 - confidence, df1, df2))
     return 0;
   else
     return 1;
  }
 else
   printf(m1316);
 return 0;
}

int scheffe(char **files,int *which,double confidence,double *real_confidence)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 /*Last Changed 07.05.2001*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int i, j, counts[2], df;
 double means[2], **observations, mse, sse = 0.0, variance, c1min, c1max, res, tmp;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 for (i = 0; i < 2; i++)
  {
   means[i] = 0.0;
   for (j = 0; j < counts[i]; j++)
     means[i] += observations[i][j];
   means[i] /= counts[i];
  }
 if (means[0] > means[1])
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);    
 for (i = 0; i < 2; i++)
   for (j = 0; j < counts[i]; j++)
     sse += pow(observations[i][j] - means[i], 2);
 df = counts[0] + counts[1] - 2;
 mse = sse / df;
 free_2d((void**) observations, 2);
 variance = sqrt(mse * (1.0 / counts[0] + 1.0 / counts[1]));
 tmp = fabs(means[0] - means[1]) / variance;
 (*real_confidence) = 1 - f_distribution(tmp * tmp, 1, df);
 if (get_parameter_i("tailed") == 2)
  {
   res = sqrt(f_distribution_inverse(1 - confidence, 1, df));
   c1min = means[0] - means[1] - variance * res;
   c1max = means[0] - means[1] + variance * res;
   if ((c1min < 0) && (c1max > 0))
     return 0;
   else
     return 1;
  }
 else
   printf(m1316);
 return 0;
}

int signtest(char **files, int *which, double confidence, double *real_confidence)
{
 /*Last Changed 28.01.2005*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int j, counts[2], plus, minus, total, range;
 double **observations, prob;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
   return 0;
 plus = 0;
 minus = 0;
 for (j = 0; j < counts[0]; j++)
  {
   if (observations[0][j] < observations[1][j])
     plus++;
   if (observations[0][j] > observations[1][j])
     minus++;
  }
 free_2d((void**)observations, 2);
 if (plus < minus)
  {
   (*which) = 1;
   if (!get_parameter_o("accuracy")) 
     (*which) = 3 - (*which);    
   range = plus;
  }
 else
  {
   (*which) = 2; 
   if (!get_parameter_o("accuracy")) 
     (*which) = 3 - (*which);    
   range = minus;
  }
 total = plus + minus;
 prob = 0.0;
 for (j = 0; j <= range; j++)
   prob += binom(total, j) / pow(2, total);
 (*real_confidence) = prob;
 if (get_parameter_i("tailed") == 1)
  {
   if (prob < 1 - confidence)
     return 1;
   else
     return 0;
  }
 else
  {
   if (prob < (1 - confidence) / 2)
     return 1;
   else
     return 0;
  }
}

int ttest(char **files, int *which, double confidence, double *real_confidence)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 /*Last Changed 12.02.2002*/
 int i, j, counts[2], df;
 double means[2], **observations, sse = 0.0, nom, denom, upper, lower;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 for (i = 0; i < 2; i++)
  {
   means[i] = 0.0;
   for (j = 0; j < counts[i]; j++)
     means[i] += observations[i][j];
   means[i] /= counts[i];
  }
 if (means[0] > means[1])
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);       
 for (i =  0; i < 2; i++)
   for (j = 0; j < counts[i]; j++)
     sse += pow(observations[i][j] - means[i], 2);
 free_2d((void**)observations, 2);
 denom = sqrt(sse);
 df = counts[0] + counts[1] - 2;
 nom = (means[0] - means[1]) * sqrt((counts[0] * counts[1] * df) / (counts[0] + counts[1] + 0.0));
 (*real_confidence) = 1 - t_distribution(nom / denom, df);
 if (get_parameter_i("tailed") == 1)
  {
   upper = t_distribution_inverse(1 - confidence, df);
   if ((nom / denom) < upper)
     return 0;
   else
     return 1;
  }
 else
  {
   upper = t_distribution_inverse((1 - confidence) / 2, df);
   lower = -t_distribution_inverse((1 - confidence) / 2, df);
   if ((nom / denom) > lower && (nom / denom) < upper)
     return 0;
   else
     return 1;
  }
}

int ttest5x2(char **files, int *which, double confidence, double *real_confidence)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 /*Last Changed 28.05.2002*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int counts[2], i, df;
 double **observations, *difference, *variance, *means, num, denom, sum, upper, lower;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 difference = (double *)safemalloc(counts[0] * sizeof(double), "ttest5x2", 15);
 means = (double *)safemalloc((counts[0] / 2) * sizeof(double), "ttest5x2", 16);
 variance = (double *)safemalloc((counts[0] / 2) * sizeof(double), "ttest5x2", 17);
 sum = 0;
 for (i = 0; i < counts[0]; i++)
  {
   difference[i] = observations[0][i] - observations[1][i];
   write_array_variables("aout", i + 1, 1, "f", difference[i]);
   sum += difference[i];
  }
 if (sum > 0)
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);       
 denom = 0;
 for (i = 0; i < (counts[0] / 2); i++)
  {
   means[i] = (difference[2 * i] + difference[2 * i + 1]) / 2;
   variance[i] = (difference[2 * i] - means[i]) * (difference[2 * i] - means[i]) + (difference[2 * i + 1] - means[i]) * (difference[2 * i + 1] - means[i]);
   denom += variance[i];
  }
 denom /= 5;
 denom = sqrt(denom);
 num = difference[0];
 free_2d((void**)observations, 2);
 safe_free(difference);
 safe_free(means);
 safe_free(variance);
 df = counts[0] / 2;
 if (denom != 0)
  {
   sum = t_distribution(num / denom, df);
   (*real_confidence) = 1 - sum;
  }
 else
  {
   (*real_confidence) = 0.5;
   return 0;
  }
 if (get_parameter_i("tailed") == 1)
  {
   upper = t_distribution_inverse(1 - confidence, df);
   if ((num / denom) < upper)
     return 0;
   else
     return 1;
  }
 else
  {
   upper = t_distribution_inverse((1 - confidence) / 2, df);
   lower = -t_distribution_inverse((1 - confidence) / 2, df);
   if ((num / denom) > lower && (num / denom) < upper)
     return 0;
   else
     return 1;
  }
}

int ttestpaired(char **files, int *which, double confidence, double *real_confidence)
{
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int counts[2], i, df;
 double **observations, *difference, sum, upper, lower, mean, variance;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 difference = (double *)safemalloc(counts[0] * sizeof(double), "ttestcv", 15);
 sum = 0;
 for (i = 0; i < counts[0]; i++)
  {
   difference[i] = observations[0][i] - observations[1][i];
   write_array_variables("aout", i + 1, 1, "f", difference[i]);
   sum += difference[i];
  }
 mean = sum / counts[0];
 if (sum > 0)
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);       
 sum = 0;
 for (i = 0; i < counts[0]; i++)
   sum += (difference[i] - mean) * (difference[i] - mean);
 df = counts[0] - 1;
 variance = sqrt(sum / df);
 free_2d((void**)observations, 2);
 safe_free(difference);
 if (variance == 0)
  {
  (*real_confidence) = 0.5;
   return 0;
  }
 if (get_parameter_i("tailed") == 1)
  {
   upper = t_distribution_inverse(1 - confidence, df);
   (*real_confidence) = 1.0 - t_distribution(sqrt(counts[0]) * mean / variance, df);
   if ((sqrt(counts[0]) * mean / variance) < upper)
     return 0;
   else
     return 1;
  }
 else
  {
   upper = t_distribution_inverse((1 - confidence) / 2, df);
   lower = -t_distribution_inverse((1 - confidence) / 2, df);
   (*real_confidence) = 1.0 - t_distribution(fabs(sqrt(counts[0]) * mean / variance), df);
   if ((sqrt(counts[0]) * mean / variance) > lower && (sqrt(counts[0]) * mean / variance) < upper)
     return 0;
   else
     return 1;
  }
}

int tukey(char **files,int *which,double confidence,double *real_confidence)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 /*Last Changed 28.06.2001*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int i, j, counts[2], df;
 double means[2], **observations, mse, sse = 0.0, variance, c1min, c1max, res, tmp;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 for (i = 0; i < 2; i++)
  {
   means[i] = 0.0;
   for (j = 0; j < counts[i]; j++)
     means[i] += observations[i][j];
   means[i] /= counts[i];
  }
 if (means[0] > means[1])
   (*which) = 1;
 else
   (*which) = 2;
 if (!get_parameter_o("accuracy")) 
   (*which) = 3 - (*which);       
 for (i = 0; i < 2; i++)
   for (j = 0; j < counts[i]; j++)
     sse += pow(observations[i][j] - means[i], 2);
 df = counts[0] + counts[1] - 2;
 mse = sse / df;
 free_2d((void**)observations, 2);
 variance = sqrt((mse / 2) * (1.0 / counts[0] + 1.0 / counts[1]));
 tmp = fabs(means[0] - means[1]) / variance;
 (*real_confidence) = studentized_range(tmp, df, 2);
 res = studentized_range_inverse(confidence, df, 2);
 c1min = means[0] - means[1] - variance * res;
 c1max = means[0] - means[1] + variance * res;
 if (get_parameter_i("tailed") == 2)
   if ((c1min < 0) && (c1max > 0))
     return 0;
   else
     return 1;
 else
   printf(m1316);
 return 0;
}

double vanderwaerdan(char **files)
{
 /*!Last Changed 13.01.2009 added find_observations_for_multiple_file, qsort, assign_ranks_to_observations*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 14.02.2002*/
 int treatments, df;
 int *counts = NULL, i, k;
 Obs *obs = NULL;
 double *T, S = 0, sumd, res;
 obs = read_observations_for_multiple_file(files, &counts, &k, &treatments);
 T = (double *)safecalloc(treatments, sizeof(double), "vanderwaerdan", 27);
 qsort(obs, k, sizeof(Obs), sort_function_observation_ascending);
 assign_ranks_to_observations(obs, k);
 for (i = 0; i < k; i++)
  {
   obs[i].rij /= k+1;
   obs[i].rij = z_inverse(obs[i].rij);
   S += obs[i].rij * obs[i].rij;
   T[obs[i].observation_no] += obs[i].rij;
  }
 S /= k - 1;
 sumd = 0;
 for (i = 0; i < treatments; i++)
  {
   T[i] /= counts[i];
   sumd += counts[i] * T[i] * T[i];
  }
	safe_free(counts);
 sumd /= S;
 safe_free(obs);
 safe_free(T);
 df = treatments - 1;
 res = chi_square(df, sumd);
 return 1 - res;
}

int permutation_test_paired(char **files, int *which, double confidence, double *real_confidence)
{
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int i, counts[2], count, total;
 double **observations, *difference, mean, current;
 int* iterator;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 difference = (double *)safemalloc(counts[0] * sizeof(double), "permutation_test_paired", 15);
 for (i = 0; i < counts[0]; i++)
   difference[i] = observations[0][i] - observations[1][i];
 free_2d((void**)observations, 2);
 mean = sum_of_list_double(difference, counts[0]) / counts[0];
 iterator = first_iterator(counts[0]);
 count = 0;
 total = 0;
 do{
   current = 0;
   for (i = 0; i < counts[0]; i++)
     if (iterator[i])
       current += fabs(difference[i]);
     else
       current += -fabs(difference[i]);
   total++;
   if (mean > current / counts[0])
     count++;
 }while (next_iterator(iterator, counts[0], 1));
 if (get_parameter_i("tailed") == 2)
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < ((1 + confidence) / 2) && (*real_confidence) > ((1 - confidence) / 2))
     return 0;
   else
     return 1;
  }
 else
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < confidence)
     return 0;
   else
     return 1;
  }
}

int permutation_test(char **files, int *which, double confidence, double *real_confidence)
{
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int i, counts[2], count, total;
 double **observations, sum[2], current[2];
 int* combination;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 sum[0] = sum_of_list_double(observations[0], counts[0]);
 sum[1] = sum_of_list_double(observations[1], counts[1]);
 combination = first_combination(counts[0]);
 count = 0;
 total = 0;
 do{
   current[0] = 0;
   for (i = 0; i < counts[0]; i++)
     if (combination[i] < counts[0])
       current[0] += observations[0][combination[i]];
     else
       current[0] += observations[1][combination[i] - counts[0]];
   current[1] = sum[0] + sum[1] - current[0];
   total++;
   if (sum[0] / counts[0] - sum[1] / counts[1] > current[0] / counts[0] - current[1] / counts[1])
     count++;
 }while (next_combination(combination, counts[0], counts[0] + counts[1] - 1));
 free_2d((void**)observations, 2);
 if (get_parameter_i("tailed") == 2)
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < ((1 + confidence) / 2) && (*real_confidence) > ((1 - confidence) / 2))
     return 0;
   else
     return 1;
  }
 else
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < confidence)
     return 0;
   else
     return 1;
  }
}

int signed_rank_test(char **files, int *which, double confidence, double *real_confidence)
{
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 int i, counts[2], *iterator, count, total;
 double** observations, ranksum, current;
 Obs *obs = NULL;
 observations = read_observations_for_two_file(files, counts);
 if (!observations)
  {
   *real_confidence = 0.5;
   return 0;
  }
 if (counts[0] != counts[1])
  {
   printf(m1315);
   free_2d((void**)observations, 2);
   return 0;
  }
 obs = (Obs *)safemalloc(counts[0] * sizeof(Obs), "wilcoxon", 7);
 for (i = 0; i < counts[0]; i++)
  {
   obs[i].observation = fabs(observations[0][i] - observations[1][i]);
   obs[i].index = i;
   if (observations[0][i] > observations[1][i])
     obs[i].observation_no = 1;
   else
     obs[i].observation_no = -1;
   obs[i].rij = 0;
  }
 free_2d((void**)observations, 2);
 qsort(obs, counts[0], sizeof(Obs), sort_function_observation_ascending);
 assign_ranks_to_observations(obs, counts[0]);
 ranksum = 0;
 for (i = 0; i < counts[0]; i++)
   if (obs[i].observation_no == 1)
     ranksum += obs[i].rij;
 iterator = first_iterator(counts[0]);
 count = 0;
 total = 0;
 do{
   current = 0;
   for (i = 0; i < counts[0]; i++)
     if (iterator[i])
       current += obs[i].rij;
   total++;
   if (ranksum > current)
     count++;
 }while (next_iterator(iterator, counts[0], 1));
 safe_free(obs);
 if (get_parameter_i("tailed") == 2)
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < ((1 + confidence) / 2) && (*real_confidence) > ((1 - confidence) / 2))
     return 0;
   else
     return 1;
  }
 else
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < confidence)
     return 0;
   else
     return 1;
  }
}

int rank_sum_test(char **files, int *which, double confidence, double *real_confidence)
{
 /*Last Changed 02.02.2004 added safemalloc Added alpha/2 levels*/
 /*One Tailed Test for H0: mu1 <= mu2 H1: mu1 > mu2*/
 /*Two Tailed Test for H0: mu1 = mu2 H1: mu1 != mu2*/
 /*confidence = 0.95 for alpha = 0.05, confidence = 0.99 for alpha = 0.01 etc.*/
 /*Last Changed 30.01.2002*/
 FILE *myfile;
 int i, j, counts[2], k, count, total, *combination;
 double T[2] = {0}, tmp, current;
 Obs *obs;
 for (i = 0; i < 2; i++)
   counts[i] = file_length(files[i]);
 obs = (Obs *)safemalloc((counts[0] + counts[1]) * sizeof(Obs), "rank_sum_test", 7);
 k = 0;
 for (i = 0; i < 2; i++)
  {
   if ((myfile = fopen(files[i], "r")) == NULL)
    {
     printf(m1274, files[i]);
     return 0;
    }
   for (j = 0; j < counts[i]; j++)
    {
     fscanf(myfile, "%lf", &tmp);
     obs[k].observation = tmp;
     obs[k].observation_no = i;
     obs[k].rij = 0;
     obs[k].index = k;
     k++;
    }
   fclose(myfile);
  }
 qsort(obs, k, sizeof(Obs), sort_function_observation_ascending);
 assign_ranks_to_observations(obs, k);
 for (i = 0; i < k; i++)
   T[obs[i].observation_no] += obs[i].rij;
 count = 0;
 total = 0;
 combination = first_combination(counts[0]);
 do{
   current = 0;
   for (i = 0; i < counts[0]; i++)
     current += obs[combination[i]].rij;
   if (T[0] > current)
     count++;
   total++;
 }while (next_combination(combination, counts[0], counts[0] + counts[1] - 1)); 
 safe_free(obs);
 if (get_parameter_i("tailed") == 2)
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < ((1 + confidence) / 2) && (*real_confidence) > ((1 - confidence) / 2))
     return 0;
   else
     return 1;
  }
 else/*one tailed*/
  {
   (*real_confidence) = count / (total + 0.0);
   if ((*real_confidence) < confidence)
     return 0;
   else
     return 1;
  }
}
