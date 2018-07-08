#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "classification.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "matrix.h"
#include "mlp.h"
#include "parameter.h"
#include "perceptron.h"
#include "statistics.h"

extern Datasetptr current_dataset;

/**
 * Creates a copy of a neural network structure. Allocates memory for the structure (weights, layers, etc.) and copies the weights and parameters to the destination network
 * @param[in] source Source neural network
 * @return Copy of the source
 */
Mlpnetwork copy_of_Mlpnetwork(Mlpnetwork source)
{
	/*!Last Changed 29.06.2005*/
 Mlpnetwork dest;
	int li;
	dest = allocate_mlpnnetwork(source.P);
	dest.P = source.P;
	for (li = 0; li < source.P.layernum + 1; li++)
		 matrix_identical(&(dest.weights[li]), source.weights[li]);
	return dest;
}

/**
 * There are two types of problems which MLP networks here solve: CLASSIFICATION and REGRESSION. For each of these problems there is a function to train the corresponding network. This function is a wrapper function and decides  which of those two functions to call according to the dataset in hand (whether it is a CLASSIFICATION or REGRESSION dataset).
 * @param[in] train Training data stored as a matrix
 * @param[in] cv Validation data stored as a matrix
 * @param[in] mlpparams Parameters for traninig such as number of epochs, learning rate, momentum, etc.
 * @return Trained neural network
 */
Mlpnetwork mlp_network_train_general(matrix train, matrix cv, Mlpparametersptr mlpparams)
{
	/*!Last Changed 26.06.2005*/
	mlpparams->inputnum = train.row;
	mlpparams->trnum = train.row;
	mlpparams->cvnum = cv.row;
 if (current_dataset->classno == 0)
  	return mlpn_general(train, cv, mlpparams, REGRESSION);
	else
  	return mlpn_general(train, cv, mlpparams, CLASSIFICATION);
}

/**
 * Given a neural network, calculates the loglikelihood of the testing data
 * @param[in] neuralnetwork Neural network structure
 * @param[in] test Test data
 * @return Total loglikelihood of the test data
 */
double log_likelihood_of_mlp_network(Mlpnetwork neuralnetwork, matrix test)
{
	/*!Last Changed 01.07.2005*/
	double loglikelihood;
	testMlpnNetwork_cls(test, neuralnetwork, &loglikelihood);
	return loglikelihood;
}

/**
 * Calls corresponding test functions according to the problem at hand (CLASSIFICATION and REGRESSION).
 * @param[in] neuralnetwork Neural network structure
 * @param[in] test Test data
 * @return For CLASSIFICATION problems, error in percentage, such as 12.79. For REGRESSION problems, mean squared error (MSE), that is total square error divided by the number of examples in the test set.
 */
double test_mlp_network(Mlpnetwork neuralnetwork, matrix test)
{
	/*!Last Changed 26.06.2005*/
	double loglikelihood;
	int performance;
	if (current_dataset->classno == 0)
	  return testMlpnNetwork_reg(test, neuralnetwork) / test.row;
	else
		{
	  performance = testMlpnNetwork_cls(test, neuralnetwork, &loglikelihood);
			return 100.0 * ((test.row - performance + 0.0) / test.row);
		}
}

/**
 * Generates mlpparams structure and fills it with the given parameters.
 * @param[in] data Training data
 * @param[in] validation Validation data
 * @param[in] weightdecay Will weight decay be applied? YES or NO.
 * @param[in] decayalpha If weight decay will be applied, this will be the alpha parameter of the weight decay.
 * @param[in] layernum Number of layers in the neural network
 * @param[in] hiddennodes Number of hidden nodes at each layer. Stored as an array. hiddennodes[i] is the number of hidden nodes at layer i.
 * @param[in] windowsize Size of the window in the Dynamic Node Creation algorithm.
 * @param[in] errorthreshold Error threshold parameter in the Dynamic Node Creation algorithm. If the error drops below this threshold, the program stops.
 * @param[in] errordropratio Error drop ratio parameter in the Dynamic Node Creation algorithm.
 * @param[in] maxhidden Maximum number of hidden units to be added in the Dynamic Node Creation algorithm
 * @return Parameters of the MLP algorithm stored as a structure
 */
Mlpparameters prepare_Mlpparameters(Instanceptr data, Instanceptr validation, int weightdecay, double decayalpha, int layernum, int* hiddennodes, int windowsize, double errorthreshold, double errordropratio, int maxhidden, SUPERVISED_ALGORITHM_TYPE algorithm_type)
{
 /*!Last Changed 16.04.2007 added dnc parameter preparation*/
	/*!Last Changed 25.06.2005*/
	int i;
	Mlpparameters mlpparams;
	mlpparams.alpha = get_parameter_f("alpha");
 switch (algorithm_type)
  {
   case CLASSIFICATION:mlpparams.classnum = current_dataset->classno;
                       break;
   case    AUTOENCODER:mlpparams.classnum = current_dataset->multifeaturecount - 1;
                       break;
   case     REGRESSION:mlpparams.classnum = 1;
                       break;
               default:mlpparams.classnum = 1;
                       break;
  }
	mlpparams.dimnum = current_dataset->multifeaturecount - 1;
	mlpparams.epochs = get_parameter_i("perceptronrun");
	mlpparams.eta = get_parameter_f("learning_rate");
	mlpparams.etadecrease = get_parameter_f("etadecrease");
	mlpparams.weightdecay = weightdecay;
	mlpparams.decayalpha = decayalpha;
 mlpparams.windowsize = windowsize;
 mlpparams.errorthreshold = errorthreshold;
 mlpparams.errordropratio = errordropratio;
	mlpparams.layernum = layernum;
	mlpparams.maxhidden = maxhidden;
 if (layernum == 1 && hiddennodes == NULL)
   mlpparams.hidden[0] = 1;
 else
	  for (i = 0; i < layernum; i++)
    	mlpparams.hidden[i] = hiddennodes[i];
	mlpparams.initialeta = get_parameter_f("learning_rate");
	mlpparams.inputnum = data_size(data);
	mlpparams.trnum = mlpparams.inputnum;
	mlpparams.cvnum = data_size(validation);
	mlpparams.randrange = 0.01;
	return mlpparams;
}

