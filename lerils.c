#include<limits.h>
#include "data.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "lerils.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "rule.h"
#include "univariatetree.h"
#include "univariaterule.h"

extern Datasetptr current_dataset;

/**
 * Finds all possible split points for a continuous feature in a dataset. For a continuous feature, if there are K different values for that feature, there are K - 1 possible split points.
 * @param[in] instances Link list of instances
 * @param[in] fno Index of the feature
 * @param[out] count Number of distinct split points
 * @return An array of possible split points of that feature in the dataset
 */
double* possible_split_points(Instanceptr instances, int fno, int* count)
{
 int j, size;
 Instanceptr instancelist;
 double* result, value, valuebefore = -INT_MAX;
 *count = 1;
 result = safemalloc(sizeof(double), "possible_split_points", 5);
 result[0] = -INT_MAX;
 instancelist = sort_instances(instances, fno, &size);
 if (current_dataset->features[fno].type == INTEGER)
   valuebefore = instancelist[0].values[fno].intvalue;  
 else
   valuebefore = instancelist[0].values[fno].floatvalue;
 j = 1;
 while (j < size)
  {
   if (current_dataset->features[fno].type == INTEGER)
     value = instancelist[j].values[fno].intvalue;  
   else
     value = instancelist[j].values[fno].floatvalue;
   if (value != valuebefore)
    {
     (*count)++;
     result = alloc(result, (*count) * sizeof(double), *count);
     result[(*count) - 1] = (value + valuebefore) / 2;
     valuebefore = value;
    }
   j++;
  }
 (*count)++;
 result = alloc(result, (*count) * sizeof(double), *count);
 result[(*count) - 1] = +INT_MAX;
 safe_free(instancelist);
 return result;
}

int rule_cover_instance_lerils(Decisionconditionptr* rule, int* relevant, Instanceptr inst)
{
 int i, covered;
 covered = YES;
 for (i = 0; i < current_dataset->featurecount; i++)
   if (relevant[i])
     if (!condition_cover_instance(inst, rule[i]))
      {
       covered = NO;
       break;
      }
 return covered;
}

double description_length_of_rule_lerils(Decisionconditionptr* rule, int* relevant, Instanceptr instances, int positive)
{
 int i, s = 0, n = 0, p = 0;
 Instanceptr tmp;
 for (i = 0; i < current_dataset->featurecount; i++)
   if (relevant[i])
     s++;
 if (s == 0)
   return +INT_MAX;
 tmp = instances;
 while (tmp)
  {
   if (rule_cover_instance_lerils(rule, relevant, tmp))
    {
     if (give_classno(tmp) == positive)
       p++;
     else
       n++;
    }
   tmp = tmp->next;
  }
 if (p != 0)
   return (s + n) / (p + 0.0);
 else 
   return INT_MAX;
}

int split_point_lower_and_upper_limits(double splitpoint, double* limits, int count)
{
 int lower = 0, upper = count - 1, middle = (lower + upper) / 2;
 while (upper - lower > 1)
  {
   if (splitpoint < limits[middle])
     upper = middle;
   else 
     lower = middle;
   middle = (upper + lower) / 2;
  }
 return lower;
}

