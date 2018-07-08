#include "dataset.h"
#include "decisiontree.h"
#include "hmm.h"
#include "libmemory.h"
#include "libmisc.h"
#include "loadmodel.h"
#include "matrix.h"
#include "messages.h"
#include "mlp.h"
#include "rbf.h"

/**
 * Creates and reads a matrix from a model file.
 * @param[in] infile File handle
 * @param[in] size1 Number of rows of the matrix
 * @param[in] size2 Number of columns of the matrix
 * @return Matrix allocated and read
 */
matrix load_matrix(FILE* infile, int size1, int size2)
{
 /*!Last Changed 25.12.2005*/
	matrix result;
	int i, j;
	result = matrix_alloc(size1, size2);
 for (i = 0; i < size1; i++)
		 for (j = 0; j < size2; j++)
				 fscanf(infile, "%lf", &(result.values[i][j]));
	return result;
}

/**
 * Creates and reads a double array from a model file
 * @param[in] infile File handle
 * @param[in] size Size of the array
 * @return A double array allocated and read
 */
double* load_array(FILE* infile, int size)
{
 /*!Last Changed 22.12.2005*/
 int i;
 double* result;
 result = safemalloc(size * sizeof(double), "load_array", 3);
 for (i = 0; i < size; i++) 
   fscanf(infile, "%lf", &(result[i]));
 return result;
}

/**
 * Creates and reads an integer array from a model file
 * @param[in] infile File handle
 * @param[in] size Size of the array
 * @return An integer array allocated and read
 */
int* load_array_int(FILE* infile, int size)
{
 /*!Last Changed 22.12.2005*/
 int i, *result;
 result = safemalloc(size * sizeof(int), "load_array_int", 2);
 for (i = 0; i < size; i++) 
   fscanf(infile, "%d", &(result[i]));
 return result;
}

/**
 * Creates and read a two-dimensional double array from a model file
 * @param[in] infile File handle
 * @param[in] size1 Size of the first dimension of the 2d array
 * @param[in] size2 Size of the second dimension of the 2d array
 * @return A 2d double array allocated and read
 */
double** load_array_two(FILE* infile, int size1, int size2)
{
 /*!Last Changed 25.12.2005*/
 int i, j;
 double** result;
 result = (double**) safemalloc_2d(sizeof(double*), sizeof(double), size1, size2, "load_array_two", 3);
 for (i = 0; i < size1; i++) 
		 for (j = 0; j < size2; j++)
     fscanf(infile, "%lf", &(result[i][j]));
 return result;
}

/**
 * Reads and creates a support vector from a model file
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return A support vector allocated and read
 */
Svm_nodeptr load_support_vector(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.12.2005*/
 Svm_nodeptr result;
 int i;
 result = safemalloc(d->multifeaturecount * sizeof(Svm_node), "load_support_vector", 3);
 for (i = 0; i < d->multifeaturecount; i++)
  {
   fscanf(infile, "%d %lf", &(result[i].index), &(result[i].value));
   if (result[i].index == -1)
    {
     result = alloc(result, (i + 1) * sizeof(Svm_node), i + 1);
     return result;
    }
  }
 result[i].index = -1;
 result[i].value = 0.0;
 return result;
}

/**
 * Reads an array of support vectors from a model file. The function uses load_support_vector for reading single support vector
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @param[in] size Number of support vectors to be read (Size of the output array)
 * @return An array of support vectors allocated and read
 */
Svm_nodeptr* load_array_support_vector(FILE* infile, Datasetptr d, int size)
{
 /*!Last Changed 26.12.2005*/
 int i;
 Svm_nodeptr* result;
 result = safemalloc(size * sizeof(Svm_nodeptr), "load_array_support_vector", 3);
 for (i = 0; i < size; i++)
   result[i] = load_support_vector(infile, d);
 return result;
}

/**
 * Reads mean and variance from a model file
 * @param[in] infile File handle
 * @param[out] mean Mean value
 * @param[out] variance Variance value
 */
void load_output_mean_and_variance(FILE* infile, double *mean, double* variance)
{
 /*!Last Changed 27.12.2005*/
	fscanf(infile, "%lf%lf", mean, variance);
}

/**
 * Creates and reads an instance with continuous features
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @param[in] class_loaded If BOOLEAN_TRUE class information (or regression) is also loaded.
 * @warning The instance does not contain the class definition feature
 * @return An instance allocated and read
 */
Instance load_instance(FILE* infile, Datasetptr d, int class_loaded)
{
 /*!Last Changed 15.01.2009 added loading class or regression information*/
 /*!Last Changed 22.12.2005*/
 Instance inst;
 int i;
 inst.values = safemalloc(d->multifeaturecount * sizeof(union value), "load_instance", 3);
 inst.sublist = NULL;
 inst.class_labels = NULL;
 for (i = 0; i < d->multifeaturecount; i++)
   if (i != d->classdefno)
     fscanf(infile, "%lf", &(inst.values[i].floatvalue));
   else
     if (d->classno == 0)
       fscanf(infile, "%lf", &(inst.values[i].floatvalue));
     else
       fscanf(infile, "%d", &(inst.values[i].stringindex));
 return inst; 
}

