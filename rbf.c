#include "clustering.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "perceptron.h"
#include "rbf.h"

/**
 * Frees memory allocated to the RBF network weights
 * @param[in] weights RBF network weights
 */
void free_rbfweights(Rbfweights weights)
{
 /*!Last Changed 21.08.2007*/
 matrix_free(weights.m);
 matrix_free(weights.s);
 matrix_free(weights.w);
}

/**
 * RBF Network destructor. Frees memory allocated to the RBF network
 * @param[in] network RBF network
 */
void free_rbfnetwork(Rbfnetworkptr network)
{
	/*!Last Changed 19.08.2007*/
 free_rbfweights(network->weights);
	safe_free(network);
}

/**
 * Allocates memory for the RBF network weights.
 * @param[out] weights RBF network weights
 * @param[in] params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 */
void allocate_rbfweights(Rbfweights* weights, Rbf_parameterptr params)
{
 /*!Last Changed 21.08.2007*/
 weights->m = matrix_alloc(params->hidden, params->input);
 weights->s = matrix_alloc(1, params->hidden);
 weights->w = matrix_alloc(params->output, params->hidden + 1);
}

/**
 * Constructor of RBF network. Allocates memory for the RBF network.
 * @param[in] params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 * @return Allocated and parameters set RBF network
 */
Rbfnetworkptr allocate_rbfnetwork(Rbf_parameterptr params)
{
	/*!Last Changed 19.08.2007*/
	Rbfnetworkptr network;
	network = safemalloc(sizeof(Rbfnetwork), "allocate_rbfnetwork", 2);
	network->hidden = params->hidden;
	network->output = params->output;
	network->input = params->input;
 allocate_rbfweights(&(network->weights), params);
	return network;
}

/**
 * Initializes the RBF network. This includes setting the mean (means are set using k-means clustering where k is the number of gaussians) and variance (variances are set using cluster_spread function) values of each hidden gaussian and assigning random weights to the RBF network weights (between -0.01 and 0.01).
 * @param[in] network RBF network
 * @param[in] data Training data
 * @param[in] params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 */
void initialize_rbfnetwork(Rbfnetworkptr network, Instanceptr data, Rbf_parameterptr params)
{
	/*!Last Changed 20.08.2007*/
	int i, j;
	double* spreads;
	Instanceptr clustermeans, tmp;
	clustermeans = k_means_clustering(data, params->hidden);
	tmp = clustermeans;
	for (i = 0; i < params->hidden; i++)
	 {
		 for (j = 0; j < network->input; j++)
				 network->weights.m.values[i][j] = real_feature(tmp, j);
		 tmp = tmp->next;
	 }
	spreads = cluster_spreads(data, clustermeans, params->hidden);
 for (i = 0; i < params->hidden; i++)
   network->weights.s.values[0][i] = spreads[i];
	assignTwoDimRandomWeights(&(network->weights.w), params->randrange);
	deallocate(clustermeans);
	safe_free(spreads);
}

/**
 * Copies weights of the source RBF network to the destination RBF network.
 * @param[out] dest Destination RBF network
 * @param[in] source Source RBF network
 */
void copy_rbfweights(Rbfweights* dest, Rbfweights source)
{
 /*!Last Changed 21.08.2007*/
 matrix_identical(&(dest->m), source.m);
 matrix_identical(&(dest->s), source.s);
 matrix_identical(&(dest->w), source.w);
}

/**
 * Creates a copy of the RBF network.
 * @param[in] network RBF network for which a copy will be created.
 * @param[in] params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 * @return A copy of the RBF network
 */
Rbfnetworkptr copy_of_rbfnetwork(Rbfnetworkptr network, Rbf_parameterptr params)
{
	/*!Last Changed 19.08.2007*/
	Rbfnetworkptr copy;
	copy = allocate_rbfnetwork(params);
 copy_rbfweights(&(copy->weights), network->weights);
	return copy;
}

/**
 * Calculates RBF gaussian value for an instance.
 * @param[in] inst Input instance
 * @param[in] inputdim Number of features in the dataset. 
 * @param[in] mh Mean vector of the gaussian.
 * @return RBF gaussian value
 */
