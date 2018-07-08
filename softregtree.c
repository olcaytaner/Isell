#include <math.h>
#include "instance.h"
#include "instancelist.h"
#include "libmemory.h"
#include "librandom.h"
#include "perceptron.h"
#include "regressiontree.h"
#include "softregtree.h"

extern Datasetptr current_dataset;

double soft_regtree_output(Regressionnodeptr node, Instanceptr inst)
{
 if (node->featureno == LEAF_NODE)
   if (node->leaftype == CONSTANTLEAF)
     node->output = node->regressionvalue;
   else
     node->output = linear_fit_value(node->w, inst);
 else
  {
   node->left->weight = sigmoid(linear_fit_value(node->w, inst));
   node->right->weight = 1 - node->left->weight;
   node->output = node->left->weight * soft_regtree_output(node->left, inst) + node->right->weight * soft_regtree_output(node->right, inst);
  }
 return node->output;
}

double mse_error_of_soft_regression_tree(Regressionnodeptr rootnode, Instanceptr list)
{
 int count = 0;
 double r, o, mse = 0.0;
 Instanceptr inst = list;
 while (inst)
  {
   r = give_regressionvalue(inst);
   o = soft_regtree_output(rootnode, inst);
   mse += (r - o) * (r - o);
   inst = inst->next;
   count++;
  }
 return mse / count;
}

void gradient_descent_soft_regtree(Regressionnodeptr rootnode, Regressionnodeptr node, Softregtree_parameterptr param, double eta, double lambda)
{
 int i, j, k, *array, datasize;
 double o, r, multiplier, delta, feature_value;
 Instanceptr inst; 
 Regressionnodeptr tmpnode;
 datasize = data_size(rootnode->instances);
 node->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
 if (param->leaftype == CONSTANTLEAF)
  {
   node->left->regressionvalue = produce_random_value(-0.01, +0.01);
   node->right->regressionvalue = produce_random_value(-0.01, +0.01);
  }
 else
  {
   node->left->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
   node->right->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
  }
 for (i = 0; i < param->epochs; i++)
  {
   array = random_array(datasize);
   for (j = 0; j < datasize; j++)
    {
     inst = data_x(rootnode->instances, array[j]);
     o = soft_regtree_output(rootnode, inst);
     r = give_regressionvalue(inst);
     multiplier = eta * (r - o);
     tmpnode = node;
     while (tmpnode->parent)
      {
       multiplier *= tmpnode->weight;
       tmpnode = tmpnode->parent;
      }
     if (param->leaftype == CONSTANTLEAF)
      {
       node->left->regressionvalue += multiplier * node->left->weight;
       node->right->regressionvalue += multiplier * node->right->weight;
      }
     for (k = 0; k < current_dataset->multifeaturecount; k++)
      {      
       if (k != 0)
         feature_value = real_feature(inst, k - 1);
       else
         feature_value = 1;
       delta = multiplier * feature_value;
       if (param->regularization == NO_REGULARIZATION)
         node->w.values[k][0] += delta * (node->left->output - node->right->output) * node->left->weight * node->right->weight;
       else
         if (param->regularization == L1_REGULARIZATION)
          {
           if (feature_value > 0)
             node->w.values[k][0] += ((1 - lambda) * delta * (node->left->output - node->right->output) * node->left->weight * node->right->weight) / datasize + eta * lambda / current_dataset->multifeaturecount;
           else
             node->w.values[k][0] += ((1 - lambda) * delta * (node->left->output - node->right->output) * node->left->weight * node->right->weight) / datasize - eta * lambda / current_dataset->multifeaturecount;
          }
         else
           node->w.values[k][0] += ((1 - lambda) * delta * (node->left->output - node->right->output) * node->left->weight * node->right->weight) / datasize + (eta * lambda * feature_value) / current_dataset->multifeaturecount;           
       if (param->leaftype != CONSTANTLEAF)
        { 
         node->left->w.values[k][0] += delta * node->left->weight;
         node->right->w.values[k][0] += delta * node->right->weight;
        }
      }
    }
   safe_free(array);   
   eta = eta * param->etadecrease;
  }
}

void free_soft_regtree_ws(Regressionnodeptr node)
{
 matrix_free(node->w);
 if (node->leaftype != CONSTANTLEAF)
  {
   matrix_free(node->left->w);
   matrix_free(node->right->w);
  } 
}

void copy_soft_regtree_ws(Regressionnodeptr dest, Regressionnodeptr src)
{
 dest->w = matrix_copy(src->w);
 if (src->leaftype == CONSTANTLEAF)
  {
   dest->left->regressionvalue = src->left->regressionvalue;
   dest->right->regressionvalue = src->right->regressionvalue;
  }
 else
  {
   dest->left->w = matrix_copy(src->left->w);
   dest->right->w = matrix_copy(src->right->w);
  }
}

