#include <limits.h>
#include <math.h>
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "statistics.h"
#include "univariatetree.h"

#ifdef MPI
extern int prank;
int Firsttime;
#endif

extern Datasetptr current_dataset;
extern int mustrealize;

/**
 * Recursive function that returns the number of univariate decision nodes
 * @param[in] node Decision tree node
 * @return Number of univariate decision nodes in the subtree rooted this node
 */
int decisiontree_univariate_node_count(Decisionnodeptr node)
{
 /*!Last Changed 24.11.2004*/
 if (node->featureno == LEAF_NODE)
   return 0;
 else
   if (node->featureno != LINEAR && node->featureno != QUADRATIC)
     return 1 + decisiontree_univariate_node_count(node->left) + decisiontree_univariate_node_count(node->right);
   else
     return decisiontree_univariate_node_count(node->left) + decisiontree_univariate_node_count(node->right);
}

/**
 * Constructor for a univariate condition
 * @param[in] featureindex Index of the best feature selected
 * @param[in] comparison Character describing the type of comparison (\< for smaller, \> for larger etc.)
 * @param[in] value Split threshold
 * @return A univariate condition initialized
 */
Decisioncondition createcondition(int featureindex, char comparison, double value)
{
/*!Last Changed 08.04.2003*/
 Decisioncondition rule;
 rule.featureindex = featureindex;
 rule.comparison = comparison;
 rule.value = value;
 return rule;
}

/**
 * Tests a univariate split with the given test data. 
 * @param[in] testingdata Header of the link list containing the test instances
 * @param[in] fno Index of the feature of the univariate split
 * @param[in] split Threshold of the univariate split
 * @param[in] leftclass Correct class for the instances whose feature fno has the value less than the split
 * @param[in] rightclass Correct class for the instances whose feature fno has the value larger than the split
 * @return Number of instances in the test data that are incorrectly classified.
 */
int test_univariate(Instanceptr testingdata, int fno, double split, int leftclass, int rightclass)
{
 /*!Last Changed 23.09.2004*/
 int i, error = 0;
 double value;
 Instanceptr test = testingdata;
 while (test)
  {
   value = real_feature(test, fno);   
   i = give_classno(test);
   if (value <= split)
    {
     if (i != leftclass)
       error++;
    }
   else
    {
     if (i != rightclass)
       error++;
    }
   test = test->next;
  }
 return error;
}

void find_occurences_for_discrete_feature(Instanceptr instances, int fno, int ratio[MAXVALUES][MAXCLASSES])
{
/*!Last Changed 22.04.2003*/
 int i, j;
 Instanceptr tmp = instances;
 for (i = 0; i < MAXVALUES; i++)
   for (j = 0;j < MAXCLASSES; j++)
     ratio[i][j] = 0;
 while (tmp)
  {
   j = give_classno(tmp);
   i = tmp->values[fno].stringindex;
   ratio[i][j]++;
   tmp = tmp->next;
  }
}

double information_gain_for_discrete_feature(Instanceptr instances,int fno)
{
/*!Last Changed 22.03.2002*/
 int ratio[MAXVALUES][MAXCLASSES];
 find_occurences_for_discrete_feature(instances, fno, ratio);
 return information_gain_from_ratios(ratio);
}

int bucketsort_instances(Instanceptr instances, int size, int fno)
{
 /*!Last Changed 26.11.2005 removed bug counts is deallocated before buckets is deallocated*/
 /*!Last Changed 21.05.2005*/
 int **buckets, *counts, *current, bcount, i, j, k, bucketindex, bottom = current_dataset->features[fno].min.intvalue;
 Instanceptr tmplist;
 bcount = current_dataset->features[fno].max.intvalue - bottom + 1;
 counts = safecalloc(bcount, sizeof(int), "bucketsort_instances", 4);
 buckets = safemalloc(bcount * sizeof(int *), "bucketsort_instances", 5);
 tmplist = safemalloc(size * sizeof(Instance), "bucketsort_instances", 6);
 for (i = 0; i < size; i++)
  {
   bucketindex = instances[i].values[fno].intvalue - bottom;
   if (bucketindex >= bcount || bucketindex < 0)
    {
     printf(m1267, fno, instances[i].values[fno].intvalue, bucketindex, current_dataset->features[fno].min.intvalue, current_dataset->features[fno].max.intvalue);
     safe_free(counts);
     safe_free(buckets);
     safe_free(tmplist);
     return 0;
    }
   (counts[bucketindex])++;
  }
 for (i = 0; i < bcount; i++)
   if (counts[i] > 0)
     buckets[i] = safemalloc(counts[i] * sizeof(int), "bucketsort_instances", 14);
   else
     buckets[i] = NULL;
 current = safecalloc(bcount, sizeof(int), "bucketsort_instances", 17);
 for (i = 0; i < size; i++)
  {
   bucketindex = instances[i].values[fno].intvalue - bottom;
   buckets[bucketindex][current[bucketindex]] = i;
   (current[bucketindex])++;
  }
 k = 0;
 for (i = 0; i < bcount; i++)
   for (j = 0; j < counts[i]; j++)
    {
     tmplist[k] = instances[buckets[i][j]];
     k++;
    }
 for (k = 0; k < size; k++)
   instances[k] = tmplist[k];
 safe_free(current);
 safe_free(tmplist);
 for (i = 0; i < bcount; i++)
    if (counts[i] > 0)
      safe_free(buckets[i]);
 safe_free(counts);
 safe_free(buckets);
 return 1;
}

Instanceptr sort_instances(Instanceptr instances,int fno,int* size)
{
 /*!Last Changed 21.05.2005*/
 /*!Last Changed 02.02.2004*/
 /*!Last Changed 26.01.2003*/
 Instanceptr result, tmp = instances;
 int i, sorted = 1;
 (*size) = data_size(instances);
 result = (Instanceptr)safemalloc((*size)*sizeof(Instance), "sort_instances", 4);
 i = 0;
 while (tmp)
  {
   result[i] = (*tmp);
   if (i > 0)
    {
     if (!mustrealize)
      {
       if (current_dataset->features[fno].type == INTEGER)
        {
         if (result[i - 1].values[fno].intvalue > result[i].values[fno].intvalue)
           sorted = 0;
        }
       else
        {
         if (result[i - 1].values[fno].floatvalue > result[i].values[fno].floatvalue)
           sorted = 0;
        }
      }
     else
      {
       if (real_feature(&(result[i - 1]), fno) > real_feature(&(result[i]), fno))
         sorted = 0;
      }
    }
   i++;   
   tmp = tmp->next;
  }
 if (!sorted)
  {
   if (!mustrealize && current_dataset->features[fno].type == INTEGER && (current_dataset->features[fno].max.intvalue - current_dataset->features[fno].min.intvalue < (*size)))
    {
     if (!bucketsort_instances(result, *size, fno))
       quicksort_instances(result, 0, i - 1, fno);
    }
   else
     quicksort_instances(result, 0, i - 1, fno);  
  }
 return result;
}

