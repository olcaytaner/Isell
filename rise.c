#include <limits.h>
#include <math.h>
#include "data.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "rise.h"
#include "rule.h"
#include "univariaterule.h"

extern Datasetptr current_dataset;

Decisionruleptr most_specific_generalization(Decisionruleptr rule, Instanceptr nearest)
{
 int fno;
 Decisionruleptr newrule;
 Decisionconditionptr current, before = NULL, next;
 newrule = create_copy_of_rule(rule);
 newrule->next = rule->next;
 current = newrule->start;
 while (current)
  {
   next = current->next;
   fno = current->featureindex;
   switch (current_dataset->features[fno].type)
    {
     case  STRING:if (nearest->values[fno].stringindex != current->value)
                    remove_condition_after(newrule, before, current);
                  else
                    before = current;
                  break;
     case INTEGER:if (nearest->values[fno].intvalue > current->upperlimit)
                    current->upperlimit = nearest->values[fno].intvalue;
                  else
                    if (nearest->values[fno].intvalue < current->lowerlimit)
                      current->lowerlimit = nearest->values[fno].intvalue;
                  before = current;
                  break;
     case    REAL:if (nearest->values[fno].floatvalue > current->upperlimit)
                    current->upperlimit = nearest->values[fno].floatvalue;
                  else
                    if (nearest->values[fno].floatvalue < current->lowerlimit)
                      current->lowerlimit = nearest->values[fno].floatvalue;
                  before = current;
                  break;
    }
   current = next;
  }
 return newrule;
}

Instanceptr nearest_example_not_covered(Model_riseptr m, Decisionruleptr rule, Instanceptr instances)
{
 Instanceptr tmp, nearest = NULL;
 double distance, mindistance = +INT_MAX;
 tmp = instances;
 while (tmp)
  {
   if (give_classno(tmp) == rule->classno && !rule_cover_instance(tmp, *rule))
    {
     distance = distance_to_rule(m, tmp, rule);
     if (distance < mindistance)
      {
       mindistance = distance;
       nearest = tmp;
      }
    }
   tmp = tmp->next;
  }
 return nearest;
}

Decisionruleptr nearest_rule(Model_riseptr m, Instanceptr instance, int leaveoneout)
{
 Decisionruleptr rule, nearest = NULL;
 double mindistance = +INT_MAX, distance, minlaplace = +INT_MAX, laplace;
 rule = m->rules.start;
 while (rule)
  {
   if (rule->covered + rule->falsepositives > 1 || !leaveoneout)
    {
     distance = distance_to_rule(m, instance, rule);
     if (distance < mindistance)
      {
       mindistance = distance;
       nearest = rule;
       minlaplace = laplace_estimate_of_rule_kclass(*rule);
      }
     else
       if (distance == mindistance)
        {
         laplace = laplace_estimate_of_rule_kclass(*rule);
         if (laplace < minlaplace)
          {
           minlaplace = laplace;
           nearest = rule;           
          }
         else
           if (laplace == minlaplace && m->counts[rule->classno] > m->counts[nearest->classno])
             nearest = rule;
        }
    }
   rule = rule->next;
  }
 return nearest;
}

Decisionruleptr create_rule_from_instance(Instanceptr inst)
{
 int i;
 Decisionruleptr result;
 Decisionconditionptr newcondition;
 result = empty_rule(give_classno(inst));
 for (i = 0; i < current_dataset->featurecount; i++)
   if (current_dataset->features[i].type != CLASSDEF)
    {
     newcondition = empty_condition();
     newcondition->conditiontype = UNIVARIATE_CONDITION;
     newcondition->featureindex = i;
     newcondition->freeparam = 1;
     switch (current_dataset->features[i].type)
      {
       case INTEGER:newcondition->lowerlimit = inst->values[i].intvalue;
                    newcondition->upperlimit = inst->values[i].intvalue;
                    newcondition->comparison = '*';
                    break;
       case    REAL:newcondition->lowerlimit = inst->values[i].floatvalue;
                    newcondition->upperlimit = inst->values[i].floatvalue;
                    newcondition->comparison = '*';
                    break;
       case  STRING:newcondition->comparison = '=';
                    newcondition->value = inst->values[i].stringindex;
                    break;
      }
     add_condition(result, newcondition);
    }
 return result;
}

