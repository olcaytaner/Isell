#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "classification.h"
#include "data.h"
#include "distributions.h"
#include "instance.h"
#include "instancelist.h"
#include "lang.h"
#include "libarray.h"
#include "libfile.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "messages.h"
#include "parameter.h"
#include "statistics.h"
#include "tests.h"

extern Datasetptr current_dataset;

double gaussian_vector(Instanceptr x, Instanceptr mean, double stdev)
{
 /*Last Changed 11.07.2006 if stdev = 0 returns 0*/
 int i;
 double sum = 0, x_i, m_i;
 if (stdev == 0)
   return 0;
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
  {
   x_i = real_feature(x, i);
   m_i = real_feature(mean, i);
   sum += (x_i - m_i) * (x_i - m_i);
  }
 return (1 / (stdev * sqrt(2 * PI))) * safeexp(-sum / (2 * stdev * stdev));
}

double gaussian_multivariate(Instanceptr inst, double* mean, matrix covariance)
{
 /*!Last Changed 12.01.2009 added mahalanobis_distance*/
 /*!Last Changed 25.02.2007*/
 matrix x, mu;
 double det, result;
 int d = current_dataset->multifeaturecount - 1;
 det = matrix_determinant(covariance);
 if (dequal(det, 0.0))
   return 0.0;
 x = instance_to_matrix(inst);
 /*Prepare mean matrix*/
 mu = matrix_alloc(1, x.col);
 memcpy(mu.values[0], mean, x.col * sizeof(double));
 result = (1 / sqrt(pow(2 * PI, d) * det)) * safeexp(-mahalanobis_distance(x, mu, covariance) / 2);
 matrix_free(x);
 matrix_free(mu);
 return result;
}

double gaussian(double x, double mean, double stdev)
{
 return (1 / (stdev * sqrt(2 * PI))) * safeexp(-((x - mean) * (x - mean)) / (2 * stdev * stdev));
}

void produce_distribution(double probability, int howmany, int samplesize, char *f_name)
{
 /*Last Changed 23.04.2001*/
 FILE *myfile;
 int i, j, k;
 char pst[READLINELENGTH];
 if ((myfile = fopen(f_name, "w")) == NULL)
  {
   printf(m1314, f_name);
   return;
  }
 for (i = 0; i < howmany; i++)
  {
   k = 0;
   for (j = 0; j < samplesize; j++)
     if (produce_random_value(0, 1) < probability)
       k++;
   set_precision(pst, NULL, "\n");
   fprintf(myfile, pst, k / (samplesize + 0.0));
  }
 fclose(myfile);
}

int compare_two_files_using_test(TEST_TYPE testtype, char **two_file, int *which, double confidence, double *real_confidence)
{
 /*!Last Changed 13.01.2009 added hoteling t and multi wilcoxon test*/
 /*!Last Changed 16.07.2005*/
 switch (testtype)
  {
   case BONFERRONI           :return bonferroni(two_file, which, confidence, real_confidence);
                              break;
   case SCHEFFE              :return scheffe(two_file, which, confidence, real_confidence);
                              break;
   case TUKEY                :return tukey(two_file, which, confidence, real_confidence);
                              break;
   case RANKSUMTEST          :return rank_sum_test(two_file, which, confidence, real_confidence);
                              break;
   case SIGNEDRANKTEST       :return signed_rank_test(two_file, which, confidence, real_confidence);
                              break;
   case TTEST                :return ttest(two_file, which, confidence, real_confidence);
                              break;
   case FTEST                :return ftest5x2(two_file, which, confidence, real_confidence);
                              break;
   case PAIREDT5X2           :return ttest5x2(two_file, which, confidence, real_confidence);
                              break;
   case PAIREDTTEST          :return ttestpaired(two_file, which, confidence, real_confidence);
                              break;
   case COMBINEDT5X2         :return combined5x2t(two_file, which, confidence, real_confidence);
                              break;
   case REGRESSIONF          :return regressionftest(two_file, which, confidence, real_confidence);
                              break;
   case SIGNTEST             :return signtest(two_file, which, confidence, real_confidence);
                              break;
   case HOTELLINGT           :return hotellingt(two_file, which, confidence, real_confidence);
                              break;
   case NEMENYI              :return nemenyi_two_sample(two_file, which, confidence, real_confidence);
                              break;
   case NOTEST               :return no_test(two_file, which, confidence, real_confidence);
                              break;
   case PERMUTATIONTEST      :return permutation_test(two_file, which, confidence, real_confidence);
                              break;
   case PAIREDPERMUTATIONTEST:return permutation_test(two_file, which, confidence, real_confidence);
                              break;
   default                   :printf("This test type is not supported\n");
                              return 0;
                              break;
  }
 return 0;
}