void exchange_instances(Instanceptr instances, int i, int j)
{
/*!Last Changed 26.01.2003*/
 Instance tmp;
 tmp = instances[i];
 instances[i] = instances[j];
 instances[j] = tmp;
}

int sort_function_instances(Instanceptr x, Instanceptr y, int fno)
{
 /*!Last Changed 22.05.2005*/
 if (!mustrealize)
   if (current_dataset->features[fno].type == INTEGER)
     if (x->values[fno].intvalue < y->values[fno].intvalue)
       return -1;
     else
       if (x->values[fno].intvalue > y->values[fno].intvalue)
         return 1;
       else
         return 0;
   else
     if (x->values[fno].floatvalue < y->values[fno].floatvalue)
       return -1;
     else
       if (x->values[fno].floatvalue > y->values[fno].floatvalue)
         return 1;
       else
         return 0;
 else
   if (real_feature(x, fno) < real_feature(y, fno))
     return -1;
   else
     if (real_feature(x, fno) > real_feature(y, fno))
       return 1;
     else
       return 0;
}

Instance quicksort_three_elements(Instanceptr instances, int start, int end, int fno)
{
 /*!Last Changed 22.05.2005*/
 int mid = (start + end) / 2;
 if (sort_function_instances(&(instances[mid]), &(instances[start]), fno) < 0)
   exchange_instances(instances, start, mid);
 if (sort_function_instances(&(instances[end]), &(instances[mid]), fno) < 0)
   exchange_instances(instances, end, mid);
 if (sort_function_instances(&(instances[mid]), &(instances[start]), fno) < 0)
   exchange_instances(instances, start, mid);
 exchange_instances(instances, mid, end);
 return instances[end];
}

int quicksort_partition(Instanceptr instances, int start, int end, int fno)
{
 /*!Last Changed 22.01.2004*/
 /*!Last Changed 26.01.2003*/
 Instance x;
 int i = start - 1, j;
 x = quicksort_three_elements(instances, start, end, fno);
 for (j = start; j < end; j++)
   if (sort_function_instances(&(instances[j]), &x, fno) <= 0)
    {
     i++;
     exchange_instances(instances,i,j);
    }
 exchange_instances(instances,i+1,end);
 return i + 1;
}

int quicksort_all_sorted(Instanceptr instances, int start, int end, int fno)
{
 /*!Last Changed 21.05.2005*/
 int i;
 if (current_dataset->features[fno].type == INTEGER)
  {
   for (i = start + 1; i <= end; i++)
     if (instances[i].values[fno].intvalue < instances[i - 1].values[fno].intvalue)
       return 0;
  }
 else
  {
   for (i = start + 1; i <= end; i++)
     if (instances[i].values[fno].floatvalue < instances[i - 1].values[fno].floatvalue)
       return 0;
  }   
 return 1;
}

void quicksort_instances(Instanceptr instances,int start,int end,int fno)
{
 /*!Last Changed 26.01.2003*/
 int pivot;
 if (start < end)
  {
   if (!mustrealize)
     if (quicksort_all_sorted(instances, start, end, fno))
       return;
   pivot = quicksort_partition(instances, start, end, fno);
   quicksort_instances(instances, start, pivot - 1, fno);
   quicksort_instances(instances, pivot + 1, end, fno);
  }
}

double* find_value_list(Instanceptr instances, int fno, int* size)
{
/*!Last Changed 22.03.2002*/
 Instanceptr tmp = instances;
 double* values = NULL, value;
 int i, found;
 (*size) = 0;
 while (tmp)
  {
   found = 0;
   if (current_dataset->features[fno].type == INTEGER)
     value = tmp->values[fno].intvalue;
   else
     value = tmp->values[fno].floatvalue;
   for (i = 0; i < (*size); i++)
     if (values[i] == value)
      {
       found = 1;
       break;
      }
   if (!found)
    {
     (*size)++;
     values = (double *)alloc(values, (*size) * sizeof(double), (*size));
     values[(*size) - 1] = value;
    }
   tmp = tmp->next;
  }
 return values;
}

int find_best_split_for_feature_i(Instanceptr instances, int currentfeature, double* split, double* bestgain)
{
 int j, k, counts[2][MAXCLASSES], size, isbetter = NO;
 Instanceptr sorted;
 double gain, value, valuebefore;
 switch (current_dataset->features[currentfeature].type)
  {
   case REAL   :
   case INTEGER:sorted = sort_instances(instances, currentfeature, &size);
                for (j = 0; j < current_dataset->classno; j++)
                 {
                  counts[0][j]=0;
                  counts[1][j]=0;
                 }
                for (k = 0; k < size; k++)
                 {
                  j = give_classno(&(sorted[k]));
                  counts[0][j]++;
                 }
                k = 0;
                valuebefore = -INT_MAX;
                while (k < size)
                 {
                  if (current_dataset->features[currentfeature].type == INTEGER)
                    value = sorted[k].values[currentfeature].intvalue;
                  else
                    value = sorted[k].values[currentfeature].floatvalue;
                  if (value != valuebefore)
                   {
                    if (valuebefore != -INT_MAX)
                     {
                      gain = information_gain(counts);
                      if (dless(gain, *bestgain))
                       {
                        *bestgain = gain;
                        isbetter = YES;
                        (*split) = (value + valuebefore)/2;
                        if (*bestgain == 0)
                         {
                          safe_free(sorted);
                          return YES;
                         }
                       }
                     }
                    valuebefore = value;
                   }
                  j = give_classno(&(sorted[k]));
                  counts[0][j]--;
                  counts[1][j]++;
                  k++;
                 }
                safe_free(sorted);
                break;
   case STRING :gain = information_gain_for_discrete_feature(instances, currentfeature);
                if (dless(gain, *bestgain))
                 {
                  *bestgain = gain;
                  isbetter = YES;
                  (*split) = 0;
                  if (gain == 0)
                    return YES;
                 }
                break;
  }
 return isbetter;
}

int find_best_feature(Instanceptr instances, double* split)
{
/*!Last Changed 23.03.2002*/
 int i, bestfeature = -1;
 double bestgain = +INT_MAX;
 for (i = 0; i < current_dataset->featurecount; i++)
   if (find_best_split_for_feature_i(instances, i, split, &bestgain))
    {
     bestfeature = i;
     if (bestgain == 0)
       return i;
    }
 if (bestgain == ISELL)
   return -1;
 else
   return bestfeature;
}

