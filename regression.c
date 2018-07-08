#include <stdlib.h>
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "mlp.h"
#include "model.h"
#include "parameter.h"
#include "rbf.h"
#include "regression.h"
#include "regressionrule.h"
#include "regressiontree.h"
#include "softregtree.h"
#include "svm.h"
#include "svmkernel.h"
#include "svmprepare.h"
#include "svmregtree.h"
#include "svmsimplex.h"

extern Datasetptr current_dataset;
extern FILE* output;

/**
 * Prints mean square error results, runtime for a single run to the output file
 * @param[in] testdata Link list containing the test data
 * @param[in] result Structure containing the results
 * @param[in] runtime Runtime of the algorithm in seconds
 * @param[in] printbinary If printbinary is YES, the output for each test instance will be printed, otherwise only the error or accuracy will be printed
 * @param[in] displayruntime If displayruntime is YES, runtime will be prined, otherwise not printed
 */
void print_regression_results(Instanceptr testdata, Regressionresultptr result, double runtime, int printbinary, int displayruntime)
{
 /*!Last Changed 25.05.2006 added regression value*/
	/*!Last Changed 13.12.2004 added printbinary*/
 /*!Last Changed 10.03.2004*/
	char pst[READLINELENGTH], pst2[READLINELENGTH];
	int i;
 double regvalue;
 Instanceptr tmp;
	if (printbinary)
	 {
   set_precision(pst, NULL, "\n");
   set_precision(pst2, NULL, " ");
   tmp = testdata;
		 for (i = 0; i < result->datasize; i++)
    {
     regvalue = give_regressionvalue(tmp);
     fprintf(output, "%d ", tmp->index);
     fprintf(output, pst2, regvalue);
     fprintf(output, pst, result->absoluteerrors[i]);
     tmp = tmp->next;
    }
	 }
	else
  	if (displayruntime)
			 {
				 set_precision(pst, NULL, " %.3lf\n");
	  		fprintf(output, pst, result->regressionperformance / result->datasize, runtime); 		 
			 }
	  else
			 {
				 set_precision(pst, NULL, "\n");
		  	fprintf(output, pst, result->regressionperformance / result->datasize); 		 
			 }
}

/**
 * Constructor of regression result structure. Allocates memory for absolute errors.
 * @param[in] datasize Number of test instances in the dataset
 * @return Empty regression result structure
 */
Regressionresultptr performance_initialize_regression(int datasize)
{
	/*!Last Changed 13.12.2004*/
 Regressionresultptr result;
 result = safemalloc(sizeof(Regressionresult), "performance_initialize_regression", 2);
	result->regressionperformance = 0;
 result->absoluteerrors = safemalloc(datasize * sizeof(double), "performance_initialize_regression", 4);
 result->datasize = datasize;
 result->epsilonloss = 0.0;
 return result;
}

/**
 * Destructor of regression result structure. Frees memory allocated for absolute errors.
 * @param[in] result Regression result structure
 */
void free_performance_regression(Regressionresultptr result){
 /*!Last Changed 01.08.2007*/
 safe_free(result->absoluteerrors);
 safe_free(result);
}

/**
 * Training algorithm for ONEFEATURE. ONEFEATURE algorithm is a regression tree algorithm with one node regressiontree.
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the ONEFEATURE algorithm
 * @return Model of the ONEFEATURE algorithm.
 */
void* train_onefeature(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 11.08.2005 added model for onefeature algorithm*/
	/*!Last Changed 10.12.2004*/
	char dummy;
 Model_onefeatureptr m;
	Instanceptr leftgroup = NULL, rightgroup = NULL;
	m = safemalloc(sizeof(Model_onefeature), "train_onefeature", 4);
	m->bestfeature = best_regression_feature(*trdata, &(m->bestsplit), &dummy);
	divide_instancelist_according_to_split(trdata, &leftgroup, &rightgroup, m->bestfeature, m->bestsplit);
	m->leftmean = find_mean_of_regression_value(leftgroup);
	m->rightmean = find_mean_of_regression_value(rightgroup);
	deallocate(leftgroup);
	deallocate(rightgroup);
	return m;
}

