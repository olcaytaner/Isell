#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "classification.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "lang.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"
#include "model.h"
#include "parameter.h"
#include "statistics.h"

extern Datasetptr Datasets;
extern Datasetptr current_dataset;
extern int performance;

/**
 * Calculates the number of free parameters in a Hessian matrix. The number of free parameters is: \sum_i \frac{\lambda_i}{\lambda_i + \alpha}
 * @param[in] eigenvalues Eigenvalues of the Hessian matrix. Stored as an array. eigenvalues[i] is the i'th eigenvalue.
 * @param[in] weightcount Number of dimensions of the Hessian matrix (Size of the eigenvalues array).
 * @param[in] alpha Alpha parameter
 * @return Number of parameters estimated
 */
double number_of_free_parameters_of_hessian(double* eigenvalues, int weightcount, double alpha)
{
	/*!Last Changed 26.06.2005*/
	double sum = 0;
	int i;
	for (i = 0; i < weightcount; i++)
		 sum += eigenvalues[i] / (eigenvalues[i] + alpha);
	return sum;
}

double hinge_loss(double y, double fx)
{
 HINGELOSS_TYPE hingelosstype = get_parameter_l("hingelosstype");
 if (1 - y * fx > 0.0)
   switch (hingelosstype)
    {
     case REGULAR_LOSS:return 1 - y * fx;
     case HINGELOSS_01:if (1 - y * fx < 1.0)
                         return 1 - y * fx;
                       else
                         return 1.0;
     case HINGELOSS_02:if (1 - y * fx < 2.0)
                         return 1 - y * fx;
                       else
                         return 2.0;
    } 
 return 0.0;
}

double epsilon_sensitive_loss(double y, double fx, double epsilon)
{
 if (fabs(y - fx) > epsilon)
   return fabs(y - fx) - epsilon;
 return 0.0;
}

/**
 * Finds the best model for a single layer multilayer perceptron using Akaike Information Criterion (model selection criterion).
 * @param[in] data Training data used to train the neural network
 * @param[in] validation Validation data used to find best weights for a given a neural network structure. The best weights are the weights, for which the network has the smallest error on the validation data.
 * @param[in] alpha Alpha parameter used in the calculation of free parameters of the Hessian matrix
 * @param[in] maxhidden Number of maximum hidden units to search
 * @param[in] problemtype Type of the machine learning problem. CLASSIFICATION for a classification problem, REGRESSION for a regression problem, 2 for a one-vs-all problem where the index of the class one is given in the parameter positive, 3 for one-class-problem where the class index is given in the parameter positive.
 * @return Best neural network structure
 */
Mlpnetwork mlp_model_selection_aic(Instanceptr data, Instanceptr validation, double alpha, int maxhidden, int problemtype)
{
	/*!Last Changed 25.06.2005*/
 int i, hiddennodes[1] = {1};
	Mlpparameters mlpparams;
	matrix train, cv, hessian, eigenvectors;
	Mlpnetwork neuralnetwork, best;
	double* eigenvalues, d, aicerror, loglikelihood, trainerror, besterror = +INT_MAX;
	train = instancelist_to_matrix(data, problemtype, -1, MULTIVARIATE_LINEAR);
	cv = instancelist_to_matrix(validation, problemtype, -1, MULTIVARIATE_LINEAR);
 if (current_dataset->classno == 0)
	  mlpparams = prepare_Mlpparameters(data, validation, 1, alpha, 1, hiddennodes, 0, 0.0, 0.0, 0, REGRESSION);
 else
   mlpparams = prepare_Mlpparameters(data, validation, 1, alpha, 1, hiddennodes, 0, 0.0, 0.0, 0, CLASSIFICATION);
	for (i = 1; i <= maxhidden; i++)
	 {
			mlpparams.hidden[0] = i;
			neuralnetwork = mlp_network_train_general(train, cv, &mlpparams);
			if (current_dataset->classno == 0)
			 {
     hessian = hessian_matrix_reg(neuralnetwork, data);
					trainerror = test_mlp_network(neuralnetwork, train);
			 }
			else
			 {
     hessian = hessian_matrix_cls(neuralnetwork, data);
			  loglikelihood = log_likelihood_of_mlp_network(neuralnetwork, train);
			 }
   eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, hessian);
			matrix_free(eigenvectors);
			d = number_of_free_parameters_of_hessian(eigenvalues, hessian.row, alpha);
			if (current_dataset->classno == 0)
				 aicerror = aic_squared_error(trainerror, d, train.row);
			else
  			aicerror = aic_loglikelihood(loglikelihood, d);
			if (i != 1 && aicerror < besterror)
				 free_mlpnnetwork(&best);
			if (i == 1 || aicerror < besterror)
			 {
					best = copy_of_Mlpnetwork(neuralnetwork);
					besterror = aicerror;
			 }
			free_mlpnnetwork(&neuralnetwork);
			matrix_free(hessian);
	 }
	matrix_free(train);
	matrix_free(cv);
	return best;
}

/**
 * Finds the best model for a single layer multilayer perceptron using Bayesian Information Criterion (model selection criterion).
 * @param[in] data Training data used to train the neural network
 * @param[in] validation Validation data used to find best weights for a given a neural network structure. The best weights are the weights, for which the network has the smallest error on the validation data.
 * @param[in] alpha Alpha parameter used in the calculation of free parameters of the Hessian matrix
 * @param[in] maxhidden Number of maximum hidden units to search
 * @param[in] problemtype Type of the machine learning problem. CLASSIFICATION for a classification problem, REGRESSION for a regression problem, 2 for a one-vs-all problem where the index of the class one is given in the parameter positive, 3 for one-class-problem where the class index is given in the parameter positive.
 * @return Best neural network structure
 */
Mlpnetwork mlp_model_selection_bic(Instanceptr data, Instanceptr validation, double alpha, int maxhidden, int problemtype)
{
	/*!Last Changed 25.06.2005*/
	/*!Last Changed 25.06.2005*/
 int i, hiddennodes[1] = {1};
	Mlpparameters mlpparams;
	matrix train, cv, hessian, eigenvectors;
	Mlpnetwork neuralnetwork, best;
	double* eigenvalues, d, bicerror, trainerror, besterror = +INT_MAX, loglikelihood;
	train = instancelist_to_matrix(data, problemtype, -1, MULTIVARIATE_LINEAR);
	cv = instancelist_to_matrix(validation, problemtype, -1, MULTIVARIATE_LINEAR);
 if (current_dataset->classno == 0)
	  mlpparams = prepare_Mlpparameters(data, validation, 1, alpha, 1, hiddennodes, 0, 0.0, 0.0, 0, REGRESSION);
 else
   mlpparams = prepare_Mlpparameters(data, validation, 1, alpha, 1, hiddennodes, 0, 0.0, 0.0, 0, CLASSIFICATION);
	for (i = 1; i <= maxhidden; i++)
	 {
			mlpparams.hidden[0] = i;
			neuralnetwork = mlp_network_train_general(train, cv, &mlpparams);
			if (current_dataset->classno == 0)
			 {
     hessian = hessian_matrix_reg(neuralnetwork, data);
					trainerror = test_mlp_network(neuralnetwork, train);
			 }
			else
			 {
     hessian = hessian_matrix_cls(neuralnetwork, data);
			  loglikelihood = log_likelihood_of_mlp_network(neuralnetwork, train);
			 }
   eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, hessian);
			matrix_free(eigenvectors);
			d = number_of_free_parameters_of_hessian(eigenvalues, hessian.row, alpha);
			if (current_dataset->classno == 0)
				 bicerror = bic_squared_error(trainerror, d, train.row);
			else
  			bicerror = bic_loglikelihood(loglikelihood, d, train.row);
			if (i != 1 && bicerror < besterror)
				 free_mlpnnetwork(&best);
			if (i == 1 || bicerror < besterror)
			 {
					best = copy_of_Mlpnetwork(neuralnetwork);
					besterror = bicerror;
			 }
			free_mlpnnetwork(&neuralnetwork);
			matrix_free(hessian);
	 }
	matrix_free(train);
	matrix_free(cv);
	return best;
}

