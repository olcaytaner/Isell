#include "data.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "krule.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "rule.h"

extern Datasetptr current_dataset;

int** count_feature_combinations_one_vs_rest(Instanceptr instances, int* iter, int size, int* permsize, int positive)
{
 int i, classno, permpos, permmultiplier;
 int** featurecounts;
 Instanceptr tmp = instances;
 (*permsize) = 1;
 for (i = 0; i < size; i++)
   (*permsize) *= current_dataset->features[iter[i]].valuecount;
 featurecounts = (int**) safecalloc_2d(sizeof(int*), sizeof(int), *permsize, 2, "count_feature_combinations", 7);
 while (tmp != NULL)
  {
   classno = give_classno(tmp);
   permpos = 0;
   permmultiplier = 1;
   for (i = 0; i < size; i++)
    {
     permpos += permmultiplier * tmp->values[iter[i]].stringindex;
     permmultiplier *= current_dataset->features[iter[i]].valuecount;
    }
   if (classno == positive)
     featurecounts[permpos][0]++;
   else
     featurecounts[permpos][1]++;
   tmp = tmp->next;
  }
 return featurecounts;
}

void update_best_discrete_condition(Korderingsplit* split, int* perm[], int* rangelist, int* iter, int** featurecounts, int partitioncounts[], double* bestgain)
{
	int i, j, size = split->featurecount, permpos, permmultiplier;
 int* mixediter;
 int counts[2][2];
	double gain, it, itdot;
 counts[COVERED][POSITIVE_CLASS] = 0;
 counts[COVERED][NEGATIVE_CLASS] = 0;
 counts[UNCOVERED][POSITIVE_CLASS] = partitioncounts[POSITIVE_CLASS];
 counts[UNCOVERED][NEGATIVE_CLASS] = partitioncounts[NEGATIVE_CLASS];
 mixediter = first_mixed_iterator(size);
 it = information(partitioncounts[POSITIVE_CLASS], partitioncounts[POSITIVE_CLASS] + partitioncounts[NEGATIVE_CLASS]);
 do{
   permpos = 0;
   permmultiplier = 1;
   for (i = 0; i < size; i++)
    {
     permpos += permmultiplier * perm[i][mixediter[i]];
     permmultiplier *= rangelist[i];
    }
   counts[COVERED][POSITIVE_CLASS] += featurecounts[permpos][POSITIVE_CLASS];
   counts[COVERED][NEGATIVE_CLASS] += featurecounts[permpos][NEGATIVE_CLASS];
   counts[UNCOVERED][POSITIVE_CLASS] -= featurecounts[permpos][POSITIVE_CLASS];
   counts[UNCOVERED][NEGATIVE_CLASS] -= featurecounts[permpos][NEGATIVE_CLASS];
   itdot = information(counts[COVERED][POSITIVE_CLASS], counts[COVERED][POSITIVE_CLASS] + counts[COVERED][NEGATIVE_CLASS]);
   gain = counts[COVERED][POSITIVE_CLASS] * (it - itdot);
   if (dless(*bestgain, gain))
    {
     *bestgain = gain;
     for (i = 0; i < size; i++)
      {
       split->featurelist[i] = iter[i];
       split->discretesplit[i] = mixediter[i];
       split->sortorder[i] = safemalloc(rangelist[i] * sizeof(int), "find_best_discrete_split", 70);
       for (j = 0; j < rangelist[i]; j++)
         split->sortorder[i][j] = perm[i][j];
      }
    }
 }while (next_mixed_iterator(mixediter, size, rangelist));
 safe_free(mixediter);
}

