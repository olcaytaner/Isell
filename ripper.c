#include<limits.h>
#include<string.h>
#include "data.h"
#include "instance.h"
#include "instancelist.h"
#include "krule.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "ripper.h"
#include "rule.h"
#include "univariaterule.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;

Decisionruleptr grow_revision_rule(Instanceptr* growset, Decisionruleptr rule, int positive, Ripper_parameterptr param)
{
 /* Last Changed 21.01.2004*/
 /* Last Changed 14.01.2004*/
 /* Last Changed 20.05.2003*/
 Decisionruleptr newrule = create_copy_of_rule(rule);
 Decisionconditionptr newcondition;
 Instanceptr obey, notobey = NULL;
 int n, pnew, nnew;
 double bestgain;
 obey = (*growset);
 if (rule_cover_negative(obey, *newrule))
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
     if (notobey == NULL || nnew != n)
       n = nnew;
     else
      {
       free_condition(*newcondition);
       safe_free(newcondition);       
       break;
      }
     send_exceptions(newcondition, &obey, &notobey);
     add_condition(newrule, newcondition);
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
 return newrule;
}

void optimize_ruleset(Ruleset* r, int positive, Instanceptr* instances, double proportion, Ripper_parameterptr param)
{
 /* Last Changed 24.11.2003 If after revising and renewing rules there are instances that are not covered by ruleset learn new rules to cover them*/
 /* Last Changed 20.06.2003 Rewritten*/
 /* Last Changed 21.05.2003*/
 int first, rulecount = 0, fp, fn, covered, uncovered;
 Decisionruleptr rule, replacementrule, revisionrule, before, next, newrule;
 Instanceptr grow, prune, removed = NULL, pruneremoved = NULL;
 double replacementdl, revisiondl, dl, smallestdl, dlnow, datadlwithout, ruledl, datadlwith;
 before = NULL;
 fp = 0;
 covered = 0;
 uncovered = data_size(*instances);
 fn = find_class_count(*instances, positive);
 rule = r->start;
 while (rule && rule->classno != positive)
  {
   before = rule;
   rule = rule->next;
   rulecount++;
  }
 while (rule && fn > 0)
  {
   dl = relative_compression(r, positive, rulecount, *instances, NULL, NULL, NULL, proportion);
   divide_instancelist(instances, &prune, &grow, 3);
   pruneremoved = NULL;
   replacementrule = growrule(&grow, positive, param);
   remove_rule_after(r, before, rule);
   add_rule_after(r, before, replacementrule);
   remove_covered_by_ruleset(&prune, &pruneremoved, *r, rulecount + 1);
   if (replacementrule->decisioncount > 0)
     prunerule_in_optimize_phase(replacementrule, prune, positive);
   update_counts(replacementrule, grow, 1);
   update_counts(replacementrule, prune, 0);
   update_counts(replacementrule, pruneremoved, 0);
   replacementdl = relative_compression(r, positive, rulecount, grow, prune, pruneremoved, NULL, proportion);
   revisionrule = grow_revision_rule(&grow, rule, positive, param);
   remove_rule_after(r, before, replacementrule);
   add_rule_after(r, before, revisionrule);
   prunerule_in_optimize_phase(revisionrule, prune, positive);
   update_counts(revisionrule, grow, 1);
   update_counts(revisionrule, prune, 0);
   update_counts(revisionrule, pruneremoved, 0);
   revisiondl = relative_compression(r, positive, rulecount, grow, prune, pruneremoved, NULL, proportion);
   *instances = restore_instancelist(*instances, grow);
   merge_instancelist(instances, prune);
   merge_instancelist(instances, pruneremoved);
   if (dless(replacementdl, dl) || dless(revisiondl, dl))
     if (dless(replacementdl,revisiondl))
      {
       remove_rule_after(r, before, revisionrule);
       add_rule_after(r, before, replacementrule);
       free_rule(*revisionrule);
       safe_free(revisionrule);
       free_rule(*rule);
       safe_free(rule);
       remove_covered(instances, &removed, replacementrule);
       rule = replacementrule;
      }
     else
      {
       free_rule(*replacementrule);
       safe_free(replacementrule);
       free_rule(*rule);
       safe_free(rule);       
       remove_covered(instances, &removed, revisionrule);
       rule = revisionrule;
      }
   else
    {
     remove_rule_after(r, before, revisionrule);
     add_rule_after(r, before, rule);
     free_rule(*revisionrule);
     safe_free(revisionrule);
     free_rule(*replacementrule);
     safe_free(replacementrule);
     remove_covered(instances, &removed, rule);
    }
   covered += (rule->falsepositives + rule->covered);
   uncovered -= (rule->falsepositives + rule->covered);
   fp += rule->falsepositives;
   fn -= rule->covered;
   before = rule;
   rule = rule->next;
   rulecount++;
  }
 if (!rule)
  {
   smallestdl = +INT_MAX;
   first = 1;
   while (fn > 0)
    {
     divide_instancelist(instances, &prune, &grow, 3);
     newrule = growrule(&grow, positive, param);
     if (newrule->start)
       prunerule(newrule, prune, positive, 2);
     if (!(newrule->start))
      {
       *instances = restore_instancelist(*instances, grow);
       merge_instancelist(instances, prune);
       free_rule(*newrule);
       safe_free(newrule);
       break;
      }
     update_counts(newrule, grow, 1);
     update_counts(newrule, prune, 0);
     *instances = restore_instancelist(*instances, grow);
     merge_instancelist(instances, prune);
     datadlwithout = description_length_of_exceptions(covered, uncovered, fp, fn, proportion);
     ruledl = description_length_of_rule(*newrule, r->possibleconditioncount);
     datadlwith = description_length_of_exceptions(covered + newrule->covered + newrule->falsepositives, uncovered - newrule->covered - newrule->falsepositives, fp + newrule->falsepositives, fn - newrule->covered, proportion);
     if (first)
       dlnow = datadlwith + ruledl;
     else
       dlnow = dlnow - datadlwithout + datadlwith + ruledl;
     if (dless(dlnow,smallestdl) && (first != 1 || error_rate_of_rule(*newrule) < 0.5))
      {
       smallestdl = dlnow;
       insert_rule_and_update_counts(newrule, r, &covered, &uncovered, &fp, &fn);
       remove_covered(instances, &removed, newrule);
      }
     else
       if ((dlnow - smallestdl > 64) || (error_rate_of_rule(*newrule) >= 0.5))
        {
         free_rule(*newrule);
         safe_free(newrule);
         break;
        }
       else
        {
         insert_rule_and_update_counts(newrule, r, &covered, &uncovered, &fp, &fn);
         remove_covered(instances, &removed, newrule);
        }
     first = 0;
    }
  }
 else
  {
   while (rule)
    {
     next = rule->next;
     if (rule->classno == positive)
      {
       before->next = next;
       free_rule(*rule);
       safe_free(rule);
       r->rulecount--;
       if (next == NULL)
         r->end = before;
      }
     else
       before = rule;
     rule = next;
    }
   if (before)
     before->next = NULL;
  }
 merge_instancelist(instances, removed);
}