/**
 * Calculates the complexity of a neural network in terms of number of weights in the network.
 * @param[in] N Neural network structure
 * @return Total number of weights in the neural network
 */
int number_of_weights_in_mlp_network(Mlpnetwork N)
{
	/*!Last Changed 18.06.2005*/
	int result = 0, i;
	for (i = 0; i <= N.P.layernum; i++)
		 result += N.weights[i].row * N.weights[i].col;
	return result;
}

/**
 * Given the data, produces the Hessian matrix of a neural network structure for CLASSIFICATION problem. For the definition of the Hessian matrix look at Pattern Classification by Duda, Hart and Stork (Second edition) pages 226 and 608.
 * @param[in] N Neural network structure
 * @param[in] train Training data stored as a link list
 * @return Hessian matrix
 */
matrix hessian_matrix_cls(Mlpnetwork N, Instanceptr train)
{
	/*!Last Changed 18.06.2005*/
	int weightcount, firstlayer, k, h, hprime, i, iprime, index1, index2, classno;
	double mul, yk, zh, sum1, sum2, zhprime, xi, xiprime, vkh, vkhprime;
	Instanceptr tmp;
	matrix result, testY, testH, inst;
	weightcount = number_of_weights_in_mlp_network(N);
	result = matrix_alloc(weightcount, weightcount);
	testY = matrix_alloc(1, N.P.classnum);
	testH = matrix_alloc(1, N.P.hidden[0]);
	firstlayer = N.weights[0].row * N.weights[0].col;
	tmp = train;
	while (tmp)
	 {
		 classno = give_classno(tmp);
		 inst = instance_to_matrix(tmp);
  	calculate_hidden_values(inst.values[0], N.weights[0], &testH);
		 calculate_output_values_cls(testH.values[0], N.weights[1], &testY);
			/*Both in the second layer*/
   for (k = 0; k < N.weights[1].row; k++)
			 {
					yk = testY.values[0][k];
				 for (h = 0; h < N.weights[1].col; h++)
					 {
						 zh = testH.values[0][h];
  					for (hprime = 0; hprime < N.weights[1].col; hprime++)
							 {
									zhprime = testH.values[0][hprime];
									index1 = firstlayer + k * N.weights[1].col + h;
									index2 = firstlayer + k * N.weights[1].col + hprime;
									result.values[index1][index2] += zh * zhprime * yk * (1 - yk);
							 }
					 }
			 }
   /*Both in the first layer*/
			index1 = 0;
			for (h = 0; h < N.weights[0].row; h++)
			 {
					zh = testH.values[0][h];
				 for (i = 0; i < N.weights[0].col; i++)
					 {
						 xi = inst.values[0][i];
						 index2 = 0;
						 for (hprime = 0; hprime < N.weights[0].row; hprime++)
							 {
								 zhprime = testH.values[0][hprime];
								 if (h == hprime)
									 {
           sum1 = 0;
											for (k = 0; k < N.weights[1].row; k++)
											 {
												 vkhprime = N.weights[1].values[k][hprime + 1];
													yk = testY.values[0][k];
												 if (k == classno)
														 sum1 += vkhprime * (yk - 1);
													else
														 sum1 += vkhprime * yk;
											 }
											sum1 = sum1 * (1 - 2 * zhprime);
									 }
									else
										 sum1 = 0;
									sum2 = 0;
         for (k = 0; k < N.weights[1].row; k++)
									 {
										 vkh = N.weights[1].values[k][h + 1];
											vkhprime = N.weights[1].values[k][hprime + 1];
											yk = testY.values[0][k];
											sum2 += vkh * vkhprime * yk * (1 - yk);
									 }
         sum2 = zh * (1 - zh) * sum2;
								 for (iprime = 0; iprime < N.weights[0].col; iprime++)
									 { 
										 xiprime = inst.values[0][iprime];
    							mul = xi * xiprime * zhprime * (1 - zhprime);
           result.values[index1][index2] += mul * (sum1 + sum2);
										 index2++;
									 }
							 }
						 index1++;
					 }
			 }
   /*One weight in each layer*/
			index1 = 0;
			for (h = 0; h < N.weights[0].row; h++)
			 {
				 zh = testH.values[0][h];
				 for (i = 0; i < N.weights[0].col; i++)
					 {
						 index2 = firstlayer;
							xi = inst.values[0][i];
							mul = xi * zh * (1 - zh);
						 for (k = 0; k < N.weights[1].row; k++)
							 {
								 yk = testY.values[0][k];
								 for (hprime = 0; hprime < N.weights[1].col; hprime++)
									 {
										 if (h == hprime - 1)
												 if (k == classno)
  												 sum1 = yk - 1;
													else
														 sum1 = yk;
											else
											  sum1 = 0;
											zhprime = testH.values[0][hprime];
											vkh = N.weights[1].values[k][h + 1];
											sum2 = zhprime * vkh * yk * (1 - yk);
           result.values[index1][index2] += mul * (sum1 + sum2);
											result.values[index2][index1] += mul * (sum1 + sum2);
										 index2++;
									 }
							 }
							index1++;
					 }
			 }
   matrix_free(inst);
		 tmp = tmp->next;
	 }
	matrix_free(testY);
	matrix_free(testH);
	return result;
}

/**
 * Given the data, produces the Hessian matrix of a neural network structure for REGRESSION problem. For the definition of the Hessian matrix look at Pattern Classification by Duda, Hart and Stork (Second edition) pages 226 and 608.
 * @param[in] N Neural network structure
 * @param[in] train Training data stored as a link list
 * @return Hessian matrix
 */