matrix distance_between_discrete_values_of_feature(int*** feature_counts, int fno)
{
 int i, j, k, count = current_dataset->features[fno].valuecount, *counts;
 double distance;
 matrix result;
 counts = safecalloc(count, sizeof(int), "distance_between_discrete_values_of_feature", 3);
 result = matrix_alloc(count, count);
 for (i = 0; i < count; i++)
   for (j = 0; j < current_dataset->classno; j++)
     counts[i] += feature_counts[j][fno][i];
 for (i = 0; i < count; i++)
   for (j = i + 1; j < count; j++)
    {
     distance = 0.0;
     for (k = 0; k < current_dataset->classno; k++)
       if (counts[i] > 0 && counts[j] > 0)
         distance += fabs(feature_counts[k][fno][i] / (counts[i] + 0.0) - feature_counts[k][fno][j] / (counts[j] + 0.0));
     result.values[i][j] = distance * distance;
     result.values[j][i] = distance * distance;
    }
 safe_free(counts);
 return result;
}

void update_nearest_rule(Model_riseptr m, Decisionruleptr* nearestrule, Instanceptr instances, int operation, Decisionruleptr rule)
{
 int i = 0;
 Instanceptr tmp;
 tmp = instances;
 while (tmp)
  {
   if (operation == 0 && nearestrule[i] == rule)
     nearestrule[i] = nearest_rule(m, tmp, YES);
   else
     if (operation == 1 && rule->falsepositives + rule->covered > 1)
      {
       if (!nearestrule[i])
         nearestrule[i] = rule;
       else
        {
         if (distance_to_rule(m, tmp, rule) < distance_to_rule(m, tmp, nearestrule[i]))
           nearestrule[i] = rule;
         else 
           if (distance_to_rule(m, tmp, rule) == distance_to_rule(m, tmp, nearestrule[i]))
            {
             if (laplace_estimate_of_rule_kclass(*rule) < laplace_estimate_of_rule_kclass(*(nearestrule[i])))
               nearestrule[i] = rule;
             else
               if (laplace_estimate_of_rule_kclass(*rule) == laplace_estimate_of_rule_kclass(*(nearestrule[i])))
                {
                 if (m->counts[rule->classno] > m->counts[nearestrule[i]->classno])
                   nearestrule[i] = rule;
                }
            }
        }
      }
   i++;
   tmp = tmp->next;
  }
}

double distance_to_rule(Model_riseptr m, Instanceptr inst, Decisionruleptr rule)
{
 Decisionconditionptr condition;
 int fno;
 double distance = 0.0;
 condition = rule->start;
 while (condition)
  {
   if (!condition_cover_instance(inst, condition))
    {
     fno = condition->featureindex;
     switch (current_dataset->features[fno].type)
      {
       case  STRING:distance += m->svdm[fno].values[inst->values[fno].stringindex][(int)condition->value];
                    break;
       case INTEGER:if (inst->values[fno].intvalue > condition->upperlimit)
                      distance += pow((inst->values[fno].intvalue - condition->upperlimit) / (current_dataset->features[fno].max.intvalue - current_dataset->features[fno].min.intvalue), 2);
                    else
                      distance += pow((condition->lowerlimit - inst->values[fno].intvalue) / (current_dataset->features[fno].max.intvalue - current_dataset->features[fno].min.intvalue), 2);
                    break;
       case    REAL:if (inst->values[fno].floatvalue > condition->upperlimit)
                      distance += pow((inst->values[fno].floatvalue - condition->upperlimit) / (current_dataset->features[fno].max.floatvalue - current_dataset->features[fno].min.floatvalue), 2);
                    else
                      distance += pow((condition->lowerlimit - inst->values[fno].floatvalue) / (current_dataset->features[fno].max.floatvalue - current_dataset->features[fno].min.floatvalue), 2);
                    break;
      }
    }
   condition = condition->next;
  }
 return distance;
}

