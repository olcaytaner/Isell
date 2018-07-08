#include "dataset.h"
#include "decisiontree.h"
#include "hmm.h"
#include "instancelist.h"
#include "matrix.h"
#include "messages.h"
#include "libmisc.h"
#include "regressiontree.h"
#include "savemodel.h"
#include "svmsimplex.h"

/**
 * Saves a matrix to model file
 * @param[in] modelfile File handle for model
 * @param[in] m Matrix to be saved
 */
void save_matrix(FILE* modelfile, matrix m)
{
 /*Last Changed 25.12.2005*/
	int i, j;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
	for (i = 0; i < m.row; i++)
	 {
		 for (j = 0; j < m.col; j++)
				 fprintf(modelfile, pst, m.values[i][j]);
			fprintf(modelfile, "\n");
	 }
}

/**
 * Saves one-dimensional double array to model file
 * @param[in] modelfile File handle for model
 * @param[in] array Array to be saved
 * @param[in] size Size of the array
 */
void save_array(FILE* modelfile, double* array, int size)
{
 /*Last Changed 22.12.2005*/
 int i;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " "); 
 for (i = 0; i < size; i++)
   fprintf(modelfile, pst, array[i]);
 fprintf(modelfile, "\n");
}

/**
 * Saves one-dimensional integer array to model file
 * @param[in] modelfile File handle for model
 * @param[in] array Array to be saved
 * @param[in] size Size of the array
 */
void save_array_int(FILE* modelfile, int* array, int size)
{
 /*Last Changed 26.12.2005*/
 int i;
 for (i = 0; i < size; i++)
   fprintf(modelfile, "%d ", array[i]);
 fprintf(modelfile, "\n");
}

/**
 * Saves two-dimensional double array to model file
 * @param[in] modelfile File handle for model
 * @param[in] array Array to be saved
 * @param[in] size1 Size of the first dimension of the array
 * @param[in] size1 Size of the second dimension of the array
 */
void save_array_two(FILE* modelfile, double** array, int size1, int size2)
{
 /*Last Changed 25.12.2005*/
 int i, j;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " "); 
 for (i = 0; i < size1; i++)
	 {
		 for (j = 0; j < size2; j++)
     fprintf(modelfile, pst, array[i][j]);
   fprintf(modelfile, "\n");
	 }
}

/**
 * Saves a single support vector to model file 
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] sv The support vector to be saved
 */
void save_support_vector(FILE* modelfile, Datasetptr d, Svm_nodeptr sv)
{
 /*Last Changed 26.12.2005*/
 int i = 0;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
 while (sv[i].index != -1)
  {
   fprintf(modelfile, "%d ", sv[i].index);
   fprintf(modelfile, pst, sv[i].value);
   i++;
  }
 fprintf(modelfile, "%d ", sv[i].index);
 fprintf(modelfile, pst, sv[i].value);
 fprintf(modelfile, "\n");
}

/**
 * Saves an array of support vectors to model file
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] array Array of support vectors to be saved
 * @param[in] size Size of the array
 */
void save_array_support_vector(FILE* modelfile, Datasetptr d, Svm_nodeptr* array, int size)
{
 /*Last Changed 26.12.2005*/
 int i;
 for (i = 0; i < size; i++)
   save_support_vector(modelfile, d, array[i]);
}

/**
 * Saves a realized instance to model file
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] inst Instance to be saved
 * @param[in] class_saved If BOOLEAN_TRUE class information (or regression) is also saved
 */
void save_instance(FILE* modelfile, Datasetptr d, Instanceptr inst, int class_saved)
{
 /*!Last Changed 15.01.2009 added also class or regression information*/
 /*!Last Changed 22.12.2005*/
 int i;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " "); 
 for (i = 0; i < d->multifeaturecount; i++)
   if (i != d->classdefno)
     fprintf(modelfile, pst, inst->values[i].floatvalue);
   else
     if (d->classno == 0)
       fprintf(modelfile, pst, inst->values[i].floatvalue);
     else
       fprintf(modelfile, "%d ", inst->values[i].stringindex);
 fprintf(modelfile, "\n");
}