/**
 * Creates and reads an instance with continuous and/or discrete features. The instance also contains the class definition feature.
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return An instance allocated and read
 */
Instance load_instance_discrete(FILE* infile, Datasetptr d)
{
 /*!Last Changed 22.12.2005*/
 Instance inst;
 int i;
 inst.values = safemalloc(d->featurecount * sizeof(union value), "load_instance_discrete", 3);
 inst.sublist = NULL;
 inst.class_labels = NULL;
 for (i = 0; i < d->featurecount; i++)
   switch (d->features[i].type)
    {
     case INTEGER :
     case REAL    :
					case REGDEF  :fscanf(infile, "%lf", &(inst.values[i].floatvalue));
                   break;
     case STRING  :
     case CLASSDEF:fscanf(infile, "%d", &(inst.values[i].stringindex));
                   break;
     default      :printf(m1238, d->features[i].type);
                   break;
    }
 return inst; 
}

/**
 * Loads the model learned by the SELECTMAX algorithm. The model file contains: \n
 * Number of classes \n
 * Prior probability of class 1 Prior probability of class 2 ... Prior probability of class K
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the SELECTMAX algorithm
 */
void* load_model_selectmax(FILE* infile, Datasetptr d)
{
 /*!Last Changed 19.12.2005*/
 Model_selectmaxptr m;
 m = safemalloc(sizeof(Model_selectmax), "load_model_selectmax", 2);
 fscanf(infile, "%d", &(m->classno));
 m->priors = load_array(infile, d->classno);
 return m;
}

/**
 * Loads the model learned by the GAUSSIAN algorithm. The model file contains: \n
 * Mean vector of class 1 \n
 * Mean vector of class 2 \n
 * ... \n
 * Mean vector of class K \n
 * Variance of class 1 Variance of class 2 ... Variance of class K \n
 * Prior probability of class 1 Prior probability of class 2 ... Prior probability of class K
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the GAUSSIAN algorithm
 */
void* load_model_gaussian(FILE* infile, Datasetptr d)
{
 /*!Last Changed 22.12.2005*/
 Model_gaussianptr m;
 int i;
 Instanceptr tmp, tmpbefore = NULL;
 m = safemalloc(sizeof(Model_gaussian), "load_model_gaussian", 4);
 for (i = 0; i < d->classno; i++)
  {
   tmp = safemalloc(sizeof(Instance), "load_model_gaussian", 7);
   *tmp = load_instance(infile, d, BOOLEAN_FALSE);
   if (tmpbefore)
     tmpbefore->next = tmp;
   else
     m->mean = tmp;
   tmpbefore = tmp;
  }
 tmp->next = NULL;
 m->variance = load_array(infile, d->classno);
 m->priors = load_array(infile, d->classno); 
 return m;
}

/**
 * Loads the model learned by the KNN algorithm. The model file contains: \n
 * Number of training instances \n
 * Instance 1 \n
 * Instance 2 \n
 * ... \n
 * Instance N
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the KNN algorithm
 */
void* load_model_knn(FILE* infile, Datasetptr d)
{
 /*!Last Changed 22.12.2005*/
 int size, i;
 Instanceptr tmp, tmpbefore = NULL, head;
 fscanf(infile, "%d", &size);
 for (i = 0; i < size; i++)
  {
   tmp = safemalloc(sizeof(Instance), "load_model_knn", 6);
   *tmp = load_instance_discrete(infile, d);
   if (tmpbefore)
     tmpbefore->next = tmp;
   else
     head = tmp;
   tmpbefore = tmp;
  }
 tmp->next = NULL;
 return head;
}

void load_model_softtree(FILE* infile, Datasetptr d, Decisionnodeptr rootnode)
{
	int leaftype;
 rootnode->left = NULL;
 rootnode->parent = NULL;
 rootnode->right = NULL;
 rootnode->string = NULL;
	rootnode->instances = NULL;
 rootnode->counts = NULL;
 rootnode->rule = NULL;
 fscanf(infile, "%d", &(rootnode->featureno));
	if (rootnode->featureno == LEAF_NODE)
  {
   fscanf(infile, "%d", &leaftype);
   rootnode->leaftype = leaftype;
   if (rootnode->leaftype == CONSTANTLEAF)
     fscanf(infile, "%lf", &(rootnode->w0));
   else
     rootnode->w = load_matrix(infile, d->multifeaturecount, 1);  
  }
	else
  {
   rootnode->w = load_matrix(infile, d->multifeaturecount, 1);  
   rootnode->left = safemalloc(sizeof(Decisionnode), "load_model_softtree", 20);
   load_model_softtree(infile, d, rootnode->left);
   rootnode->left->parent = rootnode;
   rootnode->right = safemalloc(sizeof(Decisionnode), "load_model_softtree", 23);
   load_model_softtree(infile, d, rootnode->right);
   rootnode->right->parent = rootnode;   
  }
}