/**
 * Testing algorithm for ONEFEATURE.
 * @param[in] data Test instance.
 * @param[in] model Model of the ONEFEATURE algorithm.
 * @param[in] parameters Parameters of the ONEFEATURE algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_onefeature(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 11.08.2005 added model for onefeature algorithm*/
	/*!Last Changed 10.12.2004*/
	Model_onefeatureptr m;
 Prediction predicted;
	m = (Model_onefeatureptr) model;
	if (data->values[m->bestfeature].floatvalue <= m->bestsplit)
   predicted.regvalue = m->leftmean;
 else
   predicted.regvalue = m->rightmean;
 return predicted;
}

/**
 * Training algorithm for KNN regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the KNN regression algorithm
 * @return Model of the KNN regression algorithm.
 */
void* train_knnreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 19.04.2004*/
	/*!Last Changed 10.03.2004*/
	return *trdata;
}

/**
 * Testing algorithm for KNN regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the KNN regression algorithm.
 * @param[in] parameters Parameters of the KNN regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_knnreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 19.04.2004*/
	/*!Last Changed 10.03.2004*/
	int i;
	Prediction predicted;
	Instanceptr *nearestneighbors, trdata;
	Knn_parameterptr p;
 p = (Knn_parameterptr) parameters;
	trdata = (Instanceptr) model;
 if (current_dataset->storetype == STORE_KERNEL)
   nearestneighbors = find_all_neighbors(trdata, data, &(p->nearcount));
 else
	  nearestneighbors = find_nearest_neighbors(trdata, data, p->nearcount);
	predicted.regvalue = 0;
 for (i = 0; i < p->nearcount; i++)
			predicted.regvalue += give_regressionvalue(nearestneighbors[i]);
	predicted.regvalue /= p->nearcount;
	safe_free(nearestneighbors);
 return predicted;
}

void* train_regstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	Regressionnodeptr regtreenode;
	Regtree_parameterptr p;
	p = (Regtree_parameterptr) parameters;
 regtreenode = createnewregnode(NULL, 1, p->leaftype);
 regtreenode->cvinstances = *trdata;
 regtreenode->instances = *trdata;
	regtreenode->featureno = best_hybrid_regression_tree_split(regtreenode, p);
 make_regression_children(regtreenode, p->leaftype);
 setup_regressionnode_properties(regtreenode->left, p->leaftype);
 setup_regressionnode_properties(regtreenode->right, p->leaftype);
 make_leaf_regression_node(regtreenode->left, p->leaftype);
 make_leaf_regression_node(regtreenode->right, p->leaftype);
	*trdata = NULL;
	return regtreenode;
}

Prediction test_regstump(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_regressiontree(data, model, parameters, posterior);
}

void* train_softregstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	Regressionnodeptr regtreenode;
	Softregtree_parameterptr p;
	p = (Softregtree_parameterptr) parameters;
 regtreenode = createnewregnode(NULL, 1, p->leaftype);
 regtreenode->weight = 1.0;
 regtreenode->instances = *trdata;
 if (p->leaftype == CONSTANTLEAF)
   regtreenode->regressionvalue = produce_random_value(-0.01, +0.01);
 else
   regtreenode->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
 regtreenode->cvinstances = NULL;
 find_best_soft_regtree_split(regtreenode, regtreenode, *trdata, p);
 make_leaf_regression_node(regtreenode->left, p->leaftype);
 make_leaf_regression_node(regtreenode->right, p->leaftype);
 if (regtreenode->featureno != LEAF_NODE)
   deallocate(regtreenode->instances);
	*trdata = NULL;
	return regtreenode;
}

Prediction test_softregstump(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_softregressiontree(data, model, parameters, posterior);
}

/**
 * Training algorithm for REGRESSIONTREE. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the REGRESSIONTREE algorithm
 * @return Model of the REGRESSIONTREE algorithm.
 */
void* train_regressiontree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 25.05.2006 added in_list ...*/
	/*!Last Changed 06.03.2005*/
	/*!Last Changed 09.12.2004*/
	Regressionnodeptr regtreenode;
	Regtree_parameterptr p;
	p = (Regtree_parameterptr) parameters;
 regtreenode = createnewregnode(NULL, 1, p->leaftype);
 regtreenode->cvinstances = *cvdata;
 regtreenode->instances = *trdata;
	create_regtree(regtreenode, p);
 if (in_list(p->modelselectionmethod, 3, MODEL_SELECTION_CV, MODEL_SELECTION_AIC, MODEL_SELECTION_BIC) || p->prunetype == VALIDATIONPRUNING)
  	prune_regression_tree(regtreenode, p);
	*trdata = NULL;
	return regtreenode;
}