Ruleset generate_rule_pool(Instanceptr instances, int positive, int M, double p1)
{
 double **thresholds;
 int i, j, k, p, *counts, size, pos, **relevant, bestbit, bestp, bestdirection;
 Decisionconditionptr** included;
 Decisionconditionptr current;
 Decisionruleptr current_rule;
 Instanceptr tmp;
 Ruleset result;
 double prob, best_description_length, description_length;
 empty_ruleset(&result);
 size = data_size(instances);
 thresholds = safecalloc(current_dataset->featurecount, sizeof(double*), "learn_ruleset_lerils_oneclass", 4);
 counts = safecalloc(current_dataset->featurecount, sizeof(int), "learn_ruleset_lerils_oneclass", 5);
 for (i = 0; i < current_dataset->featurecount; i++)
   if (in_list(current_dataset->features[i].type, 2, INTEGER, REAL))
     thresholds[i] = possible_split_points(instances, i, &(counts[i]));
 included = (Decisionconditionptr**) safemalloc_2d(sizeof(Decisionconditionptr*), sizeof(Decisionconditionptr), M, current_dataset->featurecount, "generate_rule_pool", 10);
 relevant = (int**) safemalloc_2d(sizeof(int *), sizeof(int), M, current_dataset->featurecount, "generate_rule_pool", 11);
 for (i = 0; i < M; i++)
  {
   do{
      pos = myrand() % size;
      tmp = data_x(instances, pos);
   }while (give_classno(tmp) != positive);
   for (j = 0; j < current_dataset->featurecount; j++)
     if (current_dataset->features[j].type != CLASSDEF)
      {
       current = empty_condition();
       current->conditiontype = UNIVARIATE_CONDITION;
       current->freeparam = 1;
       current->featureindex = j;
       included[i][j] = current;
       prob = produce_random_value(0.0, 1.0);
       if (prob < 0.5)
         relevant[i][j] = YES;
       else
         relevant[i][j] = NO;
       switch (current_dataset->features[j].type)
        {
         case INTEGER:pos = split_point_lower_and_upper_limits(tmp->values[j].intvalue, thresholds[j], counts[j]);
                      current->comparison = '*';
                      current->upperlimit = thresholds[j][pos + 1];
                      current->lowerlimit = thresholds[j][pos];
                      current->datasize = pos;
                      current->value = pos + 1;
                      break;
         case    REAL:pos = split_point_lower_and_upper_limits(tmp->values[j].floatvalue, thresholds[j], counts[j]);
                      current->comparison = '*';
                      current->upperlimit = thresholds[j][pos + 1];
                      current->lowerlimit = thresholds[j][pos];
                      current->datasize = pos;
                      current->value = pos + 1;
                      break;
         case  STRING:current->comparison = '=';
                      current->value = tmp->values[j].stringindex;
                      break;
        }
      }
     else
       relevant[i][j] = NO;
   /*Random walk*/
   for (j = 0; j < 3 * (current_dataset->featurecount - 1); j++)
    {
     prob = produce_random_value(0.0, 1.0);
     if (prob < p1)
      {
       do{
         pos = myrand() % current_dataset->featurecount;
       }while (current_dataset->features[pos].type == CLASSDEF); 
       relevant[i][pos] = 1 - relevant[i][pos];
      }
     else
      {
       best_description_length = description_length_of_rule_lerils(included[i], relevant[i], instances, positive);
       bestbit = -1;
       for (k = 0; k < current_dataset->featurecount; k++)
         if (current_dataset->features[k].type != CLASSDEF)
          {
           if (current_dataset->features[k].type == STRING || relevant[i][k])
            {
             relevant[i][k] = 1 - relevant[i][k];
             if (rule_cover_instance_lerils(included[i], relevant[i], tmp))
              {
               description_length = description_length_of_rule_lerils(included[i], relevant[i], instances, positive);
               if (description_length < best_description_length)
                {
                 bestbit = k;
                 best_description_length = description_length;
                }
              }
             relevant[i][k] = 1 - relevant[i][k];
            }
           else
             if (!relevant[i][k])
              {
               relevant[i][k] = 1 - relevant[i][k];
               for (p = included[i][k]->datasize - 1; p >= 0; p--)
                {
                 included[i][k]->lowerlimit = thresholds[k][p];
                 if (rule_cover_instance_lerils(included[i], relevant[i], tmp))
                  {
                   description_length = description_length_of_rule_lerils(included[i], relevant[i], instances, positive);
                   if (description_length < best_description_length)
                    {
                     bestbit = k;
                     bestp = p;
                     bestdirection = -1;
                     best_description_length = description_length;
                    }
                  }
                }
               included[i][k]->lowerlimit = thresholds[k][included[i][k]->datasize];
               for (p = ((int)included[i][k]->value) + 1; p < counts[k]; p++)
                {
                 included[i][k]->upperlimit = thresholds[k][p];
                 if (rule_cover_instance_lerils(included[i], relevant[i], tmp))
                  {
                   description_length = description_length_of_rule_lerils(included[i], relevant[i], instances, positive);
                   if (description_length < best_description_length)
                    {
                     bestbit = k;
                     bestp = p;
                     bestdirection = +1;
                     best_description_length = description_length;
                    }
                  }
                }
               included[i][k]->upperlimit = thresholds[k][(int)included[i][k]->value];
               relevant[i][k] = 1 - relevant[i][k];
              }
          }
       if (bestbit != -1)
        {
         relevant[i][bestbit] = 1 - relevant[i][bestbit];
         if (relevant[i][bestbit] && current_dataset->features[bestbit].type != STRING)
          {
           if (bestdirection == -1)
             included[i][bestbit]->lowerlimit = thresholds[bestbit][bestp];
           else
             included[i][bestbit]->upperlimit = thresholds[bestbit][bestp];
          }
        }
      }
    }
   current_rule = empty_rule(positive);
   for (j = 0; j < current_dataset->featurecount; j++)
     if (relevant[i][j])
       add_condition(current_rule, create_copy_of_condition(included[i][j]));
   if (!rule_exists(result, current_rule))
     add_rule(&result, current_rule);
   else
    {
     free_rule(*current_rule);
     safe_free(current_rule);
    }
  }
 free_2d((void**)relevant, M);
 for (i = 0; i < M; i++)
  {
   for (j = 0; j < current_dataset->featurecount; j++)
     if (current_dataset->features[j].type != CLASSDEF)
      {
       free_condition(*(included[i][j]));
       safe_free(included[i][j]);
      }
   safe_free(included[i]);
  }
 safe_free(included);
 for (i = 0; i < current_dataset->featurecount; i++)
   if (counts[i] > 0)
     safe_free(thresholds[i]);
 safe_free(thresholds);
 safe_free(counts); 
 return result;
}

