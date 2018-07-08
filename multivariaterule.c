#include <limits.h>
#include <math.h>
#include "data.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "mlp.h"
#include "multivariatetree.h"
#include "partition.h"
#include "rule.h"
#include "univariaterule.h"
#include "multivariaterule.h"
#include "svmtree.h"

extern Datasetptr current_dataset;

int error_of_ruleset_with_deleted_rules(Ruleset set, Instanceptr data, int* deleted)
{
/*!Last Changed 14.01.2004*/
 int instanceclassno, covered, error, i;
 Instanceptr test = data;
 Decisionruleptr tmp;
 error = 0;
 while (test)
  {
   instanceclassno = give_classno(test);
   covered = 0;
   tmp = set.start;
   i = 0;
   while (tmp)
    {
     if ((!deleted[i]) && rule_cover_instance(test, *tmp))
      {
       if (instanceclassno != tmp->classno)
         error++;
       covered = 1;
       break;
      }
     i++;
     tmp = tmp->next;
    }
   if (!covered)
     if (instanceclassno != set.defaultclass)
       error++;
   test=test->next;
  }
 return error;
}

int validation_error_after_pruning(Ruleset r, Instanceptr cvdata, int positive)
{
 /*!Last Changed 02.02.2004 added safecalloc*/
 /*!Last Changed 14.01.2004*/
 int result, *deleted, rulecount, tmp, i;
 Decisionruleptr rule;
 deleted = safecalloc(r.rulecount, sizeof(int), "validation_error_after_pruning", 3);
 result = error_of_ruleset_with_deleted_rules(r, cvdata, deleted);
 rule = r.start;
 rulecount = 0;
 while (rule && rule->classno != positive)
  {
   rule = rule->next;
   rulecount++;
  }
 for (i = r.rulecount - 1; i >= rulecount; i--)
  {
   deleted[i] = 1;
   tmp = error_of_ruleset_with_deleted_rules(r, cvdata, deleted);
   if (tmp > result)
     deleted[i] = 0;
   else
     result = tmp;
  }
 safe_free(deleted);
 return result;
}