/**
 * Finds the best model for a single layer multilayer perceptron using Cross-Validation based model selection. 5 \times 2 cross-validation is used.
 * @param[in] data Training data used to train and find best neural network.
 * @param[in] validation Validation data used to find best weights for a given a neural network structure. The best weights are the weights, for which the network has the smallest error on the validation data.
 * @param[in] alpha Alpha parameter used in the calculation of free parameters of the Hessian matrix
 * @param[in] maxhidden Number of maximum hidden units to search
 * @param[in] problemtype Type of the machine learning problem. CLASSIFICATION for a classification problem, REGRESSION for a regression problem, 2 for a one-vs-all problem where the index of the class one is given in the parameter positive, 3 for one-class-problem where the class index is given in the parameter positive.
 * @return Best neural network structure
 */
Mlpnetwork mlp_model_selection_crossvalidation(Instanceptr* data, Instanceptr validation, double alpha, int maxhidden, int problemtype)
{
	/*!Last Changed 26.06.2005*/
	int i, j, hiddennodes[1] = {1}, dummy, *models, dummy2;
	char **files, st[READLINELENGTH] = "-", pst[READLINELENGTH];
	FILE* mlpfile;
	Mlpparameters mlpparams;
	Instanceptr traindata, testdata;
	matrix train, test, cv, whole;
	Mlpnetwork neuralnetwork, best;
	double error;
	files = safemalloc(maxhidden * sizeof(char *), "mlp_model_selection_crossvalidation", 9);
 if (current_dataset->classno == 0)
	  mlpparams = prepare_Mlpparameters(*data, validation, 1, alpha, 1, hiddennodes, 0, 0.0, 0.0, 0, REGRESSION);
 else
   mlpparams = prepare_Mlpparameters(*data, validation, 1, alpha, 1, hiddennodes, 0, 0.0, 0.0, 0, CLASSIFICATION);
	cv = instancelist_to_matrix(validation, problemtype, -1, MULTIVARIATE_LINEAR);
	for (i = 1; i <= maxhidden; i++)
	 {
			mlpparams.hidden[0] = i;
		 files[i - 1] = safemalloc(500 * sizeof(char), "mlp_model_selection_crossvalidation", 15);
			sprintf(files[i - 1], "%s/%d.txt", get_parameter_s("tempdirectory"), i);
			mlpfile = fopen(files[i - 1], "w");
   if (!mlpfile)
     return best;
			for (j = 0; j < 5; j++)
			 {
				 divide_instancelist(data, &traindata, &testdata, 2);
	    train = instancelist_to_matrix(traindata, problemtype, -1, MULTIVARIATE_LINEAR);
	    test = instancelist_to_matrix(testdata, problemtype, -1, MULTIVARIATE_LINEAR);
					neuralnetwork = mlp_network_train_general(train, cv, &mlpparams);
					error = test_mlp_network(neuralnetwork, test);
					free_mlpnnetwork(&neuralnetwork);
			  set_precision(pst, NULL, "\n");
     fprintf(mlpfile, pst, error);
					neuralnetwork = mlp_network_train_general(test, cv, &mlpparams);
					error = test_mlp_network(neuralnetwork, train);
					free_mlpnnetwork(&neuralnetwork);
			  set_precision(pst, NULL, "\n");
     fprintf(mlpfile, pst, error);
					matrix_free(train);
					matrix_free(test);
					*data = traindata;
					merge_instancelist(data, testdata);
			 }
			fclose(mlpfile);
	 }
 models = multiple_comparison(files, maxhidden, PAIREDT5X2, st, &dummy, &dummy2, 1);
	whole = instancelist_to_matrix(*data, problemtype, -1, MULTIVARIATE_LINEAR); 
	mlpparams.hidden[0] = models[0];
	best = mlp_network_train_general(whole, cv, &mlpparams);
	matrix_free(cv);
	matrix_free(whole);
	free_2d((void **)files, maxhidden);
	safe_free(models);
	return best;
}

/**
 * Finds the best model for a single layer multilayer perceptron using selected model selection technique.
 * @param[in] data Data used for testing the neural network (20 percent), find the best alpha parameter (16 percent), finding the best weights for a given neural network structure (12.8 percent), training a given neural network (51.2 percent).
 * @param[in] modelselectionmethod Model selection technique. MODEL_SELECTION_AIC for Akaike Information Criterion based model selection, MODEL_SELECTION_BIC for Bayesian Information Criterion based model selection and MODEL_SELECTION_CV for cross-validation based model selection.
 * @param[out] hiddennodes Number of hidden nodes in the best neural network structure
 * @param[in] maxhidden Number of maximum hidden units to search
 * @param[out] bestalpha The alpha parameter of the best neural network structure. Best alpha is searched in two phases. In the first phase exponential search is applied. The alpha values 0.001, 0.002, 0.004, ..., 100 are tried and the best one is selected (besta). In the second phase linear search is applied. The alpha values besta / 2, besta / 2 + besta / 8, besta / 2 + 2besta / 8, ..., 3besta / 2 are tried.
 * @return Mean square error of the best neural network structure on the test data (created from the data as explained above).
 */