double distance_to_rbf_gaussian(Instanceptr inst, int inputdim, double* mh)
{
	/*!Last Changed 19.08.2007*/
	int j;
	double sum;
	sum = 0.0;
	for (j = 0; j < inputdim; j++)
			sum += (real_feature(inst, j) - mh[j]) * (real_feature(inst, j) - mh[j]);
	return sum;
}

/**
 * Forward propagation for the RBF network. First calculates gaussian values (p) using input units and weights between input units and gaussians. Second calculates output values (y) using gaussian values and weights between gaussians and output units. If the problem is classification, it also produces probability estimates using softmax function on the output values (y).
 * @param[in] network RBF network
 * @param[in] inst Input instance
 * @param[out] p Calculated values for the gaussians
 * @param[out] y Calculated values for the output units
 */
void rbf_forward_propagation(Rbfnetworkptr network, Instanceptr inst, double* p, double* y)
{
	/*!Last Changed 19.08.2007*/
 double dist, s_h;
	int i, h;
	p[0] = 1.0;
	for (h = 1; h < network->hidden + 1; h++)
  {
   dist = distance_to_rbf_gaussian(inst, network->input, network->weights.m.values[h - 1]);
   s_h = network->weights.s.values[0][h - 1];
			p[h] = safeexp(-dist / (2 * s_h * s_h));
  }
	for (i = 0; i < network->output; i++)
	 {
		 y[i] = 0.0;
			for (h = 0; h < network->hidden + 1; h++)
				 y[i] += network->weights.w.values[i][h] * p[h];
	 }
	if (network->output > 1) /*For classification*/
  	softmax(y, network->output);
}

/**
 * Calculates the difference between the real output and the actual output.
 * @param[in] Number of output units. 1 for regression problem, 2 or more for a classification problem
 * @param[in] inst Input instance
 * @param[in] classno Real class of the input instance 
 * @param[in] y Output values from the forward propagation.
 * @return r_i - y_i.
 */
double real_actual_difference(int output, Instanceptr inst, int classno, double* y)
{
	/*!Last Changed 19.08.2007*/
	double ri;
 if (output == 1)
			ri = give_regressionvalue(inst);
	else
			if (give_classno(inst) != classno)
				 ri = 0.0;
			else
					ri = 1.0;
	return ri - y[classno];
}

/**
 * Backward propagation for the RBF network. First, it calculates the update values for the weights between the output units and the gaussians. Second, it calculates the update values for the weights between the input units and the gaussians (updates for the mean vectors and the variances).
 * @param[in] network RBF network
 * @param[in] inst Input instance
 * @param[in] p Gaussian values calculated using the forward propagation
 * @param[in] y Output values calculates using the forward propagation
 * @param[in] params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 * @return Update values for the weights of the RBF network
 */
Rbfweights rbf_backward_propagation(Rbfnetworkptr network, Instanceptr inst, double* p, double* y, Rbf_parameterptr params)
{
	/*!Last Changed 19.08.2007*/
	int i, h, j;
	double sum, x_j, m_hj, s_h;
 Rbfweights changeinweights;
 allocate_rbfweights(&changeinweights, params);
	/*Update wih*/
	for (i = 0; i < network->output; i++)
		 for (h = 0; h < network->hidden + 1; h++)
				 changeinweights.w.values[i][h] = params->eta * real_actual_difference(network->output, inst, i, y) * p[h];
	/*Update mhj*/
	for (h = 0; h < network->hidden; h++)
		 for (j = 0; j < network->input; j++)
			 {
				 x_j = real_feature(inst, j);
					m_hj = network->weights.m.values[h][j];
					s_h = network->weights.s.values[0][h];
				 sum = 0.0;
				 for (i = 0; i < network->output; i++)
						 sum += real_actual_difference(network->output, inst, i, y) * network->weights.w.values[i][h];
					changeinweights.m.values[h][j] = (params->eta * sum * p[h] * (x_j - m_hj)) / (s_h * s_h);   
			 }
	/*Update sh*/
	for (h = 0; h < network->hidden; h++)
	 {
		 s_h = network->weights.s.values[0][h];
			sum = 0.0;
			for (i = 0; i < network->output; i++)
					sum += real_actual_difference(network->output, inst, i, y) * network->weights.w.values[i][h];
			changeinweights.s.values[0][h] = (params->eta * sum * p[h] * distance_to_rbf_gaussian(inst, network->input, network->weights.m.values[h])) / (s_h * s_h * s_h);
	 }
 return changeinweights;
}