void optimize_ruleset_multivariate(Ruleset* r, int positive, Instanceptr* instances, Instanceptr cvdata, Ripper_parameterptr param)
{
 /* Last Changed 14.01.2004*/
 Decisionruleptr rule, before = NULL, replacementrule, revisionrule, next, newrule;
 Instanceptr grow, prune, removed = NULL;
 int error, revisionerror, replacementerror;
 rule = r->start;
 while (rule && rule->classno != positive)
  {
   before = rule;
   rule = rule->next;
  }
 while (rule && class_exists_in_list(*instances, positive))
  {
   error = validation_error_after_pruning(*r, cvdata, positive);
   divide_instancelist(instances, &prune, &grow, 3);
   replacementrule = growrule_multivariate(&grow, positive, param);
   remove_rule_after(r, before, rule);
   add_rule_after(r, before, replacementrule);
   if (replacementrule->decisioncount > 0)
     prunerule_in_optimize_phase(replacementrule, prune, positive);
   replacementerror = validation_error_after_pruning(*r, cvdata, positive);
   revisionrule = grow_revision_rule_multivariate(&grow, rule, positive, param);
   remove_rule_after(r, before, replacementrule);
   add_rule_after(r, before, revisionrule);
   prunerule_in_optimize_phase(revisionrule, prune, positive);
   revisionerror = validation_error_after_pruning(*r, cvdata, positive);
   if ((revisionerror < error) || (replacementerror < error))
     if (replacementerror <= revisionerror)
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
   *instances = restore_instancelist(*instances, grow);
   merge_instancelist(instances, prune);
   before = rule;
   rule = rule->next;
  }
 if (!rule)
  {
   while (class_exists_in_list(*instances, positive))
    {
     divide_instancelist(instances, &prune, &grow, 3);    
     newrule = growrule_multivariate(&grow, positive, param);
     if (newrule->start)
       prunerule(newrule, prune, positive, 2);
     update_counts(newrule, grow, 1);
     update_counts(newrule, prune, 0);
     if (!(newrule->start) || (error_rate_of_rule(*newrule) >= 0.5))
      {
       free_rule(*newrule);
       safe_free(newrule);
       *instances = restore_instancelist(*instances, grow);
       merge_instancelist(instances, prune);
       break;
      }
     add_rule(r, newrule);
     *instances = restore_instancelist(*instances, grow);
     merge_instancelist(instances, prune);     
     remove_covered(instances, &removed, newrule);
    }
  }
 else
  {
   while (rule)
    {
     next = rule->next;
     if (rule->classno == positive)
      {
       if (before)
         before->next = next;
       else
         r->start = next;
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

void simplify_ruleset_multivariate(Ruleset* r, int positive, Instanceptr cvdata)
{
 /*!Last Changed 14.01.2004*/
 Decisionruleptr firstrule, deleted, tmprule, deletedbefore;
 int minerror = error_of_ruleset(*r, cvdata), error;
 firstrule = r->start;
 while (firstrule && firstrule->classno != positive)
   firstrule = firstrule->next;
 if (!firstrule)
   return;
 deleted = r->end;
 while (deleted && deleted->next != firstrule)
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
   remove_rule_after(r, deletedbefore, deleted);
   error = error_of_ruleset(*r, cvdata);
   if (error > minerror)
     add_rule_after(r, deletedbefore, deleted);
   else
    {
     free_rule(*deleted);
     safe_free(deleted);
     minerror = error;
    }
   deleted = deletedbefore;
  }
}

int count_multivariate_condition_over_threshold(Instanceptr list, Decisionconditionptr condition)
{
 /*!Last Changed 09.01.2004*/
 int result = 0;
 Instanceptr tmp = list;
 while (tmp)
  {
   if (multiply_with_matrix(tmp, condition->w) > condition->w0)
     result++;
   tmp = tmp->next;
  }
 return result;
}

Decisionconditionptr find_best_multivariate_condition_svm(Instanceptr* growset, int positive)
{
 /*!Last Changed 18.02.2008*/
 Decisionconditionptr result;
 int i;
 Partition p;
 Svm_split split;
 result = empty_condition();
 result->conditiontype = MULTIVARIATE_CONDITION;
 result->featureindex = LINEAR;
 result->freeparam = current_dataset->multifeaturecount - 1;
 result->comparison = '>';
 result->w = matrix_alloc(1, current_dataset->multifeaturecount - 1);
 p = get_two_class_partition_for_one_vs_all(current_dataset, positive);
	split = find_best_svm_split_for_partition(*growset, p);
	result->w0 = split.rho;
	for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
 		result->w.values[0][i] = split.weights[i];
	free_svm_split(split);
 free_partition(p);
 return result;
}

Decisionconditionptr find_best_multivariate_condition_lp(Instanceptr* growset, int positive)
{
 /*!Last Changed 17.11.2005 added result->freeparam*/
 /*!Last Changed 26.06.2005*/
 /*!Last Changed 28.05.2004*/
 Mlpparameters mlpparams;
 Mlpnetwork neuralnetwork;
 matrix train;
 Decisionconditionptr result;
 int i;
 result = empty_condition();
 result->conditiontype = MULTIVARIATE_CONDITION;
 result->featureindex = LINEAR;
 result->freeparam = current_dataset->multifeaturecount - 1;
 mlpparams = prepare_Mlpparameters(*growset, *growset, 0, 0.0, 0, NULL, 0, 0.0, 0.0, 0, CLASSIFICATION);
 train = instancelist_to_matrix(*growset, 2, positive, MULTIVARIATE_LINEAR);
 neuralnetwork = mlpn_general(train, train, &mlpparams, CLASSIFICATION);
 result->comparison = '>';
 result->w0 = neuralnetwork.weights->values[1][0] - neuralnetwork.weights->values[0][0];
 result->w = matrix_alloc(1, current_dataset->multifeaturecount - 1);
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
   result->w.values[0][i] = neuralnetwork.weights->values[0][i + 1] - neuralnetwork.weights->values[1][i + 1];
 free_mlpnnetwork(&neuralnetwork);
 matrix_free(train);
 return result;
}

Decisionconditionptr find_best_multivariate_condition(Instanceptr* growset, int positive, MULTIVARIATE_TYPE multivariate_type, Ripper_parameterptr param)
{
 /*!Last Changed 18.02.2008 added linear svm*/
 /*!Last Changed 28.05.2004*/
 switch (param->multivariatealgorithm)
  {
   case  LINEAR_LDA:return find_best_multivariate_condition_lda(growset, positive, multivariate_type, param->variance_explained);
   case  LINEAR_LP :return find_best_multivariate_condition_lp(growset, positive);
   case  LINEAR_SVM:return find_best_multivariate_condition_svm(growset, positive);
   default         :return find_best_multivariate_condition_lda(growset, positive, multivariate_type, param->variance_explained);
  }
}

Decisionconditionptr find_best_multivariate_condition_lda(Instanceptr* growset, int positive, MULTIVARIATE_TYPE multivariate_type, double variance_explained)
{
 /*!Last Changed 21.09.2004 m number of features, n number of eigenvectors*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 09.01.2004*/
 int positivecount, negativecount, posover, negover, newdim, size = data_size(*growset), d = current_dataset->multifeaturecount - 1;
 Instanceptr positives = NULL, negatives = NULL;
 double it, itdot, gainover, gainunder;
 Decisionconditionptr result;
 result = empty_condition();
 result->featureindex = multivariate_type;
 result->conditiontype = MULTIVARIATE_CONDITION;
 divide_instancelist_one_vs_rest(growset, &positives, &negatives, positive);
 if (d < size)
  {
   if (!best_multivariate_split_lda_with_pca(positives, negatives, &(result->w), &(result->w0), multivariate_type, &newdim, variance_explained))
    {
     result->featureindex = -1;
     matrix_free(result->w);
     *growset = positives;
     merge_instancelist(growset, negatives);
     return result;
    }
  }
 else
  {
   if (!best_multivariate_split_lda_with_svd(positives, negatives, &(result->w), &(result->w0), multivariate_type, &newdim, variance_explained))
    {
     result->featureindex = -1;
     matrix_free(result->w);
     *growset = positives;
     merge_instancelist(growset, negatives);
     return result;
    }
  }
 result->freeparam = newdim;
 positivecount = data_size(positives);
 negativecount = data_size(negatives);
 it = information(positivecount, positivecount + negativecount);
 posover = count_multivariate_condition_over_threshold(positives, result);
 negover = count_multivariate_condition_over_threshold(negatives, result);
 if ((posover + negover) == positivecount + negativecount || (posover + negover) == 0)
  {
   result->featureindex = -1;
   matrix_free(result->w);
  }
 itdot = information(posover, posover + negover);
 gainover = posover * (it - itdot);
 itdot = information(positivecount - posover, positivecount - posover + negativecount - negover);
 gainunder = (positivecount - posover) * (it - itdot);
 if (gainover > gainunder)
   result->comparison = '>';
 else
   result->comparison = '<';
 *growset = positives;
 merge_instancelist(growset, negatives);
 return result;
}

Decisionruleptr growrule_multivariate(Instanceptr* growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 21.01.2004*/
 /*!Last Changed 29.12.2003*/
 /*!Last Changed 02.12.2003*/
 /*!Last Changed 10.05.2003*/
 Decisionruleptr result;
 Decisionconditionptr newcondition = NULL;
 Instanceptr obey, notobey = NULL;
 int n, pnew, nnew;
 result = empty_rule(positive);
 obey = (*growset);
 if (rule_cover_negative(obey, *result))
   n = 1;
 else
   n = 0;
 while (n > 0)
  {
   newcondition = find_best_multivariate_condition(&obey, positive, MULTIVARIATE_LINEAR, param);
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

Decisionruleptr grow_revision_rule_multivariate(Instanceptr* growset, Decisionruleptr rule, int positive, Ripper_parameterptr param)
{
 /* Last Changed 21.01.2004*/
 /* Last Changed 14.01.2004*/
 /* Last Changed 20.05.2003*/
 Decisionruleptr newrule = create_copy_of_rule(rule);
 Decisionconditionptr newcondition;
 Instanceptr obey, notobey = NULL;
 int n, pnew, nnew;
 obey = (*growset);
 if (rule_cover_negative(obey, *newrule))
   n = 1;
 else
   n = 0;
 while (n > 0)
  {
   newcondition = find_best_multivariate_condition(&obey, positive, MULTIVARIATE_LINEAR, param);
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

Instanceptr learn_multivariate_rules_for_positive_class(Instanceptr* instances, Ruleset* ruleset, int posclass, Instanceptr cvdata, Ripper_parameterptr param)
{
 Instanceptr grow, prune, removed = NULL;
 Decisionruleptr newrule;
 int j;
 while (class_exists_in_list(*instances, posclass))
  {
   divide_instancelist(instances, &prune, &grow, 3);    
   newrule = growrule_multivariate(&grow, posclass, param);
   if (newrule->start)
     prunerule(newrule, prune, posclass, 2);
   update_counts(newrule, grow, 1);
   update_counts(newrule, prune, 0);
   if (!(newrule->start) || (error_rate_of_rule(*newrule) >= 0.5))
    {
     free_rule(*newrule);
     safe_free(newrule);
     *instances = restore_instancelist(*instances, grow);
     merge_instancelist(instances, prune);
     break;
    }
   add_rule(ruleset, newrule);
   *instances = restore_instancelist(*instances, grow);
   merge_instancelist(instances, prune);     
   remove_covered(instances, &removed, newrule);
  }
 for (j = 0; j < param->optimizecount; j++)
  {
   merge_instancelist(instances, removed);
   removed = NULL;
   optimize_ruleset_multivariate(ruleset, posclass, instances, cvdata, param);
   simplify_ruleset_multivariate(ruleset, posclass, cvdata);
   remove_covered_by_ruleset(instances, &removed, *ruleset, 0);
  }
 return removed;
}

Ruleset multivariate_rule_learner(Instanceptr* instances, Instanceptr cvdata, Ripper_parameterptr param)
{
 /*!Last Changed 09.01.2004*/
 Ruleset result;
 int classno, *counts, *classes, posclass, classcount, i, maxcount;
 Instanceptr removed, totalremoved = NULL;
 empty_ruleset(&result);
 classno = all_in_one_class(*instances, &maxcount);
 if (!(classno % 10))
   return result;
 counts = find_class_counts(*instances);
 classes = find_max_occured(counts, current_dataset->classno, &classcount);
 safe_free(counts);
 result.defaultclass = classes[0];
 for (i = classcount - 1; i > 0; i--)
  {
   posclass = classes[i];
   removed = learn_multivariate_rules_for_positive_class(instances, &result, posclass, cvdata, param);
   merge_instancelist(&totalremoved, removed);
  }
 safe_free(classes);
 merge_instancelist(instances, totalremoved);
 return result;
}
