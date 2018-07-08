#include <stdarg.h>
#include <limits.h>
#include "c45rules.h"
#include "data.h"
#include "instance.h"
#include "instancelist.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "rule.h"

extern Datasetptr current_dataset;

/**
 * Sort the rules in a ruleset by class index
 * @param[in] r The ruleset
 */
void sort_rules(Ruleset* r)
{
 /*!Last Changed 05.11.2003 If i is r->end then after exchange j will be  r->end*/
 /*!Last Changed 30.10.2003*/
 Decisionruleptr i;
 Decisionruleptr j;
 Decisionruleptr ibefore;
 Decisionruleptr tmp;
 Decisionruleptr jbefore;
 if (r->start == NULL)
   return;
 for (ibefore = r->start, i = r->start->next; i != NULL; ibefore = i, i = i->next)
   for (jbefore = NULL, j = r->start; j != i; jbefore = j, j = j->next)
     if (j->classno > i->classno)
      {
       ibefore->next = j;
       if (jbefore)
         jbefore->next = i;
       else
         r->start = i;
       if (i->next == NULL)
         r->end = j;
       tmp = j->next;
       j->next = i->next;
       i->next = tmp;
       break;
      }
}

/**
 * Calculates total description length of a ruleset and the description length of the exceptions in the link lists of instances. The instance lists are given as variable arguments.
 * @param[in] r The ruleset
 * @param[in] positive Index of the positive class
 * @param[in] count Number of instance lists (Number of variable arguments)
 * @return Description length of the ruleset + Description length of the exceptions in the first link list + Description length of the exceptions in the second link list + ...
 */
double description_length_of_all(Ruleset r, int positive, int count, ...)
{
 /*!Last Changed 19.12.2003*/
 /*!Last Changed 29.11.2003*/
 /*!Last Changed 18.05.2003*/
 int i = 0, C = 0, U = 0, fp = 0, fn = 0, pcount = 0;
 double total = 0, result = description_length_of_ruleset(r, positive);
 Instanceptr tmp;
 va_list ap;
 va_start(ap, count);
 while (i < count)
  {
   tmp = va_arg(ap, Instanceptr);
   if (tmp)
    {
     pcount += find_class_count(tmp, positive);
     total += data_size(tmp);
     if (positive != -1)
       exception_handling(r, tmp, positive, &C, &U, &fp, &fn);
     else
       count_exceptions_for_ruleset(r, tmp, &C, &U, &fp, &fn);
    }
   i++;
  }
 if (positive != -1)
   result += description_length_of_exceptions(C, U, fp, fn, pcount/total);
 else
   result += description_length_of_exceptions(C, U, fp, fn, 0.5);
 return result;
}

/**
 * Produces a subset of rules from the rules in a ruleset.
 * @param[out] result Subset of rules (stored as a link list of rules)
 * @param[in] rules Original ruleset (stored as an array of rules)
 * @param[in] subset Indices of the selected rules to produce the subset. subset[i] is the index of the rule in the original subset to put in the i'th position to the resulting subset of rules.
 * @param[in] subsetcount Number of rules in the subset (Size of the subset array).
 */
void create_subset_of_ruleset(Ruleset* result, Decisionruleptr* rules, int* subset, int subsetcount)
{
 /*!Last Changed 30.10.2003*/
 int i;
 result->start = NULL;
 result->end = NULL;
 result->rulecount = subsetcount;
 if (subsetcount == 0)
   return;
 result->start = rules[subset[0]];
 result->end = rules[subset[subsetcount-1]];
 result->end->next = NULL;
 for (i = 0; i < subsetcount - 1; i++)
   rules[subset[i]]->next = rules[subset[i + 1]];
}

/**
 * Deallocates the rules, which are not in the subset of rules, from the rules array.
 * @param rules Rules array
 * @param rulecount Number of rules in the rules array (Size of the rules array)
 * @param subset Indices of the rules in the subset.
 * @param subsetcount Number of rules in the subset (Size of the subset array)
 */
void remove_rules_according_indexes_of_rule(Decisionruleptr* rules, int rulecount, int* subset, int subsetcount)
{
 /*!Last Changed 02.12.2004*/
 int i, j;
 BOOLEAN found;
 for (i = 0; i < rulecount; i++)
  {
   found = BOOLEAN_FALSE;
   for (j = 0; j < subsetcount; j++)
     if (subset[j] == i)
      {
       found = BOOLEAN_TRUE;
       break;
      }
   if (!found)
    {
     free_rule(*(rules[i]));
     safe_free(rules[i]);
    }
  }
}

