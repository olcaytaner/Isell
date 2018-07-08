#include <math.h>
#include <string.h>
#include "cn2.h"
#include "data.h"
#include "decisiontree.h"
#include "distributions.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "rule.h"
#include "univariaterule.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

int is_better_candidate(Ruleset newstar, int maxstar, double laplaceestimate)
{
 if (newstar.rulecount < maxstar)
   return YES;
 if (laplace_estimate_of_rule(*(newstar.end)) > laplaceestimate)
   return YES;
 return NO;
}

Decisionruleptr generate_candidate_for_newstar(Decisionruleptr current, int fno, char comparison, double split, int falsepositives, int covered)
{
 Decisionruleptr result;
 Decisionconditionptr newcondition;
 result = create_copy_of_rule(current);
 newcondition = empty_condition();
 newcondition->conditiontype = UNIVARIATE_CONDITION;
 newcondition->freeparam = 1;
 newcondition->featureindex = fno;
 newcondition->comparison = comparison;
 newcondition->value = split;
 result->falsepositives = falsepositives;
 result->covered = covered;
 add_condition(result, newcondition);
 return result;
}

void add_candidate_to_newstar(Decisionruleptr candidate, Ruleset* newstar, int maxstar)
{
 Decisionruleptr tmp, tmpbefore = NULL, deleted;
 tmp = newstar->start;
 while (tmp)
  {
   if (laplace_estimate_of_rule(*tmp) > laplace_estimate_of_rule(*candidate))
     break;
   tmpbefore = tmp;
   tmp = tmp->next;
  }
 add_rule_after(newstar, tmpbefore, candidate);
 if (newstar->rulecount > maxstar)
  {
   deleted = newstar->end;
   remove_rule(newstar, newstar->end);
   free_rule(*deleted);
   safe_free(deleted);
  }
}

void specialize_complex(Decisionruleptr current, Ruleset* newstar, Instanceptr* instances, int positive, int maxstar)
{
 int i, j, size, *featurecounts, *positivecounts, poscount, totalpositive;
 double split, prevvalue, value;
 Decisionruleptr candidate;
 Instanceptr covered = NULL, instancelist, tmp;
 remove_covered(instances, &covered, current);
 totalpositive = find_class_count(covered, positive);
 size = data_size(covered);
 for (i = 0; i < current_dataset->featurecount; i++)
   switch (current_dataset->features[i].type)
    { 
     case INTEGER:
     case    REAL:instancelist = sort_instances(covered, i, &size);
                  if (current_dataset->features[i].type == INTEGER)
                    prevvalue = instancelist[0].values[i].intvalue;
                  else
                    prevvalue = instancelist[0].values[i].floatvalue;
                  if (give_classno(&(instancelist[0])) == positive)
                    poscount = 1;
                  else
                    poscount = 0;
                  for (j = 1; j < size; j++)
                   {
                    if (current_dataset->features[i].type == INTEGER)
                      value = instancelist[j].values[i].intvalue;
                    else
                      value = instancelist[j].values[i].floatvalue;
                    if (value != prevvalue)
                     {
                      split = (value + prevvalue) / 2;
                      if (is_better_candidate(*newstar, maxstar, laplace_estimate(j - poscount, j)))
                       {
                        candidate = generate_candidate_for_newstar(current, i, '<', split, j - poscount, poscount);
                        add_candidate_to_newstar(candidate, newstar, maxstar);
                       }
                      if (is_better_candidate(*newstar, maxstar, laplace_estimate(size - j - totalpositive + poscount, size - j)))
                       {
                        candidate = generate_candidate_for_newstar(current, i, '>', split, size - j - totalpositive + poscount, totalpositive - poscount);
                        add_candidate_to_newstar(candidate, newstar, maxstar);
                       }
                      prevvalue = value;
                     }
                    if (give_classno(&(instancelist[j])) == positive)
                      poscount++;
                   }
                  safe_free(instancelist);
                  break;
     case  STRING:featurecounts = safecalloc(current_dataset->features[i].valuecount, sizeof(int), "find_best_condition", 94);
                  positivecounts = safecalloc(current_dataset->features[i].valuecount,sizeof(int), "find_best_condition", 95); 
                  tmp = covered;
                  while (tmp)
                   {
                    featurecounts[tmp->values[i].stringindex]++;
                    if (give_classno(tmp) == positive)
                      positivecounts[tmp->values[i].stringindex]++;
                    tmp = tmp->next;
                   }
                  for (j = 0; j < current_dataset->features[i].valuecount; j++)
                    if (featurecounts[j] != size && is_better_candidate(*newstar, maxstar, laplace_estimate(featurecounts[j] - positivecounts[j], featurecounts[j])))
                     {
                      candidate = generate_candidate_for_newstar(current, i, '=', j, featurecounts[j] - positivecounts[j], positivecounts[j]);
                      add_candidate_to_newstar(candidate, newstar, maxstar);
                     }
                  safe_free(featurecounts);
                  safe_free(positivecounts);
                  break;
    }
 merge_instancelist(instances, covered); 
}

