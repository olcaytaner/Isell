#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include "classification.h"
#include "data.h"
#include "decisiontree.h"
#include "instance.h"
#include "ktree.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "matrix.h"
#include "messages.h"
#include "parameter.h"
#include "partition.h"
#include "rule.h"
#include "statistics.h"
#include "univariatetree.h"

extern Instance mean;
extern Instance variance;
extern int *class_performance;
extern int *class_counts;
extern int performance;
extern Datasetptr current_dataset;
extern FILE* output;
extern int mustrealize;

/**
 * Calculates total complexity of a decision rule. The complexity of a rule is the sum of complexities of each condition in that rule. The complexity of a condition is the number of free parameters + 1.
 * @param[in] rule Decision rule
 * @return Total complexity of the rule
 */
int complexity_of_rule(Decisionruleptr rule)
{
 /*!Last Changed 19.02.2005*/
	int result = 0;
	Decisionconditionptr condition;
	condition = rule->start;
	while (condition)
	 {
		 result += condition->freeparam + 1;
		 condition = condition->next;
	 }
	return result; 
}

/**
 * Calculates total complexity of the rules of a given class in a ruleset. The complexity of a rule is calculated with the function complexity_of_rule.
 * @param[in] set The ruleset
 * @param[in] positive Index of the class to which rules belong to
 * @return Total complexity of rules of a specific class
 */
int complexity_of_ruleset(Ruleset set, int positive)
{
 /*!Last Changed 19.02.2005*/
	int result = 0;
	Decisionruleptr rule;
	rule = set.start;
	while (rule)
	 {
		 if (rule->classno == positive)
				 result += complexity_of_rule(rule);
		 rule = rule->next;
	 }
	return result; 
}

/**
 * Calculates total complexity of a ruleset. The complexity of a ruleset is the sum of complexities of each rule in that ruleset. The complexity of a rule is calculated with the function complexity_of_rule.
 * @param[in] set The ruleset
 * @return Total complexity of a ruleset
 */
int ruleset_complexity(Ruleset set)
{
 /*!Last Changed 20.02.2005*/
	int result = 0;
	Decisionruleptr rule;
	rule = set.start;
	while (rule)
	 {
			result += complexity_of_rule(rule);
		 rule = rule->next;
	 }
	return result;
}

/**
 * S value
 * @param[in] n Number of items
 * @param[in] k Number of items in the set
 * @param[in] p Ratio of number of items in the set to the number of items.
 * @return S value
 */
double S(int n, int k, double p)
{
 /*!Last Changed 18.05.2003*/
 if (p > 0 && p < 1)
   return -log2(p) * k + - log2(1.0 - p) * (n - k);
 else
   return 0;
}

/**
 * Calculates the total description length of a rule in a ruleset. 
 * @param[in] r The rule
 * @param[in] possibleconditioncount Total number of possible split points in the dataset
 * @return Total description length of a rule
 */
double description_length_of_rule(Decisionrule r, int possibleconditioncount)
{
 /*!Last Changed 02.12.2003*/
 double result = 0;
 if (r.decisioncount != 0)
   result += log2(r.decisioncount);
 if (log2(r.decisioncount) > 1)
   result += log2(log2(r.decisioncount));
 result += S(possibleconditioncount, r.decisioncount, (r.decisioncount + 0.0) / possibleconditioncount);
 return result / 2;
}

/**
 * Calculates the total description length of a ruleset. The description length of a ruleset is the sum of the description lengths of the rules in that ruleset.
 * @param[in] r Ruleset
 * @param[in] positive If positive is -1, the function calculates the descriptin length of the ruleset. If it is a class index, the function calculates the description length of the rules belonging to the positive class.
 * @return Description length of a ruleset
 */
double description_length_of_ruleset(Ruleset r, int positive)
{
 /*!Last Changed 02.12.2003 added description_length_of_rule*/
 /*!Last Changed 18.05.2003*/
 double result = 0;
 Decisionruleptr tmp;
 tmp = r.start;
 while (tmp)
  {
   if (tmp->classno == positive || positive == -1)
     result += description_length_of_rule(*tmp, r.possibleconditioncount);
   tmp = tmp->next;
  }
 return result;
}

/**
 * Calculates description length of the exceptions.
 * @param[in] C Number of examples covered
 * @param[in] U Number of examples uncovered
 * @param[in] fp Number of false positives
 * @param[in] fn Number of false negatives
 * @param[in] proportion Proportion of ...
 * @return Description length of exceptional examples
 */
double description_length_of_exceptions(int C, int U, int fp, int fn, double proportion)
{
 /*!Last Changed 24.11.2003 added S*/
 /*!Last Changed 04.11.2003 added extra checks for zero division*/
 /*!Last Changed 12.06.2003 added proportion*/
 /*!Last Changed 19.05.2003*/
 double result = 0; 
 int e = fp + fn;
 result = result + log2(C + U + 1);
 if (C >= U)
  {
   if (C != 0)
     result += S(C, fp, (proportion * (e + 0.0)) / C);
   if (U != 0)
     result += S(U, fn, (fn + 0.0) / U);
  }
 else
  {
   proportion = 1 - proportion;
   if (C != 0)
     result += S(C, fp, (fp + 0.0) / C);
   if (U != 0)
     result += S(U, fn, (proportion * (e + 0.0)) / U);
  }
 return result / 2;
}

