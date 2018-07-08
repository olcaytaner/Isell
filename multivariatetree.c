#include <math.h>
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "matrix.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "mlp.h"
#include "multivariatetree.h"
#include "perceptron.h"
#include "regressiontree.h"
#include "rule.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

/**
 * Recursive function that returns the number of multivariate linear decision nodes
 * @param[in] node Decision tree node
 * @return Number of multivariate linear decision nodes in the subtree rooted this node
 */
int decisiontree_multivariate_linear_node_count(Decisionnodeptr node)
{
 /*!Last Changed 24.11.2004*/
 if (node->featureno == LEAF_NODE)
   return 0;
 else
   if (node->featureno == LINEAR)
     return 1 + decisiontree_multivariate_linear_node_count(node->left) + decisiontree_multivariate_linear_node_count(node->right);
   else
     return decisiontree_multivariate_linear_node_count(node->left) + decisiontree_multivariate_linear_node_count(node->right);
}

/**
 * Recursive function that returns the number of multivariate quadratic decision nodes
 * @param[in] node Decision tree node
 * @return Number of multivariate quadratic decision nodes in the subtree rooted this node
 */
int decisiontree_multivariate_quadratic_node_count(Decisionnodeptr node)
{
 /*!Last Changed 29.01.2005*/
 if (node->featureno == LEAF_NODE)
   return 0;
 else
   if (node->featureno == QUADRATIC)
     return 1 + decisiontree_multivariate_quadratic_node_count(node->left) + decisiontree_multivariate_quadratic_node_count(node->right);
   else
     return decisiontree_multivariate_quadratic_node_count(node->left) + decisiontree_multivariate_quadratic_node_count(node->right);
}

/**
 * Constructor for a multivariate condition.
 * @param[in] w Weight matrix for the features (or 2-d features)
 * @param[in] w0 Bias value
 * @param[in] comparison Character describing the type of comparison (\< for smaller, \> for larger etc.)
 * @param[in] node_type Type of decision tree node
 * @return A multivariate condition initialized
 */
Decisioncondition createmultivariatecondition(matrix w, double w0, char comparison, int node_type)
{
/*!Last Changed 04.10.2004*/
 Decisioncondition condition;
 condition.w = matrix_copy(w);
 condition.w0 = w0;
 condition.comparison = comparison;
 condition.featureindex = node_type;
 return condition;
}

/**
 * Tests a multivariate split with the given test data. 
 * @param[in] testingdata Header of the link list containing the test instances
 * @param[in] w Weight matrix of the multivariate split
 * @param[in] w0 Bias value of the multivariate split
 * @param[in] leftclass Correct class for the instances whose linear combination of features has the value less than w0
 * @param[in] rightclass Correct class for the instances whose linear combination of features has the value larger than the split
 * @return Number of instances in the test data that are incorrectly classified.
 */
int test_multivariate(Instanceptr testingdata, matrix w, double w0, int leftclass, int rightclass)
{
 /*!Last Changed 23.09.2004*/
 int i, error = 0;
 double value;
 Instanceptr test = testingdata;
 while (test)
  {
   value = multiply_with_matrix(test, w);
   i = give_classno(test);
   if (value <= w0)
    {
     if (i != leftclass)
       error++;
    }
   else
    {
     if (i != rightclass)
       error++;
    }
   test = test->next;
  }
 return error;
}

/**
 * Frees memory allocated to the conditions in the rules in this node
 * @param[in] node Decision tree node
 */
void deallocate_multivariate_conditions(Decisionnodeptr node)
{
 /*!Last Changed 04.10.2004*/
 int i;
 for (i = 0; i < node->conditioncount; i++)
   free_condition(node->rule[i]);
}

int best_multivariate_split_lp(Instanceptr positives, Instanceptr negatives, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim)
{
 /*!Last Changed 26.05.2005*/
 /*!Last Changed 13.02.2005*/
 Mlpparameters mlpparams;
 Mlpnetwork neuralnetwork;
 matrix train1, train2, train;
 int i, dim;
 if (multivariate_type == MULTIVARIATE_LINEAR)
   dim = current_dataset->multifeaturecount - 1;
 else
   dim = multifeaturecount_2d(current_dataset);
 mlpparams = prepare_Mlpparameters(positives, positives, 0, 0.0, 0, NULL, 0, 0.0, 0.0, 0, CLASSIFICATION);
 mlpparams.inputnum = data_size(positives) + data_size(negatives);
 mlpparams.cvnum = mlpparams.inputnum;
 train1 = instancelist_to_matrix(positives, 3, 1, multivariate_type);
 train2 = instancelist_to_matrix(negatives, 3, 0, multivariate_type);
 train = matrix_merge_rows(train1, train2);
 matrix_free(train1);
 matrix_free(train2);
 neuralnetwork = mlpn_general(train, train, &mlpparams, CLASSIFICATION);
 *newdim = dim;
 *w0 = neuralnetwork.weights->values[1][0] - neuralnetwork.weights->values[0][0];
 *w = matrix_alloc(1, dim);
 for (i = 0; i < dim; i++)
   (*w).values[0][i] = neuralnetwork.weights->values[0][i + 1] - neuralnetwork.weights->values[1][i + 1];
 free_mlpnnetwork(&neuralnetwork);
 matrix_free(train);
 return 1;
}