/**
 * Deallocates the rules, which are not in the subset of rules, from the rules array.
 * @param rules Rules array
 * @param rulecount Number of rules in the rules array (Size of the rules array)
 * @param subsetrules Subset of rules stored as an array
 * @param subsetcount Number of rules in the subsetrules (Size of the subsetrules array)
 */
void remove_rules_according_rules_themselves(Decisionruleptr* rules, int rulecount, Decisionruleptr* subsetrules, int subsetcount)
{
 /*!Last Changed 02.12.2004*/
 int i, j;
 BOOLEAN found;
 for (i = 0; i < rulecount; i++)
  {
   found = BOOLEAN_FALSE;
   for (j = 0; j < subsetcount; j++)
     if (rules[i] == subsetrules[j])
      {
       found = BOOLEAN_TRUE;
       break;
      }
   if (!found)
    {
     free_rule(*(rules[i]));
     safe_free(rules[i]);
    }
  }
}

/**
 * Searches the best subset of a ruleset which has the minimum description length considering both the description length of the rules and the exceptions in the training data.
 * @param[in] start Starting rule in the ruleset (Header of the link list of rules)
 * @param[in] rulecount Number of rules in the ruleset
 * @param[in] traindata Training data
 * @param[in] conditioncount Number of possible conditions in the training data.
 * @return Best subset of rules having minimum description length (Stored as a link list of rules)
 */
Ruleset check_all_subsets(Decisionruleptr start, int rulecount, Instanceptr traindata, int conditioncount)
{
 /*!Last Changed 02.12.2004 added remove_rules_according_indexes_of_rule*/
 /*!Last Changed 30.10.2003*/
 Ruleset result;
 Decisionruleptr* rules;
 int **subsets, *counts, res, i, minindex = -1;
 double minlength = +1000000, length;
 empty_ruleset(&result);
 result.possibleconditioncount = conditioncount;
 rules = create_array_from_link_list_of_rules(start, rulecount);
 subsets = generate_subsets(rulecount, &res, &counts);
 for (i = 0; i < res; i++)
  {
   create_subset_of_ruleset(&result, rules, subsets[i], counts[i]);
   length = description_length_of_all(result, rules[0]->classno, 1, traindata);
   if (dless(length,minlength))
    {
     minlength = length;
     minindex = i;
    }
  }
 create_subset_of_ruleset(&result, rules, subsets[minindex], counts[minindex]);
 remove_rules_according_indexes_of_rule(rules, rulecount, subsets[minindex], counts[minindex]);
 for (i = 0; i < res; i++) 
   if (counts[i] > 0)
     safe_free(subsets[i]);
 safe_free(subsets);
 safe_free(counts);
 safe_free(rules);
 return result;
}

/**
 * Finds the best ruleset by applying one the following operators repeatedly until there is no improvement in the description length \n
 * i) Removing a rule from the inlist that decreases the description length most \n
 * ii) Moving a rule from outlist to the inlist that decreases the description length most \n
 * The operator that has the minimum description length is applied.
 * @param[out] inlist Current ruleset possibly to remove from or add rules to
 * @param[in] outlist Ruleset possibly to add rules from
 * @param[in] instances Training data
 * @param[in] classno Index of the positive class
 * @return Best description length
 */
