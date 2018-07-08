#include <math.h>
#include "dataset.h"
#include "decisiontree.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "multivariatetree.h"
#include "statistics.h"
#include "univariatetree.h"
#include "vc.h"

/**
 * Calculates the VC-dimension of univariate decision tree which states that the VC-dimension of the univariate decision tree depends on the number of decision nodes in the tree (N) and the number of features in the dataset (d).
 * @param[in] testdata testdata[0] = log d * N, testdata[1] = log d, testdata[2] = N.
 * @return The VC-dimension of univariate decision tree
 */
double decisiontree_vc_dimension_univariate1(double* testdata)
{
 /*Last Changed 01.10.2006*/
 const double mean[3] = {77.871900,3.053800,25.500000};
 const double variance[3] = {3066.280443,1.307234,208.458458};
 const double weights0[4] = {75.767898 ,45.757710 ,1.083343 ,3.206944 };
 double sum, weights_j, x_j, output;
 int i, j;
 double testdataconverted[3];
 for (i = 0; i < 3; i++)
   testdataconverted[i] = (testdata[i] - mean[i]) / sqrt(variance[i]);
 sum = weights0[0];
 for (j = 1; j < 4; j++)
  {
   weights_j = weights0[j];
   x_j = testdataconverted[j - 1];
   sum = sum + weights_j * x_j;
  }
 output = sum;
 return output;
}

/**
 * Calculates the VC-dimension of multivariate linear decision tree which states that the VC-dimension of the multivariate linear decision tree depends on the number of decision nodes in the tree (N) and the number of features in the dataset (d).
 * @param[in] testdata testdata[0] = d * N, testdata[1] = d, testdata[2] = N.
 * @return The VC-dimension of multivariate linear decision tree
 */
double decisiontree_vc_dimension_linear1(double* testdata)
{
 /*Last Changed 01.10.2006*/
 const double mean[3] = {267.750000,10.500000,25.500000};
 const double variance[3] = {51556.243744,33.283283,208.458458};
 const double weights0[4] = {85.843669 ,47.643896 ,2.714944 ,11.436310 };
 double sum, weights_j, x_j, output;
 int i, j;
 double testdataconverted[3];
 for (i = 0; i < 3; i++)
   testdataconverted[i] = (testdata[i] - mean[i]) / sqrt(variance[i]);
 sum = weights0[0];
 for (j = 1; j < 4; j++)
  {
   weights_j = weights0[j];
   x_j = testdataconverted[j - 1];
   sum = sum + weights_j * x_j;
  }
 output = sum;
 return output;
}

/**
 * Calculates the VC-dimension of multivariate quadratic decision tree which states that the VC-dimension of the multivariate quadratic decision tree depends on the number of decision nodes in the tree (N) and the number of features in the dataset (d).
 * @param[in] testdata testdata[0] = d * d * N, testdata[1] = d * d, testdata[2] = d * N, testdata[3] = d, testdata[4] = N.
 * @return The VC-dimension of multivariate quadratic decision tree
 */
double decisiontree_vc_dimension_quadratic1(double* testdata)
{
 /*Last Changed 01.10.2006*/
 const double mean[5] = {596.750000,38.500000,85.250000,5.500000,15.500000};
 const double variance[5] = {443780.422241,1054.565217,4882.629599,8.277592,75.167224};
 const double weights0[6] = {92.533543 ,1.971860 ,12.871168 ,50.327602 ,6.863087 ,4.789140 };
 double sum, weights_j, x_j, output;
 int i, j;
 double testdataconverted[5];
 for (i = 0; i < 5; i++)
   testdataconverted[i] = (testdata[i] - mean[i]) / sqrt(variance[i]);
 sum = weights0[0];
 for (j = 1; j < 6; j++)
  {
   weights_j = weights0[j];
   x_j = testdataconverted[j - 1];
   sum = sum + weights_j * x_j;
  }
 output = sum;
 return output;
}

/**
 * In the VC-dimension estimation algorithms, one need a pre-estimate of the VC-dimension of the classifier. This pre-estimate will be used as a starting point in the estimation algorithm. This function calculates the pre-estimate of the discrete univariate decision tree. Its calculated as log d * N, where d is the number of features in the dataset, N is the number of decision nodes in the tree.
 * @param[in] d The number of features in the dataset
 * @param[in] nodecount The number of decision nodes in the discrete univariate tree.
 * @return Pre-estimate of the VC-dimension of the univariate decision tree with discrete features.
 */
