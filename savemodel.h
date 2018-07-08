#ifndef __savemodel_H
#define __savemodel_H

#include "classification.h"
#include "hmm.h"
#include "mlp.h"
#include "rbf.h"
#include "regression.h"
#include "typedef.h"

void save_array(FILE* modelfile, double* array, int size);
void save_array_int(FILE* modelfile, int* array, int size);
void save_array_support_vector(FILE* modelfile, Datasetptr d, Svm_nodeptr* array, int size);
void save_array_two(FILE* modelfile, double** array, int size1, int size2);
void save_condition(FILE* modelfile, Decisionconditionptr condition);
void save_instance(FILE* modelfile, Datasetptr d, Instanceptr inst, int class_saved);
void save_instance_2d(FILE* modelfile, Datasetptr d, Instanceptr inst, int class_saved);
void save_instance_discrete(FILE* modelfile, Datasetptr d, Instanceptr inst);
void save_matrix(FILE* modelfile, matrix m);
void save_mean_and_variance(FILE* modelfile, Datasetptr d, Instance mean, Instance variance, int mustrealize);
void save_model_c45(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level);
void save_model_divs(FILE* modelfile, Datasetptr d, Model_divsptr m);
void save_model_gaussian(FILE* modelfile, Datasetptr d, Model_gaussianptr m);
void save_model_gprocessreg(FILE* modelfile, Datasetptr d, Model_gprocessregressionptr m);
void save_model_hmm(FILE* modelfile, Datasetptr d, Hmmptr* hmmarray);
void save_model_knn(FILE* modelfile, Datasetptr d, Instanceptr m);
void save_model_ldaclass(FILE* modelfile, Datasetptr d, Model_ldaclassptr m);
void save_model_linearreg(FILE* modelfile, Datasetptr d, Model_linearregressionptr m);
void save_model_logistic(FILE* modelfile, Datasetptr d, double** m);
void save_model_mlpgeneric(FILE* modelfile, Datasetptr d, Mlpnetworkptr m);
void save_model_multildt(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level);
void save_model_naivebayes(FILE* modelfile, Datasetptr d, Model_naivebayesptr m);
void save_model_nearestmean(FILE* modelfile, Datasetptr d, Instanceptr m);
void save_model_nodeboundedtree(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level);
void save_model_onefeature(FILE* modelfile, Datasetptr d, Model_onefeatureptr m);
void save_model_qdaclass(FILE* modelfile, Datasetptr d, Model_qdaclassptr m);
void save_model_quantizereg(FILE* modelfile, Datasetptr d, Model_quantizeregptr m);
void save_model_randomforest(FILE* modelfile, Datasetptr d, Decisionnodeptr* m, int treecount);
void save_model_rbf(FILE* modelfile, Datasetptr d, Rbfnetworkptr m);
void save_model_regrule(FILE* modelfile, Datasetptr d, Regressionrulesetptr m);
void save_model_regtree(FILE* modelfile, Datasetptr d, Regressionnodeptr m, int level);
void save_model_ripper(FILE* modelfile, Datasetptr d, Rulesetptr m);
void save_model_rise(FILE* modelfile, Datasetptr d, Model_riseptr m);
void save_model_selectaverage(FILE* modelfile, Datasetptr d, double* m);
void save_model_selectmax(FILE* modelfile, Datasetptr d, Model_selectmaxptr m);
void save_model_softtree(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level);
void save_model_svm(FILE* modelfile, Datasetptr d, Svm_modelptr m);
void save_model_svmtree(FILE* modelfile, Datasetptr d, Decisionnodeptr* m, int treecount);
void save_rule(FILE* modelfile, Decisionruleptr rule, Datasetptr d);
void save_support_vector(FILE* modelfile, Datasetptr d, Svm_nodeptr sv);
void save_output_mean_and_variance(FILE* modelfile, double mean, double variance);

#endif