/**
 * Loads the model learned by the C45 algorithm. The model file contains at each node: \n
 * Feature No \n
 * If the node is a leaf node Number of classes Number of instances from class 1 Number of instances from class 2 ... Number of instances from class K \n
 * Else \n
 * If the feature is a continuous feature Split Threshold \n
 * Model of the 1st child \n
 * Model of the 2nd child \n
 * ... \n
 * Model of the Lth child
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the C45 algorithm
 */
void load_model_c45(FILE* infile, Datasetptr d, Decisionnodeptr rootnode)
{
 /*!Last Changed 17.07.2006*/
 /*!Last Changed 22.12.2005*/
	int i;
 rootnode->left = NULL;
 rootnode->parent = NULL;
 rootnode->right = NULL;
 rootnode->string = NULL;
	rootnode->rule = NULL;
	rootnode->instances = NULL;
 rootnode->conditiontype = UNIVARIATE_CONDITION; 
 fscanf(infile, "%d", &(rootnode->featureno));
	if (rootnode->featureno >= 0 && in_list(d->features[rootnode->featureno].type, 2, REAL, INTEGER))
		 fscanf(infile, "%lf", &(rootnode->split));
	if (rootnode->featureno == LEAF_NODE)
	 {
		 fscanf(infile, "%d", &(rootnode->classno));
			rootnode->counts = load_array_int(infile, d->classno);
	 }
	else
	 {
		 rootnode->counts = NULL;
   if (in_list(d->features[rootnode->featureno].type, 2, REAL, INTEGER))
    {
				 rootnode->left = safemalloc(sizeof(Decisionnode), "load_model_c45", 20);
     load_model_c45(infile, d, rootnode->left);
     rootnode->left->parent = rootnode;
				 rootnode->right = safemalloc(sizeof(Decisionnode), "load_model_c45", 22);
     load_model_c45(infile, d, rootnode->right);
     rootnode->right->parent = rootnode;
    }
   else
			 {
				 rootnode->string = safemalloc(d->features[rootnode->featureno].valuecount * sizeof(Decisionnode), "load_model_c45", 27);
     for (i = 0; i < d->features[rootnode->featureno].valuecount; i++)
      {
						 load_model_c45(infile, d, &(rootnode->string[i]));
       rootnode->string[i].parent = rootnode;      
      }
			 }
	 }
}

void load_model_svmtree(FILE* infile, Datasetptr d, Decisionnodeptr* forest, int treecount)
{
 int i;
 for (i = 0; i < treecount; i++)
  {
   forest[i] = safemalloc(sizeof(Decisionnode), "load_model_svmtree", 30);
   load_model_c45(infile, d, forest[i]);
  }
}

void load_model_randomforest(FILE* infile, Datasetptr d, Decisionnodeptr* forest, int treecount)
{
 load_model_svmtree(infile, d, forest, treecount);
}

/**
 * Loads the model learned by the LOGISTIC DISCRIMINATION algorithm. The model file contains: \n
 * Weight vector of class 1 \n
 * Weight vector of class 2 \n
 * ... \n
 * Weight vector of class K
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the LOGISTIC DISCRIMINATION algorithm
 */
void* load_model_logistic(FILE* infile, Datasetptr d)
{
 /*!Last Changed 25.12.2005*/
	return load_array_two(infile, d->classno, d->multifeaturecount);
}

/**
 * Loads the model learned by the RBF (Radial Basis Functions) algorithm. The model file contains: \n
 * Number of hidden units (basis) Number of output units (classes) \n
 * Mean vector of basis 1 \n
 * Mean vector of basis 2 \n
 * ... \n
 * Mean vector of basis H \n
 * Variance of basis 1 Variance of basis 2 ... Variance of basis H \n
 * Weight vector from the basis functions to the output unit 1 \n
 * Weight vector from the basis functions to the output unit 2 \n
 * ... \n
 * Weight vector from the basis functions to the output unit K \n
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the RBF algorithm
 */
void* load_model_rbf(FILE* infile, Datasetptr d)
{
	/*!Last Changed 19.08.2007*/
	Rbfnetworkptr m;
	m = safemalloc(sizeof(Rbfnetwork), "load_model_rbf", 2);
	fscanf(infile, "%d %d", &(m->hidden), &(m->output));
	m->weights.m = load_matrix(infile, m->hidden, d->multifeaturecount - 1);
	m->weights.s = load_matrix(infile, 1, m->hidden);
	m->weights.w = load_matrix(infile, m->output, m->hidden + 1);
	return m;
}