int decisiontree_vc_dimension_preestimation_discrete(Datasetptr d, int nodecount)
{
 /*Last Changed 01.10.2006*/
 /*Last Changed 23.05.2006*/
 if (nodecount == 0 || (d->featurecount - log2(nodecount)) < 0)
   return 1;
 return (int) (log2(d->featurecount - log2(nodecount) + 2) * nodecount);
}

/**
 * In the VC-dimension estimation algorithms, one need a pre-estimate of the VC-dimension of the classifier. This pre-estimate will be used as a starting point in the estimation algorithm. This function calculates the pre-estimate of the (i) univariate, (ii) multivariate linear, and (iii) multivariate quadratic decision tree. They are calculates as (i) (log d + 1) * N, (ii) (d + 1) * N, and (iii) ((d + 1) (d + 2) / 2) * N, where d is the number of features in the dataset, N is the number of decision nodes in the tree.
 * @param[in] d The number of features in the dataset
 * @param[in] nodecount The number of decision nodes in the discrete univariate tree.
 * @param[in] nodetype Type of the decision tree. MODEL_UNI for univariate, MODEL_LIN for multivariate linear, and MODEL_QUA for multivariate quadratic decision trees.
 * @return Pre-estimate of the VC-dimension of the univariate, multivariate linear, and multivariate quadratic decision tree.
 */
int decisiontree_vc_dimension_preestimation(Datasetptr d, int nodecount, MODEL_TYPE nodetype)
{
 /*Last Changed 06.11.2007*/
 int dim = d->multifeaturecount - 1;
 switch (nodetype)
  {
   case MODEL_UNI:return (nodecount + 1) * (log2(dim + 1) + 1) / 2;
                  break;
   case MODEL_LIN:return nodecount * (dim + 1);
                  break;
   case MODEL_QUA:return nodecount * (((dim + 1) * (dim + 2)) / 2);
                  break;
  }
 return 1;
}

/**
 * Calculates the VC-dimension of the univariate decision tree with discrete features. The formula depends on the number of decision nodes in the subtree (N), the number of features in the dataset (D). If there are no decision nodes returns 1.
 * @param[in] d Number of features in the dataset
 * @param[in] rootnode Root node of the subtree
 * @return The VC-dimension of the univariate decision tree with discrete features
 */
double decisiontree_vc_dimension_discrete_old(Datasetptr d, Decisionnodeptr rootnode)
{
 /*!Last Changed 04.11.2009*/
 int D, N;
 if (rootnode->featureno == LEAF_NODE)
   return 1;
 D = d->featurecount - 1;
 N = decisiontree_node_count(rootnode);
 return 0.6734 + 1.0117 * N + 0.1939 * log2(D - log2(N)) * N + 0.7147 * log2(D - log2(N));
}

/**
 * Calculates the VC-dimension of the univariate decision tree with discrete features. The formula is recursive, which depends on the number of decision nodes in the subtree (N), the number of features in the dataset (D). If there are no decision nodes returns 1.
 * @param[in] d Number of features in the dataset
 * @param[in] rootnode Root node of the subtree
 * @return The VC-dimension of the univariate decision tree with discrete features
 */
double decisiontree_vc_dimension_discrete_recursive_old(int d, Decisionnodeptr rootnode)
{
 /*Last Changed 20.04.2008*/
 int N;
 double leftplusright;
 if (rootnode->featureno == LEAF_NODE)
   return 1;
 N = decisiontree_node_count(rootnode);
 leftplusright = decisiontree_vc_dimension_discrete_recursive_old(d - 1, &(rootnode->string[0])) + decisiontree_vc_dimension_discrete_recursive_old(d - 1, &(rootnode->string[1]));
 return 0.6076 + 1.0077 * leftplusright + 0.3657 * log2(d) - 0.8058 * log2(N);
}

/**
 * Calculates the VC-dimension of the univariate decision tree with discrete features. The formula is recursive, which depends on the number of features in the dataset (D).
 * @param[in] d Number of features in the dataset
 * @param[in] rootnode Root node of the subtree
 * @return The VC-dimension of the univariate decision tree with discrete features
 */