matrix hessian_matrix_reg(Mlpnetwork N, Instanceptr train)
{
	/*!Last Changed 18.06.2005*/
	int weightcount, firstlayer, h, hprime, i, iprime, index1, index2;
	double mul, yk, zh, sum1, sum2, zhprime, xi, xiprime, vkh, vkhprime, regvalue;
	Instanceptr tmp;
	matrix result, testY, testH, inst;
	weightcount = number_of_weights_in_mlp_network(N);
	result = matrix_alloc(weightcount, weightcount);
	testH = matrix_alloc(1, N.P.hidden[0]);
	testY = matrix_alloc(1, 1);
	firstlayer = N.weights[0].row * N.weights[0].col;
	tmp = train;
	while (tmp)
	 {
		 regvalue = give_regressionvalue(tmp);
   yk = testY.values[0][0];
		 inst = instance_to_matrix(tmp);
  	calculate_hidden_values(inst.values[0], N.weights[0], &testH);
		 calculate_output_values_reg(testH.values[0], N.weights[1], &testY);
			/*Both in the second layer*/
   for (h = 0; h < N.weights[1].col; h++)
				 for (hprime = 0; hprime < N.weights[1].col; hprime++)
					 {
							zh = testH.values[0][h];
							zhprime = testH.values[0][hprime];
							index1 = firstlayer + h;
							index2 = firstlayer + hprime;
							result.values[index1][index2] += zh * zhprime;
					 }
   /*Both in the first layer*/
			index1 = 0;
			for (h = 0; h < N.weights[0].row; h++)
			 {
					zh = testH.values[0][h];
     vkh = N.weights[1].values[0][h + 1];
				 for (i = 0; i < N.weights[0].col; i++)
					 {
						 xi = inst.values[0][i];
						 index2 = 0;
						 for (hprime = 0; hprime < N.weights[0].row; hprime++)
							 {
         vkhprime = N.weights[1].values[0][hprime + 1];
								 zhprime = testH.values[0][hprime];
								 if (h == hprime)
           sum1 = (yk - regvalue) * (1 - 2 * zhprime);
									else
										 sum1 = 0;
									sum2 = zh * (1 - zh) * vkh;
								 for (iprime = 0; iprime < N.weights[0].col; iprime++)
									 { 
										 xiprime = inst.values[0][iprime];
    							mul = xi * xiprime * zhprime * (1 - zhprime) * vkhprime;
           result.values[index1][index2] += mul * (sum1 + sum2);
										 index2++;
									 }
							 }
						 index1++;
					 }
			 }
   /*One weight in each layer*/
			index1 = 0;
			for (h = 0; h < N.weights[0].row; h++)
			 {
				 zh = testH.values[0][h];
				 for (i = 0; i < N.weights[0].col; i++)
					 {
						 index2 = firstlayer;
							xi = inst.values[0][i];
							mul = xi * zh * (1 - zh);
							for (hprime = 0; hprime < N.weights[1].col; hprime++)
								{
									if (h == hprime - 1)
  									sum1 = yk - regvalue;
									else
											sum1 = 0;
									zhprime = testH.values[0][hprime];
									vkh = N.weights[1].values[0][h + 1];
									sum2 = zhprime * vkh;
         result.values[index1][index2] += mul * (sum1 + sum2);
									result.values[index2][index1] += mul * (sum1 + sum2);
									index2++;
							 }
							index1++;
					 }
			 }
   matrix_free(inst);
		 tmp = tmp->next;
	 }
	matrix_free(testY);
	matrix_free(testH);
	return result;
}

/**
 * Softmax operation on the output units, to transform the output values into probability estimates. First, we take power of e to the output units. Second, each element of the array is divided to the sum of array of elements to normalize (sum of the elements of the array will be 1). The difference of this function from the softmax function is that, this function also calculates the output values, then makes softmax on those output values.
 * @param[in] inputdata Input vector
 * @param[in] weights Weights from the last layer (input units in LP, hidden units in MLP) to the output units.
 * @param[out] Y Output values to produce.
 */
void softmax_with_calculation_of_outputs(double *inputdata, matrix weights, matrixptr Y)
{
	/*!Last Changed 09.02.2004 Oya*/
	int i, j;
	double sum;
	sum = 0;
	for (i = 0; i < Y->col; i++)
	 {
 		Y->values[0][i] = weights.values[i][0];     /*for bias term*/
 		for (j = 1; j < weights.col; j++)
  			Y->values[0][i] = Y->values[0][i] + weights.values[i][j] * inputdata[j - 1];
 		Y->values[0][i] = exp(Y->values[0][i]);
 		sum = sum + Y->values[0][i];
	 }
	for (i = 0; i < Y->col; i++)
 		Y->values[0][i] = Y->values[0][i] / sum;
}

/**
 * Constructor for a neural network structure. Allocate weights and sets the parameters of the algorithm
 * @param[in] P Parameters of the problem such as learning rate, momentum, number of epochs etc.
 * @return Weights allocated neural network structure
 */
Mlpnetwork allocate_mlpnnetwork(Mlpparameters P)
{
	/*!Last Changed 09.02.2004 Oya*/
	Mlpnetwork nN;	
	nN.weights = allocate_weights(P, RANDOM, 0);
	nN.P = P;	
	return nN;
}

/**
 * Allocates weights and initializes (may not) them to random, or given values.
 * @param[in] P Parameters of the MLP algorithm. The function learns the number of input units, the number of hidden nodes at each layer and the number of output units from this parameter structure.
 * @param[in] inittype Type of initialization. Has three values: NO_INITIALIZATION for not initializing the weights after allocating, RANDOM for randomly initializing weights given the random range in the parameter structure, and INITIAL_VALUE for initializing weights to a fixed value.
 * @param[in] initvalue Initial value for initializing network weights for the INITIAL_VALUE inittype.
 * @return Weight matrix array allocated and initialized. array[i] contains the weights between the i - 1'th hidden layer and the i'th hidden layer.
 */