/**
 * Calculates error made by a condition in a rule. The number of misclassified instances of a condition is the sum of negative instances those are covered by this condition and positive instances those not covered by this condition.
 * @param[in] c The condition
 * @param[in] list Link list of instances
 * @param[in] positive Index of the positive class
 * @return The number of misclassified instances of a given condition
 */
int error_of_condition(Decisionconditionptr c, Instanceptr list, int positive)
{
	/*!Last Changed 21.01.2004*/
	int error = 0;
	Instanceptr tmp;
	tmp = list;
	while (tmp)
	 {
		 if (condition_cover_instance(tmp, c))
			 {
				 if (give_classno(tmp) != positive)
						 error++;
			 }
			else
				 if (give_classno(tmp) == positive)
						 error++;
		 tmp = tmp->next;
	 }
	return error;
}

/**
 * Counts the number of positive and negative examples covered by this condition
 * @param[in] c The decision condition
 * @param[out] p Number of positive examples covered by this condition
 * @param[out] n Number of negative examples covered by this condition
 * @param[in] posclass Index of the positive class
 * @param[in] list Link list of instances
 */
void count_examples_for_condition(Decisionconditionptr c, int* p, int* n, int posclass, Instanceptr list)
{
	/*!Last Changed 02.12.2003*/
	Instanceptr tmp;
	*p = 0;
	*n = 0;
	tmp = list;
	while (tmp)
	 {
		 if (condition_cover_instance(tmp, c))
    {
				 if (give_classno(tmp) == posclass)
						 (*p)++;
					else
						 (*n)++;
    }
		 tmp = tmp->next;
	 }
}

/**
 * Calculates total number of conditions in the rules of a ruleset
 * @param[in] r Ruleset
 * @return Number of conditions in a ruleset
 */
int condition_count(Ruleset r)
{
	/*!Last Changed 10.11.2003*/
	int result = 0;
	Decisionruleptr tmp;
	tmp = r.start;
	while (tmp)
	 {
		 result += tmp->decisioncount;
		 tmp = tmp->next;
	 }
	return result;
}

/**
 * Print a condition in a rule with a nice format to the output file. The output will be like:\n
 * F1 < 4.2 for continuous univariate conditions \n
 * F2 = red for discrete univariate conditions \n
 * 3.2F1 - 1.2F2 + 4.2F3 > 6.1 for multivariate linear conditions \n
 * 1.2F1F1 + 2.3F1F2 - 3.4F1F3 + 1.2F2F2 + 1.3F2F3 - 4.5F3F3 + 1.2F1 - 1.4F2 + 4.2F3 < 3.7 for multivariate quadratic conditions
 * @param[in] condition The condition
 */
