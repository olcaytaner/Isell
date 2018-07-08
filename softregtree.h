#ifndef __softtree_H
#define __softtree_H
#include "parameter.h"
#include "typedef.h"

void    copy_soft_regtree_ws(Regressionnodeptr dest, Regressionnodeptr src);
int     create_softregtree(Regressionnodeptr rootnode, Regressionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param);
BOOLEAN find_best_soft_regtree_split(Regressionnodeptr rootnode, Regressionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param);
void    free_soft_regtree_ws(Regressionnodeptr node);
void    gradient_descent_soft_regtree(Regressionnodeptr rootnode, Regressionnodeptr node, Softregtree_parameterptr param, double eta, double lambda);
double  mse_error_of_soft_regression_tree(Regressionnodeptr rootnode, Instanceptr list);
double  soft_output_of_subregressiontree(Regressionnodeptr subtree, double input);
double  soft_regtree_output(Regressionnodeptr node, Instanceptr inst);

#endif