double search_best_ruleset(Ruleset* inlist, Ruleset* outlist, Instanceptr instances, int classno)
{
 /*!Last Changed 31.10.2003*/
 double length, bestlength;
 int found = 1;
 Decisionruleptr inbefore, bestdeleted, bestinserted, outbefore, before, tmp, inlistlast, tmpnext;
 bestlength = description_length_of_all(*inlist, classno, 1, instances);
 while (found)
  {
   found = 0;
   before = NULL;
   tmp = inlist->start;
   while (tmp)
    {
     tmpnext = tmp->next;
     remove_rule_after(inlist, before, tmp);
     length = description_length_of_all(*inlist, classno, 1, instances);
     if (dless(length, bestlength))
      {
       bestlength = length;
       found = 1;
       inbefore = before;
       bestdeleted = tmp;
      }
     add_rule_after(inlist, before, tmp);
     before = tmp;
     tmp = tmpnext;
    }
   before = NULL;
   inlistlast = inlist->end;
   tmp = outlist->start;
   while (tmp)
    {
     tmpnext = tmp->next;
     remove_rule_after(outlist, before, tmp);
     add_rule(inlist, tmp);
     length = description_length_of_all(*inlist, classno, 1, instances);
     if (dless(length, bestlength))
      {
       bestlength = length;
       found = 2;
       outbefore = before;
       bestinserted = tmp;
      }
     remove_rule_after(inlist, inlistlast, tmp);
     add_rule_after(outlist, before, tmp);
     before = tmp;
     tmp = tmpnext;
    }
   if (found)
    {
     if (found == 1)
       remove_rule_after(inlist, inbefore, bestdeleted); 
     else
      {
       remove_rule_after(outlist, outbefore, bestinserted);
       add_rule(inlist, bestinserted);
      }
    }
  }
 return bestlength;
}

/**
 * Divides the ruleset into two parts. The first part contains percentage \% of rules of the ruleset (Stored in the inlist), second part contains (1 - percentage \%) of rules of the ruleset
 * @param[in] rules Rules stored as an array
 * @param[in] rulecount Number of rules in the rules array (Size of the rules array)
 * @param[in] percentage Percentage (Number between 0 and 100)
 * @param[out] inlist Percentage \% of rules
 * @param[out] outlist 1 - Percentage \% of rules
 */
void create_percentage_of_ruleset(Decisionruleptr* rules, int rulecount, int percentage, Ruleset* inlist, Ruleset* outlist)
{
 /*!Last Changed 01.11.2003*/
 int* rastgele, i;
 empty_ruleset(inlist);
 empty_ruleset(outlist);
 rastgele = random_array(rulecount);
 i = 0;
 while (i < rulecount)
  {
   if (i < (rulecount * percentage) / 10)
     add_rule(inlist, rules[rastgele[i]]);
   else
     add_rule(outlist, rules[rastgele[i]]);
   i++;
  }
 safe_free(rastgele);
}

/**
 * Creates an array of rules from a ruleset that stores the rules as a link list
 * @param[in] start Header of the rules link list
 * @param[in] rulecount Number of rules in the rules link list
 * @return An array of rules
 */
Decisionruleptr* create_array_from_link_list_of_rules(Decisionruleptr start, int rulecount)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 31.10.2003*/
 int i;
 Decisionruleptr tmp;
 Decisionruleptr* result;
 result = (Decisionruleptr*) safemalloc(rulecount * sizeof(Decisionruleptr), "create_array_from_link_list_of_rules", 4);
 for (i = 0, tmp = start; i < rulecount; i++)
  {
   result[i] = tmp;
   if (i != (rulecount - 1))
     tmp = tmp->next;
  }
 return result;
}

/**
 * Divides the original ruleset into two different parts with different percentages as 0, 10, 20, ..., 100. Checks if removing rules from the first part or adding rules from the second part to the first part will decrease the description length. It is mainly an heuristic to find best ruleset.
 * @param[in] start Header of the link list of rules
 * @param[in] rulecount Number of rules in the link list of rules
 * @param[in] traindata Training data
 * @param[in] conditioncount Number of possible conditions in the training data
 * @return Best ruleset
 */
Ruleset check_different_percentages(Decisionruleptr start, int rulecount, Instanceptr traindata, int conditioncount)
{
 /*!Last Changed 02.12.2004 added remove_rules_according_rules_themselves*/
 /*!Last Changed 05.11.2003 If best ruleset is empty return empty ruleset*/
 /*!Last Changed 31.10.2003*/
 int i = 0, bestcount, classno = start->classno;
 double bestlength = +1000000, length;
 Ruleset best, inlist, outlist;
 Decisionruleptr* rules;
 Decisionruleptr* bestrules = NULL;
 rules = create_array_from_link_list_of_rules(start, rulecount);
 empty_ruleset(&best);
 for (i = 0; i <= 10; i++)
  {
   create_percentage_of_ruleset(rules, rulecount, i, &inlist, &outlist);
   inlist.possibleconditioncount = conditioncount;
   length = search_best_ruleset(&inlist, &outlist, traindata, classno);
   if (dless(length,bestlength))
    {
     bestlength = length;
     if (bestrules)
       safe_free(bestrules);
     bestrules = create_array_from_link_list_of_rules(inlist.start, inlist.rulecount);
     bestcount = inlist.rulecount;
    }
   safe_free(inlist.counts);
   safe_free(outlist.counts);
  }
 remove_rules_according_rules_themselves(rules, rulecount, bestrules, bestcount);
 best.possibleconditioncount = conditioncount;
 if (bestcount != 0)
  {
   best.rulecount = bestcount;
   best.start = bestrules[0];
   best.end = bestrules[bestcount - 1];
   best.end->next = NULL;
  }
 for (i = 0; i < bestcount - 1; i++)
   bestrules[i]->next = bestrules[i+1];
 if (bestrules)
   safe_free(bestrules);
 safe_free(rules);
 return best;
}