/**
 * Loads the model learned by the MLP (Multilayer Perceptron) or LP (Linear Perceptron) algorithm. For MLP, the model file contains: \n
 * Number of inputs (features) Number of output units (classes) Number of layers \n
 * Weight matrix from the input units to the hidden layer 1 \n
 * Weight matrix from hidden layer 1 to the hidden layer 2 \n
 * ... \n
 * Weight matrix from hidden layer D to the output units \n
 * \n
 * For LP, the model file contains: \n
 * Number of inputs (features) Number of output units (classes) 0 \n
 * Weight matrix from the input units to the output units 
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the MLP or LP algorithm
 */
void* load_model_mlpgeneric(FILE* infile, Datasetptr d)
{
 /*!Last Changed 25.12.2005*/
	int i;
 Mlpnetworkptr m;
	m = safemalloc(sizeof(Mlpnetwork), "load_model_mlpgeneric", 3);
	fscanf(infile, "%d %d %d", &(m->P.dimnum), &(m->P.classnum), &(m->P.layernum));
	for (i = 0; i < m->P.layernum; i++)
		 fscanf(infile, "%d", &(m->P.hidden[i]));
	m->weights = safemalloc((m->P.layernum + 1) * sizeof(matrix), "load_model_mlpgeneric", 7);
	for (i = 0; i < m->P.layernum; i++)
			if (i == 0)
	 			m->weights[i] = load_matrix(infile, m->P.hidden[i], m->P.dimnum + 1);
			else
			 	m->weights[i] = load_matrix(infile, m->P.hidden[i], m->P.hidden[i-1] + 1);
	if (m->P.layernum == 0)
			m->weights[m->P.layernum] = load_matrix(infile, m->P.classnum, m->P.dimnum + 1);
	else
			m->weights[m->P.layernum] = load_matrix(infile, m->P.classnum, m->P.hidden[m->P.layernum - 1] + 1);	
	return m;
}

/**
 * Loads the model learned by the MULTILDT algorithm. The model file contains at each node: \n
 * Feature No \n
 * If the node is a leaf node Number of classes Number of instances from class 1 Number of instances from class 2 ... Number of instances from class K \n
 * Else \n
 * Bias value (w_0) \n
 * Weight vector \n
 * Model of the left child \n
 * Model of the right child
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the MULTILDT algorithm
 */
void* load_model_multildt(FILE* infile, Datasetptr d)
{
 /*!Last Changed 25.12.2005*/
 Decisionnodeptr tmp;
 tmp = safemalloc(sizeof(Decisionnode), "load_model_c45", 2);
 tmp->left = NULL;
 tmp->parent = NULL;
 tmp->right = NULL;
 tmp->string = NULL;
	tmp->rule = NULL;
	tmp->instances = NULL;
	tmp->conditiontype = MULTIVARIATE_CONDITION;
 fscanf(infile, "%d", &(tmp->featureno));
	if (tmp->featureno == LEAF_NODE)
	 {
		 fscanf(infile, "%d", &(tmp->classno));
			tmp->counts = load_array_int(infile, d->classno);
	 }
	else
	 {
  	fscanf(infile, "%lf", &(tmp->w0));
			tmp->w = load_matrix(infile, 1, d->multifeaturecount - 1);
		 tmp->counts = NULL;
 		tmp->left = load_model_multildt(infile, d);
   tmp->left->parent = tmp;
			tmp->right = load_model_multildt(infile, d);
   tmp->right->parent = tmp;
	 }
 return tmp;
}

void* load_model_nodeboundedtree(FILE* infile, Datasetptr d)
{
 /*!Last Changed 25.12.2005*/
 Decisionnodeptr tmp;
 tmp = safemalloc(sizeof(Decisionnode), "load_model_nodeboundedtree", 2);
 tmp->left = NULL;
 tmp->parent = NULL;
 tmp->right = NULL;
 tmp->string = NULL;
	tmp->rule = NULL;
	tmp->instances = NULL;
	tmp->conditiontype = HYBRID_CONDITION;
 fscanf(infile, "%d", &(tmp->featureno));
	switch (tmp->featureno)
	 {
	  case LEAF_NODE:fscanf(infile, "%d", &(tmp->classno));
				              tmp->counts = load_array_int(infile, d->classno);
																		break;
			case LINEAR   :fscanf(infile, "%lf", &(tmp->w0));
               			tmp->w = load_matrix(infile, 1, d->multifeaturecount - 1);
		                tmp->counts = NULL;
                  break;
			case QUADRATIC:fscanf(infile, "%lf", &(tmp->w0));
				              tmp->w = load_matrix(infile, 1, multifeaturecount_2d(d));
		                tmp->counts = NULL;
                  break;
			default       :fscanf(infile, "%lf", &(tmp->split));
		                tmp->counts = NULL;
																		break;
	 }
	if (tmp->featureno != LEAF_NODE)
	 {
 		tmp->left = load_model_nodeboundedtree(infile, d);
			tmp->right = load_model_nodeboundedtree(infile, d);
	 }
 return tmp;
}

