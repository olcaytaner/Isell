#include <limits.h>
#include <string.h>
#include "decisiontree.h"
#include "ktree.h"
#include "libmath.h"
#include "libmemory.h"
#include "libiteration.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

Korderingsplit copy_of_korderingsplit(Korderingsplit ksplit)
{
	int i, size;
	Korderingsplit newsplit;
	newsplit.featurecount = ksplit.featurecount;
	newsplit.discretesplit = safemalloc(newsplit.featurecount * sizeof(int), "copy_of_korderingsplit", 4);
	newsplit.featurelist = safemalloc(newsplit.featurecount * sizeof(int), "copy_of_korderingsplit", 5);
	newsplit.sortorder = safemalloc(newsplit.featurecount * sizeof(int*), "copy_of_korderingsplit", 6);
	memcpy(newsplit.discretesplit, ksplit.discretesplit, newsplit.featurecount * sizeof(int));
	memcpy(newsplit.featurelist, ksplit.featurelist, newsplit.featurecount * sizeof(int));
	for (i = 0; i < newsplit.featurecount; i++)
  {
   size = current_dataset->features[newsplit.featurelist[i]].valuecount;
   newsplit.sortorder[i] = safemalloc(size * sizeof(int), "copy_of_korderingsplit", 12);
   memcpy(newsplit.sortorder[i], ksplit.sortorder[i], size * sizeof(int));
  }
	return newsplit;
}

/**
 * Constructor for the univariate condition with K-ordering split
 * @param[in] ksplit K-ordering split
 * @return A univariate condition initialized
 */
Decisioncondition create_kordering_condition(Korderingsplit ksplit, char comparison)
{
 Decisioncondition rule;
 rule.featureindex = DISCRETEMIX;
 rule.ksplit = copy_of_korderingsplit(ksplit);
	rule.comparison = comparison;
 return rule;
}