/**
 * Saves a 2-d instance to model file
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] inst 2-d instance to be saved
 * @param[in] class_saved If BOOLEAN_TRUE class information (or regression) is also saved
 */
void save_instance_2d(FILE* modelfile, Datasetptr d, Instanceptr inst, int class_saved)
{
 /*!Last Changed 15.01.2009 added also class or regression information*/
 /*!Last Changed 22.12.2005*/
 int i, dim = multifeaturecount_2d(d) + 1;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " "); 
 for (i = 0; i < dim; i++)
   if (i != d->classdefno)
     fprintf(modelfile, pst, inst->values[i].floatvalue);
   else
     if (d->classno == 0)
       fprintf(modelfile, pst, inst->values[i].floatvalue);
     else
       fprintf(modelfile, "%d ", inst->values[i].stringindex);
 fprintf(modelfile, "\n");
}

/**
 * Saves a regular instance (may contain discrete features) to model file
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] inst Instance to be saved
 */
void save_instance_discrete(FILE* modelfile, Datasetptr d, Instanceptr inst)
{
 /*Last Changed 22.12.2005*/
 int i;
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " "); 
 for (i = 0; i < d->featurecount; i++)
   switch (d->features[i].type)
    {
     case REAL    :
     case INTEGER :
					case REGDEF  :fprintf(modelfile, pst, inst->values[i].floatvalue);
                   break;
     case STRING  :
     case CLASSDEF:fprintf(modelfile, "%d ", inst->values[i].stringindex);
                   break;
     default      :printf(m1238, d->features[i].type);
                   break;
    }
 fprintf(modelfile, "\n");
}

/**
 * Saves the mean and the variance of the regression values of the training instances
 * @param[in] modelfile File handle for model
 * @param[in] mean Mean of the regression values of the training instances
 * @param[in] variance Variance of the regression values of the training instances
 */
void save_output_mean_and_variance(FILE* modelfile, double mean, double variance)
{
 /*Last Changed 27.12.2005*/
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
	fprintf(modelfile, pst, mean);
 set_precision(pst, NULL, "\n");
	fprintf(modelfile, pst, variance);
}

/**
 * Saves the mean and variance of the training instances
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] mean Mean vector (stored as an instance) of the training instances
 * @param[in] variance Variance vector (stored as an instance) of the training instances
 * @param[in] mustrealize If mustrealize is YES, the algorithm needs converted dataset (discrete features converted to continuous features), so the mean and variance vectors are converted instances. If mustrealize is NO, the algorithm does not need conversion, so the mean and variance vectors are regular instances.
 */
void save_mean_and_variance(FILE* modelfile, Datasetptr d, Instance mean, Instance variance, int mustrealize)
{
 /*Last Changed 22.12.2005*/
	if (mustrealize)
	 {
   save_instance(modelfile, d, &mean, BOOLEAN_FALSE);
   save_instance(modelfile, d, &variance, BOOLEAN_FALSE);
	 }
	else
	 {
   save_instance_discrete(modelfile, d, &mean);
   save_instance_discrete(modelfile, d, &variance);
	 }
}

/**
 * Loads the model learned by the SELECTMAX algorithm. The model file contains: \n
 * Number of classes \n
 * Prior probability of class 1 Prior probability of class 2 ... Prior probability of class K
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the SELECTMAX algorithm
 */
void save_model_selectmax(FILE* modelfile, Datasetptr d, Model_selectmaxptr m)
{
 /*Last Changed 19.12.2005*/
 fprintf(modelfile, "%d\n", m->classno);
 save_array(modelfile, m->priors, d->classno);
}

/**
 * Loads the model learned by the GAUSSIAN algorithm. The model file contains: \n
 * Mean vector of class 1 \n
 * Mean vector of class 2 \n
 * ... \n
 * Mean vector of class K \n
 * Variance of class 1 Variance of class 2 ... Variance of class K \n
 * Prior probability of class 1 Prior probability of class 2 ... Prior probability of class K
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the GAUSSIAN algorithm
 */
