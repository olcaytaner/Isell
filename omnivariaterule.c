#include "data.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "model.h"
#include "multivariaterule.h"
#include "omnivariaterule.h"
#include "rule.h"
#include "statistics.h"
#include "tests.h"
#include "univariaterule.h"

extern Datasetptr current_dataset;

/**
 * Generate a string of model types in the conditions of the rules in a ruleset. For univariate model 'U', for multivariate linear model 'L' and for multivariate quadratic model 'Q' character is used. The rules are separated via semicolons. 
 * @param[in] r Ruleset
 * @param[out] st Output string. For example the string U;LUL;QL says that there are three rules in the ruleset. The first rule has one univariate condition. Second rule has the first and third conditions multivariate linear and the second univariate. Third rule has two conditions, where the first condition is quadratic and the second is linear.
 */
void write_nodetypes_rule(Ruleset r, char* st)
{
 /*!Last Changed 19.02.2005*/
 /*!Last Changed 24.01.2004*/
 Decisionruleptr tmprule;
 Decisionconditionptr tmpcondition;
 sprintf(st, "%s", "");
 tmprule = r.start;
 while (tmprule)
  {
   tmpcondition = tmprule->start;
   while (tmpcondition)
    {     
     if (tmpcondition->featureindex == LINEAR)
       sprintf(st, "%sL", st);
     else
       if (tmpcondition->featureindex == QUADRATIC)
         sprintf(st, "%sQ", st);
       else 
         if (tmpcondition->featureindex == UNIVARIATE)
           sprintf(st, "%sU", st);
         else
           if (tmpcondition->featureindex == DISCRETEMIX)
             sprintf(st, "%s%c", st, 'A' + tmpcondition->ksplit.featurecount - 1);
           else
             sprintf(st, "%sU", st);
     tmpcondition = tmpcondition->next;
    }
   sprintf(st, "%s-%d;", st, tmprule->classno);
   tmprule = tmprule->next;
  }
}

/**
 * Generate a string of model types and number of examples covered in the conditions of the rules in a ruleset. For univariate model 'U', for multivariate linear model 'L' and for multivariate quadratic model 'Q' character is used. The rules are separated via semicolons from each other. The model characters and the number of examples covered are separated via colons. The conditions in a rule are separated by '-' from each other.
 * @param[in] r Ruleset
 * @param[out] st Output string. For example the string 20,U;10,L-40,U-5,L;23,Q-60,L says that there are three rules in the ruleset. The first rule has one univariate condition which covers 20 instances. Second rule has the first and third conditions multivariate linear and the second univariate covering 10 and 5 instances respectively. Third rule has two conditions, where the first condition is quadratic covering 23 and the second is linear covering 60 instances.
 */
void write_datasizes_and_nodetypes_rule(Ruleset r, char* st)
{
 /*!Last Changed 19.02.2005*/
 /*!Last Changed 24.01.2004*/
 Decisionruleptr tmprule;
 Decisionconditionptr tmpcondition;
 sprintf(st, "%s", "");
 tmprule = r.start;
 while (tmprule)
  {
   tmpcondition = tmprule->start;
   while (tmpcondition)
    {
     sprintf(st,"%s-%d,",st,tmpcondition->datasize);
     if (tmpcondition->featureindex == LINEAR)
       sprintf(st,"%sL",st);
     else 
       if (tmpcondition->featureindex == QUADRATIC)         
         sprintf(st,"%sQ",st);
       else
         sprintf(st,"%sU",st);
     tmpcondition = tmpcondition->next;
    }
   sprintf(st,"%s;",st);
   tmprule = tmprule->next;
  }
}

/**
 * Calculates the loglikelihood of a set of positive and negative instances.
 * @param[in] poscount Number of positive examples in the set
 * @param[in] negcount Number of negative examples in the set
 * @return Loglikelihood of a set containing positive and negative instances (two class)
 */
double log_likelihood_for_rule_induction(int poscount, int negcount)
{
 /*!Last Changed 20.02.2005*/
 int sum;
 double ratio;
 sum = poscount + negcount;
 if (sum * poscount * negcount == 0)
   return 0;
 ratio = poscount / (sum + 0.0);
 return poscount * log2(ratio);
}

/**
 * Calculates loglikelihood of a condition in a rule.
 * @param[in] c The condition
 * @param[in] list Link list of instances
 * @param[in] positive Index of the positive class
 * @param[out] covered Number of positive instances in the list
 * @return Loglikelihood of a condition
 */