/**
 * Testing algorithm for REGRESSIONTREE.
 * @param[in] data Test instance.
 * @param[in] model Model of the REGRESSIONTREE algorithm.
 * @param[in] parameters Parameters of the REGRESSIONTREE algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_regressiontree(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 06.03.2005*/
	/*!Last Changed 09.12.2004*/
	Regressionnodeptr regtreenode;
 Prediction predicted;
	regtreenode = (Regressionnodeptr) model;
 while (regtreenode->featureno != -1)
   regtreenode = next_regression_node(regtreenode, data); 
 if (regtreenode->leaftype == CONSTANTLEAF)
   predicted.regvalue = regtreenode->regressionvalue;
 else
   predicted.regvalue = linear_fit_value(regtreenode->w, data);
 return predicted;
}

/**
 * Training algorithm for SOFT REGRESSIONTREE. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the SOFT REGRESSIONTREE algorithm
 * @return Model of the SOFT REGRESSIONTREE algorithm.
 */
void* train_softregressiontree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	Regressionnodeptr regtreenode;
	Softregtree_parameterptr p;
	p = (Softregtree_parameterptr) parameters;
 regtreenode = createnewregnode(NULL, 1, p->leaftype);
 regtreenode->weight = 1.0;
 regtreenode->instances = *trdata;
 if (p->leaftype == CONSTANTLEAF)
   regtreenode->regressionvalue = produce_random_value(-0.01, +0.01);
 else
   regtreenode->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
 create_softregtree(regtreenode, regtreenode, *cvdata, p);
 if (regtreenode->featureno != LEAF_NODE)
   deallocate(regtreenode->instances);
 deallocate(*cvdata);
	*trdata = NULL;
	return regtreenode;
}

/**
 * Testing algorithm for SOFT REGRESSIONTREE.
 * @param[in] data Test instance.
 * @param[in] model Model of the SOFT REGRESSIONTREE algorithm.
 * @param[in] parameters Parameters of the SOFT REGRESSIONTREE algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_softregressiontree(Instanceptr data, void* model, void* parameters, double* posterior)
{
	Regressionnodeptr regtreenode;
 Prediction predicted;
	regtreenode = (Regressionnodeptr) model;
 predicted.regvalue = soft_regtree_output(regtreenode, data);
 return predicted;
}

/**
 * Training algorithm for SVMREGRESSIONTREE. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the SVMREGRESSIONTREE algorithm
 * @return Model of the SVMREGRESSIONTREE algorithm.
 */
void* train_svmregtree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 01.11.2009*/
 Regressionnodeptr regtreenode;
 Regtree_parameterptr p;
 p = (Regtree_parameterptr) parameters;
 regtreenode = createnewregnode(NULL, 1, p->leaftype);
 regtreenode->cvinstances = *cvdata;
 regtreenode->instances = *trdata;
 create_svmregtree(regtreenode, p);
 if (in_list(p->modelselectionmethod, 3, MODEL_SELECTION_CV, MODEL_SELECTION_AIC, MODEL_SELECTION_BIC) || p->prunetype == VALIDATIONPRUNING)
   prune_regression_tree(regtreenode, p);
 *trdata = NULL;
 return regtreenode;
}

/**
 * Testing algorithm for SVMREGRESSIONTREE.
 * @param[in] data Test instance.
 * @param[in] model Model of the SVMREGRESSIONTREE algorithm.
 * @param[in] parameters Parameters of the SVMREGRESSIONTREE algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_svmregtree(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_regressiontree(data, model, parameters, posterior);
}

/**
 * Training algorithm for RBF regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the RBF regression algorithmining algorithm for orithm
 * @return Model of the RBF regression algorithm.
 */
void* train_rbfreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	Rbf_parameterptr params;
	params = (Rbf_parameterptr) parameters;
	params->input = current_dataset->multifeaturecount - 1;
	return train_rbfnetwork_reg(*trdata, *cvdata, params);
}

