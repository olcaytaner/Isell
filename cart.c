#include <math.h>
#include "decisiontree.h"
#include "instance.h"
#include "multivariatetree.h"
#include "cart.h"

extern Datasetptr current_dataset;

int find_best_cart_split(Decisionnodeptr node)
{
 int i;
 Instanceptr inst;
 double w0, v, gamma, bestc, c, delta, impurity, bestimpurity, bestdelta, bestgamma;
 matrix w;
 BOOLEAN improved = BOOLEAN_TRUE;
 node->w = matrix_alloc(1, current_dataset->multifeaturecount - 1);
 w = matrix_alloc(1, current_dataset->multifeaturecount - 1);
 node->w0 = 0;
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
   node->w.values[0][i] = 1 / sqrt(current_dataset->multifeaturecount - 1);
 while (improved)
  {
   matrix_identical(&w, node->w);
   w0 = node->w0;
   for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
    {
     bestimpurity = ISELL;
     for (gamma = -0.25; gamma < 0.3; gamma += 0.25)
      {
       inst = node->instances;
       while (inst)
        {
         v = multiply_with_matrix(inst, w);
         if (real_feature(inst, i) + gamma != 0.0)
          {
           delta = (v - w0) / (real_feature(inst, i) + gamma);
           w.values[0][i] -= delta;
           w0 += delta * gamma;
           impurity = information_gain_for_multivariate_split(node->instances, w, w0);
           w.values[0][i] += delta;
           w0 -= delta * gamma;
           if (impurity < bestimpurity)
            {                     
             bestimpurity = impurity;
             bestdelta = delta;
             bestgamma = gamma;
            }
          }
         inst = inst->next;
        }
      }
     if (bestimpurity != ISELL)
      {
       w.values[0][i] -= bestdelta;
       w0 += bestdelta * bestgamma;      
      }
    }
   bestimpurity = ISELL;
   inst = node->instances;
   while (inst)
    {
     c = multiply_with_matrix(inst, w);
     impurity = information_gain_for_multivariate_split(node->instances, w, c);
     if (impurity < bestimpurity)
      {
       bestc = c;
       bestimpurity = impurity;
      }
     inst = inst->next;
    }
   if (bestimpurity != ISELL)
     w0 = bestc;
   if (information_gain_for_multivariate_split(node->instances, w, w0) < information_gain_for_multivariate_split(node->instances, node->w, node->w0))
    {
     matrix_identical(&(node->w), w);
     node->w0 = w0;
     improved = BOOLEAN_TRUE;
    }
   else
     improved = BOOLEAN_FALSE;
  }
 matrix_free(w);
 if (information_gain_for_multivariate_split(node->instances, node->w, node->w0) == ISELL)
   return 0;
 return 1;
}

int create_cartchildren(Decisionnodeptr node, C45_parameterptr param)
{
 node->conditiontype = MULTIVARIATE_CONDITION;
 if (!(can_be_divided(node, param)))
   return 1;
 if (!find_best_cart_split(node))
  {
   make_leaf_node(node);
   return 1;
  }
 node->lineard = current_dataset->multifeaturecount - 1;
 node->featureno = LINEAR; 
 if (!(make_multivariate_children(node)))
   return 1;
 create_cartchildren(node->left, param);
 create_cartchildren(node->right, param);
 return 1;
}
