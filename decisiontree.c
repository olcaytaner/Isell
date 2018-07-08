#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "classification.h"
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "lang.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "messages.h"
#include "mlp.h"
#include "model.h"
#include "options.h"
#include "parameter.h"
#include "perceptron.h"
#include "rule.h"
#include "svm.h"
#include "svmkernel.h"
#include "vc.h"

extern Datasetptr current_dataset;

/**
 * Convert a decision tree node to a leaf node
 * @param[in] node Decision tree node
 */
void make_leaf_node(Decisionnodeptr node)
{
 /*!Last Changed 22.09.2004*/
 node->featureno = LEAF_NODE;
 node->left = NULL;
 node->right = NULL;
 node->string = NULL;
}

/**
 * Recursive function which calculates the number of false positives. If the node is a decision node, it calls itself with its leaves, if the node is a leaf, returns the false positive count
 * @param[in] node Decision tree node
 * @return Number of false positives in the subtree rooted this node. The number of instances in the leaf node except the instances from maximum occuring class
 */
int decisiontree_falsepositive_count(Decisionnodeptr node)
{
 /*!Last Changed 20.04.2008*/
 /*!Last Changed 30.09.2007*/
 int result = 0, i, fno;
 if (node->featureno == LEAF_NODE)
   return node->falsepositives;
 else
  {
   fno = node->featureno;
   if (fno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[fno].type == STRING)
     for (i = 0; i < current_dataset->features[fno].valuecount; i++)
       result += decisiontree_falsepositive_count(&(node->string[i]));
   else
    {
     result += decisiontree_falsepositive_count(node->left);
     result += decisiontree_falsepositive_count(node->right);
    }
   return result;
  }
}

/**
 * Recursive function that calculates the overall complexity. The complexities of the nodes are: \n
 * 1. Univariate node: 2 (one for the feature, one for the split point) \n
 * 2. Linear node: d + 1, where d is the number of dimensions after dimension reduction using PCA \n
 * 3. Quadratic node: m + 1, where m is the number of dimensions after dimension reduction over 2-d dataset using PCA
 * @param[in] node Decision tree node
 * @return Complexity of the subtree rooted this node
 */
int decisiontree_complexity_count(Decisionnodeptr node)
{
 /*!Last Changed 24.11.2004*/
 if (node->featureno == LEAF_NODE)
   if (node->leaftype == LINEARLEAF)
     return current_dataset->multifeaturecount - 1;
   else
     return 0;
 else
   if (node->featureno >= 0)
     return 2 + decisiontree_complexity_count(node->left) + decisiontree_complexity_count(node->right);
   else
     if (node->featureno == LINEAR)
       return node->lineard + 1 + decisiontree_complexity_count(node->left) + decisiontree_complexity_count(node->right);
     else
       if (node->featureno == QUADRATIC)
         return node->quadraticd + 1 + decisiontree_complexity_count(node->left) + decisiontree_complexity_count(node->right);
       else
         return current_dataset->multifeaturecount + decisiontree_complexity_count(node->left) + decisiontree_complexity_count(node->right);
}

/**
 * Recursive function that calculates the number of leaf nodes
 * @param[in] node Decision tree node
 * @return Number of decision leaves in the subtree rooted this node
 */
int decisiontree_leaf_count(Decisionnodeptr node)
{
 /*!Last Changed 24.11.2004*/
 int fno, result = 0, i;
 if (node->featureno == LEAF_NODE)
   return 1;
 else
  {
   fno = node->featureno;
   if (fno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[fno].type == STRING)
     for (i = 0; i < current_dataset->features[fno].valuecount; i++)
       result += decisiontree_leaf_count(&(node->string[i]));
   else
    {
     result += decisiontree_leaf_count(node->left);
     result += decisiontree_leaf_count(node->right);
    }
   return result;
  }
}

/**
 * Recursive function that returns the number of decision nodes (not including decision leaves)
 * @param[in] node Decision tree node
 * @return Number of decision nodes in the subtree rooted this node
 */
int decisiontree_node_count(Decisionnodeptr node)
{
 /*!Last Changed 24.11.2004*/
 int fno, result = 1, i;
 if (node->featureno == LEAF_NODE)
   return 0;
 else
  {
   fno = node->featureno;
   if (fno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[fno].type == STRING)
     for (i = 0; i < current_dataset->features[fno].valuecount; i++)
      {
       result += decisiontree_node_count(&(node->string[i]));
      }
   else
    {
     result += decisiontree_node_count(node->left);
     result += decisiontree_node_count(node->right);
    }
   return result;
  }
}


/**
 * Recursive function that returns the number of conditions as if the decision tree is converted to a ruleset.
 * @param[in] node Decision tree node
 * @return Number of conditions in the subtree rooted this node
 */