int connected_components(Linkptr edges, int nodecount)
{
 /*Last Changed 12.03.2004*/
 int** components, i, j, *counts, compcount = 0, compf, comps;
 Linkptr e;
 components = (int **)safemalloc_2d(sizeof(int *), sizeof(int), nodecount, nodecount, "connected_components", 3);
 counts = (int *)safemalloc(nodecount * sizeof(int), "connected_components", 4);
 for (i = 0; i < nodecount; i++)
  {
   components[i][0] = i;
   counts[i] = 1;
  }
 e = edges;
 while (e)
  {
   for (i = 0; i < nodecount; i++)
     for (j = 0; j < counts[i]; j++)
       if (components[i][j] == e->from)
        {
         compf = i;
         i = nodecount;
         break;
        }
   for (i = 0; i < nodecount; i++)
     for (j = 0; j < counts[i]; j++)
       if (components[i][j] == e->to)
        {
         comps = i;
         i = nodecount;
         break;
        }
   if (compf != comps)
    {
     components[compf] = alloc(components[compf], (counts[compf] + counts[comps]) * sizeof(int), counts[compf] + counts[comps]);
     for (i = 0; i < counts[comps]; i++)
       components[compf][counts[compf] + i] = components[comps][i];
     counts[compf] = counts[compf] + counts[comps];
     safe_free(components[comps]);
     counts[comps] = 0;
    }
   e = e->next;
  }
 for (i = 0; i < nodecount; i++)
   if (counts[i] > 0)
     safe_free(components[i]);
 safe_free(components);
 for (i = 0; i < nodecount; i++)
   if (counts[i] > 0)
     compcount++;
 safe_free(counts);
 return compcount;
}

int find_best_classifier(char**files, int filecount, TEST_TYPE testtype)
{
 /*Last Changed 21.10.2004*/
 int i, *indexes, found, best, which, res;
 char **two_file;
 double* means, confidence = 1 - (get_parameter_f("confidencelevel") / (filecount - 1)), conf;
 means = safemalloc(filecount * sizeof(double), "find_best_classifier", 4);
 indexes = safemalloc(filecount * sizeof(int), "find_best_classifier", 5);
 two_file = safemalloc(2 * sizeof(char *), "find_best_classifier", 6);
 for (i = 0; i < filecount; i++)
  {
   indexes[i] = i;
   means[i] = file_mean(&(files[i]), 1);
  }
 sort_two_arrays(means, indexes, filecount);
 found = 0;
 best = indexes[0];
 for (i = 1; i < filecount; i++)
   if (best > indexes[i])
    {
     two_file[0] = strcopy(two_file[0], files[indexes[i]]);
     two_file[1] = strcopy(two_file[1], files[indexes[0]]);
     res = compare_two_files_using_test(testtype, two_file, &which, confidence, &conf);
     if (!res)
      {
       found = 1;
       break;
      }
    }
 safe_free(means);
 safe_free(indexes);
 safe_free(two_file);
 if (!found)
   return best + 1;
 else
   return -1;
}

