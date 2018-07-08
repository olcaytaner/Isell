#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "clustering.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "messages.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "model.h"
#include "options.h"
#include "perceptron.h"
#include "regressiontree.h"
#include "statistics.h"
#include "svmtree.h"
#include "tests.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;
extern double* absoluteerrors;

/**
 * Finds out which of the two distances are smaller: The euclidian distance between an instance and the mean vector of the instances in the left child, or the distance between that instance and the mean vector of the instances in the right child.
 * @param[in] tmp The instance
 * @param[in] nodetype Type of function in the regression node. QUADRATIC for multivariate quadratic split, LINEAR for multivariate linear split.
 * @param[in] leftcenter Mean vector of the instances stored in the left child
 * @param[in] rightcenter Mean vector of the instances stored in the right child
 * @return LEFTCHILD if the distance to the left child's mean vector is smaller, RIGHTCHILD otherwise.
 */
CHILD_TYPE nearest_center_in_regression_tree(Instanceptr tmp, NODE_TYPE nodetype, Instanceptr leftcenter, Instanceptr rightcenter)
{
	/*!Last Changed 24.03.2005*/
	int oldfcount;
	Instanceptr tmp2d;
	double dist1, dist2;
	if (nodetype == QUADRATIC)
		{
			tmp2d = generate_2d_copy_of_instance(tmp);
   oldfcount = current_dataset->multifeaturecount;
	  current_dataset->multifeaturecount = multifeaturecount_2d(current_dataset) + 1;
			dist1 = distance_between_instances(tmp2d, leftcenter);
			dist2 = distance_between_instances(tmp2d, rightcenter);
   current_dataset->multifeaturecount = oldfcount;
			deallocate(tmp2d);
		}
	else
		{
		 dist1 = distance_between_instances(tmp, leftcenter);
		 dist2 = distance_between_instances(tmp, rightcenter);
		}
	if (dist1 < dist2)
		 return LEFTCHILD;
	else
		 return RIGHTCHILD;
}

/**
 * Recursive function which generates a string containing level of the node, regression tree node type, number of instances in that node using the preorder traversal. The string is something like 1;U;5 which says that the node is in the first level, the node is a univariate node (L for linear, Q for quadratic nodes) and has 5 instances.
 * @param[in] node Regression tree node
 * @param[out] st[] Output string
 * @param[in] level Level of the node
 * @warning The string must not exceed READLINELENGTH characters.
 */
void regressiontree_node_types(Regressionnodeptr node, char st[READLINELENGTH], int level)
{
	if (node->featureno != LEAF_NODE)
	 {
		 if (node->featureno >= 0)
				 sprintf(st,"%s%d-U-%d;", st, level, node->covered);
			else
				 if (node->featureno == LINEAR)
						 sprintf(st,"%s%d-L-%d;", st, level, node->covered);
					else
						 if (node->featureno == QUADRATIC) 
								 sprintf(st,"%s%d-Q-%d;", st, level, node->covered);
   regressiontree_node_types(node->left, st, level + 1);
   regressiontree_node_types(node->right, st, level + 1);
	 }
}

/**
 * Recursive function to calculate the regression error of a decision tree subtree. If the node is a leaf node, it adds the squared error. If the node is a nonleaf node, it calculates the square error of the left subtree and the right subtree and adds them.
 * @param[in] node Regression tree root node of the regression subtree
 * @return Squared error of a regression subtree
 */
double mse_error_of_regression_tree(Regressionnodeptr node)
{
 /*!Last Changed 02.03.2005*/
	if (node->featureno == LEAF_NODE)
		 return node->mseerror;
	else
		 return mse_error_of_regression_tree(node->left) + mse_error_of_regression_tree(node->right);
}

int regressiontree_leaf_count(Regressionnodeptr node)
{
 int result = 0;
 if (node->featureno == LEAF_NODE)
   return 1;
 else
  {
   result += regressiontree_leaf_count(node->left);
   result += regressiontree_leaf_count(node->right);
   return result;
  }
}

int regressiontree_node_count(Regressionnodeptr node)
{
 int result = 1;
 if (node->featureno == LEAF_NODE)
   return 0;
 else
  {
   result += regressiontree_node_count(node->left);
   result += regressiontree_node_count(node->right);
  }
 return result;
}

/**
 * Calculates the total complexity of a regression subtree. The complexity of a regression subtree is: \n
 * Complexity of a leaf node if the subtree is a leaf node. \n
 * Sum of complexities of left subtree and right subtree and the complexity of a regression node \n
 * The complexity of a leaf node is: \n
 * 1 if the regression fit is constant \n
 * d if the regression fit is linear \n
 * The complexity of a nonleaf regression node is: \n
 * 2 if the node is a univariate \n
 * (d + 1) if the node is a multivariate linear node where d is the number of features in the dataset \n
 * ((d + 1)(d + 2)) / 2 if the node is a multivariate quadratic node where d is the number of features in the dataset
 * @param[in] node Regression tree root node of the regression subtree
 * @return The overall complexity of the given regression subtree
 */
int complexity_of_regression_tree(Regressionnodeptr node)
{
	/*!Last Changed 24.03.2005*/
 /*!Last Changed 02.03.2005*/
	int d;
	if (node->featureno == LEAF_NODE)
  	if (node->leaftype == CONSTANTLEAF)
	  	 return 1;
	  else
		   return current_dataset->multifeaturecount;
	else
	 {
		 if (node->featureno >= 0)
				 d = 2;
			else
				 if (node->featureno == LINEAR)
						 d = current_dataset->multifeaturecount;
					else
       if (node->featureno == QUADRATIC)
						   d = multifeaturecount_2d(current_dataset) + 1;
       else
         d = 2;
		 return d + complexity_of_regression_tree(node->left) + complexity_of_regression_tree(node->right);
	 }
}