double log_likelihood_of_condition(Decisionconditionptr c, Instanceptr list, int positive, int* covered)
{
 /*!Last Changed 19.02.2005*/
 int poscovered = 0, negcovered = 0, posuncovered = 0, neguncovered = 0, classno;
 Instanceptr tmp;
 tmp = list;
 while (tmp)
  {
   classno = give_classno(tmp);
   if (condition_cover_instance(tmp, c))
     if (classno == positive)
       poscovered++;
     else
       negcovered++;
   else
     if (classno == positive)
       posuncovered++;
     else
       neguncovered++;
   tmp = tmp->next;
  }
 *covered = poscovered + posuncovered;
 return log_likelihood_for_rule_induction(poscovered, negcovered) + log_likelihood_for_rule_induction(posuncovered, neguncovered);
}

/**
 * Calculates loglikelihood of a rule in a ruleset.
 * @param[in] c The rule
 * @param[in] list Link list of instances
 * @param[out] covered Number of positive instances in the list
 * @return Loglikelihood of a rule
 */
double log_likelihood_of_rule(Decisionruleptr c, Instanceptr list, int* covered)
{
 /*!Last Changed 19.02.2005*/
 int poscovered = 0, negcovered = 0, posuncovered = 0, neguncovered = 0, classno;
 Instanceptr tmp;
 tmp = list;
 while (tmp)
  {
   classno = give_classno(tmp);
   if (rule_cover_instance(tmp, *c))
     if (classno == c->classno)
       poscovered++;
     else
       negcovered++;
   else
     if (classno == c->classno)
       posuncovered++;
     else
       neguncovered++;
   tmp = tmp->next;
  }
 *covered = poscovered + posuncovered;
 return log_likelihood_for_rule_induction(poscovered, negcovered) + log_likelihood_for_rule_induction(posuncovered, neguncovered);
}

/**
 * Calculates loglikelihood of a ruleset.
 * @param[in] set The ruleset
 * @param[in] list Link list of instances
 * @param[in] positive Index of the positive class
 * @param[out] covered Number of positive instances in the list
 * @return Loglikelihood of a ruleset
 */
double log_likelihood_of_ruleset(Ruleset set, Instanceptr list, int positive, int* covered)
{
 /*!Last Changed 19.02.2005*/
 int *poscounts, *negcounts, posuncovered = 0, neguncovered = 0, classno, i, iscovered;
 Instanceptr tmp;
 Decisionruleptr rule;
 double result = 0;
 poscounts = safecalloc(set.rulecount, sizeof(int), "log_likelihood_of_ruleset", 5);
 negcounts = safecalloc(set.rulecount, sizeof(int), "log_likelihood_of_ruleset", 6);
 tmp = list;
 while (tmp)
  {
   rule = set.start;
   i = 0;
   classno = give_classno(tmp);
   iscovered = 0;
   while (rule)
    {
     if (rule_cover_instance(tmp, *rule))
      {
       iscovered = 1;
       if (classno == rule->classno)
         poscounts[i]++;
       else
         negcounts[i]++;
       break;
      }
     i++;
     rule = rule->next;
    }
   if (!iscovered)
    {
     if (classno == positive)
       posuncovered++;
     else
       neguncovered++;
    }
   tmp = tmp->next;
  }
 *covered = 0;
 for (i = 0; i < set.rulecount; i++)
  {
   *covered += poscounts[i];
   result += log_likelihood_for_rule_induction(poscounts[i], negcounts[i]);
  }
 *covered += posuncovered;
 result += log_likelihood_for_rule_induction(posuncovered, neguncovered);
 safe_free(poscounts);
 safe_free(negcounts);
 return result;
}