matrixptr allocate_weights(Mlpparameters P, INITIALIZATION_TYPE inittype, int initvalue)
{
 /*!Last Changed 09.02.2004 Oya*/
 /*if inittype is 0 then assign random values, then init value is useless*/
 /*else if inittype is 1 assign initvalue*/
 /*else no initialization*/
 matrixptr weights;
 int i; /*allocations*/
 weights = (matrixptr) safemalloc ((P.layernum + 1) * sizeof(matrix), "allocate_weights", 3);
 for (i = 0; i < P.layernum; i++)
  {
   if (i == 0)
     weights[i] = matrix_alloc(P.hidden[i], P.dimnum + 1);
   else
     weights[i] = matrix_alloc(P.hidden[i], P.hidden[i - 1] + 1);
  }
 if (P.layernum == 0)
   weights[P.layernum] = matrix_alloc(P.classnum, P.dimnum + 1);
 else
   weights[P.layernum] = matrix_alloc(P.classnum, P.hidden[P.layernum - 1] + 1);
 for (i = 0; i <= P.layernum; i++)
   if (inittype == RANDOM)
     assignTwoDimRandomWeights(&weights[i], P.randrange);
   else 
     if (inittype == INITIAL_VALUE)
       initialize_matrix(&weights[i], initvalue); 
 return weights;
}

/**
 * Reallocate weights of a specific layer of a neural network. This may be due to the adding hidden units to one hidden layer as in Dynamic Node Creation algorithm.
 * @param[in] P Parameters of the MLP algorithm. The function learns the number of input units, the number of hidden nodes at each layer and the number of output units from this parameter structure.
 * @param[out] weights Array of matrices containing the weights of the neural network. weights[i] contains the weights between the i - 1'th hidden layer and the i'th hidden layer.
 * @param[in] hiddenindex Hidden layer in which new units were added
 * @param[in] count Number of hidden units added to the given hidden layer.
 * @param[in] inittype Type of initialization. Has three values: NO_INITIALIZATION for not initializing the weights after allocating, RANDOM for randomly initializing weights given the random range in the parameter structure, and INITIAL_VALUE for initializing weights to a fixed value.
 * @param[in] initvalue Initial value for initializing network weights for the INITIAL_VALUE inittype.
 */
void reallocate_weights(Mlpparameters P, matrixptr weights, int hiddenindex, int count, INITIALIZATION_TYPE inittype, int initvalue)
{
	/*!Last Changed 17.04.2007*/
	int colcount, rowcount;
	if (hiddenindex == 0)
		 colcount = P.dimnum + 1;
	else
		 colcount = P.hidden[hiddenindex - 1] + 1;
	if (hiddenindex != P.layernum - 1)
		 rowcount = P.hidden[hiddenindex + 1];
 else
		 rowcount = P.classnum;
	weights[hiddenindex] = reallocate_matrix(weights[hiddenindex], P.hidden[hiddenindex] + count, colcount);
 weights[hiddenindex + 1] = reallocate_matrix(weights[hiddenindex + 1], rowcount, P.hidden[hiddenindex] + count + 1);
	if (inittype == RANDOM)
	 {
   assign_partial_random_weights(&weights[hiddenindex], P.hidden[hiddenindex], P.hidden[hiddenindex] + count - 1, 0, colcount - 1, P.randrange);
	  assign_partial_random_weights(&weights[hiddenindex + 1], 0, rowcount - 1, P.hidden[hiddenindex] + 1, P.hidden[hiddenindex] + count, P.randrange);
	 }
	else
		 if (inittype == INITIAL_VALUE)
		  {
     initialize_partial_matrix(&weights[hiddenindex], P.hidden[hiddenindex], P.hidden[hiddenindex] + count - 1, 0, colcount - 1, initvalue);
	    initialize_partial_matrix(&weights[hiddenindex + 1], 0, rowcount - 1, P.hidden[hiddenindex] + 1, P.hidden[hiddenindex] + count, initvalue);
			 }
}

/**
 * Allocates memory for the hidden unit values.
 * @param[in] P Parameters of the MLP algorithm. The function learns the number of input units, the number of hidden nodes at each layer and the number of output units from this parameter structure.
 * @return A matrix array containing the hidden unit values. matrix[i] contains the hidden unit values in the i'th hidden layer. It is a one row matrix. If the algorithm is LP, the function returns NULL (Linear perceptron does not contain any hidden layers).
 */
matrixptr allocate_hiddens(Mlpparameters P)
{
	/*!Last Changed 09.02.2004 Oya*/
	matrixptr hiddens;
	int i;	 
	/*allocations*/
	if (P.layernum > 0)
	 {
	 	hiddens = (matrixptr) safemalloc (P.layernum * sizeof(matrix), "allocate_hiddens", 4);
	 	for (i = 0; i < P.layernum; i++)
	  		hiddens[i] = matrix_alloc(1, P.hidden[i]);
		 return hiddens;
	 }
	else
	 	return NULL;	
}

/**
 * Reallocates hidden unit values for a specific hidden layer. This may be due to the adding hidden units to one hidden layer as in Dynamic Node Creation algorithm.
 * @param[out] hiddens A matrix array containing the hidden unit values. hiddens[i] contains the hidden unit values in the i'th hidden layer. It is a one row matrix.
 * @param[in] P Parameters of the MLP algorithm. The function learns the number of input units, the number of hidden nodes at each layer and the number of output units from this parameter structure.
 * @param[in] hiddenindex Hidden layer in which new units were added
 * @param[in] count Number of hidden units added to the given hidden layer.
 */
void reallocate_hiddens(matrixptr hiddens, Mlpparameters P, int hiddenindex, int count)
{
 /*!Last Changed 17.04.2007*/
 hiddens[hiddenindex] = reallocate_matrix(hiddens[hiddenindex], 1, P.hidden[hiddenindex] + count);
}

/**
 * Destructor for the neural network structure. Deallocates memory allocated for the weights.
 * @param[in] nN Neural network structure
 */
void free_mlpnnetwork(Mlpnetworkptr nN)
{	
	/*!Last Changed 09.02.2004 Oya*/
	free_weights(nN->weights, nN->P.layernum);
}

/**
 * Frees memory allocated for the weights in a neural network structure
 * @param[in] weights Array of matrices containing the weights of the neural network. weights[i] contains the weights between the i - 1'th hidden layer and the i'th hidden layer.
 * @param[in] layer Number of hidden layers in the neural network
 */