double mlp_model_selection(Instanceptr* data, MODEL_SELECTION_TYPE modelselectionmethod, int* hiddennodes, int maxhidden, double* bestalpha)
{
	/*!Last Changed 25.06.2005*/
 char varname[MAXLENGTH];
 char varvalue[MAXLENGTH], pst[READLINELENGTH];
	Mlpnetwork best, neuralnetwork;
	double a, alphaerror, besterror = +INT_MAX, result, besta;
	matrix alpha, test;
	int i, problemtype, totalrun = 1;
	Instanceptr testdata, data1, data2, data3, alphavalidation, epochvalidation;
	if (current_dataset->classno == 0)
		 problemtype = REGRESSION;
	else
		 problemtype = CLASSIFICATION;
	divide_instancelist(data, &testdata, &data1, 5);
	divide_instancelist(&data1, &alphavalidation, &data2, 5);
	divide_instancelist(&data2, &epochvalidation, &data3, 5);
	alpha = instancelist_to_matrix(alphavalidation, problemtype, -1, MULTIVARIATE_LINEAR);
	test = instancelist_to_matrix(testdata, problemtype, -1, MULTIVARIATE_LINEAR);
	/*Exponential search*/
	for (a = 0.001; a < 100; a *= 2)
	 {
  	switch (modelselectionmethod)
			 {
			  case MODEL_SELECTION_CV :neuralnetwork = mlp_model_selection_crossvalidation(&data3, epochvalidation, a, maxhidden, problemtype);
				                          break;
	    case MODEL_SELECTION_AIC:neuralnetwork = mlp_model_selection_aic(data3, epochvalidation, a, maxhidden, problemtype);
			  	                        break;
			  case MODEL_SELECTION_BIC:neuralnetwork = mlp_model_selection_bic(data3, epochvalidation, a, maxhidden, problemtype);
				                          break;
     default                 :break;
			 }
			alphaerror = test_mlp_network(neuralnetwork, alpha);
			if (a != 0.001 && alphaerror < besterror)
				 free_mlpnnetwork(&best);
			if (a == 0.001 || alphaerror < besterror)
			 {
					besta = a;
					best = copy_of_Mlpnetwork(neuralnetwork);
					besterror = alphaerror;
			 }
	  sprintf(varname, "aout1[%d]", totalrun);
	  sprintf(varvalue, "%d", neuralnetwork.P.hidden[0]);
  	set_variable(varname, varvalue);
	  sprintf(varname, "aout2[%d]", totalrun);
			set_precision(pst, NULL, NULL);
	  sprintf(varvalue, pst, alphaerror);
  	set_variable(varname, varvalue);
   totalrun++;
			free_mlpnnetwork(&neuralnetwork);
	 }
	/*Linear search*/
	for (i = 1; i < 12; i++)
	 {
		 a = besta / 2 + (i * besta) / 8;
  	switch (modelselectionmethod)
			 {
			  case MODEL_SELECTION_CV :neuralnetwork = mlp_model_selection_crossvalidation(&data3, epochvalidation, a, maxhidden, problemtype);
				                          break;
	    case MODEL_SELECTION_AIC:neuralnetwork = mlp_model_selection_aic(data3, epochvalidation, a, maxhidden, problemtype);
			  	                        break;
			  case MODEL_SELECTION_BIC:neuralnetwork = mlp_model_selection_bic(data3, epochvalidation, a, maxhidden, problemtype);
				                          break;
     default                 :break;
			 }
			alphaerror = test_mlp_network(neuralnetwork, alpha);
			if (alphaerror < besterror)
			 {
	  		free_mlpnnetwork(&best);
					best = copy_of_Mlpnetwork(neuralnetwork);
					besta = a;
					besterror = alphaerror;
			 }
	  sprintf(varname, "aout1[%d]", totalrun);
	  sprintf(varvalue, "%d", neuralnetwork.P.hidden[0]);
   set_variable(varname, varvalue);
	  sprintf(varname, "aout2[%d]", totalrun);
			set_precision(pst, NULL, NULL);
	  sprintf(varvalue, pst, alphaerror);
  	set_variable(varname, varvalue);
   totalrun++;
			free_mlpnnetwork(&neuralnetwork);
	 }
	matrix_free(alpha);
	*hiddennodes = best.P.hidden[0];
	result = test_mlp_network(best, test);
	free_mlpnnetwork(&best);
	matrix_free(test);
	deallocate(testdata);
	deallocate(alphavalidation);
	deallocate(epochvalidation);
	deallocate(data3);
	*bestalpha = besta;
	return result;
}

/**
 * Calculates Akaike Information Criterion generalization error estimate. The formula is: ((N + d) / (N - d)) * error
 * @param[in] error Training error of the classifier
 * @param[in] d Number of features of the dataset
 * @param[in] N Number of instances in the training set.
 * @return Akaike Information Criterion generalization error estimate
 */
double aic_squared_error(double error, double d, int N)
{
	/*!Last Changed 22.01.2005*/
	double dim;
 dim = (d + 0.0) / N;
 return ((1 + dim) / (1 - dim)) * error;
}

/**
 * Calculates Akaike Information Criterion generalization error estimate. The formula is: 2 sigma (d / N) + error
 * @param[in] error Training error of the classifier
 * @param[in] d Number of features of the dataset
 * @param[in] N Number of instances in the training set.
 * @param[in] sigma Sigma parameter of the dataset.
 * @return Akaike Information Criterion generalization error estimate
 */
double aic_squared_error_with_sigma(double error, double d, int N, double sigma)
{
	/*!Last Changed 10.02.2005*/
	double dim;
 dim = (d + 0.0) / N;
 return error + 2 * sigma * dim;
}

/**
 * Calculates Akaike Information Criterion based on the log likelihood of the data. The formula is: -L + d
 * @param[in] loglikelihood Log likelihood of the training data (L)
 * @param[in] d Number of dimensions of the dataset
 * @return Akaike Information Criterion based on the log likelihood of the data
 */
double aic_loglikelihood(double loglikelihood, double d)
{
	/*!Last Changed 22.01.2005*/
	return -loglikelihood + d;
}

/**
 * Calculates Bayesian Information Criterion generalization error estimate. The formula is: ((N + d * (log N - 1)) / (N - d)) * error
 * @param[in] error Training error of the classifier
 * @param[in] d Number of features of the dataset
 * @param[in] N Number of instances in the training set.
 * @return Bayesian Information Criterion generalization error estimate
 */
double bic_squared_error(double error, double d, int N)
{
	/*!Last Changed 22.01.2005*/
	double nom, denom;
	nom = N + d * (log(N) - 1);
	denom = N - d;
 return (nom / denom) * error;
}

/**
 * Calculates Bayesian Information Criterion generalization error estimate. The formula is: 2 d log N sigma (d / N) + error
 * @param[in] error Training error of the classifier
 * @param[in] d Number of features of the dataset
 * @param[in] N Number of instances in the training set.
 * @param[in] sigma Sigma parameter of the dataset.
 * @return Bayesian Information Criterion generalization error estimate
 */
double bic_squared_error_with_sigma(double error, double d, int N, double sigma)
{
	/*!Last Changed 10.02.2005*/
	double dim;
	dim = (d + 0.0) / N;
 return error + log(N) * dim * sigma;
}

/**
 * Calculates Bayesian Information Criterion based on the log likelihood of the data. The formula is: -L + d log N / 2
 * @param[in] loglikelihood Log likelihood of the training data (L)
 * @param[in] d Number of features of the dataset
 * @param[in] N Number of instances in the training set.
 * @return Bayesian Information Criterion based on the log likelihood of the data
 */
double bic_loglikelihood(double loglikelihood, double d, int N)
{
	/*!Last Changed 22.01.2005*/
	return -loglikelihood + (d * log(N)) / 2;
}

/**
 * Calculates epsilon value in the generalization error formula(s) of the Structural Risk Minimization model selection technique.
 * @param[in] h VC-dimension of the classifier
 * @param[in] N Number of instances in the training set.
 * @param[in] a1 First parameter of SRM
 * @param[in] a2 Second parameter of SRM
 * @return Epsilon value in the generalization error formula of SRM
 */