void simplify_ruleset(Ruleset* r, int positive, Instanceptr list1, Instanceptr list2, double proportion)
{
 /*!Last Changed 04.12.2003 Rewritten*/
 /*!Last Changed 31.05.2003 Delete rules starting from the last rule until the first rule*/
 /*!Last Changed 19.05.2003 If deleted is the first rule error occurs*/
 /*!Last Changed 18.05.2003*/
 int covered = 0, uncovered, fp = 0, fn, needrecount = 0;
 Decisionruleptr deleted, deletedbefore, tmprule;
 uncovered = data_size(list1) + data_size(list2);
 fn = find_class_count(list1, positive) + find_class_count(list2, positive);
 tmprule = r->start;
 while (tmprule && tmprule->classno != positive)
   tmprule = tmprule->next;
 while (tmprule)
  {
   uncovered -= (tmprule->covered + tmprule->falsepositives);
   covered += (tmprule->covered + tmprule->falsepositives);
   fn -= tmprule->covered;
   fp += tmprule->falsepositives;
   tmprule = tmprule->next;
  }
 deleted = r->end;
 while (deleted)
  {
   tmprule = r->start;
   if (tmprule != deleted)
    {
     while (tmprule->next != deleted)
       tmprule = tmprule->next;
     deletedbefore = tmprule;
    }
   else
     deletedbefore = NULL;
   if (deleted->classno == positive)
     if (can_delete_rule(deleted, &covered, &uncovered, &fp, &fn, proportion, r->possibleconditioncount))
      {
       remove_rule_after(r, deletedbefore, deleted);
       free_rule(*deleted);
       safe_free(deleted);
       needrecount = 1;
      }
   deleted = deletedbefore;
  }
 if (needrecount)
  {
   fp = 0;
   covered = 0;
   uncovered = data_size(list1) + data_size(list2);
   fn = find_class_count(list1, positive) + find_class_count(list2, positive);
   tmprule = r->start;
   while (tmprule && tmprule->classno != positive)
     tmprule = tmprule->next;
   if (tmprule)
    {
     initialize_counts(tmprule);
     update_counts_rules(tmprule, list1, &covered, &uncovered, &fp, &fn);
     update_counts_rules(tmprule, list2, &covered, &uncovered, &fp, &fn);
    }
  }
}