void no_correction(Comparisonptr comparisons, int compcount, double confidence)
{
 /*Last Changed 14.05.2005*/
 int i;
 for (i = 0; i < compcount; i++)
   if (comparisons[i].confidence < confidence)
     comparisons[i].accepted = 0;
}

void bonferroni_correction(Comparisonptr comparisons, int compcount, double confidence)
{
 /*Last Changed 18.11.2004*/
 int i;
 double corrected = confidence / compcount;
 for (i = 0; i < compcount; i++)
   if (comparisons[i].confidence < corrected)
     comparisons[i].accepted = 0;
}

void holm_correction(Comparisonptr comparisons, int compcount, double confidence)
{
 /*Last Changed 18.11.2004*/
 int i, j;
 double corrected;
 Comparison tmp;
 for (i = 0; i < compcount; i++)
   for (j = i + 1; j < compcount; j++)
     if (comparisons[i].confidence > comparisons[j].confidence)
      {
       tmp = comparisons[i];
       comparisons[i] = comparisons[j];
       comparisons[j] = tmp;
      }
 i = 0;
 corrected = confidence / (compcount - i);
 while (i < compcount && comparisons[i].confidence < corrected)
  {
   comparisons[i].accepted = 0;
   i++;
   corrected = confidence / (compcount - i);
  }
}

int generate_tests(char **files, int filecount)
{
 int *counts, **subsets, res, i, j;
 FILE *myfile;
 if (filecount < 2)
   return 0;
 myfile = fopen(files[0], "w");
 if (!myfile)
   return -1;
 subsets = generate_subsets(filecount - 1, &res, &counts);
 for (i = 0; i < res; i++)
   if (counts[i] > 1)
    {
     fprintf(myfile, "multitest ");
     for (j = 0; j < counts[i]; j++)
       fprintf(myfile, "%s ", files[subsets[i][j] + 1]);
     fprintf(myfile, "\n");
    }
 for (i = 0; i < res; i++)
   if (counts[i] > 0)
     safe_free(subsets[i]);
 safe_free(subsets);
 safe_free(counts);
 fclose(myfile);
 return 1;
}

Comparisonptr all_pairwise_comparisons(char** files, int filecount, int correctiontype, TEST_TYPE testtype)
{
 int i, j, compcount = 0, which;
 double confidence = get_parameter_f("confidencelevel"), conf;
 char **two_file;
 Comparisonptr comparisons;
 two_file = safemalloc(2 * sizeof(char *), "all_pairwise_comparisons", 3);
 comparisons = safemalloc(((filecount * (filecount - 1)) / 2) * sizeof(Comparison), "all_pairwise_comparisons", 5);
 for (i = 0; i < filecount; i++)
   for (j = i + 1; j < filecount; j++)
    {
     two_file[0] = strcopy(two_file[0], files[i]);
     two_file[1] = strcopy(two_file[1], files[j]);
     compare_two_files_using_test(testtype, two_file, &which, 1 - confidence, &conf);
     safe_free(two_file[0]);
     safe_free(two_file[1]);
     comparisons[compcount].alg1 = i;
     comparisons[compcount].alg2 = j;
     comparisons[compcount].confidence = 1 - conf;
     comparisons[compcount].accepted = 1;
     compcount++;
    }
 switch (correctiontype)
  {
   case BONFERRONI_CORRECTION:bonferroni_correction(comparisons, compcount, confidence);
                              break;
   case HOLM_CORRECTION      :holm_correction(comparisons, compcount, confidence);
                              break;
   case NO_CORRECTION        :no_correction(comparisons, compcount, confidence);
                              break;
  }
 safe_free(two_file);
 return comparisons;
}