/**
 * Makes a regression node a leaf node.
 * @param[in] node Regression tree node
 * @param[in] leaftype Type of regression fit in the leaf node. CONSTANTLEAF if the regression fit is constant, LINEARLEAF if the regression fit is linear.
 */
void make_leaf_regression_node(Regressionnodeptr node, LEAF_TYPE leaftype)
{
	/*!Last Changed 25.08.2005 added leaftype*/
 /*!Last Changed 01.03.2005*/
 node->featureno = LEAF_NODE;
	node->leaftype = leaftype;
 node->left = NULL;
 node->right = NULL;
	node->string = NULL;
}

/**
 * Finds best univariate split (best feature and threshold pair) at a regression node where one compares different splits based on errors found using linear regression fits.
 * @param[in] instances Instance list at the regression node
 * @param[out] split Best split threshold
 * @param[out] comparison Best split character '<' for smaller '<' for larger
 * @return Best feature. If there is no possibility for a split returns -1 (There are k instances in the node where all feature values of all those instances are equal).
 */
int best_regression_feature_linear_leaves(Instanceptr instances, double* split, char* comparison)
{
	/*!Last Changed 28.02.2005*/
 int bestfeature = -1, i, size, k;
 Instanceptr sorted;
	matrix w, X, r;
 double value, valuebefore, mse, bestmse = +INT_MAX;
	double lefterror, righterror;
 for (i = 0; i < current_dataset->featurecount;i++)
   switch (current_dataset->features[i].type)
    {
     case REAL   :sorted = sort_instances(instances, i, &size);
						            k = 0;
                  valuebefore = -INT_MAX;
                  while (k < size)
                   {
                    value = sorted[k].values[i].floatvalue;
                    if (value != valuebefore)
                     {
                      if (valuebefore != -INT_MAX)
                       {
                        create_regression_matrices_for_array_of_instances(sorted, 0, k - 1, &X, &r);
      		                w = multivariate_regression(X, r);
																								matrix_free(X);
																								matrix_free(r);
				                    lefterror = variance_of_linear_regression_for_array_of_instances(w, sorted, 0, k - 1, LINEARLEAF);
                        matrix_free(w);
                        create_regression_matrices_for_array_of_instances(sorted, k, size - 1, &X, &r);
      		                w = multivariate_regression(X, r);
																								matrix_free(X);
																								matrix_free(r);
				                    righterror = variance_of_linear_regression_for_array_of_instances(w, sorted, k, size - 1, LINEARLEAF);
                        matrix_free(w);
																							 mse = lefterror + righterror;
																								if (mse < bestmse)
																								 {
																									 if ((lefterror / k) < (righterror / (size - k)))
																											 *comparison = '<';
																										else
																											 *comparison = '>';
																									 bestmse = mse;
																										bestfeature = i;
                          *split = (valuebefore + value) / 2;
																								 }
                       }
                      valuebefore = value;
                     }
                    k++;
                   }
                  safe_free(sorted);
                  break;
    }
 return bestfeature;
}

/**
 * Finds best univariate split (best feature and threshold pair) at a regression node where one compares different splits based on errors found using constant regression fits.
 * @param[in] instances Instance list at the regression node
 * @param[out] split Best split threshold
 * @param[out] comparison Best split character '<' for smaller '<' for larger
 * @return Best feature. If there is no possibility for a split returns -1 (There are k instances in the node where all feature values of all those instances are equal).
 */
int best_regression_feature(Instanceptr instances, double* split, char* comparison)
{
 /*!Last Changed 09.12.2004*/
 int i, k, bestfeature = -1, size, leftcount, rightcount;
 Instanceptr sorted;
 double value, valuebefore, regvalue, mse, bestmse = +INT_MAX;
	double leftmean, leftmeanbefore, lefterror;
	double rightmean, rightmeanbefore, righterror;
 for (i = 0; i < current_dataset->featurecount;i++)
   switch (current_dataset->features[i].type)
    {
     case REAL   :sorted = sort_instances(instances,i,&size);
                  k = 0;
                  valuebefore = -INT_MAX;
																		leftcount = 0;
																		rightcount = size;
																		leftmean = 0;
																		rightmean = find_mean_of_regression_value(instances);
																		lefterror = 0;
																		righterror = find_variance_of_regression_value(instances, rightmean);
                  while (k < size)
                   {
                    value = sorted[k].values[i].floatvalue;
                    if (value != valuebefore)
                     {
                      if (valuebefore != -INT_MAX)
                       {
																							 mse = lefterror + righterror;
																								if (mse < bestmse)
																								 {
																									 if ((lefterror / leftcount) < (righterror / rightcount))
																											 *comparison = '<';
																										else
																											 *comparison = '>';
																									 bestmse = mse;
																										bestfeature = i;
                          *split = (valuebefore + value) / 2;
																								 }
                       }
                      valuebefore = value;
                     }
                    regvalue = give_regressionvalue(&(sorted[k]));
																				leftmeanbefore = leftmean;
																				rightmeanbefore = rightmean;
																				leftmean = (leftmean * leftcount + regvalue) / (leftcount + 1);
																				rightmean = (rightmean * rightcount - regvalue) / (rightcount - 1);
																				lefterror = lefterror + (leftmeanbefore - leftmean) * (leftmeanbefore - leftmean) * leftcount + (regvalue - leftmean) * (regvalue - leftmean);
																				righterror = righterror - (rightmeanbefore - rightmean) * (rightmeanbefore - rightmean) * (rightcount - 1) - (regvalue - rightmeanbefore) * (regvalue - rightmeanbefore);
																				leftcount++;
																				rightcount--;
                    k++;
                   }
                  safe_free(sorted);
                  break;
    }
 return bestfeature;
}