int make_children(Decisionnodeptr node)
{
 /*!Last Changed 02.02.2004*/
 /*!Last Changed 08.04.2003*/
 Instanceptr inst,leftbefore=NULL,rightbefore=NULL,*before;
 Decisionnodeptr left,right;
 Decisioncondition ruleleft, ruleright, rule;
 int i,count;
 double value;
 if (node->featureno == LEAF_NODE || node->instances == NULL)
  {
   make_leaf_node(node);
   return 0;
  }
 switch (current_dataset->features[node->featureno].type)
  {
   case INTEGER:
   case REAL   :left = createnewnode(node, 1);
                right = createnewnode(node, 1);
                node->string = NULL;
                node->left = left;
                node->right = right;
                ruleleft = createcondition(node->featureno,'<',node->split);
                ruleleft.conditiontype = UNIVARIATE_CONDITION;
                addcondition(node->left, ruleleft);
                ruleright = createcondition(node->featureno,'>',node->split);
                ruleright.conditiontype = UNIVARIATE_CONDITION;
                addcondition(node->right, ruleright);
                inst=node->instances;
                while (inst)
                 {
                  if (current_dataset->features[node->featureno].type==INTEGER)
                    value=inst->values[node->featureno].intvalue;
                  else
                    value=inst->values[node->featureno].floatvalue;
                  if (value<=node->split)
                   {
                    if (leftbefore)
                      leftbefore->next=inst;
                    else
                      left->instances=inst;
                    leftbefore=inst;
                   }
                  else
                   {
                    if (rightbefore)
                      rightbefore->next=inst;
                    else
                      right->instances=inst;
                    rightbefore=inst;
                   }
                  inst=inst->next;
                 }
                if (leftbefore)
                  leftbefore->next=NULL;
                if (rightbefore)
                  rightbefore->next=NULL;
                node->instances=NULL;
                break;
   case STRING :count=current_dataset->features[node->featureno].valuecount;
                node->string = createnewnode(node, count);                
                before = (Instanceptr*)safemalloc(count*sizeof(Instanceptr), "make_children", 56);
                node->left = NULL;
                node->right = NULL;
                for (i = 0; i < count; i++)
                 {
                  node->string[i].instances = NULL;
                  rule = createcondition(node->featureno, '=', i);
                  rule.conditiontype = UNIVARIATE_CONDITION;
                  addcondition(&(node->string[i]), rule);
                  before[i] = NULL;
                 }
                inst=node->instances;
                while (inst)
                 {
                  i=inst->values[node->featureno].stringindex;
                  if (before[i]==NULL)
                   {
                    node->string[i].instances=inst;
                    before[i]=inst;
                   }
                  else
                   {
                    before[i]->next=inst;
                    before[i]=inst;
                   }
                  inst=inst->next;
                 }
                node->instances=NULL;
                for (i=0;i<count;i++)
                 {
                  if (before[i])
                    before[i]->next=NULL;
                  else
                    node->string[i].instances=NULL;
                 }
                safe_free(before);
                break;
  }
 return 1;
}

int** count_feature_combinations(Instanceptr instances, int* iter, int size, int* permsize)
{
 int i, classno, permpos, permmultiplier;
 int** featurecounts;
 Instanceptr tmp = instances;
 (*permsize) = 1;
 for (i = 0; i < size; i++)
   (*permsize) *= current_dataset->features[iter[i]].valuecount;
 featurecounts = (int**) safecalloc_2d(sizeof(int*), sizeof(int), *permsize, current_dataset->classno, "count_feature_combinations", 7);
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
   featurecounts[permpos][classno]++;
   tmp = tmp->next;
  }
 return featurecounts;
}

int find_discrete_split_count(int size)
{
 int* iter;
 int i, j, sum = 0, anysame, multiplier1, multiplier2;
 iter = first_iterator(size);
 do{
   anysame = NO;
   for (i = 0; i < size; i++)
     for (j = i + 1; j < size; j++)
       if (iter[i] == iter[j])
         anysame = YES;
   for (i = 0; i < size; i++)
     if (iter[i] == current_dataset->classdefno)
       anysame = YES;
   if (anysame)
     continue;
   multiplier1 = 1;
   for (i = 0; i < size; i++)
     multiplier1 *= factorial(current_dataset->features[iter[i]].valuecount); 
   multiplier2 = 1;
   for (i = 0; i < size; i++)
     multiplier2 *= current_dataset->features[iter[i]].valuecount; 
   sum += multiplier1 * (multiplier2 - 1);
 }while (next_iterator(iter, size, current_dataset->featurecount - 1));
 safe_free(iter); 
 return sum; 
}

int create_c45children(Decisionnodeptr node, C45_parameterptr p)
{
 /*!Last Changed 10.08.2005 parametre eklendi*/
 /*!Last Changed 23.01.2003*/
 int count, i;
 node->conditiontype = UNIVARIATE_CONDITION;
 if (!(can_be_divided(node, p)))
   return 1;
 node->featureno = find_best_feature(node->instances,&(node->split));
 if (!(make_children(node)))
   return 1;
 switch (current_dataset->features[node->featureno].type)
  {
   case INTEGER:
   case REAL   :create_c45children(node->left, p);
                create_c45children(node->right, p);
                break;
   case STRING :count = current_dataset->features[node->featureno].valuecount;
                for (i = 0; i < count; i++)
                  create_c45children(&(node->string[i]), p);
                break;
  }
 return 1;
}

double information_gain_for_split(Instanceptr instances, int fno, double split, int conditiontype)
{
/*!Last Changed 16.04.2003
 find_counts_for_split(instances, fno, split, counts[0], counts[1]); satiri ekledim*/ 
/*!Last Changed 26.01.2003*/
 int counts[2][MAXCLASSES];
 find_counts_for_split(instances, fno, split, counts[0], counts[1], conditiontype);
 return information_gain(counts);
}

void left_and_right_sum(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, int* leftcount, int* rightcount, int conditiontype)
{
/* Last Changed 09.02.2003*/
 int *classassignments, i;
 double value;
 Instanceptr tmp;
 *leftsum = 0;
 *rightsum = 0;
 *leftcount = 0;
 *rightcount = 0;
 if (fno < 0 || instances == NULL || p.count == 0 || all_same_side(p))
   return;
 classassignments = find_class_assignments(p);
 tmp = instances;
 while (tmp)
  {
   if (conditiontype == HYBRID_CONDITION)
     value = real_feature(tmp, fno);
   else
     if (current_dataset->features[fno].type == INTEGER)
       value = tmp->values[fno].intvalue;
     else
       value = tmp->values[fno].floatvalue;
   i = give_classno(tmp);
   if (classassignments[i]==LEFT)
    {
     (*leftcount)++;
     (*leftsum) += value;
    }
   else
    {
     (*rightcount)++;
     (*rightsum) += value;
    }
   tmp = tmp->next;
  }
 safe_free(classassignments);
}

