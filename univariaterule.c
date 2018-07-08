#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "c45rules.h"
#include "data.h"
#include "decisiontree.h"
#include "distributions.h"
#include "instance.h"
#include "instancelist.h"
#include "krule.h"
#include "libarray.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "nodeboundedtree.h"
#include "partition.h"
#include "rule.h"
#include "univariaterule.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

/**
 * Counts total number of possible split points in a dataset. For a continuous feature, if there are K different values for that feature, there are K - 1 possible split points. For a discrete feature, if there are K different values for that feature, there are K possible split points.
 * @param[in] instances Link list of instances
 * @return Total number of possible split points in a dataset
 */
int possible_condition_count(Instanceptr instances)
{
 /*!Last Changed 18.05.2003*/
 int i, j, result = 0, size;
 Instanceptr instancelist;
 double valuebefore, value;
 for (i = 0; i < current_dataset->featurecount; i++)
   switch (current_dataset->features[i].type)
    {
     case INTEGER:
     case    REAL:instancelist = sort_instances(instances, i, &size);
                  j = 0;
                  valuebefore = INT_MAX;
                  while (j < size)
                   {
                    if (current_dataset->features[i].type == INTEGER)
                      value = instancelist[j].values[i].intvalue;  
                    else
                      value = instancelist[j].values[i].floatvalue;
                    if (value != valuebefore)
                     {
                      result += 2;
                      valuebefore = value;
                     }
                    j++;
                   }
                  safe_free(instancelist);
                  break;
     case  STRING:result += current_dataset->features[i].valuecount;
                  break;
    }
 return result;
}

void prunerule_in_optimize_phase(Decisionruleptr rule, Instanceptr pruneset, int positive)
{
 /*!Last Changed 02.12.2004 added free_condition and safe_free*/
 /* Last Changed 21.05.2003*/
 int improved = 1;
 double performance, bestperformance = rule_value(rule, positive, pruneset);
 Decisionconditionptr best, bestbefore, tmpcondition, tmpbefore;
 while (improved)
  {
   improved = 0;
   tmpbefore = NULL;
   tmpcondition = rule->start;
   while (tmpcondition)
    {
     remove_condition_after(rule, tmpbefore, tmpcondition);
     performance = rule_value(rule, positive, pruneset);
     if (dless(performance, bestperformance) || dequal(performance, bestperformance))
      {
       bestperformance = performance;
       bestbefore = tmpbefore;
       best = tmpcondition;
       improved = 1;
      }
     add_condition_after(rule, tmpbefore, tmpcondition);
     tmpbefore = tmpcondition;
     tmpcondition = tmpcondition->next;
    }
   if (improved)
    {
     remove_condition_after(rule, bestbefore, best);
     free_condition(*best);
     safe_free(best);
    }
  }
}

double description_length_with_or_without_rule(Ruleset* r, int positive, int startrule, Instanceptr list1, Instanceptr list2, Instanceptr list3, Instanceptr list4, double proportion, int with)
{
 /*!Last Changed 04.12.2003*/
 int covered = 0, uncovered, fp = 0, fn, rulecount = 0;
 Decisionruleptr rule = r->start;
 double dlnow = 0;
 uncovered = data_size(list1) + data_size(list2) + data_size(list3) + data_size(list4);
 fn = find_class_count(list1, positive) + find_class_count(list2, positive) + find_class_count(list3, positive) + find_class_count(list4, positive);
 while (rule && rule->classno != positive)
  {
   rulecount++;
   rule = rule->next;
  }
 while (rulecount < startrule)
  {
   dlnow += description_length_of_rule(*rule, r->possibleconditioncount);
   uncovered -= rule->covered + rule->falsepositives;
   covered += rule->covered + rule->falsepositives;
   fp += rule->falsepositives;
   fn -= rule->covered;
   rule = rule->next;
   rulecount++;
  }
 if (!with)
   rule = rule->next;
 if (rule)
  {
   initialize_counts(rule);
   update_counts_rules(rule, list1, &covered, &uncovered, &fp, &fn);
   update_counts_rules(rule, list2, &covered, &uncovered, &fp, &fn);
   update_counts_rules(rule, list3, &covered, &uncovered, &fp, &fn);
   update_counts_rules(rule, list4, &covered, &uncovered, &fp, &fn);
   while (rule)
    {
     if (!can_delete_rule(rule, &covered, &uncovered, &fp, &fn, proportion, r->possibleconditioncount))
       dlnow += description_length_of_rule(*rule, r->possibleconditioncount);
     rule = rule->next;
    }
  }
 dlnow += description_length_of_exceptions(covered, uncovered, fp, fn, proportion);
 return dlnow;
}