void write_condition(Decisionconditionptr condition)
{
	/*!Last Changed 29.01.2004*/
	/*!Last Changed 30.10.2003*/
	char st[10], pst[READLINELENGTH];
	int sindex, i, j, dim = current_dataset->multifeaturecount - 1;
	double newsplit;
	double newweight, *weights;
	weights = safecalloc(dim, sizeof(double), "write_condition", 5);
	if (condition->next != NULL)
		 strcpy(st," AND ");
	else
		 strcpy(st,"");
	if (condition->featureindex >= 0)
   switch (current_dataset->features[condition->featureindex].type)
			 { 
    	case INTEGER:fprintf(output,"F%d %c %d%s",condition->featureindex + 1, condition->comparison, (int)condition->value, st);
		  		            break;
   		case    REAL:if (!mustrealize)
  						            fprintf(output,"F%d %c %5.4f%s", condition->featureindex + 1, condition->comparison, condition->value, st);
						            else
																		 {
																			 newsplit = (double) (condition->value * sqrt(variance.values[condition->featureindex].floatvalue) + mean.values[condition->featureindex].floatvalue);
                    fprintf(output,"F%d %c %5.4f%s", condition->featureindex + 1, condition->comparison, newsplit, st);
																		 }
	  			            break;
   		case  STRING:sindex = (int) condition->value;
						            fprintf(output,"F%d %c %s%s",condition->featureindex + 1,condition->comparison,current_dataset->features[condition->featureindex].strings[sindex], st);
		  		            break;
	   }
	else
	 {
		 if (condition->featureindex == LINEAR)
			 {
     newsplit = (double) condition->w0;
		   for (i = 0; i < dim - 1; i++)
					 {
						 newweight = condition->w.values[0][i] / sqrt(variance.values[i].floatvalue);
							newsplit += (double) (mean.values[i].floatvalue * newweight);
							set_precision(pst, NULL, "F%d ");
			  	 fprintf(output, pst, newweight, i + 1);
					 }
					newweight = condition->w.values[0][i] / sqrt(variance.values[i].floatvalue);
					newsplit += (double) (mean.values[i].floatvalue * newweight);
			  set_precision(pst, NULL, NULL);
			  fprintf(output, pst, newweight);
					set_precision(pst, "F%d %c ", "%s");
			  fprintf(output, pst, dim, condition->comparison, newsplit, st);
			 }
			else
				 if (condition->featureindex == QUADRATIC)
					 {
						 sindex = 0;
       newsplit = (double) condition->w0;
   		  for (i = 0; i < dim; i++)
							  for (j = i; j < dim; j++)
									 {
										 newweight = condition->w.values[0][sindex] / sqrt(variance.values[i].floatvalue * variance.values[j].floatvalue);
											weights[i] -= newweight * mean.values[j].floatvalue;
											weights[j] -= newweight * mean.values[i].floatvalue;
											newsplit -= (double) (newweight * mean.values[i].floatvalue * mean.values[j].floatvalue);
           set_precision(pst, NULL, "F%dF%d ");
										 fprintf(output, pst, newweight, i + 1, j + 1);  
										 sindex++;
									 }
							for (i = 0; i < dim - 1; i++)
							 {								 
								 newweight = condition->w.values[0][sindex] / sqrt(variance.values[i].floatvalue);
								 weights[i] += newweight;
									newsplit += (double) (newweight * mean.values[i].floatvalue);
         set_precision(pst, NULL, "F%d ");
								 fprintf(output, pst, weights[i], i + 1);  
								 sindex++;
							 }
							newweight = condition->w.values[0][sindex] / sqrt(variance.values[i].floatvalue);
							weights[i] += newweight;
							newsplit += (double) (newweight * mean.values[i].floatvalue);
			    set_precision(pst, NULL, NULL);
       fprintf(output, pst, weights[i]);
			    set_precision(pst, "F%d %c ", "%s");
       fprintf(output, pst, i + 1, condition->comparison, newsplit, st);
					 }
		}
	safe_free(weights);
}

/**
 * Print a rule in a ruleset in a nice format to the output file. Examples: \n
 * F1 < 4.2 AND F2 > 2.3 THEN CLASS = poisoned(12/0) \n
 * 3.2F1 - 1.2F2 + 4.2F3 > 6.1 AND F2 > 6.3 THEN CLASS = positive(63/12) \n
 * 1.2F1F1 + 2.3F1F2 - 3.4F1F3 + 1.2F2F2 + 1.3F2F3 - 4.5F3F3 + 1.2F1 - 1.4F2 + 4.2F3 < 3.7 THEN CLASS = democrat(15/7) 
 * @param[in] rule The rule
 */
void write_rule(Decisionruleptr rule)
{
	/*!Last Changed 30.10.2003*/
	Decisionconditionptr condition;
	condition = rule->start;
	while (condition)
	 {
		 write_condition(condition);
		 condition = condition->next;
	 }
	fprintf(output, " THEN CLASS = %s.(%d/%d)\n",current_dataset->features[current_dataset->classdefno].strings[rule->classno], rule->covered, rule->falsepositives);
}

/**
 * Print a whole ruleset in a nice format to an output file.
 * @param[in] r The ruleset
 */
void write_ruleset(Ruleset r)
{
	/*!Last Changed 30.10.2003*/
	Decisionruleptr rule;
	rule = r.start;
	while (rule)
	 {
		 write_rule(rule);
		 rule = rule->next;
	 }
}

int error_of_ruleset(Ruleset set,Instanceptr traindata)
{
	/*!Last Changed 14.01.2004*/
 /*!Last Changed 04.11.2003*/
 int instanceclassno, covered, error;
 Instanceptr test = traindata;
	Decisionruleptr tmp;
	error = 0;
 while (test)
  {
   instanceclassno = give_classno(test);
			covered = 0;
			tmp = set.start;
			while (tmp)
			 {
				 if (rule_cover_instance(test, *tmp))
					 {
       if (instanceclassno != tmp->classno)
								 error++;
							covered = 1;
							break;
					 }
				 tmp = tmp->next;
			 }
			if (!covered)
				 if (instanceclassno != set.defaultclass)
						 error++;
   test=test->next;
  }
	return error;
}

void exception_handling(Ruleset r, Instanceptr instances, int positive, int* C, int* U, int* fp, int* fn)
{
	/*!Last Changed 19.05.2003 More than two classes are considered again*/
 /*!Last Changed 18.05.2003*/
	Instanceptr tmp = instances;
	Decisionruleptr rule;
	int covered, instanceclassno;
 while (tmp)
	 {
	  instanceclassno = give_classno(tmp);
		 rule = r.start;
			covered = 0;
			while (rule)
			 {
				 if ((rule->classno == positive || positive == -1) && rule_cover_instance(tmp, *rule))
					 {
						 covered = 1;
							(*C)++;
				   if (instanceclassno != rule->classno)
								 (*fp)++;
						 break;
					 }
				 rule = rule->next;
			 }
			if (!covered)
			 {
				 (*U)++;
				 if (instanceclassno == positive || (positive == -1 && instanceclassno != r.defaultclass))
						 (*fn)++;
			 }
		 tmp = tmp -> next;
	 }
}

