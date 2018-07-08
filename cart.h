#ifndef __cart_H
#define __cart_H
#include "parameter.h"
#include "typedef.h"

int create_cartchildren(Decisionnodeptr node, C45_parameterptr param);
int find_best_cart_split(Decisionnodeptr node);

#endif