/**
 * Merge the rulesets of all classes into one ruleset
 * @param[out] r Output ruleset (merged)
 * @param[in] sets[] Array of rulesets. sets[i] is the ruleset for class i.
 * @param[in] classcount Number of classes in the problem (Size of the sets array).  
 */
void merge_rulesets(Ruleset* r, Ruleset sets[MAXCLASSES], int classcount)
{
 /*!Last Changed 15.11.2010*/
 /*!Last Changed 01.11.2003*/
 int i;
 safe_free(r->counts);
 empty_ruleset(r);
 for (i = 0; i < classcount; i++)
   merge_ruleset(r, sets[i]);
}

/**
 * The algorithm tries to optimize rulesets for each class. For each class \n
 * (i) If the number of rules is less than or equal to 10, it checks all subsets of those rules to find optimal subset of rules. \n
 * (ii) If the number of rules is more than 10, it checks different percentages from 0 to 100 to find the optimum subset of rules.
 * @param[inout] r Output ruleset that will contain the rulesets of all classes. Original ruleset contains the rules extracted from the C4.5 decision tree.
 * @param[in] traindata Training data
 */
void find_class_rulesets(Ruleset* r, Instanceptr traindata)
{
/*!Last Changed 30.10.2003*/
 Ruleset sets[MAXCLASSES];
 Decisionruleptr rule = r->start;
 Decisionruleptr rulenext;
 Decisionruleptr rulesetstart = rule;
 int i, currentclass = rule->classno, rulecount = 1, classcount = 0;
 rule = rule->next;
 while (rule != NULL)
  {
   if (rule)
     rulenext = rule->next;
   if (rule->classno != currentclass)
    {
     if (rulecount <= 10)
       sets[classcount] = check_all_subsets(rulesetstart, rulecount, traindata, r->possibleconditioncount);
     else
       sets[classcount] = check_different_percentages(rulesetstart, rulecount, traindata, r->possibleconditioncount);
     classcount++;
     rulecount = 1;
     rulesetstart = rule;
     currentclass = rule->classno;
    }
   else
     rulecount++;
   rule = rulenext;
  }
 if (rulecount <= 10)
   sets[classcount] = check_all_subsets(rulesetstart, rulecount, traindata, r->possibleconditioncount);
 else
   sets[classcount] = check_different_percentages(rulesetstart, rulecount, traindata, r->possibleconditioncount);
 classcount++;
 merge_rulesets(r, sets, classcount);
 for (i = 0; i < classcount; i++)
   safe_free(sets[i].counts);
}

/**
 * Checks if the pessimistic error is smaller than the original error
 * @param Y1 Number of positive examples covered before applying some operator
 * @param Y2 Number of positive examples covered after applying some operator
 * @param E1 Number of negative examples covered before applying some operator
 * @param E2 Number of negative examples covered after applying some operator
 * @return YES if the pessimistic error is smaller, NO otherwise.
 */
int is_pessimistic_error_rate_smaller(int Y1, int Y2, int E1, int E2)
{
/*!Last Changed 27.10.2003*/
 double ucfr, ucfrnegative;
 ucfr = (E1 + 0.0) / (Y1 + E1);
 ucfrnegative = (E1 + E2 + 0.0) / (Y1 + E1 + Y2 + E2);
 if (ucfr >= ucfrnegative)
   return YES;
 return NO;
}

/**
 * Generalizes a rule by removing conditions which do not increase the pessimistic error
 * @param[out] rule The rule
 * @param[in] traindata Training data
 */