void free_weights(matrixptr weights, int layer)
{
	/*!Last Changed 09.02.2004 Oya*/
	int i;	
	for (i = 0; i < (layer + 1); i++)
		 matrix_free(weights[i]);
	safe_free(weights);
}

/**
 * Frees memory allocated for the hidden unit values in a neural network structure
 * @param[in] hiddens A matrix array containing the hidden unit values. hiddens[i] contains the hidden unit values in the i'th hidden layer. It is a one row matrix.
 * @param[in] layer Number of hidden layers in the neural network
 */
void free_hiddens(matrixptr hiddens, int layer)
{
	/*!Last Changed 09.02.2004 Oya*/
	int i;	
	if (layer > 0)
	 {	
 		for (i = 0; i < layer; i++)
 			 matrix_free(hiddens[i]);
 		safe_free(hiddens);
	 }
}

void calculate_hidden_values(double *inputdata, matrix weight, matrixptr hidden)
{
	/*!Last Changed 09.02.2004 Oya*/
	int i, hi;
	double sum;	
	for (hi = 0; hi < weight.row; hi++)
  {
		 sum = weight.values[hi][0];
		 for (i = 1; i < weight.col; i++)
    {
				 sum = sum + weight.values[hi][i] * inputdata[i - 1];
		  }
		 hidden->values[0][hi] = sigmoid(sum);
	 }
}

void calculate_output_values_cls(double *inputdata, matrix weight, matrixptr output)
{	
	/*!Last Changed 09.02.2004 Oya*/
	softmax_with_calculation_of_outputs(inputdata, weight, output);
}

void calculate_output_values_reg(double *inputdata, matrix weight, matrixptr output)
{	
	/*!Last Changed 16.03.2004 Oya*/
	int i, hi;
	double sum;	
	for (hi=0; hi<weight.row; hi++)
  {
		 sum = weight.values[hi][0];
		 for (i=1; i<weight.col; i++)
    {
		 		sum = sum + weight.values[hi][i] * inputdata[i-1];
		  }
		 output->values[0][hi] = sum;
	 }
}

double calculate_error_values_cls(double outputclass, matrix output, matrixptr output_error)
{
	/*!Last Changed 09.02.2004 Oya*/
	int i;
	double sqrderror = 0;
	/*for error in classification*/
	for (i=0; i<output.col; i++)
	 {
 		output_error->values[0][i] = equal((int)outputclass,i) - output.values[0][i];
 		sqrderror = sqrderror + (-1 * equal((int)outputclass,i) * log(output.values[0][i]));
	 }
	sqrderror = sqrderror / output.col;
	return sqrderror;
}

double calculate_error_values_reg(double outputdata, matrix output, matrixptr output_error)
{	
	/*!Last Changed 16.03.2004 Oya*/
	int i;
	double sqrderror = 0;
	/*for error in regression*/
	for (i=0; i<output.col; i++)
  {
		 output_error->values[0][i] = outputdata - output.values[0][i];
		 sqrderror = sqrderror + output_error->values[0][i] * output_error->values[0][i];
	 }
	sqrderror = sqrderror / output.col;
	return sqrderror;
}

double calculate_error_values_autoencoder(double* outputdata, matrix output, matrixptr output_error)
{
	int i;
	double sqrderror = 0;
	for (i = 0; i < output.col; i++)
  {
   output_error->values[0][i] = outputdata[i] - output.values[0][i];
   sqrderror = sqrderror + output_error->values[0][i] * output_error->values[0][i];
  }
	sqrderror = sqrderror / output.col;
	return sqrderror;
}

void calculate_change_in_weights(double *inputdata, matrixptr weights, matrixptr hiddens, matrixptr dweights, matrix output_error, Mlpparameters P)
{
	/*!Last Changed 09.02.2004 Oya*/
	int li, i, j, pi;
	/*
	li layer index
	i, j row and column index for the current matrix
	pi row index of the weight matrix of above layer
	*/
	double sum = 0, tmp;
	for (li = P.layernum; li >= 0; li--)
	 {
 		if (P.layernum == 0)
			 {
	  		for (i=0; i<weights[li].row; i++)
					 {
						 tmp = P.eta * output_error.values[0][i];
		   		dweights[li].values[i][0] = tmp;
   				for (j=1; j<weights[li].col; j++)
		    			dweights[li].values[i][j] = tmp * inputdata[j-1]; 
					 }
			 }
		 else 
				 if (li==P.layernum)
					 {
    			for (i=0; i<weights[li].row; i++)
							 {
								 tmp = P.eta * output_error.values[0][i];
				     dweights[li].values[i][0] = tmp;
				     for (j=1; j<weights[li].col; j++)
					      dweights[li].values[i][j] = tmp * hiddens[li-1].values[0][j-1]; 
							 }
					 }
		   else
					 {
			    for (i=0; i<weights[li].row; i++)
							 {
				     sum = 0;
				     for (pi=0; pi<weights[li+1].row; pi++)
      					sum = sum + dweights[li+1].values[pi][i+1] * weights[li+1].values[pi][i+1];
									tmp = sum * (1 - hiddens[li].values[0][i]);
				     dweights[li].values[i][0] = tmp;
				     for (j=1; j<weights[li].col; j++)
									 {
      					if (li == 0)
				       		dweights[li].values[i][j] = tmp * inputdata[j-1];
      					else
       						dweights[li].values[i][j] = tmp * hiddens[li-1].values[0][j-1];
								  }
							 }
					 }
		} 
}

void update_weights(matrixptr weights, matrixptr dweights, matrixptr prevWeights, Mlpparameters P)
{
	/*!Last Changed 23.06.2005 added weight decay*/
	/*!Last Changed 09.02.2004 Oya*/
	int li, i, j;	
	for (li=0; li<(P.layernum+1); li++)
 		for (i=0; i<weights[li].row; i++)
  			for (j=0; j<weights[li].col; j++)
  					weights[li].values[i][j] = weights[li].values[i][j] + dweights[li].values[i][j] + P.alpha * prevWeights[li].values[i][j] + P.weightdecay * P.decayalpha * P.eta * weights[li].values[i][j]; 
}