double relative_compression(Ruleset* r, int positive, int startrule, Instanceptr list1, Instanceptr list2, Instanceptr list3, Instanceptr list4, double proportion)
{
 /*!Last Changed 04.12.2003*/
 double dlwithout, dlwith;
 dlwithout = description_length_with_or_without_rule(r, positive, startrule, list1, list2, list3, list4, proportion, 0);
 dlwith = description_length_with_or_without_rule(r, positive, startrule, list1, list2, list3, list4, proportion, 1);
 return dlwith-dlwithout;
}

void check_for_bestgain(int positive, int total, double it, double* bestgain, Decisionconditionptr result, char comparison, double value, int featureindex)
{
 /*!Last Changed 27.05.2003 gain = positive * (it - itdot);*/
 /*!Last Changed 10.05.2003*/
 double itdot, gain;
 if (positive > 0)
  {
   itdot = information(positive, total);
   gain = positive * (it - itdot);
   if (dless(*bestgain, gain))
    {
     *bestgain = gain;
     result->comparison = comparison;
     result->value = value;
     result->featureindex = featureindex;
    }
  }
}

Decisionconditionptr find_best_condition_for_realized_features(Instanceptr growset, int positive)
{
 /*!Last Changed 22.01.2004*/
 int i, j, size, tpositive, t, tdotpositive, tdot, equal, equalpositive;
 Instanceptr instancelist, tmp;
 double bestgain = 0, valuebefore, value, it;
 Decisionconditionptr result;
 result = empty_condition();
 result->conditiontype = HYBRID_CONDITION;
 result->freeparam = 1;
 tmp = growset;
 tpositive = 0;
 t = 0;
 while (tmp)
  {
   t++;
   if (give_classno(tmp) == positive)
     tpositive++;
   tmp = tmp->next;
  }
 if (tpositive == 0)
  {
   safe_free(result);
   return NULL;
  }
 it = information(tpositive, t);
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
  { 
   instancelist = sort_instances(growset, i, &size);
   valuebefore = real_feature(&(instancelist[0]), i);
   tdot = 1;
   if (give_classno(&(instancelist[0])) == positive)
     tdotpositive = 1;
   else
     tdotpositive = 0;
   equalpositive = tdotpositive;
   equal = 1;
   for (j = 1; j < size; j++)
    {
     value = real_feature(&(instancelist[j]), i);
     if (value != valuebefore)
      {
       check_for_bestgain(tdotpositive, tdot, it, &bestgain, result, '<', valuebefore + DBL_EPSILON, i);
       check_for_bestgain(tpositive - tdotpositive + equalpositive, t - tdot + equal, it, &bestgain, result, '>', valuebefore - DBL_EPSILON, i);
       valuebefore = value;
       equal = 0;
       equalpositive = 0;
      }
     equal++;
     if (give_classno(&(instancelist[j])) == positive)
      {
       tdotpositive++;
       equalpositive++;
      }
     tdot++;
    }
   check_for_bestgain(equalpositive, equal, it, &bestgain, result, '>', value - DBL_EPSILON, i);
   safe_free(instancelist);
  }
 return result;
}

