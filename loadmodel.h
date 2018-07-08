#ifndef __loadmodel_H
#define __loadmodel_H

#include "classification.h"
#include "regression.h"
#include "typedef.h"

double*            load_array(FILE* infile, int size);
int*               load_array_int(FILE* infile, int size);
Svm_nodeptr*       load_array_support_vector(FILE* infile, Datasetptr d, int size);
double**           load_array_two(FILE* infile, int size1, int size2);
Decisionconditionptr load_condition(FILE* infile, Datasetptr d);
Instance    load_instance(FILE* infile, Datasetptr d, int class_loaded);
Instance    load_instance_discrete(FILE* infile, Datasetptr d);
matrix             load_matrix(FILE* infile, int size1, int size2);
void               load_model_c45(FILE* infile, Datasetptr d, Decisionnodeptr rootnode);
void*              load_model_divs(FILE* infile, Datasetptr d);
void*              load_model_gaussian(FILE* infile, Datasetptr d);
void*              load_model_gprocessreg(FILE* infile, Datasetptr d);
void*              load_model_hmm(FILE* infile, Datasetptr d);
void*              load_model_knn(FILE* infile, Datasetptr d);
void*              load_model_ldaclass(FILE* infile, Datasetptr d);
void*              load_model_linearreg(FILE* infile, Datasetptr d);
void*              load_model_logistic(FILE* infile, Datasetptr d);
void*              load_model_mlpgeneric(FILE* infile, Datasetptr d);
void*              load_model_multildt(FILE* infile, Datasetptr d);
void*              load_model_naivebayes(FILE* infile, Datasetptr d);
void*              load_model_nearestmean(FILE* infile, Datasetptr d);
void*              load_model_nodeboundedtree(FILE* infile, Datasetptr d);
void*              load_model_onefeature(FILE* infile, Datasetptr d);
void*              load_model_qdaclass(FILE* infile, Datasetptr d);
void*              load_model_quantizereg(FILE* infile, Datasetptr d);
void               load_model_randomforest(FILE* infile, Datasetptr d, Decisionnodeptr* forest, int treecount);
void*              load_model_rbf(FILE* infile, Datasetptr d);
void*              load_model_regrule(FILE* infile, Datasetptr d);
void               load_model_regtree(FILE* infile, Datasetptr, Regressionnodeptr rootnode);
void*              load_model_ripper(FILE* infile, Datasetptr d);
void*              load_model_rise(FILE* infile, Datasetptr d);
void*              load_model_selectaverage(FILE* infile, Datasetptr d);
void*              load_model_selectmax(FILE* infile, Datasetptr d);
void               load_model_softtree(FILE* infile, Datasetptr d, Decisionnodeptr rootnode);
void*              load_model_svm(FILE* infile, Datasetptr d);
void               load_model_svmtree(FILE* infile, Datasetptr d, Decisionnodeptr* forest, int treecount);
Decisionruleptr      load_rule(FILE* infile, Datasetptr d);
Svm_nodeptr        load_support_vector(FILE* infile, Datasetptr d);
void               load_output_mean_and_variance(FILE* infile, double *mean, double* variance);

#endif