/**
 * Calculates the linear regression fit to a given instance.
 * @param[in] w One column weight matrix containing the regression weights. w[0][0] is the bias value, w[i][0] is the weight of the i + 1'th feature.
 * @param[in] inst The instance
 * @return Linear regression fit to a given instance
 */
double linear_fit_value(matrix w, Instanceptr inst)
{
	/*!Last Changed 28.02.2005*/
	int i;
	double result = w.values[0][0];
	for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
			result += w.values[i + 1][0] * real_feature(inst, i);
	return result;
}

/**
 * Calculates the total squared error for a partial list of instances which are stored in an array.
 * @param[in] w One column weight matrix containing the regression weights. w[0][0] is the bias value, w[i][0] is the weight of the i + 1'th feature.
 * @param[in] list Array storing the instances. list[i] is the i'th instance.
 * @param[in] start Start index of the partial array
 * @param[in] end Ending index of the partial array
 * @param[in] leaftype Type of regression fit in the leaf node. CONSTANTLEAF if the regression fit is constant, LINEARLEAF if the regression fit is linear.
 * @return Total squared error of a group of instances stored in a part of an array.
 */
double variance_of_linear_regression_for_array_of_instances(matrix w, Instanceptr list, int start, int end, LEAF_TYPE leaftype)
{
	/*!Last Changed 01.03.2005*/
	int i, size = end - start + 1;
	double variance = 0;
	for (i = 0; i < size; i++)
		 variance += instance_regression_error(w, 0, &(list[start + i]), leaftype);
	return variance;
}

/**
 * Calculates the squared error of a single instance.
 * @param[in] w One column weight matrix containing the regression weights. w[0][0] is the bias value, w[i][0] is the weight of the i + 1'th feature.
 * @param[in] regressionvalue The constant regression fit.
 * @param[in] instance The instance
 * @param[in] leaftype Type of regression fit in the leaf node. CONSTANTLEAF if the regression fit is constant, LINEARLEAF if the regression fit is linear.
 * @return Squared error of a single instance
 */
double instance_regression_error(matrix w, double regressionvalue, Instanceptr instance, LEAF_TYPE leaftype)
{
	/*!Last Changed 02.03.2005*/
	double mean, regvalue;
	if (leaftype == CONSTANTLEAF)
		 mean = regressionvalue;
	else
  	mean = linear_fit_value(w, instance);
	regvalue = give_regressionvalue(instance);
	return (regvalue - mean) * (regvalue - mean);
}

/**
 * Calculates the total squared error for a list of instances which are stored in a link list, where the regression fit is linear.
 * @param[in] w One column weight matrix containing the regression weights. w[0][0] is the bias value, w[i][0] is the weight of the i + 1'th feature.
 * @param[in] list Link list of instances
 * @param[in] leaftype Type of regression fit in the leaf node. CONSTANTLEAF if the regression fit is constant, LINEARLEAF if the regression fit is linear.
 * @return Total squared error of a link list of instances.
 */
double variance_of_linear_regression(matrix w, Instanceptr list, LEAF_TYPE leaftype)
{
	/*!Last Changed 28.02.2005*/
	double variance = 0;
	Instanceptr tmp;
	tmp = list;
	while (tmp)
	 {
			variance += instance_regression_error(w, 0, tmp, leaftype);
		 tmp = tmp->next;
	 }
	return variance;
}

/**
 * Creates the X and r matrices that are used in multivariate regression from a partial array of instances. See the definition in the book "Introduction to Machine Learning" by Ethem Alpaydin p. 101.
 * @param[in] list Array storing the instances. list[i] is the i'th instance.
 * @param[in] start Start index of the partial array
 * @param[in] end Ending index of the partial array
 * @param[out] X Matrix X
 * @param[out] r One column matrix r
 */
void create_regression_matrices_for_array_of_instances(Instanceptr list, int start, int end, matrix* X, matrix* r)
{
	/*!Last Changed 01.03.2005*/
	int i, size = end - start + 1, j;
	*X = matrix_alloc(size, current_dataset->multifeaturecount);
	*r = matrix_alloc(size, 1);
	for (i = 0; i < size; i++)
	 {
		 (*X).values[i][0] = 1;
		 for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
			  (*X).values[i][j + 1] = real_feature(&(list[start + i]), j);
			(*r).values[i][0] = give_regressionvalue(&(list[start + i]));
	 }
}

/**
 * Creates the X and r matrices that are used in multivariate regression from a likn list of instances. See the definition in the book "Introduction to Machine Learning" by Ethem Alpaydin p. 101.
 * @param[in] list Link list of instances
 * @param[out] X Matrix X
 * @param[out] r One column matrix r
 */
void create_regression_matrices(Instanceptr list, matrix* X, matrix* r)
{
	/*!Last Changed 01.03.2005*/
	int i, size = data_size(list), j;
	Instanceptr tmp;
	*X = matrix_alloc(size, current_dataset->multifeaturecount);
	*r = matrix_alloc(size, 1);
 tmp = list;
	i = 0;
	while (tmp)
	 {
		 (*X).values[i][0] = 1;
		 for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
			  (*X).values[i][j + 1] = real_feature(tmp, j);
			(*r).values[i][0] = give_regressionvalue(tmp);
			i++;	 
		 tmp = tmp->next;
	 }
}

/**
 * Solves for the weight matrix w using the X and r matrices in multivariate regression.
 * @param[in] X Matrix X
 * @param[in] r One column matrix r
 * @return One column matrix w which stores the weights of the features and the bias value
 */
matrix multivariate_regression(matrix X, matrix r)
{
	/*!Last Changed 28.02.2005*/
	matrix XT, XTX, tmpmatrix, result;
	XT = matrix_transpose(X);
	XTX = matrix_multiply(XT, X);
	matrix_inverse(&XTX);
	tmpmatrix = matrix_multiply(XTX, XT);
	matrix_free(XTX);
	matrix_free(XT);
	result = matrix_multiply(tmpmatrix, r);
	matrix_free(tmpmatrix);
	return result;
}