void generalize_rule(Decisionruleptr rule, Instanceptr traindata)
{
 /*!Last Changed 27.10.2003*/
 int Y1, Y2, E1, E2;
 Decisionconditionptr condition, conditionbefore;
 conditionbefore = NULL;
 condition = rule->start;
 calculate_performance_for_prune(traindata, *rule, &Y1, &E1, rule->classno);
 while (condition)
  {
   remove_condition_after(rule, conditionbefore, condition);
   calculate_performance_for_prune(traindata, *rule, &Y2, &E2, rule->classno);
   if (is_pessimistic_error_rate_smaller(Y1, Y2, E1, E2))
    {
     free_condition(*condition);
     safe_free(condition);
     if (conditionbefore)
       condition = conditionbefore->next;
     else
       condition = rule->start;
     Y1 = Y2;
     E1 = E2;
    }
   else
    {
     add_condition_after(rule, conditionbefore, condition);
     conditionbefore = condition;
     condition = condition->next;
    }
  }
}

/**
 * Generalizes the rules in a ruleset. It calls generalize_rule for each rule
 * @param[in] r Ruleset
 * @param[in] traindata Training data
 */
void generalize_c45rules(Ruleset* r, Instanceptr traindata)
{
 /*!Last Changed 27.10.2003*/
 Decisionruleptr rule;
 rule = r->start;
 while (rule)
  {
   generalize_rule(rule, traindata);
   rule = rule->next;
  }
}

/**
 * Chooses the default class for a given ruleset. The test instances those are not covered by any rule from the ruleset will be assigned to the default class. The class having the maximum number of instances those are not covered by any rules will be selected as the default class.
 * @param[in] r The ruleset
 * @param[in] traindata Training data
 */
void choose_default_class(Ruleset* r, Instanceptr traindata)
{
 /*!Last Changed 02.02.2004 added safecalloc*/
 /*!Last Changed 03.11.2003*/
 int* classcounts, classno, i, max;
 Instanceptr tmp = traindata;
 classcounts = (int*) safecalloc(current_dataset->classno, sizeof(int), "choose_default_class", 3);
 while (tmp)
  {
   if (!ruleset_cover_instance(tmp, *r))
    {
     classno = give_classno(tmp);
     classcounts[classno]++;
    }
   tmp = tmp->next;
  }
 max = -1;
 for (i = 0; i < current_dataset->classno; i++)
   if (classcounts[i] > max)
    {
     r->defaultclass = i;
     max = classcounts[i];
    }
 safe_free(classcounts);
}

/**
 * Finds the best class order for the whole ruleset. The algorithm is \n
 * 1. Start with the empty ruleset r \n
 * 2. For each class ruleset cr \n
 * 3.    Add cr to the ruleset r \n
 * 4.    If the ruleset r has less false positive count than the current minimum false positive count \n
 * 5.      Update current minimum false positive count and set the best class ruleset to cr \n
 * 6.    Remove cr from the ruleset r \n
 * 7. Add best class ruleset to r and continue with step 2 until there are no class rulesets left.
 * @param[inout] r The original ruleset
 * @param[in] traindata Training data
 */