void find_best_discrete_condition_1(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain)
{
 int* perm[1];
 perm[0] = first_permutation(rangelist[0]);
 do{
   update_best_discrete_condition(split, perm, rangelist, iter, featurecounts, partitioncounts, bestgain);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

void find_best_discrete_condition_2(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain)
{
 int* perm[2];
 perm[0] = first_permutation(rangelist[0]);
 do{
   perm[1] = first_permutation(rangelist[1]);
   do{
     update_best_discrete_condition(split, perm, rangelist, iter, featurecounts, partitioncounts, bestgain);
   }while (next_permutation(perm[1], rangelist[1]));
   safe_free(perm[1]);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

void find_best_discrete_condition_3(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain)
{
 int* perm[3];
 perm[0] = first_permutation(rangelist[0]);
 do{
   perm[1] = first_permutation(rangelist[1]);
   do{
     perm[2] = first_permutation(rangelist[2]);
     do{
       update_best_discrete_condition(split, perm, rangelist, iter, featurecounts, partitioncounts, bestgain);
     }while (next_permutation(perm[2], rangelist[2]));
     safe_free(perm[2]);
   }while (next_permutation(perm[1], rangelist[1]));
   safe_free(perm[1]);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

void find_best_discrete_condition_4(Korderingsplit* split, int* rangelist, int* iter, int** featurecounts, int partitioncounts[], int positive, double* bestgain)
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
         update_best_discrete_condition(split, perm, rangelist, iter, featurecounts, partitioncounts, bestgain);
       }while (next_permutation(perm[3], rangelist[3]));
       safe_free(perm[3]);
     }while (next_permutation(perm[2], rangelist[2]));
     safe_free(perm[2]);
   }while (next_permutation(perm[1], rangelist[1]));
   safe_free(perm[1]);
 }while (next_permutation(perm[0], rangelist[0]));
 safe_free(perm[0]);
}

Decisionconditionptr find_best_discrete_condition_omnivariate(Instanceptr growset, int positive)
{
 double bestgain = 0.0, gain;
 Decisionconditionptr* conditions;
 Decisionconditionptr result;
 int bestdepth = -1, depth, modelcount = get_parameter_i("hybridspace");
 conditions = safemalloc(modelcount * sizeof(Decisionconditionptr), "find_best_discrete_condition_omnivariate", 4);
 for (depth = 0; depth < modelcount; depth++)
  {
   conditions[depth] = find_best_discrete_condition(growset, depth + 1, positive, &gain);
   if (gain > bestgain)
    {
     bestgain = gain;
     bestdepth = depth;
    }
  }
 for (depth = 0; depth < modelcount; depth++)
   if (depth != bestdepth)
    {
     free_condition(*(conditions[depth]));
     safe_free(conditions[depth]);
    }
 if (bestdepth != -1)
   result = conditions[bestdepth];
 else
   result = NULL;
 safe_free(conditions);
 return result;
}

Decisionconditionptr find_best_discrete_condition(Instanceptr growset, int depth, int positive, double* bestgain)
{
 int* iter;
 int* rangelist;
 int counts[2];
 int** featurecounts;
 int permsize;
 int i, j, anysame;
 Decisionconditionptr result;
 result = empty_condition();
 result->conditiontype = UNIVARIATE_CONDITION;
 result->featureindex = DISCRETEMIX;
 result->freeparam = 1;
 result->comparison = '<';
 *bestgain = 0.0;
 iter = first_iterator(depth);
 result->ksplit.featurecount = depth;
 result->ksplit.sortorder = safemalloc(depth * sizeof(int*), "find_best_discrete_split", 6);
 result->ksplit.featurelist = safemalloc(depth * sizeof(int), "find_best_discrete_split", 7);
 result->ksplit.discretesplit = safemalloc(depth * sizeof(int), "find_best_discrete_split", 8);
 rangelist = safemalloc(depth * sizeof(int), "find_best_discrete_split", 9);
 counts[POSITIVE_CLASS] = find_class_count(growset, positive);
 counts[NEGATIVE_CLASS] = data_size(growset) - counts[POSITIVE_CLASS];
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
   featurecounts = count_feature_combinations_one_vs_rest(growset, iter, depth, &permsize, positive);
   for (i = 0; i < depth; i++)
     rangelist[i] = current_dataset->features[iter[i]].valuecount;
   switch (depth)
    {
     case 1:find_best_discrete_condition_1(&(result->ksplit), rangelist, iter, featurecounts, counts, positive, bestgain);
            break;
     case 2:find_best_discrete_condition_2(&(result->ksplit), rangelist, iter, featurecounts, counts, positive, bestgain);
            break;
     case 3:find_best_discrete_condition_3(&(result->ksplit), rangelist, iter, featurecounts, counts, positive, bestgain);
            break;
     case 4:find_best_discrete_condition_4(&(result->ksplit), rangelist, iter, featurecounts, counts, positive, bestgain);
            break;
     default:find_best_discrete_condition_1(&(result->ksplit), rangelist, iter, featurecounts, counts, positive, bestgain);
             break;
    }
   free_2d((void**) featurecounts, permsize);
 }while (next_iterator(iter, depth, current_dataset->featurecount - 1));
 safe_free(iter);
 safe_free(rangelist);
 if (*bestgain == 0)
   result->featureindex = -1;
 return result;
}