/**
 * Constructor for one or more regression tree node(s). Allocates memory for the regression node(s), initializes the variables, sets the node(s) as a leaf node(s).
 * @param[in] parentnode Parent regression node of this(these) node(s).
 * @param[in] howmany Number of regression nodes to create
 * @return Empty allocated regression tree node(s). Generated as leaf node(s).
 */
Regressionnodeptr createnewregnode(Regressionnodeptr parentnode, int howmany, LEAF_TYPE leaftype)
{
	/*!Last Changed 25.08.2005 added leaftype*/
	/*!Last Changed 09.12.2004*/
	int j;
	Regressionnodeptr newnode;
	newnode = (Regressionnodeptr)safemalloc(howmany * sizeof(struct regressionnode), "createnewregnode", 3);
	newnode->featureno = LEAF_NODE;
	newnode->leaftype = leaftype;
	newnode->instances = NULL;
	newnode->cvinstances = NULL;
	for (j = 0; j < howmany; j++)
			newnode[j].parent = parentnode;
	return newnode;
}

/**
 * Gets the next node in the regression tree testing. For example, if the decision function in the current node is a univariate function (x_i, w_0), and if the instance has a value larger than w_0 in the feature i, next decision node will be the right child of the current node, otherwise it will be the left child node (if the instance has a value smaller than w_0 in the feature i).
 * @param[in] currentnode Current regression tree node which is tested
 * @param[in] tmp Current instance
 * @return Next node in the regression tree according to the decision function in the current node and the instance
 */
Regressionnodeptr next_regression_node(Regressionnodeptr currentnode, Instanceptr tmp)
{
	/*!Last Changed 24.03.2005*/
	/*!Last Changed 09.12.2004*/
	if (currentnode->featureno >= 0)
   if (tmp->values[currentnode->featureno].floatvalue <= currentnode->split)
     return currentnode->left;
   else
     return currentnode->right;
	else
			if (nearest_center_in_regression_tree(tmp, currentnode->featureno, currentnode->leftcenter, currentnode->rightcenter) == LEFTCHILD)
     return currentnode->left;
   else
     return currentnode->right;
}

/**
 * Tests a regression tree with the given test data.
 * @param[in] root Root node of the regression tree
 * @param[in] testingdata Header of the link list containing the test instances
 * @return Total squared error of a list of instances calculated using the given regression tree.
 */
double test_regtree(Regressionnodeptr root, Instanceptr testingdata)
{
	/*!Last Changed 28.02.2005*/
	/*!Last Changed 09.12.2004*/
 Instanceptr test = testingdata;
 Regressionnodeptr currentnode;
	double performance = 0;
 while (test)
  {
   currentnode = root;
   while (currentnode->featureno != LEAF_NODE)
     currentnode = next_regression_node(currentnode, test); 
			performance += instance_regression_error(currentnode->w, currentnode->regressionvalue, test, currentnode->leaftype);
   test = test->next;
  }
	return performance;
}

/**
 * Recursive destructor function of a regression tree. Frees memory allocated to the instances and validation instances (if the node is a leaf node), left and right mean vectors, etc.
 * @param[in] node Regression tree node
 */
void deallocate_regression_tree(Regressionnodeptr node)
{
	/*!Last Changed 23.03.2005 added deallocation of left and right centers*/
	/*!Last Changed 09.12.2004*/
 if (node->featureno == LEAF_NODE)
  {
   if (node->instances)
     deallocate(node->instances);
   if (node->cvinstances)
     deallocate(node->cvinstances);
   if (node->leaftype == LINEARLEAF)
     matrix_free(node->w);
  }
 else
  {
		 if (in_list(node->featureno, 2, LINEAR, QUADRATIC))
			 {
				 deallocate(node->leftcenter);
				 deallocate(node->rightcenter);
			 }
   else
     if (node->featureno == SOFTLIN)
       matrix_free(node->w);
   deallocate_regression_tree(node->left);
   safe_free(node->left);
   deallocate_regression_tree(node->right);
   safe_free(node->right);
  }
}

int make_regression_children(Regressionnodeptr node, LEAF_TYPE leaftype)
{
	/*!Last Changed 24.03.2005*/
	/*!Last Changed 09.12.2004*/
 Regressionnodeptr left, right;
 if (node->featureno == LEAF_NODE || node->instances == NULL)
	 {
		 make_leaf_regression_node(node, leaftype);
   return 0;
	 }
 left = createnewregnode(node, 1, leaftype);
 right = createnewregnode(node, 1, leaftype);
	if (node->featureno >= 0)
	 {
  	divide_instancelist_according_to_split(&(node->instances), &(left->instances), &(right->instances), node->featureno, node->split);
	  divide_instancelist_according_to_split(&(node->cvinstances), &(left->cvinstances), &(right->cvinstances), node->featureno, node->split);
	 }
	else
		 if (node->featureno == LINEAR)
			 {
				 divide_instancelist_according_centers(&(node->instances), &(left->instances), &(right->instances), node->leftcenter, node->rightcenter, NO);
				 divide_instancelist_according_centers(&(node->cvinstances), &(left->cvinstances), &(right->cvinstances), node->leftcenter, node->rightcenter, NO);
			 }
			else
			 {
				 divide_instancelist_according_centers(&(node->instances), &(left->instances), &(right->instances), node->leftcenter, node->rightcenter, YES);
				 divide_instancelist_according_centers(&(node->cvinstances), &(left->cvinstances), &(right->cvinstances), node->leftcenter, node->rightcenter, YES);
			 }
	node->left = left;
	node->right = right;
 return 1;
}