double backPropMlpn_cls(double *Train, matrixptr weights, matrixptr hidden, matrixptr prevWeights, Mlpparameters P)
{
	/*!Last Changed 09.02.2004 Oya*/
	matrix output_error, Y;
	matrixptr dweights;
	double sqrderror;
 int li;
	output_error = matrix_alloc(1, P.classnum);
	Y = matrix_alloc(1, P.classnum);
	dweights = allocate_weights(P, NO_INITIALIZATION, 0);/*dweights has the same dimensions as weights*/	
	/*calculate hidden layers*/
	for (li = 0; li < P.layernum; li++)
	 {
 		if (li == 0)
	  		calculate_hidden_values(Train, weights[0], &hidden[0]);
	 	else
	  		calculate_hidden_values(hidden[li-1].values[0], weights[li], &hidden[li]);
	 }
	if (P.layernum == 0) 	/*calculate outputs*/
		 calculate_output_values_cls(Train, weights[P.layernum], &Y);
	else
		 calculate_output_values_cls(hidden[P.layernum-1].values[0], weights[P.layernum], &Y);	
	sqrderror = calculate_error_values_cls(Train[P.dimnum], Y, &output_error); 	/*calculate error*/
	calculate_change_in_weights(Train, weights, hidden, dweights, output_error, P); 	/*calculate the change in weights*/
	update_weights(weights, dweights, prevWeights, P); 	/*add the change and momentum value to weights*/
	/*update prevWeights*/
	for (li=0; li<(P.layernum+1); li++)
		 matrix_identical(&(prevWeights[li]), dweights[li]);
	matrix_free(output_error);
	matrix_free(Y);
	free_weights(dweights, P.layernum);	
	return sqrderror;
}

double backPropMlpn_reg(double *Train, matrixptr weights, matrixptr hidden, matrixptr prevWeights, Mlpparameters P)
{	
	/*!Last Changed 16.03.2004 Oya*/
	matrix output_error, Y;
	matrixptr dweights;
	double sqrderror;
 int li;
	output_error = matrix_alloc(1, P.classnum);
	Y = matrix_alloc(1, P.classnum);
	dweights = allocate_weights(P, NO_INITIALIZATION, 0);/*dweights has the same dimensions as weights*/	
	/*calculate hidden layers*/
	for (li=0; li<P.layernum; li++)
	 {
 		if (li==0)
	  		calculate_hidden_values(Train, weights[0], &hidden[0]);
	 	else
	  		calculate_hidden_values(hidden[li-1].values[0], weights[li], &hidden[li]);
	 }
	if (P.layernum == 0) 	/*calculate outputs*/
		 calculate_output_values_reg(Train, weights[P.layernum], &Y);
	else
		 calculate_output_values_reg(hidden[P.layernum-1].values[0], weights[P.layernum], &Y);	
	sqrderror = calculate_error_values_reg(Train[P.dimnum], Y, &output_error); 	/*calculate error*/
	calculate_change_in_weights(Train, weights, hidden, dweights, output_error, P); 	/*calculate the change in weights*/
	update_weights(weights, dweights, prevWeights, P); 	/*add the change and momentum value to weights*/
	/*update prevWeights*/
	for (li=0; li<(P.layernum+1); li++)
		 matrix_identical(&(prevWeights[li]), dweights[li]);
	matrix_free(output_error);
	matrix_free(Y);
	free_weights(dweights, P.layernum);	
	return sqrderror;
}

double backPropMlpn_autoencoder(double *Train, matrixptr weights, matrixptr hidden, matrixptr prevWeights, Mlpparameters P)
{
	matrix output_error, Y;
	matrixptr dweights;
	double sqrderror;
 int li;
	output_error = matrix_alloc(1, P.classnum);
	Y = matrix_alloc(1, P.classnum);
	dweights = allocate_weights(P, NO_INITIALIZATION, 0);/*dweights has the same dimensions as weights*/
	/*calculate hidden layers*/
	for (li = 0; li < P.layernum; li++)
  {
   if (li == 0)
     calculate_hidden_values(Train, weights[0], &hidden[0]);
   else
     calculate_hidden_values(hidden[li-1].values[0], weights[li], &hidden[li]);
  }
 calculate_output_values_reg(hidden[P.layernum-1].values[0], weights[P.layernum], &Y);
	sqrderror = calculate_error_values_autoencoder(Train, Y, &output_error); 	/*calculate error*/
	calculate_change_in_weights(Train, weights, hidden, dweights, output_error, P); 	/*calculate the change in weights*/
	update_weights(weights, dweights, prevWeights, P); 	/*add the change and momentum value to weights*/
	/*update prevWeights*/
	for (li = 0; li < (P.layernum + 1); li++)
   matrix_identical(&(prevWeights[li]), dweights[li]);
	matrix_free(output_error);
	matrix_free(Y);
	free_weights(dweights, P.layernum);
	return sqrderror;
}

double testMlpnInput_cls(double *Test, matrix* testY, Mlpnetwork N, int* correctlyclassified, double* prob)
{
	/*!Last Changed 09.02.2004 Oya*/
	double sqrderror;
	matrix output_error;
	matrixptr testH;
	int li, output, classno;		
	output_error = matrix_alloc(1, N.P.classnum);
	*testY = matrix_alloc(1, N.P.classnum);
	testH = allocate_hiddens(N.P);
	/*calculate hidden layers*/
	for (li=0; li<N.P.layernum; li++)
	 {
 		if (li == 0)
  			calculate_hidden_values(Test, N.weights[0], &testH[0]);
	 	else
	  		calculate_hidden_values(testH[li-1].values[0], N.weights[li], &testH[li]);
	 }
	/*calculate outputs*/
	if (N.P.layernum == 0)
  	calculate_output_values_cls(Test, N.weights[N.P.layernum], testY);
	else
		 calculate_output_values_cls(testH[N.P.layernum-1].values[0], N.weights[N.P.layernum], testY);
	/*calculate error*/
	sqrderror = calculate_error_values_cls(Test[N.P.dimnum], *testY, &output_error);
	output = findMaxOutput(testY->values[0], N.P.classnum);
	classno = (int)Test[N.P.dimnum];
	*prob = testY->values[0][classno];
	matrix_free(output_error);
	free_hiddens(testH, N.P.layernum);	
	if (classno == output)
		 *correctlyclassified = 1;
	else
		 *correctlyclassified = 0;
	return sqrderror;
}

