#ifndef __svmregtree_H
#define __svmregtree_H
#include "parameter.h"
#include "typedef.h"

int               find_best_svmregtree_feature(Instanceptr instances, double* split, double C, LEAF_TYPE leaftype);
int               create_svmregtree(Regressionnodeptr node, Regtree_parameterptr param);
int               optimize_C_for_svmregtreenode(Regressionnodeptr node, LEAF_TYPE leaftype);

#endif

