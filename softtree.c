#include <math.h>
#include <string.h>
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "perceptron.h"
#include "regressiontree.h"
#include "softtree.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

double* soft_tree_output_K_class(Decisionnodeptr node, Instanceptr inst)
{
 int i;
 double *left_output, *right_output;
 if (node->featureno == LEAF_NODE)
   memcpy(node->outputs, node->w0s, current_dataset->classno * sizeof(double));
 else
  {
   node->left->weight = sigmoid(linear_fit_value(node->w, inst));
   node->right->weight = 1 - node->left->weight;
   left_output = soft_tree_output_K_class(node->left, inst);
   right_output = soft_tree_output_K_class(node->right, inst);
   for (i = 0; i < current_dataset->classno; i++)
     node->outputs[i] = node->left->weight * left_output[i] + node->right->weight * right_output[i];
  }
 return node->outputs;
}

double soft_tree_output(Decisionnodeptr node, Instanceptr inst)
{
 if (node->featureno == LEAF_NODE)
   if (node->leaftype == CONSTANTLEAF)
     node->output = node->w0;
   else
     node->output = linear_fit_value(node->w, inst);
 else
  {
   node->left->weight = sigmoid(linear_fit_value(node->w, inst));
   node->right->weight = 1 - node->left->weight;
   node->output = node->left->weight * soft_tree_output(node->left, inst) + node->right->weight * soft_tree_output(node->right, inst);
  }
 return node->output;
}

int error_of_soft_tree_K_class(Decisionnodeptr rootnode, Instanceptr list)
{
 int count = 0, classno;
 double* o;
 Instanceptr inst = list;
 while (inst)
  {
   classno = give_classno(inst);
   o = soft_tree_output_K_class(rootnode, inst);
   if (find_max_in_list_double(o, current_dataset->classno) != classno)
     count++;
   inst = inst->next;
  }
 return count;
}

int error_of_soft_tree(Decisionnodeptr rootnode, Instanceptr list)
{
 int count = 0, classno;
 double o;
 Instanceptr inst = list;
 while (inst)
  {
   classno = give_classno(inst);
   o = sigmoid(soft_tree_output(rootnode, inst));
   if ((o > 0.5 && classno == 0) || (o < 0.5 && classno == 1))
     count++;
   inst = inst->next;
  }
 return count;
}

void gradient_descent_soft_tree_K_class(Decisionnodeptr rootnode, Decisionnodeptr node, Softregtree_parameterptr param, double eta)
{
 int i, j, k, t, r, *array, datasize;
 double sum, multiplier, delta, feature_value, *output, *previous_update_left, *previous_update_right, *previous_update;
 Instanceptr inst;
 Decisionnodeptr tmpnode;
 datasize = data_size(rootnode->instances);
 node->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
 node->left->w0s = random_array_double(current_dataset->classno, -0.01, +0.01);
 node->right->w0s = random_array_double(current_dataset->classno, -0.01, +0.01);
 previous_update = safecalloc(current_dataset->multifeaturecount, sizeof(double), "gradient_descent_soft_tree_K_class", 9);
 previous_update_left = safecalloc(current_dataset->classno, sizeof(double), "gradient_descent_soft_tree_K_class", 9);
 previous_update_right = safecalloc(current_dataset->classno, sizeof(double), "gradient_descent_soft_tree_K_class", 9);
 for (i = 0; i < param->epochs; i++)
  {
   array = random_array(datasize);
   for (j = 0; j < datasize; j++)
    {
     inst = data_x(rootnode->instances, array[j]);
     output = soft_tree_output_K_class(rootnode, inst);
     softmax(output, current_dataset->classno);
     r = give_classno(inst);
     multiplier = eta;
     tmpnode = node;
     while (tmpnode->parent)
      {
       multiplier *= tmpnode->weight;
       tmpnode = tmpnode->parent;
      }
     for (k = 0; k < current_dataset->classno; k++)
      {      
       node->left->w0s[k] += multiplier * (kronecker_delta(r, k) - output[k]) * node->left->weight + param->alpha * previous_update_left[k];
       previous_update_left[k] = multiplier * (kronecker_delta(r, k) - output[k]) * node->left->weight;
       node->right->w0s[k] += multiplier * (kronecker_delta(r, k) - output[k]) * node->right->weight + param->alpha * previous_update_right[k];
       previous_update_right[k] = multiplier * (kronecker_delta(r, k) - output[k]) * node->right->weight;
      }
     for (t = 0; t < current_dataset->multifeaturecount; t++)
      {
       if (t != 0)
         feature_value = real_feature(inst, t - 1);
       else
         feature_value = 1;
       sum = 0;
       for (k = 0; k < current_dataset->classno; k++)
        {
         delta = multiplier * (kronecker_delta(r, k) - output[k]) * feature_value * (node->left->outputs[k] - node->right->outputs[k]) * node->left->weight * node->right->weight;
         sum += delta;
        }
       node->w.values[t][0] += sum + param->alpha * previous_update[t];
       previous_update[t] = sum;
     }
    }
   safe_free(array);
   eta = eta * param->etadecrease;
  }
 safe_free(previous_update_left);
 safe_free(previous_update_right);
 safe_free(previous_update);
}