Decisionconditionptr find_best_condition(Instanceptr growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 17.12.1005 added result->freeparam = 1*/
 /*!Last Changed 02.02.2004 added safecalloc*/
 /*!Last Changed 21.01.2004 added empty_condition*/
 /*!Last Changed 10.05.2003*/
 int i, j, size, tpositive, t, tdotpositive, tdot, equal, equalpositive, *featurecounts, *positivecounts;
 Instanceptr instancelist, tmp;
 double bestgain = 0, valuebefore, value, it, split;
 Partition p;
 Decisionconditionptr result;
 p = get_start_partition(growset, positive);
 result = empty_condition();
 result->conditiontype = UNIVARIATE_CONDITION;
 result->freeparam = 1;
 tmp = growset;
 tpositive = 0;
 t = 0;
 while (tmp)
  {
   t++;
   if (give_classno(tmp) == positive)
     tpositive++;
   tmp = tmp->next;
  }
 if (tpositive == 0)
  {
   safe_free(result);
   return NULL;
  }
 it = information(tpositive, t);
 for (i = 0; i < current_dataset->featurecount; i++)
   switch (current_dataset->features[i].type)
    { 
     case INTEGER:
     case    REAL:if ((!param->usefisher) || (t < param->smallsetsize && param->smallset))
                   {
                    instancelist = sort_instances(growset, i, &size);
                    if (current_dataset->features[i].type == INTEGER)
                      valuebefore = instancelist[0].values[i].intvalue;
                    else
                      valuebefore = instancelist[0].values[i].floatvalue;
                    tdot = 1;
                    if (give_classno(&(instancelist[0])) == positive)
                      tdotpositive = 1;
                    else
                      tdotpositive = 0;
                    equalpositive = tdotpositive;
                    equal = 1;
                    for (j = 1; j < size; j++)
                     {
                      if (current_dataset->features[i].type == INTEGER)
                        value = instancelist[j].values[i].intvalue;
                      else
                        value = instancelist[j].values[i].floatvalue;
                      if (value != valuebefore)
                       {
                        check_for_bestgain(tdotpositive, tdot, it, &bestgain, result, '<', valuebefore + DBL_EPSILON, i);
                        check_for_bestgain(tpositive - tdotpositive + equalpositive, t - tdot + equal, it, &bestgain, result, '>', valuebefore - DBL_EPSILON, i);
                        valuebefore = value;
                        equal = 0;
                        equalpositive = 0;
                       }
                      equal++;
                      if (give_classno(&(instancelist[j])) == positive)
                       {
                        tdotpositive++;
                        equalpositive++;
                       }
                      tdot++;
                     }
                    check_for_bestgain(equalpositive, equal, it, &bestgain, result, '>', value - DBL_EPSILON, i);
                    safe_free(instancelist);
                   }
                  else
                   {
                    if (param->withoutoutliers)
                      split = find_split_for_partition_without_outliers(growset, p, i, param->conditiontype, param->variancerange);
                    else
                      split = find_split_for_partition(growset, p, i, param->conditiontype);
                    tdotpositive = 0;
                    tdot = 0;
                    tmp = growset;
                    while (tmp)
                     {
                      if (current_dataset->features[i].type == INTEGER)
                        value = tmp->values[i].intvalue;
                      else
                        value = tmp->values[i].floatvalue;
                      if (value < split)
                       {
                        tdot++;
                        if (give_classno(tmp) == positive)
                          tdotpositive++;
                       }
                      tmp = tmp->next;
                     }
                    check_for_bestgain(tdotpositive, tdot, it, &bestgain, result, '<', split, i);
                    check_for_bestgain(tpositive - tdotpositive, t - tdot, it, &bestgain, result, '>', split, i);
                   }
                  break;
     case  STRING:featurecounts = safecalloc(current_dataset->features[i].valuecount,sizeof(int), "find_best_condition", 94);
                  positivecounts = safecalloc(current_dataset->features[i].valuecount,sizeof(int), "find_best_condition", 95); 
                  tmp = growset;
                  while (tmp)
                   {
                    featurecounts[tmp->values[i].stringindex]++;
                    if (give_classno(tmp) == positive)
                      positivecounts[tmp->values[i].stringindex]++;
                    tmp = tmp->next;
                   }
                  for (j = 0; j < current_dataset->features[i].valuecount; j++)
                    check_for_bestgain(positivecounts[j], featurecounts[j], it, &bestgain, result, '=', j, i);
                  safe_free(featurecounts);
                  safe_free(positivecounts);
                  break;
    }
 free_partition(p);
 return result;
}