void accumulate_instances_regression_leaf(Instanceptr* data, Instanceptr* cvdata, Regressionnodeptr leaf)
{
 /*!Last Changed 02.03.2005*/
	Instanceptr tmp;
	if ((*data) == NULL)
		 *data = leaf->instances;
	else
	 {
		 tmp = *data;
			while (tmp->next != NULL)
				 tmp = tmp->next;
			tmp->next = leaf->instances;
	 }
	leaf->instances = NULL;
	if ((*cvdata) == NULL)
		 *cvdata = leaf->cvinstances;
	else
	 {
		 tmp = *cvdata;
			while (tmp->next != NULL)
				 tmp = tmp->next;
			tmp->next = leaf->cvinstances;
	 }
	leaf->cvinstances = NULL;
}

void accumulate_instances_regression_tree(Instanceptr* data, Instanceptr* cvdata, Regressionnodeptr root)
{
 /*!Last Changed 02.03.2005*/
	if (root->featureno == LEAF_NODE)
		 accumulate_instances_regression_leaf(data, cvdata, root);
	else
	 {
   accumulate_instances_regression_tree(data, cvdata, root->left);
			accumulate_instances_regression_tree(data, cvdata, root->right);
	 }
}

void prune_regression_tree(Regressionnodeptr root, Regtree_parameterptr param)
{
	/*!Last Changed 02.03.2005*/
	double generalizationerrorbeforeprune = 0.0, generalizationerrorafterprune = 0.0, errorbeforeprune;
	int dafterprune, dbeforeprune;
 if (root->featureno == LEAF_NODE)
   return;
	if (param->leaftype == CONSTANTLEAF)
		 dafterprune = 1;
	else
		 dafterprune = current_dataset->multifeaturecount;
 dbeforeprune = complexity_of_regression_tree(root);
	errorbeforeprune = mse_error_of_regression_tree(root);
	switch (param->modelselectionmethod)
	 {
	  case MODEL_SELECTION_CV :generalizationerrorbeforeprune = root->left->errorforprune + root->right->errorforprune;
				                        generalizationerrorafterprune = root->errorforprune;
										                  break;
	  case MODEL_SELECTION_AIC:generalizationerrorbeforeprune = aic_squared_error(errorbeforeprune / root->covered, dbeforeprune, root->covered);
				                        generalizationerrorafterprune = aic_squared_error(root->mseerror / root->covered, dafterprune, root->covered);
				                        break;
			case MODEL_SELECTION_BIC:generalizationerrorbeforeprune = bic_squared_error(errorbeforeprune / root->covered, dbeforeprune, root->covered);
				                        generalizationerrorafterprune = bic_squared_error(root->mseerror / root->covered, dafterprune, root->covered);
				                        break;
    default                :if (param->prunetype == VALIDATIONPRUNING)
                             {
                              generalizationerrorbeforeprune = root->left->errorforprune + root->right->errorforprune;
                              generalizationerrorafterprune = root->errorforprune;
                             }
                            else
                              printf(m1230, param->modelselectionmethod);
                            break;
	 }
	if (generalizationerrorafterprune < generalizationerrorbeforeprune)
	 {
			accumulate_instances_regression_tree(&(root->instances), &(root->cvinstances), root);
   deallocate_regression_tree(root->left);
			safe_free(root->left);
   deallocate_regression_tree(root->right);
			safe_free(root->right);
			make_leaf_regression_node(root, param->leaftype);
	 }
	else
	 {
		 prune_regression_tree(root->left, param);
		 prune_regression_tree(root->right, param);
	 }
}

void setup_regressionnode_properties(Regressionnodeptr node, LEAF_TYPE leaftype)
{
	matrix X, r, w;
	node->covered = data_size(node->instances);
	if (leaftype == CONSTANTLEAF)
	 {
   node->regressionvalue = find_mean_of_regression_value(node->instances);
   node->mseerror = find_variance_of_regression_value(node->instances, node->regressionvalue);
	 }
	else
	 {
   create_regression_matrices(node->instances, &X, &r);
   w = multivariate_regression(X, r);
   matrix_free(X);
   matrix_free(r);
   node->w = w;
   node->mseerror = variance_of_linear_regression(w, node->instances, leaftype);
	 } 
}

int can_be_divided_regression(Regressionnodeptr node, Regtree_parameterptr param)
{
	/*!Last Changed 01.03.2005*/
 setup_regressionnode_properties(node, param->leaftype);
 if (param->prunetype == VALIDATIONPRUNING || (param->prunetype == MODELSELECTIONPRUNING && param->modelselectionmethod == MODEL_SELECTION_CV))
  {
  	if (param->leaftype == CONSTANTLEAF)
     node->errorforprune = find_variance_of_regression_value(node->cvinstances, node->regressionvalue);
   else
     node->errorforprune = variance_of_linear_regression(node->w, node->cvinstances, param->leaftype);
  }
	if (node->mseerror / node->covered <= param->sigma)
	 {
		 make_leaf_regression_node(node, param->leaftype);
		 return 0;
	 }
	if (param->prunetype == PREPRUNING)
	 {
			if ((node->covered + 0.0) / current_dataset->traincount < param->pruningthreshold)
			 {
	  	 make_leaf_regression_node(node, param->leaftype);
     return 0;
			 }
	 }
	return 1;
}

double constant_or_linear_model_mse(Instanceptr train, Instanceptr test, LEAF_TYPE leaftype)
{
	/*!Last Changed 26.03.2005*/
	double regvalue, error;
	matrix X, r, w;
	if (leaftype == CONSTANTLEAF)
	 {
  	regvalue = find_mean_of_regression_value(train);
			error = find_variance_of_regression_value(test, regvalue);
	 }
	else
	 {
   create_regression_matrices(train, &X, &r);
		 w = multivariate_regression(X, r);
			matrix_free(X);
			matrix_free(r);
			error = variance_of_linear_regression(w, test, leaftype);
			matrix_free(w);
	}
	return error;
}