int decisiontree_condition_count(Decisionnodeptr node)
{
 /*!Last Changed 23.09.2004 added hybridcondition*/
 /*!Last Changed 10.11.2003*/
 int result = 0, fno, i;
 if (node->featureno == LEAF_NODE)
   return node->conditioncount;
 else
  {
   fno = node->featureno;
   if (fno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[fno].type == STRING)
     for (i = 0; i < current_dataset->features[fno].valuecount; i++)
       result += decisiontree_condition_count(&(node->string[i]));
   else
    {
     result += decisiontree_condition_count(node->left);
     result += decisiontree_condition_count(node->right);
    }
   return result;
  }
}

/**
 * Recursive function that returns the number of rules as if the decision tree is converted to a ruleset. Each leave correspond to a rule. Therefore the function is equal to decisiontree_leaf_count.
 * @param[in] node Decision tree node
 * @return Number of rules in the subtree rooted this node
 */
int decisiontree_rule_count(Decisionnodeptr node)
{
 /*!Last Changed 23.09.2004 added hybridcondition*/
 /*!Last Changed 10.11.2003*/
 int result = 0, fno, i;
 if (node->featureno == LEAF_NODE)
   return 1;
 else
  {
   fno = node->featureno;
   if (fno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[fno].type == STRING)
     for (i = 0; i < current_dataset->features[fno].valuecount; i++)
       result += decisiontree_rule_count(&(node->string[i]));
   else
   {
    result += decisiontree_rule_count(node->left);
    result += decisiontree_rule_count(node->right);
   }
   return result;
  }
}

/**
 * Add the instances in the leaf node to a list thereby accumulating instances in a tree to a link list
 * @param[out] data Header of the link list containing instance accumulated until here
 * @param[in] leaf Decision tree leaf
 */
void accumulate_instances_leaf(Instanceptr* data, Decisionnodeptr leaf)
{
 /*!Last Changed 27.10.2003*/
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
}

/**
 * Recursive function that accumulates all instances in a decision tree and put them into a single link list. Since the instances are stored in the leaf nodes in the decision tree, this function will call itself until it comes across a leaf node where it calls accumulate_instances_leaf
 * @param[out] data Header of the output link list
 * @param[in] root Root node of the decision tree
 */
void accumulate_instances_tree(Instanceptr* data, Decisionnodeptr root)
{
 /*!Last Changed 28.05.2005 updated according to SVMTREE*/
 /*!Last Changed 24.11.2004 added hybridcondition*/
 /*!Last Changed 27.09.2004 updated to include multivariate nodes*/
 /*!Last Changed 27.10.2003*/
 int i, fno;
 if (root->featureno == LEAF_NODE)
   accumulate_instances_leaf(data, root);
 else
  {
   fno = root->featureno;
   if (fno < 0 || root->conditiontype == HYBRID_CONDITION)
    {
     accumulate_instances_tree(data, root->left);
     accumulate_instances_tree(data, root->right);
    }
   else
     switch (current_dataset->features[fno].type){
       case INTEGER:
       case REAL   :accumulate_instances_tree(data, root->left);
                    accumulate_instances_tree(data, root->right);
                    break;
       case STRING :for (i=0;i<current_dataset->features[fno].valuecount;i++)
                      accumulate_instances_tree(data, &(root->string[i]));
                    break; 
     }
  }
}

BOOLEAN setup_node_properties(Decisionnodeptr node)
{
 int maxcount, classno;
 node->counts = find_class_counts(node->instances);
 classno = all_in_one_class(node->instances, &maxcount);
 node->classno = classno / 10;
 node->covered = data_size(node->instances);
 node->falsepositives = node->covered - maxcount; 
 if (!(classno % 10))
   return BOOLEAN_FALSE;
 else
   return BOOLEAN_TRUE;
}

/**
 * Can the decision tree node be expanded? This function answers this question according to the parameters of the tree algorithm. If prepruning is applied, the ratio of the number of instances in this node to the overall number of instances must be larger than a threshold so that one can expand this node. If other prunings applied or no pruning applied, the node mus be pure (does contain instances from only one class)
 * @param[in] node Decision tree node
 * @param[in] p Parameters (type of pruning applied, pruning threshold used in prepruning)
 * @return YES if the decision node can be expanded, NO otherwise.
 */
int can_be_divided(Decisionnodeptr node, C45_parameterptr p)
{
 /*!Last Changed 10.08.2005 parametre ile prunetype ile threshold kontrol ediliyor*/
 /*!Last Changed 12.04.2003 prepruning eklendi*/
 /*  node->classno = classno/10; satiri eklendi Last Changed 12.04.2003*/
 int size;
 if (!setup_node_properties(node))
  {
   make_leaf_node(node);
   return NO;
  }
 if (p && p->prunetype == PREPRUNING)
  {
   size = data_size(node->instances);
   if ((size + 0.0) / current_dataset->traincount < p->pruningthreshold)
    {
     make_leaf_node(node);
     return NO;
    }
  }
 return YES;
}