int best_multivariate_split_lda_with_pca(Instanceptr positives, Instanceptr negatives, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained)
{
 /*!Last Changed 12.01.2009 added pooled_covariance*/
 /*!Last Changed 21.09.2004*/
 int positivecount, negativecount, newdimension, dim, matrixdim;
 Instance meanpositive, meannegative; 
 /* 1 x 1 matrices*/
 matrix splitmatrix;
 /* 1 x d matrices*/
 matrix meanposmatrix, meannegmatrix, meandifference, meansum;
 /* 1 x k matrices*/
 matrix bmatrix, tmpresult2, tmpresult3;
 /* k x 1 matrices*/
 matrix transposeresult;
 /* d x d matrices*/
 matrix covariancenegative, covariancepositive, totalcovariance, tmpeigenvec;
 /* d x k matrices*/
 matrix eigenvec;
 /* k x d matrices*/
 matrix transposeeigenvec, tmpresult;
 /* k x k matrices*/
 matrix sw;
 double* eigenval = NULL, posratio, negratio;
 positivecount = data_size(positives);
 negativecount = data_size(negatives);
 dim = current_dataset->multifeaturecount - 1;
 if (positivecount <= 1 || negativecount <= 1)
  {
   *w = identity(1);
   return 0;
  }
 posratio = (positivecount + 0.0) / (positivecount + negativecount);
 negratio = (negativecount + 0.0) / (positivecount + negativecount);
 if (multivariate_type == MULTIVARIATE_LINEAR)
  {
   meanpositive = find_mean(positives);
   meannegative = find_mean(negatives);
   meanposmatrix = instance_to_matrix(&meanpositive);
   meannegmatrix = instance_to_matrix(&meannegative);
   covariancepositive = covariance(positives, meanpositive);
   covariancenegative = covariance(negatives, meannegative);
   matrixdim = dim;
   safe_free(meanpositive.values);
   safe_free(meannegative.values);
  }
 else /*QUADRATIC*/
  {   
   meanposmatrix = find_mean_2d(positives);
   meannegmatrix = find_mean_2d(negatives);
   matrixdim = multifeaturecount_2d(current_dataset);
   covariancepositive = covariance_2d(positives, meanposmatrix);
   covariancenegative = covariance_2d(negatives, meannegmatrix);
  }
 totalcovariance = pooled_covariance(covariancepositive, positivecount, covariancenegative, negativecount);
 matrix_free(covariancepositive);
 matrix_free(covariancenegative);
 eigenval = find_eigenvalues_eigenvectors(&tmpeigenvec, totalcovariance, &newdimension, variance_explained);
 safe_free(eigenval);
 *newdim = newdimension + 1;
 eigenvec = matrix_partial(tmpeigenvec, 0, matrixdim - 1, 0, newdimension);
 matrix_free(tmpeigenvec);
 transposeeigenvec = matrix_transpose(eigenvec);
 tmpresult = matrix_multiply(transposeeigenvec, totalcovariance);
 matrix_free(totalcovariance);
 sw = matrix_multiply(tmpresult, eigenvec);
 matrix_free(tmpresult);
 meandifference = matrix_difference(meanposmatrix, meannegmatrix);
 bmatrix = matrix_multiply(meandifference, eigenvec);
 matrix_free(meandifference);
 matrix_inverse(&sw);
 tmpresult2 = matrix_multiply(bmatrix, sw);
 *w = matrix_multiply(tmpresult2, transposeeigenvec);
 matrix_free(transposeeigenvec);
 matrix_free(bmatrix);
 matrix_free(sw);
 meansum = matrix_sum(meanposmatrix, meannegmatrix);
 matrix_free(meanposmatrix);
 matrix_free(meannegmatrix);
 tmpresult3 = matrix_multiply(meansum, eigenvec);
 matrix_free(eigenvec);
 matrix_free(meansum);
 transposeresult = matrix_transpose(tmpresult2);
 matrix_free(tmpresult2);
 splitmatrix = matrix_multiply(tmpresult3, transposeresult);
 matrix_free(tmpresult3);
 matrix_free(transposeresult);
 *w0 = -(-0.5*splitmatrix.values[0][0]-log(negratio / posratio));
 matrix_free(splitmatrix);
 return 1;
}

