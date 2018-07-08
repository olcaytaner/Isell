#ifndef metaregression_h
#define metaregression_h

#include "typedef.h"

/*! Algorithm structure for each supervised machine learning meta-algorithm*/
struct meta_regression_algorithm{
 void** (*train_algorithm)(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters);
 Prediction (*test_algorithm)(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior);
 char* name;
};

typedef struct meta_regression_algorithm Meta_Regression_Algorithm;

Prediction test_bagging_regression(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior);
void**     train_bagging_regression(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters);

#endif