void left_and_right_sum_without_outliers(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, int* leftcount, int* rightcount, double llower, double lupper, double rlower, double rupper)
{
/* Last Changed 30.12.2003*/
 int *classassignments, i;
 double value;
 Instanceptr tmp;
 *leftsum = 0;
 *rightsum = 0;
 *leftcount = 0;
 *rightcount = 0;
 if (fno < 0 || instances == NULL || p.count == 0 || all_same_side(p))
   return;
 classassignments = find_class_assignments(p);
 tmp = instances;
 while (tmp)
  {
   if (current_dataset->features[fno].type == INTEGER)
     value = tmp->values[fno].intvalue;
   else
     value = tmp->values[fno].floatvalue;
   i = give_classno(tmp);
   if (classassignments[i]==LEFT && value >= llower && value <= lupper)
    {
     (*leftcount)++;
     (*leftsum) += value;
    }
   else
     if (value >= rlower && value <= rupper)
      {
       (*rightcount)++;
       (*rightsum) += value;
      }
   tmp = tmp->next;
  }
 safe_free(classassignments);
}

void left_and_right_variance_without_outliers(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, double leftmean, double rightmean, double llower, double lupper, double rlower, double rupper)
{
/* Last Changed 30.12.2003*/
 int *classassignments, i;
 double value, sub, added;
 Instanceptr tmp;
 *leftsum = 0;
 *rightsum = 0;
 if (fno < 0 || instances == NULL || p.count == 0 || all_same_side(p))
   return;
 classassignments = find_class_assignments(p);
 tmp = instances;
 while (tmp)
  {
   if (current_dataset->features[fno].type == INTEGER)
     value = tmp->values[fno].intvalue;
   else
     value = tmp->values[fno].floatvalue;
   i = give_classno(tmp);
   if (classassignments[i] == LEFT && value >= llower && value <= lupper)
    {
     sub = value - leftmean;
     added = sub * sub;
     (*leftsum) += added;
    }
   else
     if (value >= rlower && value <= rupper)
      {
       sub = value - rightmean;
       added = sub * sub;
       (*rightsum) += added;
      }
   tmp = tmp->next;
  }
 safe_free(classassignments);
}

void left_and_right_variance(Instanceptr instances, Partition p, int fno, double* leftsum, double* rightsum, double leftmean, double rightmean, int conditiontype)
{
/* Last Changed 09.02.2003*/
 int *classassignments, i;
 double value, sub, added;
 Instanceptr tmp;
 *leftsum = 0;
 *rightsum = 0;
 if (fno < 0 || instances == NULL || p.count == 0 || all_same_side(p))
   return;
 classassignments = find_class_assignments(p);
 tmp = instances;
 while (tmp)
  {
   if (conditiontype == HYBRID_CONDITION)
     value = real_feature(tmp, fno);
   else
     if (current_dataset->features[fno].type == INTEGER)
       value = tmp->values[fno].intvalue;
     else
       value = tmp->values[fno].floatvalue;
   i = give_classno(tmp);
   if (classassignments[i] == LEFT)
    {
     sub = value - leftmean;
     added = sub * sub;
     (*leftsum) += added;
    }
   else
    {
     sub = value - rightmean;
     added = sub * sub;
     (*rightsum) += added;
    }
   tmp = tmp->next;
  }
 safe_free(classassignments);
}

double solve_for_ldt_second_order_equation(double ml, double mr, double sl, double sr, int nl, int nr)
{
 double result, a, b, c, root1, root2;
 a = sl - sr;
 b = 2 * (ml*sr - mr*sl);
 if (nl > 0 && sr > 0 && nr>0 && sl>0)
   c = mr*mr*sl - ml*ml*sr + 2*sl*sr* log2((nl*sqrt(sr))/(nr*sqrt(sl)));
 else
   c = 0;
 if (b*b - 4*a*c < 0)
   result = (ml + mr)/2;
 else
   if (a == 0)
    {
     if (b != 0)
       result = -c/b;
     else
       result = 0;  
    }
   else
    {
     root1 = (-b + sqrt(b*b - 4*a*c))/(2*a);
     root2 = (-b - sqrt(b*b - 4*a*c))/(2*a);
     if (ml > mr)
      {
       if (root1 > mr && root1 < ml)
         result = root1;
       else
         if (root2 > mr && root2 < ml)
           result = root2;
         else
           result = (ml + mr)/2;
      }
     else
       if (root1 < mr && root1 > ml)
         result = root1;
       else
         if (root2 < mr && root2 > ml)
           result = root2;
         else
           result = (ml + mr)/2;
    }
 return result;
}

double find_split_for_partition_without_outliers(Instanceptr instances, Partition p, int fno, int conditiontype, double variancerange)
{
 /*!Last Changed 30.12.2003*/
 int nl = 0, nr = 0;
 double ml, mr, sr, sl, llower, lupper, rlower, rupper;
 if (p.count == 0 || instances == NULL || fno < 0 || all_same_side(p))
   return ISELL;
 left_and_right_sum(instances, p, fno, &ml, &mr, &nl, &nr, conditiontype);
 if (nl > 0)
   ml /= nl;
 else
   return ISELL;
 if (nr > 0)
   mr /= nr;
 else
   return ISELL;
 left_and_right_variance(instances, p, fno, &sl, &sr, ml, mr, conditiontype);
 if (nl > 1)
   sl /= (nl-1);
 else
   sl = 0;
 if (nr > 1)
   sr /= (nr-1);
 else
   sr = 0;
 llower = ml - variancerange * sqrt(sl);
 lupper = ml + variancerange * sqrt(sl);
 rlower = mr - variancerange * sqrt(sr);
 rupper = mr + variancerange * sqrt(sr);
 left_and_right_sum_without_outliers(instances, p, fno, &ml, &mr, &nl, &nr, llower, lupper, rlower, rupper);
 if (nl > 0)
   ml /= nl;
 else
   return ISELL;
 if (nr > 0)
   mr /= nr;
 else
   return ISELL;
 left_and_right_variance_without_outliers(instances, p, fno, &sl, &sr, ml, mr, llower, lupper, rlower, rupper);
 if (nl > 1)
   sl /= (nl-1);
 else
   sl = 0;
 if (nr > 1)
   sr /= (nr-1);
 else
   sr = 0;
 return solve_for_ldt_second_order_equation(ml, mr, sl, sr, nl, nr);
}

double find_split_for_partition(Instanceptr instances, Partition p, int fno, int conditiontype)
{
/*!Last Changed 29.12.2003*/
 int nl = 0, nr = 0;
 double ml, mr, sr, sl;
 if (p.count == 0 || instances == NULL || fno < 0 || all_same_side(p))
   return ISELL;
 left_and_right_sum(instances, p, fno, &ml, &mr, &nl, &nr, conditiontype);
 if (nl > 0)
   ml /= nl;
 else
   return ISELL;
 if (nr > 0)
   mr /= nr;
 else
   return ISELL;
 left_and_right_variance(instances, p, fno, &sl, &sr, ml, mr, conditiontype);
 if (nl > 1)
   sl /= (nl-1);
 else
   sl = 0;
 if (nr > 1)
   sr /= (nr-1);
 else
   sr = 0;
 return solve_for_ldt_second_order_equation(ml, mr, sl, sr, nl, nr);
}