void send_exceptions(Decisionconditionptr condition, Instanceptr* obey, Instanceptr* notobey)
{
 /*!Last Changed 10.05.2003*/
 Instanceptr tmp, next, last, tmpbefore;
 last = *notobey;
 if (last)
   while (last->next)
     last = last->next;
 tmpbefore = NULL;
 tmp = *obey;
 while (tmp)
  {
   next = tmp->next;
   if (!condition_cover_instance(tmp, condition))
    {
     if (last)
       last->next = tmp;
     else
       (*notobey) = tmp;
     last = tmp;
    }
   else
    {
     if (tmpbefore)
       tmpbefore->next = tmp;
     else
       (*obey) = tmp;
     tmpbefore = tmp;
    }
   tmp = next;
  }
 if (last)
   last->next = NULL;
 if (tmpbefore)
   tmpbefore->next = NULL;
 else
   (*obey) = NULL;
}

double rule_value_metric(int n, int p, int N, int P, int reptype)
{
 /*!Last Changed 18.06.2003 integer division problem*/
 /*!Last Changed 19.05.2003 if p + n is zero returns inf*/
 /*!Last Changed 10.05.2003*/
 if (reptype == 0)
   return (p + N - n) / (P + N + 0.0);
 else
   if ((p + n) != 0)
     return (p - n) / (p + n + 2.0);
   else
     return 0;
}

Decisionruleptr growrule(Instanceptr* growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 21.01.2004*/
 /*!Last Changed 29.12.2003*/
 /*!Last Changed 02.12.2003*/
 /*!Last Changed 10.05.2003*/
 Decisionruleptr result;
 Decisionconditionptr newcondition = NULL;
 Instanceptr obey, notobey = NULL;
 int n, pnew, nnew;
 double bestgain;
 result = empty_rule(positive);
 obey = (*growset);
 if (rule_cover_negative(obey, *result))
   n = 1;
 else
   n = 0;
 while (n > 0)
  {
   if (get_parameter_o("usediscrete") == ON)
     if (get_parameter_i("smallsetsize") == 0)
       newcondition = find_best_discrete_condition_omnivariate(obey, positive);
     else
       newcondition = find_best_discrete_condition(obey, get_parameter_i("smallsetsize"), positive, &bestgain);
   else
     newcondition = find_best_condition(obey, positive, param);
   if (newcondition && newcondition->featureindex != -1)
    {
     count_examples_for_condition(newcondition, &pnew, &nnew, positive, obey);
     if (result->decisioncount == 0 || nnew != n)
       n = nnew;
     else
      {
       free_condition(*newcondition);
       safe_free(newcondition);
       break;
      }
     send_exceptions(newcondition, &obey, &notobey);
     add_condition(result, newcondition);
    }
   else
    {
     if (newcondition && newcondition->featureindex == -1)
      {
       free_condition(*newcondition);
       safe_free(newcondition);
      }
     break;
    }
  }
 if (notobey)
  {
   merge_instancelist(&notobey, obey);
   (*growset) = notobey;
  }
 else
   (*growset) = obey;
 return result;
}