/**
 * Add one condition to the rule array of condition list.
 * @param[in] node Decision tree node
 * @param[in] condition Condition to be added
 */
void addcondition(Decisionnodeptr node, Decisioncondition condition)
{
/*!Last Changed 08.04.2003*/
 node->rule = alloc(node->rule, (node->conditioncount + 1) * sizeof(Decisioncondition), node->conditioncount + 1);
 node->rule[node->conditioncount] = condition;
 node->conditioncount++;
}

/**
 * Constructor for one or more decision tree node(s). Allocates memory for the decision node(s), initializes the variables, sets the node(s) as a leaf node(s) and if the parent node exists (the node is not a root node) copies the conditions in the parent node to this node.
 * @param[in] parentnode Parent decision node of this(these) node(s).
 * @param[in] howmany Number of decision nodes to create
 * @return Empty allocated decision tree node(s). Generated as leaf node(s).
 */
Decisionnodeptr createnewnode(Decisionnodeptr parentnode, int howmany)
{
 /*!Last Changed 14.07.2006*/
 /*!Last Changed 23.11.2004 added possibleconditioncount*/
 /*!Last Changed 26.09.2004 added parent*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 08.04.2003*/
 int i,j;
 Decisionnodeptr newnode;
 newnode = (Decisionnodeptr)safemalloc(howmany * sizeof(struct decisionnode), "createnewnode", 3);
 if (parentnode)
  {
   for (j = 0; j < howmany; j++)
    {
     newnode[j].possibleconditioncount = parentnode->possibleconditioncount;
     newnode[j].parent = parentnode;
     newnode[j].rule = safemalloc(parentnode->conditioncount*sizeof(Decisioncondition), "createnewnode", 10);
     for (i = 0; i < parentnode->conditioncount; i++)
       newnode[j].rule[i] = copy_of_condition(parentnode->rule[i]); 
     newnode[j].conditioncount = parentnode->conditioncount;
    }
  }
 else
   for (j = 0; j < howmany; j++)
    {
     newnode[j].parent = parentnode;
     newnode[j].rule = NULL;
     newnode[j].conditioncount = 0;
    } 
 for (j = 0; j < howmany; j++)
  {
   newnode[j].featureno = LEAF_NODE;
   newnode[j].counts = NULL;
   newnode[j].instances = NULL;
   newnode[j].expanded = BOOLEAN_FALSE;
   newnode[j].leaftype = CONSTANTLEAF;
   newnode[j].outputs = NULL;
   newnode[j].w0s = NULL;
  }
 return newnode;
}

/**
 * Checks if the instance has the exact value of the selected threshold in the selected feature for the univariate split, if the linear combination of the features has the selected bias value for the multivariate split. If the equality check is done for noninteger values (for real features in the univariate split and for all multivariate splits), two numbers are said to be equal if they have the same value in a DELTA (0.0001) interval.
 * @param[in] currentnode Decision tree node
 * @param[in] tmp Instance
 * @return YES for equality, NO otherwise.
 */
int instance_value_equal_to_split(Decisionnodeptr currentnode, Instanceptr tmp)
{
 /*!Last Changed 27.09.2004*/
 double value;
 if (currentnode->featureno >= 0)
   if (currentnode->conditiontype != HYBRID_CONDITION)
    {
     switch (current_dataset->features[currentnode->featureno].type)
      {
       case INTEGER:if (tmp->values[currentnode->featureno].intvalue == currentnode->split)
                      return YES;
                    else
                      return NO;
                    break;
       case    REAL:if (tmp->values[currentnode->featureno].floatvalue >= currentnode->split - 0.0001 && tmp->values[currentnode->featureno].floatvalue <= currentnode->split + 0.0001)
                      return YES;
                    else
                      return NO;
                    break;
      }
    }
   else
    {
     value = real_feature(tmp, currentnode->featureno);
     if (value >= currentnode->split - 0.0001 && value <= currentnode->split + 0.0001)
       return YES;
     else
       return NO;
    }
 else
  {
   value = multiply_with_matrix(tmp, currentnode->w);
   if (value >= currentnode->w0 - 0.0001 && value <= currentnode->w0 + 0.0001)
     return YES;
   else
     return NO;
  }
 return NO;
}