void find_cliques(char** files, int filecount)
{
 BOOLEAN isclique, subset;
 char clique[MAXLENGTH];
 int *counts, **subsets, res, i, j, k, t, index, cliquecount = 0, *cliques = NULL, count = 0;
 Comparisonptr comparisons;
 comparisons = all_pairwise_comparisons(files, filecount, get_parameter_l("correctiontype"), get_parameter_l("testtype"));
 subsets = generate_subsets(filecount, &res, &counts);
 for (i = 0; i < res; i++)
   if (counts[i] > 1)
    {
     isclique = BOOLEAN_TRUE;
     for (j = 0; j < counts[i]; j++)
       for (k = j + 1; k < counts[i]; k++)
        {
         index = -1;
         for (t = 0; t < (filecount * (filecount - 1)) / 2; t++)
           if (comparisons[t].alg1 == subsets[i][j] && comparisons[t].alg2 == subsets[i][k])
            {
             index = t;
             break;
            }
         if (index == -1)
          {
           printf("Error in indexing\n");
           isclique = BOOLEAN_FALSE;
           break;           
          }
         if (!comparisons[index].accepted)
          {
           isclique = BOOLEAN_FALSE;
           break;
          }
        }
     if (isclique)
      {
       cliques = alloc(cliques, (cliquecount + 1) * sizeof(int), cliquecount + 1);
       cliques[cliquecount] = i;
       cliquecount++;
      }   
    }
 for (i = 0; i < cliquecount; i++)
  {
   subset = BOOLEAN_FALSE;
   for (j = 0; j < cliquecount; j++)
     if (i != j && is_subset(subsets[cliques[i]], subsets[cliques[j]], counts[cliques[i]], counts[cliques[j]]))
      {
       subset = BOOLEAN_TRUE;
       break;
      }
   if (!subset)
    {
     count++;
     sprintf(clique, "(");
     for (j = 0; j < counts[cliques[i]] - 1; j++)
       sprintf(clique, "%s%d ", clique, subsets[cliques[i]][j] + 1);
     sprintf(clique, "%s%d)", clique, subsets[cliques[i]][j] + 1);  
     write_array_variables("aout", count, 1, "s", clique);
    }
  }
 set_default_variable("out1", count + 0.0);
 for (i = 0; i < res; i++)
   if (counts[i] > 0)
     safe_free(subsets[i]);
 safe_free(subsets);
 safe_free(counts); 
}

int* multiple_comparison(char **files,int filecount,TEST_TYPE testtype, char* st, int* monotone, int* componentcount, CORRECTION_TYPE correctiontype)
{
 /*Last Changed 18.11.2004 added holm's correction*/
 /*Last Changed 25.02.2004 added monotonicity*/
 /*Last Changed 02.02.2004 added safemalloc Runs only with accuracy set to 0*/
 /*Last Changed 09.02.2003 !!!!  put "-" into st before calling !!!!!*/
 int i, j, *result, edgecount = 0, *inthelist, found, compcount = (filecount * (filecount - 1)) / 2;
 Linkptr edges = NULL, newedge, lastedge = NULL, tmp, tmpbefore, tmp1;
 Comparisonptr comparisons;
 char ch;
 result = (int *)safemalloc(filecount*sizeof(int), "multiple_comparison", 7);
 inthelist = (int *)safecalloc(filecount, sizeof(int), "multiple_comparison", 8);
 comparisons = all_pairwise_comparisons(files, filecount, correctiontype, testtype);
 for (i = 0; i < compcount; i++)
   if (!comparisons[i].accepted) /*If the test rejects*/
    {
     newedge = (Linkptr) safemalloc(sizeof(Link), "multiple_comparison", 36);
     newedge -> from = comparisons[i].alg1;
     newedge -> to = comparisons[i].alg2;
     ch = comparisons[i].alg1 + 1 + '0';
     sprintf(st,"%s%c",st,ch);
     ch = comparisons[i].alg2 + 1 + '0';
     sprintf(st,"%s%c",st,ch);
     sprintf(st,"%s%c",st,'-');
     newedge -> next = NULL;
     if (lastedge != NULL)
       lastedge -> next = newedge;
     else
       edges = newedge;
     lastedge = newedge;
     edgecount++;
    }
 safe_free(comparisons);
 *componentcount = connected_components(edges, filecount);
 for (j = 0; j < filecount; j++)
   for (i = 0; i < filecount; i++)
     if (!(inthelist[i]))
      {
       tmp = edges;
       while (tmp && tmp->from != i)
         tmp = tmp->next;
       if (!tmp)
        {
         tmp = edges;
         tmpbefore = NULL;
         while (tmp)
          {
           tmp1 = tmp->next;
           if (tmp->to != i)
             tmpbefore = tmp;
           else
            {
             if (tmpbefore)
               tmpbefore -> next = tmp1;
             else
               edges = tmp1; 
             safe_free(tmp);
            }
           tmp = tmp1;
          }
         result[j] = i+1;
         inthelist[i] = 1;
         break;
        }
      }
 found = 0;
 for (i = 0; i < filecount - 1; i++)
   for (j = i + 1; j < filecount; j++)
     if (result[i] > result[j])
       found++;
 if (found == edgecount)
   *monotone = 1;
 else
   *monotone = 0;
 safe_free(inthelist);
 return result;
}

