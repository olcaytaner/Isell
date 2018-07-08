#ifndef __rbf_H
#define __rbf_H

#include "matrix.h"

/*! Structure storing the weights of the Radial Basis Function network*/
struct rbfweights{
                  /*! Mean vectors of the Gaussians (Radials)*/
                  matrix m; /*hj hidden, input*/
                  /*! Standard deviations of the Gaussians (Radials)*/
                  matrix s; /*1h hidden*/
                  /*! Weights between the hidden units (Gaussians) and the output units*/
                  matrix w; /*ih (hidden + 1), output*/
};

typedef struct rbfweights Rbfweights;

/*! Structure storing the Radial Basis Function network*/
struct rbfnetwork{
                  /*! Number of Gaussians (Radials) */
																		int hidden;
                  /*! Number of output units (number of classes for CLASSIFICATION problem, 1 for REGRESSION problem) */
																		int output;
                  /*! Number of input units (Number of features in the dataset + 1) */
																		int input;
                  /*! Weights of the RBF network*/
                  Rbfweights weights;
};

typedef struct rbfnetwork Rbfnetwork;
typedef Rbfnetwork* Rbfnetworkptr;

/*! Structure storing the parameters of the Radial Basis Function Network algorithms*/
struct rbf_parameter{
                     /*! Number of output units (number of classes for CLASSIFICATION problem, 1 for REGRESSION problem) */
	                    int output;
                     /*! Number of Gaussians (Radials) */
	                    int hidden;
                     /*! Number of input units (Number of features in the dataset + 1) */
	                    int input;
	                    /*! Number of epochs to train RBF */
                     int epochs;
	                    /*! Best epoch index for which the error is minimum on the validation set */
                     int bestepoch;
	                    /*! Initial learning rate value */
                     double initialeta;
	                    /*! Random initialization range of the weights. For example if randrange is +0.01, the weights are initialized values between -0.01 and +0.01*/
                     double randrange;
	                    /*! Current learning rate of the algorithm*/
                     double eta;
	                    /*! Decrease in the learning rate. This value is multiplied with current learning rate after each epoch*/
                     double etadecrease;
                     /*! Momentum weight*/
                     double alpha;
};

typedef struct rbf_parameter Rbf_parameter;
typedef Rbf_parameter* Rbf_parameterptr;

Rbfnetworkptr allocate_rbfnetwork(Rbf_parameterptr params);
void          allocate_rbfweights(Rbfweights* weights, Rbf_parameterptr params);
Rbfnetworkptr copy_of_rbfnetwork(Rbfnetworkptr network, Rbf_parameterptr params);
void          copy_rbfweights(Rbfweights* dest, Rbfweights source);
double        distance_to_rbf_gaussian(Instanceptr inst, int inputdim, double* mh);
void          free_rbfnetwork(Rbfnetworkptr network);
void          free_rbfweights(Rbfweights weights);
void          initialize_rbfnetwork(Rbfnetworkptr network, Instanceptr data, Rbf_parameterptr params);
Rbfweights    rbf_backward_propagation(Rbfnetworkptr network, Instanceptr inst, double* p, double* y, Rbf_parameterptr params);
void          rbf_forward_propagation(Rbfnetworkptr network, Instanceptr inst, double* p, double* y);
double        real_actual_difference(int output, Instanceptr inst, int classno, double* y);
Rbfnetworkptr train_rbfnetwork_cls(Instanceptr traindata, Instanceptr cvdata, Rbf_parameterptr params);
Rbfnetworkptr train_rbfnetwork_reg(Instanceptr traindata, Instanceptr cvdata, Rbf_parameterptr params);
int           test_rbfnetwork_cls(Rbfnetworkptr network, Instanceptr data);
double        test_rbfnetwork_reg(Rbfnetworkptr network, Instanceptr data);
void          update_rbfweights(Rbfnetworkptr network, Rbfweights delta, Rbfweights prevweights, Rbf_parameterptr params);

#endif