int best_multivariate_split_lda_with_svd(Instanceptr positives, Instanceptr negatives, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained)
{
 /*!Last Changed 12.01.2009 added pooled_covariance*/
 /*!Last Changed 21.09.2004*/
 int positivecount, negativecount;
 Instance meanpositive, meannegative; 
 /* 1 x 1 matrices*/
 matrix splitmatrix;
 /* 1 x d matrices*/
 matrix meanposmatrix, meannegmatrix, meandifference, meansum;
 /* d x d matrices*/
 matrix sw, transposeresult, covariancenegative, covariancepositive, totalcovariance;
 double posratio, negratio;
 positivecount = data_size(positives);
 negativecount = data_size(negatives);
 if (positivecount <= 1 || negativecount <= 1)
  {
   *w = identity(1);
   return 0;
  }
 posratio = (positivecount + 0.0) / (positivecount + negativecount);
 negratio = (negativecount + 0.0) / (positivecount + negativecount);
 if (multivariate_type == MULTIVARIATE_LINEAR)
  {
   meanpositive = find_mean(positives);
   meannegative = find_mean(negatives);
   meanposmatrix = instance_to_matrix(&meanpositive);
   meannegmatrix = instance_to_matrix(&meannegative);
   covariancepositive = covariance(positives, meanpositive);
   covariancenegative = covariance(negatives, meannegative);
   safe_free(meanpositive.values);
   safe_free(meannegative.values);
  }
 else /*QUADRATIC*/
  {   
   meanposmatrix = find_mean_2d(positives);
   meannegmatrix = find_mean_2d(negatives);
   covariancepositive = covariance_2d(positives, meanposmatrix);
   covariancenegative = covariance_2d(negatives, meannegmatrix);
  }
 totalcovariance = pooled_covariance(covariancepositive, positivecount, covariancenegative, negativecount); 
 matrix_free(covariancepositive);
 matrix_free(covariancenegative);
 sw = pseudoinverse(totalcovariance, variance_explained, newdim);
 matrix_free(totalcovariance);
 meandifference = matrix_difference(meanposmatrix, meannegmatrix);
 *w = matrix_multiply(meandifference, sw);
 matrix_free(sw);
 matrix_free(meandifference);
 meansum = matrix_sum(meanposmatrix, meannegmatrix);
 matrix_free(meanposmatrix);
 matrix_free(meannegmatrix);
 transposeresult = matrix_transpose(*w);
 splitmatrix = matrix_multiply(meansum, transposeresult);
 matrix_free(meansum);
 matrix_free(transposeresult);
 *w0 = -(-0.5*splitmatrix.values[0][0]-log(negratio / posratio));
 matrix_free(splitmatrix);
 return 1;
}

int make_multivariate_children(Decisionnodeptr node)
{
 /*!Last Changed 04.10.2004 added rules*/
 /*!Last Changed 26.09.2004 added parent*/
 /*!Last Changed 21.09.2004*/
 Instanceptr inst, leftbefore=NULL, rightbefore=NULL;
 Decisionnodeptr left, right;
 double value, threshold;
 Decisioncondition ruleleft, ruleright;
 if (node->featureno == LEAF_NODE || node->instances == NULL)
  {
   if (in_list(node->featureno, 2, LINEAR, QUADRATIC))
     matrix_free(node->w);
   make_leaf_node(node);
   return 0;
  }
 left = createnewnode(node, 1);
 right = createnewnode(node, 1);
 node->string = NULL;
 node->left = left;
 node->right = right;
 inst = node->instances;
 if (node->conditiontype != HYBRID_CONDITION)
   node->featureno = LINEAR;
 if (in_list(node->featureno, 2, LINEAR, QUADRATIC))
  {
   ruleleft = createmultivariatecondition(node->w, node->w0, '<', node->featureno);
   ruleright = createmultivariatecondition(node->w, node->w0, '>', node->featureno);
  }
 else
  {
   ruleleft = createcondition(node->featureno, '<', node->split);
   ruleright = createcondition(node->featureno, '>', node->split);
  }
 addcondition(node->left, ruleleft);
 addcondition(node->right, ruleright);
 while (inst)
  {
   if (in_list(node->featureno, 2, LINEAR, QUADRATIC))
    {
     value = multiply_with_matrix(inst, node->w);
     threshold = node->w0;
    }
   else
    {
     value = real_feature(inst, node->featureno);
     threshold = node->split;
    }
   if (value <= threshold)
    {
     if (leftbefore)
       leftbefore->next = inst;
     else
       left->instances = inst;
     leftbefore = inst;
    }
   else
    {
     if (rightbefore)
       rightbefore->next = inst;
     else
       right->instances = inst;
     rightbefore = inst;
    }
   inst = inst->next;
  }
 if (leftbefore)
   leftbefore->next = NULL;
 if (rightbefore)
   rightbefore->next = NULL;
 node->instances = NULL;
 return 1;
}