void find_best_discrete_split_1(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain)
{
 int* perm[1];
 perm[0] = first_permutation(rangelist[0]);
 do{
   update_best_discrete_split(ksplit, perm, rangelist, iter, featurecounts, nodecounts, bestgain);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

void find_best_discrete_split_2(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain)
{
 int* perm[2];
 perm[0] = first_permutation(rangelist[0]);
 do{
   perm[1] = first_permutation(rangelist[1]);
   do{
     update_best_discrete_split(ksplit, perm, rangelist, iter, featurecounts, nodecounts, bestgain);
   }while (next_permutation(perm[1], rangelist[1]));
   safe_free(perm[1]);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

void find_best_discrete_split_3(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain)
{
 int* perm[3];
 perm[0] = first_permutation(rangelist[0]);
 do{
   perm[1] = first_permutation(rangelist[1]);
   do{
     perm[2] = first_permutation(rangelist[2]);
     do{
       update_best_discrete_split(ksplit, perm, rangelist, iter, featurecounts, nodecounts, bestgain);
     }while (next_permutation(perm[2], rangelist[2]));
     safe_free(perm[2]);
   }while (next_permutation(perm[1], rangelist[1]));
   safe_free(perm[1]);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

void find_best_discrete_split_4(Korderingsplit* ksplit, int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain)
{
 int* perm[4];
 perm[0] = first_permutation(rangelist[0]);
 do{
   perm[1] = first_permutation(rangelist[1]);
   do{
     perm[2] = first_permutation(rangelist[2]);
     do{
       perm[3] = first_permutation(rangelist[3]);
       do{
         update_best_discrete_split(ksplit, perm, rangelist, iter, featurecounts, nodecounts, bestgain);
       }while (next_permutation(perm[3], rangelist[3]));
       safe_free(perm[3]);
     }while (next_permutation(perm[2], rangelist[2]));
     safe_free(perm[2]);
   }while (next_permutation(perm[1], rangelist[1]));
   safe_free(perm[1]);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

Korderingsplit find_best_discrete_split(Decisionnodeptr node, int depth, double* gain)
{
 int* iter;
 int* rangelist;
 int** featurecounts;
 int permsize;
 int i, j, anysame;
 Korderingsplit ksplit;
 *gain = +INT_MAX;
 iter = first_iterator(depth);
 ksplit.featurecount = depth;
 ksplit.sortorder = safemalloc(depth * sizeof(int*), "find_best_discrete_split", 6);
 for (i = 0; i < depth; i++)
   ksplit.sortorder[i] = safemalloc(sizeof(int), "find_best_discrete_split", 6);
 ksplit.featurelist = safemalloc(depth * sizeof(int), "find_best_discrete_split", 7);
 ksplit.discretesplit = safemalloc(depth * sizeof(int), "find_best_discrete_split", 8);
 rangelist = safemalloc(depth * sizeof(int), "find_best_discrete_split", 9);
 do{
   anysame = NO;
   for (i = 0; i < depth; i++)
     for (j = i + 1; j < depth; j++)
       if (iter[i] == iter[j])
         anysame = YES;
   for (i = 0; i < depth; i++)
     if (iter[i] == current_dataset->classdefno)
       anysame = YES;
   if (anysame)
     continue;
   featurecounts = count_feature_combinations(node->instances, iter, depth, &permsize);
   for (i = 0; i < depth; i++)
     rangelist[i] = current_dataset->features[iter[i]].valuecount;
   switch (depth)
    {
     case 1:find_best_discrete_split_1(&ksplit, rangelist, iter, featurecounts, node->counts, gain);
            break;
     case 2:find_best_discrete_split_2(&ksplit, rangelist, iter, featurecounts, node->counts, gain);
            break;
     case 3:find_best_discrete_split_3(&ksplit, rangelist, iter, featurecounts, node->counts, gain);
            break;
     case 4:find_best_discrete_split_4(&ksplit, rangelist, iter, featurecounts, node->counts, gain);
            break;
     default:find_best_discrete_split_1(&ksplit, rangelist, iter, featurecounts, node->counts, gain);
             break;
    }
   free_2d((void**) featurecounts, permsize);
 }while (next_iterator(iter, depth, current_dataset->featurecount - 1));
 safe_free(iter);
 safe_free(rangelist);
 if (*gain == ISELL)
   ksplit.featurecount = 0;
 return ksplit;
}

void update_best_discrete_split(Korderingsplit* split, int* perm[], int* rangelist, int* iter, int** featurecounts, int* nodecounts, double* bestgain)
{
	int i, j, size = split->featurecount, permpos, permmultiplier;
 int* mixediter;
 int counts[2][MAXCLASSES];
	double gain;
 for (i = 0; i < MAXCLASSES; i++)
   counts[0][i] = counts[1][i] = 0;
 for (i = 0; i < current_dataset->classno; i++)
   counts[1][i] = nodecounts[i];
 mixediter = first_mixed_iterator(size);
 do{
   permpos = 0;
   permmultiplier = 1;
   for (i = 0; i < size; i++)
    {
     permpos += permmultiplier * perm[i][mixediter[i]];
     permmultiplier *= rangelist[i];
    }
   for (i = 0; i < current_dataset->classno; i++)
    {
     counts[0][i] += featurecounts[permpos][i];
     counts[1][i] -= featurecounts[permpos][i];
    }
   gain = information_gain(counts);
   if (dless(gain, *bestgain))
    {
     *bestgain = gain;
     for (i = 0; i < size; i++)
      {
       split->featurelist[i] = iter[i];
       split->discretesplit[i] = mixediter[i];
       safe_free(split->sortorder[i]);
       split->sortorder[i] = safemalloc(rangelist[i] * sizeof(int), "update_best_discrete_split", 70);
       for (j = 0; j < rangelist[i]; j++)
         split->sortorder[i][j] = perm[i][j];
      }
    }
 }while (next_mixed_iterator(mixediter, size, rangelist));
 safe_free(mixediter);
}

int make_children_discrete(Decisionnodeptr node)
{
 Instanceptr inst, leftbefore = NULL, rightbefore = NULL;
 Decisionnodeptr left, right;
 Decisioncondition ruleleft, ruleright;
 if (node->featureno == LEAF_NODE || node->instances == NULL)
  {
   make_leaf_node(node);
   return 0;
  }
 left = createnewnode(node, 1);
 right = createnewnode(node, 1);
 node->string = NULL;
 node->left = left;
 node->right = right;
 ruleleft = create_kordering_condition(node->ksplit, '<');
 ruleleft.conditiontype = UNIVARIATE_CONDITION;
 addcondition(node->left, ruleleft);
 ruleright = create_kordering_condition(node->ksplit, '>');
 ruleright.conditiontype = UNIVARIATE_CONDITION;
 addcondition(node->right, ruleright);
 inst = node->instances;
 while (inst)
  {
   if (is_smaller_discrete_order(inst, node->ksplit))
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

int create_ktreechildren(Decisionnodeptr node, C45_parameterptr p)
{
 int i, bestdepth;
 double gain, bestgain = +INT_MAX;
 Korderingsplit ksplit, bestksplit;
 node->conditiontype = UNIVARIATE_CONDITION;
 if (!(can_be_divided(node, p)))
   return 1;
 node->featureno = DISCRETEMIX;
 node->ksplit.featurecount = get_parameter_i("smallsetsize");
 if (node->ksplit.featurecount == 0)
  {
   for (i = 1; i <= get_parameter_i("hybridspace"); i++)
    {
     ksplit = find_best_discrete_split(node, i, &gain);
     if (gain < bestgain)
      {
       bestgain = gain;
       bestdepth = i;
       bestksplit = ksplit;
      }
    }
   node->ksplit = bestksplit;
  }
 else
   node->ksplit = find_best_discrete_split(node, node->ksplit.featurecount, &gain);
 if (node->ksplit.featurecount == 0)
  {
   safe_free(node->ksplit.sortorder);
   safe_free(node->ksplit.featurelist);
   safe_free(node->ksplit.discretesplit);
   node->featureno = LEAF_NODE;
   return 1;
  }
 if (!(make_children_discrete(node)))
   return 1;
 create_ktreechildren(node->left, p);
 create_ktreechildren(node->right, p);
 return 1;
}