int description_length_of_ruleset_lerils(Ruleset rule_pool, int* included, Instanceptr instances, int positive)
{
 int i, exception = 0, literals = 0, M = rule_pool.rulecount, classno;
 BOOLEAN covered;
 Instanceptr tmp;
 Decisionruleptr current_rule;
 for (current_rule = rule_pool.start, i = 0; i < M; current_rule = current_rule->next, i++)
  {
   current_rule->falsepositives = 0;
   if (included[i])
     literals += current_rule->decisioncount;
  }
 tmp = instances;
 while (tmp)
  {
   classno = give_classno(tmp);
   covered = BOOLEAN_FALSE;
   for (current_rule = rule_pool.start, i = 0; i < M; current_rule = current_rule->next, i++)
     if (included[i] && rule_cover_instance(tmp, *current_rule))
      {
       covered = BOOLEAN_TRUE;
       break;
      }
   if ((covered && classno != positive) || (!covered && classno == positive))
    {
     if (covered)
       (current_rule->falsepositives)++;
     exception++;
    }
   tmp = tmp->next;
  }
 return exception + literals;
}

Ruleset optimal_subset_of_rule_pool(Ruleset rule_pool, Instanceptr instances, int positive, int restarts)
{
 int i, j, M = rule_pool.rulecount, bestflip;
 int best_description_length = +INT_MAX, current_best_description_length, description_length, improved; 
 int *current_ruleset, *best_ruleset;
 double p;
 Decisionruleptr current_rule;
 Ruleset result;
 current_ruleset = safemalloc(M * sizeof(int), "optimal_subset_of_rule_pool", 4);
 best_ruleset = safemalloc(M * sizeof(int), "optimal_subset_of_rule_pool", 5);
 for (i = 0; i < restarts; i++)
  {
   /*Initialize ruleset*/
   for (j = 0; j < M; j++)
    {
     p = produce_random_value(0.0, 1.0);
     if (p < 0.5)
       current_ruleset[j] = YES;
     else 
       current_ruleset[j] = NO;
    }
   /*Greedy search*/
   current_best_description_length = description_length_of_ruleset_lerils(rule_pool, current_ruleset, instances, positive);
   improved = YES;
   while (improved)
    {
     improved = NO;
     bestflip = -1;
     for (j = 0; j < M; j++)
      {
       /*flip rules, 1 becomes 0, 0 becomes 1*/;
       current_ruleset[j] = 1 - current_ruleset[j]; 
       description_length = description_length_of_ruleset_lerils(rule_pool, current_ruleset, instances, positive);
       if (description_length < current_best_description_length)
        {
         current_best_description_length = description_length;
         improved = YES;
         bestflip = j;
        }
       current_ruleset[j] = 1 - current_ruleset[j];
      }
     if (improved)
       current_ruleset[bestflip] = 1 - current_ruleset[bestflip];
    }
   if (current_best_description_length < best_description_length)
    {
     best_description_length = current_best_description_length;
     memcpy(best_ruleset, current_ruleset, M * sizeof(int));
    }
  }
 empty_ruleset(&result);
 for (current_rule = rule_pool.start, i = 0; i < M; current_rule = current_rule->next, i++)
   if (best_ruleset[i])
     add_rule(&result, create_copy_of_rule(current_rule));
 safe_free(current_ruleset);
 safe_free(best_ruleset);
 return result;
}