double regression_tree_model_selection_error(Instanceptr* train, Instanceptr *test, NODE_TYPE nodetype, LEAF_TYPE leaftype)
{
	/*!Last Changed 24.03.2005*/
	char dummy;
	int fno;
	double split, error;
	Instanceptr left, right, leftinstances = NULL, rightinstances = NULL, testleft = NULL, testright = NULL;
	switch (nodetype)
	 {
	  case UNIVARIATE:if (leaftype == CONSTANTLEAF)
                     fno = best_regression_feature(*train, &split, &dummy);
	                  else
		                   fno = best_regression_feature_linear_leaves(*train, &split, &dummy);
  	                divide_instancelist_according_to_split(train, &leftinstances, &rightinstances, fno, split);
  	                divide_instancelist_according_to_split(test, &testleft, &testright, fno, split);
																			break;
			case LINEAR    :find_linear_regression_split(*train, &left, &right);
				               divide_instancelist_according_centers(train, &leftinstances, &rightinstances, left, right, NO);
				               divide_instancelist_according_centers(test, &testleft, &testright, left, right, NO);
				               break;
			case QUADRATIC :find_quadratic_regression_split(*train, &left, &right);
				               divide_instancelist_according_centers(train, &leftinstances, &rightinstances, left, right, YES);
				               divide_instancelist_according_centers(test, &testleft, &testright, left, right, YES);
				               break;
   default        :printf(m1231, nodetype);
                   break;
	 }
	error = constant_or_linear_model_mse(leftinstances, testleft, leaftype) + constant_or_linear_model_mse(rightinstances, testright, leaftype);
 *train = restore_instancelist(*train, leftinstances);
	merge_instancelist(train, rightinstances);	
 *test = restore_instancelist(*test, testleft);
	merge_instancelist(test, testright);	
	return error;
}

int best_hybrid_regression_tree_split_crossvalidation(Regressionnodeptr node, Regtree_parameterptr param)
{
	/*!Last Changed 24.03.2005*/
 int hybridspace = get_parameter_i("hybridspace");
 FILE* errorfiles[3];
	int i, k, trainsize, testsize, bestmodel = 0, *models, dummy, dummy2, testtype;
	Instanceptr train = NULL, test = NULL;
	char **files, st[READLINELENGTH] = "-", dummy3, pst[READLINELENGTH], buffer[READLINELENGTH];
	double error;
 files = safemalloc(hybridspace * sizeof(char *), "best_hybrid_regression_tree_split_crossvalidation", 5);
 strcpy(buffer, get_parameter_s("tempdirectory"));
 strcat(buffer, "/univariate.txt");
 files[0] = strcopy(files[0], buffer);
 strcpy(buffer, get_parameter_s("tempdirectory"));
 strcat(buffer, "/linear.txt");
 files[1] = strcopy(files[1], buffer);
 if (hybridspace == 3)
  {
   strcpy(buffer, get_parameter_s("tempdirectory"));
   strcat(buffer, "/quadratic.txt");
   files[2] = strcopy(files[2], buffer);
  }
 for (i = 0; i < hybridspace; i++)
  {
   errorfiles[i] = fopen(files[i], "w");
   if (!errorfiles[i])
     return 0;
  }
	for (i = 0; i < 5; i++)
	 {
		 divide_instancelist(&(node->instances), &train, &test, 2);
			trainsize = data_size(train);
			testsize = data_size(test);
			if (trainsize <= 1 || testsize <= 1)
			 {
  			node->instances = restore_instancelist(node->instances, train);
	  		merge_instancelist(&(node->instances), test);	
					bestmodel = 1;
				 break;
			 }
   for (k = 0; k < hybridspace; k++)
    {
     switch (k)
      {
       case MODEL_UNI:error = regression_tree_model_selection_error(&train, &test, UNIVARIATE, param->leaftype);
                      break;
       case MODEL_LIN:error = regression_tree_model_selection_error(&train, &test, LINEAR, param->leaftype);
                      break;
       case MODEL_QUA:error = regression_tree_model_selection_error(&train, &test, QUADRATIC, param->leaftype);
                      break;
      }
     set_precision(pst, NULL, "\n");
     fprintf(errorfiles[k], pst, error / (testsize + 0.0));
    }
   for (k = 0; k < hybridspace; k++)
    {
     switch (k)
      {
       case MODEL_UNI:error = regression_tree_model_selection_error(&test, &train, UNIVARIATE, param->leaftype);
                      break;
       case MODEL_LIN:error = regression_tree_model_selection_error(&test, &train, LINEAR, param->leaftype);
                      break;
       case MODEL_QUA:error = regression_tree_model_selection_error(&test, &train, QUADRATIC, param->leaftype);
                      break;
      }
     set_precision(pst, NULL, "\n");
     fprintf(errorfiles[k], pst, error / (trainsize + 0.0));
    }
			node->instances = restore_instancelist(node->instances, train);
			merge_instancelist(&(node->instances), test);	
			train = NULL;
			test = NULL;
	 }
 for (i = 0; i < hybridspace; i++)
   fclose(errorfiles[i]);
 if (bestmodel)
	 {
		 free_2d((void**)files, hybridspace);
   if (param->leaftype == CONSTANTLEAF)
     node->featureno = best_regression_feature(node->instances, &(node->split), &dummy3);
	  else
		   node->featureno = best_regression_feature_linear_leaves(node->instances, &(node->split), &dummy3);
			if (node->featureno == LEAF_NODE)
				 return 0;
			return 1;
	 }
 testtype = get_parameter_l("testtype");
 if (in_list(testtype, 9, FTEST, PAIREDT5X2, COMBINEDT5X2, PERMUTATIONTEST, PAIREDPERMUTATIONTEST, SIGNTEST, RANKSUMTEST, SIGNEDRANKTEST, NOTEST))
   models = multiple_comparison(files, hybridspace, testtype, st, &dummy, &dummy2, param->correctiontype);
 else
   models = multiple_comparison(files, hybridspace, FTEST, st, &dummy, &dummy2, param->correctiontype);
	switch (models[0])
	 {
	  case 1:if (param->leaftype == CONSTANTLEAF)
            return best_regression_feature(node->instances, &(node->split), &dummy3);
	         else
		          return best_regression_feature_linear_leaves(node->instances, &(node->split), &dummy3);
										break;
			case 2:find_linear_regression_split(node->instances, &(node->leftcenter), &(node->rightcenter));
  				    node->featureno = LINEAR;
				      break;
			case 3:find_quadratic_regression_split(node->instances, &(node->leftcenter), &(node->rightcenter));
  				    node->featureno = QUADRATIC;
				      break;
	 }
	safe_free(models);
	free_2d((void**)files, hybridspace);
	return 1;
}