/**
 * Testing algorithm for RBF regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the RBF regression algorithm.
 * @param[in] parameters Parameters of the RBF regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_rbfreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
	Rbfnetworkptr network;
	double *p;
 Prediction predicted;
	network = (Rbfnetworkptr) model;
	p = safemalloc((network->hidden + 1) * sizeof(double), "test_rbfreg", 4);
	rbf_forward_propagation(network, data, p, &(predicted.regvalue));
	safe_free(p);
	return predicted;
}

/**
 * Training algorithm for MLP or LP regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the MLP or LP regression algorithm
 * @return Model of the MLP or LP regression algorithm.
 */
void* train_mlpgenericreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 08.12.2004 added matrix_free(cv)*/
	Mlpparameters mlpparams;
 Mlpnetworkptr neuralnetwork;
	matrix train, cv;
	neuralnetwork = safemalloc(sizeof(Mlpnetwork), "train_mlpgenericreg", 4);
 mlpparams = *((Mlpparametersptr) parameters);
	mlpparams.inputnum = data_size(*trdata);
	mlpparams.trnum = mlpparams.inputnum;
	mlpparams.cvnum = data_size(*cvdata);
	train = instancelist_to_matrix(*trdata, REGRESSION, -1, MULTIVARIATE_LINEAR);
	cv = instancelist_to_matrix(*cvdata, REGRESSION, -1, MULTIVARIATE_LINEAR);
	*neuralnetwork = mlpn_general(train, cv, &mlpparams, REGRESSION);
	matrix_free(train);
	matrix_free(cv);
	return neuralnetwork;
}

/**
 * Testing algorithm for MLP or LP regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the MLP or LP regression algorithm.
 * @param[in] parameters Parameters of the MLP or LP regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_mlpgenericreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 08.12.2004 added matrix_free(cv)*/
 Mlpnetwork neuralnetwork;
	matrix test, testY, output_error;
 Prediction predicted;
 neuralnetwork = *((Mlpnetworkptr) model);
 output_error = matrix_alloc(1, 1);
	test = instancelist_to_matrix(data, REGRESSION, -1, MULTIVARIATE_LINEAR);
 testMlpnInput_reg(test.values[0], &testY, neuralnetwork, &output_error);
 matrix_free(output_error);
 matrix_free(testY);
	matrix_free(test);
 return predicted;
}

/**
 * Training algorithm for DNC (Dynamic Node Creation) regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the DNC regression algorithm
 * @return Model of the DNC regression algorithm.
 */
void* train_dncreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 16.04.2007*/
 Mlpparameters mlpparams;
 Mlpnetworkptr neuralnetwork;
 matrix train;
 neuralnetwork = safemalloc(sizeof(Mlpnetwork), "train_dncreg", 4);
 mlpparams = *((Mlpparametersptr) parameters);
 mlpparams.inputnum = data_size(*trdata);
 mlpparams.trnum = mlpparams.inputnum;
 train = instancelist_to_matrix(*trdata, REGRESSION, -1, MULTIVARIATE_LINEAR);
 *neuralnetwork = dnc_reg(train, &mlpparams);
 matrix_free(train);
 return neuralnetwork;
}

/**
 * Testing algorithm for DNC (Dynamic Node Creation) regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the DNC regression algorithm.
 * @param[in] parameters Parameters of the DNC regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_dncreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 16.04.2007*/
 return test_mlpgenericreg(data, model, parameters, posterior);
}

/**
 * Training algorithm for SELECTAVERAGE. This algorithm is a correspondant algorithm to SELECTMAX in classification. It assigns the average of the training data to all test data.
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the SELECTAVERAGE algorithm
 * @return Model of the SELECTAVERAGE algorithm.
 */
void* train_selectaverage(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 08.12.2004*/
	double* trainmean;
	trainmean = safemalloc(sizeof(double), "train_selectaverage", 2);
	*trainmean = find_mean_of_regression_value(*trdata);
 return trainmean;
}

