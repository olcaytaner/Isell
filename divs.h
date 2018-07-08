#ifndef __divs_H
#define __divs_H
#include "classification.h"
#include "typedef.h"

int           classify_using_divs_oracle(Instanceptr current, Model_divsptr model, int M, int epsilon);
int           condition_cover_instance3(Instanceptr instance, Condition condition);
Condition*    discriminating_rule(Instanceptr training, Instanceptr counter, int* count);
BOOLEAN       epsilon_neighbor(Instanceptr current, Condition** training_example, int rulecount, int* conditioncount, int M, int epsilon);
Model_divsptr learn_ruleset_divs(Instanceptr instances);
BOOLEAN       m_belongs(Instanceptr current, Condition* counter_example, int conditioncount, int M);
void          tune_consistency_and_generality(Model_divsptr model, Instanceptr cvdata);

#endif