void count_exceptions_for_ruleset(Ruleset r, Instanceptr instances, int* C, int* U, int* fp, int* fn)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
	/*!Last Changed 19.12.2003*/
	Instanceptr tmp = instances;
	Decisionruleptr rule;
	int iscovered, covered2 = *C, uncovered2 = *U, fp2 = *fp, fn2 = *fn, *classcoverings, i, classno, instanceclassno;
	classcoverings = (int *) safemalloc(current_dataset->classno * sizeof(int), "count_exceptions_for_ruleset", 4);
 while (tmp)
	 {
		 rule = r.start;
			iscovered = 0;
			for (i = 0; i < current_dataset->classno; i++)
				 classcoverings[i] = 0;
			instanceclassno = give_classno(tmp);
			while (rule)
			 {
				 if (rule_cover_instance(tmp, *rule))
					 {
						 iscovered = 1;
							classcoverings[rule->classno]++;
					 }
				 rule = rule->next;
			 }
			if (!iscovered)
			 {
				 uncovered2++;
				 if (instanceclassno != r.defaultclass)
						 fn2++;
			 }
			else
			 {
				 covered2++;
     classno = find_max_in_list(classcoverings, current_dataset->classno);
					if (instanceclassno != classno)
						 fp2++;
			 }
		 tmp = tmp -> next;
	 }
	safe_free(classcoverings);
	*C = covered2;
	*U = uncovered2;
	*fp = fp2;
	*fn = fn2;
}

Decisioncondition copy_of_condition(Decisioncondition condition)
{
	/* Last Changed 04.10.2004*/
	Decisioncondition result;
 result.conditiontype = condition.conditiontype;
	result.comparison = condition.comparison;
	result.featureindex = condition.featureindex;
 result.upperlimit = condition.upperlimit;
 result.lowerlimit = condition.lowerlimit;
	result.w0 = condition.w0;
	if (in_list(condition.featureindex, 2, LINEAR, QUADRATIC))
		 result.w = matrix_copy(condition.w);
	result.value = condition.value;
	result.datasize = condition.datasize;
	result.freeparam = condition.freeparam;
	if (condition.featureindex == DISCRETEMIX)
	  result.ksplit = copy_of_korderingsplit(condition.ksplit);
	result.next = NULL;
	return result;
}

Decisionconditionptr create_copy_of_condition(Decisionconditionptr condition)
{
	/* Last Changed 02.02.2004 added safemalloc*/
	/* Last Changed 27.01.2004*/
	/* Last Changed 13.01.2004*/
	/* Last Changed 20.05.2003*/
	Decisionconditionptr result = safemalloc(sizeof(Decisioncondition), "create_copy_of_condition", 1);
	result->comparison = condition->comparison;
	result->featureindex = condition->featureindex;
	result->conditiontype = condition->conditiontype;
	result->upperlimit = condition->upperlimit;
	result->lowerlimit = condition->lowerlimit;
	result->w0 = condition->w0;
	if (in_list(condition->featureindex, 2, LINEAR, QUADRATIC))
		 result->w = matrix_copy(condition->w);
	result->value = condition->value;
	result->datasize = condition->datasize;
	result->freeparam = condition->freeparam;
	if (condition->featureindex == DISCRETEMIX)
  	result->ksplit = copy_of_korderingsplit(condition->ksplit);
	result->next = NULL;
	return result;
}

Decisionruleptr create_copy_of_rule(Decisionruleptr rule)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
	/*!Last Changed 20.05.2003*/
 int i;
	Decisionruleptr result = safemalloc(sizeof(Decisionrule), "create_copy_of_rule", 1);
	Decisionconditionptr newcondition, tmp, last = NULL;
	result->classno = rule->classno;
	result->decisioncount = rule->decisioncount;
 result->covered = rule->covered;
 result->falsepositives = rule->falsepositives;
	result->next = NULL;
	result->start = NULL;
 result->counts = safecalloc(current_dataset->classno, sizeof(int), "create_copy_of_rule", 7);
 for (i = 0; i < current_dataset->classno; i++)
   result->counts[i] = rule->counts[i];
	tmp = rule->start;
	while (tmp)
	 {
		 newcondition = create_copy_of_condition(tmp);
			if (tmp == rule->start)
				 result->start = newcondition;
			else
				 last->next = newcondition;
			last = newcondition;
			tmp = tmp->next;
	 }
	result->end = last;
	return result;
}

/**
 * Extracts a single rule from a decision tree leaf.
 * @param[out] r Ruleset to which the extracted rule will be added
 * @param[in] leaf Decision tree leaf node
 */