double decisiontree_vc_dimension_discrete_recursive(Datasetptr current_dataset, int d, Decisionnodeptr rootnode)
{
 int i, sum;
 BOOLEAN all_leaves = BOOLEAN_TRUE;
 if (rootnode->featureno == LEAF_NODE)
   return 1;
 for (i = 0; i < current_dataset->features[rootnode->featureno].valuecount; i++)
   if (rootnode->string[i].featureno != LEAF_NODE)
     all_leaves = BOOLEAN_FALSE;
 if (all_leaves)
  {
   sum = 1;
   for (i = 0; i < current_dataset->featurecount; i++)
     if (current_dataset->features[i].type != CLASSDEF)
       sum += pow(2, current_dataset->features[i].valuecount - 1) - 1; 
   return log2(sum) + 1;
  }
 sum = 0;
 for (i = 0; i < current_dataset->features[rootnode->featureno].valuecount; i++)
   sum += ((int) decisiontree_vc_dimension_discrete_recursive(current_dataset, d - 1, &(rootnode->string[i])));
 return sum;
}

/**
 * Calculates the VC-dimension of the univariate decision tree with continuous features. The formula is recursive, which depends on the number of features in the dataset (D).
 * @param[in] current_dataset Pointer to the current dataset
 * @param[in] rootnode Root node of the subtree
 * @return The VC-dimension of the univariate decision tree with discrete features
 */
double decisiontree_vc_dimension_continuous_recursive(Datasetptr current_dataset, Decisionnodeptr rootnode)
{
 if (rootnode->featureno == LEAF_NODE)
   return 1;
 if (rootnode->left->featureno == LEAF_NODE && rootnode->right->featureno == LEAF_NODE)
   return log2(current_dataset->featurecount - 1) + 1;
 return decisiontree_vc_dimension_continuous_recursive(current_dataset, rootnode->left) + decisiontree_vc_dimension_continuous_recursive(current_dataset, rootnode->right);
}

int decisiontree_vc_dimension_recursive(Datasetptr current_dataset, Decisionnodeptr root)
{
 if (number_of_symbolic_features(current_dataset->name) == current_dataset->featurecount - 1)
   return (int) decisiontree_vc_dimension_discrete_recursive(current_dataset, current_dataset->featurecount - 1, root);
 else
   if (number_of_numeric_features(current_dataset->name) == current_dataset->featurecount - 1)
     return decisiontree_vc_dimension_continuous_recursive(current_dataset, root);
   else
     return 1;
}


/**
 * Calculates the VC-dimension of the (i) univariate, (ii) multivariate linear, and (iii) multivariate quadratic decision tree using (i) decisiontree_vc_dimension_univariate1, (ii) decisiontree_vc_dimension_linear1, and (iii) decisiontree_vc_dimension_quadratic1 functions. The formula is not recursive.
 * @param[in] d Number of features in the dataset.
 * @param[in] nodecount Number of decision nodes in the decision tree.
 * @param[in] nodetype Type of the decision tree. MODEL_UNI for univariate, MODEL_LIN for multivariate linear, and MODEL_QUA for multivariate quadratic decision trees.
 * @return The VC-dimension of the univariate, multivariate linear, and multivariate quadratic decision tree.
 */
int decisiontree_vc_dimension_wrt_nodetype(Datasetptr d, int nodecount, MODEL_TYPE nodetype)
{
 /*Last Changed 01.10.2006*/
 double* testdata, logd, dimension, multiplier;
 int dim = d->multifeaturecount - 1, size;
 logd = log2(dim);
 switch (nodetype)
  {
   case MODEL_UNI:multiplier = logd;
                  size = 3;
                  break;
   case MODEL_LIN:multiplier = dim;
                  size = 3;
                  break;
   case MODEL_QUA:multiplier = dim;
                  size = 5;
                  break;
  }
 testdata = safemalloc(size * sizeof(double), "decisiontree_vc_dimension_wrt_nodetype", 10);
 testdata[size - 3] = multiplier * nodecount;
 testdata[size - 2] = multiplier;
 testdata[size - 1] = nodecount;
 switch (nodetype)
  {
   case MODEL_UNI:dimension = decisiontree_vc_dimension_univariate1(testdata);
                  break;
   case MODEL_LIN:dimension = decisiontree_vc_dimension_linear1(testdata);
                  break;
   case MODEL_QUA:testdata[0] = dim * dim * nodecount;
                  testdata[1] = dim * dim;
                  dimension = decisiontree_vc_dimension_quadratic1(testdata);
                  break;
  }
 safe_free(testdata);
 return ((int) dimension);
}

