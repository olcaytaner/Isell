#ifndef __multivariatetree_H
#define __multivariatetree_H

int               best_multivariate_split_lda_with_pca(Instanceptr positives, Instanceptr negatives, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained);
int               best_multivariate_split_lda_with_svd(Instanceptr positives, Instanceptr negatives, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained);
int               best_multivariate_split_lp(Instanceptr positives, Instanceptr negatives, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim);
int               create_multildtchildren(Decisionnodeptr node, Multildt_parameterptr param);
Decisioncondition createmultivariatecondition(matrix w, double w0, char comparison, int node_type);
void              deallocate_multivariate_conditions(Decisionnodeptr node);
int               decisiontree_multivariate_linear_node_count(Decisionnodeptr node);
int               decisiontree_multivariate_quadratic_node_count(Decisionnodeptr node);
int               find_best_multi_ldt_split(Instanceptr* instances, matrix* bestw, double *bestw0, MULTIVARIATE_TYPE multivariate_type, double variance_explained);
void              find_counts_for_multivariate_split(Instanceptr instances, matrix w, double w0, int* leftcounts, int* rightcounts);
double            find_gain_for_partition_multivariate(Instanceptr* instances, Partition p, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained);
int               find_split_for_partition_multivariate(Instanceptr* instances, Partition p, matrix* w, double*  w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained);
double            information_gain_for_multivariate_split(Instanceptr instances, matrix w, double w0);
double            log_likelihood_for_multi_ldt_splits(Instanceptr data, matrix w, double w0);
int               make_multivariate_children(Decisionnodeptr node);
int               test_multivariate(Instanceptr testingdata, matrix w, double w0, int leftclass, int rightclass);

#endif