void file_whiskers(char* file, double p25, double p75, double* swhisker, double* lwhisker)
{
 /*Last Changed 29.01.2007*/
 FILE* myfile;
 int count, i;
 double lowerbound, upperbound, smallest, largest, observation;
 smallest = p25;
 largest = p75;
 lowerbound = p25 - (p75 - p25) * 1.5;
 upperbound = p75 + (p75 - p25) * 1.5;
 count = file_length(file);
 if ((myfile = fopen(file, "r")) == NULL)
  {
   printf(m1274, file);
   return;
  }
 for (i = 0; i < count; i++)
  {
   fscanf(myfile, "%lf", &observation);
   if (observation <= smallest && observation >= lowerbound)
     smallest = observation;
   if (observation >= largest && observation <= upperbound)
     largest = observation;
  } 
 *swhisker = smallest;
 *lwhisker = largest;
}

double file_percentile(char *file, double percentile)
{
 /*Last Changed 11.07.2005*/
 FILE *myfile;
 double *observations, pos, result, weight;
 int count, i, posi;
 count = file_length(file);
 observations = safemalloc(count * sizeof(double), "file_percentile", 5);
 if ((myfile = fopen(file, "r")) == NULL)
  {
   printf(m1274, file);
   return 0;
  }
 for (i = 0; i < count; i++)
   fscanf(myfile, "%lf", &(observations[i]));
 fclose(myfile); 
 qsort(observations, count, sizeof(double), sort_function_double_ascending);
 pos = (2 * count * percentile) / 100;
 posi = (int) pos;
 if (pos <= 1)
   result = observations[0];
 else
   if (pos >= 2 * count - 1)
     result = observations[count - 1];
   else
     if (pos == posi && posi % 2 == 1)
       result = observations[(posi - 1) / 2];
     else
       if (posi % 2 == 1)
        {
         weight = (pos - posi) / 2;
         result = weight * observations[(posi - 1) / 2] + (1 - weight) * observations[(posi + 1) / 2];
        }
       else
        {
         weight = (pos - posi + 1) / 2;
         result = weight * observations[posi / 2 - 1] + (1 - weight) * observations[posi / 2];
        }
 safe_free(observations);
 return result;
}

double file_mean(char **files,int filecount)
{
/*Last Changed 26.04.2002*/
 FILE *myfile;
 double sum=0,temp;
 int counts,i,total=0,j;
 for (i=0;i<filecount;i++)
  {
   counts=file_length(files[i]);
   if ((myfile=fopen(files[i],"r"))==NULL)
    {
     printf(m1274, files[i]);
     return 0;
    }
   for (j=0;j<counts;j++)
    {
     fscanf(myfile,"%lf",&temp);
     sum+=temp;
    }
   total+=counts;
   fclose(myfile);
  }
 return sum/total;
}