void save_model_gaussian(FILE* modelfile, Datasetptr d, Model_gaussianptr m)
{
 /*Last Changed 22.12.2005*/
 int i;
 Instanceptr m_i;
 m_i = m->mean;
 for (i = 0; i < d->classno; i++)
  {
   save_instance(modelfile, d, m_i, BOOLEAN_FALSE);
   m_i = m_i->next;
  }
 save_array(modelfile, m->variance, d->classno);
 save_array(modelfile, m->priors, d->classno);
}

/**
 * Loads the model learned by the KNN algorithm. The model file contains: \n
 * Number of training instances \n
 * Instance 1 \n
 * Instance 2 \n
 * ... \n
 * Instance N
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the KNN algorithm
 */
void save_model_knn(FILE* modelfile, Datasetptr d, Instanceptr m)
{
 /*Last Changed 22.12.2005*/
 Instanceptr tmp;
 fprintf(modelfile, "%d\n", data_size(m));
 tmp = m;
 while (tmp)
  {
   save_instance_discrete(modelfile, d, tmp);
   tmp = tmp->next;
  }
}

void save_model_softtree(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level)
{
 /*Last Changed 28.09.2011*/
 char pst[READLINELENGTH];
 int i, size;
 for (i = 0; i < level; i++)
   fprintf(modelfile, "  ");
 set_precision(pst, NULL, "\n"); 
 fprintf(modelfile, "%d ", m->featureno);
 if (m->featureno == LEAF_NODE)
  {
   size = data_size(m->instances);
   fprintf(modelfile, "%d ", m->leaftype);
   if (m->leaftype == CONSTANTLEAF)
     fprintf(modelfile, pst, m->w0);   
   else
     save_matrix(modelfile, m->w);   
  }
 else
  {
   save_matrix(modelfile, m->w);
   save_model_softtree(modelfile, d, m->left, level + 1);
   save_model_softtree(modelfile, d, m->right, level + 1);
  } 
}

/**
 * Loads the model learned by the C4.5 algorithm. The model file contains at each node: \n
 * Feature No \n
 * If the node is a leaf node Number of classes Number of instances from class 1 Number of instances from class 2 ... Number of instances from class K \n
 * Else \n
 * If the feature is a continuous feature Split Threshold \n
 * Model of the 1st child \n
 * Model of the 2nd child \n
 * ... \n
 * Model of the Lth child
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the C4.5 algorithm
 */
void save_model_c45(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level)
{
 /*Last Changed 22.12.2005*/
 char pst[READLINELENGTH];
 int i;
	for (i = 0; i < level; i++)
		 fprintf(modelfile, "  ");
 set_precision(pst, NULL, " "); 
 fprintf(modelfile, "%d ", m->featureno);
 if (m->featureno >= 0 && in_list(d->features[m->featureno].type, 2, REAL, INTEGER))
   fprintf(modelfile, pst, m->split);
 if (m->featureno == LEAF_NODE)
  {
   fprintf(modelfile, "%d ", m->classno);
   save_array_int(modelfile, m->counts, d->classno);
   return;
  }
 else
	 {
		 fprintf(modelfile, "\n");
   if (in_list(d->features[m->featureno].type, 2, REAL, INTEGER))
    {
     save_model_c45(modelfile, d, m->left, level + 1);
     save_model_c45(modelfile, d, m->right, level + 1);
    }
   else
     for (i = 0; i < d->features[m->featureno].valuecount; i++)
       save_model_c45(modelfile, d, &(m->string[i]), level + 1);
	 }
}

void save_model_svmtree(FILE* modelfile, Datasetptr d, Decisionnodeptr* m, int treecount)
{
 int i;
 for (i = 0; i < treecount; i++)
   save_model_c45(modelfile, d, m[i], 0);
}

void save_model_randomforest(FILE* modelfile, Datasetptr d, Decisionnodeptr* m, int treecount)
{
 save_model_svmtree(modelfile, d, m, treecount);
}

/**
 * Loads the model learned by the LOGISTIC DISCRIMINATION algorithm. The model file contains: \n
 * Weight vector of class 1 \n
 * Weight vector of class 2 \n
 * ... \n
 * Weight vector of class K
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the LOGISTIC DISCRIMINATION algorithm
 */
