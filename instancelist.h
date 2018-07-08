#ifndef __instancelist_H
#define __instancelist_H

#include "stdio.h"
#include "typedef.h"
#include "partition.h"

Instanceptr    bootstrap_sample_without_stratification(Instanceptr list);
Instanceptr    copy_of_instancelist(Instanceptr list);
Instanceptr*   create_array_of_instances(Instanceptr instances);
int*           create_index_array_for_instances(Instanceptr instances, int *count);
Instanceptr    create_instancelist(int* indexes, int count, Instanceptr* list);
Instanceptr    create_instancelist_from_rules(Decisionconditionptr rules, int count, Instanceptr* list);
void           create_list_from_exceptions(Instanceptr* list, Instanceptr* list2);
int            data_size(Instanceptr myptr);
Instanceptr    data_x(Instanceptr myptr,int index);
void           divide_instancelist(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, int mod);
void           divide_instancelist_according_centers(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, Instanceptr leftcenter, Instanceptr rightcenter, int quadratic);
void           divide_instancelist_according_partition(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, Partition p);
void           divide_instancelist_according_to_soft_split(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, matrix w);
void           divide_instancelist_according_to_split(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, int fno, double split);
Instanceptr*   divide_instancelist_into_classes(Instanceptr* parentlist);
Instanceptr*   divide_instancelist_into_parts(Instanceptr* parentlist, int partcount);
void           divide_instancelist_one_vs_rest(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, int positive);
void           divide_instancelist_one_vs_rest_with_copy(Instanceptr parentlist, Instanceptr* list, int positive);
void           divide_instancelist_one_vs_one(Instanceptr* parentlist, Instanceptr* list, int positive, int negative);
void           divide_instancelist_one_vs_one_with_copy(Instanceptr parentlist, Instanceptr* list, int positive, int negative);
Instanceptr*   find_all_neighbors(Instanceptr list, Instanceptr item, int* count);
double         find_k_nearest_neighbor_distance(Instanceptr list, Instanceptr item, int count);
double         find_mean_of_regression_value(Instanceptr instances);
Instanceptr*   find_nearest_neighbors(Instanceptr list, Instanceptr item, int count);
Instanceptr*   find_nearest_neighbors_before_normalize(Instanceptr list, Instanceptr item, int count, Instance mean, Instance variance);
double         find_variance_of_regression_value(Instanceptr instances, double mean);
Instanceptr    generate_2d_copy_of_instance_list(Instanceptr list);
int*           get_classes(Instanceptr instances, int *count);
matrix         instancelist_to_matrix(Instanceptr list, int problemtype, int positive, MULTIVARIATE_TYPE multivariate_type);
Instanceptr    last_element_of_list(Instanceptr list);
void           merge_instancelist(Instanceptr* list1, Instanceptr list2);
void           merge_instancelist_groups(Instanceptr* mergedlist, Instanceptr* lists, int count);
void           merge_instancelist_without_outliers(Instanceptr* list1, Instanceptr* list2, int classno);
void           print_class_information(FILE* outfile, Instanceptr listptr);
void           print_instance_list(FILE* outfile, char s, Datasetptr d, Instanceptr listptr);
void           print_instance_list_with_extra_features(FILE* outfile, char s, Datasetptr d, Instanceptr listptr, int extra_features);
void           print_kernel_matrix(FILE* outfile, Instanceptr listptr);
Instanceptr    restore_instancelist(Instanceptr mainlist,Instanceptr sublist);
void           reverse_instancelist(Instanceptr* list);

#endif