void find_uni_and_multi_regression_splits(Regressionnodeptr node, Instanceptr* leftlincenter, Instanceptr* rightlincenter, Instanceptr* leftquacenter, Instanceptr* rightquacenter)
{
	/*!Last Changed 24.03.2005*/
	char dummy3;
 if (node->leaftype == CONSTANTLEAF)
   node->featureno = best_regression_feature(node->instances, &(node->split), &dummy3);
	else
		 node->featureno = best_regression_feature_linear_leaves(node->instances, &(node->split), &dummy3);
 find_linear_regression_split(node->instances, leftlincenter, rightlincenter);
 find_quadratic_regression_split(node->instances, leftquacenter, rightquacenter);
}

void set_best_model_in_regression_tree(Regressionnodeptr node, int bestmodel, Instanceptr leftl, Instanceptr rightl, Instanceptr leftq, Instanceptr rightq)
{
	/*!Last Changed 24.03.2005*/
	switch (bestmodel)
	 {
	  case MODEL_UNI:deallocate(leftl);
										        deallocate(rightl);
				              deallocate(leftq);
				              deallocate(rightq);
        										break;
	  case MODEL_LIN:node->featureno = LINEAR;
				              node->leftcenter = leftl;
				              node->rightcenter = rightl;
				              deallocate(leftq);
				              deallocate(rightq);
										        break;				      
			case MODEL_QUA:node->featureno = QUADRATIC;
				              node->leftcenter = leftq;
				              node->rightcenter = rightq;
				              deallocate(leftl);
				              deallocate(rightl);
										        break;				      
	 }
}

int best_hybrid_regression_tree_split_aic_or_bic(Regressionnodeptr node, Regtree_parameterptr param)
{
	/*!Last Changed 27.03.2005*/
	int i, size = data_size(node->instances), d, bestmodel;
	double error, results[3];
 Instanceptr leftl, rightl, leftq, rightq, leftinstances, rightinstances;
 find_uni_and_multi_regression_splits(node, &leftl, &rightl, &leftq, &rightq);
	for (i = 0; i < 3; i++)
	 {
   leftinstances = rightinstances = NULL;
		 switch (i)
    {
		   case MODEL_UNI:divide_instancelist_according_to_split(&(node->instances), &leftinstances, &rightinstances, node->featureno, node->split);
						              d = 2;
						              break;
					case MODEL_LIN:divide_instancelist_according_centers(&(node->instances), &leftinstances, &rightinstances, leftl, rightl, NO);
						              d = current_dataset->multifeaturecount;
						              break;
					case MODEL_QUA:divide_instancelist_according_centers(&(node->instances), &leftinstances, &rightinstances, leftq, rightq, YES);
						              d = multifeaturecount_2d(current_dataset) + 1;
						              break;
    }
			error = (constant_or_linear_model_mse(leftinstances, leftinstances, param->leaftype) + constant_or_linear_model_mse(rightinstances, rightinstances, param->leaftype)) / size;
			if (param->modelselectionmethod == 1)
     results[i] = aic_squared_error(error, d, size);
			else
				 results[i] = bic_squared_error(error, d, size);
  	node->instances = restore_instancelist(node->instances, leftinstances);
	  merge_instancelist(&(node->instances), rightinstances);	
	 }
	bestmodel = find_min_in_list_double(results, 3);
	set_best_model_in_regression_tree(node, bestmodel, leftl, rightl, leftq, rightq);
	return 1;
}

int find_linear_regression_split(Instanceptr data, Instanceptr* left, Instanceptr* right)
{
	/*!Last Changed 24.03.2005*/
	int size = data_size(data);
	Instanceptr list;
	if (size < 2)
		 return 0;
	list = k_means_clustering(data, 2);
	*left = list;
	*right = list->next;
	list->next = NULL;
	return 1;
}

int find_quadratic_regression_split(Instanceptr data, Instanceptr* left, Instanceptr* right)
{
	/*!Last Changed 23.03.2005*/
	Instanceptr newlist, list;
	int size = data_size(data), oldfcount;
	if (size < 2)
		 return 0;
 newlist = generate_2d_copy_of_instance_list(data);
 oldfcount = current_dataset->multifeaturecount;
	current_dataset->multifeaturecount = multifeaturecount_2d(current_dataset) + 1;
	list = k_means_clustering(newlist, 2);
	*left = list;
	*right = list->next;
	list->next = NULL;
	deallocate(newlist);
 current_dataset->multifeaturecount = oldfcount;
	return 1;
}