double srm_epsilon(int h, int N, double a1, double a2)
{
	/*!Last Changed 22.01.2005*/
	double nom, denom, nu;
	nu = 0.01;
	nom = h * (log((a2 * N) / (h + 0.0)) + 1) - log (nu / 4.0);
	denom = N;
	return (a1 * nom) / denom;
}

/**
 * Calculates generalization error of a classifier based on Structural Risk Minimization.
 * @param[in] error Training error of the classifier
 * @param[in] h VC-dimension of the classifier
 * @param[in] N Number of instances in the training set.
 * @param[in] a1 First parameter of SRM
 * @param[in] a2 Second parameter of SRM
 * @return Generalization error of a classifier based on SRM
 */
double srm_classification(double error, int h, int N, double a1, double a2)
{
	/*!Last Changed 22.01.2005*/
 double epsilon, upperbound;
	epsilon = srm_epsilon(h, N, a1, a2);
 upperbound = error + (epsilon / 2) * (1 + sqrt(1 + (4 * error) / epsilon));
	return upperbound;
}

/**
 * Calculates generalization error of a regressor based on Structural Risk Minimization.
 * @param[in] error Training error (mean square error) of the regressor
 * @param[in] h VC-dimension of the regressor
 * @param[in] N Number of instances in the training set.
 * @param[in] a1 First parameter of SRM
 * @param[in] a2 Second parameter of SRM
 * @return Generalization error of a classifier based on SRM
 */
double srm_regression(double error, int h, int N, double a1, double a2)
{
	/*!Last Changed 22.01.2005*/
	double epsilon, upperbound;
	int c;
	c = 1;
	epsilon = srm_epsilon(h, N, a1, a2);
	upperbound = error / (1 - c * sqrt(epsilon));
	return upperbound;
}

/**
 * Calculates total description length of a model based on Minimum Description Length principle
 * @param[in] error Training error of the classifier
 * @param[in] d Number of features of the dataset
 * @param[in] lambda Model complexity in terms of number of parameters
 * @param[in] number_of_bits Number of bits to represent one parameter
 * @return Total description length of a model based on Minimum Description Length principle
 */
double mdl(double error, int d, double lambda, int number_of_bits)
{
	/*!Last Changed 22.01.2005*/
	return error + d * lambda * number_of_bits;
}

/**
 * Checks if the current candidate model is added to the candidate list. If it is already in the list, the function does nothing. Otherwise, it adds the current model to the candidate model list.
 * @param[out] filenames Error rate file names of the models in the candidate list
 * @param[in] directory The directory in which the error rate files reside in.
 * @param[in] model Current candidate model to be searched
 * @param[out] models Candidate model list
 * @param[out] filecount Number of models in the candidate model list (Size of the models and filenames arrays)
 */
void search_and_add_candidate_model(char** filenames, char* directory, int model, int* models, int* filecount)
{
	/*!Last Changed 02.06.2004*/
	int i;
	char filename[READLINELENGTH];
	for (i = 0; i < *filecount; i++)
		 if (models[i] == model)
				 break;
	if (i == *filecount)
	 {
		 (*filecount)++;
			sprintf(filename,"%s-%d.txt", directory, model);
			strcpy(filenames[(*filecount) - 1], filename); 
   models[(*filecount) - 1] = model;
	 }
}

void hmm_add_candidate_model(char** filenames, char* directory, int index, Hmm_modelptr models, int* models_searched, int* filecount)
{
 /*!Last Changed 02.06.2004*/
 char filename[READLINELENGTH];
 (*filecount)++;
 models[index].tried = BOOLEAN_TRUE;
 models_searched[(*filecount) - 1] = index;
 sprintf(filename,"%s/%dM%dS.txt", directory, models[index].mixture, models[index].states);
 strcpy(filenames[(*filecount) - 1], filename); 
}

Hmm_model hmm_model_simulation_multitest(char* simulationdirectory, int maxstates, int maxmixture, int* tries)
{
 /*!Last Changed 10.01.2009*/
 char **filenames, directory[500], tmpst[500];
 int filecount, i, j, k, m, c, *model, *models_searched, current_best_index;
 Hmm_modelptr bestmodel;
 Hmm_modelptr models;
 Hmm_model result;
 models = safemalloc(sizeof(Hmm_model) * maxstates * maxmixture, "hmm_model_simulation_multitest", 4);
 models_searched = safemalloc(sizeof(int) * MAX_MODEL_SEARCH, "hmm_model_simulation_multitest", 4);
 k = 0;
 for (i = 1; i <= maxstates; i++)
   for (j = 1; j <= maxmixture; j++)
    {
     models[k].states = i;
     models[k].mixture = j;
     models[k].index = k;
     models[k].selected = BOOLEAN_FALSE;
     models[k].tried = BOOLEAN_FALSE;
     k++;
    }
 bestmodel = &(models[0]);
 bestmodel->selected = BOOLEAN_TRUE;
 filenames = (char **) safemalloc_2d(sizeof(char *), sizeof(char), MAX_MODEL_SEARCH, 500, "hmm_model_simulation_multitest", 3);
 sprintf(directory,"%s", simulationdirectory);
 while (BOOLEAN_TRUE)
  {
   filecount = 0;
   /*Remove one state*/
   if (bestmodel->states > 1)
     hmm_add_candidate_model(filenames, directory, bestmodel->index - maxmixture, models, models_searched, &filecount);
   /*Remove one mixture*/
   if (bestmodel->mixture > 1)
     hmm_add_candidate_model(filenames, directory, bestmodel->index - 1, models, models_searched, &filecount);
   /*Add best model to comparison*/
   hmm_add_candidate_model(filenames, directory, bestmodel->index, models, models_searched, &filecount);
   /*Add one mixture*/
   if (bestmodel->mixture < maxmixture)
     hmm_add_candidate_model(filenames, directory, bestmodel->index + 1, models, models_searched, &filecount);
   /*Add one state*/
   if (bestmodel->states < maxstates)
     hmm_add_candidate_model(filenames, directory, bestmodel->index + maxmixture, models, models_searched, &filecount);
   sprintf(tmpst,"-");
   model = multiple_comparison(filenames, filecount, get_parameter_l("testtype"), tmpst, &m, &c, get_parameter_l("correctiontype"));
   current_best_index = models_searched[model[0] - 1]; 
   if (current_best_index == bestmodel->index)
    {
     safe_free(model);
     break;
    }
   else
    {
     if (models[current_best_index].selected)
       break;
     bestmodel = &(models[current_best_index]);
     bestmodel->selected = BOOLEAN_TRUE;
    }
   safe_free(model);
  }
 *tries = 0;
 for (i = 0; i < maxstates * maxmixture; i++)
   if (models[i].tried)
     (*tries)++;
 result = *bestmodel;
 free_2d((void**)filenames, MAX_MODEL_SEARCH);
 safe_free(models);
 safe_free(models_searched);
 return result;
}