double file_variance(char **files,int filecount)
{
/*Last Changed 27.04.2002*/
 FILE *myfile;
 double mean = file_mean(files, filecount), sum = 0, temp;
 int counts,i,total=0,j;
 for (i=0;i<filecount;i++)
  {
   counts = file_length(files[i]);
   if ((myfile = fopen(files[i],"r")) == NULL)
    {
     printf(m1274, files[i]);
     return 0;
    }
   for (j=0;j<counts;j++)
    {
     fscanf(myfile,"%lf",&temp);
     sum+=(temp-mean)*(temp-mean);
    }
   total+=counts;
   fclose(myfile);
  }
 return sqrt(sum/(total-1));
}

void generate_statistical_data(char* filename, int datacount, int zeroonecount, double errorrate)
{
 FILE* myfile;
 int i, j, zeros, x;
 char pst[READLINELENGTH];
 myfile = fopen(filename,"w");
 if (!myfile)
   return;
 for (i = 0; i<datacount; i++)
  {
   zeros = 0;
   for (j = 0; j<zeroonecount; j++)
    {
     x = myrand()*myrand();
     if (((x%10000)/10000.0)<errorrate)
       zeros++;
    }
   set_precision(pst, NULL, "\n");
   fprintf(myfile, pst, zeros / (zeroonecount + 0.0));
  }
 fclose(myfile);
}

double produce_normal_value(double mean, double stdev)
{
 /*Last Changed 08.04.2004*/
 int i;
 double x = 0;
 for (i = 0; i < 12; i++)
   x += produce_random_value(0, 1);
 x -= 6;
 return mean + x * stdev;
}

void produce_normal_distribution(char* filename, int N, double mean, double stdev)
{
 /*Last Changed 08.04.2004*/
 FILE* myfile;
 int i;
 char pst[READLINELENGTH];
 myfile = fopen(filename, "w");
 if (!myfile)
   return;
 for (i = 0; i < N; i++)
  {
   set_precision(pst, NULL, "\n");
   fprintf(myfile, pst, produce_normal_value(mean, stdev));
  }
 fclose(myfile);
}

void bootstrap(char* infile, char* outfile)
{
 /*Last Changed 08.04.2004*/
 FILE* inputfile, *outputfile;
 double *input;
 char pst[READLINELENGTH];
 int len = file_length(infile), i, index;
 inputfile = fopen(infile, "r");
 if (!inputfile)
   return;
 input = safemalloc(len * sizeof(double), "bootstrap", 5);
 for (i = 0; i < len; i++)
   fscanf(inputfile, "%lf", &(input[i]));
 fclose(inputfile);
 outputfile = fopen(outfile, "w");
 if (!outputfile)
   return;
 for (i = 0; i < len; i++)
  {
   index = myrand() % len;
   set_precision(pst, NULL, "\n");
   fprintf(outputfile, pst, input[index]);
  }
 fclose(outputfile);
 safe_free(input);
}

void multiple_tests(double (*algorithm)(char** files), char** params, int paramcount)
{
/*Last Changed 30.04.2002*/
 double tmp;
 if (paramcount > 0)
   if (paramcount > 1)
    {
     tmp = algorithm(params);
     set_default_variable("out2",tmp);
     if (tmp >= 1 - get_parameter_f("confidencelevel"))
      {
       if (get_parameter_o("displayresulton"))
         printf(m1033);
       set_default_variable("out1",1.0);
      }
     else
      {
       if (get_parameter_o("displayresulton"))
         printf(m1034);
       set_default_variable("out1",0.0);
      }
    }
   else
     printf(m1035);
 else
   printf(m1036);
}

