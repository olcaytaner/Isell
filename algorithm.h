#ifndef __algorithm_H
#define __algorithm_H

#include "classification.h"
#include "regression.h"
#include "typedef.h"

/*! Algorithm structure for each supervised machine learning algorithm*/
struct algorithm{
                 /*! Training function of the machine learning algorithm. Training functions for classification algorithms are in the classification.c file, training functions for regression algorithms are in the regression.c file*/
                 void* (*train_algorithm)(Instanceptr* trdata, Instanceptr* cvdata, void* parameters); 
                 /*! Testing function of the machine learning algorithm. Testing functions for classification algorithms are in the classification.c file, training functions for regression algorithms are in the regression.c file*/
                 Prediction (*test_algorithm)(Instanceptr data, void* model, void* parameters, double* posterior);
                 /*! Name of the algorithm. Mainly used in test code generation */
                 char* name;
                 /*! Do we need conversion from discrete features to numeric features in this machine learning algorithm? Values: YES or NO*/
                 int mustrealize;
                 /*! Do we need normalize features in this machine learning algorithm? Values: YES or NO*/
                 int mustnormalize;
                 /*! Do we need validation data in this machine learning algorithm? Values: YES or NO*/
                 int cvdata_needed;
};

typedef struct algorithm Algorithm;

int                       check_supervised_algorithm_type(int algorithm, Datasetptr current_dataset);
int                       complexity_of_supervised_algorithm(int algorithm, void* model, void* parameters, Datasetptr current_dataset);
YES_NO                    cvdata_needed(int algorithm);
void                      display_output(int algorithm, void* model);
void                      free_supervised_algorithm_parameters(int algorithm, void* parameters);
void                      free_model_of_supervised_algorithm(int algorithm, void* model, Datasetptr current_dataset);
void                      generate_test_code(FILE* testcodefile, int algorithm, void* model, void* parameters, int language, Datasetptr current_dataset);
char*                     get_algorithm_name(int algorithm);
SUPERVISED_ALGORITHM_TYPE get_algorithm_type(int algorithm);
int                       get_supervised_algorithm_index(char* algname);
int                       is_cvdata_needed(int algorithm);
YES_NO                    must_normalize(int algorithm);
YES_NO                    must_realize(int algorithm);
void*                     prepare_supervised_algorithm_parameters(int algorithm);
void*                     tune_hyperparameters_of_the_supervised_algorithm(int algorithm, Instanceptr* traindata, Instanceptr* cvdata, Instanceptr* tunedata, void* parameters);
#endif
