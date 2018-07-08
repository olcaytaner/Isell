#ifndef __regressionrule_H
#define __regressionrule_H

void                 add_regression_condition(Regressionruleptr rule, Regressionconditionptr condition);
void                 add_regression_condition_after(Regressionruleptr rule, Regressionconditionptr before, Regressionconditionptr inserted);
void                 add_regression_rule(Regressionruleset* set, Regressionruleptr rule);
void                 add_regression_rule_after(Regressionruleset* r, Regressionruleptr before, Regressionruleptr inserted);
Regressionconditionptr best_regression_condition(Instanceptr* data, Regrule_parameterptr param);
void                 best_regression_condition_aic_or_bic(Regressionconditionptr condition, Instanceptr* data, LEAF_TYPE leaftype, int modelselectionmethod);
void                 best_regression_condition_crossvalidation(Regressionconditionptr condition, Instanceptr* data, LEAF_TYPE leaftype, int correctiontype);
void                 best_single_regression_condition(Regressionconditionptr condition, Instanceptr* data, NODE_TYPE nodetype, LEAF_TYPE leaftype);
int                  can_be_divided_rule(Instanceptr data, double sigma, matrix* w, double* regvalue, LEAF_TYPE leaftype);
int                  complexity_of_regression_rule(Regressionruleptr rule);
int                  complexity_of_regression_ruleset(Regressionruleset set);
Regressionconditionptr create_copy_of_regression_condition(Regressionconditionptr condition);
Regressionruleptr      create_copy_of_regression_rule(Regressionruleptr rule);
Regressionconditionptr empty_regression_condition();
Regressionruleptr      empty_regression_rule();
Regressionruleset    empty_regression_ruleset();
void                 find_linear_regression_condition(Instanceptr* data, Instanceptr* left, Instanceptr* right, char* comparison, LEAF_TYPE leaftype);
void                 find_quadratic_regression_condition(Instanceptr* data, Instanceptr* left, Instanceptr* right, char* comparison, LEAF_TYPE leaftype);
void                 find_uni_and_multi_regression_conditions(Regressionconditionptr condition, Instanceptr data, Instanceptr* leftlincenter, Instanceptr* rightlincenter, Instanceptr* leftquacenter, Instanceptr* rightquacenter, LEAF_TYPE leaftype);
void                 free_regression_rule(Regressionrule rule);
void                 free_regression_ruleset(Regressionruleset r);
double               generalization_error_of_regression_rule(Regressionruleptr rule, Instanceptr data, MODEL_SELECTION_TYPE modelselectionmethod);
double               generalization_error_of_regression_ruleset(Regressionruleset r, Instanceptr data, MODEL_SELECTION_TYPE modelselectionmethod);
Regressionruleptr      grow_regression_revision_rule(Instanceptr* growset, Regressionruleptr rule, Regrule_parameterptr param);
Regressionruleptr      grow_regression_rule(Instanceptr* growset, Regrule_parameterptr param);
void                 prune_regression_rule(Regressionruleptr rule, Instanceptr dataset, int modelselectionmethod);
double               mse_of_rule(Regressionruleptr c, Instanceptr list, int* covered);
double               mse_of_ruleset(Regressionruleset set, Instanceptr list, int* count);
void                 optimize_regression_ruleset(Regressionruleset* r, Instanceptr* traindata, Instanceptr* cvdata, Regrule_parameterptr param);
int                  regression_condition_count(Regressionruleset r);
int                  regression_condition_cover_instance(Instanceptr instance, Regressionconditionptr condition);
int                  regression_rule_cover_instance(Instanceptr instance, Regressionruleptr rule);
Regressionruleset    regression_rule_learner(Instanceptr* traindata, Instanceptr* cvdata, Regrule_parameterptr param);
void                 remove_regression_condition_after(Regressionruleptr rule, Regressionconditionptr before, Regressionconditionptr deleted);
void                 remove_regression_covered(Instanceptr* list1, Instanceptr* list2, Regressionruleptr rule);
void                 remove_regression_rule_after(Regressionruleset* r, Regressionruleptr before, Regressionruleptr deleted);
void                 send_regression_exceptions(Regressionconditionptr condition, Instanceptr* obey, Instanceptr* notobey);
char                 separate_which_group(Instanceptr* data, Instanceptr left, Instanceptr right, int quadratic, LEAF_TYPE leaftype);
void                 set_best_model_in_regression_rule(Regressionconditionptr condition, MODEL_TYPE bestmodel, Instanceptr leftl, Instanceptr rightl, Instanceptr leftq, Instanceptr rightq);
void                 simplify_regression_ruleset(Regressionruleset* r, Instanceptr data, int modelselectionmethod);
void                 write_nodetypes_regression_rule(Regressionruleset r, char* st);

#endif