void binary_comparison(int (*algorithm)(char** files,int *which,double confidence,double *real_confidence), char** params, int paramcount)
{
 int who;
 double conf = 0.0;
 if (paramcount >= 2)
   if (!(algorithm(params, &who, 1 - get_parameter_f("confidencelevel"), &conf)))
    {
     if (get_parameter_o("displayresulton"))
      {
       if (get_parameter_i("tailed") == 2)
         printf(m1034);
       else
         printf(m1111);
      }
     set_default_variable("out1", 0.0);
    }
   else
    {
     if (who == 1)
      {
       if (get_parameter_o("displayresulton"))
         printf(m1045);
      }
     else
       if (get_parameter_o("displayresulton"))
         if (get_parameter_i("tailed") == 2)
           printf(m1046);
     if (get_parameter_i("tailed") == 1)      
       set_default_variable("out1", 1.0);
     else
       set_default_variable("out1", who + 0.0);
    }
 else
   printf(m1047);
 set_default_variable("out2", conf);
}

void binary_tests_compare(char** names, int count, int equality, FILE* output)
{
 /*Last Changed 02.02.2004 added safemalloc*/
 char test[TESTCOUNT][50]={"bonferroni","scheffe","cvttest","ftest",
                           "tpaired5x2","ttest","wilcoxon","tukey"};
 double* results[TESTCOUNT];
 char st[100]="",st1[500];
 char* p[4], pst[READLINELENGTH];
 int i,j,k,l,m;
 FILE* infile;
 for (i=0;i<TESTCOUNT;i++)
  {
   results[i]=(double *)safemalloc((count-2)*sizeof(double), "binary_tests_compare", 10);
   for (j=0;j<count-2;j++)
     for (k=0;k<2;k++)
      {
       strcpy(st,test[i]);
       strcat(st,"-");
       strcat(st,names[j+1+k]);
       strcat(st,"-");
       strcat(st,names[j+2-k]);
       strcat(st,".txt");
       infile=fopen(st,"r");
       if (infile!=NULL)
        {
         for (l=0;l<30;l++)
          {
           fgets(st1,500,infile);
           p[0] = strtok(st1,";\n");
           for (m=0;m<3;m++)
             p[m+1]=strtok(NULL,";\n");
           if (strcmp(p[3],names[0])==0)
            {
             if (equality==0)
               results[i][j]=atof(p[2-k])/10;
             else
               results[i][j]=atof(p[0])/10;
             break;
            }
          }
         fclose(infile);
        }
      }
  }
 for (i = 0; i < TESTCOUNT; i++)
  {
   for (j = 0; j < count - 2; j++)
    {
     set_precision(pst, NULL, " ");
     fprintf(output, pst, results[i][j]);
    }
   fprintf(output,"\n");
   safe_free(results[i]);
  }
}

Performance_point tpr_fpr_performance(int tp, int fp, int tn, int fn)
{
 Performance_point p;
 if (tn + fp != 0)
   p.x = (fp + 0.0) / (tn + fp + 0.0);
 else
   p.x = 0.0;
 if (tp + fn != 0)
   p.y = (tp + 0.0) / (tp + fn + 0.0);
 else
   p.y = 0.0;
 return p;
}

Performance_point precision_recall_performance(int tp, int fp, int tn, int fn)
{
 Performance_point p;
 if (tp + fn != 0)
   p.x = (tp + 0.0) / (tp + fn + 0.0);
 else
   p.x = 0.0;
 if (tp + fp != 0)
   p.y = (tp + 0.0) / (tp + fp + 0.0);
 else
   p.y = 1.0;
 return p;
}

matrix construct_posterior_matrix(double** posteriors, Instanceptr data)
{
 int i;
 matrix m;
 Instanceptr tmp = data;
 m = matrix_alloc(data_size(data), current_dataset->classno + 1);
 for (i = 0; i < m.row; i++)
  {
   memcpy(m.values[i], posteriors[i], current_dataset->classno * sizeof(double));
   m.values[i][current_dataset->classno] = give_classno(tmp);
   tmp = tmp->next;
  }
 return m;
}