int mlp_model_simulation_multitest(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries)
{
 /*!Last Changed 26.05.2006 added starthidden*/
	/*!Last Changed 29.10.2005*/
	/*!Last Changed 02.06.2004*/
	char **filenames, filename[500], directory[500], tmpst[500];
	int numberofclasses, bestmodel, filecount, models[MAX_MODEL_SEARCH], i, j, tmp, m, c, *model, *modelselected, *modelstried;
 if (starthidden < 0 || starthidden > maxhidden)
   bestmodel = 0;
 else
   bestmodel = starthidden;
	filenames = (char **) safemalloc_2d(sizeof(char *), sizeof(char), MAX_MODEL_SEARCH, 500, "mlp_model_simulation", 3);
	modelselected = (int *) safecalloc(maxhidden + 1, sizeof(int), "mlp_model_simulation", 4);
	modelstried = (int *) safecalloc(maxhidden + 1, sizeof(int), "mlp_model_simulation", 5);
	modelselected[bestmodel] = 1;
	modelstried[bestmodel] = 1;
	sprintf(directory,"%s/%s", simulationdirectory, d->name);
	if (d->classno == 0)
		 numberofclasses = 1;
	else
		 numberofclasses = d->classno;
	while (1)
	 {
		 filecount = 0;
		 search_and_add_candidate_model(filenames, directory, bestmodel, models, &filecount);
		 if (bestmodel == 0)
			 {
				 if (numberofclasses <= maxhidden)
					 {
						 search_and_add_candidate_model(filenames, directory, numberofclasses, models, &filecount);
							if (2 * numberofclasses <= maxhidden)
								 search_and_add_candidate_model(filenames, directory, 2 * numberofclasses, models, &filecount);
							else
						   search_and_add_candidate_model(filenames, directory, maxhidden, models, &filecount);
					 }
					else
						 search_and_add_candidate_model(filenames, directory, maxhidden, models, &filecount);
					if ((numberofclasses + d->multifeaturecount) / 4 <= maxhidden)
					 {
						 search_and_add_candidate_model(filenames, directory, (numberofclasses + d->multifeaturecount) / 4, models, &filecount);
							if ((numberofclasses + d->multifeaturecount) / 2 <= maxhidden)
								 search_and_add_candidate_model(filenames, directory, (numberofclasses + d->multifeaturecount) / 2, models, &filecount);
							else
						   search_and_add_candidate_model(filenames, directory, maxhidden, models, &filecount);
					 }
					else
						 search_and_add_candidate_model(filenames, directory, maxhidden, models, &filecount);
			 }
			else
			 {
				 if (bestmodel + 1 <= maxhidden)
					 {
  				 search_and_add_candidate_model(filenames, directory, bestmodel + 1, models, &filecount);
							if (1.5 * bestmodel <= maxhidden)
								 search_and_add_candidate_model(filenames, directory, (int) (1.5 * bestmodel), models, &filecount);
							else
         search_and_add_candidate_model(filenames, directory, maxhidden, models, &filecount);
					 }
					else
						 search_and_add_candidate_model(filenames, directory, maxhidden, models, &filecount);
					search_and_add_candidate_model(filenames, directory, bestmodel - 1, models, &filecount);
     search_and_add_candidate_model(filenames, directory, (int) (0.5 * bestmodel), models, &filecount);
			 }
			if (filecount == 1)
				 break;
			for (i = 0; i < filecount; i++)
				 for (j = i + 1; j < filecount; j++)
						 if (models[i] > models[j])
							 {
								 tmp = models[i];
									models[i] = models[j];
									models[j] = tmp;
									strcpy(filename, filenames[i]);
									strcpy(filenames[i], filenames[j]);
									strcpy(filenames[j], filename);
							 }
			sprintf(tmpst,"-");
			for (i = 0; i < filecount; i++)
				 modelstried[models[i]] = 1;
			model = multiple_comparison(filenames, filecount, get_parameter_l("testtype"), tmpst, &m, &c, get_parameter_l("correctiontype"));
			if (models[model[0] - 1] == bestmodel)
			 {
				 safe_free(model);
				 break;
			 }
			else
			 {
				 if (modelselected[models[model[0] - 1]])
						 break;
				 bestmodel = models[model[0] - 1];
					modelselected[bestmodel] = 1;
			 }
			safe_free(model);
	 }
	*tries = 0;
	for (i = 0; i <= maxhidden; i++)
		 if (modelstried[i])
				 (*tries)++;
	free_2d((void**)filenames, MAX_MODEL_SEARCH);
	safe_free(modelselected);
	safe_free(modelstried);
 return bestmodel;
}

int* generate_candidate_mlp_models(Datasetptr d, int maxhidden, int bestmodel, int*modelcount, MODEL_SEARCH_DIRECTION_TYPE searchdirection)
{
	/*!Last Changed 29.10.2005*/
	/*!Last Changed 17.07.2005*/
	int numberofclasses, addmaxhidden = 0, *list;
	list = safemalloc(MAX_MODEL_SEARCH * sizeof(int), "generate_candidate_mlp_models", 2);
	*modelcount = 0;
	if (d->classno == 0)
		 numberofclasses = 1;
	else
		 numberofclasses = d->classno;
	search_and_add_into_list(list, modelcount, bestmodel);
	if (bestmodel == 0)
		{
			if (numberofclasses < maxhidden)
				{
					search_and_add_into_list(list, modelcount, numberofclasses);
					if (2 * numberofclasses < maxhidden)
						 search_and_add_into_list(list, modelcount, 2 * numberofclasses);
					else
							addmaxhidden = 1;
				}
			else
					addmaxhidden = 1;
			if ((numberofclasses + d->multifeaturecount) / 4 <= maxhidden)
				{
					search_and_add_into_list(list, modelcount, (numberofclasses + d->multifeaturecount) / 4);
					if ((numberofclasses + d->multifeaturecount) / 2 <= maxhidden)
							search_and_add_into_list(list, modelcount, (numberofclasses + d->multifeaturecount) / 2);
					else
							addmaxhidden = 1;
				}
			else
					addmaxhidden = 1;
		}
	else
		{
			if (bestmodel + 1 < maxhidden)
				{
					search_and_add_into_list(list, modelcount, bestmodel + 1);
					if (bestmodel * 1.5 < maxhidden)
							search_and_add_into_list(list, modelcount, (int) (bestmodel * 1.5));
					else
							addmaxhidden = 1;
				}
			else
					addmaxhidden = 1;
   if (bestmodel - 1 > 0)
					search_and_add_into_list(list, modelcount, bestmodel - 1);
			if (0.5 * bestmodel > 1)
					search_and_add_into_list(list, modelcount, (int) (0.5 * bestmodel));
		}
	if (addmaxhidden)
			search_and_add_into_list(list, modelcount, maxhidden);
	if (searchdirection == BACKWARD)
		 search_and_add_into_list(list, modelcount, 0);
	return list;
}