int is_smaller_discrete_order(Instanceptr inst, Korderingsplit ksplit)
{
 int i, j, fvalue, fno;
 for (i = 0; i < ksplit.featurecount; i++)
  {
   fno = ksplit.featurelist[i];
   fvalue = inst->values[fno].stringindex;
   if (fvalue == ksplit.sortorder[i][ksplit.discretesplit[i]])
     continue;
   for (j = 0; j < ksplit.discretesplit[i]; j++)
     if (fvalue == ksplit.sortorder[i][j])
       return YES;
   return NO;
  }
 return YES;
}

/**
 * Gets the next node in the decision tree testing. For example, if the decision function in the current node is a univariate function (x_i, w_0), and if the instance has a value larger than w_0 in the feature i, next decision node will be the right child of the current node, otherwise it will be the left child node (if the instance has a value smaller than w_0 in the feature i).
 * @param[in] currentnode Current decision tree node which is tested
 * @param[in] tmp Current instance
 * @return Next node in the decision tree according to the decision function in the current node and the instance
 */
Decisionnodeptr next_node(Decisionnodeptr currentnode, Instanceptr tmp)
{
 /*!Last Changed 28.05.2005 added SVM*/
 /*!Last Changed 26.09.2004*/
 double value, threshold;
 if (currentnode->featureno >= 0)
   if (currentnode->conditiontype != HYBRID_CONDITION)
    {
     switch (current_dataset->features[currentnode->featureno].type)
      {
       case INTEGER:if (tmp->values[currentnode->featureno].intvalue<=currentnode->split)
                      return currentnode->left;
                    else
                      return currentnode->right;
                    break;
       case    REAL:if (tmp->values[currentnode->featureno].floatvalue<=currentnode->split)
                      return currentnode->left;
                    else
                      return currentnode->right;
                    break;
       case  STRING:return &(currentnode->string[tmp->values[currentnode->featureno].stringindex]);
                    break;
      }
    }
   else
    {
     value = real_feature(tmp, currentnode->featureno);
     if (value <= currentnode->split)
       return currentnode -> left;
     else
       return currentnode -> right;
    }
 else
  {
   if (in_list(currentnode->featureno, 2, LINEAR, QUADRATIC))
    {
     value = multiply_with_matrix(tmp, currentnode->w);
     threshold = currentnode->w0;
    }
   else
     if (currentnode->featureno == SVMLIN)
      {
       value = svm_split_value(tmp, currentnode->svmsplit);
       threshold = 0;
      }
     else
       if (currentnode->featureno == DISCRETEMIX)
        {
         if (is_smaller_discrete_order(tmp, currentnode->ksplit))
           return currentnode->left;
         else
           return currentnode->right;
        }
       else
         return NULL;
   if (value <= threshold)
     return currentnode -> left;
   else
     return currentnode -> right;
  }
 return NULL;
}

/**
 * Tests a decision tree with the given test data.
 * @param[in] root Root node of the decision tree
 * @param[in] testingdata Header of the link list containing the test instances
 * @return Number of correctly classified instances using the given decision tree.
 */
int test_tree(Decisionnodeptr root, Instanceptr testingdata)
{
 /*!Last Changed 01.11.2005*/
 /*!Last Changed 26.09.2004*/
 /*!Last Changed 22.03.2002*/
 int i, performance = 0;
 Instanceptr test = testingdata;
 Decisionnodeptr currentnode;
 while (test)
  {
   currentnode = root;
   while (currentnode->featureno != LEAF_NODE)
     currentnode = next_node(currentnode, test); 
   i = give_classno(test);
   if (i == currentnode->classno)
     performance++;
   test = test->next;
  }
 return performance;
}

/**
 * Frees memory allocated to the rules and if the split is a multivariate split the memory allocated for the matrix containing the weights of the features
 * @param[in] node Decision tree node
 */
void deallocate_tree_rule(Decisionnodeptr node)
{
 /*!Last Changed 29.11.2004*/
 int i;
 if (node->rule)
  {
   for (i = 0; i < node->conditioncount; i++)
			 {
     if (in_list(node->rule[i].featureindex, 2, LINEARRULE, QUADRATICRULE))
       matrix_free(node->rule[i].w);
					if (node->rule[i].featureindex == DISCRETEMIX)
						 deallocate_korderingsplit(node->rule[i].ksplit);
				}
   safe_free(node->rule);
  }
}

void deallocate_korderingsplit(Korderingsplit ksplit)
{
	int i;
 safe_free(ksplit.featurelist);
 safe_free(ksplit.discretesplit);
 for (i = 0; i < ksplit.featurecount; i++)
   safe_free(ksplit.sortorder[i]);
 safe_free(ksplit.sortorder);
}

/**
 * Recursive destructor function of a decision tree. Frees memory allocated to the counts array, instances (if the node is a leaf node), rules, matrix of weights for multivariate splits, etc.
 * @param[in] node Decision tree node
 */