void prunerule(Decisionruleptr rule, Instanceptr pruneset, int positive, int reptype)
{
 /*!Last Changed 02.12.2004 added free_condition and safe_free*/
 /*!Last Changed 16.06.2003 added if (P == 0) return;*/
 /*!Last Changed 10.05.2003*/
 int P = 0, N = 0, p = 0, n = 0, improved = 1;
 double v, bestv;
 Instanceptr tmp;
 Decisionconditionptr best, bestbefore, tmpcondition, tmpbefore;
 tmp = pruneset;
 while (tmp)
  {
   if (give_classno(tmp) == positive)
    {
     P++;
     if (rule_cover_instance(tmp, *rule))
       p++;
    }
   else
    {
     N++;
     if (rule_cover_instance(tmp, *rule))
       n++;
    }
   tmp = tmp->next;
  }
 if (P == 0)
   return;
 bestv = rule_value_metric(n, p, N, P, reptype);
 while (improved)
  {
   improved = 0;
   tmpbefore = NULL;
   tmpcondition = rule->start;
   while (tmpcondition)
    {
     remove_condition_after(rule, tmpbefore, tmpcondition);
     calculate_performance_for_prune(pruneset, *rule, &p, &n, positive);
     v = rule_value_metric(n, p, N, P, reptype);
     if (dless(bestv, v) || dequal(bestv, v))
      {
       bestv = v;
       bestbefore = tmpbefore;
       best = tmpcondition;
       improved = 1;
      }
     add_condition_after(rule, tmpbefore, tmpcondition);
     tmpbefore = tmpcondition;
     tmpcondition = tmpcondition->next;
    }
   if (improved)
    {
     remove_condition_after(rule, bestbefore, best);
     free_condition(*best);
     safe_free(best);
    }
  }
}

double rule_value(Decisionruleptr rule, int positive, Instanceptr instances)
{
 /*!Last Changed 01.12.2003*/
 int negative = 0, total = 0;
 Instanceptr tmp;
 tmp = instances;
 while (tmp)
  {
   if (rule_cover_instance(tmp, *rule))
    {
     if (give_classno(tmp) != positive)
       negative++;
    }
   else
     if (give_classno(tmp) == positive)
       negative++;     
   total++;
   tmp = tmp->next;
  }
 return negative/(total+0.0);
}

void remove_covered_by_ruleset(Instanceptr* list1, Instanceptr* list2, Ruleset r, int startcount)
{
 /*!Last Changed 22.05.2003*/
 Instanceptr tmp, next, last, tmpbefore;
 int covered, i = 0;
 Decisionruleptr rule, startrule;
 startrule = r.start;
 while (i < startcount)
  {
   startrule = startrule->next;
   i++;
  }
 last = *list2;
 if (last)
   while (last->next)
     last = last->next;
 tmpbefore = NULL;
 tmp = *list1;
 while (tmp)
  {
   next = tmp->next;
   rule = startrule;
   covered = 0;
   while (rule)
    {
     if (rule_cover_instance(tmp, *rule))
      {
       if (last)
         last->next = tmp;
       else
         (*list2) = tmp;
       last = tmp;
       covered = 1;
       break;
      }
     rule = rule->next;
    }
   if (!covered)
    {
     if (tmpbefore)
       tmpbefore->next = tmp;
     else
       (*list1) = tmp;
     tmpbefore = tmp;
    }
   tmp = next;
  }
 if (last)
   last->next = NULL;
 if (tmpbefore)
   tmpbefore->next = NULL;
 else
   (*list1) = NULL;
}

void remove_covered(Instanceptr* list1, Instanceptr* list2, Decisionruleptr rule)
{
 /*!Last Changed 10.05.2003*/
 Instanceptr tmp, next, last, tmpbefore;
 last = *list2;
 if (last)
   while (last->next)
     last = last->next;
 tmpbefore = NULL;
 tmp = *list1;
 while (tmp)
  {
   next = tmp->next;
   if (rule_cover_instance(tmp, *rule))
    {
     if (last)
       last->next = tmp;
     else
       (*list2) = tmp;
     last = tmp;
    }
   else
    {
     if (tmpbefore)
       tmpbefore->next = tmp;
     else
       (*list1) = tmp;
     tmpbefore = tmp;
    }
   tmp = next;
  }
 if (last)
   last->next = NULL;
 if (tmpbefore)
   tmpbefore->next = NULL;
 else
   (*list1) = NULL;
}