Hmm_model hmm_model_simulation_most(char* simulationdirectory, int maxstates, int maxmixture, int* tries, MODEL_SEARCH_DIRECTION_TYPE searchdirection)
{
 /*!Last Changed 10.01.2009*/
 char **two_file;
 int *list, modelcount, i, j, k, bestoftwo;
 BOOLEAN improved = BOOLEAN_TRUE;
 Hmm_modelptr bestmodel;
 Hmm_model result;
 Hmm_modelptr models;
 list = safemalloc(sizeof(int) * MAX_MODEL_SEARCH, "hmm_model_simulation_most", 4);
 models = safemalloc(sizeof(Hmm_model) * maxstates * maxmixture, "hmm_model_simulation_most", 5);
 k = 0;
 for (i = 1; i <= maxstates; i++)
   for (j = 1; j <= maxmixture; j++)
    {
     models[k].states = i;
     models[k].mixture = j;
     models[k].index = k;
     models[k].selected = BOOLEAN_FALSE;
     models[k].tried = BOOLEAN_FALSE;
     k++;
    }
 if (searchdirection == FORWARD)
   bestmodel = &(models[0]);
 else
   bestmodel = &(models[k - 1]);
 bestmodel->tried = BOOLEAN_TRUE;
 bestmodel->selected = BOOLEAN_TRUE;
 two_file = (char**) safemalloc_2d(sizeof(char *), sizeof(char), 2, 500, "hmm_model_simulation_most", 20);
 while (improved)
  {
   modelcount = 0;
   /*Remove one state*/
   if (bestmodel->states > 1)
     list[modelcount++] = bestmodel->index - maxmixture;
   /*Remove one mixture*/
   if (bestmodel->mixture > 1)
     list[modelcount++] = bestmodel->index - 1;
   /*Add best model to comparison*/
   list[modelcount++] = bestmodel->index;
   /*Add one mixture*/
   if (bestmodel->mixture < maxmixture)
     list[modelcount++] = bestmodel->index + 1;
   /*Add one state*/
   if (bestmodel->states < maxstates)
     list[modelcount++] = bestmodel->index + maxmixture;
   if (searchdirection == FORWARD)
     qsort(list, modelcount, sizeof(int), sort_function_int_ascending);
   else
     qsort(list, modelcount, sizeof(int), sort_function_int_descending);
   improved = BOOLEAN_FALSE;
   for (i = 0; i < modelcount; i++)
     if (list[i] != bestmodel->index)
      {
       models[list[i]].tried = BOOLEAN_TRUE;
       bestoftwo = compare_two_hmm_models(two_file, simulationdirectory, *bestmodel, models[list[i]]);
       if ((bestmodel->index < list[i] && bestoftwo == 1) || (bestmodel->index > list[i] && bestoftwo == 0))
        {
         bestmodel = &(models[list[i]]);
         improved = BOOLEAN_TRUE;
         break;
        }
      }
   if (bestmodel->selected)
     improved = BOOLEAN_FALSE;
   else
     bestmodel->selected = BOOLEAN_TRUE;
  }
 safe_free(list);
 *tries = 0;
 for (i = 0; i < maxstates * maxmixture; i++)
   if (models[i].tried)
     (*tries)++;
 result = *bestmodel;
 safe_free(models);
 free_2d((void**)two_file, 2);
 return result;
}

int mlp_model_simulation_most(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries, MODEL_SEARCH_DIRECTION_TYPE searchdirection)
{
 /*!Last Changed 26.05.2006 added starthidden*/
	/*!Last Changed 17.07.2005*/
	char **two_file;
	int bestmodel, *list, modelcount, i, improved = 1, bestoftwo, *bestmodels, bestmodelcount = 1, *modelstried;
	modelstried = (int *) safecalloc(maxhidden + 1, sizeof(int), "mlp_model_simulation_most", 3);
 if (starthidden < 0 || starthidden > maxhidden)
  {
  	if (searchdirection == FORWARD)
		   bestmodel = 0;
	  else
  		 bestmodel = maxhidden;
  }
 else
   bestmodel = starthidden;
	modelstried[bestmodel] = 1;
	bestmodels = safemalloc((maxhidden + 2) * sizeof(int), "mlp_model_simulation_forw1", 9);
	bestmodels[0] = bestmodel;
 two_file = (char**) safemalloc_2d(sizeof(char *), sizeof(char), 2, 500, "mlp_model_simulation_forw1", 9);
	while (improved)
	 {
		 list = generate_candidate_mlp_models(d, maxhidden, bestmodel, &modelcount, searchdirection); 
			if (searchdirection == FORWARD)
  			qsort(list, modelcount, sizeof(int), sort_function_int_ascending);
			else
				 qsort(list, modelcount, sizeof(int), sort_function_int_descending);
			improved = 0;
			for (i = 0; i < modelcount; i++)
				 if (list[i] != bestmodel)
					 {
						 modelstried[list[i]] = 1;
			  	 bestoftwo = compare_two_mlp_models(two_file, simulationdirectory, d, bestmodel, list[i]);
			  	 if ((bestmodel < list[i] && bestoftwo == 1) || (bestmodel > list[i] && bestoftwo == 0))
							 {
				  		 bestmodel = list[i];
				  			improved = 1;
				  			break;
							 }
					 }
   for (i = 0; i < bestmodelcount; i++)
				 if (bestmodels[i] == bestmodel)
					 {
						 improved = 0;
							break;
					 }
			bestmodels[bestmodelcount] = bestmodel;
			bestmodelcount++;
			safe_free(list);
	 }
	*tries = 0;
	for (i = 0; i <= maxhidden; i++)
		 if (modelstried[i])
				 (*tries)++;
	safe_free(bestmodels);
	safe_free(modelstried);
	free_2d((void**)two_file, 2);
	return bestmodel;
}

int compare_two_hmm_models(char** two_file, char* simulationdirectory, Hmm_model model1, Hmm_model model2)
{
 /*!Last Changed 10.01.2009*/
 int dummy;
 double confidence = 1 - get_parameter_f("confidencelevel"), conf;
 if (model1.index < model2.index)
  {
   sprintf(two_file[0], "%s/%dM%dS.txt", simulationdirectory, model1.mixture, model1.states);
   sprintf(two_file[1], "%s/%dM%dS.txt", simulationdirectory, model2.mixture, model2.states);
  }
 else
  {
   sprintf(two_file[0], "%s/%dM%dS.txt", simulationdirectory, model2.mixture, model2.states);
   sprintf(two_file[1], "%s/%dM%dS.txt", simulationdirectory, model1.mixture, model1.states);
  }
 return compare_two_files_using_test(get_parameter_l("testtype"), two_file, &dummy, confidence, &conf);
}

int compare_two_mlp_models(char** two_file, char* simulationdirectory, Datasetptr d, int model1, int model2)
{
	/*!Last Changed 17.07.2005*/
	int min, max, dummy;
	double confidence = 1 - get_parameter_f("confidencelevel"), conf;
	min = imin(model1, model2);
	max = model1 + model2 - min;
	sprintf(two_file[0], "%s/%s-%d.txt", simulationdirectory, d->name, min);
	sprintf(two_file[1], "%s/%s-%d.txt", simulationdirectory, d->name, max);
 return compare_two_files_using_test(get_parameter_l("testtype"), two_file, &dummy, confidence, &conf);
}

