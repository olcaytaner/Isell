#ifndef __decisionforest_H
#define __decisionforest_H

#include "parameter.h"
#include "typedef.h"

int create_kforestchildren(Decisionnodeptr node, Kforest_parameterptr p);
int create_randomforestchildren(Decisionnodeptr node, Randomforest_parameterptr p);

#endif