int rule_significantly_better(Decisionruleptr current, double positive_ratio)
{
 double pvalue, sum = 0.0;
 double falsepositives, covered;
 if (current->covered == 0 || error_rate_of_rule(*current) > 0.5)
   return NO;
 covered = positive_ratio * (current->covered + current->falsepositives);
 falsepositives = (1 - positive_ratio) * (current->covered + current->falsepositives);
 if (current->covered != 0 && covered != 0.0)
   sum += current->covered * log(current->covered / covered);
 if (current->falsepositives != 0 && falsepositives != 0.0)
   sum += current->falsepositives * log(current->falsepositives / falsepositives);
 sum *= 2;
 pvalue = chi_square(1, sum);
 if (pvalue > get_parameter_f("confidencelevel"))
   return NO;
 return YES;
}

Decisionruleptr find_best_complex(Instanceptr* instances, int positive, int maxstar, double positive_ratio)
{
 Ruleset star;
 Ruleset newstar;
 Decisionruleptr best_complex = NULL;
 Decisionruleptr rule;
 empty_ruleset(&star);
 rule = empty_rule(positive);
 rule->covered = find_class_count(*instances, positive);
 rule->falsepositives = data_size(*instances) - rule->covered;
 add_rule(&star, rule);
 while (star.rulecount > 0)
  {
   /*Specialize complexes*/
   empty_ruleset(&newstar);
   rule = star.start;
   while (rule)
    {
     if (rule->falsepositives != 0)
       specialize_complex(rule, &newstar, instances, positive, maxstar);
     rule = rule->next;
    }
   /*Check for best complex*/
   rule = newstar.start;
   while (rule)
    {
     if (rule_significantly_better(rule, positive_ratio) && (best_complex == NULL || laplace_estimate_of_rule(*rule) < laplace_estimate_of_rule(*best_complex)))
      {
       if (best_complex != NULL)
        {
         free_rule(*best_complex);
         safe_free(best_complex);
        }
       best_complex = create_copy_of_rule(rule);
      }
     rule = rule->next;
    }
   free_ruleset(star);
   star = newstar;
  }
 safe_free(star.counts);
 return best_complex;
}

Ruleset learn_ruleset_cn2(Instanceptr* instances, Cn2_parameterptr param)
{
 Decisionruleptr newrule;
 int classno, classcount, *classes, posclass;
 Instanceptr covered = NULL;
 int *counts, maxcount, i;
 double positive_ratio;
 Ruleset result;
 BOOLEAN givenperm;
 if (strlen(param->classpermutation) == 0)
   givenperm = BOOLEAN_FALSE;
 else
   givenperm = BOOLEAN_TRUE; 
 empty_ruleset(&result);
 classno = all_in_one_class(*instances, &maxcount);
 if (!(classno % 10))
   return result; 
 counts = find_class_counts(*instances);
 if (!givenperm)
   classes = find_max_occured(counts, current_dataset->classno, &classcount);
 else
   classcount = current_dataset->classno;
 if (!givenperm)
   result.defaultclass = classes[0];
 else
   result.defaultclass = param->classpermutation[classcount - 1] - '0';
 for (i = classcount - 1; i > 0; i--)
  {
   if (!givenperm)
     posclass = classes[i];
   else
     posclass = param->classpermutation[classcount - i - 1] - '0';
   positive_ratio = find_class_count(*instances, posclass) / (data_size(*instances) + 0.0);
   while (find_class_count(*instances, posclass) > 0)
    {
     newrule = find_best_complex(instances, posclass, param->maxstar, positive_ratio);
     if (newrule)
      {
       remove_covered(instances, &covered, newrule);
       add_rule(&result, newrule);
      }
     else 
       break;
    }
  }
 merge_instancelist(instances, covered);
 safe_free(counts);
 if (!givenperm)
   safe_free(classes);
 return result;
}