void deallocate_tree(Decisionnodeptr node)
{
 /*!Last Changed 24.05.2006 added free_children*/
 /*!Last Changed 20.06.2005*/
 /*!Last Changed 28.05.2005 added SVM*/
 /*!Last Changed 23.09.2004 added hybrid condition and multivariate*/
 /*!Last Changed 08.04.2003*/
 /*Added freeing rules array at each node*/
 int i;
 if (node->counts)
   safe_free(node->counts);
 if (node->outputs)
   safe_free(node->outputs);
 if (node->instances)
   deallocate(node->instances);
 if (node->featureno == LEAF_NODE)
  {
   if (node->leaftype == LINEARLEAF)
     matrix_free(node->w);
   deallocate_tree_rule(node);
   if (node->w0s)
     safe_free(node->w0s);
  }
 else
  {
   if (node->featureno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[node->featureno].type==STRING)
    {
     for (i = 0; i < current_dataset->features[node->featureno].valuecount; i++)
       deallocate_tree(&(node->string[i]));
     safe_free(node->string);
    }
   else
    {
     if (in_list(node->featureno, 3, LINEAR, QUADRATIC, SOFTLIN))
       matrix_free(node->w);
     else
       if (node->featureno == SVMLIN)
         free_svm_split(node->svmsplit);
       else
         if (node->featureno == DISCRETEMIX)
           deallocate_korderingsplit(node->ksplit);
     free_children(node);
    }
   deallocate_tree_rule(node);
  }
}

/**
 * Recursive function to prune a decision tree. The function checks if replacing current subtree with a leaf node will increase the error on the validation data. If it does not increase the error, the subtree is pruned.
 * @param[in] root Root node of the decision tree
 * @param[in] toberemoved Root node of the subtree that is checked to be pruned
 * @param[in] cvdata Validation data
 */
void prune_tree(Decisionnodeptr root, Decisionnodeptr toberemoved, Instanceptr cvdata)
{
 /*!Last Changed 24.05.2006 added free_children*/
 /*!Last Changed 10.08.2005 added root in order to remove c45node*/
 /*!Last Changed 28.05.2005 added SVM*/
 /*!Last Changed 22.09.2004 handles also multivariate decision trees*/
 /*!Last Changed 18.03.2004*/
 /*!Last Changed 23.03.2002*/
 int performancebefore, performance, tmp, i;
 if (toberemoved->featureno == LEAF_NODE)
   return;
 performancebefore = test_tree(root, cvdata);
 tmp = toberemoved->featureno;
 toberemoved->featureno = LEAF_NODE;
 performance = test_tree(root, cvdata);
 if (performance >= performancebefore)
  {
   if (in_list(tmp, 2, LINEAR, QUADRATIC))
     matrix_free(toberemoved->w);
   else
     if (tmp == SVMLIN)
       free_svm_split(toberemoved->svmsplit);
   toberemoved->featureno = tmp;
   accumulate_instances_tree(&(toberemoved->instances), toberemoved);
   toberemoved->featureno = LEAF_NODE;
   if (tmp >= 0 && (root->conditiontype != HYBRID_CONDITION) && current_dataset->features[tmp].type == STRING)
    {
     for (i = 0; i < current_dataset->features[tmp].valuecount;i++)
       deallocate_tree(&(toberemoved->string[i]));
     safe_free(toberemoved->string);
     toberemoved->string = NULL;
    }
   else
     free_children(toberemoved);
  }
 else
  {
   toberemoved->featureno = tmp;
   if (toberemoved->featureno >= 0 && (root->conditiontype != HYBRID_CONDITION) && current_dataset->features[toberemoved->featureno].type == STRING)
    {
     for (i = 0;i < current_dataset->features[toberemoved->featureno].valuecount;i++)
       prune_tree(root, &(toberemoved->string[i]), cvdata);
    }
   else
    {
     prune_tree(root, toberemoved->left, cvdata);
     prune_tree(root, toberemoved->right, cvdata);
    }
  }
}

double entropy(int* counts)
{
 double p, result = 0.0;
 int i, sum = 0;
 for (i = 0; i < current_dataset->classno; i++)
   sum += counts[i];
 for (i = 0; i < current_dataset->classno; i++)
   if (counts[i] > 0 && sum > 0)
    {
     p = counts[i] / (sum + 0.0);
     result -= p * log2(p);
    }
 return result;
}

/**
 * Calculates weighted information gain from a two dimensional list of counts.
 * @param[in] ratio[][] ratio[i][j] is the number of occurence of class j in the i'th group of the problem. If the problem is dividing examples in two groups (like left child and right child of the tree), ratio[0][j] will be the number of examples from class j on the left node and ratio[1][j] will be the number of examples from class j on the right node.
 * @return Weighted information gain calculated as \sum_i w_i \sum_j -p_{ij} log p_{ij} where w_i = \sum_j ratio[i][j] and p_{ij} = ratio[i][j] / \sum_j ratio[i][j].
 */
