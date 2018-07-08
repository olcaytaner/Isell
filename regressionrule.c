#include "clustering.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "messages.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "model.h"
#include "regressionrule.h"
#include "regressiontree.h"
#include "statistics.h"
#include "tests.h"

extern Datasetptr current_dataset;

/**
 * Generate a string of model types in the conditions of the rules in a ruleset. For univariate model 'U', for multivariate linear model 'L' and for multivariate quadratic model 'Q' character is used. The rules are separated via semicolons. 
 * @param[in] r Ruleset
 * @param[out] st Output string. For example the string U;LUL;QL says that there are three rules in the ruleset. The first rule has one univariate condition. Second rule has the first and third conditions multivariate linear and the second univariate. Third rule has two conditions, where the first condition is quadratic and the second is linear.
 */
void write_nodetypes_regression_rule(Regressionruleset r, char* st)
{
	/*!Last Changed 06.03.2005*/
	Regressionruleptr tmprule;
	Regressionconditionptr tmpcondition;
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
								 sprintf(st, "%sU", st);
				 tmpcondition = tmpcondition->next;
			 }
			sprintf(st, "%s;", st);
		 tmprule = tmprule->next;
	 }
}

/**
 * Calculates number of conditions in a ruleset
 * @param[in] r Ruleset
 * @return Number of conditions in a regression rule set.
 */
int regression_condition_count(Regressionruleset r)
{
	/*!Last Changed 06.03.2005*/
	int result = 0;
	Regressionruleptr tmp;
	tmp = r.start;
	while (tmp)
	 {
		 result += tmp->conditioncount;
		 tmp = tmp->next;
	 }
	return result;
}

/**
 * Method used to simplify a ruleset by removing unnecessary rules. Unnecessary rules are the rules that when they are removed the generalization error of the ruleset does not decrease. The generalization error of a ruleset is determined according to the given model selection criteria.
 * @param[in] r Ruleset
 * @param[in] data Data on which generalization error will be calculated
 * @param[in] modelselectionmethod Type of model selection technique. MODEL_SELECTION_AIC for Akaike Information Criterion based model selection, MODEL_SELECTION_BIC for Bayesian Information Criterion based model selection and MODEL_SELECTION_CV for cross-validation based model selection.
 */
void simplify_regression_ruleset(Regressionruleset* r, Instanceptr data, int modelselectionmethod)
{
	/*!Last Changed 06.03.2005*/
	Regressionruleptr firstrule, deleted, tmprule, deletedbefore;
	double minerror, error;
	minerror = generalization_error_of_regression_ruleset(*r, data, modelselectionmethod);
	firstrule = r->start;
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
			remove_regression_rule_after(r, deletedbefore, deleted);
			error = generalization_error_of_regression_ruleset(*r, data, modelselectionmethod);
			if (error > minerror)
				 add_regression_rule_after(r, deletedbefore, deleted);
			else
			 {
				 free_regression_rule(*deleted);
					safe_free(deleted);
				 minerror = error;
			 }
			deleted = deletedbefore;
	 }
}

/**
 * Adds one rule after a specific rule. Since the rules in a ruleset are stored as a link list, this operation is like adding a node after a given node in a link list. Specifically the links are updated.
 * @param[in] r Ruleset
 * @param[in] before The rule after which the new rule will be added.
 * @param[in] inserted The rule that will be added.
 */