void update_counts_rules(Decisionruleptr start, Instanceptr list1, int* covered, int* uncovered, int* fp, int* fn)
{
 /*!Last Changed 04.11.2003*/
 Decisionruleptr tmprule;
 Instanceptr tmpinstance;
 int classno;
 tmpinstance = list1;
 while (tmpinstance)
  {
   tmprule = start;
   while (tmprule)
    {
     if (rule_cover_instance(tmpinstance, *tmprule))
      {
       (*covered)++;
       (*uncovered)--;
       classno = give_classno(tmpinstance);
       if (classno == tmprule->classno)
        {
         tmprule->covered++;
         (*fn)--;
        }
       else
        {
         (*fp)++;
         tmprule->falsepositives++;
        }
       break;
      }
     tmprule = tmprule->next;
    }
   tmpinstance = tmpinstance->next;
  }
}

int are_conditions_equal(Decisionconditionptr condition1, Decisionconditionptr condition2)
{
 if (condition1->featureindex != condition2->featureindex)
   return NO;
 if (in_list(current_dataset->features[condition1->featureindex].type, 2, INTEGER, REAL))
  {
   if (condition1->upperlimit != condition2->upperlimit || condition1->lowerlimit != condition2->lowerlimit)
     return NO;
   if (condition1->comparison != condition2->comparison)
     return NO;
  }
 else
   if (condition1->value != condition2->value)
     return NO;
 return YES;
}

int are_rules_equal(Decisionruleptr rule1, Decisionruleptr rule2)
{
 Decisionconditionptr condition1, condition2;
 if (rule1->decisioncount != rule2->decisioncount)
   return NO;
 condition1 = rule1->start;
 condition2 = rule2->start;
 while (condition1)
  {
   if (!are_conditions_equal(condition1, condition2))
     return NO;
   condition1 = condition1->next;
   condition2 = condition2->next;
  }
 return YES;
}

int rule_exists(Ruleset ruleset, Decisionruleptr rule)
{
 Decisionruleptr tmp;
 tmp = ruleset.start;
 while (tmp)
  {
   if (are_rules_equal(rule, tmp))
     return YES;
   tmp = tmp->next;
  }
 return NO;
}

int exception_count_of_ruleset(Ruleset r, Instanceptr instances, int positive)
{
 Instanceptr tmp;
 int covered, classno, exception_count = 0;
 tmp = instances;
 while (tmp != NULL)
  {
   covered = ruleset_cover_instance(tmp, r);
   classno = give_classno(tmp);
   if ((classno == positive && !covered) || (classno != positive && covered))
     exception_count++;
   tmp = tmp->next;
  }
 return exception_count;
}

int can_delete_rule(Decisionruleptr rule, int* covered, int* uncovered, int* fp, int* fn, double proportion, int possibleconditioncount)
{
 /*!Last Changed 04.12.2003*/
 int covered1, uncovered1, fp1, fn1;
 double error, dlwith, dlwithout;
 covered1 = (*covered) - (rule->falsepositives + rule->covered);
 uncovered1 = (*uncovered) + (rule->falsepositives + rule->covered);
 fp1 = (*fp) - rule->falsepositives;
 fn1 = (*fn) + rule->covered;
 dlwith = description_length_of_exceptions(*covered, *uncovered, *fp, *fn, proportion) + description_length_of_rule(*rule, possibleconditioncount);
 dlwithout = description_length_of_exceptions(covered1, uncovered1, fp1, fn1, proportion);
 error = (rule->falsepositives + 0.0) / (rule->falsepositives + rule->covered);
 if ((dlwith >= dlwithout) || (error >= 0.5))
  {
   *covered = covered1;
   *uncovered = uncovered1;
   *fp = fp1;
   *fn = fn1;
   return YES;
  }
 else
   return NO;
}

void insert_rule_and_update_counts(Decisionruleptr newrule, Ruleset* r, int* covered, int* uncovered, int* fp, int* fn)
{
 /*!Last Changed 02.11.2003*/
 add_rule(r, newrule);
 *covered += newrule->covered + newrule->falsepositives;
 *uncovered -= newrule->covered + newrule->falsepositives;
 *fp += newrule->falsepositives;
 *fn -= newrule->covered;
}