/**
 * Testing algorithm for SELECTAVERAGE.
 * @param[in] data Test instance.
 * @param[in] model Model of the SELECTAVERAGE algorithm.
 * @param[in] parameters Parameters of the SELECTAVERAGE algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_selectaverage(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 08.12.2004*/
	Prediction predicted;
	predicted.regvalue = *((double*) model);
 return predicted;
}

/**
 * Training algorithm for Gaussian process regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Gaussian process regression
 * @return Model of the Gaussian process regression.
 */
void* train_gprocessreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 15.01.2009*/
 int i, j;
 matrix kernel_matrix, t;
 Svm_nodeptr* nodelist;
 Svm_parameter param;
 Gprocess_parameterptr gp = (Gprocess_parameterptr) parameters;
 Model_gprocessregressionptr m = safemalloc(sizeof(Model_gprocessregression), "train_gprocessreg", 3);
 Instanceptr tmp = *trdata;
 m->size = data_size(*trdata);
 m->data = *trdata;
 kernel_matrix = matrix_alloc(m->size, m->size);
 t = matrix_alloc(m->size, 1);
 nodelist = safemalloc(m->size * sizeof(Svm_nodeptr), "train_gprocessreg", 10);
 i = 0;
 while (tmp)
  {
   nodelist[i] = instance_to_svmnode(tmp);
   t.values[i][0] = give_regressionvalue(tmp);
   tmp = tmp->next;
   i++;
  }
 param.kernel_type = gp->kernel_type;
 param.coef0 = gp->coef0;
 param.gamma = gp->gamma;
 param.degree = gp->degree;
 for (i = 0; i < m->size; i++)
   for (j = 0; j < m->size; j++)
     kernel_matrix.values[i][j] = k_function(nodelist[i], nodelist[j], param);
 for (i = 0; i < m->size; i++)
   kernel_matrix.values[i][i] += gp->sigma;
 matrix_inverse(&kernel_matrix);
 matrix_product(&kernel_matrix, t);
 m->weights = safemalloc(m->size * sizeof(double), "train_gprocessreg", 25);
 for (i = 0; i < m->size; i++)
   m->weights[i] = kernel_matrix.values[i][0];
 for (i = 0; i < m->size; i++)
   safe_free(nodelist[i]);
 safe_free(nodelist);
 matrix_free(kernel_matrix);
 matrix_free(t);
 return m;
}

/**
 * Testing algorithm for Gaussian process regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the Gaussian process regression.
 * @param[in] parameters Parameters of the Gaussian process regression.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_gprocessreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 15.01.2009*/
 int i;
 Prediction predicted;
 Gprocess_parameterptr gp;
 Model_gprocessregressionptr m;
 Instanceptr tmp;
 Svm_parameter param;
 Svm_nodeptr x, y;
 predicted.regvalue = 0.0;
 gp = (Gprocess_parameterptr) parameters;
 param.kernel_type = gp->kernel_type;
 param.coef0 = gp->coef0;
 param.gamma = gp->gamma;
 param.degree = gp->degree;
 m = (Model_gprocessregressionptr) model;
 y = instance_to_svmnode(data);
 tmp = m->data;
 for (i = 0; i < m->size; i++)
  {
   x = instance_to_svmnode(tmp);
   predicted.regvalue += k_function(x, y, param) * m->weights[i];
   safe_free(x);
   tmp = tmp->next;
  }
 safe_free(y);
 return predicted;
}

/**
 * Training algorithm for LINEARREGRESSION. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the LINEARREGRESSION algorithm
 * @return Model of the LINEARREGRESSION algorithm.
 */
void* train_linearregression(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 10.12.2004*/
	Model_linearregressionptr m;
	int i, datacount = 0;
	matrix rxy, rxx, mx, mxprime, bmatrix;
	Instanceptr tmp;
	double regmean, y;
 Instance mean;
	m = safemalloc(sizeof(Model_linearregression), "train_linearregression", 6);
	rxy = matrix_alloc(1, current_dataset->multifeaturecount - 1);
 mean = find_mean(*trdata);
	mxprime = instance_to_matrix(&mean);
	mx = matrix_transpose(mxprime);
	matrix_free(mxprime);
	regmean = find_mean_of_regression_value(*trdata);
	rxx = covariance(*trdata, mean);
	matrix_inverse(&rxx);
	tmp = *trdata;
	while (tmp)
	 {
		 datacount++;
		 y = give_regressionvalue(tmp);
		 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
				 rxy.values[0][i] += (real_feature(tmp, i) - real_feature(&mean, i)) * (y - regmean);
		 tmp = tmp->next;
	 }
	for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
		 rxy.values[0][i] /= (datacount - 1);
	m->w = matrix_multiply(rxy, rxx);
	matrix_free(rxy);
 bmatrix = matrix_multiply(m->w, mx);
 m->b = regmean - bmatrix.values[0][0];
	safe_free(mean.values);
	matrix_free(bmatrix);
	matrix_free(mx);
	matrix_free(rxx);
	return m;
}