Decisionruleptr extract_rule_from_leaf(Decisionnodeptr leaf)
{
 /*!Last Changed 18.03.2004 added covered and falsepositives*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 27.10.2003*/
 int i;
 Decisionruleptr newrule = empty_rule(leaf->classno);
 Decisionconditionptr newcondition;
 for (i = 0; i < leaf->conditioncount; i++)
  {
   newcondition = (Decisionconditionptr) safemalloc(sizeof(Decisioncondition), "add_rule_from_leave", 6);
   *newcondition = copy_of_condition(leaf->rule[i]);
   newcondition->next = NULL;
   add_condition(newrule, newcondition);
  }
 newrule->covered = leaf->covered;
 newrule->falsepositives = leaf->falsepositives;
 return newrule;
}

Ruleset* create_copy_of_ruleset(Ruleset* r)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
	/*!Last Changed 20.05.2003*/
	Ruleset* result = safemalloc(sizeof(Ruleset), "create_copy_of_ruleset", 1);
	Decisionruleptr newrule, tmp, last = NULL;
	result->defaultclass = r->defaultclass;
	result->possibleconditioncount = r->possibleconditioncount;
	result->rulecount = r->rulecount;
	result->start = NULL;
 result->counts = safecalloc(current_dataset->classno, sizeof(int), "create_copy_of_ruleset", 7);
	tmp = r->start;
	while (tmp)
	 {
		 newrule = create_copy_of_rule(tmp);
			if (tmp == r->start)
				 result->start = newrule;
			else
			  last->next = newrule;
			last = newrule;
			tmp = tmp->next;
	 }
	result->end = last;
	return result;
}

void calculate_performance_for_prune(Instanceptr pruneset, Decisionrule rule, int* p, int* n, int positive)
{
 /*!Last Changed 10.05.2003*/
 Instanceptr tmp;
 tmp = pruneset;
 *p = 0;
 *n = 0;
 while (tmp)
  {
   if (rule_cover_instance(tmp, rule))
    {
     if (give_classno(tmp) == positive)
       (*p)++;
     else
       (*n)++;
    }
   tmp = tmp->next;
  }
}