double information_gain_from_ratios(int ratio[MAXVALUES][MAXCLASSES])
{
/*!Last Changed 23.04.2003*/
 int i, j, howmany = 0;
 double totalover, totalunder, sum[MAXVALUES], res[MAXVALUES], p;
 totalover = 0;
 totalunder = 0;
 for (i = 0; i < MAXVALUES; i++)
  {
   sum[i] = 0;
   for (j = 0; j < current_dataset->classno; j++)
     sum[i] += ratio[i][j];
   res[i] = 0;
   if (sum[i] > 0)
    {
     for (j = 0; j < current_dataset->classno; j++)
       if (ratio[i][j] > 0)
        {
         p = ratio[i][j] / (sum[i] + 0.0);
         res[i] -= p * log2(p);
        }
     howmany++;
    }
   totalunder += sum[i];
   totalover += sum[i] * res[i];
  }
 if (totalunder == 0 || howmany == 1)
   return ISELL;
 else
   return totalover / totalunder;
}

/**
 * Recursive function to calculate the number of instances in each class in the corresponding subtree.
 * @param[in] subtree Root node of the subtree
 * @param[out] counts counts[i] is the number of instances in class i.
 * @warning counts must be initialized to zero array before calling this function first.
 */
void find_subtree_class_counts(Decisionnodeptr subtree, int* counts)
{
 /*!Last Changed 10.02.2005*/
 int* classcounts, i;
 if (subtree->featureno == LEAF_NODE)
  {
   classcounts = find_class_counts(subtree->instances);
   for (i = 0; i < current_dataset->classno; i++)
     counts[i] += classcounts[i];
   safe_free(classcounts);
  }
 else
  {
   find_subtree_class_counts(subtree->left, counts);
   find_subtree_class_counts(subtree->right, counts);
  }
}

/**
 * Recursive function to calculate the number of instances covered and the number of false positives in the corresponding subtree.
 * @param[in] root Root node of the subtree
 * @param[out] C Number of instances covered by this subtree (in this subtree)
 * @param[out] fp Number of false positives in this subtree
 */
void subtree_covered_and_false_positives(Decisionnodeptr root, int* C, int* fp)
{
 /*!Last Changed 10.02.2005*/
 if (root->featureno == LEAF_NODE)
  {
   *C += root->covered;
   *fp += root->falsepositives;
  }
 else
  {
   subtree_covered_and_false_positives(root->left, C, fp);
   subtree_covered_and_false_positives(root->right, C, fp);
  }
}

/**
 * Recursive function to calculate the total loglikelihood of the corresponding subtree. Calls log_likelihood_for_classification.
 * @param[in] root Root node of the subtree
 * @return Loglikelihood of the subtree
 */
double decisiontree_loglikelihood(Decisionnodeptr root)
{
 /*!Last Changed 10.02.2005*/
 int* classcounts;
 double loglikelihood;
 if (root->featureno == LEAF_NODE)
  {
   classcounts = find_class_counts(root->instances);
   loglikelihood = log_likelihood_for_classification(classcounts);
   safe_free(classcounts);
   return loglikelihood;
  }
 else
   return decisiontree_loglikelihood(root->left) + decisiontree_loglikelihood(root->right);
}

/**
 * Given the number of instances in each class, this function calculates the loglikelihood.
 * @param[in] classcounts[] Number of instances in each class. classcounts[i] is the number of instances in class i.
 * @return Likelihood of an instance from class i is calculated as classcounts[i] / \sum classcounts[i]. Loglikelihood of that instance is log(classcounts[i] / \sum classcounts[i]). Since we assume that instances are i.i.d, we sum up all the loglikelihoods.
 */
double log_likelihood_for_classification(int classcounts[])
{
 /*!Last Changed 22.11.2004*/
 int i, sum = 0;
 double result = 0, p;
 for (i = 0; i < current_dataset->classno; i++)
   sum += classcounts[i];
 for (i = 0; i < current_dataset->classno; i++)
   if (classcounts[i] != 0)
    {
     p = classcounts[i] / (sum + 0.0);
     result += classcounts[i] * log2(p);
    }
 return result;
}

/**
 * Calculates weighted information gain from a two dimensional list of counts.
 * @param[in] ratio[][] ratio[i][j] is the number of occurence of class j in the i'th group of the problem. If the problem is dividing examples in two groups (like left child and right child of the tree), ratio[0][j] will be the number of examples from class j on the left node and ratio[1][j] will be the number of examples from class j on the right node.
 * @return Weighted information gain calculated as \sum_i w_i \sum_j -p_{ij} log p_{ij} where w_i = (\sum_j ratio[i][j]) / (\sum_j w_j) and p_{ij} = ratio[i][j] / \sum_j ratio[i][j].
 */
