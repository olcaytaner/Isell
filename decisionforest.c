#include <limits.h>
#include "data.h"
#include "decisionforest.h"
#include "decisiontree.h"
#include "instancelist.h"
#include "ktree.h"
#include "libmemory.h"
#include "librandom.h"
#include "parameter.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

int create_kforestchildren(Decisionnodeptr node, Kforest_parameterptr p)
{
 int iteration, j, t, fno, permsize;
 double bestgain = ISELL;
 int** perm;
 int* array;
 int* iter;
 int* rangelist;
 int** featurecounts;
 Korderingsplit ksplit;
 node->conditiontype = UNIVARIATE_CONDITION;
 node->featureno = DISCRETEMIX;
 if (!(setup_node_properties(node)))
  {
   make_leaf_node(node);
   return 1;
  }
 iteration = 0;
 ksplit.featurecount = p->k;
 ksplit.sortorder = safemalloc(p->k * sizeof(int*), "create_kforestchildren", 20);
 for (j = 0; j < p->k; j++)
   ksplit.sortorder[j] = safemalloc(sizeof(int), "create_kforestchildren", 22);
 ksplit.featurelist = safemalloc(p->k * sizeof(int), "create_kforestchildren", 23);
 ksplit.discretesplit = safemalloc(p->k * sizeof(int), "create_kforestchildren", 24);
 perm = safemalloc(p->k * sizeof(int*), "create_kforestchildren", 25);
 rangelist = safemalloc(p->k * sizeof(int), "create_kforestchildren", 26);
 iter = safemalloc(p->k * sizeof(int), "create_kforestchildren", 27);
 while (iteration < p->featuresize)
  {
   array = random_array(current_dataset->featurecount);
   j = 0;
   t = 0;
   while (j < p->k)
    {
     fno = array[t];
     if (current_dataset->features[fno].type != CLASSDEF)
      {
       iter[j] = fno;
       j++;
      }
     t++;
    }
   safe_free(array);
   featurecounts = count_feature_combinations(node->instances, iter, p->k, &permsize);
   for (j = 0; j < p->k; j++)
    {
     rangelist[j] = current_dataset->features[iter[j]].valuecount;
     perm[j] = random_array(rangelist[j]);
    }
   update_best_discrete_split(&ksplit, perm, rangelist, iter, featurecounts, node->counts, &bestgain);
   iteration++;
   for (j = 0; j < p->k; j++)
     safe_free(perm[j]);
   free_2d((void**) featurecounts, permsize);
  }
 safe_free(perm);
 safe_free(iter);
 safe_free(rangelist);
 if (bestgain == ISELL)
  {
   for (j = 0; j < p->k; j++)
     safe_free(ksplit.sortorder[j]);
   safe_free(ksplit.sortorder);
   safe_free(ksplit.discretesplit);
   safe_free(ksplit.featurelist);
   node->featureno = LEAF_NODE;
   return 1;
  }
 else
   node->ksplit = ksplit;
 if (!(make_children_discrete(node)))
   return 1;
 create_kforestchildren(node->left, p);
 create_kforestchildren(node->right, p);
 return 1;
}

int create_randomforestchildren(Decisionnodeptr node, Randomforest_parameterptr p)
{
 int count, i, t, fno;
 int* array;
 double bestgain = +INT_MAX;
 node->conditiontype = UNIVARIATE_CONDITION;
 if (!(setup_node_properties(node)))
  {
   make_leaf_node(node);
   return 1;
  }
 i = 0;
 t = 0;
 array = random_array(current_dataset->featurecount);
 while (i < p->featuresize)
  {
   fno = array[t];
   if (current_dataset->features[fno].type != CLASSDEF)
    {
     if (find_best_split_for_feature_i(node->instances, fno, &(node->split), &bestgain))
       node->featureno = fno;
     i++;
    }
   t++;
  }
 safe_free(array);
 if (bestgain == ISELL)
   node->featureno = -1;
 if (!(make_children(node)))
   return 1;
 switch (current_dataset->features[node->featureno].type)
  {
   case INTEGER:
   case REAL   :create_randomforestchildren(node->left, p);
                create_randomforestchildren(node->right, p);
                break;
   case STRING :count = current_dataset->features[node->featureno].valuecount;
                for (i = 0; i < count; i++)
                  create_randomforestchildren(&(node->string[i]), p);
                break;
  }
 return 1;
}