void save_model_logistic(FILE* modelfile, Datasetptr d, double** m)
{
 /*Last Changed 25.12.2005*/
	save_array_two(modelfile, m, d->classno, d->multifeaturecount);
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
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the RBF algorithm
 */
void save_model_rbf(FILE* modelfile, Datasetptr d, Rbfnetworkptr m)
{
	/*Last Changed 19.08.2007*/
	fprintf(modelfile, "%d %d\n", m->hidden, m->output);
	save_matrix(modelfile, m->weights.m);
	save_matrix(modelfile, m->weights.s);
	save_matrix(modelfile, m->weights.w);
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
 * Weight matrix from the input units to the output units  * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the MLP or LP algorithm
 */
void save_model_mlpgeneric(FILE* modelfile, Datasetptr d, Mlpnetworkptr m)
{
 /*Last Changed 25.12.2005*/
	int i;
	fprintf(modelfile, "%d %d %d\n", m->P.dimnum, m->P.classnum, m->P.layernum);
 save_array_int(modelfile, m->P.hidden, m->P.layernum);
	for (i = 0; i <= m->P.layernum; i++)
			save_matrix(modelfile, m->weights[i]);
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
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the MULTILDT algorithm
 */
void save_model_multildt(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level)
{
 /*Last Changed 25.12.2005*/
 char pst[READLINELENGTH];
 int i;
	for (i = 0; i < level; i++)
		 fprintf(modelfile, "  ");
 set_precision(pst, NULL, " "); 
 fprintf(modelfile, "%d ", m->featureno);
 if (m->featureno == LEAF_NODE)
  {
   fprintf(modelfile, "%d ", m->classno);
   save_array_int(modelfile, m->counts, d->classno);
  }
 else
	 {
  	fprintf(modelfile, pst, m->w0);
   save_matrix(modelfile, m->w);
   save_model_multildt(modelfile, d, m->left, level + 1);
   save_model_multildt(modelfile, d, m->right, level + 1);
	 }
}

void save_model_nodeboundedtree(FILE* modelfile, Datasetptr d, Decisionnodeptr m, int level)
{
 /*Last Changed 19.03.2006*/
 char pst[READLINELENGTH];
 int i;
	for (i = 0; i < level; i++)
		 fprintf(modelfile, "  ");
 set_precision(pst, NULL, " "); 
 fprintf(modelfile, "%d ", m->featureno);
	switch (m->featureno)
	 {
	  case LEAF_NODE:fprintf(modelfile, "%d ", m->classno);
                  save_array_int(modelfile, m->counts, d->classno);
                  break;
			case LINEAR   :
			case QUADRATIC:fprintf(modelfile, pst, m->w0);
                  save_matrix(modelfile, m->w);
                  break;
			default       :fprintf(modelfile, pst, m->split);
				              break;
	 }
	if (m->featureno != LEAF_NODE)
	 {
   save_model_nodeboundedtree(modelfile, d, m->left, level + 1);
   save_model_nodeboundedtree(modelfile, d, m->right, level + 1);
	 }
}

/**
 * Loads the model learned by the NEAREST MEAN algorithm. The model file contains: \n
 * Mean vector of class 1 \n
 * Mean vector of class 2 \n
 * ... \n
 * Mean vector of class K
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the NEAREST MEAN algorithm
 */
void save_model_nearestmean(FILE* modelfile, Datasetptr d, Instanceptr m)
{
 /*Last Changed 25.12.2005*/
 int i;
 Instanceptr m_i;
 m_i = m;
 for (i = 0; i < d->classno; i++)
  {
   save_instance_discrete(modelfile, d, m_i);
   m_i = m_i->next;
  }
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
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the NAIVE BAYES algorithm
 */
void save_model_naivebayes(FILE* modelfile, Datasetptr d, Model_naivebayesptr m)
{
 /*Last Changed 11.08.2007*/
 int i, j;
 save_array_two(modelfile, m->m, d->classno, d->featurecount);
	save_array(modelfile, m->priors, d->classno);
	save_array(modelfile, m->s, d->featurecount);
 for (i = 0; i < d->classno; i++)
		 for (j = 0; j < d->featurecount; j++)
				 if (d->features[j].type == STRING)
						 save_array(modelfile, m->p[i][j], d->features[j].valuecount);
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
 * @param[in] modelfile File handle for model
 * @param[in] d The dataset
 * @param[in] m The model learned by the QDA algorithm
 */
void save_model_qdaclass(FILE* modelfile, Datasetptr d, Model_qdaclassptr m)
{
 /*Last Changed 05.08.2007*/
 int i;
 save_array(modelfile, m->w0, d->classno);
 save_array(modelfile, m->priors, d->classno);
 for (i = 0; i < d->classno; i++)
   save_matrix(modelfile, m->W[i]);
 for (i = 0; i < d->classno; i++)
   save_matrix(modelfile, m->w[i]);
}

void save_model_ldaclass(FILE* modelfile, Datasetptr d, Model_ldaclassptr m)
{
 /*Last Changed 26.12.2005*/
 int i;
 fprintf(modelfile, "%d %d\n", m->newfeaturecount, m->oldfeaturecount);
 save_array(modelfile, m->w0, d->classno);
 for (i = 0; i < d->classno; i++)
   save_matrix(modelfile, m->w[i]);
 save_matrix(modelfile, m->eigenvectors);
}

void save_model_svm(FILE* modelfile, Datasetptr d, Svm_modelptr m)
{
 /*Last Changed 26.12.2005*/
 char pst[READLINELENGTH];
 set_precision(pst, NULL, "\n");
 fprintf(modelfile, "%d %d\n", m->nr_class, m->l);
 fprintf(modelfile, "%d %d %d\n", m->param.svm_type, m->param.kernel_type, m->param.degree);
 fprintf(modelfile, pst, m->param.gamma);
 fprintf(modelfile, pst, m->param.coef0);
 fprintf(modelfile, pst, m->param.p);
 if (!in_list(m->param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
   save_array_int(modelfile, m->nSV, m->nr_class);
 save_array(modelfile, m->rho, ((m->nr_class - 1) * m->nr_class) / 2);
 if (!in_list(m->param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
  {
   save_array(modelfile, m->probA, ((m->nr_class - 1) * m->nr_class) / 2);
   save_array(modelfile, m->probB, ((m->nr_class - 1) * m->nr_class) / 2);
  }
 save_array_two(modelfile, m->sv_coef, m->nr_class - 1, m->l);
 save_array_support_vector(modelfile, d, m->SV, m->l);
}

void save_condition(FILE* modelfile, Decisionconditionptr condition)
{
 /*Last Changed 26.12.2005*/
 char pst[READLINELENGTH];
 fprintf(modelfile, "%d %c ", condition->featureindex, condition->comparison);
 if (condition->featureindex >= 0)
  {
   if (condition->comparison != '*')
    {
     set_precision(pst, NULL, " ");
     fprintf(modelfile, pst, condition->value);
    }
   else 
    {    
     set_precision(pst, NULL, " ");
     fprintf(modelfile, pst, condition->lowerlimit);
     set_precision(pst, NULL, " ");
     fprintf(modelfile, pst, condition->upperlimit);    
    }
  }
 else
  {
   set_precision(pst, NULL, "\n");
   fprintf(modelfile, pst, condition->w0);
   save_matrix(modelfile, condition->w);
  }
}

void save_rule(FILE* modelfile, Decisionruleptr rule, Datasetptr d)
{
 /*Last Changed 26.12.2005*/
 Decisionconditionptr tmp;
 fprintf(modelfile, "%d %d %d %d\n", rule->classno, rule->decisioncount, rule->falsepositives, rule->covered);
 save_array_int(modelfile, rule->counts, d->classno);
 tmp = rule->start;
 while (tmp)
  {
   save_condition(modelfile, tmp);
   tmp = tmp->next;
  }
 fprintf(modelfile, "\n");
}

void save_model_ripper(FILE* modelfile, Datasetptr d, Rulesetptr m)
{
 /*Last Changed 26.12.2005*/
 Decisionruleptr tmp;
 fprintf(modelfile, "%d %d\n", m->defaultclass, m->rulecount);
 save_array_int(modelfile, m->counts, d->classno);
 tmp = m->start;
 while (tmp)
  {
   save_rule(modelfile, tmp, d);
   tmp = tmp->next;
  }
}

void save_model_rise(FILE* modelfile, Datasetptr d, Model_riseptr m)
{
 int i;
 Decisionruleptr tmp;
 fprintf(modelfile, "%d\n", m->rules.rulecount);
 save_array_int(modelfile, m->counts, d->classno);
 tmp = m->rules.start;
 while (tmp)
  {
   save_rule(modelfile, tmp, d);
   tmp = tmp->next;
  }
 for (i = 0; i < d->featurecount; i++)
   if (d->features[i].type == STRING)
     save_matrix(modelfile, m->svdm[i]);
}

void save_model_divs(FILE* modelfile, Datasetptr d, Model_divsptr m)
{
 int i, j, k;
 fprintf(modelfile, "%d %d %d\n", m->datacount, m->M, m->epsilon);
 for (i = 0; i < m->datacount; i++)
  {
   fprintf(modelfile, "%d %d\n", m->rulecounts[i], m->classno[i]);
   save_array_int(modelfile, m->conditioncounts[i], m->rulecounts[i]);
   for (j = 0; j < m->rulecounts[i]; j++)
    {
     for (k = 0; k < m->conditioncounts[i][j]; k++)
       fprintf(modelfile, "%d %c %.6f\n", m->hypotheses[i][j][k].featureindex, m->hypotheses[i][j][k].comparison, m->hypotheses[i][j][k].value);
    }
  }
}

void save_model_onefeature(FILE* modelfile, Datasetptr d, Model_onefeatureptr m)
{
 /*Last Changed 27.12.2005*/
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
	fprintf(modelfile, "%d ", m->bestfeature);
	fprintf(modelfile, pst, m->bestsplit);
	fprintf(modelfile, pst, m->leftmean);
	fprintf(modelfile, pst, m->rightmean);
}

void save_model_selectaverage(FILE* modelfile, Datasetptr d, double* m)
{
 /*Last Changed 27.12.2005*/
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
	fprintf(modelfile, pst, *m);
}

void save_model_linearreg(FILE* modelfile, Datasetptr d, Model_linearregressionptr m)
{
 /*Last Changed 27.12.2005*/
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
	fprintf(modelfile, pst, m->b);
	save_matrix(modelfile, m->w);
}

void save_model_quantizereg(FILE* modelfile, Datasetptr d, Model_quantizeregptr m)
{
 /*Last Changed 27.12.2005*/
 char pst[READLINELENGTH];
 set_precision(pst, NULL, " ");
	fprintf(modelfile, "%d ", m->partitioncount);
	if (d->multifeaturecount == 2)
	 {
		 fprintf(modelfile, pst, m->binx);
		 fprintf(modelfile, pst, m->minx);
			save_array(modelfile, m->meansone, m->partitioncount);
	 }
	else
	 {
		 fprintf(modelfile, pst, m->binx);
		 fprintf(modelfile, pst, m->minx);
		 fprintf(modelfile, pst, m->biny);
		 fprintf(modelfile, pst, m->miny);
			save_array_two(modelfile, m->meanstwo, m->partitioncount, m->partitioncount);
			save_matrix(modelfile, m->eigenvectors);
	 }
}

void save_model_regtree(FILE* modelfile, Datasetptr d, Regressionnodeptr m, int level)
{
 /*Last Changed 29.05.2006*/
 char pst[READLINELENGTH];
 int i, size;
 for (i = 0; i < level; i++)
   fprintf(modelfile, "  ");
 set_precision(pst, NULL, "\n"); 
 fprintf(modelfile, "%d ", m->featureno);
 if (m->featureno == LEAF_NODE)
  {
   size = data_size(m->instances);
   fprintf(modelfile, "%d ", m->leaftype);
   if (m->leaftype == CONSTANTLEAF)
     fprintf(modelfile, pst, m->regressionvalue);   
   else
     save_matrix(modelfile, m->w);   
  }
 else
  {
   if (m->featureno >= 0)
     fprintf(modelfile, pst, m->split);   
   else
     if (m->featureno == LINEAR)
      {
       save_instance(modelfile, d, m->leftcenter, BOOLEAN_FALSE);
       save_instance(modelfile, d, m->rightcenter, BOOLEAN_FALSE);
      }
     else
       if (m->featureno == QUADRATIC)
        {
         save_instance_2d(modelfile, d, m->leftcenter, BOOLEAN_FALSE);
         save_instance_2d(modelfile, d, m->rightcenter, BOOLEAN_FALSE);
        }
       else
         if (m->featureno == SOFTLIN)
           save_matrix(modelfile, m->w);
   save_model_regtree(modelfile, d, m->left, level + 1);
   save_model_regtree(modelfile, d, m->right, level + 1);
  } 
}

void save_model_hmm(FILE* modelfile, Datasetptr d, Hmmptr* hmmarray)
{
 /*Last Changed 02.04.2007*/
 /*Last Changed 26.02.2007*/
 int i, j, k;
 char pst[READLINELENGTH];
 Multinomialptr m;
 Hmmptr h;
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Dirichletptr di;
 Dirichletmixtureptr dm;
 set_precision(pst, NULL, "\n");
 for (k = 0; k < d->classno; k++)
  {
   h = hmmarray[k];
   fprintf(modelfile, "%d %d\n", h->statecount, h->statetype);
   for (i = 0; i < h->statecount; i++)
    {
     save_array(modelfile, h->states[i].a_dot_j, h->statecount);
     save_array(modelfile, h->states[i].a_i_dot, h->statecount);
     fprintf(modelfile, pst, h->states[i].pi);
     switch (h->statetype)
      {
       case          STATE_DISCRETE:m = (Multinomialptr) (h->states[i].b);
                                    for (j = 0; j < d->featurecount; j++)
                                      if (d->features[j].type == STRING)
                                        save_array(modelfile, m->prob[j], d->features[j].valuecount);
                                    break;
       case          STATE_GAUSSIAN:g = (Gaussianptr) (h->states[i].b);
                                    save_array(modelfile, g->mean, d->multifeaturecount - 1);
                                    save_matrix(modelfile, g->covariance);
                                    break;
       case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) (h->states[i].b);
                                    fprintf(modelfile, "%d\n", gm->componentcount);
                                    save_array(modelfile, gm->weights, gm->componentcount);
                                    for (j = 0; j < gm->componentcount; j++)
                                     {
                                      save_array(modelfile, gm->components[j].mean, d->multifeaturecount - 1);
                                      save_matrix(modelfile, gm->components[j].covariance); 
                                     }
                                    break;
       case         STATE_DIRICHLET:di = (Dirichletptr) (h->states[i].b);
                                    save_array(modelfile, di->alpha, d->multifeaturecount - 1);
                                    break;
       case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) (h->states[i].b);
                                    fprintf(modelfile, "%d\n", dm->componentcount);
                                    save_array(modelfile, dm->weights, dm->componentcount);
                                    for (j = 0; j < dm->componentcount; j++)
                                      save_array(modelfile, dm->components[j].alpha, d->multifeaturecount - 1);
                                    break;
      }
    }
  }
}

void save_model_gprocessreg(FILE* modelfile, Datasetptr d, Model_gprocessregressionptr m)
{
 /*!Last Changed 15.01.2009*/
 Instanceptr tmp;
 fprintf(modelfile, "%d\n", m->size);
 tmp = m->data;
 while (tmp)
  {
   save_instance(modelfile, d, tmp, BOOLEAN_TRUE);
   tmp = tmp->next;
  }
 save_array(modelfile, m->weights, m->size);
}

void save_model_regrule(FILE* modelfile, Datasetptr d, Regressionrulesetptr m)
{
 /*Last Changed 27.12.2005*/
}