double information_gain(int ratio[2][MAXCLASSES])
{
 int sum[2] = {0,0},i,j;
 double res[2] = {0,0},p;
 for (j = 0; j < 2; j++)
  {
   for (i = 0; i < current_dataset->classno; i++)
     sum[j] += ratio[j][i];
   if (sum[j] == 0)
     return ISELL;
   for (i = 0; i < current_dataset->classno; i++)
     if (ratio[j][i] > 0)
      {
       p = ratio[j][i] / (sum[j] + 0.0);
       res[j] -= p * log2(p);
      }
  }
 return (res[0] * sum[0] + res[1] * sum[1]) / (sum[0] + sum[1]);
}

double error_of_split(int counts[2][MAXCLASSES], int N)
{
 /*!Last Changed 19.07.2009*/
 int leftclass, rightclass;
 leftclass = find_max_in_list(counts[0], MAXCLASSES);
 rightclass = find_max_in_list(counts[1], MAXCLASSES);
 return 1 - (counts[0][leftclass] + counts[1][rightclass]) / (N + 0.0);
}

/**
 * Constructor for the partition structure. It allocates memory for the assignments array and sets the classes array with the classes of the problem.
 * @param[in] instances Training data
 * @return Empty partition
 */
Partition get_empty_partition(Instanceptr instances)
{
 /*!Last Changed 31.05.2005*/
 Partition result;
 int count;
 result.classes = get_classes(instances, &count);
 result.count = count;
 result.assignments = (int *)safecalloc(count, sizeof(int), "get_empty_partition", 5);
 return result;
}

/**
 * Prepares the initial partitioning of classes of the problem. If this is a two-class problem, first class is assigned to the left group, second class is assigned to the right group. If this is a K-class problem (K > 2), classes are assigned to left and right groups randomly.
 * @param[in] instances Training data
 * @return Random partitioning of classes into two groups
 */
Partition get_initial_partition(Instanceptr instances)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 26.01.2003*/
 Partition result;
 int i, count;
 result.classes = get_classes(instances, &count);
 result.count = count;
 result.assignments = (int *)safemalloc(count*sizeof(int), "get_initial_partition", 5);
 if (count == 2)
  {
   result.assignments[0] = LEFT;
   result.assignments[1] = RIGHT;
  }
 else
   for (i = 0; i < count; i++)
     result.assignments[i] = myrand() % 2;
 return result;
}

/**
 * Prepares the partitioning of 1-vs-others problem. One class is assigned to the left group, other classes are assigned to the right group.
 * @param[in] instances Training data
 * @param[in] leftclass The class 1 in the 1-vs-others problem.
 * @return Partitioning of classes
 */
Partition get_start_partition(Instanceptr instances, int leftclass)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 29.12.2003*/
 /*!Last Changed 04.02.2003*/
 Partition result;
 int i, count;
 result.classes = get_classes(instances, &count);
 result.count = count;
 result.assignments = (int *)safemalloc(count*sizeof(int), "get_start_partition", 5);
 for (i = 0; i < count; i++)
   result.assignments[i] = RIGHT;
 for (i = 0; i < count; i++)
   if (result.classes[i] == leftclass)
    {
     result.assignments[i] = LEFT;
     break;
    }
 return result;
}

/**
 * Recursive destructor function for decision tree nodes.
 * @param[in] node Root node of the subtree
 */
void free_children(Decisionnodeptr node)
{
 /*!Last Changed 20.04.2008*/
 /*!Last Changed 24.05.2006*/
 int i, fno = node->featureno;
 if (fno >= 0 && (node->conditiontype != HYBRID_CONDITION) && current_dataset->features[fno].type == STRING)
  {
   for (i = 0; i < current_dataset->features[fno].valuecount; i++)
     deallocate_tree(&(node->string[i]));
   safe_free(node->string);
   node->string = NULL;
  }
 else
  {
   deallocate_tree(node->left);
   safe_free(node->left);
   node->left = NULL;
   deallocate_tree(node->right);
   safe_free(node->right); 
   node->right = NULL;
  }
}

double svm_split_value(Instanceptr inst, Svm_split s)
{
 /*!Last Changed 28.05.2005*/
 int i;
 double value = 0;
 Svm_nodeptr x;
 x = instance_to_svmnode(inst);
 for (i = 0; i < s.l; i++)
   value += s.sv_coef[i] * k_function(x, s.SV[i], s.param);
 value -= s.rho;
 safe_free(x);
 return value;
}

