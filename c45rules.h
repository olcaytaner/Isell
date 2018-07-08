#ifndef __c45rules_H
#define __c45rules_H

#include "typedef.h"

void             add_rules_from_tree(Ruleset* r, Decisionnodeptr tree);
Ruleset          check_all_subsets(Decisionruleptr start, int rulecount, Instanceptr traindata, int conditioncount);
Ruleset          check_different_percentages(Decisionruleptr start, int rulecount, Instanceptr traindata, int conditioncount);
void             choose_default_class(Ruleset* r, Instanceptr traindata);
Decisionruleptr* create_array_from_link_list_of_rules(Decisionruleptr start, int rulecount);
void             create_percentage_of_ruleset(Decisionruleptr* rules, int rulecount, int percentage, Ruleset* inlist, Ruleset* outlist);
Ruleset          create_ruleset_from_decision_tree(Decisionnodeptr tree);
void             create_subset_of_ruleset(Ruleset* result, Decisionruleptr* rules, int* subset, int subsetcount);
double           description_length_of_all(Ruleset r, int positive, int count, ...);
void             find_class_rulesets(Ruleset* r, Instanceptr traindata);
void             generalize_c45rules(Ruleset* r, Instanceptr traindata);
void             generalize_rule(Decisionruleptr rule, Instanceptr traindata);
int              is_pessimistic_error_rate_smaller(int Y1, int Y2, int E1, int E2);
void             merge_rulesets(Ruleset* r, Ruleset sets[MAXCLASSES], int classcount);
void             polish_ruleset(Ruleset* r, Instanceptr traindata);
void             rank_classes(Ruleset* r, Instanceptr traindata);
void             remove_rules_according_indexes_of_rule(Decisionruleptr* rules, int rulecount, int* subset, int subsetcount);
void             remove_rules_according_rules_themselves(Decisionruleptr* rules, int rulecount, Decisionruleptr* subsetrules, int subsetcount);
double           search_best_ruleset(Ruleset* inlist, Ruleset* outlist, Instanceptr instances, int classno);
void             sort_rules(Ruleset* r);

#endif
