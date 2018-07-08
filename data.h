#ifndef __data_H
#define __data_H

enum{
     LOWER = 0,
     UPPER};

#include "typedef.h"
#include "matrix.h"

int              all_in_one_class(Instanceptr myptr, int* maxcount);
int***           allocate_feature_counts();
matrix           between_class_matrix(Instanceptr myptr);
matrix           class_covariance(Instanceptr myptr, int classno, Instanceptr classmean);
BOOLEAN          class_exists_in_list(Instanceptr myptr, int classno);
int              count_exceptions(Instanceptr list);
matrix           covariance(Instanceptr myptr, Instance currentmean);
matrix           covariance_2d(Instanceptr myptr, matrix currentmean);
void             dimension_reduce_with_pca(Instanceptr inst, int howmany, matrix eigenvect);
matrix           distance_matrix(Instanceptr myptr);
matrix           distance_matrix_with_k(Instanceptr myptr, int k);
matrix           distance_matrix_with_threshold(Instanceptr myptr, double epsilon);
Instanceptr      empty_link_list_of_instances(int fcount, int count);
double           exception_performance(Instanceptr list);
void             find_bounds_of_feature(Datasetptr d, Instanceptr myptr, int fno, double* min, double* max);
void			          find_bounds_of_feature_class(Instanceptr myptr, int fno, int cno, double* min, double* max);/*oya*/
int              find_error_as_leaf(Instanceptr myptr);
int***           find_feature_counts(Instanceptr myptr);
int              find_class_count(Instanceptr myptr, int classno);
int*             find_class_counts(Instanceptr myptr);
Instanceptr      find_class_means(Instanceptr myptr,int *counts);
double*          find_class_variance(Instanceptr myptr, Instanceptr classmeans, int *counts);
int*             find_classes(Instanceptr myptr);
Instanceptr      find_linear_interpolation_data(Instanceptr data1, Instanceptr data2, double param);/*oya*/
Instance         find_mean(Instanceptr myptr);
matrix           find_mean_2d(Instanceptr myptr);
Instance         find_mean_with_missing(Instanceptr myptr);
Instance         find_variance(Instanceptr myptr);
int			           find_thresholded_class_count(Instanceptr myptr, int classno, int fno, double fval, int dir);/*oya*/
void             free_feature_counts(int*** feature_counts);
void             normalize(Instanceptr myptr, Instance mean, Instance variance);
void             normalize_instance(Instanceptr myptr, Instance mean, Instance variance);
void             normalize_regression_values(Instanceptr myptr, double mean, double variance);
void             realize(Instanceptr myptr);
void             realize_instance(Instanceptr myptr);
void             unnormalize_instance(Instanceptr myptr, Instance mean, Instance variance);
matrix           within_class_matrix(Instanceptr myptr);

#endif
