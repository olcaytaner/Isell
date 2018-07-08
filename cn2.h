#ifndef __cn2_H
#define __cn2_H
#include "parameter.h"
#include "typedef.h"

void            add_candidate_to_newstar(Decisionruleptr candidate, Ruleset* newstar, int maxstar);
Decisionruleptr find_best_complex(Instanceptr* instances, int positive, int maxstar, double positive_ratio);
Decisionruleptr generate_candidate_for_newstar(Decisionruleptr current, int fno, char comparison, double split, int falsepositives, int covered);
int             is_better_candidate(Ruleset newstar, int maxstar, double laplaceestimate);
Ruleset         learn_ruleset_cn2(Instanceptr* instances, Cn2_parameterptr param);
int             rule_significantly_better(Decisionruleptr current, double positive_ratio);
void            specialize_complex(Decisionruleptr current, Ruleset* newstar, Instanceptr* instances, int positive, int maxstar);

#endif