Ruleset learn_ruleset_lerils_oneclass(Instanceptr instances, Lerils_parameterptr param, int positive)
{
 Ruleset rule_pool, result;
 rule_pool = generate_rule_pool(instances, positive, param->pool_size, param->flip_probability);
 result = optimal_subset_of_rule_pool(rule_pool, instances, positive, param->restarts);
 free_ruleset(rule_pool);
 return result;
}

Ruleset order_rulesets(Ruleset* sets, Instanceptr* instances)
{
 int i, j, *included, minexception, bestclass, exception, *counts;
 Ruleset result;
 Instanceptr covered = NULL;
 empty_ruleset(&result);
 included = (int *)safecalloc(current_dataset->classno, sizeof(int), "order_rulesets", 4);
 for (i = 0; i < current_dataset->classno; i++)
  {
   bestclass = -1;
   minexception = +INT_MAX;
   for (j = 0; j < current_dataset->classno; j++)
     if (!included[j])
      {
       exception = exception_count_of_ruleset(sets[j], *instances, j);
       if (exception < minexception)
        {
         minexception = exception;
         bestclass = j;
        }
      }
   remove_covered_by_ruleset(instances, &covered, sets[bestclass], 0);
   merge_ruleset(&result, sets[bestclass]);
   included[bestclass] = YES;
  }
 counts = find_class_counts(*instances);
 result.defaultclass = find_max_in_list(counts, current_dataset->classno);
 merge_instancelist(instances, covered);
 safe_free(counts);
 safe_free(included);
 return result;
}

Ruleset learn_ruleset_lerils(Instanceptr* instances, Lerils_parameterptr param)
{
 int i;
 Ruleset* sets;
 Ruleset result;
 sets = safemalloc(current_dataset->classno * sizeof(Ruleset), "learn_ruleset_lerils", 3);
 for (i = 0; i < current_dataset->classno; i++) 
   sets[i] = learn_ruleset_lerils_oneclass(*instances, param, i);
 result = order_rulesets(sets, instances);
 for (i = 0; i < current_dataset->classno; i++) 
   safe_free(sets[i].counts);
 safe_free(sets);
 return result;
}