void gradient_descent_soft_tree(Decisionnodeptr rootnode, Decisionnodeptr node, Softregtree_parameterptr param, double eta, double lambda)
{
 int i, j, k, *array, datasize;
 double o, r, multiplier, delta, feature_value;
 Instanceptr inst; 
 Decisionnodeptr tmpnode;
 datasize = data_size(rootnode->instances);
 node->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
 if (param->leaftype == CONSTANTLEAF)
  {
   node->left->w0 = produce_random_value(-0.01, +0.01);
   node->right->w0 = produce_random_value(-0.01, +0.01);
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
     o = sigmoid(soft_tree_output(rootnode, inst));
     r = give_classno(inst);
     multiplier = eta * (r - o);
     tmpnode = node;
     while (tmpnode->parent)
      {
       multiplier *= tmpnode->weight;
       tmpnode = tmpnode->parent;
      }
     if (param->leaftype == CONSTANTLEAF)
      {
       node->left->w0 += multiplier * node->left->weight;
       node->right->w0 += multiplier * node->right->weight;
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

void free_soft_tree_ws_K_class(Decisionnodeptr node)
{
 matrix_free(node->w);
 safe_free(node->left->w0s);
 safe_free(node->right->w0s);
}

void free_soft_tree_ws(Decisionnodeptr node)
{
 matrix_free(node->w);
 if (node->leaftype != CONSTANTLEAF)
  {
   matrix_free(node->left->w);
   matrix_free(node->right->w);
  } 
}

void copy_soft_tree_ws_K_class(Decisionnodeptr dest, Decisionnodeptr src)
{
 dest->w = matrix_copy(src->w);
 dest->left->w0s = clone_array(src->left->w0s, current_dataset->classno);
 dest->right->w0s = clone_array(src->right->w0s, current_dataset->classno);
}

void copy_soft_tree_ws(Decisionnodeptr dest, Decisionnodeptr src)
{
 dest->w = matrix_copy(src->w);
 if (src->leaftype == CONSTANTLEAF)
  {
   dest->left->w0 = src->left->w0;
   dest->right->w0 = src->right->w0;
  }
 else
  {
   dest->left->w = matrix_copy(src->left->w);
   dest->right->w = matrix_copy(src->right->w);
  }
}

BOOLEAN find_best_soft_tree_split_K_class(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param)
{
 int i, datasize, performancebefore, performancebest, performancenow;
 double eta, *tmp;
 BOOLEAN result = BOOLEAN_TRUE;
 Decisionnodeptr bestnode;
 tmp = clone_array(node->w0s, current_dataset->classno);
 performancebefore = error_of_soft_tree_K_class(rootnode, cvdata);
 performancebest = performancebefore;
 datasize = data_size(rootnode->instances);
 node->left = createnewnode(node, 1);
 node->left->outputs = safemalloc(current_dataset->classno * sizeof(double), "find_best_soft_tree_split_K_class", 8);
 node->left->leaftype = CONSTANTLEAF;
 node->right = createnewnode(node, 1);
 node->right->outputs = safemalloc(current_dataset->classno * sizeof(double), "find_best_soft_tree_split_K_class", 11);
 node->right->leaftype = CONSTANTLEAF;
 node->featureno = SOFTLIN;
 bestnode = createnewnode(node, 1);
 bestnode->outputs = safemalloc(current_dataset->classno * sizeof(double), "find_best_soft_tree_split_K_class", 15);
 bestnode->left = createnewnode(node, 1);
 bestnode->left->outputs = safemalloc(current_dataset->classno * sizeof(double), "find_best_soft_tree_split_K_class", 17);
 bestnode->right = createnewnode(node, 1);
 bestnode->right->outputs = safemalloc(current_dataset->classno * sizeof(double), "find_best_soft_tree_split_K_class", 19);
 for (i = 0; i <= 10; i++)
  {
   eta = 10.0 / pow(2, i);
   gradient_descent_soft_tree_K_class(rootnode, node, param, eta);
   performancenow = error_of_soft_tree_K_class(rootnode, cvdata);
   if (performancenow < performancebest)
    {
     if (performancebest != performancebefore)
       free_soft_tree_ws_K_class(bestnode);
     performancebest = performancenow;
     copy_soft_tree_ws_K_class(bestnode, node);
    }
   free_soft_tree_ws_K_class(node);
  }
 if (performancebefore <= performancebest)
  {
   result = BOOLEAN_FALSE;
   deallocate_tree_rule(node->left);
   deallocate_tree_rule(node->right);
   safe_free(node->left->outputs);
   safe_free(node->right->outputs);
   safe_free(node->left);
   safe_free(node->right);
   make_leaf_node(node);
   memcpy(node->w0s, tmp, current_dataset->classno * sizeof(double));
  }
 else
  {
   copy_soft_tree_ws_K_class(node, bestnode);
   safe_free(node->w0s);
  }
 if (performancebest != performancebefore)
   free_soft_tree_ws_K_class(bestnode);
 safe_free(tmp);
 deallocate_tree_rule(bestnode->left);
 deallocate_tree_rule(bestnode->right);
 deallocate_tree_rule(bestnode);
 safe_free(bestnode->left->outputs);
 safe_free(bestnode->right->outputs);
 safe_free(bestnode->outputs);
 safe_free(bestnode->left);
 safe_free(bestnode->right);
 safe_free(bestnode);
 return result;
}

BOOLEAN find_best_soft_tree_split(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param)
{
 int i, j, datasize, performancebefore, performancebest, performancenow;
 double tmp, eta, lambda;
 BOOLEAN result = BOOLEAN_TRUE;
 Decisionnodeptr bestnode;
 matrix tmpw; 
 if (param->leaftype != CONSTANTLEAF)
   tmpw = matrix_copy(node->w);
 else
   tmp = node->w0;
 performancebefore = error_of_soft_tree(rootnode, cvdata);
 performancebest = performancebefore;
 datasize = data_size(rootnode->instances);
 node->left = createnewnode(node, 1);
 node->left->leaftype = param->leaftype;
 node->right = createnewnode(node, 1);
 node->right->leaftype = param->leaftype;
 node->featureno = SOFTLIN;
 bestnode = createnewnode(node, 1);
 bestnode->left = createnewnode(node, 1);
 bestnode->right = createnewnode(node, 1);
 if (param->regularization == NO_REGULARIZATION)
  {
   for (i = 0; i <= 10; i++)
    {
     eta = 10.0 / pow(2, i);
     gradient_descent_soft_tree(rootnode, node, param, eta, 0.0);
     performancenow = error_of_soft_tree(rootnode, cvdata);
     if (performancenow < performancebest)
      {    
       if (performancebest != performancebefore)
         free_soft_tree_ws(bestnode);
       performancebest = performancenow;
       copy_soft_tree_ws(bestnode, node);
      }
     free_soft_tree_ws(node);
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
       gradient_descent_soft_tree(rootnode, node, param, eta, lambda);
       performancenow = error_of_soft_tree(rootnode, cvdata);
       if (performancenow < performancebest)
        {    
         if (performancebest != performancebefore)
           free_soft_tree_ws(bestnode);
         performancebest = performancenow;
         copy_soft_tree_ws(bestnode, node);
        }
       free_soft_tree_ws(node);
      }
    }
  }
 if (performancebefore <= performancebest)
  {
   result = BOOLEAN_FALSE;
   deallocate_tree_rule(node->left);
   deallocate_tree_rule(node->right);
   safe_free(node->left);
   safe_free(node->right);
   make_leaf_node(node);
   if (param->leaftype != CONSTANTLEAF)
     node->w = tmpw;
   else
     node->w0 = tmp;
  }
 else
  {
   if (param->leaftype != CONSTANTLEAF)
     matrix_free(tmpw);
   copy_soft_tree_ws(node, bestnode);
   if (param->regularization != NO_REGULARIZATION)
     for (j = 0; j < current_dataset->multifeaturecount; j++)
       if (fabs(node->w.values[j][0]) < 1 / (1000.0 * current_dataset->multifeaturecount))
         node->w.values[j][0] = 0.0;
  }
 if (performancebest != performancebefore)
   free_soft_tree_ws(bestnode);
 deallocate_tree_rule(bestnode->left);
 deallocate_tree_rule(bestnode->right);
 deallocate_tree_rule(bestnode);
 safe_free(bestnode->left);
 safe_free(bestnode->right);
 safe_free(bestnode);
 return result;
}

void soft_tree_dimension_reduction(Decisionnodeptr node, char st[READLINELENGTH], int level)
{
 int i, count;
	if (node->featureno != LEAF_NODE)
	 {
   count = 0;
   for (i = 0; i < current_dataset->multifeaturecount; i++)
     if (node->w.values[i][0] != 0)
       count++;
   sprintf(st,"%s%d-%d;", st, level, count);
   soft_tree_dimension_reduction(node->left, st, level + 1);
   soft_tree_dimension_reduction(node->right, st, level + 1);
	 }
}

int create_softtree_K_class(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param)
{
 if (node != rootnode)
   node->instances = NULL;
 if (find_best_soft_tree_split_K_class(rootnode, node, cvdata, param))
  {
   create_softtree_K_class(rootnode, node->left, cvdata, param);
   create_softtree_K_class(rootnode, node->right, cvdata, param);
  }
 return 1;
}

int create_softtree(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param)
{
 if (node != rootnode)
   node->instances = NULL;
 if (find_best_soft_tree_split(rootnode, node, cvdata, param))
  {
   create_softtree(rootnode, node->left, cvdata, param);
   create_softtree(rootnode, node->right, cvdata, param);
  }
 return 1;
}