double generalization_error_of_ruleset_with_deleted_rules(Ruleset set, Instanceptr data, int* deleted, int positive, int modelselectionmethod)
{
 /*!Last Changed 20.02.2005*/
 Instanceptr test = data;
 Decisionruleptr tmp;
 int complexity = 0, i, *poscounts, *negcounts, posuncovered = 0, neguncovered = 0, classno, iscovered, covered;
 double loglikelihood, result;
 tmp = set.start;
 i = 0;
 while (tmp)
  {
   if ((!deleted[i]) && tmp->classno == positive)
     complexity += complexity_of_rule(tmp);
   tmp = tmp->next;
   i++;
  }
 poscounts = safecalloc(i, sizeof(int), "generalization_error_of_ruleset_with_deleted_rules", 14); 
 negcounts = safecalloc(i, sizeof(int), "generalization_error_of_ruleset_with_deleted_rules", 15); 
 while (test)
  {
   classno = give_classno(test);
   tmp = set.start;
   i = 0;
   iscovered = 0;
   while (tmp)
    {
     if (!(deleted[i]) && rule_cover_instance(test, *tmp))
      {
       iscovered = 1;
       if (tmp->classno == classno)
         poscounts[i]++;
       else
         negcounts[i]++;
       break;
      }
     i++;
     tmp = tmp->next;
    }
   if (!iscovered)
    {
     if (classno == positive)
       posuncovered++;
     else
       neguncovered++;
    }
   test = test->next;
  }
 loglikelihood = 0;
 covered = 0;
 for (i = 0; i < set.rulecount; i++)
   if (!(deleted[i]))
    {
     covered += poscounts[i];
     loglikelihood += log_likelihood_for_rule_induction(poscounts[i], negcounts[i]);
    }
 loglikelihood += log_likelihood_for_rule_induction(posuncovered, neguncovered);
 covered += posuncovered;
 switch (modelselectionmethod)
  {
   case MODEL_SELECTION_AIC:result = aic_loglikelihood(loglikelihood, complexity);
                            break;
   case MODEL_SELECTION_BIC:result = bic_loglikelihood(loglikelihood, complexity, covered);
                            break;
  }
 safe_free(poscounts);
 safe_free(negcounts);
 return result;
}

double generalization_error_after_pruning(Ruleset r, Instanceptr data, int positive, int modelselectionmethod)
{
 /*!Last Changed 19.02.2005*/
 int *deleted, rulecount, i;
 double result, tmp;
 Decisionruleptr rule;
 deleted = safecalloc(r.rulecount, sizeof(int), "generalization_error_after_pruning", 4);
 result = generalization_error_of_ruleset_with_deleted_rules(r, data, deleted, positive, modelselectionmethod);
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
   tmp = generalization_error_of_ruleset_with_deleted_rules(r, data, deleted, positive, modelselectionmethod);
   if (tmp > result)
     deleted[i] = 0;
   else
     result = tmp;
  }
 safe_free(deleted);
 return result;
}