/**
 * Testing algorithm for LINEARREGRESSION.
 * @param[in] data Test instance.
 * @param[in] model Model of the LINEARREGRESSION algorithm.
 * @param[in] parameters Parameters of the LINEARREGRESSION algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_linearregression(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 10.12.2004*/
	Model_linearregressionptr m;
	int i;
 Prediction predicted;
 predicted.regvalue = 0.0;
	m = (Model_linearregressionptr) model;
	for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
		 predicted.regvalue += real_feature(data, i) * m->w.values[0][i];
	predicted.regvalue += m->b;
 return predicted;
}

/**
 * Training algorithm for QUANTIZED REGRESSION. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the QUANTIZED REGRESSION algorithm
 * @return Model of the QUANTIZED REGRESSION algorithm.
 */
void* train_quantizereg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 21.12.2004*/
	Model_quantizeregptr model;
	Quantizereg_parameterptr param;
	int i, j, k, m, x, y, **countstwo, *countsone, found, totalcount;
	double maxx, maxy, xvalue, yvalue, total, *eigenvalues;
	Instanceptr tmp;
	matrix cov;
	Instance mean;
	model = safemalloc(sizeof(Model_quantizereg), "train_quantizereg", 6);
	param = (Quantizereg_parameterptr) parameters;
	model->partitioncount = param->partitioncount;
	if (current_dataset->featurecount > 2)
	 {
   mean = find_mean(*trdata);
  	cov = covariance(*trdata, mean);
	  safe_free(mean.values);
   eigenvalues = find_eigenvalues_eigenvectors_symmetric(&(model->eigenvectors), cov);
  	matrix_free(cov);
   safe_free(eigenvalues);
   dimension_reduce_with_pca(*trdata, 2, model->eigenvectors);
	  find_bounds_of_feature(current_dataset, *trdata, 0, &(model->minx), &maxx);
			model->binx = (maxx - model->minx) / param->partitioncount;
	  find_bounds_of_feature(current_dataset, *trdata, 1, &(model->miny), &maxy);
			model->biny = (maxy - model->miny) / param->partitioncount;
			countstwo = (int**) safecalloc_2d(sizeof(int *), sizeof(int), param->partitioncount, param->partitioncount, "train_quantizereg", 19);
			model->meanstwo = (double**) safecalloc_2d(sizeof(double *), sizeof(double), param->partitioncount, param->partitioncount, "train_quantizereg", 20);
			tmp = *trdata;
			while (tmp)
			 {
				 xvalue = real_feature(tmp, 0);
				 yvalue = real_feature(tmp, 1);
					x = (int) ((xvalue - model->minx) / model->binx);
					if (x == param->partitioncount)
						 x--;
					y = (int) ((yvalue - model->miny) / model->biny);
					if (y == param->partitioncount)
						 y--;
					model->meanstwo[x][y] += give_regressionvalue(tmp);
					(countstwo[x][y])++;
				 tmp = tmp->next;
			 }
			for (i = 0; i < param->partitioncount; i++)
				 for (j = 0; j < param->partitioncount; j++)
						 if (countstwo[i][j] != 0)
								 model->meanstwo[i][j] /= countstwo[i][j];
			for (i = 0; i < param->partitioncount; i++)
				 for (j = 0; j < param->partitioncount; j++)
						 if (countstwo[i][j] == 0)
							 {
								 found = 0;
								 k = 1;
									totalcount = 0;
									total = 0;
         while (!found)
									 {
										 for (m = j - k; m <= j + k; m++)
												 if (between(i - k, 0, param->partitioncount - 1) && between(m, 0, param->partitioncount - 1) && countstwo[i - k][m] != 0)
													 {
														 total += countstwo[i - k][m] * model->meanstwo[i - k][m];
															totalcount += countstwo[i - k][m];
														 found = 1;
													 }
										 for (m = j - k; m <= j + k; m++)
												 if (between(i + k, 0, param->partitioncount - 1) && between(m, 0, param->partitioncount - 1) && countstwo[i + k][m] != 0)
													 {
														 total += countstwo[i + k][m] * model->meanstwo[i + k][m];
															totalcount += countstwo[i + k][m];
														 found = 1;
													 }
											for (m = i - k; m <= i + k; m++)
												 if (between(j - k, 0, param->partitioncount - 1) && between(m, 0, param->partitioncount - 1) && countstwo[m][j - k] != 0)
													 {
														 total += countstwo[m][j - k] * model->meanstwo[m][j - k];
															totalcount += countstwo[m][j - k];
															found = 1;
													 }
											for (m = i - k; m <= i + k; m++)
												 if (between(j + k, 0, param->partitioncount - 1) && between(m, 0, param->partitioncount - 1) && countstwo[m][j + k] != 0)
													 {
														 total += countstwo[m][j + k] * model->meanstwo[m][j + k];
															totalcount += countstwo[m][j + k];
															found = 1;
													 }
											k++;
									 }
									model->meanstwo[i][j] = total / totalcount; 
							 } 
	 }
	else
	 {
	  find_bounds_of_feature(current_dataset, *trdata, 0, &(model->minx), &maxx);
			model->binx = (maxx - model->minx) / (param->partitioncount);
			model->meansone = safecalloc(param->partitioncount, sizeof(double), "train_quantizereg", 86);
			countsone = safecalloc(param->partitioncount, sizeof(int), "quantizereg", 87);
			tmp = *trdata;
			while (tmp)
			 {
				 xvalue = real_feature(tmp, 0);
					x = (int) ((xvalue - model->minx) / model->binx);
					if (x == param->partitioncount)
						 x--;
					model->meansone[x] += give_regressionvalue(tmp);
					(countsone[x])++;
				 tmp = tmp->next;
			 }
			for (i = 0; i < param->partitioncount; i++)
					if (countsone[i] != 0)
							model->meansone[i] /= countsone[i];
   for (i = 0; i < param->partitioncount; i++)
				 if (countsone[i] == 0)
					 {
						 found = 0;
							totalcount = 0;
							total = 0;
							k = 1;
							while (!found)
							 {
								 if (between(i - k, 0, param->partitioncount - 1) && countsone[i - k] != 0)
									 {
										 total += countsone[i - k] * model->meansone[i - k];
											totalcount += countsone[i - k];
											found = 1;
									 }
									if (between(i + k, 0, param->partitioncount - 1) && countsone[i + k] != 0)
									 {
										 total += countsone[i + k] * model->meansone[i + k];
											totalcount += countsone[i + k];
											found = 1;
									 }
								 k++;
							 }
							model->meansone[i] = total / totalcount;
					 }
	 }
	if (current_dataset->featurecount > 2)
			free_2d((void**)countstwo, param->partitioncount);
	else
			safe_free(countsone);
	return model;
}