Performance_pointptr find_performance_points(matrix posteriors, int* number_of_points, int posclass, int negclass, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn))
{
 int i, fp = 0, tp = 0, classno, p = 0, n = 0;
 double fprev = -INT_MAX;
 Performance_pointptr points = NULL;
 for (i = 0; i < posteriors.row; i++)
  {
   if (posteriors.values[i][posteriors.col - 1] == posclass)
     p++;
   if (posteriors.values[i][posteriors.col - 1] == negclass)
     n++;
  }
 (*number_of_points) = 0;
 matrix_sort_by_column(posteriors, posclass, DESCENDING);
 for (i = 0; i < posteriors.row; i++)
  {
   classno = posteriors.values[i][posteriors.col - 1];
   if (posteriors.values[i][posclass] != fprev)
    {
     points = alloc(points, (*number_of_points + 1) * sizeof(Performance_point), *number_of_points + 1);
     points[*number_of_points] = calculate_performance(tp, fp, n - fp, p - tp);
     if (fprev > 0.5 && posteriors.values[i][posclass] <= 0.5)
       points[*number_of_points].equal_loss = YES;
     else 
       points[*number_of_points].equal_loss = NO;
     fprev = posteriors.values[i][posclass];
     (*number_of_points)++;
    }
   if (classno == posclass)
     tp++;
   if (classno == negclass)
     fp++;
  }
 points = alloc(points, (*number_of_points + 1) * sizeof(Performance_point), *number_of_points + 1);
 points[*number_of_points] = calculate_performance(tp, fp, n - fp, p - tp);
 (*number_of_points)++;
 return points;
}

double find_area_under_the_curve_2class(matrix posteriors, int posclass, int negclass, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn))
{
 int i, number_of_points;
 double area = 0.0;
 Performance_pointptr points;
 points = find_performance_points(posteriors, &number_of_points, posclass, negclass, calculate_performance);
 for (i = 0; i < number_of_points - 1; i++)
   area += (points[i].y + points[i + 1].y) * (points[i + 1].x - points[i].x) / 2;
 safe_free(points);
 return area;
}

double find_area_under_the_curve_kclass(matrix posteriors, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn))
{
 double sum = 0.0;
 int i, j;
 for (i = 0; i < current_dataset->classno; i++)
   for (j = i + 1; j < current_dataset->classno; j++)
     sum += find_area_under_the_curve_2class(posteriors, i, j, calculate_performance);
 return (2 * sum) / (current_dataset->classno * (current_dataset->classno - 1));
}

/**
 * Constructs the confusion matrix information from the posterior array.
 * @param posterior_file_name Name of the file that contains the posterior information
 * @param decision_threshold Default value is 0.5. If the posterior probability of positive class is larger than this value, the instance is correctly classified
 * @return 
 */
Confusion_matrix construct_confusion_matrix_from_posteriors(matrix posteriors, double decision_threshold)
{
 /*!Last Changed 17.01.2009*/
 int i;
 Confusion_matrix result = {0, 0, 0, 0};
 for (i = 0; i < posteriors.row; i++)
   if (posteriors.values[i][POSITIVE_CLASS] >= decision_threshold) /*positive class assigned*/
     if (posteriors.values[i][2] == POSITIVE_CLASS)
       result.true_positives++;
     else
       result.false_positives++;
   else /*negative class assigned*/
     if (posteriors.values[i][2] == NEGATIVE_CLASS)
       result.true_negatives++;
     else
       result.false_negatives++;
 return result;
}

Confusion_matrixptr construct_confusion_matrix_from_file(char* posterior_file_name, double decision_threshold)
{
 /*!Last Changed 17.01.2009*/
 matrixptr posteriors;
 Confusion_matrixptr result;
 int i, numberofruns = get_parameter_i("runcount") * get_parameter_i("foldcount");
 result = safemalloc(numberofruns * sizeof(Confusion_matrix), "construct_confusion_matrix_from_file", 4);
 posteriors = read_posteriors(numberofruns, posterior_file_name);
 for (i = 0; i < numberofruns; i++)
  {
   result[i] = construct_confusion_matrix_from_posteriors(posteriors[i], decision_threshold);
   matrix_free(posteriors[i]);
  }
 safe_free(posteriors);
 return result;
}