Hmm_model hmm_model_simulation_one_forward(char* simulationdirectory, int maxstates, int* tries)
{
 /*!Last Changed 10.01.2009*/
 char **two_file;
 int i;
 Hmm_model bestmodel;
 Hmm_modelptr models;
 models = safemalloc(sizeof(Hmm_model) * maxstates, "hmm_model_simulation_one_forward", 4);
 for (i = 0; i < maxstates; i++)
  {
   models[i].states = i + 1;
   models[i].mixture = 1;
   models[i].index = i;
  }
 bestmodel = models[0];
 two_file = (char**) safemalloc_2d(sizeof(char *), sizeof(char), 2, 500, "hmm_model_simulation_one_forward", 12);
 *tries = 1;
 while (bestmodel.index < maxstates)
  {
   (*tries)++;
   if (compare_two_hmm_models(two_file, simulationdirectory, bestmodel, models[bestmodel.index + 1]) == 0)
     break;
   bestmodel = models[bestmodel.index + 1];
  }
 free_2d((void**)two_file, 2);
 safe_free(models);
 return bestmodel;
}

int mlp_model_simulation_one_forward(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries)
{
 /*!Last Changed 26.05.2006 added starthidden*/
	/*!Last Changed 16.07.2005*/
	char **two_file;
	int bestmodel;
 if (starthidden < 0 || starthidden > maxhidden)
   bestmodel = 0;
 else
   bestmodel = starthidden;
 two_file = (char**) safemalloc_2d(sizeof(char *), sizeof(char), 2, 500, "mlp_model_simulation_forward", 4);
	*tries = 1;
	while (bestmodel < maxhidden)
	 {
			(*tries)++;
			if (compare_two_mlp_models(two_file, simulationdirectory, d, bestmodel, bestmodel + 1) == 0)
				 break;
			bestmodel++;
	 }
	free_2d((void**)two_file, 2);
	return bestmodel;
}

Hmm_model hmm_model_simulation_one_backward(char* simulationdirectory, int maxstates, int* tries)
{
 /*!Last Changed 10.01.2009*/
 char **two_file;
 int i;
 Hmm_model bestmodel;
 Hmm_modelptr models;
 models = safemalloc(sizeof(Hmm_model) * maxstates, "hmm_model_simulation_one_backward", 4);
 for (i = 0; i < maxstates; i++)
  {
   models[i].states = i + 1;
   models[i].mixture = 1;
   models[i].index = i;
  }
 bestmodel = models[maxstates - 1];
 two_file = (char**) safemalloc_2d(sizeof(char *), sizeof(char), 2, 500, "hmm_model_simulation_one_backward", 12);
 *tries = 1;
 while (bestmodel.index > 0)
  {
   (*tries)++;
   if (compare_two_hmm_models(two_file, simulationdirectory, bestmodel, models[bestmodel.index - 1]) == 0)
     break;
   bestmodel = models[bestmodel.index - 1];
  }
 safe_free(models);
 free_2d((void**)two_file, 2);
 return bestmodel;
}

int mlp_model_simulation_one_backward(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries)
{
 /*!Last Changed 26.05.2006 added starthidden*/
	/*!Last Changed 16.07.2005*/
	char **two_file;
	int bestmodel;
 if (starthidden < 0 || starthidden > maxhidden)
   bestmodel = maxhidden;
 else
   bestmodel = starthidden;
 two_file = (char**) safemalloc_2d(sizeof(char *), sizeof(char), 2, 500, "mlp_model_simulation_backward", 4);
	*tries = 1;
	while (bestmodel > 0)
	 {
			(*tries)++;
			if (compare_two_mlp_models(two_file, simulationdirectory, d, bestmodel - 1, bestmodel) == 1)
				 break;
			bestmodel--;
	 }
	free_2d((void**)two_file, 2);
	return bestmodel;
}

Hmm_model hmm_model_simulation(MOST_SEARCH_TYPE simulation_algorithm, char* simulationdirectory, int maxstates, int maxmixture, int* tries)
{
 /*!Last Changed 10.01.2009*/
 switch (simulation_algorithm)
  {
   case MULTIMOST    :return hmm_model_simulation_multitest(simulationdirectory, maxstates, maxmixture, tries);
                      break;
   case ONE_FORWARD  :return hmm_model_simulation_one_forward(simulationdirectory, maxstates, tries);
                      break;
   case ONE_BACKWARD :return hmm_model_simulation_one_backward(simulationdirectory, maxstates, tries);
                      break;
   case FORWARD_MOST :return hmm_model_simulation_most(simulationdirectory, maxstates, maxmixture, tries, FORWARD);
                      break;
   case BACKWARD_MOST:return hmm_model_simulation_most(simulationdirectory, maxstates, maxmixture, tries, BACKWARD);
                      break;
   default           :printf(m1242, simulation_algorithm);
                      return hmm_model_simulation_multitest(simulationdirectory, maxstates, maxmixture, tries);
                      break;
  }
}

int mlp_model_simulation(MOST_SEARCH_TYPE simulation_algorithm, char* datasetname, char* simulationdirectory, int maxhidden, int starthidden, int* tries)
{
	/*!Last Changed 16.07.2005*/
	Datasetptr d;
 d = search_dataset(Datasets, datasetname);
	if (d == NULL)
		 return 0;
	switch (simulation_algorithm)
	 {
	  case MULTIMOST    :return mlp_model_simulation_multitest(d, simulationdirectory, maxhidden, starthidden, tries);
				                  break;
			case ONE_FORWARD  :return mlp_model_simulation_one_forward(d, simulationdirectory, maxhidden, starthidden, tries);
				                  break;
			case ONE_BACKWARD :return mlp_model_simulation_one_backward(d, simulationdirectory, maxhidden, starthidden, tries);
				                  break;
			case FORWARD_MOST :return mlp_model_simulation_most(d, simulationdirectory, maxhidden, starthidden, tries, FORWARD);
				                  break;
			case BACKWARD_MOST:return mlp_model_simulation_most(d, simulationdirectory, maxhidden, starthidden, tries, BACKWARD);
				                  break;
   default           :printf(m1242, simulation_algorithm);
                      break;
	 }
	return 0;
}