/**
 * Loads the model learned by the NEAREST MEAN algorithm. The model file contains: \n
 * Mean vector of class 1 \n
 * Mean vector of class 2 \n
 * ... \n
 * Mean vector of class K
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the NEAREST MEAN algorithm
 */
void* load_model_nearestmean(FILE* infile, Datasetptr d)
{
 /*!Last Changed 25.12.2005*/
 Instanceptr m;
 int i;
 Instanceptr tmp, tmpbefore = NULL;
 for (i = 0; i < d->classno; i++)
  {
   tmp = safemalloc(sizeof(Instance), "load_model_gaussian", 7);
   *tmp = load_instance_discrete(infile, d);
   if (tmpbefore)
     tmpbefore->next = tmp;
   else
     m = tmp;
   tmpbefore = tmp;
  }
 tmp->next = NULL;
 return m;
}

/**
 * Loads the model learned by the NAIVE BAYES algorithm. The model file contains: \n
 * Mean vector of class 1 \n
 * Mean vector of class 2 \n
 * ... \n
 * Mean vector of class K \n
 * Prior probability of class 1 Prior probability of class 2 ... Prior probability of class K \n
 * Variance of class 1 Variance of class 2 ... Variance of class K \n
 * For all discrete features: \n
 * Probability of feature value 1 of feature 1 of class 1 Probability of feature value 2 of feature 1 of class 1 ... Probability of feature value L_1 of feature 1 of class 1 \n
 * Probability of feature value 1 of feature 2 of class 1 Probability of feature value 2 of feature 2 of class 1 ... Probability of feature value L_2 of feature 2 of class 1 \n
 * ... \n
 * Probability of feature value 1 of feature d of class 1 Probability of feature value 2 of feature d of class 1 ... Probability of feature value L_d of feature 2 of class 1 \n
 * ... \n
 * ... \n
 * Probability of feature value 1 of feature d of class K Probability of feature value 2 of feature d of class K ... Probability of feature value L_d of feature 2 of class K 
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the NAIVE BAYES algorithm
 */
void* load_model_naivebayes(FILE* infile, Datasetptr d)
{
 /*!Last Changed 11.08.2007*/
	int i, j;
 Model_naivebayesptr m;
 m = safemalloc(sizeof(Model_naivebayes), "load_model_naivebayes", 3);
	m->m = load_array_two(infile, d->classno, d->featurecount);
	m->priors = load_array(infile, d->classno);
	m->s = load_array(infile, d->classno);
	m->p = safemalloc(d->classno * sizeof(double **), "load_model_naivebayes", 7);
 for (i = 0; i < d->classno; i++)
	 {
		 m->p[i] = safemalloc(d->featurecount * sizeof(double *), "load_model_naivebayes", 10);
		 for (j = 0; j < d->featurecount; j++)
				 if (d->features[j].type == STRING)
						 m->p[i][j] = load_array(infile, d->features[j].valuecount);
	 }
 return m;
}

/**
 * Loads the model learned by the QDA (Quadratic Discriminant Analysis) algorithm. The model file contains: \n
 * Bias value of class 1 Bias value of class 2 ... Bias value of class K \n
 * Covariance matrix of class 1 \n
 * Covariance matrix of class 2 \n
 * ... \n
 * Covariance matrix of class K \n
 * Weight vector of class 1 \n
 * Weight vector of class 2 \n
 * ... \n
 * Weight vector of class K
 * @param[in] infile File handle
 * @param[in] d Dataset
 * @return The model learned by the QDA algorithm
 */
void* load_model_qdaclass(FILE* infile, Datasetptr d)
{
 /*!Last Changed 05.08.2007*/
 Model_qdaclassptr m;
 int i;
 m = safemalloc(sizeof(Model_qdaclass), "load_model_qdaclass", 3);
 m->w0 = load_array(infile, d->classno);
 m->priors = load_array(infile, d->classno);
 m->W = safemalloc(d->classno * sizeof(matrix), "load_model_qdaclass", 5);
 for (i = 0; i < d->classno; i++)
   m->W[i] = load_matrix(infile, d->multifeaturecount - 1, d->multifeaturecount - 1);
 m->w = safemalloc(d->classno * sizeof(matrix), "load_model_qdaclass", 8);
 for (i = 0; i < d->classno; i++)
   m->w[i] = load_matrix(infile, 1, d->multifeaturecount - 1);
 return m;
}

void* load_model_ldaclass(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.12.2005*/
 Model_ldaclassptr m;
 int i;
 m = safemalloc(sizeof(Model_ldaclass), "load_model_ldaclass", 2);
 fscanf(infile, "%d %d", &(m->newfeaturecount), &(m->oldfeaturecount));
 m->w0 = load_array(infile, d->classno);
 m->w = safemalloc(d->classno * sizeof(matrix), "load_model_ldaclass", 5);
 for (i = 0; i < d->classno; i++)
   m->w[i] = load_matrix(infile, 1, m->newfeaturecount - 1);
 m->eigenvectors = load_matrix(infile, d->multifeaturecount - 1, d->multifeaturecount - 1);
 m->eigenvalues = NULL;
 return m;
}