void add_regression_rule_after(Regressionruleset* r, Regressionruleptr before, Regressionruleptr inserted)
{
	/*!Last Changed 04.03.2005*/
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

/**
 * Removes one rule after a specific rule. Since the rules in a ruleset are stored as a link list, this operation is like removing a node after a given node in a link list. Specifically the links are updated.
 * @param[in] r Ruleset
 * @param[in] before The rule whose next rule will be removed.
 * @param[in] deleted The rule that will be deleted.
 */
void remove_regression_rule_after(Regressionruleset* r, Regressionruleptr before, Regressionruleptr deleted)
{
	/*!Last Changed 04.03.2005*/
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

/**
 * Calculates the generalization error of a single rule.
 * @param[in] rule The rule whose generalization error will be calculated
 * @param[in] data Data which will be used in generalization error calculations.
 * @param[in] modelselectionmethod Type of model selection technique. MODEL_SELECTION_AIC for Akaike Information Criterion based model selection, MODEL_SELECTION_BIC for Bayesian Information Criterion based model selection and MODEL_SELECTION_CV for cross-validation based model selection.
 * @return Generalization error of a rule
 */
double generalization_error_of_regression_rule(Regressionruleptr rule, Instanceptr data, MODEL_SELECTION_TYPE modelselectionmethod)
{
	/*!Last Changed 06.03.2005*/
	double error;
	int complexity, size;
	error = mse_of_rule(rule, data, &size);
	complexity = complexity_of_regression_rule(rule);
	switch (modelselectionmethod)
	 {
	  case MODEL_SELECTION_CV :return error;
				                        break;
			case MODEL_SELECTION_AIC:return aic_squared_error(error, complexity, size);
				                        break;
			case MODEL_SELECTION_BIC:return bic_squared_error(error, complexity, size);
				                        break;
   default                 :printf(m1230, modelselectionmethod);
                            break;
	 }
	return error;
}

/**
 * Calculates the generalization error of a ruleset.
 * @param[in] r The ruleset whose generalization error will be calculated
 * @param[in] data Data which will be used in generalization error calculations.
 * @param[in] modelselectionmethod Type of model selection technique. MODEL_SELECTION_AIC for Akaike Information Criterion based model selection, MODEL_SELECTION_BIC for Bayesian Information Criterion based model selection and MODEL_SELECTION_CV for cross-validation based model selection.
 * @return Generalization error of a ruleset
 */
double generalization_error_of_regression_ruleset(Regressionruleset r, Instanceptr data, MODEL_SELECTION_TYPE modelselectionmethod)
{
	/*!Last Changed 04.03.2005*/
	double error;
	int complexity, size;
	error = mse_of_ruleset(r, data, &size);
	complexity = complexity_of_regression_ruleset(r);
	switch (modelselectionmethod)
	 {
	  case MODEL_SELECTION_CV :return error;
				                        break;
			case MODEL_SELECTION_AIC:return aic_squared_error(error, complexity, size);
				                        break;
			case MODEL_SELECTION_BIC:return bic_squared_error(error, complexity, size);
				                        break;
   default                 :printf(m1230, modelselectionmethod);
                            break;
	 }
	return error;
}

/**
 * After growing rules in a ruleset and simplifying those rules we are not done. There is also optimization step where one tries for each rule: \n
 * 1. Learn the revision rule: One starts from the original rule but tries to change it by adding and removing conditions. \n
 * 2. Learn the replacement rule: One regrows the rule from the beginning and then prune it. \n
 * If the ruleset with the revision rule and/or with the replacement rule has less generalization error than the original ruleset, one change the ruleset to one with the minimum generalization error, otherwise no change will be done.
 * @param[in] r Ruleset
 * @param[in] traindata Training data where one calculates the generalization error using AIC or BIC.
 * @param[in] cvdata Validation data where one estimates the generalization error using CV.
 * @param[in] param Parameters of the REGRESSION RULE LEARNER. The model selection technique parameter is read from this structure.
 */
void optimize_regression_ruleset(Regressionruleset* r, Instanceptr* traindata, Instanceptr* cvdata, Regrule_parameterptr param)
{
	/*!Last Changed 03.04.2005*/
	/*!Last Changed 05.03.2005*/
	Regressionruleptr rule, before = NULL, replacementrule, revisionrule, next, newrule;
	Instanceptr removedtrain = NULL, removedcv = NULL, newsample, grow, prune;
	matrix w;
	int exitnow;
	double error, revisionerror, replacementerror, regvalue;
	rule = r->start;
	while (rule && can_be_divided_rule(*traindata, param->sigma, &w, &regvalue, r->leaftype))
	 {
			if (*cvdata)
				{
		   error = generalization_error_of_regression_ruleset(*r, *cvdata, param->modelselectionmethod);
     divide_instancelist(traindata, &prune, &grow, 3);
     replacementrule = grow_regression_rule(&grow, param);
  			if (replacementrule->conditioncount > 0)
						 prune_regression_rule(replacementrule, prune, param->modelselectionmethod);
				}
			else
			 {
		   error = generalization_error_of_regression_ruleset(*r, *traindata, param->modelselectionmethod);
				 newsample = bootstrap_sample_without_stratification(*traindata);
					replacementrule = grow_regression_rule(&newsample, param);
  			if (replacementrule->conditioncount > 0)
						 prune_regression_rule(replacementrule, *traindata, param->modelselectionmethod);
			 }
			remove_regression_rule_after(r, before, rule);
			add_regression_rule_after(r, before, replacementrule);
			if (*cvdata)
			 {
  		 replacementerror = generalization_error_of_regression_ruleset(*r, *cvdata, param->modelselectionmethod);
				 revisionrule = grow_regression_revision_rule(&grow, rule, param);
					if (revisionrule->conditioncount > 0)
						 prune_regression_rule(revisionrule, prune, param->modelselectionmethod);
			 }
			else
			 {
				 replacementerror = generalization_error_of_regression_ruleset(*r, *traindata, param->modelselectionmethod);
  			deallocate(newsample);
				 revisionrule = grow_regression_revision_rule(traindata, rule, param);
					if (revisionrule->conditioncount > 0)
       prune_regression_rule(revisionrule, *traindata, param->modelselectionmethod);
			 }
			remove_regression_rule_after(r, before, replacementrule);
			add_regression_rule_after(r, before, revisionrule);
			if (*cvdata)
			 {
  		 revisionerror = generalization_error_of_regression_ruleset(*r, *cvdata, param->modelselectionmethod);
  			*traindata = restore_instancelist(*traindata, grow);
  			merge_instancelist(traindata, prune);
			 }
			else
				 revisionerror = generalization_error_of_regression_ruleset(*r, *traindata, param->modelselectionmethod);
   if ((revisionerror < error) || (replacementerror < error))
				 if (replacementerror <= revisionerror)
					 {
						 remove_regression_rule_after(r, before, revisionrule);
							add_regression_rule_after(r, before, replacementrule);
							free_regression_rule(*revisionrule);
							safe_free(revisionrule);
							free_regression_rule(*rule);
							safe_free(rule);
  				 remove_regression_covered(traindata, &removedtrain, replacementrule);
    			if (*cvdata)
    				 remove_regression_covered(cvdata, &removedcv, replacementrule);
							rule = replacementrule;
					 }
					else
					 {
							free_regression_rule(*replacementrule);
							safe_free(replacementrule);
							free_regression_rule(*rule);
							safe_free(rule);						 
  				 remove_regression_covered(traindata, &removedtrain, revisionrule);
    			if (*cvdata)
    				 remove_regression_covered(cvdata, &removedcv, revisionrule);
							rule = revisionrule;
					 }
			else
			 {
					remove_regression_rule_after(r, before, revisionrule);
					add_regression_rule_after(r, before, rule);
					free_regression_rule(*revisionrule);
					safe_free(revisionrule);
					free_regression_rule(*replacementrule);
					safe_free(replacementrule);
  			remove_regression_covered(traindata, &removedtrain, rule);
  			if (*cvdata)
    		 remove_regression_covered(cvdata, &removedcv, rule);
			 }
			before = rule;
			rule = rule->next;
	 }
	if (!rule)
	 {
  	while (can_be_divided_rule(*traindata, param->sigma, &w, &regvalue, r->leaftype))
			 {
	  	 exitnow = 0;
  			if (*cvdata)
					 {
       divide_instancelist(traindata, &prune, &grow, 3);
       newrule = grow_regression_rule(&grow, param);
					 }
			  else
				  	newrule = grow_regression_rule(traindata, param);
  	  if (newrule && newrule->start)
					 {
    			if (*cvdata)
				  		 prune_regression_rule(newrule, prune, param->modelselectionmethod);
				  	else
					  	 prune_regression_rule(newrule, *traindata, param->modelselectionmethod);
					  if (newrule->start)
							 {
				  		 add_regression_rule(r, newrule);
      			if (*cvdata)
									 {
  			  	   *traindata = restore_instancelist(*traindata, grow);
  				  	  merge_instancelist(traindata, prune);
									 }
							 }
				  	else
					  	 exitnow = 1;
					 }
			  else
				   exitnow = 1;
			  if (!exitnow)
					 {
			  	 remove_regression_covered(traindata, &removedtrain, newrule);
  	  	 if (r->leaftype == LINEARLEAF)
      		 matrix_free(w);				 
					 }
			  else
					 {
    			if (*cvdata)
								{
  			  	 *traindata = restore_instancelist(*traindata, grow);
  				  	merge_instancelist(traindata, prune);
								}
				   break;
					 }
			 }
	  if (r->leaftype == CONSTANTLEAF)
	  	 r->regressionvalue = regvalue;
	  else
    	r->w = w;
	 }
	else
	 {
  	while (rule)
			 {
	  	 next = rule->next;
					if (before)
		  		 before->next = next;
					else
							r->start = next;
     free_regression_rule(*rule);
		  	safe_free(rule);
     r->rulecount--;
     if (next == NULL)
			  		r->end = before;
			  rule = next;
			 }
	  if (before)
	    before->next = NULL;
	 }
	merge_instancelist(traindata, removedtrain);
	if (*cvdata)
		 merge_instancelist(cvdata, removedcv);
}

/**
 * Moves instances covered by a given rule from the first list to the end of the second list.
 * @param[out] list1 First list
 * @param[out] list2 Second list
 * @param[in] rule The rule
 */
void remove_regression_covered(Instanceptr* list1, Instanceptr* list2, Regressionruleptr rule)
{
	/*!Last Changed 04.03.2005*/
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
   if (regression_rule_cover_instance(tmp, rule))
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

/**
 * Adds a given rule to the end of a ruleset
 * @param[in] set Ruleset
 * @param[in] rule The rule that will be added
 */
void add_regression_rule(Regressionruleset* set, Regressionruleptr rule)
{
	/*!Last Changed 04.03.2005*/
	if (set->rulecount == 0)
		 set->start = rule;
	else
		 set->end->next = rule;
	set->end = rule;
	rule->next = NULL;
	set->rulecount++;
}

/**
 * Removes the condition after a given condition. Since the conditions in a rule are stored as a link list, this operation is like removing a node after a given node in a link list. Specifically the links are updated.
 * @param[in] rule The rule
 * @param[in] before The condition whose next condition will be removed.
 * @param[in] deleted The condition that will be deleted.
 */
void remove_regression_condition_after(Regressionruleptr rule, Regressionconditionptr before, Regressionconditionptr deleted)
{
	/*!Last Changed 04.03.2005*/
	rule->conditioncount--;
	if (deleted->next == NULL)
		 rule->end = before;
	if (before == NULL)
		 rule->start = deleted->next;
	else
		 before->next = deleted->next;
}

/**
 * Adds one condition after a specific condition. Since the conditions in a rule are stored as a link list, this operation is like adding a node after a given node in a link list. Specifically the links are updated.
 * @param[in] rule The rule
 * @param[in] before The condition after which the new condition will be added.
 * @param[in] inserted The condition that will be added.
 */
void add_regression_condition_after(Regressionruleptr rule, Regressionconditionptr before, Regressionconditionptr inserted)
{
	/*!Last Changed 04.03.2005*/
	rule->conditioncount++;
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

/**
 * Moves instances covered by a given condition from the first list to the end of the second list.
 * @param[in] condition The condition
 * @param[out] obey First list
 * @param[out] notobey Second list
 */
void send_regression_exceptions(Regressionconditionptr condition, Instanceptr* obey, Instanceptr* notobey)
{
	/*!Last Changed 02.03.2005*/
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
   if (!regression_condition_cover_instance(tmp, condition))
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

/**
 * Calculates the mean square error of a regression rule. As a side effect it also finds the number of instances that are covered by this rule.
 * @param[in] c The rule
 * @param[in] list Instance list
 * @param[out] covered Number of instances that are covered by this rule
 * @return Mean square error of the given regression rule
 */
double mse_of_rule(Regressionruleptr c, Instanceptr list, int* covered)
{
	/*!Last Changed 02.03.2005*/
	Instanceptr tmp;
	double result = 0;
	tmp = list;
	*covered = 0;
	while (tmp)
	 {
		 if (regression_rule_cover_instance(tmp, c))
			 {
				 (*covered)++;
					result += instance_regression_error(c->w, c->regressionvalue, tmp, c->leaftype);
			 }
		 tmp = tmp->next;
	 }
	return result / (*covered);
}

/**
 * Calculates the mean square error of a regression ruleset. As a side effect it also find the number of instances in the list.
 * @param[in] set The regression ruleset
 * @param[in] list List of instances
 * @param[out] count Number of instances in the list.
 * @return Mean square error of the given regression ruleset.
 */
double mse_of_ruleset(Regressionruleset set, Instanceptr list, int* count)
{
	/*!Last Changed 02.03.2005*/
	int iscovered;
	Instanceptr tmp;
 Regressionruleptr rule;
	double result = 0;
	*count = 0;
	tmp = list;
	while (tmp)
	 {
		 rule = set.start;
			iscovered = 0;
			while (rule)
			 {
	  	 if (regression_rule_cover_instance(tmp, rule))
					 {
		  			result += instance_regression_error(rule->w, rule->regressionvalue, tmp, rule->leaftype);
							iscovered = 1;
							break;
					 }
					rule = rule->next;
			 }
			if (!iscovered)
				 result += instance_regression_error(set.w, set.regressionvalue, tmp, set.leaftype);
		 tmp = tmp->next;
			(*count)++;
	 }
	return result / (*count);
}

/**
 * Calculates the total complexity of a regression rule. The complexity of a regression rule is the sum of complexities of each condition in that rule and the complexity of the regression fit for that rule. The complexity of a condition is: \n
 * 2 if the condition is a univariate condition \n
 * (d + 1) if the condition is a multivariate linear condition where d is the number of features in the dataset \n
 * ((d + 1)(d + 2)) / 2 if the condition is a multivariate quadratic condition where d is the number of features in the dataset \n
 * The complexity of the regression fit is: \n
 * 1 if the regression fit is constant \n
 * d if the regression fit is linear
 * @param[in] rule The regression rule for which the complexity will be calculated
 * @return Total complexity of a single regression rule
 */
int complexity_of_regression_rule(Regressionruleptr rule)
{
	/*!Last Changed 27.03.2005*/
	/*!Last Changed 02.03.2005*/
	int result = 0;
 Regressionconditionptr tmp;
	tmp = rule->start;
	while (tmp)
	 {
		 if (tmp->featureindex >= 0)
  		 result += 2;
			else
				 if (tmp->featureindex == LINEAR)
						 result += current_dataset->multifeaturecount;
					else
						 if (tmp->featureindex == QUADRATIC)
								 result += multifeaturecount_2d(current_dataset) + 1;
		 tmp = tmp->next;
	 }
	if (rule->leaftype == CONSTANTLEAF)
		 result += 1;
	else
		 result += current_dataset->multifeaturecount;
	return result;
}

/**
 * Calculates the total complexity of a regression ruleset. The complexity of a regression ruleset is the sum of complexities of each rule in that ruleset and the complexity of the regression fit for the exceptions of that ruleset. The complexity of a regression rule is calculated via the function complexity_of_regression_rule. The complexity of the regression fit is: \n
 * 1 if the regression fit is constant \n
 * d if the regression fit is linear
 * @param[in] set The regression ruleset for which the complexity will be calculated
 * @return Total complexity of a single ruleset
 */
int complexity_of_regression_ruleset(Regressionruleset set)
{
	/*!Last Changed 02.03.2005*/
	int result = 0;
	Regressionruleptr rule;
	rule = set.start;
	while (rule)
	 {
		 result += complexity_of_regression_rule(rule);
		 rule = rule->next;
	 }
	if (set.leaftype == CONSTANTLEAF)
		 result += 1;
	else
		 result += current_dataset->multifeaturecount;
	return result;
}

/**
 * Checks if the given condition covers a specific instance
 * @param[in] instance The instance which will be checked for being covered
 * @param[in] condition The condition
 * @return YES if the condition covers the instance, NO otherwise.
 */
int regression_condition_cover_instance(Instanceptr instance, Regressionconditionptr condition)
{
	/*!Last Changed 27.03.2005*/
	/*!Last Changed 02.03.2005*/
	int fno;
 CHILD_TYPE nearest;
	double value;
	fno = condition->featureindex;
	if (fno >= 0)
	 {
		 value = real_feature(instance, fno);
			if (value < condition->value && condition->comparison == '>')
					return NO;
			if (value > condition->value && condition->comparison == '<')
					return NO;
	 }
	else
	 {
		 nearest = nearest_center_in_regression_tree(instance, condition->featureindex, condition->leftcenter, condition->rightcenter);
		 if (nearest == LEFTCHILD && condition->comparison == '>')
				 return NO;
		 if (nearest == RIGHTCHILD && condition->comparison == '<')
				 return NO;
  }
	return YES;
}

/**
 * Checks if the given rule covers a specific instance. A rule covers an instance if all of its conditions cover that instance.
 * @param[in] instance The instance which will be checked for being covered
 * @param[in] rule The rule
 * @return YES if the ruke covers the instance, NO otherwise.
 */
int regression_rule_cover_instance(Instanceptr instance, Regressionruleptr rule)
{
	/*!Last Changed 02.03.2005*/
	Regressionconditionptr tmp = rule->start;
	while (tmp)
	 {
		 if (!regression_condition_cover_instance(instance, tmp))
				 return NO;
		 tmp = tmp->next;
	 }
	return YES;
}

/**
 * Adds a given condition to the end of a rule
 * @param[in] rule Rule
 * @param[in] condition The condition that will be added
 */
void add_regression_condition(Regressionruleptr rule, Regressionconditionptr condition)
{
	/*!Last Changed 02.03.2005*/
	rule->conditioncount++;
	if (rule->conditioncount == 1)
		 rule->start = condition;
	else
	  rule->end->next = condition;
	rule->end = condition;
}

/**
 * Destructor function for a regression rule. Frees memory allocated to the rule. Basically it frees memory for its conditions and if the regression fit is linear, it also frees memory allocated to the weights.
 * @param[in] rule The regression rule 
 */
void free_regression_rule(Regressionrule rule)
{
	/*!Last Changed 27.03.2005*/
	/*!Last Changed 02.03.2005*/
	Regressionconditionptr tmp, next;
	tmp = rule.start;
	while (tmp)
	 {
		 next = tmp->next;
			if (in_list(tmp->featureindex, 2, LINEAR, QUADRATIC))
			 {
				 deallocate(tmp->leftcenter);
				 deallocate(tmp->rightcenter);
			 }
			safe_free(tmp);
			tmp = next;
	 }
	if (rule.leaftype == LINEARLEAF)
		 matrix_free(rule.w);
}

/**
 * Destructor function for a regression ruleset. Frees memory allocated to the ruleset. Basically it frees memory allocated to its rules using free_regression_rule function and if the regression fit is linear, it also frees memory allocated to the weights.
 * @param[in] r The regression ruleset
 */
void free_regression_ruleset(Regressionruleset r)
{
 /*!Last Changed 31.08.2008 added freeing weights*/
	/*!Last Changed 02.03.2005*/
	Regressionruleptr tmp, next;
	tmp = r.start;
	while (tmp)
	 {
		 next = tmp->next;
			free_regression_rule(*tmp);
			safe_free(tmp);
			tmp = next;
	 }
 if (r.leaftype == LINEARLEAF)
   matrix_free(r.w);
}

/**
 * Constructor for a regression ruleset.
 * @return An empty allocated regression ruleset
 */
Regressionruleset empty_regression_ruleset()
{
	/*!Last Changed 02.03.2005*/
	Regressionruleset r;
	r.rulecount = 0;
	r.start = NULL;
	r.end = NULL;
	return r;
}

/**
 * Constructor for a regression rule.
 * @return An empty allocated regression rule
 */
Regressionruleptr empty_regression_rule()
{
	/*!Last Changed 02.03.2005*/
	Regressionruleptr newrule = safemalloc(sizeof(Regressionrule), "empty_regression_rule", 1);
	newrule->conditioncount = 0;
	newrule->start = NULL;
	newrule->end = NULL;
	newrule->next = NULL;
	return newrule;
}

/**
 * Constructor for a regression condition.
 * @return An empty allocated regression condition
 */
Regressionconditionptr empty_regression_condition()
{
	/*!Last Changed 27.03.2005*/
	/*!Last Changed 02.03.2005*/
	Regressionconditionptr result = safemalloc(sizeof(Regressioncondition), "empty_regression_condition", 1);
	result->leftcenter = NULL;
	result->rightcenter = NULL;
	result->featureindex = LEAF_NODE;
	result->next = NULL;
	return result;
}

int can_be_divided_rule(Instanceptr data, double sigma, matrix* w, double* regvalue, LEAF_TYPE leaftype)
{
	/*!Last Changed 03.04.2005 added size < 3*/
	/*!Last Changed 02.03.2005*/
	int size;
	matrix X, r;
	double error;
	size = data_size(data);
	if (size < 3)
		 return 0;
	if (leaftype == CONSTANTLEAF)
	 {
  	*regvalue = find_mean_of_regression_value(data);
			error = find_variance_of_regression_value(data, *regvalue);
	 }
	else
	 {
   create_regression_matrices(data, &X, &r);
		 *w = multivariate_regression(X, r);
			matrix_free(X);
			matrix_free(r);
			error = variance_of_linear_regression(*w, data, leaftype);
	 }
	if (error / size < sigma)
		 return 0;
	return 1;
}

char separate_which_group(Instanceptr* data, Instanceptr left, Instanceptr right, int quadratic, LEAF_TYPE leaftype)
{
	/*!Last Changed 03.04.2005*/
	/*!Last Changed 26.03.2005*/
	int leftsize, rightsize;
	double lefterror, righterror;
	Instanceptr leftinstances, rightinstances;
 divide_instancelist_according_centers(data, &leftinstances, &rightinstances, left, right, quadratic);
	leftsize = data_size(leftinstances);
	rightsize = data_size(rightinstances);
	if (leftsize == 0 || rightsize == 0)
	 {
   *data = restore_instancelist(*data, leftinstances);
  	merge_instancelist(data, rightinstances);	
		 return '=';
	 }
	lefterror = constant_or_linear_model_mse(leftinstances, leftinstances, leaftype);
	righterror = constant_or_linear_model_mse(rightinstances, rightinstances, leaftype);
 *data = restore_instancelist(*data, leftinstances);
	merge_instancelist(data, rightinstances);	
	if ((lefterror / leftsize) < (righterror / rightsize))
		 return '<';
	else
		 return '>';
}

void find_linear_regression_condition(Instanceptr* data, Instanceptr* left, Instanceptr* right, char* comparison, LEAF_TYPE leaftype)
{
	/*!Last Changed 26.03.2005*/
	find_linear_regression_split(*data, left, right);
	*comparison = separate_which_group(data, *left, *right, NO, leaftype);
}

void find_quadratic_regression_condition(Instanceptr* data, Instanceptr* left, Instanceptr* right, char* comparison, LEAF_TYPE leaftype)
{
	/*!Last Changed 26.03.2005*/
	find_quadratic_regression_split(*data, left, right);
	*comparison = separate_which_group(data, *left, *right, YES, leaftype);
}

void best_single_regression_condition(Regressionconditionptr condition, Instanceptr* data, NODE_TYPE nodetype, LEAF_TYPE leaftype)
{
	/*!Last Changed 03.04.2005*/
	/*!Last Changed 26.03.2005*/
	switch (nodetype)
	 {
	  case UNIVARIATE:if (leaftype == CONSTANTLEAF)
		                   condition->featureindex = best_regression_feature(*data, &(condition->value), &(condition->comparison));
	                  else
		                   condition->featureindex = best_regression_feature_linear_leaves(*data, &(condition->value), &(condition->comparison));
																			break;
			case LINEAR    :condition->featureindex = LINEAR;
				               find_linear_regression_condition(data, &(condition->leftcenter), &(condition->rightcenter), &(condition->comparison), leaftype);
				               break;
			case QUADRATIC :condition->featureindex = QUADRATIC;
				               find_quadratic_regression_condition(data, &(condition->leftcenter), &(condition->rightcenter), &(condition->comparison), leaftype);
				               break;
   default        :printf(m1231, nodetype);
                   break;
	 }
}

void best_regression_condition_crossvalidation(Regressionconditionptr condition, Instanceptr* data, LEAF_TYPE leaftype, int correctiontype)
{
	/*!Last Changed 26.03.2005*/
 int hybridspace = get_parameter_i("hybridspace");
 FILE* errorfiles[3];
	int i, k, trainsize, testsize, bestmodel = 0, *models, dummy, dummy2, testtype;
	Instanceptr train = NULL, test = NULL;
	char **files, st[READLINELENGTH] = "-", pst[READLINELENGTH], buffer[READLINELENGTH];
	double error;
 files = safemalloc(hybridspace * sizeof(char *), "best_regression_condition_crossvalidation", 5);
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
     return;
  }
	for (i = 0; i < 5; i++)
	 {
		 divide_instancelist(data, &train, &test, 2);
			trainsize = data_size(train);
			testsize = data_size(test);
			if (trainsize <= 1 || testsize <= 1)
			 {
  			*data = restore_instancelist(*data, train);
	  		merge_instancelist(data, test);	
					bestmodel = 1;
				 break;
			 }
   for (k = 0; k < hybridspace; k++)
    {
     switch (k)
      {
       case MODEL_UNI:error = regression_tree_model_selection_error(&train, &test, UNIVARIATE, leaftype);
                      break;
       case MODEL_LIN:error = regression_tree_model_selection_error(&train, &test, LINEAR, leaftype);
                      break;
       case MODEL_QUA:error = regression_tree_model_selection_error(&train, &test, QUADRATIC, leaftype);
                      break;
      }
     set_precision(pst, NULL, "\n");
     fprintf(errorfiles[k], pst, error / (testsize + 0.0));
    }
   for (k = 0; k < hybridspace; k++)
    {
     switch (k)
      {
       case MODEL_UNI:error = regression_tree_model_selection_error(&test, &train, UNIVARIATE, leaftype);
                      break;
       case MODEL_LIN:error = regression_tree_model_selection_error(&test, &train, LINEAR, leaftype);
                      break;
       case MODEL_QUA:error = regression_tree_model_selection_error(&test, &train, QUADRATIC, leaftype);
                      break;
      }
     set_precision(pst, NULL, "\n");
     fprintf(errorfiles[k], pst, error / (trainsize + 0.0));
    }
			*data = restore_instancelist(*data, train);
			merge_instancelist(data, test);	
			train = NULL;
			test = NULL;
	 }
 for (i = 0; i < hybridspace; i++)
   fclose(errorfiles[i]);
 if (bestmodel)
	 {
		 free_2d((void**)files, hybridspace);
   best_single_regression_condition(condition, data, UNIVARIATE, leaftype);
			return;
	 }
 testtype = get_parameter_l("testtype");
 if (in_list(testtype, 9, FTEST, PAIREDT5X2, COMBINEDT5X2, PERMUTATIONTEST, PAIREDPERMUTATIONTEST, SIGNTEST, RANKSUMTEST, SIGNEDRANKTEST, NOTEST))
   models = multiple_comparison(files, hybridspace, testtype, st, &dummy, &dummy2, correctiontype);
 else
   models = multiple_comparison(files, hybridspace, FTEST, st, &dummy, &dummy2, correctiontype);
	switch (models[0])
	 {
	  case 1:best_single_regression_condition(condition, data, UNIVARIATE, leaftype);
										break;
			case 2:best_single_regression_condition(condition, data, LINEAR, leaftype);
				      break;
			case 3:best_single_regression_condition(condition, data, QUADRATIC, leaftype);
				      break;
	 }
	safe_free(models);
	free_2d((void**)files, hybridspace);
	return;
}

void find_uni_and_multi_regression_conditions(Regressionconditionptr condition, Instanceptr data, Instanceptr* leftlincenter, Instanceptr* rightlincenter, Instanceptr* leftquacenter, Instanceptr* rightquacenter, LEAF_TYPE leaftype)
{
	/*!Last Changed 27.03.2005*/
 if (leaftype == CONSTANTLEAF)
   condition->featureindex = best_regression_feature(data, &(condition->value), &(condition->comparison));
	else
		 condition->featureindex = best_regression_feature_linear_leaves(data, &(condition->value), &(condition->comparison));
 find_linear_regression_split(data, leftlincenter, rightlincenter);
 find_quadratic_regression_split(data, leftquacenter, rightquacenter);
}

void set_best_model_in_regression_rule(Regressionconditionptr condition, MODEL_TYPE bestmodel, Instanceptr leftl, Instanceptr rightl, Instanceptr leftq, Instanceptr rightq)
{
	/*!Last Changed 27.03.2005*/
	switch (bestmodel)
	 {
	  case MODEL_UNI:deallocate(leftl);
										        deallocate(rightl);
				              deallocate(leftq);
				              deallocate(rightq);
										        break;
	  case MODEL_LIN:condition->featureindex = LINEAR;
				              condition->leftcenter = leftl;
				              condition->rightcenter = rightl;
				              deallocate(leftq);
				              deallocate(rightq);
										        break;				      
			case MODEL_QUA:condition->featureindex = QUADRATIC;
				              condition->leftcenter = leftq;
				              condition->rightcenter = rightq;
				              deallocate(leftl);
				              deallocate(rightl);
										        break;				      
	 }
}

void best_regression_condition_aic_or_bic(Regressionconditionptr condition, Instanceptr* data, LEAF_TYPE leaftype, int modelselectionmethod)
{
	/*!Last Changed 27.03.2005*/
	int i, size = data_size(*data), d, bestmodel;
	double error, results[3];
 Instanceptr leftl, rightl, leftq, rightq, leftinstances, rightinstances;
 find_uni_and_multi_regression_conditions(condition, *data, &leftl, &rightl, &leftq, &rightq, leaftype);
	for (i = 0; i < 3; i++)
	 {
		 switch (i)
    {
		   case MODEL_UNI:divide_instancelist_according_to_split(data, &leftinstances, &rightinstances, condition->featureindex, condition->value);
						              d = 2;
						              break;
					case MODEL_LIN:divide_instancelist_according_centers(data, &leftinstances, &rightinstances, leftl, rightl, NO);
						              d = current_dataset->multifeaturecount;
						              break;
					case MODEL_QUA:divide_instancelist_according_centers(data, &leftinstances, &rightinstances, leftq, rightq, YES);
						              d = multifeaturecount_2d(current_dataset) + 1;
						              break;
    }
			error = (constant_or_linear_model_mse(leftinstances, leftinstances, leaftype) + constant_or_linear_model_mse(rightinstances, rightinstances, leaftype)) / size;
			if (modelselectionmethod == 1)
     results[i] = aic_squared_error(error, d, size);
			else
				 results[i] = bic_squared_error(error, d, size);
  	*data = restore_instancelist(*data, leftinstances);
	  merge_instancelist(data, rightinstances);	
	 }
	bestmodel = find_min_in_list_double(results, 3);
	set_best_model_in_regression_rule(condition, bestmodel, leftl, rightl, leftq, rightq);
}

Regressionconditionptr best_regression_condition(Instanceptr* data, Regrule_parameterptr param)
{
	/*!Last Changed 26.03.2005*/
	/*!Last Changed 02.03.2005*/
	Regressionconditionptr result;
	result = empty_regression_condition();
	switch (param->modelselectionmethod)
	 {
	  case MODEL_SELECTION_CV :best_regression_condition_crossvalidation(result, data, param->leaftype, param->correctiontype);
				                        break;
			case MODEL_SELECTION_AIC:
			case MODEL_SELECTION_BIC:best_regression_condition_aic_or_bic(result, data, param->leaftype, param->modelselectionmethod);
				                        break;
			case MODEL_SELECTION_UNI:best_single_regression_condition(result, data, UNIVARIATE, param->leaftype);
				                        break;
			case MODEL_SELECTION_LIN:best_single_regression_condition(result, data, LINEAR, param->leaftype);
				                        break;
			case MODEL_SELECTION_QUA:best_single_regression_condition(result, data, QUADRATIC, param->leaftype);
				                        break;
   default                 :printf(m1230, param->modelselectionmethod);
                            break;
	 }
	return result;
}

Regressionconditionptr create_copy_of_regression_condition(Regressionconditionptr condition)
{
	/*!Last Changed 27.03.2005*/
	/*!Last Changed 05.03.2005*/
	int oldfcount;
	Regressionconditionptr result = safemalloc(sizeof(Regressioncondition), "create_copy_of_regression_condition", 2);
	result->comparison = condition->comparison;
	result->featureindex = condition->featureindex;
	result->value = condition->value;
	if (in_list(result->featureindex, 2, LINEAR, QUADRATIC))
	 {
		 if (result->featureindex == QUADRATIC)
			 {
				 oldfcount = current_dataset->multifeaturecount;
     current_dataset->multifeaturecount = multifeaturecount_2d(current_dataset) + 1;
			 }
  	result->leftcenter = copy_of_instance(condition->leftcenter);
  	result->rightcenter = copy_of_instance(condition->rightcenter);
		 if (result->featureindex == QUADRATIC)
				 current_dataset->multifeaturecount = oldfcount;
	 }
	result->next = NULL;
	return result;
}

Regressionruleptr create_copy_of_regression_rule(Regressionruleptr rule)
{
	/*!Last Changed 05.03.2005*/
	Regressionruleptr result = safemalloc(sizeof(Regressionrule), "create_copy_of_rule", 1);
	Regressionconditionptr newcondition, tmp, last = NULL;
	result->regressionvalue = rule->regressionvalue;
	result->conditioncount = rule->conditioncount;
	result->next = NULL;
	result->start = NULL;
	result->leaftype = rule->leaftype;
	if (rule->leaftype == LINEARLEAF)
  	result->w = matrix_copy(rule->w);
	tmp = rule->start;
	while (tmp)
	 {
		 newcondition = create_copy_of_regression_condition(tmp);
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

Regressionruleptr grow_regression_revision_rule(Instanceptr* growset, Regressionruleptr rule, Regrule_parameterptr param)
{
	/*!Last Changed 05.03.2005*/
	Regressionruleptr result = create_copy_of_regression_rule(rule);
	Regressionconditionptr newcondition;
	matrix w;
	double regvalue;
	Instanceptr obey = NULL, notobey;
	notobey = (*growset);
 remove_regression_covered(&notobey, &obey, result);
 while (can_be_divided_rule(obey, param->sigma, &w, &regvalue, rule->leaftype))
	 {
			newcondition = best_regression_condition(&obey, param);
			if (newcondition->featureindex == LEAF_NODE || newcondition->comparison == '=')
			 {
				 safe_free(newcondition);
				 break;
			 }
			else
			 {
				 add_regression_condition(result, newcondition);
					send_regression_exceptions(newcondition, &obey, &notobey);
			 }
		 if (rule->leaftype == LINEARLEAF)
  		 matrix_free(w);
	 }
	result->covered = data_size(obey);
	if (notobey)
	 {
   merge_instancelist(&notobey, obey);
	  (*growset) = notobey;
	 }
	else
		 (*growset) = obey;
 if (rule->leaftype == CONSTANTLEAF)
	  result->regressionvalue = regvalue;
	else
   result->w = w;
	return result;
}

Regressionruleptr grow_regression_rule(Instanceptr* growset, Regrule_parameterptr param)
{
	/*!Last Changed 02.03.2005*/
	Regressionruleptr result;
	Regressionconditionptr newcondition;
	matrix w;
	double regvalue;
	Instanceptr obey, notobey = NULL;
 result = empty_regression_rule();
	result->leaftype = param->leaftype;
	obey = (*growset);
 while (can_be_divided_rule(obey, param->sigma, &w, &regvalue, param->leaftype))
	 {
			newcondition = best_regression_condition(&obey, param);
			if (newcondition->featureindex == LEAF_NODE || newcondition->comparison == '=')
			 {
				 safe_free(newcondition);
				 break;
			 }
			else
			 {
				 add_regression_condition(result, newcondition);
					send_regression_exceptions(newcondition, &obey, &notobey);
			 }
		 if (param->leaftype == LINEARLEAF)
  		 matrix_free(w);
	 }
	result->covered = data_size(obey);
	if (notobey)
	 {
   merge_instancelist(&notobey, obey);
	  (*growset) = notobey;
	 }
	else
		 (*growset) = obey;
 if (param->leaftype == CONSTANTLEAF)
	  result->regressionvalue = regvalue;
	else
   result->w = w;
	return result;
}

void prune_regression_rule(Regressionruleptr rule, Instanceptr dataset, int modelselectionmethod)
{
	/*!Last Changed 04.03.2005*/
	double bestgen, gen;
	int improved = 1;
	Regressioncondition *best, *bestbefore, *tmpcondition, *tmpbefore;
	bestgen = generalization_error_of_regression_rule(rule, dataset, modelselectionmethod);
	while (improved)
	 {
		 improved = 0;
			tmpbefore = NULL;
   tmpcondition = rule->start;
			while (tmpcondition)
			 {
				 remove_regression_condition_after(rule, tmpbefore, tmpcondition);
	    gen = generalization_error_of_regression_rule(rule, dataset, modelselectionmethod);
					if (gen < bestgen)
					 {
						 bestgen = gen;
							bestbefore = tmpbefore;
							best = tmpcondition;
							improved = 1;
					 }
					add_regression_condition_after(rule, tmpbefore, tmpcondition);
				 tmpbefore = tmpcondition;
				 tmpcondition = tmpcondition->next;
			 }
			if (improved)
			 {
				 remove_regression_condition_after(rule, bestbefore, best);
					safe_free(best);
			 }
	 }
}

Regressionruleset regression_rule_learner(Instanceptr* traindata, Instanceptr* cvdata, Regrule_parameterptr param)
{
	/*!Last Changed 06.03.2005*/
	Regressionruleset result;
	matrix w;
	double regvalue;
	int exitnow, i;
	Regressionruleptr newrule;
	Instanceptr grow, prune, coveredtrain = NULL;
	result = empty_regression_ruleset();
	result.leaftype = param->leaftype;
	while (can_be_divided_rule(*traindata, param->sigma, &w, &regvalue, param->leaftype))
	 {
		 exitnow = 0;
			if (*cvdata)
				{
     divide_instancelist(traindata, &prune, &grow, 3);
     newrule = grow_regression_rule(&grow, param);
				}
			else
					newrule = grow_regression_rule(traindata, param);
  	if (newrule && newrule->start)
			 {
				 if (*cvdata)
						 prune_regression_rule(newrule, prune, param->modelselectionmethod);
					else
						 prune_regression_rule(newrule, *traindata, param->modelselectionmethod);
					if (newrule->start)
					 {
						 add_regression_rule(&result, newrule);
							if (*cvdata)
							 {
  				   *traindata = restore_instancelist(*traindata, grow);
  				  	merge_instancelist(traindata, prune);
							 }
					 }
					else
						 exitnow = 1;
			 }
			else
				 exitnow = 1;
			if (!exitnow)
			 {
				 remove_regression_covered(traindata, &coveredtrain, newrule);
  		 if (param->leaftype == LINEARLEAF)
    		 matrix_free(w);				 
			 }
			else
			 {
				 if (*cvdata)
					 {
  		  	*traindata = restore_instancelist(*traindata, grow);
  			  merge_instancelist(traindata, prune);
					 }
				 break;
			 }
	 }
	if (param->leaftype == CONSTANTLEAF)
		 result.regressionvalue = regvalue;
	else
  	result.w = w;
	merge_instancelist(traindata, coveredtrain);
	for (i = 0; i < param->optimizecount; i++)
	 {
   optimize_regression_ruleset(&result, traindata, cvdata, param);
			if (*cvdata)
  			simplify_regression_ruleset(&result, *cvdata, param->modelselectionmethod);
			else
  			simplify_regression_ruleset(&result, *traindata, param->modelselectionmethod);
	 }
	return result;
}