Ruleset learn_ruleset_ripper(Instanceptr* instances, int reptype, Ripper_parameterptr param)
{
 /*!Last Changed 02.12.2004 added free_rule and safe_rule*/
 /*!Last Changed 05.05.2004 added unordered classes*/
 /*!Last Changed 03.05.2004 added given permutation*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 18.05.2003*/
 Ruleset result;
 Decisionruleptr newrule;
 BOOLEAN givenperm;
 int* counts, classno, posclass, classcount, *classes, i, j, rcount, first, covered, uncovered, fp, fn, totalinstance, maxcount, lastclass;
 double smallestdl, dlnow, ruledl, *proportions, datadlwith, datadlwithout;
 Instanceptr grow, prune, removed = NULL, totalremoved = NULL, tmp;
 if (strlen(param->classpermutation) == 0)
   givenperm = BOOLEAN_FALSE;
 else
   givenperm = BOOLEAN_TRUE;
 empty_ruleset(&result);
 if (get_parameter_o("usediscrete") == OFF)
   result.possibleconditioncount = possible_condition_count(*instances);
 else
   if (get_parameter_i("smallsetsize") == 0)
     result.possibleconditioncount = find_discrete_split_count(get_parameter_i("hybridspace"));
   else
     result.possibleconditioncount = find_discrete_split_count(get_parameter_i("smallsetsize"));
 classno = all_in_one_class(*instances, &maxcount);
 if (!(classno % 10))
   return result;
 if (reptype < 2)
   rcount = 0;
 else
   rcount = param->optimizecount;
 counts = find_class_counts(*instances);
 if (!givenperm)
   classes = find_max_occured(counts, current_dataset->classno, &classcount);
 else
   classcount = current_dataset->classno;
 proportions = safemalloc(current_dataset->classno*sizeof(double), "learn_ruleset_ripper", 24);
 if (!givenperm)
   result.defaultclass = classes[0];
 else
   result.defaultclass = param->classpermutation[classcount - 1] - '0';
 totalinstance = data_size(*instances);
 uncovered = data_size(*instances);
 if (param->rulesordered)
   lastclass = 1;
 else
   lastclass = 0;
 for (i = classcount - 1; i > lastclass - 1; i--)
  {
   if (!givenperm)
     posclass = classes[i];
   else
     posclass = param->classpermutation[classcount - i - 1] - '0';
   proportions[i] = (counts[posclass] + 0.0) / totalinstance;
   smallestdl = +INT_MAX;
   first = 1;
   covered = 0;
   fp = 0;
   fn = find_class_count(*instances, posclass);
   while (fn > 0)
    {
     divide_instancelist(instances, &prune, &grow, 3);
     newrule = growrule(&grow, posclass, param);
     if (newrule->start)
       prunerule(newrule, prune, posclass, reptype);
     if (!(newrule->start))
      {
       *instances = restore_instancelist(*instances, grow);
       merge_instancelist(instances, prune);
       free_rule(*newrule);
       safe_free(newrule);
       break;
      }
     update_counts(newrule, grow, 1);
     update_counts(newrule, prune, 0);
     if (reptype == 0)
      {
       if (test_rule(*newrule, prune) >= 0.5)
        {
         *instances = restore_instancelist(*instances, grow);
         merge_instancelist(instances, prune);
         free_rule(*newrule);
         safe_free(newrule);
         break;
        }
       else
        {
         *instances = restore_instancelist(*instances, grow);
         merge_instancelist(instances, prune);
         add_rule(&result, newrule);
         remove_covered(instances, &removed, newrule);
        }
      }
     else
      {
       *instances = restore_instancelist(*instances, grow);
       merge_instancelist(instances, prune);
       datadlwithout = description_length_of_exceptions(covered, uncovered, fp, fn, proportions[i]);
       ruledl = description_length_of_rule(*newrule, result.possibleconditioncount);
       datadlwith = description_length_of_exceptions(covered + newrule->covered + newrule->falsepositives, uncovered - newrule->covered - newrule->falsepositives, fp + newrule->falsepositives, fn - newrule->covered, proportions[i]);
       if (first)
         dlnow = datadlwith + ruledl;
       else
         dlnow = dlnow - datadlwithout + datadlwith + ruledl;
       if (dless(dlnow,smallestdl) && (first != 1 || error_rate_of_rule(*newrule) < 0.5))
        {
         smallestdl = dlnow;
         insert_rule_and_update_counts(newrule, &result, &covered, &uncovered, &fp, &fn);
         remove_covered(instances, &removed, newrule);
        }
       else
         if ((dlnow - smallestdl > 64) || (error_rate_of_rule(*newrule) >= 0.5))
          {
           free_rule(*newrule);
           safe_free(newrule);
           break;
          }
         else
          {
           insert_rule_and_update_counts(newrule, &result, &covered, &uncovered, &fp, &fn);
           remove_covered(instances, &removed, newrule);
          }
      }
     first = 0;
    }
   for (j = 0; j < rcount; j++)
    {
     merge_instancelist(instances, removed);
     removed = NULL;
     optimize_ruleset(&result, posclass, instances, proportions[i], param);
     simplify_ruleset(&result, posclass, *instances, removed, proportions[i]);
     remove_covered_by_ruleset(instances, &removed, result, 0);
    }
   if (param->rulesordered)
     merge_instancelist(&totalremoved, removed);
   else
    {
     if (param->rexenabled)
      {
       tmp = *instances;
       while (tmp)
        {
         if (give_classno(tmp) == posclass)
           tmp->isexception = 1;
         tmp = tmp->next;
        }
      }
     merge_instancelist(instances, removed);
    }
   removed = NULL;
  }
 if (param->rulesordered)
   merge_instancelist(instances, totalremoved);
 safe_free(counts);
 safe_free(proportions);
 if (!givenperm)
   safe_free(classes);
 return result;
}