BOOLEAN check_rule_consistency(Decisionruleptr rule)
{
 int count = 0;
 Decisionconditionptr condition = rule->start;
 while (condition)
  {
   count++;
   condition = condition->next;
  }
 if (count == rule->decisioncount)
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

BOOLEAN check_ruleset_consistency(Ruleset set)
{
 int count = 0;
 Decisionruleptr rule = set.start;
 while (rule)
  {
   if (!check_rule_consistency(rule))
     return BOOLEAN_FALSE;
   count++;
   rule = rule->next;
  }
 if (count == set.rulecount)
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

void initialize_counts(Decisionruleptr start)
{
	/*!Last Changed 04.12.2003*/
	Decisionruleptr tmp = start;
	while (tmp)
	 {
		 tmp->falsepositives = 0;
			tmp->covered = 0;
		 tmp = tmp->next;
	 }
}

void free_condition(Decisioncondition condition)
{
	/*!Last Changed 09.01.2004*/
	if (in_list(condition.featureindex, 2, LINEAR, QUADRATIC))
  	matrix_free(condition.w);
	if (condition.featureindex == DISCRETEMIX)
		 deallocate_korderingsplit(condition.ksplit);
}

void free_rule(Decisionrule rule)
{
	/*!Last Changed 10.05.2003*/
	Decisionconditionptr tmp, next;
	tmp = rule.start;
	while (tmp)
	 {
		 next = tmp->next;
			free_condition(*tmp);
			safe_free(tmp);
			tmp = next;
	 }
 safe_free(rule.counts);
}

void free_ruleset(Ruleset r)
{
	/*!Last Changed 21.05.2003*/
	Decisionruleptr tmp, next;
	tmp = r.start;
	while (tmp)
	 {
		 next = tmp->next;
			free_rule(*tmp);
			safe_free(tmp);
			tmp = next;
	 }
 safe_free(r.counts);
}

void empty_ruleset(Ruleset* r)
{
	/*!Last Changed 01.11.2003*/
	r->rulecount = 0;
	r->start = NULL;
	r->end = NULL;
 r->counts = safecalloc(current_dataset->classno, sizeof(int), "empty_ruleset", 4);
}

Decisionruleptr empty_rule(int classno)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
	/*!Last Changed 13.05.2003*/
	Decisionruleptr newrule = safemalloc(sizeof(Decisionrule), "empty_rule", 1);
	newrule->decisioncount = 0;
	newrule->start = NULL;
	newrule->end = NULL;
	newrule->classno = classno;
	newrule->next = NULL;
 newrule->counts = safecalloc(current_dataset->classno, sizeof(int), "empty_rule", 8);
	return newrule;
}

Decisionconditionptr empty_condition()
{
	/*!Last Changed 02.02.2004 added safemalloc*/
	/*!Last Changed 21.01.2004*/
	Decisionconditionptr result = safemalloc(sizeof(Decisioncondition), "empty_condition", 1);
	result->featureindex = -1;
	result->next = NULL;
	return result;
}

void merge_ruleset(Ruleset* set1, Ruleset set2)
{
 if (set2.rulecount != 0)
  {
   if (set1->rulecount == 0)
     set1->start = set2.start;
   if (set1->rulecount != 0)
     set1->end->next = set2.start;
   set1->end = set2.end;
   set1->rulecount += set2.rulecount;
  }
}

int condition_cover_instance2(Instanceptr instance, Decisionconditionptr condition)
{
	/*!Last Changed 19.09.2004*/
	int fno;
	double combin;
	fno = condition->featureindex;
	if (fno >= 0)
	 {
	  if (condition->conditiontype == HYBRID_CONDITION)
			 {
     if (dequal(real_feature(instance, fno), condition->value))
						 return NO;
     if (condition->comparison == '<' && real_feature(instance, fno) > condition->value)
				   return NO;
				 if (condition->comparison == '>' && real_feature(instance, fno) < condition->value)
				  	return NO;
			 }
			else
			 {
    	switch (current_dataset->features[fno].type)
					 {
	    	 case INTEGER:if (condition->comparison == '<' && instance->values[fno].intvalue >= condition->value)
		    	 														return NO;
			   		 	          if (condition->comparison == '>' && instance->values[fno].intvalue <= condition->value)
				    														return NO;
				    	           break;
		     case    REAL:if (condition->comparison == '<' && instance->values[fno].floatvalue >= condition->value - 0.0001)
				    														return NO;
				  	             if (condition->comparison == '>' && instance->values[fno].floatvalue <= condition->value + 0.0001)
				  			  	 									return NO;
					 						   					break;
   		 	case  STRING:if (instance->values[fno].stringindex != condition->value)
		    																return NO;
				    												break;
					 }
			 }
	 }
	else
		 if (in_list(fno, 2, LINEAR, QUADRATIC))
			 {
				 combin = multiply_with_matrix(instance, condition->w);
					if (combin < condition->w0 && condition->comparison == '>')
						 return NO;
					if (combin > condition->w0 && condition->comparison == '<')
						 return NO;
			 }
	return YES;
}

int condition_cover_instance(Instanceptr instance, Decisionconditionptr condition)
{
	/*!Last Changed 22.01.2004*/
	/*!Last Changed 09.01.2004*/
	/*!Last Changed 10.05.2003*/
	int fno;
	double combin;
	fno = condition->featureindex;
	if (fno >= 0)
	 {
	  if (condition->conditiontype == HYBRID_CONDITION)
			 {
     if (condition->comparison == '<' && real_feature(instance, fno) > condition->value)
				   return NO;
				 if (condition->comparison == '>' && real_feature(instance, fno) < condition->value)
				  	return NO;
			 }
			else
			 {
    	switch (current_dataset->features[fno].type)
					 {
	    	 case INTEGER:if (condition->comparison == '<' && instance->values[fno].intvalue > condition->value)
		    	 														return NO;
			   		 	          if (condition->comparison == '>' && instance->values[fno].intvalue < condition->value)
				    														return NO;
                    if (condition->comparison == '*' && (instance->values[fno].intvalue > condition->upperlimit || instance->values[fno].intvalue < condition->lowerlimit))
                      return NO;
				    	           break;
		     case    REAL:if (condition->comparison == '<' && instance->values[fno].floatvalue > condition->value)
				    														return NO;
				  	             if (condition->comparison == '>' && instance->values[fno].floatvalue < condition->value)
				  			  	 									return NO;
                    if (condition->comparison == '*' && (instance->values[fno].floatvalue > condition->upperlimit || instance->values[fno].floatvalue < condition->lowerlimit))
                      return NO;
					 						   					break;
   		 	case  STRING:if (instance->values[fno].stringindex != condition->value)
		    																return NO;
				    												break;
					 }
			 }
	 }
	else
		 if (in_list(fno, 2, LINEAR, QUADRATIC))
			 {
				 combin = multiply_with_matrix(instance, condition->w);
					if (combin < condition->w0 + 0.0001 && combin > condition->w0 - 0.0001)
						 return YES;
					if (combin < condition->w0 && condition->comparison == '>')
						 return NO;
					if (combin > condition->w0 && condition->comparison == '<')
						 return NO;
			 }
			else
				 if (fno == DISCRETEMIX)
					 {
							if (is_smaller_discrete_order(instance, condition->ksplit))
								 if (condition->comparison == '<')
								   return YES;
									else
										 return NO;
							else
								 if (condition->comparison == '>')
										 return YES;
									else
										 return NO;
					 }
	return YES;
}

int ruleset_cover_instance(Instanceptr instance, Ruleset r)
{
	/*!Last Changed 03.11.2003*/
	Decisionruleptr tmp = r.start;
	while (tmp)
	 {
		 if (rule_cover_instance(instance, *tmp))
				 return YES;
		 tmp = tmp->next;
	 }
	return NO;
}

int rule_cover_instance(Instanceptr instance, Decisionrule rule)
{
	/*!Last Changed 10.05.2003*/
	Decisionconditionptr tmp = rule.start;
	while (tmp)
	 {
		 if (!condition_cover_instance(instance, tmp))
				 return NO;
		 tmp = tmp->next;
	 }
	return YES;
}

int rule_cover_instance2(Instanceptr instance, Decisionrule rule)
{
	/*!Last Changed 19.09.2004*/
	Decisionconditionptr tmp = rule.start;
	while (tmp)
	 {
		 if (!condition_cover_instance2(instance, tmp))
				 return NO;
		 tmp = tmp->next;
	 }
	return YES;
}

int rule_cover_negative(Instanceptr instances, Decisionrule rule)
{
	/*!Last Changed 09.05.2003*/
	Instanceptr tmp;
	tmp = instances;
	while (tmp)
	 {
		 if (give_classno(tmp) != rule.classno)
		   if (rule_cover_instance(tmp, rule))
				   return YES;
		 tmp = tmp->next;
	 }
	return NO;
}

double information(int positive, int total)
{
	/*!Last Changed 10.05.2003*/
	return -log2((positive+1.0)/(total+1.0));
}

int false_positive_count(Ruleset r, Instanceptr instances)
{
	/*!Last Changed 03.11.2003*/
	int total = 0;
	Decisionruleptr tmprule;
	Instanceptr tmp;
	tmp = instances;
	while (tmp)
	 {
		 tmprule = r.start;
			while (tmprule)
			 {
				 if (rule_cover_instance(tmp, *tmprule))
					 {
		     if (give_classno(tmp) != tmprule->classno)
					  	 total++;
							break;
					 }
					tmprule = tmprule->next;
			 }
			tmp = tmp->next;
	 }
	return total;
}

int return_error_and_free_condition(Decisionconditionptr condition, Instanceptr test, int positive)
{
	/*!Last Changed 03.12.2004*/
	int error;
	if (condition)
		 if (condition->featureindex != -1)
			 {
  			error = error_of_condition(condition, test, positive);
  			free_condition(*condition);
	  		safe_free(condition);
					return error;
			 }
			else
			 {
				 free_condition(*condition);
					safe_free(condition);
			  return 0;
			 }
	else
	  return 0;
}

void add_condition_after(Decisionruleptr rule, Decisionconditionptr before, Decisionconditionptr inserted)
{
	/*!Last Changed 10.05.2003*/
	rule->decisioncount++;
	if (before == NULL)
	 {
		 inserted->next = rule->start;
		 rule->start = inserted;
	 }
	else
	 {
		 inserted->next = before->next;
		 before->next = inserted;
	 }
	if (inserted->next == NULL)
			rule->end = inserted;
}

void remove_condition(Decisionruleptr rule, Decisionconditionptr deleted)
{
	/*!Last Changed 19.12.2003*/
 Decisionconditionptr before = NULL, tmp;
	tmp = rule->start;
	while (tmp)
	 { 
		 if (tmp == deleted)
				 break;
			before = tmp;
			tmp = tmp->next;
	 }
	if (tmp)
	  remove_condition_after(rule, before, deleted);
}

void remove_condition_after(Decisionruleptr rule, Decisionconditionptr before, Decisionconditionptr deleted)
{
	/*!Last Changed 10.05.2003*/
	rule->decisioncount--;
	if (deleted->next == NULL)
		 rule->end = before;
	if (before == NULL)
		 rule->start = deleted->next;
	else
		 before->next = deleted->next;
}

void add_condition(Decisionruleptr rule, Decisionconditionptr condition)
{
	/*!Last Changed 09.05.2003*/
	rule->decisioncount++;
	if (rule->decisioncount == 1)
		 rule->start = condition;
	else
	  rule->end->next = condition;
	rule->end = condition;
}

double test_rule(Decisionrule rule, Instanceptr instances)
{
	/*!Last Changed 10.05.2003*/
	int negative = 0, total = 0;
	Instanceptr tmp;
	tmp = instances;
	while (tmp)
	 {
			if (rule_cover_instance(tmp, rule))
			 {
			  total++;
		   if (give_classno(tmp) != rule.classno)
						 negative++;
			 }
			tmp = tmp->next;
	 }
	return negative/(total+0.0);
}

void add_rule_after(Ruleset* r, Decisionruleptr before, Decisionruleptr inserted)
{
	/*!Last Changed 18.05.2003*/
	r->rulecount++;
	if (before == NULL)
	 {
		 inserted->next = r->start;
		 r->start = inserted;
	 }
	else
	 {
		 inserted->next = before->next;
		 before->next = inserted;
	 }
	if (inserted->next == NULL)
			r->end = inserted;
}

void remove_rule_after(Ruleset* r, Decisionruleptr before, Decisionruleptr deleted)
{
	/*!Last Changed 10.05.2003*/
	r->rulecount--;
	if (r->rulecount == 0)
	 {
		 r->start = NULL;
			r->end = NULL;
	 }
	else
	 {
	  if (deleted->next == NULL)
	  	 r->end = before;
	  if (before == NULL)
	  	 r->start = deleted->next;
	  else
		   before->next = deleted->next;
	 }
}

void remove_rules_of_class(Ruleset* set, int index)
{
 Decisionruleptr tmp, before = NULL, tmpnext;
 tmp = set->start;
 while (tmp)
  {
   tmpnext = tmp->next;
   if (tmp->classno == index)
    {
     if (before)
       before->next = tmp->next;
     else
       set->start = tmp->next;
     if (tmpnext == NULL)
       set->end = before;
     set->rulecount--;
     free_rule(*tmp);
     safe_free(tmp);
    }
   else
     before = tmp;
   tmp = tmpnext;
  }
}

void remove_rule(Ruleset* set, Decisionruleptr rule)
{
	/*!Last Changed 16.06.2003*/
 Decisionruleptr before = NULL, tmp;
	tmp = set->start;
	while (tmp)
	 { 
		 if (tmp == rule)
				 break;
			before = tmp;
			tmp = tmp->next;
	 }
	if (tmp)
	  remove_rule_after(set, before, rule);
}

void add_rule(Ruleset* set, Decisionruleptr rule)
{
	/*!Last Changed 04.11.2003 Added rule->next = NULL*/
	/*!Last Changed 13.05.2003*/
	if (set->rulecount == 0)
		 set->start = rule;
	else
		 set->end->next = rule;
	set->end = rule;
	rule->next = NULL;
	set->rulecount++;
}

void update_counts(Decisionruleptr rule, Instanceptr list1, int initialize)
{
	/*!Last Changed 02.11.2003*/
 int classno;
	Instanceptr tmp;
	if (initialize)
	 {
		 rule->covered = 0;
			rule->falsepositives = 0;
	 }
	tmp = list1;
	while (tmp)
	 {
		 if (rule_cover_instance(tmp, *rule))
    {
     classno = give_classno(tmp);
				 if (classno != rule->classno)
						 rule->falsepositives++;
					else
						 rule->covered++;
    }
		 tmp = tmp->next;
	 }
}

void update_counts_ruleset(Ruleset r, Instanceptr list)
{
	/*!Last Changed 18.03.2004*/
	int i;
 BOOLEAN covered;
	Decisionruleptr tmp;
	Instanceptr test = list;
 for (i = 0; i < current_dataset->classno; i++)
   r.counts[i] = 0;
	tmp = r.start;
	while (tmp)
	 {
		 tmp->covered = 0;
			tmp->falsepositives = 0;
   for (i = 0; i < current_dataset->classno; i++)
     tmp->counts[i] = 0;
		 tmp = tmp->next;
	 }
 while (test)
  {
   i = give_classno(test);
   covered = BOOLEAN_FALSE;
			tmp = r.start;
			while (tmp)
			 {
				 if (rule_cover_instance(test, *tmp))
					 {
       tmp->counts[i]++;
						 tmp->covered++;
       if (i != tmp->classno)
								 tmp->falsepositives++;
       covered = BOOLEAN_TRUE;
							break;
					 }
				 tmp = tmp->next;
			 }
   if (!covered)
     r.counts[i]++;
   test = test->next;
  }
}

void print_counts(char* st,Instanceptr list)
{
	int* counts, i;
	counts = find_class_counts(list);
	printf("----------\n");
	printf("%s\n",st);
	printf("----------\n");
 for (i = 0; i < current_dataset->classno; i++)
		 printf(m1309, i, counts[i]);
	printf("----------\n");
	safe_free(counts);
}

double error_rate_of_rule(Decisionrule r)
{
	/*!Last Changed 02.11.2003*/
	if (r.covered + r.falsepositives != 0)
  	return (r.falsepositives + 0.0) / (r.covered + r.falsepositives);
	else
		 return 1;
}

double laplace_estimate(int falsepositives, int n)
{
	return (falsepositives + 1.0) / (n + 2.0);
}

double laplace_estimate_of_rule(Decisionrule r)
{
	/*!Last Changed 05.05.2004*/
	return laplace_estimate(r.falsepositives, r.covered + r.falsepositives);
}

double laplace_estimate_of_rule_kclass(Decisionrule r)
{
	return (r.falsepositives + 1.0) / (r.covered + r.falsepositives + current_dataset->classno);
}

Decisionruleptr get_next_class_rule(Decisionruleptr previousclassrule)
{
	/*!Last Changed 05.05.2004*/
	int previousclass;
	Decisionruleptr tmp = previousclassrule;
	previousclass = tmp->classno;
	tmp = tmp->next;
	while (tmp && tmp->classno == previousclass)
			tmp = tmp->next;
	return tmp;
}