void find_counts_for_multivariate_split(Instanceptr instances, matrix w, double w0, int* leftcounts, int* rightcounts)
{
 /*!Last Changed 22.09.2004*/
 int j;
 double value;
 Instanceptr tmp = instances;
 for (j = 0; j < MAXCLASSES; j++)
  {
   leftcounts[j] = 0;
   rightcounts[j] = 0;
  }
 while (tmp)
  {
   value = multiply_with_matrix(tmp, w);
   j = give_classno(tmp);
   if (value <= w0)
     leftcounts[j]++;
   else
     rightcounts[j]++;
   tmp = tmp->next;
  }
}

double information_gain_for_multivariate_split(Instanceptr instances, matrix w, double w0)
{
/*!Last Changed 22.09.2004 */
 int counts[2][MAXCLASSES];
 find_counts_for_multivariate_split(instances, w, w0, counts[0], counts[1]);
 return information_gain(counts);
}

int find_split_for_partition_multivariate(Instanceptr* instances, Partition p, matrix* w, double*  w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained)
{
 /*!Last Changed 21.09.2004*/
 int result;
 Instanceptr positives = NULL, negatives = NULL;
 divide_instancelist_according_partition(instances, &positives, &negatives, p);
 result = best_multivariate_split_lda_with_pca(positives, negatives, w, w0, multivariate_type, newdim, variance_explained);
 *instances = positives;
 merge_instancelist(instances, negatives);
 return result;
}

double find_gain_for_partition_multivariate(Instanceptr* instances, Partition p, matrix* w, double* w0, MULTIVARIATE_TYPE multivariate_type, int* newdim, double variance_explained)
{
 /*!Last Changed 21.09.2004*/
 if (p.count == 0 || instances == NULL || all_same_side(p))
  {
   *w = identity(1);
   return ISELL;
  }
 if (!find_split_for_partition_multivariate(instances, p, w, w0, multivariate_type, newdim, variance_explained))
   return ISELL;
 return information_gain_for_multivariate_split(*instances, *w, *w0);
}

double log_likelihood_for_multi_ldt_splits(Instanceptr data, matrix w, double w0)
{
 int counts[2][MAXCLASSES];
 double result = 0;
 find_counts_for_multivariate_split(data, w, w0, counts[0], counts[1]);
 result += log_likelihood_for_classification(counts[0]);
 result += log_likelihood_for_classification(counts[1]);
 return result;
}

int find_best_multi_ldt_split(Instanceptr* instances, matrix* bestw, double *bestw0, MULTIVARIATE_TYPE multivariate_type, double variance_explained)
{
 /*!Last Changed 22.09.2004*/
 int improved, j, bestdim, dim;
 Partition initial,current,best;
 double bestgain, gain, w0;
 matrix w;
 initial = get_initial_partition(*instances);
 if (initial.count <= 2)
   bestgain = find_gain_for_partition_multivariate(instances, initial, bestw, bestw0, multivariate_type, &bestdim, variance_explained);
 else
  {
   current = create_copy(initial);
   bestgain = find_gain_for_partition_multivariate(instances, current, bestw, bestw0, multivariate_type, &bestdim, variance_explained);
   best = create_copy(current);
   improved = 1;
   while (improved)
    {
     improved = 0;
     for (j = 0; j < current.count; j++)
      {
       current.assignments[j] = 1 - current.assignments[j];
       gain = find_gain_for_partition_multivariate(instances, current, &w, &w0, multivariate_type, &dim, variance_explained);
       if (dless(gain, bestgain))
        {
         bestdim = dim;
         bestgain = gain;
         free_partition(best);
         best = create_copy(current);
         *bestw0 = w0;
         matrix_free(*bestw);
         *bestw = matrix_copy(w);
         matrix_free(w);
         improved = 1;
        }
       else
         matrix_free(w);
       current.assignments[j] = 1 - current.assignments[j];
      }
     if (improved)
      {
       free_partition(current);
       current = create_copy(best);
      }
    }
   free_partition(best);
   free_partition(current);
  }
 free_partition(initial);
 if (bestgain == ISELL)
   return 0;
 return bestdim;
}

int create_multildtchildren(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 21.09.2004*/
 node->conditiontype = MULTIVARIATE_CONDITION;
 if (!(can_be_divided(node, &(param->c45param))))
   return 1;
 node->lineard = find_best_multi_ldt_split(&(node->instances),&(node->w),&(node->w0), MULTIVARIATE_LINEAR, param->variance_explained);
 if (!(node->lineard))
  {
   matrix_free(node->w);
   make_leaf_node(node);
   return 1;
  }
 node->featureno = LINEAR; 
 if (!(make_multivariate_children(node)))
   return 1;
 create_multildtchildren(node->left, param);
 create_multildtchildren(node->right, param);
 return 1;
}
