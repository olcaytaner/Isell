#ifndef __instance_H
#define __instance_H

#include "stdio.h"
#include "typedef.h"
#include "matrix.h"
#include "svm.h"

Instanceptr    copy_of_instance(Instanceptr inst);
void           deallocate(Instanceptr myptr);
double         distance_between_instances(Instanceptr inst1, Instanceptr inst2);
double         distance_between_instances_before_normalize(Instanceptr inst1,Instanceptr inst2, Instance mean, Instance variance);
double         dot_product(Instanceptr inst1, Instanceptr inst2);
int            frame_count(Instanceptr instance);
Instanceptr    generate_2d_copy_of_instance(Instanceptr inst);
int            give_classno(Instanceptr myptr);
double         give_regressionvalue(Instanceptr myptr);
matrix         instance_to_matrix(Instanceptr inst);
Svm_nodeptr    instance_to_svmnode(Instanceptr inst);
double         multiply_with_matrix(Instanceptr inst, matrix m);
void           print_instance(FILE* outfile, char s, Datasetptr d, Instanceptr currentinstance);
void           print_instance_with_extra_features(FILE* outfile, char s, Datasetptr d, Instanceptr currentinstance, int extra_features);
double         real_feature(Instanceptr myptr,int fno);
int*           stratified_partition(int *prior_class_information, int datasize, int classnumber, int* classcounts);
int            svm_classno(Instanceptr myptr, int negative);

#endif
