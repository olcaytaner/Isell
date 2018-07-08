#ifndef __rise_H
#define __rise_H
#include "classification.h"
#include "typedef.h"

int             accuracy_of_ruleset(Decisionruleptr* nearestrule, Instanceptr instances);
Decisionruleptr create_rule_from_instance(Instanceptr inst);
matrix          distance_between_discrete_values_of_feature(int*** feature_counts, int fno);
double          distance_to_rule(Model_riseptr m, Instanceptr inst, Decisionruleptr rule);
Model_riseptr   learn_ruleset_rise(Instanceptr instances);
Decisionruleptr most_specific_generalization(Decisionruleptr rule, Instanceptr nearest);
Instanceptr     nearest_example_not_covered(Model_riseptr m, Decisionruleptr rule, Instanceptr instances);
Decisionruleptr nearest_rule(Model_riseptr m, Instanceptr instance, int leaveoneout);
void            update_nearest_rule(Model_riseptr m, Decisionruleptr* nearestrule, Instanceptr instances, int operation, Decisionruleptr rule);

#endif
