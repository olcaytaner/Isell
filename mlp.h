#ifndef __mlp_h
#define __mlp_h
#include "typedef.h"

enum initialization_type{ 
      RANDOM = 0,
      INITIAL_VALUE,
      NO_INITIALIZATION};

typedef enum initialization_type INITIALIZATION_TYPE;

#define MAX_LAYER 3
#define MAX_HIDDEN 50
#define CHANGE_PERCENTAGE 2
#define MAX_MLP_SIMULATION_ALGORITHMS 5
#define MAX_ERROR 100000000000000000000000000000000000000000000000000.0

/*! Structure storing the parameters of the Multilayer perceptron (MLP) and dynamic node creation (DNC) algorithms*/
struct mlpparameters{
 /*! Number of classes in the problem*/
 int classnum;
 /*! Number of features in the dataset*/
 int dimnum;
 /*! Number of input instances fed to the MLP (trained or tested). If training, inputnum = trnum, if testing, inputnum = number of test instances*/
 int inputnum;
 /*! Number of validation instances*/
 int cvnum;
 /*! Number of training instances*/
 int trnum;
 /*! Number of epochs to train MLP */
 int epochs;
 /*! Best epoch index for which the error is minimum on the validation set */
 int bestepoch;
 /*! Initial learning rate value */
 double initialeta;
 /*! Random initialization range of the weights. For example if randrange is +0.01, the weights are initialized values between -0.01 and +0.01*/
 double randrange; 
 /*! Momentum weight*/
 double alpha;
 /*! Current learning rate of the algorithm*/
 double eta;
 /*! Decrease in the learning rate. This value is multiplied with current learning rate after each epoch*/
 double etadecrease;
 /*! Number of hidden units at each layer*/
 int hidden[MAX_LAYER];/*0th index is for 1st layer, 1st for 2nd layer...*/
 /*! Number of hidden layers*/
 int layernum;/*number of hidden layers*/
 /*! Do we apply weight decay? Has two possible values YES for applying weight decay NO for otherwise */
 int weightdecay;
 /*! Decay weight, if weight decay is applied */
 double decayalpha;
 /*! Size of the window in the DNC algorithm. */
 int windowsize;
 /*! Error threshold in the DNC algorithm */
 double errorthreshold;
 /*! Error drop ratio in the DNC algorithm */
 double errordropratio;
 /*! Number of epochs trained. In the MLP algorithm this number is fixed, but in the DNC algorithm the learner may exit before the maximum epoch number is reached*/
 int epochstrained;
 /*! Number of maximum hidden units. Used in the DNC algorithm*/
 int maxhidden;
 /*! Total squared error */
 double sqrderror;
};
typedef struct mlpparameters Mlpparameters;
typedef struct mlpparameters* Mlpparametersptr;

/*! Multilayer Neural Network structure for n hidden layer network*/
struct mlpnetwork{ 
 /*! Weights of the Neural Network. Stored as an array of weights. weights[0] is the weights between the input units and the first hidden layer, weights[1] is the weights between the first hidden layer and the second layer etc. weights[layernum - 1] is the weights between the last hidden layer (input units if NN is a linear perceptron) and the output units. */
 matrixptr weights;
 /*! Runtime parameters of the Neural Network */
 Mlpparameters P;
};

typedef struct mlpnetwork Mlpnetwork;
typedef struct mlpnetwork* Mlpnetworkptr;

matrixptr      allocate_hiddens(Mlpparameters P);
Mlpnetwork     allocate_mlpnnetwork(Mlpparameters P);
matrixptr      allocate_weights(Mlpparameters P, INITIALIZATION_TYPE inittype, int initvalue);
double         backPropMlpn_autoencoder(double *Train, matrixptr weights, matrixptr hidden, matrixptr prevWeights, Mlpparameters P);
double         backPropMlpn_cls(double *Train, matrixptr weights, matrixptr hidden, matrixptr prevWeights, Mlpparameters P);
double         backPropMlpn_reg(double *Train, matrixptr weights, matrixptr hidden, matrixptr prevWeights, Mlpparameters P);
void           calculate_change_in_weights(double *inputdata, matrixptr weights, matrixptr hiddens, matrixptr dweights, matrix output_error, Mlpparameters P);
double         calculate_error_values_autoencoder(double* outputdata, matrix output, matrixptr output_error);
double         calculate_error_values_cls(double outputclass, matrix output, matrixptr output_error);
double         calculate_error_values_reg(double outputclass, matrix output, matrixptr output_error);
void           calculate_hidden_values(double *inputdata, matrix weight, matrixptr hidden);
void           calculate_output_values_cls(double *inputdata, matrix weight, matrixptr output);
void           calculate_output_values_reg(double *inputdata, matrix weight, matrixptr output);
Mlpnetwork     copy_of_Mlpnetwork(Mlpnetwork source);
Mlpnetwork     dnc_cls(matrix Train, Mlpparametersptr P);
Mlpnetwork     dnc_reg(matrix Train, Mlpparametersptr P);
void           free_hiddens(matrixptr hiddens, int layer);
void           free_mlpnnetwork(Mlpnetworkptr nN);
void           free_weights(matrixptr weights, int layer);
matrix         hessian_matrix_cls(Mlpnetwork N, Instanceptr train);
matrix         hessian_matrix_reg(Mlpnetwork N, Instanceptr train);
double         log_likelihood_of_mlp_network(Mlpnetwork neuralnetwork, matrix test);
Mlpnetwork     mlp_network_train_general(matrix train, matrix cv, Mlpparametersptr mlpparams);
Mlpnetwork     mlpn_general(matrix Train, matrix Validation, Mlpparametersptr P, SUPERVISED_ALGORITHM_TYPE algorithm_type);
int            number_of_weights_in_mlp_network(Mlpnetwork N);
Mlpparameters  prepare_Mlpparameters(Instanceptr data, Instanceptr validation, int weightdecay, double decayalpha, int layernum, int* hiddennodes, int windowsize, double errorthreshold, double errordropratio, int maxhidden, SUPERVISED_ALGORITHM_TYPE algorithm_type);
void           reallocate_hiddens(matrixptr hiddens, Mlpparameters P, int hiddenindex, int count);
void           reallocate_weights(Mlpparameters P, matrixptr weights, int hiddenindex, int count, INITIALIZATION_TYPE inittype, int initvalue);
void           softmax_with_calculation_of_outputs(double *inputdata, matrix weights, matrixptr Y);
double         test_mlp_network(Mlpnetwork N, matrix test);
double         testMlpnInput_autoencoder(double *Test, matrix* testY, Mlpnetwork N, matrixptr output_error);
double         testMlpnInput_cls(double *Test, matrix* testY, Mlpnetwork N, int* correctlyclassified, double* prob);
double         testMlpnInput_reg(double *Test, matrix* testY, Mlpnetwork N, matrixptr output_error);
double         testMlpnNetwork_autoencoder(matrix Data, Mlpnetwork N);
int            testMlpnNetwork_cls(matrix Data, Mlpnetwork N, double* loglikelihood);
int            testMlpnNetwork2_cls(matrix Data, Mlpnetwork N, double* loglikelihood, double *sqrdError);
double         testMlpnNetwork_reg(matrix Data, Mlpnetwork N);
void           update_weights(matrixptr weights, matrixptr dweights, matrixptr prevWeights, Mlpparameters P); 

#endif