/**
 * Calculates the VC-dimension of the omnivariate decision tree. Since omnivariate decision tree may contain univariate, multivariate linear or multivariate quadratic nodes, the VC-dimension of the omnivariate decision tree will be calculated as the sum of vc dimensions of those decision nodes.
 * @param[in] d Number of features in the dataset.
 * @param[in] node Root node of the omnivariate decision tree.
 * @return The VC-dimension of the omnivariate decision tree.
 */
int omnivariatetree_vc_dimension(Datasetptr d, Decisionnodeptr node)
{
 /*Last Changed 29.09.2006*/
 /*Last Changed 06.03.2006*/
 int uni, linear, quadratic, result = 0;
 uni = decisiontree_univariate_node_count(node);
 result += decisiontree_vc_dimension_wrt_nodetype(d, uni, MODEL_UNI);
 linear = decisiontree_multivariate_linear_node_count(node);
 result += decisiontree_vc_dimension_wrt_nodetype(d, linear, MODEL_LIN);
 quadratic = decisiontree_multivariate_quadratic_node_count(node);
 result += decisiontree_vc_dimension_wrt_nodetype(d, quadratic, MODEL_QUA);
 return result;
}

int ruleset_vc_dimension_discrete_recursive(int* condition_counts, int k, int d)
{
 int i, j, max, current;
 int *left, *right;
 if (k == 1)
  {
   if (condition_counts[0] > -1)
     return (int) (log2(binomdouble(d, condition_counts[0]) + 1) + 1);
   else
     return 1;
  }
 max = 0;
 for (i = 1; i < k; i++)
  {
   left = safemalloc(i * sizeof(int), "ruleset_vc_dimension_discrete_recursive", 8);
   for (j = 0; j < i; j++)
     left[j] = condition_counts[i - 1] - 1;
   right = safemalloc((k - i) * sizeof(int), "ruleset_vc_dimension_discrete_recursive", 11);
   for (j = 0; j < k - i; j++)
     right[j] = condition_counts[i + j] - 1;
   current = ruleset_vc_dimension_discrete_recursive(left, i, d - 1) + ruleset_vc_dimension_discrete_recursive(right, k - i, d - 1);
   if (current > max)
     max = current;
   safe_free(left);
   safe_free(right);
  }
 return max;
}

int ruleset_vc_dimension_discrete(Datasetptr current_dataset, Rulesetptr ruleset)
{
 int *condition_counts;
 int i = 0, vc, d;
 Decisionruleptr rule;
 d = current_dataset->featurecount - 1;
 rule = ruleset->start;
 condition_counts = safemalloc(ruleset->rulecount * sizeof(int), "ruleset_vc_dimension_discrete", 6);
 while (rule)
  {
   condition_counts[i] = rule->decisioncount;
   rule = rule->next;
   i++;
  }
 vc = ruleset_vc_dimension_discrete_recursive(condition_counts, ruleset->rulecount, d);
 safe_free(condition_counts);
 return vc;
}

int ruleset_vc_dimension_continuous(Datasetptr current_dataset, Rulesetptr ruleset)
{
 int sum = 0, d;
 Decisionruleptr rule;
 d = current_dataset->featurecount - 1;
 rule = ruleset->start;
 while (rule)
  {
   if (rule->decisioncount < 2)
     sum += 2;
   else
     sum += (int) (log2(binomdouble(d - 1, rule->decisioncount - 2) + 1) + 1);
   rule = rule->next;
  }
 return sum;
}

int ruleset_vc_dimension(Datasetptr current_dataset, Rulesetptr ruleset)
{
 if (number_of_symbolic_features(current_dataset->name) == current_dataset->featurecount - 1)
   return ruleset_vc_dimension_discrete(current_dataset, ruleset);
 else
    if (number_of_numeric_features(current_dataset->name) == current_dataset->featurecount - 1)
      return ruleset_vc_dimension_continuous(current_dataset, ruleset);
    else
      return 1;
}
