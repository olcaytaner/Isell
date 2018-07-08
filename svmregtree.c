#include <limits.h>
#include <math.h>
#include "instance.h"
#include "instancelist.h"
#include "libmemory.h"
#include "regressiontree.h"
#include "svmregtree.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

int find_best_svmregtree_feature(Instanceptr instances, double* split, double C, LEAF_TYPE leaftype)
{
 /*!Last Changed 04.11.2009*/
 Instanceptr sorted;
 int i, j, k, size, bestfeature;
 double difference, besterror = +INT_MAX, error, regvalue, leftmean, rightmean;
 for (i = 0; i < current_dataset->featurecount; i++)
   if (current_dataset->features[i].type != REGDEF)
    {
     sorted = sort_instances(instances, i, &size);
     leftmean = 0;
     rightmean = find_mean_of_regression_value(instances);
     for (j = 0; j < size - 1; j++)
      {
       regvalue = give_regressionvalue(&(sorted[j]));
       leftmean = (leftmean * j + regvalue) / (j + 1);
       rightmean = (rightmean * (size - j) - regvalue) / (size - j - 1);
       error = 0.0;
       for (k = 0; k <= j; k++)
        {
         difference = fabs(give_regressionvalue(&(sorted[k])) - leftmean);
         if (difference > C)
           error += difference - C;
        }
       for (k = j + 1; k < size; k++)
        {
         difference = fabs(give_regressionvalue(&(sorted[k])) - rightmean);
         if (difference > C)
           error += difference - C;
        }
       if (error < besterror)
        {
         besterror = error;
         bestfeature = i;
         *split = (sorted[j].values[i].floatvalue + sorted[j + 1].values[i].floatvalue) / 2;
        }
      }
     safe_free(sorted);
    }
 return bestfeature;
}

int optimize_C_for_svmregtreenode(Regressionnodeptr node, LEAF_TYPE leaftype)
{
 /*!Last Changed 01.11.2009*/
 int bestfeature = -1, currentfeature;
 double C, currenterror, besterror = +INT_MAX, currentsplit;
 Instanceptr validate = NULL, learn = NULL, leftinstances, rightinstances, testleft, testright;
 divide_instancelist(&(node->instances), &validate, &learn, 4);
 for (C = 0.0; C <= 3.0; C+= 0.1)
  {
   currentfeature = find_best_svmregtree_feature(learn, &currentsplit, C, leaftype);
   leftinstances = rightinstances = testleft = testright = NULL;
   divide_instancelist_according_to_split(&learn, &leftinstances, &rightinstances, currentfeature, currentsplit);
   divide_instancelist_according_to_split(&validate, &testleft, &testright, currentfeature, currentsplit);
   currenterror = constant_or_linear_model_mse(leftinstances, testleft, leaftype) + constant_or_linear_model_mse(rightinstances, testright, leaftype);
   learn = restore_instancelist(learn, leftinstances);
   merge_instancelist(&learn, rightinstances); 
   validate = restore_instancelist(validate, testleft);
   merge_instancelist(&validate, testright); 
   if (currenterror < besterror)
    {
     besterror = currenterror;
     bestfeature = currentfeature;
     node->split = currentsplit;
    }
  }
 node->instances = restore_instancelist(node->instances, learn);
 merge_instancelist(&(node->instances), validate);
 return bestfeature;
}

int create_svmregtree(Regressionnodeptr node, Regtree_parameterptr param)
{
 /*!Last Changed 01.11.2009*/
 Instanceptr leftinstances = NULL, rightinstances = NULL;
 if (!(can_be_divided_regression(node, param)))
   return 1;
 node->featureno = optimize_C_for_svmregtreenode(node, param->leaftype);
 divide_instancelist_according_to_split(&(node->instances), &leftinstances, &rightinstances, node->featureno, node->split);
 if (leftinstances == NULL || rightinstances == NULL)
  {
   make_leaf_regression_node(node, param->leaftype);
   node->instances = restore_instancelist(node->instances, leftinstances);
   merge_instancelist(&(node->instances), rightinstances);
   return 1;
  }
 node->instances = restore_instancelist(node->instances, leftinstances);
 merge_instancelist(&(node->instances), rightinstances);
 if (!(make_regression_children(node, param->leaftype)))
   return 1;
 create_svmregtree(node->left, param);
 create_svmregtree(node->right, param);
 return 1;
}