/**
 * Updates weights of the RBF network for a test instance. For momentum, the update is calculated as a weighted sum between the previous update and the current update.
 * @param[in] network RBF network
 * @param[in] delta Update values for the RBF weights calculated using backward propagation
 * @param[in] prevweights Previous weights of the RBF network
 * @param[in] params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 */
void update_rbfweights(Rbfnetworkptr network, Rbfweights delta, Rbfweights prevweights, Rbf_parameterptr params)
{
 /*!Last Changed 21.08.2007*/
 int i, h, j;
 for (i = 0; i < network->output; i++)
   for (h = 0; h < network->hidden + 1; h++)
     network->weights.w.values[i][h] = network->weights.w.values[i][h] + delta.w.values[i][h] + params->alpha * prevweights.w.values[i][h];
 for (h = 0; h < network->hidden; h++)
   for (j = 0; j < network->input; j++)
     network->weights.m.values[h][j] = network->weights.m.values[h][j] + delta.m.values[h][j] + params->alpha * prevweights.m.values[h][j];
 for (h = 0; h < network->hidden; h++)
   network->weights.s.values[0][h] = network->weights.s.values[0][h] + delta.s.values[0][h] + params->alpha * prevweights.s.values[0][h];
}

/**
 * Tests RBF network for a classification dataset.
 * @param[in] network RBF network
 * @param[in] data Test dataset.
 * @return Number of correctly classified instances.
 */
int test_rbfnetwork_cls(Rbfnetworkptr network, Instanceptr data)
{
	/*!Last Changed 19.08.2007*/
	double *p, *y;
	Instanceptr inst;
	int correct = 0, classno;
	p = safemalloc((network->hidden + 1) * sizeof(double), "test_rbfnetwork_cls", 3);
	y = safemalloc(network->output * sizeof(double), "test_rbfnetwork_cls", 4);
	inst = data;
	while (inst)
	 {
		 rbf_forward_propagation(network, inst, p, y);
			classno = findMaxOutput(y, network->output);
			if (classno == give_classno(inst))
				 correct++;
		 inst = inst->next;
	 }
	safe_free(p);
	safe_free(y);
	return correct;
}

/**
 * Trains RBF network for given classification training data. Uses cross-validation data for determining the best network in the epochs.
 * @param[in] traindata Training data
 * @param[in] cvdata Cross-validation data
 * @param params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 * @return Learned RBF network
 */
Rbfnetworkptr train_rbfnetwork_cls(Instanceptr traindata, Instanceptr cvdata, Rbf_parameterptr params)
{
	/*!Last Changed 19.08.2007*/
	int i, j, *array, performance, bestperformance, datasize = data_size(traindata);
	double *p, *y;
	Instanceptr inst;
	Rbfnetworkptr bestnetwork, network;
 Rbfweights prevweights, delta;
	network = allocate_rbfnetwork(params);
	initialize_rbfnetwork(network, traindata, params);
	p = safemalloc((network->hidden + 1) * sizeof(double), "train_rbfnetwork_cls", 7);
	y = safemalloc(network->output * sizeof(double), "train_rbfnetwork_cls", 8);
	params->eta = params->initialeta;
 allocate_rbfweights(&prevweights, params);
 for (i = 0; i < params->epochs; i++)
	 {
		 array = random_array(datasize);
			for (j = 0; j < datasize; j++)
			 {
				 inst = data_x(traindata, array[j]);
				 rbf_forward_propagation(network, inst, p, y);
					delta = rbf_backward_propagation(network, inst, p, y, params);     
     update_rbfweights(network, delta, prevweights, params);
     copy_rbfweights(&prevweights, delta);
     free_rbfweights(delta);
			 }
			safe_free(array);
			performance = test_rbfnetwork_cls(network, cvdata);
		 if (i > 0)
			 {
			  if (performance > bestperformance)
					 {
				   params->bestepoch = i;
				   bestperformance = performance;
				   free_rbfnetwork(bestnetwork);
							bestnetwork = copy_of_rbfnetwork(network, params);
					 }
			 }
		 else
			 {
			  params->bestepoch = i;
			  bestperformance = performance;
					bestnetwork = copy_of_rbfnetwork(network, params);
			 }
		 params->eta *= params->etadecrease;
	 }
	safe_free(p);
	safe_free(y);
 free_rbfweights(prevweights);
 free_rbfnetwork(network);
	return bestnetwork;
}

