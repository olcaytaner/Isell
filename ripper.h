#ifndef __ripper_H
#define __ripper_H
#include "parameter.h"
#include "typedef.h"

Decisionruleptr grow_revision_rule(Instanceptr* growset, Decisionruleptr rule, int positive, Ripper_parameterptr param);
Ruleset         learn_ruleset_ripper(Instanceptr* instances, int reptype, Ripper_parameterptr param);
void            optimize_ruleset(Ruleset* r, int positive, Instanceptr* instances, double proportion, Ripper_parameterptr param);
void            simplify_ruleset(Ruleset* r, int positive, Instanceptr list1, Instanceptr list2, double proportion);

#endif