double find_gain_for_partition(Instanceptr instances, Partition p, int fno, double* split, int conditiontype)
{
 /*!Last Changed 29.12.2003*/
 /*!Last Changed 06.02.2003*/
 (*split) = find_split_for_partition(instances, p, fno, conditiontype);
 return information_gain_for_split(instances, fno, (*split), conditiontype);
}

int find_best_ldt_feature_for_realized(Instanceptr instances, double* split)
{
/*!Last Changed 23.09.2004*/
 int i,j,bestfeature=-1,improved;
 double bestgain=+INT_MAX,gain,bestpartitiongain,currentsplit,bestsplit;
 Partition initial,current,best;
 initial = get_initial_partition(instances);
 for (i = 0 ; i < current_dataset->multifeaturecount - 1;i++)
   if (initial.count <= 2)
    {
     bestpartitiongain = find_gain_for_partition(instances,initial,i,&bestsplit, HYBRID_CONDITION);
     if (dless(bestpartitiongain,bestgain))
      {
       bestgain = bestpartitiongain;
       bestfeature = i;
       (*split) = bestsplit;
       if (bestpartitiongain == 0)
        {
         free_partition(initial);
         return bestfeature;
        }
      }
     continue;
    }
   else
    {
     current = create_copy(initial);
     bestpartitiongain = find_gain_for_partition(instances,current,i,&bestsplit, HYBRID_CONDITION);
     best = create_copy(current);
     improved = 1;
     while (improved)
      {
       improved = 0;
       for (j = 0; j < current.count; j++)
        {
         current.assignments[j] = 1-current.assignments[j];
         gain = find_gain_for_partition(instances,current,i,&currentsplit, HYBRID_CONDITION);
         if (dless(gain,bestpartitiongain))
          {
           bestsplit = currentsplit;
           bestpartitiongain = gain;
           free_partition(best);
           best = create_copy(current);
           improved = 1;
          }
         current.assignments[j]=1-current.assignments[j];
        }
       if (improved)
        {
         free_partition(current);
         current = create_copy(best);
        }
      }
     if (dless(bestpartitiongain,bestgain))
      {
       bestgain = bestpartitiongain;
       bestfeature = i;
       (*split) = bestsplit;
       if (bestgain == 0)
        {
         free_partition(best);
         free_partition(current);
         free_partition(initial);
         return bestfeature;
        }
      }
     free_partition(best);
     free_partition(current);
    }
 free_partition(initial);
 if (bestgain == ISELL)
   return -1;
 else
   return bestfeature;
}

int find_best_ldt_feature(Instanceptr instances,double* split)
{
/*!Last Changed 26.01.2003*/
 int i,j,bestfeature=-1,improved;
 double bestgain=+INT_MAX,gain,bestpartitiongain,currentsplit,bestsplit;
 Partition initial,current,best;
 initial = get_initial_partition(instances);
 for (i=0;i<current_dataset->featurecount;i++)
   switch (current_dataset->features[i].type)
    {
     case REAL   :
     case INTEGER:if (initial.count <= 2)
                   {
                    bestpartitiongain = find_gain_for_partition(instances,initial,i,&bestsplit, UNIVARIATE_CONDITION);
                    if (dless(bestpartitiongain,bestgain))
                     {
                      bestgain = bestpartitiongain;
                      bestfeature = i;
                      (*split) = bestsplit;
                      if (bestpartitiongain == 0)
                       {
                        free_partition(initial);
                        return bestfeature;
                       }
                     }
                    continue;
                   }
                  current = create_copy(initial);
                  bestpartitiongain = find_gain_for_partition(instances,current,i,&bestsplit, UNIVARIATE_CONDITION);
                  best = create_copy(current);
                  improved = 1;
                  while (improved)
                   {
                    improved = 0;
                    for (j = 0; j < current.count; j++)
                     {
                      current.assignments[j] = 1-current.assignments[j];
                      gain = find_gain_for_partition(instances,current,i,&currentsplit, UNIVARIATE_CONDITION);
                      if (dless(gain,bestpartitiongain))
                       {
                        bestsplit = currentsplit;
                        bestpartitiongain = gain;
                        free_partition(best);
                        best = create_copy(current);
                        improved = 1;
                       }
                      current.assignments[j]=1-current.assignments[j];
                     }
                    if (improved)
                     {
                      free_partition(current);
                      current = create_copy(best);
                     }
                   }
                  if (dless(bestpartitiongain,bestgain))
                   {
                    bestgain = bestpartitiongain;
                    bestfeature = i;
                    (*split) = bestsplit;
                    if (bestgain == 0)
                     {
                      free_partition(best);
                      free_partition(current);
                      free_partition(initial);
                      return bestfeature;
                     }
                   }
                  free_partition(best);
                  free_partition(current);
                  break;
     case STRING :gain=information_gain_for_discrete_feature(instances,i);
                  if (dless(gain,bestgain))
                   {
                    bestgain=gain;
                    bestfeature=i;
                    (*split)=0;
                    if (gain==0)
                      return bestfeature;
                   }
                  break;
    }
 free_partition(initial);
 if (bestgain==ISELL)
   return -1;
 else
   return bestfeature;
}

void find_counts_for_split(Instanceptr instances, int fno, double split, int* leftcounts, int* rightcounts, int conditiontype)
{
 /*!Last Changed 23.09.2004 added hybridcondition*/
 /*!Last Changed 09.02.2003*/
 int j;
 double value;
 Instanceptr tmp = instances;
 for (j = 0; j < MAXCLASSES; j++)
  {
   leftcounts[j] = 0;
   rightcounts[j] = 0;
  }
 while (tmp)
  {
   if (conditiontype != HYBRID_CONDITION)
     if (current_dataset->features[fno].type==INTEGER)
       value = tmp->values[fno].intvalue;
     else
       value = tmp->values[fno].floatvalue;
   else
     value = real_feature(tmp, fno);
   j = give_classno(tmp);
   if (value <= split)
     leftcounts[j]++;
   else
     rightcounts[j]++;
   tmp = tmp->next;
  }
}

double log_likelihood_for_uni_ldt_splits(Instanceptr data, int fno, double split, int conditiontype)
{
 /*!Last Changed 22.11.2004*/
 int counts[2][MAXCLASSES];
 double result = 0;
 find_counts_for_split(data, fno, split, counts[0], counts[1], conditiontype);
 result += log_likelihood_for_classification(counts[0]);
 result += log_likelihood_for_classification(counts[1]);
 return result;
}