void* load_model_svm(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.12.2005*/
 Svm_modelptr m;
 int svmtype, kerneltype;
 m = safemalloc(sizeof(Svm_model), "load_model_svm", 2);
 fscanf(infile, "%d %d", &(m->nr_class), &(m->l));
 fscanf(infile, "%d %d %d", &svmtype, &kerneltype, &(m->param.degree));
 m->param.kernel_type = kerneltype;
 m->param.svm_type = svmtype;
 fscanf(infile, "%lf %lf %lf", &(m->param.gamma), &(m->param.coef0), &(m->param.p));
 if (!in_list(m->param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
   m->nSV = load_array_int(infile, m->nr_class);
	else
		 m->nSV = NULL;
 m->rho = load_array(infile, ((m->nr_class - 1) * m->nr_class) / 2);
 if (!in_list(m->param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
  {
   m->probA = load_array(infile, ((m->nr_class - 1) * m->nr_class) / 2);
   m->probB = load_array(infile, ((m->nr_class - 1) * m->nr_class) / 2);
  }
 else
  {
   m->probA = NULL;
   m->probB = NULL;
  }
 m->sv_coef = load_array_two(infile, m->nr_class - 1, m->l);
 m->SV = load_array_support_vector(infile, d, m->l);
 m->prob.l = 0;
 return m;
}

Decisionconditionptr load_condition(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.12.2005*/
 Decisionconditionptr condition;
 condition = safemalloc(sizeof(Decisioncondition), "load_condition", 2);
 fscanf(infile, "%d %c", &(condition->featureindex), &(condition->comparison));
 if (condition->featureindex >= 0)
  {
   if (condition->comparison != '*')
     fscanf(infile, "%lf", &(condition->value));
   else
    {
     fscanf(infile, "%lf", &(condition->lowerlimit));
     fscanf(infile, "%lf", &(condition->upperlimit));
    }
  }
 else
  {
   fscanf(infile, "%lf", &(condition->w0));
   condition->w = load_matrix(infile, 1, d->multifeaturecount - 1);
  }
 condition->next = NULL;
 return condition;
}

Decisionruleptr load_rule(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.12.2005*/
 int i;
 Decisionruleptr rule;
 Decisionconditionptr tmp, tmpbefore;
 rule = safemalloc(sizeof(Decisionrule), "load_rule", 4);
 fscanf(infile, "%d%d%d%d", &(rule->classno), &(rule->decisioncount), &(rule->falsepositives), &(rule->covered));
 rule->counts = load_array_int(infile, d->classno);
 rule->start = load_condition(infile, d);
 tmpbefore = rule->start;
 tmp = rule->start;
 for (i = 1; i < rule->decisioncount; i++)
  {
   tmp = load_condition(infile, d);
   tmpbefore->next = tmp;
   tmpbefore = tmp;
  }
 rule->end = tmp;
 rule->next = NULL;
 return rule;
}

void* load_model_ripper(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.12.2005*/
 int i;
 Rulesetptr m;
 Decisionruleptr tmp, tmpbefore;
 m = safemalloc(sizeof(Ruleset), "load_model_ripper", 4);
 fscanf(infile, "%d%d", &(m->defaultclass), &(m->rulecount));
 m->counts = load_array_int(infile, d->classno);
 if (m->rulecount > 0)
  {
   m->start = load_rule(infile, d);
   tmpbefore = m->start;
   tmp = m->start;
   for (i = 1; i < m->rulecount; i++)
    {
     tmp = load_rule(infile, d);
     tmpbefore->next = tmp;
     tmpbefore = tmp;
    }
   m->end = tmp;
  }
 else
  {
   m->start = NULL;
   m->end = NULL;
  }
 return m;
}

void* load_model_rise(FILE* infile, Datasetptr d)
{
 int i;
 Model_riseptr m;
 Decisionruleptr tmp, tmpbefore;
 m = safemalloc(sizeof(Model_rise), "load_model_rise", 4);
 fscanf(infile, "%d", &(m->rules.rulecount));
 m->counts = load_array_int(infile, d->classno);
 if (m->rules.rulecount > 0)
  {
   m->rules.start = load_rule(infile, d);
   tmpbefore = m->rules.start;
   tmp = m->rules.start;
   for (i = 1; i < m->rules.rulecount; i++)
    {
     tmp = load_rule(infile, d);
     tmpbefore->next = tmp;
     tmpbefore = tmp;
    }
   m->rules.end = tmp;
  }
 else
  {
   m->rules.start = NULL;
   m->rules.end = NULL;
  }
 m->svdm = safemalloc(d->featurecount * sizeof(matrix), "load_model_rise", 20);
 for (i = 0; i < d->featurecount; i++)
   if (d->features[i].type == STRING)
     m->svdm[i] = load_matrix(infile, d->features[i].valuecount, d->features[i].valuecount);  
 return m;
}

void* load_model_divs(FILE* infile, Datasetptr d)
{
 int i, j, k;
 Model_divsptr m;
 m = safemalloc(sizeof(Model_divs), "load_model_divs", 4);
 fscanf(infile, "%d %d %d", &(m->datacount), &(m->M), &(m->epsilon));
 m->rulecounts = safemalloc(m->datacount * sizeof(int), "load_model_divs", 6);
 m->classno = safemalloc(m->datacount * sizeof(int), "load_model_divs", 7);
 m->hypotheses = safemalloc(m->datacount * sizeof(Condition**), "load_model_divs", 8);
 m->conditioncounts = safemalloc(m->datacount * sizeof(int*), "load_model_divs", 9);
 for (i = 0; i < m->datacount; i++)
  {
   fscanf(infile, "%d %d", &(m->rulecounts[i]), &(m->classno[i]));
   m->conditioncounts[i] = load_array_int(infile, m->rulecounts[i]);
   for (j = 0; j < m->rulecounts[i]; j++)
    {
     for (k = 0; k < m->conditioncounts[i][j]; k++)
       fscanf(infile, "%d%c%lf", &(m->hypotheses[i][j][k].featureindex), &(m->hypotheses[i][j][k].comparison), &(m->hypotheses[i][j][k].value));
    }   
  }
 return m;
}

void* load_model_onefeature(FILE* infile, Datasetptr d)
{
 /*!Last Changed 27.12.2005*/
	Model_onefeatureptr m;
	m = safemalloc(sizeof(Model_onefeature), "load_model_onefeature", 2);
	fscanf(infile, "%d%lf%lf%lf", &(m->bestfeature), &(m->bestsplit), &(m->leftmean), &(m->rightmean));
	return m;
}

void* load_model_selectaverage(FILE* infile, Datasetptr d)
{
 /*!Last Changed 27.12.2005*/
	double* m;
	m = safemalloc(sizeof(double), "load_model_selectaverage", 2);
	fscanf(infile, "%lf", m);
	return m;
}

void* load_model_linearreg(FILE* infile, Datasetptr d)
{
 /*!Last Changed 27.12.2005*/
	Model_linearregressionptr m;
	m = safemalloc(sizeof(Model_linearregression), "load_model_linearreg", 2);
	fscanf(infile, "%lf", &(m->b));
	m->w = load_matrix(infile, 1, d->multifeaturecount - 1);
	return m;
}

void* load_model_quantizereg(FILE* infile, Datasetptr d)
{
 /*!Last Changed 27.12.2005*/
	Model_quantizeregptr m;
	m = safemalloc(sizeof(Model_quantizereg), "load_model_quantizereg", 2);
	fscanf(infile, "%d", &(m->partitioncount));
	if (d->multifeaturecount == 2)
	 {
		 fscanf(infile, "%lf%lf", &(m->binx), &(m->minx));
			m->meansone = load_array(infile, m->partitioncount);
	 }
	else
	 {
		 fscanf(infile, "%lf%lf%lf%lf", &(m->binx), &(m->minx), &(m->biny), &(m->miny));
			m->meanstwo = load_array_two(infile, m->partitioncount, m->partitioncount);
			m->eigenvectors = load_matrix(infile, d->multifeaturecount - 1, d->multifeaturecount - 1);
	 }
	return m;
}

void load_model_regtree(FILE* infile, Datasetptr d, Regressionnodeptr rootnode)
{
	int leaftype;
 rootnode->left = NULL;
 rootnode->parent = NULL;
 rootnode->right = NULL;
 rootnode->string = NULL;
	rootnode->instances = NULL;
 rootnode->cvinstances = NULL;
 fscanf(infile, "%d", &(rootnode->featureno));
	if (rootnode->featureno == LEAF_NODE)
  {
   fscanf(infile, "%d", &leaftype);
   rootnode->leaftype = leaftype;
   if (rootnode->leaftype == CONSTANTLEAF)
     fscanf(infile, "%lf", &(rootnode->regressionvalue));
   else
     rootnode->w = load_matrix(infile, d->multifeaturecount, 1);  
  }
	else
  {
   if (rootnode->featureno >= 0)
     fscanf(infile, "%lf", &(rootnode->split));
   else
     if (rootnode->featureno == LINEAR)
      {
       rootnode->leftcenter = safemalloc(sizeof(Instance), "load_model_regtree", 14);
       *(rootnode->leftcenter) = load_instance(infile, d, BOOLEAN_FALSE);
       rootnode->rightcenter = safemalloc(sizeof(Instance), "load_model_regtree", 16);
       *(rootnode->rightcenter) = load_instance(infile, d, BOOLEAN_FALSE);
      }
     else
       if (rootnode->featureno == SOFTLIN)
         rootnode->w = load_matrix(infile, d->multifeaturecount, 1);
   rootnode->left = safemalloc(sizeof(Regressionnode), "load_model_regtree", 20);
   load_model_regtree(infile, d, rootnode->left);
   rootnode->left->parent = rootnode;
   rootnode->right = safemalloc(sizeof(Regressionnode), "load_model_regtree", 22);
   load_model_regtree(infile, d, rootnode->right);
   rootnode->right->parent = rootnode;
  }
}

void* load_model_hmm(FILE* infile, Datasetptr d)
{
 /*!Last Changed 26.02.2007*/
 int i, j, k, statetype;
 Hmmptr h;
 Hmmptr* hmmarray;
 Multinomialptr m;
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Dirichletptr di;
 Dirichletmixtureptr dm;
 hmmarray = safemalloc(d->classno * sizeof(Hmmptr), "load_model_hmm", 10);
 for (k = 0; k < d->classno; k++)
  {
   h = safemalloc(sizeof(Hmm), "load_model_hmm", 11);
   hmmarray[k] = h;
   fscanf(infile, "%d%d", &(h->statecount), &statetype);
   h->statetype = statetype;
   h->states = safemalloc(h->statecount * sizeof(State), "load_model_hmm", 4);
   for (i = 0; i < h->statecount; i++)
    {
     h->states[i].a_dot_j = load_array(infile, h->statecount);
     h->states[i].a_i_dot = load_array(infile, h->statecount);
     fscanf(infile, "%lf", &(h->states[i].pi));
     switch (h->statetype)
      {
       case          STATE_DISCRETE:m = safemalloc(sizeof(Multinomial), "load_model_hmm", 11);
                                    m->prob = safemalloc((d->featurecount - 1) * sizeof(double*), "load_model_hmm", 12);
                                    for (j = 0; j < d->featurecount; j++)
                                      if (d->features[j].type == STRING)
                                        m->prob[j] = load_array(infile, d->features[j].valuecount);
                                    h->states[i].b = m;
                                    break;
       case          STATE_GAUSSIAN:g = safemalloc(sizeof(Gaussian), "load_model_hmm", 18);
                                    g->mean = load_array(infile, d->multifeaturecount - 1);
                                    g->covariance = load_matrix(infile, d->multifeaturecount - 1, d->multifeaturecount - 1);
                                    h->states[i].b = g;
                                    break;
       case  STATE_GAUSSIAN_MIXTURE:gm = safemalloc(sizeof(Gaussianmixture), "load_model_hmm", 23);
                                    fscanf(infile, "%d", &(gm->componentcount));
                                    gm->weights = load_array(infile, gm->componentcount);
                                    gm->components = safemalloc(gm->componentcount * sizeof(Gaussian), "load_model_hmm", 26);
                                    for (j = 0; j < gm->componentcount; j++)
                                     {
                                      gm->components[j].mean = load_array(infile, d->multifeaturecount - 1);
                                      gm->components[j].covariance = load_matrix(infile, d->multifeaturecount - 1, d->multifeaturecount - 1);
                                     }
                                    h->states[i].b = gm;
                                    break;
       case         STATE_DIRICHLET:di = safemalloc(sizeof(Dirichlet), "load_model_hmm", 34);
                                    di->alpha = load_array(infile, d->multifeaturecount - 1);
                                    h->states[i].b = di;
                                    break;
       case STATE_DIRICHLET_MIXTURE:dm = safemalloc(sizeof(Dirichletmixture), "load_model_hmm", 38);
                                    fscanf(infile, "%d", &(dm->componentcount));
                                    dm->weights = load_array(infile, dm->componentcount);
                                    dm->components = safemalloc(dm->componentcount * sizeof(Dirichlet), "load_model_hmm", 41);
                                    for (j = 0; j < dm->componentcount; j++)
                                      dm->components[j].alpha = load_array(infile, d->multifeaturecount - 1);
                                    h->states[i].b = dm;
                                    break;
      }
    }
  }
 return hmmarray;
}

void* load_model_gprocessreg(FILE* infile, Datasetptr d)
{
 /*!Last Changed 15.01.2009*/
 int i;
 Model_gprocessregressionptr m;
 Instanceptr tmp, tmpbefore = NULL, head;
 m = safemalloc(sizeof(Model_gprocessregression), "load_model_gprocessreg", 2);
 fscanf(infile, "%d", &(m->size));
 for (i = 0; i < m->size; i++)
  {
   tmp = safemalloc(sizeof(Instance), "load_model_gprocessreg", 6);
   *tmp = load_instance(infile, d, BOOLEAN_TRUE);
   if (tmpbefore)
     tmpbefore->next = tmp;
   else
     head = tmp;
   tmpbefore = tmp;
  }
 tmp->next = NULL;
 m->data = head;
 m->weights = load_array(infile, m->size);
 return m;
}

void* load_model_regrule(FILE* infile, Datasetptr d)
{
 /*!Last Changed 27.12.2005*/
	Regressionrulesetptr m;
	m = safemalloc(sizeof(Regressionruleset), "load_model_regrule", 2);
	return m;
}