double soft_output_of_subdecisiontree(Decisionnodeptr subtree, double input1, double input2)
{
 double weight, output, multiplier;
 Decisionnodeptr tmpnode;
 if (subtree->featureno == LEAF_NODE)
  {
   if (subtree->leaftype == CONSTANTLEAF)
     output = subtree->w0;    
   else
     output = subtree->w.values[0][0] + subtree->w.values[1][0] * input1 + subtree->w.values[2][0] * input2; 
   multiplier = 1.0;
   tmpnode = subtree;
   while (tmpnode->parent)
    {
     weight = sigmoid(tmpnode->parent->w.values[0][0] + tmpnode->parent->w.values[1][0] * input1 + tmpnode->parent->w.values[2][0] * input2);
     if (tmpnode == tmpnode->parent->left)
       multiplier *= weight;
     else
       multiplier *= (1 - weight);
     tmpnode = tmpnode->parent;
    }
   return output * multiplier;   
  }
 else
   return soft_output_of_subdecisiontree(subtree->left, input1, input2) + soft_output_of_subdecisiontree(subtree->right, input1, input2);   
}

/**
 * Recursive function to calculate depth of the decision node. Depth of the leaf node is 1 and depth of a decision node is one larger than the maximum depth of its subtrees.
 * @param[in] node Root node of the subtree
 * @return Depth of the root node of the subtree
 */
int depth_of_tree(Decisionnodeptr node)
{
 /*!Last Changed 12.07.2006*/
 int i, maxdepth, depth, leftdepth, rightdepth;
 if (node->featureno == LEAF_NODE)
   return 1;
 else
  {
   if (node->featureno >= 0 && current_dataset->features[node->featureno].type == STRING)
    {
     maxdepth = -1;
     for (i = 0; i < current_dataset->features[node->featureno].valuecount; i++)
      { 
       depth = depth_of_tree(&(node->string[i]));
       if (depth > maxdepth)
         maxdepth = depth;
      }
     return maxdepth + 1;
    }
   else
    {
     leftdepth = depth_of_tree(node->left);
     rightdepth = depth_of_tree(node->right);
     if (leftdepth >= rightdepth)
       return leftdepth + 1;
     else
       return rightdepth + 1;     
    }
  }
}

void inorder_traversal_decision_tree(Decisionnodeptr node, int* index)
{
 int i;
 if (node)
  {
   if (node->featureno >= 0 && current_dataset->features[node->featureno].type == STRING)
    {   
     for (i = 0; i < current_dataset->features[node->featureno].valuecount / 2; i++)
       inorder_traversal_decision_tree(&(node->string[i]), index);
     node->x = *index;
     (*index)++;     
     for (i = current_dataset->features[node->featureno].valuecount / 2; i < current_dataset->features[node->featureno].valuecount; i++)
       inorder_traversal_decision_tree(&(node->string[i]), index);
    }
   else
    {
     inorder_traversal_decision_tree(node->left, index);
     node->x = *index;
     (*index)++;
     inorder_traversal_decision_tree(node->right, index);        
    }
  }
}

void level_of_decision_tree(Decisionnodeptr node)
{
 int i;
 if (node)
  {
   if (node->parent)
     node->y = node->parent->y + 1;
   else
     node->y = 0;
   if (node->featureno >= 0 && current_dataset->features[node->featureno].type == STRING)
     for (i = 0; i < current_dataset->features[node->featureno].valuecount; i++)
       level_of_decision_tree(&(node->string[i]));
   else
    {
     level_of_decision_tree(node->left);
     level_of_decision_tree(node->right);       
    }
  }
}

/**
 * Calculates the level of a decision tree node. The level of the root node of the decision tree is 0, the level of an inner decision node is one larger than the level of its parent node.
 * @param[in] node Root node of the subtree
 * @return Level of the node.
 */
int level_of_node(Decisionnodeptr node)
{
 /*!Last Changed 08.06.2006*/
 int level = 0;
 Decisionnodeptr tmp = node;
 while (tmp->parent)
  {
   level++;
   tmp = tmp->parent;
  }
 return level;
}

void leaf_distribution(Decisionnodeptr rootnode, int* counts)
{
 /*!Last Changed 01.10.2006 added decisiontree with single leaf node*/
 /*!Last Changed 13.07.2006*/
 int level, index;
 if (rootnode->featureno == LEAF_NODE)
  {
   level = level_of_node(rootnode);
   if (level == 0)
     index = 1;
   else
     if (rootnode->parent->left == rootnode) 
       if (rootnode->parent->right->featureno == LEAF_NODE)
         index = 2 * level;
       else
         index = 2 * level + 1;
     else
       if (rootnode->parent->left->featureno == LEAF_NODE)
         index = 2 * level;
       else
         index = 2 * level + 1;
   counts[index - 1]++;
  }
 else
  {
   leaf_distribution(rootnode->left, counts);
   leaf_distribution(rootnode->right, counts);
  }
}