int create_ldtchildren(Decisionnodeptr node, C45_parameterptr p)
{
/*!Last Changed 23.01.2003*/
 int count, i;
 node->conditiontype = UNIVARIATE_CONDITION;
 if (!(can_be_divided(node, p)))
   return 1;
#ifdef MPI
 if (get_parameter_o("parallel") && get_parameter_i("proccount")>1)
  {
   switch (get_parameter_i("paralleltype"))
    {
     case 0:node->featureno = find_best_ldt_feature_mpi(node->instances,&(node->split));
            break;
     case 1:node->featureno = find_best_ldt_feature_mpi_rulebased(node);
            break;            
     case 2:Firsttime = 1;
            node->featureno = divide_data_to_find_best_feature(node->instances,&(node->split));
            break;
     case 3:create_ldtchildren_iterative(node);
            return 1;
            break;
    }
  }
 else
   node->featureno = find_best_ldt_feature(node->instances,&(node->split));
#else
 node->featureno = find_best_ldt_feature(node->instances,&(node->split)); 
#endif
 if (!(make_children(node)))
   return 1;
 switch (current_dataset->features[node->featureno].type)
  {
   case INTEGER:
   case REAL   :create_ldtchildren(node->left, p);
                create_ldtchildren(node->right, p);
                break;
   case STRING :count = current_dataset->features[node->featureno].valuecount;
                for (i = 0; i < count; i++)
                  create_ldtchildren(&(node->string[i]), p);
                break;
  }
 return 1;
}

#ifdef MPI

MPI_Datatype* create_datatype_for_rule(Decisionconditionptr rule)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 09.04.2003*/
 int block_lengths[3];
 MPI_Aint displacements[3];
 MPI_Datatype typelist[3];
 MPI_Aint start_address;
 MPI_Aint address;
 MPI_Datatype* ruletype = safemalloc(sizeof(MPI_Datatype), "create_datatype_for_rule", 6);
 block_lengths[0] = 1;
 block_lengths[1] = 1;
 block_lengths[2] = 1; 
 typelist[0] = MPI_INT;
 typelist[1] = MPI_CHAR;
 typelist[2] = MPI_DOUBLE;
 displacements[0] = 0;
 MPI_Address(&(rule->featureindex), &start_address);
 MPI_Address(&(rule->comparison), &address);
 displacements[1] = address - start_address;
 MPI_Address(&(rule->value), &address);
 displacements[2] = address - start_address;
 MPI_Type_struct(3, block_lengths, displacements, typelist, ruletype);
 MPI_Type_commit(ruletype);
 return ruletype;
}

double information_gain_for_split_mpi(double split)
{
/*!Last Changed 09.02.2003*/
 int counts[2][MAXCLASSES], i, j, countsleft[MAXCLASSES], countsright[MAXCLASSES];
 MPI_Status status;
 for (i = 0; i < 2; i++)
   for (j = 0; j < MAXCLASSES; j++)
     counts[i][j] = 0;
 for (i = 1; i < get_parameter_i("proccount"); i++)
   MPI_Send(&split,1,MPI_DOUBLE,i,9,MPI_COMM_WORLD);
 for (i = 1; i < get_parameter_i("proccount"); i++)
  {
   MPI_Recv(countsleft,MAXCLASSES,MPI_INT,i,7,MPI_COMM_WORLD,&status);
   for (j = 0; j < MAXCLASSES; j++)
     counts[0][j] += countsleft[j]; 
   MPI_Recv(countsright,MAXCLASSES,MPI_INT,i,8,MPI_COMM_WORLD,&status);
   for (j = 0; j < MAXCLASSES; j++)
     counts[1][j] += countsright[j]; 
  }
 return information_gain(counts);
}

void send_partition_info_to_clients(int* indexlist, int count, int fno, Partition p)
{
/*!Last Changed 23.04.2003*/
 int i, dummy = 1, samedata = -1;
 for (i = 1; i < get_parameter_i("proccount"); i++)
  {
   if (Firsttime)
    {
     MPI_Send(&count,1,MPI_INT,i,1,MPI_COMM_WORLD);
     MPI_Send(indexlist,count,MPI_INT,i,2,MPI_COMM_WORLD);
    }
   else
    {
     MPI_Send(&dummy,1,MPI_INT,i,1,MPI_COMM_WORLD);
     MPI_Send(&samedata,1,MPI_INT,i,2,MPI_COMM_WORLD);
    }
   MPI_Send(&fno,1,MPI_INT,i,3,MPI_COMM_WORLD);
   MPI_Send(&(p.count),1,MPI_INT,i,4,MPI_COMM_WORLD);
   MPI_Send(p.classes,p.count,MPI_INT,i,5,MPI_COMM_WORLD);
   MPI_Send(p.assignments,p.count,MPI_INT,i,6,MPI_COMM_WORLD);
  }
 Firsttime = 0;
}

double find_gain_for_discrete_feature(int* indexlist, int count, int fno, Partition p)
{
/*!Last Changed 23.04.2003*/
 int i, j, k, ratio[MAXVALUES][MAXCLASSES], total[MAXVALUES][MAXCLASSES];
 MPI_Status status;
 send_partition_info_to_clients(indexlist, count, fno, p);
 for (i = 0; i < MAXVALUES; i++)
   for (j = 0; j < MAXCLASSES; j++)
     total[i][j] = 0;
 for (k = 1; k < get_parameter_i("proccount"); k++)
  {
   MPI_Recv(ratio,MAXVALUES*MAXCLASSES,MPI_INT,k,1,MPI_COMM_WORLD,&status);
   for (i = 0; i < MAXVALUES; i++)
     for (j = 0; j < MAXCLASSES; j++)
       total[i][j] += ratio[i][j];
  }
 return information_gain_from_ratios(total);
}

double find_gain_for_partition_mpi(int* indexlist, int count, Partition p, int fno, double* split)
{
/*!Last Changed 27.04.2003 Reduce ve allreduce yerine send ve recv konuldu*/
/*!Last Changed 10.02.2003*/
 int i, leftcount, rightcount, nl = 0, nr = 0;
 double leftsum, rightsum, ml = 0, mr = 0, sl = 0, sr = 0;
 MPI_Status status;
 send_partition_info_to_clients(indexlist, count, fno, p);
 for (i = 1; i < get_parameter_i("proccount"); i++)
  {
   MPI_Recv(&leftsum,1,MPI_DOUBLE,i,1,MPI_COMM_WORLD,&status);
   ml += leftsum;
   MPI_Recv(&rightsum,1,MPI_DOUBLE,i,2,MPI_COMM_WORLD,&status);
   mr += rightsum;
   MPI_Recv(&leftcount,1,MPI_INT,i,3,MPI_COMM_WORLD,&status);
   nl += leftcount;
   MPI_Recv(&rightcount,1,MPI_INT,i,4,MPI_COMM_WORLD,&status);
   nr += rightcount;
  }
 if (nl > 0)
   ml = ml / nl;
 else
   ml = 0;
 if (nr > 0)
   mr = mr / nr;
 else
   mr = 0;
 for (i = 1; i < get_parameter_i("proccount"); i++)
  {
   MPI_Send(&ml,1,MPI_DOUBLE,i,7,MPI_COMM_WORLD);
   MPI_Send(&mr,1,MPI_DOUBLE,i,8,MPI_COMM_WORLD);
  }
 for (i = 1; i < get_parameter_i("proccount"); i++)
  {
   MPI_Recv(&leftsum,1,MPI_DOUBLE,i,5,MPI_COMM_WORLD,&status);
   sl += leftsum;
   MPI_Recv(&rightsum,1,MPI_DOUBLE,i,6,MPI_COMM_WORLD,&status);
   sr += rightsum;
  }
 if (nl > 1)
   sl /= (nl-1);
 else
   sl = 0;
 if (nr > 1)
   sr /= (nr-1);
 else
   sr = 0;
 (*split) = solve_for_ldt_second_order_equation(ml, mr, sl, sr, nl, nr);
 return information_gain_for_split_mpi(*split);
}