BOOLEAN find_best_soft_regtree_split(Regressionnodeptr rootnode, Regressionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param)
{
 int i, j, datasize;
 double performancebefore, performancenow, performancebest, tmp, eta, lambda;
 BOOLEAN result = BOOLEAN_TRUE;
 matrix tmpw;
 Regressionnodeptr bestnode;
 if (param->leaftype != CONSTANTLEAF)
   tmpw = matrix_copy(node->w);
 else
   tmp = node->regressionvalue;
 performancebefore = mse_error_of_soft_regression_tree(rootnode, cvdata);
 performancebest = performancebefore;
 datasize = data_size(rootnode->instances);
 node->left = createnewregnode(node, 1, param->leaftype);
 node->right = createnewregnode(node, 1, param->leaftype);
 node->featureno = SOFTLIN;
 bestnode = createnewregnode(node, 1, param->leaftype);
 bestnode->left = createnewregnode(node, 1, param->leaftype);
 bestnode->right = createnewregnode(node, 1, param->leaftype);
 if (param->regularization == NO_REGULARIZATION)
  {
   for (i = 0; i <= 10; i++)
    {
     eta = 10.0 / (pow(2, i));
     gradient_descent_soft_regtree(rootnode, node, param, eta, 0.0);
     performancenow = mse_error_of_soft_regression_tree(rootnode, cvdata);
     if (performancenow < performancebest)
      {    
       if (performancebest != performancebefore)
         free_soft_regtree_ws(bestnode);
       performancebest = performancenow;
       copy_soft_regtree_ws(bestnode, node);
      }
     free_soft_regtree_ws(node);
    }
  }
 else
  {
   for (i = 0; i <= 10; i++)
    {
     eta = 10.0 / pow(2, i);
     for (j = 1; j <= 10; j++)
      {
       lambda = 1.0 / pow(2, j);
       gradient_descent_soft_regtree(rootnode, node, param, eta, lambda);
       performancenow = mse_error_of_soft_regression_tree(rootnode, cvdata);
       if (performancenow < performancebest)
        {    
         if (performancebest != performancebefore)
           free_soft_regtree_ws(bestnode);
         performancebest = performancenow;
         copy_soft_regtree_ws(bestnode, node);
        }
       free_soft_regtree_ws(node);
      }
    }
  }
 if (performancebefore - performancebest <= 0.0001)
  {
   result = BOOLEAN_FALSE;
   safe_free(node->left);
   safe_free(node->right);
   make_leaf_regression_node(node, param->leaftype);
   if (param->leaftype != CONSTANTLEAF)
     node->w = tmpw;
   else
     node->regressionvalue = tmp;
  }
 else
  {
   if (param->leaftype != CONSTANTLEAF)
     matrix_free(tmpw);
   copy_soft_regtree_ws(node, bestnode);
   if (param->regularization != NO_REGULARIZATION)
     for (j = 0; j < current_dataset->multifeaturecount; j++)
       if (fabs(node->w.values[j][0]) < 1 / (1000.0 * current_dataset->multifeaturecount))
         node->w.values[j][0] = 0.0;
  }
 if (performancebest != performancebefore)
   free_soft_regtree_ws(bestnode);
 safe_free(bestnode->left);
 safe_free(bestnode->right);
 safe_free(bestnode);
 return result;
}

double soft_output_of_subregressiontree(Regressionnodeptr subtree, double input)
{
 double weight, output, multiplier;
 Regressionnodeptr tmpnode;
 if (subtree->featureno == LEAF_NODE)
  {
   if (subtree->leaftype == CONSTANTLEAF)
     output = subtree->regressionvalue;    
   else
     output = subtree->w.values[0][0] + subtree->w.values[1][0] * input; 
   multiplier = 1.0;
   tmpnode = subtree;
   while (tmpnode->parent)
    {
     weight = sigmoid(tmpnode->parent->w.values[0][0] + tmpnode->parent->w.values[1][0] * input);
     if (tmpnode == tmpnode->parent->left)
       multiplier *= weight;
     else
       multiplier *= (1 - weight);
     tmpnode = tmpnode->parent;
    }
   return output * multiplier;   
  }
 else
   return soft_output_of_subregressiontree(subtree->left, input) + soft_output_of_subregressiontree(subtree->right, input);   
}

int create_softregtree(Regressionnodeptr rootnode, Regressionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param)
{
 node->cvinstances = NULL;
 if (node != rootnode)
   node->instances = NULL;
 if (find_best_soft_regtree_split(rootnode, node, cvdata, param))
  {
   create_softregtree(rootnode, node->left, cvdata, param);
   create_softregtree(rootnode, node->right, cvdata, param);
  }
 return 1;
}