double testMlpnInput_reg(double *Test, matrix* testY, Mlpnetwork N, matrixptr output_error)
{
	/*!Last Changed 16.03.2004 Oya*/
	double sqrderror;
	matrixptr testH;
	int li;			
	*testY = matrix_alloc(1, N.P.classnum);
	testH = allocate_hiddens(N.P);
	/*calculate hidden layers*/
	for (li=0; li<N.P.layernum; li++)
	 {
 		if (li==0)
  			calculate_hidden_values(Test, N.weights[0], &testH[0]);
	 	else
	  		calculate_hidden_values(testH[li-1].values[0], N.weights[li], &testH[li]);
	 }
	/*calculate outputs*/
	if (N.P.layernum == 0)
   calculate_output_values_reg(Test, N.weights[N.P.layernum], testY);
	else
		 calculate_output_values_reg(testH[N.P.layernum-1].values[0], N.weights[N.P.layernum], testY);
	/*calculate error*/
	sqrderror = calculate_error_values_reg(Test[N.P.dimnum], *testY, output_error);
	free_hiddens(testH, N.P.layernum);
	return sqrderror;
}

double testMlpnInput_autoencoder(double *Test, matrix* testY, Mlpnetwork N, matrixptr output_error)
{
	double sqrderror;
	matrixptr testH;
	int li;
	*testY = matrix_alloc(1, N.P.classnum);
	testH = allocate_hiddens(N.P);
	/*calculate hidden layers*/
	for (li = 0; li < N.P.layernum; li++)
  {
   if (li == 0)
     calculate_hidden_values(Test, N.weights[0], &testH[0]);
   else
     calculate_hidden_values(testH[li-1].values[0], N.weights[li], &testH[li]);
  }
	/*calculate outputs*/
 calculate_output_values_reg(testH[N.P.layernum-1].values[0], N.weights[N.P.layernum], testY);
	/*calculate error*/
	sqrderror = calculate_error_values_autoencoder(Test, *testY, output_error);
	free_hiddens(testH, N.P.layernum);
	return sqrderror;
}

int testMlpnNetwork_cls(matrix Data, Mlpnetwork N, double* loglikelihood)
{
	/*!Last Changed 22.05.2005 fixed class results*/
	/*!Last Changed 09.02.2004 Oya*/
	int k, res, performance = 0;
	double prob;
	matrix testY;
	*loglikelihood = 0;
	for (k = 0; k < Data.row; k++)
	 {
		 testMlpnInput_cls(Data.values[k], &testY, N, &res, &prob);
			if (prob != 0)
				 *loglikelihood += log2(prob);
   performance += res;				 
			matrix_free(testY);
	 }
	return performance;
}

int testMlpnNetwork2_cls(matrix Data, Mlpnetwork N, double* loglikelihood, double *sqrdError)
{
	/*!Last Changed 22.05.2005 fixed class results*/
	/*!Last Changed 09.02.2004 Oya*/
	int k, res, performance = 0;
	double prob;	
	matrix testY;
	*sqrdError = 0;
	*loglikelihood = 0;
	for (k = 0; k < Data.row; k++)
	 {
		 *sqrdError += testMlpnInput_cls(Data.values[k], &testY, N, &res, &prob);
			if (prob != 0)
				 *loglikelihood += log2(prob);
   performance += res;				 
			matrix_free(testY);
	 }
	*sqrdError = *sqrdError / Data.row;
	return performance;
}

double testMlpnNetwork_reg(matrix Data, Mlpnetwork N)
{
	/*!Last Changed 16.03.2004 Oya*/
	double sqrdError = 0;
	matrix output_error, testY;
	int k;
 output_error = matrix_alloc(1, N.P.classnum);
	for (k = 0; k < Data.row; k++)
	 {
   sqrdError =  sqrdError + testMlpnInput_reg(Data.values[k], &testY, N, &output_error);
   matrix_free(testY);
	 } 	
	matrix_free(output_error);
	return sqrdError;
}

double testMlpnNetwork_autoencoder(matrix Data, Mlpnetwork N)
{
	double sqrdError = 0;
	matrix output_error, testY;
	int k;
 output_error = matrix_alloc(1, N.P.classnum);
	for (k = 0; k < Data.row; k++)
  {
   sqrdError =  sqrdError + testMlpnInput_autoencoder(Data.values[k], &testY, N, &output_error);
   matrix_free(testY);
  }
	matrix_free(output_error);
	return sqrdError;
}

Mlpnetwork dnc_cls(matrix Train, Mlpparametersptr P)
{
 /*!Last Changed 16.04.2007*/
 Mlpnetwork nN;
 matrixptr prevWeights, H;
 double *errorinwidth, sqrdError, lastError, control;
 int tr, trial, *I, iindex, k, lastNum = 0;
 nN = allocate_mlpnnetwork(*P);
 prevWeights = allocate_weights(*P, INITIAL_VALUE, 0);
 H = allocate_hiddens(*P);
 errorinwidth = safemalloc(P->windowsize * sizeof(double), "dnc_cls", 8);
 P->eta = P->initialeta;
 for (tr = 0; tr < P->epochs; tr++)
  {
   trial = tr + 1;
   sqrdError = 0.0;  
   I = random_array(P->inputnum);
   for (k = 0; k < P->inputnum; k++)
    {
     iindex = I[k];
     sqrdError = sqrdError + backPropMlpn_cls(Train.values[iindex], nN.weights, H, prevWeights, *P);
    }
   safe_free(I);
   sqrdError = sqrdError / P->inputnum; 
   P->sqrderror = sqrdError;
   if (sqrdError <= P->errorthreshold)
     break;
   if (tr == 0)
     lastError = sqrdError;  
   if ((trial > P->windowsize) && ((trial - P->windowsize) >= lastNum))
    {
				 if (P->maxhidden != 0 && P->hidden[0] == P->maxhidden)
						 break;
     control = fabs(sqrdError - errorinwidth[(trial - 1) % P->windowsize]) / lastError;
     if (control < P->errordropratio)
      {
       lastError = sqrdError;
       lastNum = trial;
       free_weights(prevWeights, P->layernum);
							reallocate_weights(*P, nN.weights, 0, 1, RANDOM, 0);
							reallocate_hiddens(H, *P, 0, 1);
							H[0].values[0][P->hidden[0]] = 0.0;
       P->hidden[0]++;
       P->eta = P->initialeta;
       prevWeights = allocate_weights(*P, INITIAL_VALUE, 0);
      }
    }
   errorinwidth[(trial - 1) % P->windowsize] = sqrdError;    
   P->eta = P->eta * P->etadecrease;
  }    
 nN.P = *P;
 safe_free(errorinwidth);
 free_weights(prevWeights, P->layernum);
 free_hiddens(H, P->layernum);    
 P->epochstrained = trial;
 return nN;
}