void optimize_ruleset_modelselection(Ruleset* r, int positive, Instanceptr* instances, Ripper_parameterptr param)
{
 /* Last Changed 19.02.2005*/
 Decisionruleptr rule, before = NULL, replacementrule, revisionrule, next, newrule;
 Instanceptr removed = NULL, newsample;
 double error, revisionerror, replacementerror;
 rule = r->start;
 while (rule && rule->classno != positive)
  {
   before = rule;
   rule = rule->next;
  }
 while (rule && class_exists_in_list(*instances, positive))
  {
   error = generalization_error_after_pruning(*r, *instances, positive, param->modelselectionmethod);
   newsample = bootstrap_sample_without_stratification(*instances);
   replacementrule = growrule_omnivariate(&newsample, positive, param);
   remove_rule_after(r, before, rule);
   add_rule_after(r, before, replacementrule);
   if (replacementrule->decisioncount > 0)
     prunerule_with_modelselection(replacementrule, newsample, param->modelselectionmethod);
   deallocate(newsample);
   replacementerror = generalization_error_after_pruning(*r, *instances, positive, param->modelselectionmethod);
   revisionrule = grow_revision_rule_omnivariate(instances, rule, positive, param);
   remove_rule_after(r, before, replacementrule);
   add_rule_after(r, before, revisionrule);
   prunerule_with_modelselection(revisionrule, *instances, param->modelselectionmethod);
   revisionerror = generalization_error_after_pruning(*r, *instances, positive, param->modelselectionmethod);
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
   before = rule;
   rule = rule->next;
  }
 if (!rule)
  {
   while (class_exists_in_list(*instances, positive))
    {
     newrule = growrule_omnivariate(instances, positive, param);
     if (newrule->start)
       prunerule_with_modelselection(newrule, *instances, param->modelselectionmethod);
     update_counts(newrule, *instances, 1);
     if (!(newrule->start) || (error_rate_of_rule(*newrule) >= 0.5))
      {
       free_rule(*newrule);
       safe_free(newrule);
       break;
      }
     add_rule(r, newrule);
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

void simplify_ruleset_modelselection(Ruleset* r, int positive, Instanceptr instances, int modelselectionmethod)
{
 /*!Last Changed 19.02.2005*/
 Decisionruleptr firstrule, deleted, tmprule, deletedbefore;
 double minerror, error, loglikelihood;
 int complexity, covered;
 complexity = complexity_of_ruleset(*r, positive);
 loglikelihood = log_likelihood_of_ruleset(*r, instances, positive, &covered);
 switch (modelselectionmethod)
  {
   case MODEL_SELECTION_AIC:minerror = aic_loglikelihood(loglikelihood, complexity);
                            break;
   case MODEL_SELECTION_BIC:minerror = bic_loglikelihood(loglikelihood, complexity, covered);
                            break;
  }
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
   complexity = complexity_of_ruleset(*r, positive);
   loglikelihood = log_likelihood_of_ruleset(*r, instances, positive, &covered);
   switch (modelselectionmethod)
    {
     case MODEL_SELECTION_AIC:error = aic_loglikelihood(loglikelihood, complexity);
                              break;
     case MODEL_SELECTION_BIC:error = bic_loglikelihood(loglikelihood, complexity, covered);
                              break;
    }
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

Decisionconditionptr find_best_hybrid_condition_aic(Instanceptr* growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 19.02.2005*/
 int hybridspace = get_parameter_i("hybridspace");
 double loglikelihood, results[3];
 int bestmodel, i, covered;
 Decisionconditionptr conditions[3];
 for (i = 0; i < hybridspace; i++)
  {
   switch (i)
    {
     case MODEL_UNI:conditions[0] = find_best_condition_for_realized_features(*growset, positive);
                    break;
     case MODEL_LIN:conditions[1] = find_best_multivariate_condition(growset, positive, MULTIVARIATE_LINEAR, param);
                    break;
     case MODEL_QUA:conditions[2] = find_best_multivariate_condition(growset, positive, MULTIVARIATE_QUADRATIC, param);
                    break;
    }
   if (conditions[i])
    {
     loglikelihood = log_likelihood_of_condition(conditions[i], *growset, positive, &covered);
     results[i] = aic_loglikelihood(loglikelihood, conditions[i]->freeparam + 1);
    }
   else
     results[i] = -1;
  }
 bestmodel = find_min_in_list_double(results, hybridspace);
 if (bestmodel == -1)
  {
   return NULL;
  }
 else
  {
   for (i = 0; i < hybridspace; i++)
     if (i != bestmodel && conditions[i])
      {
       free_condition(*(conditions[i]));
       safe_free(conditions[i]);
      }
   conditions[bestmodel]->datasize = data_size(*growset);
   conditions[bestmodel]->conditiontype = HYBRID_CONDITION;
   return conditions[bestmodel];
  }
}

Decisionconditionptr find_best_hybrid_condition_bic(Instanceptr* growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 19.02.2005*/
 int hybridspace = get_parameter_i("hybridspace");
 double loglikelihood, results[3];
 int bestmodel, i, covered;
 Decisionconditionptr conditions[3];
 for (i = 0; i < hybridspace; i++)
  {
   switch (i)
    {
     case MODEL_UNI:conditions[0] = find_best_condition_for_realized_features(*growset, positive);
                    break;
     case MODEL_LIN:conditions[1] = find_best_multivariate_condition(growset, positive, MULTIVARIATE_LINEAR, param);
                    break;
     case MODEL_QUA:conditions[2] = find_best_multivariate_condition(growset, positive, MULTIVARIATE_QUADRATIC, param);
                    break;
    }
   if (conditions[i])
    {
     loglikelihood = log_likelihood_of_condition(conditions[i], *growset, positive, &covered);
     results[i] = bic_loglikelihood(loglikelihood, conditions[i]->freeparam + 1, covered);
    }
   else
     results[i] = -1;
  }
 bestmodel = find_min_in_list_double(results, hybridspace);
 if (bestmodel == -1)
  {
   return NULL;
  }
 else
  {
   for (i = 0; i < hybridspace; i++)
     if (i != bestmodel && conditions[i])
      {
       free_condition(*(conditions[i]));
       safe_free(conditions[i]);
      }
   conditions[bestmodel]->datasize = data_size(*growset);
   conditions[bestmodel]->conditiontype = HYBRID_CONDITION;
   return conditions[bestmodel];
  }
}

Decisionconditionptr find_best_hybrid_condition(Instanceptr* growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 19.02.2005*/
 switch (param->modelselectionmethod)
  {
   case MODEL_SELECTION_CV :return find_best_hybrid_condition_crossvalidation(growset, positive, param);
                            break;
   case MODEL_SELECTION_AIC:return find_best_hybrid_condition_aic(growset, positive, param);
                            break;
   case MODEL_SELECTION_BIC:return find_best_hybrid_condition_bic(growset, positive, param);
                            break;
   default                 :return find_best_hybrid_condition_crossvalidation(growset, positive, param);
                            break;
  }
}

Decisionconditionptr find_best_hybrid_condition_crossvalidation(Instanceptr* growset, int positive, Ripper_parameterptr param)
{
 /*!Last Changed 30.01.2004 added denom == 0 condition*/
 /*!Last Changed 22.01.2004*/
 int hybridspace = get_parameter_i("hybridspace");
 Decisionconditionptr result, condition;
 char **files, st[READLINELENGTH] = "-", pst[READLINELENGTH], buffer[READLINELENGTH];
 Instanceptr train = NULL, test = NULL, traindata, testdata;
 int bestmodel = 0, error, trainsize, testsize, *models, dummy, dummy2, i, j, k, datasize, testtype;
 FILE* errorfiles[3];
 files = safemalloc(hybridspace * sizeof(char *), "find_best_hybrid_ldt_split_crossvalidation", 6); 
 strcpy(buffer, get_parameter_s("tempdirectory"));
 strcat(buffer, "/univariate.txt");
 files[0] = strcopy(files[0], buffer);
 strcpy(buffer, get_parameter_s("tempdirectory"));
 strcat(buffer, "/linear.txt");
 files[1] = strcopy(files[1], buffer);
 if (hybridspace == 3)
  {
   strcpy(buffer, get_parameter_s("tempdirectory"));
   strcat(buffer, "/quadratic.txt");
   files[2] = strcopy(files[2], buffer);
  }
 for (i = 0; i < hybridspace; i++)
  {
   errorfiles[i] = fopen(files[i], "w");
   if (!errorfiles[i])
     return NULL;
  }
 for (i = 0; i < 5; i++)
  {
   divide_instancelist(growset, &train, &test, 2);
   trainsize = data_size(train);
   testsize = data_size(test);
   if (trainsize <= 1 || testsize <= 1)
    {
     *growset = restore_instancelist(*growset, train);
     merge_instancelist(growset, test); 
     bestmodel = 1;
     break;
    }
   for (j = 0; j < 2; j++)
    {
     if (j == 0)
      {
       traindata = train;
       testdata = test;
       datasize = testsize;
      }
     else
      {
       traindata = test;
       testdata = train;
       datasize = trainsize;
      }
     for (k = 0; k < hybridspace; k++)
      {
       switch (k)
        {
         case MODEL_UNI:condition = find_best_condition_for_realized_features(traindata, positive);
                        break;
         case MODEL_LIN:condition = find_best_multivariate_condition(&traindata, positive, MULTIVARIATE_LINEAR, param);
                        break;
         case MODEL_QUA:condition = find_best_multivariate_condition(&traindata, positive, MULTIVARIATE_QUADRATIC, param);
                        break;
        }
       error = return_error_and_free_condition(condition, testdata, positive);
       set_precision(pst, NULL, "\n");
       fprintf(errorfiles[k], pst, error / (datasize + 0.0));
      }
    }
   *growset = restore_instancelist(*growset, testdata);
   merge_instancelist(growset, traindata); 
   train = NULL;
   test = NULL;
  }
 for (i = 0; i < hybridspace; i++)
   fclose(errorfiles[i]);
 if (bestmodel)
  {
   free_2d((void**)files, hybridspace);
   result = find_best_condition_for_realized_features(*growset, positive);
   if (result)
     result->datasize = trainsize + testsize;
   return result;
  }
 testtype = get_parameter_l("testtype");
 if (in_list(testtype, 9, FTEST, PAIREDT5X2, COMBINEDT5X2, PERMUTATIONTEST, PAIREDPERMUTATIONTEST, SIGNTEST, RANKSUMTEST, SIGNEDRANKTEST, NOTEST))
   models = multiple_comparison(files, hybridspace, testtype, st, &dummy, &dummy2, param->correctiontype);
 else
   models = multiple_comparison(files, hybridspace, FTEST, st, &dummy, &dummy2, param->correctiontype);
 switch (models[0])
  {
   case 1:result = find_best_condition_for_realized_features(*growset, positive);
          break;
   case 2:result = find_best_multivariate_condition(growset, positive, MULTIVARIATE_LINEAR, param);
          break;
   case 3:result = find_best_multivariate_condition(growset, positive, MULTIVARIATE_QUADRATIC, param);
          break;
  }
 safe_free(models);
 free_2d((void**)files, hybridspace);
 if (result)
  {
   result->conditiontype = HYBRID_CONDITION;
   result->datasize = trainsize + testsize;
  }
 return result;
}

void prunerule_with_modelselection(Decisionruleptr rule, Instanceptr dataset, int modelselectionmethod)
{
 /*!Last Changed 19.02.2005*/
 double bestgen, gen, loglikelihood;
 int complexity, improved = 1, covered;
 Decisionconditionptr best, bestbefore, tmpcondition, tmpbefore;
 complexity = complexity_of_rule(rule);
 loglikelihood = log_likelihood_of_rule(rule, dataset, &covered);
 switch (modelselectionmethod)
  {
   case MODEL_SELECTION_AIC:bestgen = aic_loglikelihood(loglikelihood, complexity);
                            break;
   case MODEL_SELECTION_BIC:bestgen = bic_loglikelihood(loglikelihood, complexity, covered);
                            break;
  }
 while (improved)
  {
   improved = 0;
   tmpbefore = NULL;
   tmpcondition = rule->start;
   while (tmpcondition)
    {
     remove_condition_after(rule, tmpbefore, tmpcondition);
     complexity = complexity_of_rule(rule);
     loglikelihood = log_likelihood_of_rule(rule, dataset, &covered);
     switch (modelselectionmethod)
      {
       case MODEL_SELECTION_AIC:gen = aic_loglikelihood(loglikelihood, complexity);
                                break;
       case MODEL_SELECTION_BIC:gen = bic_loglikelihood(loglikelihood, complexity, covered);
                                break;
      } 
     if (gen < bestgen)
      {
       bestgen = gen;
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

Decisionruleptr growrule_omnivariate(Instanceptr* growset, int positive, Ripper_parameterptr param)
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
   newcondition = find_best_hybrid_condition(&obey, positive, param);
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

Decisionruleptr grow_revision_rule_omnivariate(Instanceptr* growset, Decisionruleptr rule, int positive, Ripper_parameterptr param)
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
   newcondition = find_best_hybrid_condition(&obey, positive, param);
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

Ruleset hybrid_rule_learner(Instanceptr* instances, Instanceptr cvdata, Ripper_parameterptr param)
{
 /*!Last Changed 19.02.2005*/
 Ruleset result;
 int classno, *counts, *classes, posclass, classcount, i, j, maxcount;
 Instanceptr grow, prune, removed = NULL, totalremoved = NULL;
 Decisionruleptr newrule;
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
   while (class_exists_in_list(*instances, posclass))
    {
     if (param->modelselectionmethod == 0)
      {
       divide_instancelist(instances, &prune, &grow, 3);
       newrule = growrule_omnivariate(&grow, posclass, param);
      }
     else
       newrule = growrule_omnivariate(instances, posclass, param);
     if (newrule->start)
      {
       if (param->modelselectionmethod == 0)       
         prunerule(newrule, prune, posclass, 2);
       else
         prunerule_with_modelselection(newrule, *instances, param->modelselectionmethod);
      }
     if (param->modelselectionmethod == 0)
      {
       update_counts(newrule, grow, 1);
       update_counts(newrule, prune, 0);
      }
     else
       update_counts(newrule, *instances, 1);
     if (!(newrule->start) || (error_rate_of_rule(*newrule) >= 0.5))
      {
       free_rule(*newrule);
       safe_free(newrule);
       if (param->modelselectionmethod == 0)
        {
         *instances = restore_instancelist(*instances, grow);
         merge_instancelist(instances, prune);
        }
       break;
      }
     add_rule(&result, newrule);
     if (param->modelselectionmethod == 0)
      {
       *instances = restore_instancelist(*instances, grow);
       merge_instancelist(instances, prune);     
      }
     remove_covered(instances, &removed, newrule);
    }
   for (j = 0; j < param->optimizecount; j++)
    {
     merge_instancelist(instances, removed);
     removed = NULL;
     if (param->modelselectionmethod == 0)
      {
       optimize_ruleset_multivariate(&result, posclass, instances, cvdata, param);
       simplify_ruleset_multivariate(&result, posclass, cvdata);
      }
     else
      {
       optimize_ruleset_modelselection(&result, posclass, instances, param);
       simplify_ruleset_modelselection(&result, posclass, *instances, param->modelselectionmethod);
      }
     remove_covered_by_ruleset(instances, &removed, result, 0);
    }
   merge_instancelist(&totalremoved, removed);
   removed = NULL;
  }
 safe_free(classes);
 merge_instancelist(instances, totalremoved);
 return result;
}
