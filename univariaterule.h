#ifndef __univariaterule_H
#define __univariaterule_H

#include "classification.h"
#include "parameter.h"
#include "typedef.h"

int                  are_conditions_equal(Decisionconditionptr condition1, Decisionconditionptr condition2);
int                  are_rules_equal(Decisionruleptr rule1, Decisionruleptr rule2);
int                  can_delete_rule(Decisionruleptr rule, int* covered, int* uncovered, int* fp, int* fn, double proportion, int possibleconditioncount);
void                 check_for_bestgain(int positive, int total, double it, double* bestgain, Decisionconditionptr result, char comparison, double value, int featureindex);
double               description_length_with_or_without_rule(Ruleset* r, int positive, int startrule, Instanceptr list1, Instanceptr list2, Instanceptr list3, Instanceptr list4, double proportion, int with);
int                exception_count_of_ruleset(Ruleset r, Instanceptr instances, int positive);
Decisionconditionptr find_best_condition(Instanceptr growset, int positive, Ripper_parameterptr param);
Decisionconditionptr find_best_condition_for_realized_features(Instanceptr growset, int positive);
Decisionruleptr      growrule(Instanceptr* growset, int positive, Ripper_parameterptr param);
void                 insert_rule_and_update_counts(Decisionruleptr newrule, Ruleset* r, int* covered, int* uncovered, int* fp, int* fn);
int                  possible_condition_count(Instanceptr instances);
void                 prunerule(Decisionruleptr rule, Instanceptr pruneset, int positive, int reptype);
void                 prunerule_in_optimize_phase(Decisionruleptr rule, Instanceptr pruneset, int positive);
double               relative_compression(Ruleset* r, int positive, int startrule, Instanceptr list1, Instanceptr list2, Instanceptr list3, Instanceptr list4, double proportion);
void                 remove_covered(Instanceptr* list1, Instanceptr* list2, Decisionruleptr rule);
void                 remove_covered_by_ruleset(Instanceptr* list1, Instanceptr* list2, Ruleset r, int startcount);
int                  rule_exists(Ruleset ruleset, Decisionruleptr rule);
double               rule_value(Decisionruleptr rule, int positive, Instanceptr instances);
double               rule_value_metric(int n, int p, int N, int P, int reptype);
void                 send_exceptions(Decisionconditionptr condition, Instanceptr* obey, Instanceptr* notobey);
void                 update_best_discrete_condition(Korderingsplit* split, int* perm[], int* rangelist, int* iter, int** featurecounts, int partitioncounts[], double* bestgain);
void                 update_counts_rules(Decisionruleptr start, Instanceptr list1, int* covered, int* uncovered, int* fp, int* fn);

#endif