int accuracy_of_ruleset(Decisionruleptr* nearestrule, Instanceptr instances)
{
 int accuracy = 0, i = 0;
 Decisionruleptr rule;
 Instanceptr tmp;
 tmp = instances;
 while (tmp)
  {
   rule = nearestrule[i];
   if (rule)
    {
     if (give_classno(tmp) == rule->classno)
       accuracy++;
    }
   i++;
   tmp = tmp->next;
  }
 return accuracy;
}

Model_riseptr learn_ruleset_rise(Instanceptr instances)
{
 int i, classno, maxcount, ruleexists;
 int ***feature_counts;
 int improvement = YES, bestaccuracy, accuracy;
 Model_riseptr m;
 Decisionruleptr newrule, generalizedrule, rule, rulebefore, nextrule;
 Decisionruleptr* nearestrule;
 Instanceptr tmp;
 m = safemalloc(sizeof(Model_rise), "learn_ruleset_rise", 4);
 m->counts = find_class_counts(instances);
 m->svdm = safemalloc(current_dataset->featurecount * sizeof(matrix), "learn_ruleset_rise", 6);
 nearestrule = safemalloc(data_size(instances) * sizeof(Decisionruleptr), "learn_ruleset_rise", 6);
 feature_counts = find_feature_counts(instances);
 for (i = 0; i < current_dataset->featurecount; i++)
   if (current_dataset->features[i].type == STRING)
     m->svdm[i] = distance_between_discrete_values_of_feature(feature_counts, i);
 free_feature_counts(feature_counts);     
 empty_ruleset(&(m->rules));
 m->rules.defaultclass = find_max_in_list(m->counts, current_dataset->classno);
 classno = all_in_one_class(instances, &maxcount);
 if (!(classno % 10))
   return m;
 tmp = instances;
 while (tmp)
  {
   newrule = create_rule_from_instance(tmp);
   add_rule(&(m->rules), newrule);
   tmp = tmp->next;
  }
 rule = m->rules.start;
 while (rule)
  {
   update_counts(rule, instances, YES);
   rule = rule->next;
  }
 i = 0;
 tmp = instances;
 while (tmp)
  {
   nearestrule[i] = nearest_rule(m, tmp, YES);
   i++;
   tmp = tmp->next;
  }
 bestaccuracy = accuracy_of_ruleset(nearestrule, instances);
 while (improvement)
  {
   improvement = NO;
   rule = m->rules.start;
   rulebefore = NULL;
   while (rule)
    {
     nextrule = rule->next;
     tmp = nearest_example_not_covered(m, rule, instances);
     if (tmp)
      {
       generalizedrule = most_specific_generalization(rule, tmp);
       update_counts(generalizedrule, instances, YES);
       if (rule_exists(m->rules, generalizedrule))
         ruleexists = YES;
       else
         ruleexists = NO;
       remove_rule_after(&(m->rules), rulebefore, rule);
       update_nearest_rule(m, nearestrule, instances, 0, rule);
       add_rule_after(&(m->rules), rulebefore, generalizedrule);
       update_nearest_rule(m, nearestrule, instances, 1, generalizedrule);
       accuracy = accuracy_of_ruleset(nearestrule, instances);
       if (accuracy >= bestaccuracy)
        {
         bestaccuracy = accuracy;
         improvement = YES;
         free_rule(*rule);
         safe_free(rule);
         if (ruleexists)
          {
           remove_rule_after(&(m->rules), rulebefore, generalizedrule);
           update_nearest_rule(m, nearestrule, instances, 0, generalizedrule);
           bestaccuracy = accuracy_of_ruleset(nearestrule, instances);
           free_rule(*generalizedrule);
           safe_free(generalizedrule);
          }
         else
           rulebefore = generalizedrule;
        }
       else
        {
         remove_rule_after(&(m->rules), rulebefore, generalizedrule);
         update_nearest_rule(m, nearestrule, instances, 0, generalizedrule);
         add_rule_after(&(m->rules), rulebefore, rule);
         update_nearest_rule(m, nearestrule, instances, 1, rule);
         rulebefore = rule;
         free_rule(*generalizedrule);
         safe_free(generalizedrule);
        }
      }
     else
       rulebefore = rule;
     rule = nextrule;
    }
  }
 safe_free(nearestrule);
 return m; 
}
