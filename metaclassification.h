#ifndef metaclassification_h
#define metaclassification_h

#include "typedef.h"

/*! Algorithm structure for each supervised machine learning meta-algorithm*/
struct meta_classification_algorithm{
 /*! Training function of the supervised machine learning meta-algorithm.*/
 void** (*train_algorithm)(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters);
 /*! Testing function of the machine learning algorithm. Testing functions for classification algorithms are in the classification.c file, training functions for regression algorithms are in the regression.c file*/
 Prediction (*test_algorithm)(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior);
 /*! Name of the algorithm. Mainly used in test code generation */
 char* name;
};

typedef struct meta_classification_algorithm Meta_Classification_Algorithm;

Classificationresultptr ensemble(matrixptr* posteriors, int learner_count, int fold_count);
void                    free_model_of_meta_classification_algorithm(int meta_algorithm, int algorithm, void** model, Datasetptr current_dataset);
Prediction              test_bagging_classification(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior);
Prediction              test_one_vs_one(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior);
Prediction              test_one_vs_rest(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior);
void**                  train_bagging_classification(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters);
void**                  train_one_vs_one(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters);
void**                  train_one_vs_rest(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters);

#endif
