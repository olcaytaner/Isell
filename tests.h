#ifndef __tests_H
#define __tests_H

#include "matrix.h"
#include "datastructure.h"

enum { 
      SKEWNESS = 0,
      KURTOSIS};

enum test_type{ 
      BONFERRONI = 0,
      SCHEFFE,
      TUKEY,
      FTEST,
      NEWMANKEULS,
      DUNCAN,
      RANKSUMTEST,
      SIGNEDRANKTEST,
      TTEST,
      VANDERWAERDAN,
      PAIREDT5X2,
      PAIREDTTEST,
      COMBINEDT5X2,
      REGRESSIONF,
      SIGNTEST,
      PERMUTATIONTEST,
      PAIREDPERMUTATIONTEST,
      HOTELLINGT,
      NEMENYI,
      NOTEST};

typedef enum test_type TEST_TYPE;

/*! Observations of K treatments. Used in the nonparametric multiple comparison algorithms such as Kruskal Wallis test*/
struct obs{
           /*! Treatment number of the observation*/
           int observation_no;
           /*! Observation itself*/
           double observation;
           /*! r value of the observation. r value is the index of the observation after sorting the observations in the increasing order. If two or more observations are equal, r value is equal to the average of the index values.*/
           double rij;
           /*! Index of the observation*/
           int index;
          };

typedef struct obs Obs;
typedef Obs* Obsptr;

double    anova(char **files);
void      assign_ranks_to_observations(Obsptr obs, int size);
void      assign_ranks_to_equal_size_observations(Obsptr* obs, int k, int N);
int       bonferroni(char **files,int *which,double confidence,double *real_confidence);
int       combined5x2t(char **files,int *which,double confidence,double *real_confidence);
matrix    distance_matrix_for_multiple_multivariate_sample(matrix observations[], int L, int N);
double    friedman(char **files);
int       ftest5x2(char **files,int *which,double confidence,double *real_confidence);
void      height_directed_preorder_traversal(Graphptr mst, int* visit_count, int current, matrix distancematrix);
int       hotellingt(char **files, int *which, double confidence, double *real_confidence);
int       index_in_multiple_sample(int index, int* pos, matrix observations[], int L);
double    kruskalwallis(char **files);
double    kruskalwallis_multivariate(char **files);
double    manova(char **files);
Graphptr  minimum_spanning_tree_from_observations(matrix observations[], matrix distancematrix, int L, int N);
int       multivariate_normality_test(char* filename, double confidence, double* test_statistic);
void      nemenyi(char **files, double confidence);
int       nemenyi_two_sample(char **files, int *which, double confidence, double *real_confidence);
int       no_test(char **files, int *which, double confidence, double *real_confidence);
int       permutation_test(char **files, int *which, double confidence, double *real_confidence);
int       permutation_test_paired(char **files, int *which, double confidence, double *real_confidence);
char*     range_tests(char **files, int filecount, TEST_TYPE testtype, int* bestindex, int* caseno, char* newst);
int       rank_sum_test(char **files,int *which,double confidence,double *real_confidence);
void      ranks_from_minimum_spanning_tree(Graphptr mst, matrix distancematrix);
matrix    read_multivariate_observations_for_file(char* filename);
int       read_multivariate_observations_for_multiple_file(char** files, matrix observations[], int L);
int       read_multivariate_observations_for_two_file(char** files, matrix observations[2]);
Obsptr*   read_observations_for_multiple_equal_size_file(char **files, int* size, int* count);
Obs*      read_observations_for_multiple_file(char** files, int** counts, int* size, int* filecount);
double**  read_observations_for_two_file(char** files, int* counts);
int       regressionftest(char **files,int *which,double confidence,double *real_confidence);
void      save_mst_to_array_variables(Graphptr mst, matrix observations[], int L);
int       scheffe(char **files,int *which,double confidence,double *real_confidence);
int       signed_rank_test(char **files, int *which, double confidence, double *real_confidence);
int       signtest(char **files, int *which, double confidence, double *real_confidence);
int       smirnov(char **files, int *which, double confidence, double *real_confidence);
int       ttest(char **files,int *which,double confidence,double *real_confidence);
int       ttest5x2(char **files,int *which,double confidence,double *real_confidence);
int       ttestpaired(char **files,int *which,double confidence,double *real_confidence);
int       tukey(char **files,int *which,double confidence,double *real_confidence);
double    vanderwaerdan(char **files);
int       waldwolfowitz(char **files, int *which, double confidence, double *real_confidence);

#endif