void find_best_split_for_features_between_i_and_j(Instanceptr instances,int fnostart, int fnoend)
{
/*!Last Changed 10.02.2003*/
 int j, improved, bestfno, fno;
 double bestsplit, bestgain= +INT_MAX, gain, currentsplit;
 Partition initial,current,best;
 for (fno = fnostart; fno <= fnoend; fno++)
  {
   initial = get_initial_partition(instances);
   switch (current_dataset->features[fno].type)
    {
     case REAL   :
     case INTEGER:if (initial.count <= 2)
                   {
                    gain = find_gain_for_partition(instances, initial, fno, &bestsplit, UNIVARIATE_CONDITION);
                    if (dless(gain,bestgain))
                     {
                      bestsplit = currentsplit;
                      bestgain = gain;
                      bestfno = fno;
                     }
                   }
                  else
                   {
                    current = create_copy(initial);
                    bestgain = find_gain_for_partition(instances, current, fno, &bestsplit, UNIVARIATE_CONDITION);
                    best = create_copy(current);
                    improved = 1;
                    while (improved)
                     {
                      improved = 0;
                      for (j = 0; j < current.count; j++)
                       {
                        current.assignments[j] = 1-current.assignments[j];
                        gain = find_gain_for_partition(instances, current, fno, &currentsplit, UNIVARIATE_CONDITION);
                        if (dless(gain,bestgain))
                         {
                          bestsplit = currentsplit;
                          bestgain = gain;
                          bestfno = fno;
                          free_partition(best);
                          best = create_copy(current);
                          improved = 1;
                         }
                        current.assignments[j] = 1 - current.assignments[j];
                       }
                      if (improved)
                       {
                        free_partition(current);
                        current = create_copy(best);
                       }
                     }
                    free_partition(best);
                    free_partition(current);
                   }
                  break;
     case STRING :gain = information_gain_for_discrete_feature(instances,fno);
                  if (dless(gain,bestgain))
                   {
                    bestgain = gain;
                    bestfno = fno;
                   }
                  break;
    }
   free_partition(initial);
  }
 MPI_Send(&fno,1,MPI_INT,0,1,MPI_COMM_WORLD);
 MPI_Send(&bestsplit,1,MPI_DOUBLE,0,2,MPI_COMM_WORLD);
 MPI_Send(&bestgain,1,MPI_DOUBLE,0,3,MPI_COMM_WORLD);
}

void get_information_from_clients(int procend,double* bestgain, int* bestfeature, double* split)
{
/*!Last Changed 04.02.2003*/
 int j, featureno;
 double currentsplit, gain;
 MPI_Status status;
 for (j = 1; j < procend; j++)
  {
   MPI_Recv(&featureno,1,MPI_INT,j,1,MPI_COMM_WORLD,&status);
   MPI_Recv(&currentsplit,1,MPI_DOUBLE,j,2,MPI_COMM_WORLD,&status);
   MPI_Recv(&gain,1,MPI_DOUBLE,j,3,MPI_COMM_WORLD,&status);
   if (gain < (*bestgain))
    {
     (*bestgain) = gain;
     (*bestfeature) = featureno;
     (*split) = currentsplit;
    }
  }
}

int find_best_ldt_feature_mpi(Instanceptr instances,double* split)
{
 /*!Last Changed 11.12.2003*/
 /*!Last Changed 10.02.2003*/
 int i, bestfeature = -1, *indexlist = NULL, count, procindex = 1, fstart, fend, ratio, more;
 double bestgain = +INT_MAX;
 indexlist = create_index_array_for_instances(instances, &count);
 if (current_dataset->featurecount < get_parameter_i("proccount"))
  {
   for (i = 0; i < current_dataset -> featurecount; i++)
    {
     MPI_Send(&count,1,MPI_INT,procindex,1,MPI_COMM_WORLD);
     MPI_Send(indexlist,count,MPI_INT,procindex,2,MPI_COMM_WORLD);
     MPI_Send(&i,1,MPI_INT,procindex,3,MPI_COMM_WORLD);
     MPI_Send(&i,1,MPI_INT,procindex,4,MPI_COMM_WORLD);
     procindex++;
    }
  }
 else
  {
   for (i = 1; i < get_parameter_i("proccount"); i++)
    {
     MPI_Send(&count,1,MPI_INT,procindex,1,MPI_COMM_WORLD);
     MPI_Send(indexlist,count,MPI_INT,procindex,2,MPI_COMM_WORLD);
     ratio = (current_dataset->featurecount) / (get_parameter_i("proccount")-1);
     more = (current_dataset->featurecount) % (get_parameter_i("proccount")-1);
     if (i <= more + 1)
       fstart = (i - 1) * (ratio + 1);
     else
       fstart = (i - 1) * ratio + more;
     if (i <= more)
       fend = i * (ratio + 1) - 1;
     else
       fend = i * ratio + more - 1;
     MPI_Send(&fstart,1,MPI_INT,procindex,3,MPI_COMM_WORLD);
     MPI_Send(&fend,1,MPI_INT,procindex,4,MPI_COMM_WORLD);
     procindex++;
    }
  }
 get_information_from_clients(procindex,&bestgain,&bestfeature,split);
 safe_free(indexlist);
 if (bestgain==ISELL)
   return -1;
 else
   return bestfeature;
}

