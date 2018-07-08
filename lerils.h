#ifndef __lerils_H
#define __lerils_H
#include "parameter.h"
#include "typedef.h"

double  description_length_of_rule_lerils(Decisionconditionptr* rule, int* relevant, Instanceptr instances, int positive);
int     description_length_of_ruleset_lerils(Ruleset rule_pool, int* included, Instanceptr instances, int positive);
Ruleset generate_rule_pool(Instanceptr instances, int positive, int M, double p1);
Ruleset learn_ruleset_lerils(Instanceptr* instances, Lerils_parameterptr param);
Ruleset learn_ruleset_lerils_oneclass(Instanceptr instances, Lerils_parameterptr param, int positive);
Ruleset optimal_subset_of_rule_pool(Ruleset rule_pool, Instanceptr instances, int positive, int restarts);
Ruleset order_rulesets(Ruleset* sets, Instanceptr* instances);
double* possible_split_points(Instanceptr instances, int fno, int* count);
int     rule_cover_instance_lerils(Decisionconditionptr* rule, int* relevant, Instanceptr inst);
int     split_point_lower_and_upper_limits(double splitpoint, double* limits, int count);

#endif