Mlpnetwork dnc_reg(matrix Train, Mlpparametersptr P)
{
 /*!Last Changed 17.04.2007*/
 Mlpnetwork nN;
 matrixptr prevWeights, H;
 double *errorinwidth, sqrdError, lastError, control;
 int tr, trial, *I, iindex, k, lastNum = 0;
 nN = allocate_mlpnnetwork(*P);
 prevWeights = allocate_weights(*P, INITIAL_VALUE, 0);
 H = allocate_hiddens(*P);
 errorinwidth = safemalloc(P->windowsize * sizeof(double), "dnc_reg", 8);
 P->eta = P->initialeta;
 for (tr = 0; tr < P->epochs; tr++)
  {
   trial = tr + 1;
   sqrdError = 0.0;  
   I = random_array(P->inputnum);
   for (k = 0; k < P->inputnum; k++)
    {
     iindex = I[k];
     sqrdError = sqrdError + backPropMlpn_reg(Train.values[iindex], nN.weights, H, prevWeights, *P);
    }
   safe_free(I);
   sqrdError = sqrdError / P->inputnum; 
   P->sqrderror = sqrdError;   
   if (sqrdError <= P->errorthreshold)
     break;
   if (tr == 0)
     lastError = sqrdError;  
   if ((trial > P->windowsize) && ((trial - P->windowsize) >= lastNum))
    {  
				 if (P->maxhidden != 0 && P->hidden[0] == P->maxhidden)
						 break;
     control = fabs(sqrdError - errorinwidth[(trial - 1) % P->windowsize]) / lastError;
     if (control < P->errordropratio)
      {
       lastError = sqrdError;
       lastNum = trial;
       free_weights(prevWeights, P->layernum);
							reallocate_weights(*P, nN.weights, 0, 1, RANDOM, 0);
							reallocate_hiddens(H, *P, 0, 1);
							H[0].values[0][P->hidden[0]] = 0.0;
       P->hidden[0]++;
       P->eta = P->initialeta;
       prevWeights = allocate_weights(*P, INITIAL_VALUE, 0);
      }
    }
   errorinwidth[(trial - 1) % P->windowsize] = sqrdError;    
   P->eta = P->eta * P->etadecrease;
  }    
 nN.P = *P;
 safe_free(errorinwidth);
 free_weights(prevWeights, P->layernum);
 free_hiddens(H, P->layernum);    
 P->epochstrained = trial;
 return nN;
}

Mlpnetwork mlpn_general(matrix Train, matrix Validation, Mlpparametersptr P, SUPERVISED_ALGORITHM_TYPE algorithm_type)
{
	int iindex, tr, k;
	double sqrdError = 0, minError, cvError, loglikelihood;
	int *I;
	Mlpnetwork nN, minN;
	matrixptr prevWeights;
	matrixptr H;
	nN = allocate_mlpnnetwork(*P);
	prevWeights = allocate_weights(*P, INITIAL_VALUE, 0);
	H = allocate_hiddens(*P);
	P->eta = P->initialeta;
	for (tr = 0; tr < P->epochs; tr++)
  {
   sqrdError = 0;
   I = random_array(P->inputnum);
   for (k = 0; k < P->inputnum; k++)
    {
     iindex = I[k];
     switch (algorithm_type)
      {
       case CLASSIFICATION:
       default            :sqrdError = sqrdError + backPropMlpn_cls(Train.values[iindex], nN.weights, H, prevWeights, *P);
                           break;
       case REGRESSION    :sqrdError = sqrdError + backPropMlpn_reg(Train.values[iindex], nN.weights, H, prevWeights, *P);
                           break;
       case AUTOENCODER   :sqrdError = sqrdError + backPropMlpn_autoencoder(Train.values[iindex], nN.weights, H, prevWeights, *P);
                           break;
      }
    }
   safe_free(I);
   sqrdError = sqrdError / P->inputnum;
   P->sqrderror = sqrdError;
   P->eta = P->eta * P->etadecrease;
   /*check validation data and update the best network if necessary*/
   nN.P = *P;
   switch (algorithm_type)
    {
     case CLASSIFICATION:
     default            :cvError = Validation.row - testMlpnNetwork_cls(Validation, nN, &loglikelihood);
                         break;
     case REGRESSION    :cvError = testMlpnNetwork_reg(Validation, nN);
                         break;
     case AUTOENCODER   :cvError = testMlpnNetwork_autoencoder(Validation, nN);
                         break;
    }
   if (tr > 0)
    {
     if (cvError < minError)
      {
       P->bestepoch = tr;
       minError = cvError;
       free_mlpnnetwork(&minN);
       minN = copy_of_Mlpnetwork(nN);
      }
    }
   else
    {
     P->bestepoch = tr;
     minError = cvError;
     minN = copy_of_Mlpnetwork(nN);
    }
  }
	free_weights(prevWeights, P->layernum);
	free_hiddens(H, P->layernum);
	free_mlpnnetwork(&nN);
 return minN;
}