/**
 * Testing algorithm for QUANTIZED regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the QUANTIZED regression algorithm.
 * @param[in] parameters Parameters of the QUANTIZED regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_quantizereg(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 21.12.2004*/
	Model_quantizeregptr m;
	int x, y;
	double xvalue, yvalue;
 Prediction predicted;
	m = (Model_quantizeregptr) model;
	if (current_dataset->featurecount > 2)
		{
   dimension_reduce_with_pca(data, 2, m->eigenvectors);
			xvalue = real_feature(data, 0);
			yvalue = real_feature(data, 1);
			x = (int) ((xvalue - m->minx) / m->binx);
   check_and_put_in_between(&x, 0, m->partitioncount - 1);
			y = (int) ((yvalue - m->miny) / m->biny);
   check_and_put_in_between(&y, 0, m->partitioncount - 1);
   predicted.regvalue = m->meanstwo[x][y];
		}
	else
		{
			xvalue = real_feature(data, 0);
   x = (int) ((xvalue - m->minx) / m->binx);
   check_and_put_in_between(&x, 0, m->partitioncount - 1);
   predicted.regvalue = m->meansone[x];
  }
 return predicted;
}

/**
 * Training algorithm for REGRESSION RULE LEARNER. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the REGRESSION RULE LEARNER algorithm
 * @return Model of the REGRESSION RULE LEARNER algorithm.
 */
