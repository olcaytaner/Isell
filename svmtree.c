#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "svmkernel.h"
#include "svmprepare.h"
#include "svmtree.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

int support_vector_length(Svm_nodeptr sv)
{
 int i = 0;
 while (sv[i].index != -1)
   i++;
 return i + 1;
}

Svm_split find_best_svm_split_for_partition(Instanceptr data, Partition p)
{
 /*!Last Changed 18.02.2008*/
 int i, j, size;
 Svm_parameter param;
 Svm_problem prob;
 Svm_modelptr model;
 Svm_split result;
 prepare_svm_problem(&prob, TWO_CLASS, data, p);
 prepare_svm_parameters(&param, TWO_CLASS);
 model = svm_train(prob, param, BOOLEAN_FALSE);
 result.l = model->l;
 result.nSV = model->nSV[0] + model->nSV[1];
 result.param = param;
 result.rho = model->rho[0];
 result.sv_coef = safemalloc(model->l * sizeof(double), "find_best_svm_split_for_partition", 13);
 memcpy(result.sv_coef, model->sv_coef[0], model->l * sizeof(double));
 result.SV = safemalloc(model->l * sizeof(Svm_nodeptr), "find_best_svm_split_for_partition", 15);
 result.weights = safecalloc(current_dataset->multifeaturecount - 1, sizeof(double), "find_best_svm_split_for_partition", 16); 
 for (i = 0; i < model->l; i++)
  {
   size = support_vector_length(model->SV[i]);
   result.SV[i] = safemalloc(sizeof(Svm_node) * size, "find_best_svm_split_for_partition", 19);
   for (j = 0; j < size - 1; j++)
    {
     result.SV[i][j].index = model->SV[i][j].index;
     result.SV[i][j].value = model->SV[i][j].value;
     result.weights[result.SV[i][j].index] += result.sv_coef[i] * result.SV[i][j].value;
    }
   result.SV[i][j].index = model->SV[i][j].index;
   result.SV[i][j].value = model->SV[i][j].value;
  }
 free_svm_model(model);
 free_svm_problem(prob);
 return result;
}

int find_best_svmtree_feature(Instanceptr instances, double* split, int positive, double C)
{
 /*!Last Changed 03.07.2009*/
 Instanceptr tmp;
 int counts[2][MAXCLASSES];
 int i, j, t, count = data_size(instances), bestfeature = -1, negativecount, positivecount, splitcount;
 double* splits = safemalloc(count * sizeof(double), "find_best_svmtree_feature", 2);
 Splitpointptr splitpoints = safemalloc(count * sizeof(Splitpoint), "find_best_svmtree_feature", 3);
 double fvalue, bestvalue, bestfsplit, besterror = +INT_MAX, error;
 for (i = 0; i < current_dataset->featurecount; i++)
   if (current_dataset->features[i].type != CLASSDEF)
    {
     for (j = 0, tmp = instances; j < count; j++, tmp = tmp->next)
      {
       if (give_classno(tmp) == positive)
         t = -1;
       else
         t = +1;
       splits[j] = (C - t * tmp->values[i].floatvalue) / t;
      }
     qsort(splits, count, sizeof(double), sort_function_double_ascending);
     if (splits[0] == splits[count - 1])
       continue;
     splitcount = 1;
     splitpoints[0].x = splits[0];
     splitpoints[0].count = 1;
     for (j = 1; j < count; j++)
       if (splitpoints[splitcount - 1].x != splits[j])
        {
         splitpoints[splitcount].x = splits[j];
         splitpoints[splitcount].count = 1;
         splitcount++;
        }
       else
         splitpoints[splitcount - 1].count++;
     bestvalue = +INT_MAX;
     fvalue = 0.0;
     negativecount = 0;
     positivecount = 0;
     for (j = splitcount - 1; j > 0; j--)
       if (splitpoints[j].x > 0)
        {
         positivecount += splitpoints[j].count;
         fvalue += (splitpoints[j].x - splitpoints[0].x) * splitpoints[j].count;
        }
       else
         break;
     for (j = 0; j < splitcount - 1; j++)
      {
       if (fvalue < bestvalue)
        {
         bestvalue = fvalue;
         bestfsplit = -(splitpoints[j].x + splitpoints[j + 1].x) / 2;
        }
       if (splitpoints[j].x < 0)
         negativecount += splitpoints[j].count;
       else
         positivecount -= splitpoints[j].count;
       fvalue += (negativecount - positivecount) * (splitpoints[j + 1].x - splitpoints[j].x);
      }
     find_counts_for_split(instances, i, bestfsplit, counts[0], counts[1], UNIVARIATE_CONDITION);
     error = error_of_split(counts, count);
     if (error < besterror)
      {
       bestfeature = i;
       *split = bestfsplit;
       besterror = error;
      }
    }
 safe_free(splits);
 safe_free(splitpoints);
 return bestfeature;
}

int optimize_C_for_svmtreenode(Instanceptr* traindata, int positive, double* split)
{
 /*!Last Changed 22.07.2009*/
 int bestfeature = -1, currentfeature, count;
 int counts[2][MAXCLASSES];
 double C, currenterror, besterror = +INT_MAX, currentsplit;
 Instanceptr validate = NULL, learn = NULL;
 divide_instancelist(traindata, &validate, &learn, 4);
 count = data_size(validate);
 for (C = -2.0; C <= 2.0; C+= 0.2)
  {
   currentfeature = find_best_svmtree_feature(learn, &currentsplit, positive, C);
   find_counts_for_split(validate, currentfeature, currentsplit, counts[0], counts[1], UNIVARIATE_CONDITION);
   currenterror = error_of_split(counts, count);
   if (currenterror < besterror)
    {
     besterror = currenterror;
     bestfeature = currentfeature;
     *split = currentsplit;
    }
  }
 *traindata = learn;
 merge_instancelist(traindata, validate);
 return bestfeature;
}

int create_svmtreechildren(Decisionnodeptr node, Svmtree_parameterptr p, int positive)
{
 /*!Last Changed 03.07.2009*/
 int counts[2][MAXCLASSES], count, i;
 node->conditiontype = UNIVARIATE_CONDITION;
 if (!(can_be_divided(node, &(p->c45param))))
   return 1;
 node->featureno = optimize_C_for_svmtreenode(&(node->instances), positive, &(node->split));
 find_counts_for_split(node->instances, node->featureno, node->split, counts[0], counts[1], node->conditiontype);
 for (i = 0; i < 2; i++)
  {
   count = sum_of_list(counts[i], current_dataset->classno);
   if (count == 0)
    {
     make_leaf_node(node);
     return 1;
    }
  }
 if (!(make_children(node)))
   return 1;
 create_svmtreechildren(node->left, p, positive);
 create_svmtreechildren(node->right, p, positive);
 return 1;
}