int find_best_ldt_feature_mpi_rulebased(Decisionnodeptr node)
{
/*!Last Changed 09.04.2003*/
 int i, bestfeature = -1, procindex = 1;
 double bestgain = +INT_MAX;
 MPI_Datatype* myruletype;
 myruletype = create_datatype_for_rule(node->rule);
 for (i = 0; i < current_dataset -> featurecount; i++)
   if (current_dataset->features[i].type != CLASSDEF)
    {
     MPI_Send(&(node->conditioncount),1,MPI_INT,procindex,1,MPI_COMM_WORLD);
     MPI_Send(node->rule,node->conditioncount,(*myruletype),procindex,2,MPI_COMM_WORLD);
     MPI_Send(&i,1,MPI_INT,procindex,3,MPI_COMM_WORLD);
     procindex++;
     if (procindex == get_parameter_i("proccount"))
      {
       get_information_from_clients(get_parameter_i("proccount"),&bestgain,&bestfeature,&(node->split));
       procindex = 1;
      }
    }
 get_information_from_clients(procindex,&bestgain,&bestfeature,&(node->split));
 safe_free(myruletype);
 if (bestgain==ISELL)
   return -1;
 else
   return bestfeature;
}

int divide_data_to_find_best_feature(Instanceptr instances,double* split)
{
/*!Last Changed 23.04.2003
  string featurelar icin de best-feature bulunma eklendi*/
/*!Last Changed 09.02.2003*/
 int i,j,bestfeature=-1,improved,*indexlist, count;
 double bestgain=+INT_MAX,gain,bestpartitiongain,currentsplit,bestsplit;
 Partition initial,current,best;
 initial = get_initial_partition(instances);
 indexlist = create_index_array_for_instances(instances, &count);
 for (i = 0; i < current_dataset->featurecount; i++)
   switch (current_dataset->features[i].type)
    {
     case REAL   :
     case INTEGER:if (initial.count <= 2)
                   {
                    bestpartitiongain = find_gain_for_partition_mpi(indexlist,count,initial,i,&bestsplit);
                    if (dless(bestpartitiongain,bestgain))
                     {
                      bestgain = bestpartitiongain;
                      bestfeature = i;
                      (*split) = bestsplit;
                      if (bestpartitiongain == 0)
                       {
                        safe_free(indexlist);
                        return bestfeature;
                       }
                     }
                    continue;
                   }
                  current = create_copy(initial);
                  bestpartitiongain = find_gain_for_partition_mpi(indexlist,count,current,i,&bestsplit);
                  best = create_copy(current);
                  improved = 1;
                  while (improved)
                   {
                    improved = 0;
                    for (j = 0; j < current.count; j++)
                     {
                      current.assignments[j] = 1-current.assignments[j];
                      gain = find_gain_for_partition_mpi(indexlist,count,current,i,&currentsplit);
                      if (dless(gain,bestpartitiongain))
                       {
                        bestsplit = currentsplit;
                        bestpartitiongain = gain;
                        free_partition(best);
                        best = create_copy(current);
                        improved = 1;
                       }
                      current.assignments[j]=1-current.assignments[j];
                     }
                    if (improved)
                     {
                      free_partition(current);
                      current = create_copy(best);
                     }
                   }
                  if (dless(bestpartitiongain,bestgain))
                   {
                    bestgain = bestpartitiongain;
                    bestfeature = i;
                    (*split) = bestsplit;
                    if (bestgain == 0)
                     {
                      free_partition(best);
                      free_partition(current);
                      free_partition(initial);
                      safe_free(indexlist);
                      return bestfeature;
                     }
                   }
                  free_partition(best);
                  free_partition(current);
                  break;
     case  STRING:bestpartitiongain = find_gain_for_discrete_feature(indexlist,count,i,initial);
                  if (dless(bestpartitiongain,bestgain))
                   {
                    bestgain = bestpartitiongain;
                    bestfeature = i;
                    (*split) = 0;
                    if (bestpartitiongain == 0)
                     {
                      safe_free(indexlist);
                      return bestfeature;
                     }
                   }
                  break;
    }
 safe_free(indexlist);
 if (bestgain == ISELL)
   return -1;
 else
   return bestfeature;
}

void send_node_to_processors(Decisionnodeptr stack[], int i, int procno)
{
/*!Last Changed 12.04.2003*/
/*Index kopyalanma yuzunden 1 kalmis procno,1 yapinca calisti
  Last Changed 13.04.2003*/
 int *indexlist, count, nodeindex;
 indexlist = create_index_array_for_instances(stack[i]->instances, &count);
 MPI_Send(&count,1,MPI_INT,procno,1,MPI_COMM_WORLD);
 MPI_Send(indexlist,count,MPI_INT,procno,2,MPI_COMM_WORLD);
 nodeindex = i;
 MPI_Send(&nodeindex,1,MPI_INT,procno,3,MPI_COMM_WORLD);
 safe_free(indexlist);
}

void receive_update_children(Decisionnodeptr node, Decisionnodeptr children[], int procno, int* childcount)
{
/*!Last Changed 12.04.2003*/
 int count, j;
 MPI_Status status;
 MPI_Recv(&(node->featureno),1,MPI_INT,procno,1,MPI_COMM_WORLD,&status);
 MPI_Recv(&(node->split),1,MPI_DOUBLE,procno,2,MPI_COMM_WORLD,&status);
 if (make_children(node))
   switch (current_dataset->features[node->featureno].type)
    {
     case INTEGER:
     case REAL   :if (can_be_divided(node->left, NULL))
                   {
                    children[(*childcount)] = node->left;
                    (*childcount)++;
                   }
                  if (can_be_divided(node->right, NULL))
                   {
                    children[(*childcount)] = node->right;
                    (*childcount)++;
                   }  
                  break;
     case STRING :count = current_dataset->features[node->featureno].valuecount;
                  for (j = 0; j < count; j++)
                    if (can_be_divided(&(node->string[j]), NULL))
                     {
                      children[(*childcount)] = &(node->string[j]);
                      (*childcount)++;
                     }
                  break;
    }
}

int create_ldtchildren_iterative(Decisionnodeptr root)
{
/*!Last Changed 12.04.2003*/
 int nodecount, childcount, i;
 Decisionnodeptr stack[1000];
 Decisionnodeptr children[1000];
 root->conditiontype = UNIVARIATE_CONDITION;
 if (!(can_be_divided(root, NULL)))
   return 1;
 nodecount = 1;
 stack[0] = root;
 while (nodecount>0)
  {
   if (get_parameter_i("proccount")-1 > nodecount)
     for (i = 0; i < nodecount; i++)
       send_node_to_processors(stack, i, i+1);
   else
     for (i = 1; i < get_parameter_i("proccount"); i++)
       send_node_to_processors(stack, nodecount-i, i);
   childcount = 0;
   if (get_parameter_i("proccount")-1 > nodecount)
    {
     for (i = 0; i < nodecount; i++)
       receive_update_children(stack[i], children, i+1, &childcount);
     nodecount = 0;
    }
   else
    {
     for (i = 1; i < get_parameter_i("proccount"); i++)
       receive_update_children(stack[nodecount-i], children, i, &childcount);
     nodecount -= get_parameter_i("proccount")-1;
    }
   for (i = 0; i < childcount; i++)
     stack[nodecount + i] = children[i];
   nodecount += childcount;
  }
 return 1;
}
#endif