void* train_regrule(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 06.03.2005*/
	Regressionruleset* r;
	Regrule_parameterptr param;
	param = (Regrule_parameterptr) parameters;
	r = safemalloc(sizeof(Regressionruleset), "train_regrule", 4);
	*r = regression_rule_learner(trdata, cvdata, param);
	return r;
}

/**
 * Testing algorithm for REGRESSION RULE LEARNER.
 * @param[in] data Test instance.
 * @param[in] model Model of the REGRESSION RULE LEARNER algorithm.
 * @param[in] parameters Parameters of the REGRESSION RULE LEARNER algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_regrule(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 06.03.2005*/
	Regressionruleset r;
 Regressionruleptr rule;
 Prediction predicted;
	r = *((Regressionruleset*) model);
 rule = r.start;
 while (rule)
  {
   if (regression_rule_cover_instance(data, rule))
    {
     if (rule->leaftype == CONSTANTLEAF)
      {
       predicted.regvalue = rule->regressionvalue; 
       return predicted;
      }
     else
      {
       predicted.regvalue = linear_fit_value(rule->w, data); 
       return predicted;
      }
    }
   rule = rule->next;
  }
 if (r.leaftype == CONSTANTLEAF)
   predicted.regvalue = r.regressionvalue;
 else
   predicted.regvalue = linear_fit_value(r.w, data);
 return predicted;
}

/**
 * Training algorithm for C-SVM regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the C-SVM regression algorithm
 * @return Model of the C-SVM regression algorithm.
 */
void* train_svmreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 03.05.2005*/
 Partition dummyp;
	Svm_modelptr model;
	Svm_problem prob;
	Svm_parameterptr param;
	dummyp = get_two_class_partition();
	prepare_svm_problem(&prob, EPSILON_SVR, *trdata, dummyp);
	free_partition(dummyp);
	param = (Svm_parameter*) parameters;
 model = svm_train(prob, *param, 0);
	return model;
}

/**
 * Testing algorithm for C-SVM regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the C-SVM regression algorithm.
 * @param[in] parameters Parameters of the C-SVM regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_svmreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 03.05.2005*/
	Svm_nodeptr node;
 Prediction predicted;
	Svm_modelptr m;
 double* dec_values;
 double y = give_regressionvalue(data);
	m = (Svm_modelptr) model;
	node = instance_to_svmnode(data);
 dec_values = safemalloc(sizeof(double), "test_svm", 7);
 svm_predict_values(m, node, dec_values);
 predicted.epsilonloss = epsilon_sensitive_loss(y, dec_values[0], get_parameter_f("svmepsilon"));
 safe_free(dec_values); 
	predicted.regvalue = svm_predict(m, node);
	safe_free(node);
 return predicted;
}

/**
 * Training algorithm for simplex-SVM regression. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the simplex-SVM regression algorithm
 * @return Model of the simplex-SVM regression algorithm.
 */
void* train_simplexsvmreg(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	/*!Last Changed 03.05.2005*/
	Svm_simplex_parameterptr param;
 param = (Svm_simplex_parameterptr) parameters;
	return solve_svm_regression(*trdata, param);
}

/**
 * Testing algorithm for simplex-SVM regression.
 * @param[in] data Test instance.
 * @param[in] model Model of the simplex-SVM regression algorithm.
 * @param[in] parameters Parameters of the simplex-SVM regression algorithm.
 * @param[out] posterior Not used in regression.
 * @return Calculated output for the test instance.
 */
Prediction test_simplexsvmreg(Instanceptr data, void* model, void* parameters, double* posterior)
{
	/*!Last Changed 03.05.2005*/
 Prediction predicted;
 predicted.regvalue = test_svm_regression((Svm_regression_modelptr) model, (Svm_simplex_parameterptr) parameters, data);
 return predicted;
}
