#ifndef __regression_H
#define __regression_H
#include "typedef.h"
#include "matrix.h"

/*! Model learned in the ONEFEATURE regression algorithm. ONEFEATURE algorithm is the regression tree algorithm with one regression node.*/
struct model_onefeature{
                        /*! Feature selected at the root node*/
	                       int bestfeature;
                        /*! Split threshold selected at the root node*/
																								double bestsplit;
                        /*! Regression value assigned to the left leaf*/
																								double leftmean;
                        /*! Regression value assigned to the right leaf*/
																								double rightmean;
};

typedef struct model_onefeature Model_onefeature;
typedef Model_onefeature* Model_onefeatureptr;

/*! Model learned in the Gaussian process regression*/
struct model_gprocessregression{
                                /*! Number of instances*/
                                int size;
                                /*! Training data for kernel computation*/
                                Instanceptr data;
                                /*! Weight vector */
                                double* weights;
};

typedef struct model_gprocessregression Model_gprocessregression;
typedef Model_gprocessregression* Model_gprocessregressionptr;

/*! Model learned in the LINEAR REGRESSION algorithm*/
struct model_linearregression{
                              /*! Bias value of the linear regressor*/
	                             double b;
                              /*! Weight vector stored as a one row matrix. w[0][i] is the weight of feature i*/
																														matrix w;
};

typedef struct model_linearregression Model_linearregression;
typedef Model_linearregression* Model_linearregressionptr;

/*! Model learned in the QUANTIZED REGRESSION algorithm. In QUANTIZED REGRESSION, the dimension of the dataset is reduced into 2 using PCA. In the second phase, each dimension is divided into k parts to get k^2 rectangles (If there is one or two feature(s) in the dataset no PCA is done). For each rectangle, the regression value is the mean value of the outputs of the instances those fall into that rectangle */
struct model_quantizereg{
                         /*! The minimum value of the first dimension*/
	                        double minx;
                         /*! Length of each part for the first dimension*/
																									double binx;
                         /*! The minimum value of the second dimension*/
																									double miny;
                         /*! Length of each part for the second dimension*/
																									double biny;
                         /*! If there is a single feature in the dataset, one divides that dimension into k parts and get k different lines. For each line, the regression value is the mean value of the outputs of the instances those fall onto that line*/
																									double* meansone;
                         /*! The regression value of each rectangle*/
																									double** meanstwo;
                         /*! Eigenvectors used in PCA to reduce dimension*/
																									matrix eigenvectors;
                         /*! Number of parts (k)*/
																									int partitioncount;
};

typedef struct model_quantizereg Model_quantizereg;
typedef Model_quantizereg* Model_quantizeregptr;

void                free_performance_regression(Regressionresultptr result);
Regressionresultptr performance_initialize_regression(int datasize);
void                print_regression_results(Instanceptr testdata, Regressionresultptr result, double runtime, int printbinary, int displayruntime);
Prediction          test_dncreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_gprocessreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_knnreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_linearregression(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_mlpgenericreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_onefeature(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_quantizereg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_rbfreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_regressiontree(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_regrule(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_regstump(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_selectaverage(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_simplexsvmreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_softregressiontree(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_softregstump(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_svmreg(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction          test_svmregtree(Instanceptr data, void* model, void* parameters, double* posterior);
void*               train_dncreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_gprocessreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_knnreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_linearregression(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_mlpgenericreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_onefeature(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_quantizereg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_rbfreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_regressiontree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_regstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_regrule(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_selectaverage(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_simplexsvmreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_softregressiontree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_softregstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_svmreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*               train_svmregtree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);

#endif
