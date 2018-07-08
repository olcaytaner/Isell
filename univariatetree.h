#ifndef __univariatetree_H
#define __univariatetree_H
#include "parameter.h"
#include "partition.h"
#include "typedef.h"

int               bucketsort_instances(Instanceptr instances, int size, int fno);
int**             count_feature_combinations(Instanceptr instances, int* iter, int size, int* permsize);
int               create_c45children(Decisionnodeptr node, C45_parameterptr p);
int               create_ldtchildren(Decisionnodeptr node, C45_parameterptr p);
Decisioncondition createcondition(int featureindex, char comparison, double value);
int               decisiontree_univariate_node_count(Decisionnodeptr node);
void              exchange_instances(Instanceptr instances,int i,int j);
int               find_best_feature(Instanceptr instances,double* split);
int               find_best_ldt_feature(Instanceptr instances,double* split);
int               find_best_ldt_feature_for_realized(Instanceptr instances, double* split);
void              find_counts_for_split(Instanceptr instances, int fno, double split, int* leftcounts, int* rightcounts, int conditiontype);
int               find_best_split_for_feature_i(Instanceptr instances, int currentfeature, double* split, double* bestgain);
int               find_discrete_split_count(int size);
double            find_gain_for_partition(Instanceptr instances, Partition p, int fno, double* split, int conditiontype);
void              find_occurences_for_discrete_feature(Instanceptr instances, int fno, int ratio[MAXVALUES][MAXCLASSES]);
double            find_split_for_partition(Instanceptr instances, Partition p, int fno, int conditiontype);
double            find_split_for_partition_without_outliers(Instanceptr instances, Partition p, int fno, int conditiontype, double variancerange);
double*           find_value_list(Instanceptr instances,int fno,int* size);
double            information_gain_for_discrete_feature(Instanceptr instances,int fno);
double            information_gain_for_split(Instanceptr instances, int fno, double split, int conditiontype);
void              left_and_right_sum(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, int* leftcount, int* rightcount, int conditiontype);
void              left_and_right_sum_without_outliers(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, int* leftcount, int* rightcount, double llower, double lupper, double rlower, double rupper);
void              left_and_right_variance(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, double leftmean, double rightmean, int conditiontype);
void              left_and_right_variance_without_outliers(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, double leftmean, double rightmean, double llower, double lupper, double rlower, double rupper);
double            log_likelihood_for_uni_ldt_splits(Instanceptr data, int fno, double split, int conditiontype);
int               make_children(Decisionnodeptr node);
int               quicksort_all_sorted(Instanceptr instances, int start, int end, int fno);
void              quicksort_instances(Instanceptr instances,int start,int end,int fno);
int               quicksort_partition(Instanceptr instances,int start,int end,int fno);
Instance          quicksort_three_elements(Instanceptr instances, int start, int end, int fno);
double            solve_for_ldt_second_order_equation(double ml, double mr, double sl, double sr, int nl, int nr);
int               sort_function_instances(Instanceptr x, Instanceptr y, int fno);
Instanceptr       sort_instances(Instanceptr instances,int fno,int* size);
int               test_univariate(Instanceptr testingdata, int fno, double split, int leftclass, int rightclass);

#ifdef MPI
MPI_Datatype* create_datatype_for_rule(Decisionconditionptr rule);
double      information_gain_for_split_mpi(double split);
double      find_gain_for_partition_mpi(int* indexlist, int count, Partition p, int fno, double* split);
void        get_information_from_clients(int procend,double* bestgain, int* bestfeature, double* split);
void        find_best_split_for_features_between_i_and_j(Instanceptr instances,int fnostart, int fnoend);
int         find_best_ldt_feature_mpi(Instanceptr instances,double* split);
int         find_best_ldt_feature_mpi_rulebased(Decisionnodeptr node);
int         divide_data_to_find_best_feature(Instanceptr instances,double* split);
void        send_node_to_processors(Decisionnodeptr stack[], int i, int procno);
void        receive_update_children(Decisionnodeptr node, Decisionnodeptr children[], int procno, int* childcount);
int         create_ldtchildren_iterative(Decisionnodeptr root);
void        send_partition_info_to_clients(int* indexlist, int count, int fno, Partition p);
double      find_gain_for_discrete_feature(int* indexlist, int count, int fno, Partition p);
#endif

#endif