void rank_classes(Ruleset* r, Instanceptr traindata)
{
 /*!Last Changed 02.02.2004 added safecalloc*/
 /*!Last Changed 05.11.2003*/
 Decisionruleptr* classstarts = (Decisionruleptr*) safecalloc(current_dataset->classno, sizeof(Decisionruleptr), "rank_classes", 1);
 Decisionruleptr* classends = (Decisionruleptr*) safecalloc(current_dataset->classno, sizeof(Decisionruleptr), "rank_classes", 2);
 int* rulecounts = safecalloc(current_dataset->classno, sizeof(int), "rank_classes", 3);
 int* classdone = safecalloc(current_dataset->classno, sizeof(int), "rank_classes", 4);
 int classcount = 0, classno = -1, i, j, minfpc, minindex, fpc;
 Decisionruleptr tmp;
 Decisionruleptr tmpbefore;
 tmpbefore = NULL;
 tmp = r->start;
 while (tmp)
  {
   classdone[tmp->classno] = 1;
   if (tmp->classno != classno)
    {
     if (classno != -1)
       classends[classno] = tmpbefore;
     classstarts[tmp->classno] = tmp;
     classcount++;
     classno = tmp->classno;
    }
   rulecounts[classno]++;
   tmpbefore = tmp;
   tmp = tmp->next;
  }
 classends[classno] = tmpbefore;
 for (i = 0; i < current_dataset->classno; i++)
   if (classends[i])
     classends[i]->next = NULL;
 safe_free(r->counts);
 empty_ruleset(r);
 for (i = 0; i < classcount; i++)
  {
   minfpc = +INT_MAX;
   minindex = -1;
   tmp = r->end;
   for (j = 0; j < current_dataset->classno; j++)
     if (classdone[j])
      {
       if (r->rulecount == 0)
         r->start = classstarts[j];
       if (r->end)
         r->end->next = classstarts[j];
       r->end = classends[j];
       r->rulecount += rulecounts[j];
       fpc = false_positive_count(*r, traindata);
       if (fpc < minfpc)
        {
         minfpc = fpc;
         minindex = j;
        }
       r->rulecount -= rulecounts[j];
       if (r->rulecount == 0)
         r->start = NULL;
       r->end = tmp;
       if (tmp)
         tmp->next = NULL;
      }
   if (minindex != -1)
    {
     if (r->rulecount == 0)
       r->start = classstarts[minindex];
     if (r->end)
       r->end->next = classstarts[minindex];
     r->end = classends[minindex];
     r->rulecount += rulecounts[minindex];     
     classdone[minindex] = 0;
    }
   else
    {
     printf("Minimum description length larger than INT_MAX\n");
     break;
    }
  }
 safe_free(classstarts);
 safe_free(classends);
 safe_free(rulecounts);
 safe_free(classdone);
}

/**
 * Optimizes the ruleset by removing rules which do not increase the error rate
 * @param[inout] r The original ruleset
 * @param[in] traindata Training data
 */
void polish_ruleset(Ruleset* r, Instanceptr traindata)
{
 /*!Last Changed 04.11.2003*/
 Decisionruleptr tmp, tmpbefore, tmpnext, deleted, deletedbefore;
 int error_decreased = YES, minerror = error_of_ruleset(*r, traindata), error;
 while (error_decreased)
  {
   error_decreased = NO;
   tmpbefore = NULL;
   tmp = r->start;
   while (tmp)
    {
     tmpnext = tmp->next;
     remove_rule_after(r, tmpbefore, tmp);
     error = error_of_ruleset(*r, traindata);
     if (error < minerror)
      {
       minerror = error;
       deleted = tmp;
       deletedbefore = tmpbefore;
       error_decreased = YES;
      }
     add_rule_after(r, tmpbefore, tmp);
     tmpbefore = tmp;
     tmp = tmpnext;
    }
   if (error_decreased)
    {
     remove_rule_after(r, deletedbefore, deleted);
     free_rule(*deleted);
     safe_free(deleted);
    }
  }
}

/**
 * Recursively extracts rules from a decision tree node and adds to a ruleset.
 * @param[out] r Ruleset to which all rules will be added
 * @param[in] tree Decision tree node
 */
void add_rules_from_tree(Ruleset* r, Decisionnodeptr tree)
{
 /*!Last Changed 04.10.2004 added multivariate rules*/
 /*!Last Changed 24.10.2003*/
 int i, fno;
 Decisionruleptr newrule;
 if (tree->featureno == -1)
  {
   newrule = extract_rule_from_leaf(tree);
   add_rule(r, newrule);
  }
 else
  {
   fno = tree->featureno;
   if (fno < 0)
    {
     add_rules_from_tree(r, tree->left);
     add_rules_from_tree(r, tree->right);
    }
   else
     switch (current_dataset->features[fno].type){
       case INTEGER:
       case REAL:   add_rules_from_tree(r, tree->left);
                    add_rules_from_tree(r, tree->right);
                    break;
       case STRING: for (i=0;i<current_dataset->features[fno].valuecount;i++)
                      add_rules_from_tree(r, &(tree->string[i]));
                    break; 
     }
  }
}

/**
 * Creates a ruleset from the decision tree generated by C4.5 algorithm
 * @param[in] tree Root node of the decision tree
 * @return Extracted ruleset
 */
Ruleset create_ruleset_from_decision_tree(Decisionnodeptr tree)
{
 /*!Last Changed 01.11.2003*/
 Ruleset result;
 empty_ruleset(&result);
 add_rules_from_tree(&result, tree);
 return result;
}