int compare_mlp_model_selection_techniques(char *datasetname, char* simulationdirectory, int maxhidden, int starthidden)
{
	/*!Last Changed 18.07.2005*/
	char** filenames, filename[500], tmpst[READLINELENGTH], varname[MAXLENGTH], varvalue[MAXLENGTH];
	int* model, dummy, dummy2, i, j, k, selectedmodels[MAX_MLP_SIMULATION_ALGORITHMS], tries[MAX_MLP_SIMULATION_ALGORITHMS], bestmodel, indexes[MAX_MLP_SIMULATION_ALGORITHMS], sum;
	Datasetptr d;
 d = search_dataset(Datasets, datasetname);
	if (d == NULL)
		 return -1;
	filenames = safemalloc((maxhidden + 1) * sizeof(char *), "compare_mlp_model_selection_techniques", 7);
 for (i = 0; i <= maxhidden; i++)
	 {
		 sprintf(filename, "%s/%s-%d.txt", simulationdirectory, d->name, i);
			filenames[i] = strcopy(filenames[i], filename);
	 }
	strcpy(tmpst, "-");
 model = multiple_comparison(filenames, maxhidden + 1, get_parameter_l("testtype"), tmpst, &dummy, &dummy2, get_parameter_l("correctiontype"));
	free_2d((void**)filenames, maxhidden + 1);
	for (i = 0; i < MAX_MLP_SIMULATION_ALGORITHMS; i++)
	 {
		 switch (i)
	   {
		   case MULTIMOST    :bestmodel = mlp_model_simulation_multitest(d, simulationdirectory, maxhidden, starthidden, &(tries[i]));
						                  break;
					case ONE_FORWARD  :bestmodel = mlp_model_simulation_one_forward(d, simulationdirectory, maxhidden, starthidden, &(tries[i]));
						                  break;
					case ONE_BACKWARD :bestmodel = mlp_model_simulation_one_backward(d, simulationdirectory, maxhidden, starthidden, &(tries[i]));
						                  break;
					case FORWARD_MOST :bestmodel = mlp_model_simulation_most(d, simulationdirectory, maxhidden, starthidden, &(tries[i]), FORWARD);
						                  break;
					case BACKWARD_MOST:bestmodel = mlp_model_simulation_most(d, simulationdirectory, maxhidden, starthidden, &(tries[i]), BACKWARD);
						                  break;
    }
			selectedmodels[i] = search_list(model, maxhidden + 1, bestmodel + 1);
	  sprintf(varname, "aout2[%d]", i + 1);
	  sprintf(varvalue, "%d", selectedmodels[i]);
   set_variable(varname, varvalue);
			indexes[i] = i;
	 }
	for (i = 0; i < MAX_MLP_SIMULATION_ALGORITHMS; i++)
		 for (j = i + 1; j < MAX_MLP_SIMULATION_ALGORITHMS; j++)
				 if (selectedmodels[i] > selectedmodels[j])
					 {
						 swap_int(&(selectedmodels[i]), &(selectedmodels[j]));
						 swap_int(&(indexes[i]), &(indexes[j]));
							swap_int(&(tries[i]), &(tries[j]));
					 }
					else
						 if (selectedmodels[i] == selectedmodels[j] && tries[i] > tries[j])
							 {
  						 swap_int(&(selectedmodels[i]), &(selectedmodels[j]));
	  					 swap_int(&(indexes[i]), &(indexes[j]));
  							swap_int(&(tries[i]), &(tries[j]));
							 }
 i = 0;
	while (i < MAX_MLP_SIMULATION_ALGORITHMS)
	 {
		 j = i;
			sum = j + 1;
		 while (j < MAX_MLP_SIMULATION_ALGORITHMS - 1 && selectedmodels[j] == selectedmodels[j + 1] && tries[j] == tries[j + 1])
			 {
				 j++;
				 sum = sum + j + 1;
			 }
			for (k = i; k <= j; k++)
			 {
	    sprintf(varname, "aout1[%d]", indexes[k] + 1);
	    sprintf(varvalue, "%.1f", sum / (j - i + 1.0));
     set_variable(varname, varvalue);
			 }
			i = j + 1;
	 }
	return model[0] - 1;
}

Hmm_model compare_hmm_model_selection_techniques(char* simulationdirectory, int maxstates, int maxmixture)
{
 /*!Last Changed 12.04.2009*/
 char** filenames, filename[500], tmpst[READLINELENGTH], varname[MAXLENGTH], varvalue[MAXLENGTH];
 int* model, dummy, dummy2, i, j, k, selectedmodels[MAX_MLP_SIMULATION_ALGORITHMS], modelcount, tries[MAX_MLP_SIMULATION_ALGORITHMS], indexes[MAX_MLP_SIMULATION_ALGORITHMS], sum;
 Hmm_model bestmodel, *models;
 modelcount = maxstates * maxmixture;
 filenames = safemalloc(modelcount * sizeof(char *), "compare_hmm_model_selection_techniques", 5);
 models = safemalloc(modelcount * sizeof(Hmm_model), "compare_hmm_model_selection_techniques", 6);
 k = 0;
 for (i = 1; i <= maxstates; i++)
   for (j = 1; j <= maxmixture; j++)
    {
     sprintf(filename, "%s/%dM%dS.txt", simulationdirectory, j, i);
     filenames[k] = strcopy(filenames[k], filename);
     models[k].states = i;
     models[k].mixture = j;
     models[k].index = k;     
     k++;
    }
 strcpy(tmpst, "-");
 model = multiple_comparison(filenames, modelcount, get_parameter_l("testtype"), tmpst, &dummy, &dummy2, get_parameter_l("correctiontype"));
 free_2d((void**)filenames, modelcount);
 for (i = 0; i < MAX_MLP_SIMULATION_ALGORITHMS; i++)
  {
   switch (i)
    {
     case MULTIMOST    :bestmodel = hmm_model_simulation_multitest(simulationdirectory, maxstates, maxmixture, &(tries[i]));
                        break;
     case ONE_FORWARD  :bestmodel = hmm_model_simulation_one_forward(simulationdirectory, maxstates, &(tries[i]));
                        break;
     case ONE_BACKWARD :bestmodel = hmm_model_simulation_one_backward(simulationdirectory, maxstates, &(tries[i]));
                        break;
     case FORWARD_MOST :bestmodel = hmm_model_simulation_most(simulationdirectory, maxstates, maxmixture, &(tries[i]), FORWARD);
                        break;
     case BACKWARD_MOST:bestmodel = hmm_model_simulation_most(simulationdirectory, maxstates, maxmixture, &(tries[i]), BACKWARD);
                        break;
    }
   selectedmodels[i] = search_list(model, modelcount, bestmodel.index + 1);
   sprintf(varname, "aout2[%d]", i + 1);
   sprintf(varvalue, "%d", selectedmodels[i]);
   set_variable(varname, varvalue);
   indexes[i] = i;
  }
 for (i = 0; i < MAX_MLP_SIMULATION_ALGORITHMS; i++)
   for (j = i + 1; j < MAX_MLP_SIMULATION_ALGORITHMS; j++)
     if (selectedmodels[i] > selectedmodels[j])
      {
       swap_int(&(selectedmodels[i]), &(selectedmodels[j]));
       swap_int(&(indexes[i]), &(indexes[j]));
       swap_int(&(tries[i]), &(tries[j]));
      }
     else
       if (selectedmodels[i] == selectedmodels[j] && tries[i] > tries[j])
        {
         swap_int(&(selectedmodels[i]), &(selectedmodels[j]));
         swap_int(&(indexes[i]), &(indexes[j]));
         swap_int(&(tries[i]), &(tries[j]));
        }
 i = 0;
 while (i < MAX_MLP_SIMULATION_ALGORITHMS)
  {
   j = i;
   sum = j + 1;
   while (j < MAX_MLP_SIMULATION_ALGORITHMS - 1 && selectedmodels[j] == selectedmodels[j + 1] && tries[j] == tries[j + 1])
    {
     j++;
     sum = sum + j + 1;
    }
   for (k = i; k <= j; k++)
    {
     sprintf(varname, "aout1[%d]", indexes[k] + 1);
     sprintf(varvalue, "%.1f", sum / (j - i + 1.0));
     set_variable(varname, varvalue);
    }
   i = j + 1;
  }
 bestmodel = models[model[0] - 1];
 safe_free(models);
 return bestmodel;
}
