#ifndef __multivariaterule_H
#define __multivariaterule_H

#include "typedef.h"

int                count_multivariate_condition_over_threshold(Instanceptr list, Decisionconditionptr condition);
int                error_of_ruleset_with_deleted_rules(Ruleset set, Instanceptr data, int* deleted);
Decisionconditionptr find_best_multivariate_condition(Instanceptr* growset, int positive, MULTIVARIATE_TYPE multivariate_type, Ripper_parameterptr param);
Decisionconditionptr find_best_multivariate_condition_lda(Instanceptr* growset, int positive, MULTIVARIATE_TYPE multivariate_type, double variance_explained);
Decisionconditionptr find_best_multivariate_condition_lp(Instanceptr* growset, int positive);
Decisionconditionptr find_best_multivariate_condition_svm(Instanceptr* growset, int positive);
Decisionruleptr    growrule_multivariate(Instanceptr* growset, int positive, Ripper_parameterptr param);
Decisionruleptr    grow_revision_rule_multivariate(Instanceptr* growset, Decisionruleptr rule, int positive, Ripper_parameterptr param);
Instanceptr        learn_multivariate_rules_for_positive_class(Instanceptr* instances, Ruleset* ruleset, int posclass, Instanceptr cvdata, Ripper_parameterptr param);
Ruleset            multivariate_rule_learner(Instanceptr* instances, Instanceptr cvdata, Ripper_parameterptr param);
void               optimize_ruleset_multivariate(Ruleset* r, int positive, Instanceptr* instances, Instanceptr cvdata, Ripper_parameterptr param);
void               simplify_ruleset_multivariate(Ruleset* r, int positive, Instanceptr cvdata);
int                validation_error_after_pruning(Ruleset r, Instanceptr cvdata, int positive);

#endif