int best_single_regression_tree_split(Regressionnodeptr node, int nodetype)
{
	/*!Last Changed 23.03.2005*/
	char dummy;
	int found;
	switch (nodetype)
	 {
	  case UNIVARIATE:if (node->leaftype == CONSTANTLEAF)
                     return best_regression_feature(node->instances, &(node->split), &dummy);
	                  else
		                   return best_regression_feature_linear_leaves(node->instances, &(node->split), &dummy);
				               break;
		 case LINEAR    :found = find_linear_regression_split(node->instances, &(node->leftcenter), &(node->rightcenter));
				               break;
		 case QUADRATIC :found = find_quadratic_regression_split(node->instances, &(node->leftcenter), &(node->rightcenter));
				               break;
   default        :printf(m1231, nodetype);
                   found = 0;
                   break;
	 }
	if (found)
		 return nodetype;
	else
		 return LEAF_NODE;
}

int best_hybrid_regression_tree_split(Regressionnodeptr node, Regtree_parameterptr param)
{
	/*!Last Changed 27.03.2005*/
	switch (param->modelselectionmethod)
	 {
	  case MODEL_SELECTION_CV :return best_hybrid_regression_tree_split_crossvalidation(node, param);
				                        break;
			case MODEL_SELECTION_AIC:
			case MODEL_SELECTION_BIC:return best_hybrid_regression_tree_split_aic_or_bic(node, param);
				                        break;
			case MODEL_SELECTION_UNI:return best_single_regression_tree_split(node, UNIVARIATE);
				                        break;
			case MODEL_SELECTION_LIN:return best_single_regression_tree_split(node, LINEAR);
				                        break;
			case MODEL_SELECTION_QUA:return best_single_regression_tree_split(node, QUADRATIC);
				                        break;
   default                 :printf(m1230, param->modelselectionmethod);
                            break;
	 }
	return LEAF_NODE;
}

int create_regtree(Regressionnodeptr node, Regtree_parameterptr param)
{
	/*!Last Changed 09.12.2004 rewritten*/
 /*!Last Changed 11.04.2004*/
	if (!(can_be_divided_regression(node, param)))
   return 1;
	node->featureno = best_hybrid_regression_tree_split(node, param);
 if (!(make_regression_children(node, param->leaftype)))
   return 1;
 create_regtree(node->left, param);
 create_regtree(node->right, param);
 return 1;
}

void inorder_traversal_regression_tree(Regressionnodeptr node, int* index)
{
 if (node)
  { 
   inorder_traversal_regression_tree(node->left, index);
   node->x = *index;
   (*index)++;
   inorder_traversal_regression_tree(node->right, index);   
  }
}

void level_of_regression_tree(Regressionnodeptr node)
{
 if (node)
  {
   if (node->parent)
     node->y = node->parent->y + 1;
   else
     node->y = 0;
   level_of_regression_tree(node->left);
   level_of_regression_tree(node->right);   
  }
}

void soft_regtree_dimension_reduction(Regressionnodeptr node, char st[READLINELENGTH], int level)
{
 int i, count;
	if (node->featureno != LEAF_NODE)
	 {
   count = 0;
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (node->w.values[i][0] != 0)
       count++;
   sprintf(st,"%s%d-%d;", st, level, count);
   soft_regtree_dimension_reduction(node->left, st, level + 1);
   soft_regtree_dimension_reduction(node->right, st, level + 1);
	 }
}

/**
 * Recursive function to calculate depth of the regression tree. Depth of the leaf node is 1 and depth of a regression node is one larger than the maximum depth of its subtrees.
 * @param[in] node Root node of the subtree
 * @return Depth of the root node of the subtree
 */
int depth_of_regression_tree(Regressionnodeptr node)
{
 /*!Last Changed 12.07.2006*/
 int leftdepth, rightdepth;
 if (node->featureno == LEAF_NODE)
   return 1;
 else
  {
   leftdepth = depth_of_regression_tree(node->left);
   rightdepth = depth_of_regression_tree(node->right);
   if (leftdepth >= rightdepth)
     return leftdepth + 1;
   else
     return rightdepth + 1;
  }
}

int level_of_regression_node(Regressionnodeptr node)
{
 if (node == NULL)
   return 0;
 else
   return 1 + level_of_regression_node(node->parent);
}

double estimate_sigma_square_with_heuristic(Instanceptr data)
{
	/*!Last Changed 02.03.2005*/
	double* values, difference = +INT_MAX, d;
	Instanceptr tmp;
	int size = data_size(data), i;
	values = safemalloc(size * sizeof(double), "estimate_sigma_square_with_heuristic", 4);
	tmp = data;
	i = 0;
	while (tmp)
	 {
		 values[i] = give_regressionvalue(tmp);
			tmp = tmp->next;
		 i++;
	 }
	qsort(values, size, sizeof(double), sort_function_double_ascending);
	for (i = 0; i < size - 1; i++)
	 {
		 d = values[i + 1] - values[i];
		 if ((!dequal(d, 0)) && d < difference)
				 difference = d;
	 }
	safe_free(values);
	return difference * difference;
}

double estimate_sigma_square_with_knn(Instanceptr data, int nearcount)
{
	/*!Last Changed 01.03.2005*/
	Instanceptr tmp, *nearestneighbors;
	int N = 0, i;
	double result = 0, yvalue, realvalue, h;
	tmp = data;
	while (tmp)
	 {
		 nearestneighbors = find_nearest_neighbors(data, tmp, nearcount);
			yvalue = 0;
			for (i = 0; i < nearcount; i++)
  			yvalue += give_regressionvalue(nearestneighbors[i]);
			yvalue /= nearcount;
			safe_free(nearestneighbors);
   realvalue = give_regressionvalue(tmp);
			result += (double) ((realvalue - yvalue) * (realvalue - yvalue));
			N++;
			tmp = tmp->next;
	 }
	h = pow(N, 0.8) / nearcount;
 return result / (N - h);
}