/**
 * Tests RBF network for a regression dataset.
 * @param[in] network RBF network
 * @param[in] data Test dataset.
 * @return Total squared error.
 */
double test_rbfnetwork_reg(Rbfnetworkptr network, Instanceptr data)
{
	/*!Last Changed 19.08.2007*/
	double *p, *y, squaredError = 0.0;
	Instanceptr inst;
	p = safemalloc((network->hidden + 1) * sizeof(double), "test_rbfnetwork_reg", 3);
	y = safemalloc(sizeof(double), "test_rbfnetwork_reg", 4);
	inst = data;
	while (inst)
	 {
		 rbf_forward_propagation(network, inst, p, y);
			squaredError += (y[0] - give_regressionvalue(inst)) * (y[0] - give_regressionvalue(inst));
		 inst = inst->next;
	 }
	safe_free(p);
	safe_free(y);
	return squaredError;
}

/**
 * Trains RBF network for given regression training data. Uses cross-validation data for determining the best network in the epochs.
 * @param[in] traindata Training data
 * @param[in] cvdata Cross-validation data
 * @param params Parameter of the RBF learning problem containing network parameters such as number of gaussians, number of input units, etc.
 * @return Learned RBF network
 */
Rbfnetworkptr train_rbfnetwork_reg(Instanceptr traindata, Instanceptr cvdata, Rbf_parameterptr params)
{
	/*!Last Changed 19.08.2007*/
	int i, j, *array, datasize = data_size(traindata);
	double *p, *y, squaredError, minSquaredError;
	Instanceptr inst;
	Rbfnetworkptr bestnetwork, network;
 Rbfweights prevweights, delta;
	network = allocate_rbfnetwork(params);
	initialize_rbfnetwork(network, traindata, params);
	p = safemalloc((network->hidden + 1) * sizeof(double), "train_rbfnetwork_reg", 7);
	y = safemalloc(sizeof(double), "train_rbfnetwork_reg", 8);
	params->eta = params->initialeta;
 allocate_rbfweights(&prevweights, params);
 for (i = 0; i < params->epochs; i++)
	 {
		 array = random_array(datasize);
			for (j = 0; j < datasize; j++)
			 {
				 inst = data_x(traindata, array[j]);
				 rbf_forward_propagation(network, inst, p, y);
					delta = rbf_backward_propagation(network, inst, p, y, params);
     update_rbfweights(network, delta, prevweights, params);
     copy_rbfweights(&prevweights, delta);
     free_rbfweights(delta);
			 }
			safe_free(array);
			squaredError = test_rbfnetwork_reg(network, cvdata);
		 if (i > 0)
			 {
			  if (squaredError < minSquaredError)
					 {
				   params->bestepoch = i;
				   minSquaredError = squaredError;
				   free_rbfnetwork(bestnetwork);
							bestnetwork = copy_of_rbfnetwork(network, params);
					 }
			 }
		 else
			 {
			  params->bestepoch = i;
			  minSquaredError = squaredError;
					bestnetwork = copy_of_rbfnetwork(network, params);
			 }
		 params->eta *= params->etadecrease;
	 }
	safe_free(p);
	safe_free(y);
 free_rbfweights(prevweights);
	return bestnetwork;
}
