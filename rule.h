#ifndef __rule_H
#define __rule_H

#include "typedef.h"
#include "parameter.h"

void                 add_condition(Decisionruleptr rule, Decisionconditionptr condition);
void                 add_condition_after(Decisionruleptr rule, Decisionconditionptr before, Decisionconditionptr inserted);
void                 add_rule(Ruleset* set, Decisionruleptr rule);
void                 add_rule_after(Ruleset* r, Decisionruleptr before, Decisionruleptr inserted);
Decisionruleptr      add_rule_operator(Ruleset* result, Instanceptr* instances, double* bestdl);
void                 calculate_performance_for_prune(Instanceptr pruneset, Decisionrule rule, int* p, int* n, int positive);
BOOLEAN              check_rule_consistency(Decisionruleptr rule);
BOOLEAN              check_ruleset_consistency(Ruleset set);
int                  complexity_of_rule(Decisionruleptr rule);
int                  complexity_of_ruleset(Ruleset set, int positive);
int                  condition_count(Ruleset r);
int                  condition_cover_instance(Instanceptr instance, Decisionconditionptr condition);
int                  condition_cover_instance2(Instanceptr instance, Decisionconditionptr condition);
Decisioncondition    copy_of_condition(Decisioncondition condition);
void                 count_examples_for_condition(Decisionconditionptr c, int* p, int* n, int posclass, Instanceptr list);
void                 count_exceptions_for_ruleset(Ruleset r, Instanceptr instances, int* C, int* U, int* fp, int* fn);
Decisionconditionptr create_copy_of_condition(Decisionconditionptr condition);
Decisionruleptr      create_copy_of_rule(Decisionruleptr rule);
Ruleset*             create_copy_of_ruleset(Ruleset* r);
double               description_length_of_exceptions(int C, int U, int fp, int fn, double proportion);
double               description_length_of_rule(Decisionrule r, int possibleconditioncount);
double               description_length_of_ruleset(Ruleset r, int positive);
Decisionconditionptr empty_condition();
Decisionruleptr      empty_rule(int classno);
void                 empty_ruleset(Ruleset* r);
int                  error_of_condition(Decisionconditionptr c, Instanceptr list, int positive);
int                  error_of_ruleset(Ruleset set,Instanceptr traindata);
double               error_rate_of_rule(Decisionrule r);
void                 exception_handling(Ruleset r, Instanceptr instances, int positive, int* C, int* U, int* fp, int* fn);
Decisionruleptr      extract_rule_from_leaf(Decisionnodeptr leaf);
int                  false_positive_count(Ruleset r, Instanceptr instances);
void                 free_rule(Decisionrule rule);
void                 free_ruleset(Ruleset r);
void                 free_condition(Decisioncondition condition);
Decisionruleptr      get_next_class_rule(Decisionruleptr previousclassrule);
double               information(int positive, int total);
void                 initialize_counts(Decisionruleptr start);
double               laplace_estimate(int falsepositives, int n);
double               laplace_estimate_of_rule(Decisionrule r);
double               laplace_estimate_of_rule_kclass(Decisionrule r);
void                 merge_ruleset(Ruleset* set1, Ruleset set2);
void                 print_counts(char* st,Instanceptr list);
Decisionruleptr      prune_rule_operator(Ruleset* result, Instanceptr instances, double* bestdl, int* improved, Decisionconditionptr* deletedcondition);
void                 remove_condition(Decisionruleptr rule, Decisionconditionptr deleted);
void                 remove_condition_after(Decisionruleptr rule, Decisionconditionptr before, Decisionconditionptr deleted);
void                 remove_rule(Ruleset* set, Decisionruleptr rule);
void                 remove_rule_after(Ruleset* r, Decisionruleptr before, Decisionruleptr deleted);
Decisionruleptr      remove_rule_operator(Ruleset* result, Instanceptr instances, double* bestdl, int* improved);
void                 remove_rules_of_class(Ruleset* set, int index);
int                  return_error_and_free_condition(Decisionconditionptr condition, Instanceptr test, int positive);
Decisionruleptr      revise_rule_operator(Ruleset* result, Instanceptr* instances, double* bestdl, int* improved, Decisionconditionptr* bestcondition);
int                  rule_cover_instance(Instanceptr instance, Decisionrule rule);
int                  rule_cover_instance2(Instanceptr instance, Decisionrule rule);
int                  rule_cover_negative(Instanceptr instances, Decisionrule rule);
Ruleset              rule_learner(Instanceptr instances);
int                  ruleset_complexity(Ruleset set);
int                  ruleset_cover_instance(Instanceptr instance, Ruleset r);
double               S(int n, int k, double p);
double               test_rule(Decisionrule rule, Instanceptr instances);
void                 update_counts(Decisionruleptr rule, Instanceptr list1, int initialize);
void                 update_counts_ruleset(Ruleset r, Instanceptr list);
void                 write_condition(Decisionconditionptr condition);
void                 write_rule(Decisionruleptr rule);
void                 write_ruleset(Ruleset r);

#endif
