#ifndef __omnivariaterule_H
#define __omnivariaterule_H

Decisionconditionptr find_best_hybrid_condition(Instanceptr* growset, int positive, Ripper_parameterptr param);
Decisionconditionptr find_best_hybrid_condition_aic(Instanceptr* growset, int positive, Ripper_parameterptr param);
Decisionconditionptr find_best_hybrid_condition_bic(Instanceptr* growset, int positive, Ripper_parameterptr param);
Decisionconditionptr find_best_hybrid_condition_crossvalidation(Instanceptr* growset, int positive, Ripper_parameterptr param);
double             generalization_error_after_pruning(Ruleset r, Instanceptr data, int positive, int modelselectionmethod);
double             generalization_error_of_ruleset_with_deleted_rules(Ruleset set, Instanceptr data, int* deleted, int positive, int modelselectionmethod);
Decisionruleptr    growrule_omnivariate(Instanceptr* growset, int positive, Ripper_parameterptr param);
Decisionruleptr    grow_revision_rule_omnivariate(Instanceptr* growset, Decisionruleptr rule, int positive, Ripper_parameterptr param);
Ruleset            hybrid_rule_learner(Instanceptr* instances, Instanceptr cvdata, Ripper_parameterptr param);
double             log_likelihood_for_rule_induction(int poscount, int negcount);
double             log_likelihood_of_condition(Decisionconditionptr c, Instanceptr list, int positive, int* covered);
double             log_likelihood_of_rule(Decisionruleptr c, Instanceptr list, int* covered);
double             log_likelihood_of_ruleset(Ruleset set, Instanceptr list, int positive, int* covered);
void               optimize_ruleset_modelselection(Ruleset* r, int positive, Instanceptr* instances, Ripper_parameterptr param);
void               prunerule_with_modelselection(Decisionruleptr rule, Instanceptr dataset, int modelselectionmethod);
void               simplify_ruleset_modelselection(Ruleset* r, int positive, Instanceptr instances, int modelselectionmethod);
void               write_datasizes_and_nodetypes_rule(Ruleset r, char* st);
void               write_nodetypes_rule(Ruleset r, char* st);

#endif
