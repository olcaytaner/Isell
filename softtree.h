#ifndef __softtree_H
#define __softtree_H
#include "parameter.h"
#include "typedef.h"

void    copy_soft_tree_ws(Decisionnodeptr dest, Decisionnodeptr src);
void    copy_soft_tree_ws_K_class(Decisionnodeptr dest, Decisionnodeptr src);
int     create_softtree(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param);
int     create_softtree_K_class(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param);
int     error_of_soft_tree(Decisionnodeptr rootnode, Instanceptr list);
int     error_of_soft_tree_K_class(Decisionnodeptr rootnode, Instanceptr list);
BOOLEAN find_best_soft_tree_split(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param);
BOOLEAN find_best_soft_tree_split_K_class(Decisionnodeptr rootnode, Decisionnodeptr node, Instanceptr cvdata, Softregtree_parameterptr param);
void    free_soft_tree_ws(Decisionnodeptr node);
void    free_soft_tree_ws_K_class(Decisionnodeptr node);
void    gradient_descent_soft_tree(Decisionnodeptr rootnode, Decisionnodeptr node, Softregtree_parameterptr param, double eta, double lambda);
void    gradient_descent_soft_tree_K_class(Decisionnodeptr rootnode, Decisionnodeptr node, Softregtree_parameterptr param, double eta);
void    soft_tree_dimension_reduction(Decisionnodeptr node, char st[READLINELENGTH], int level);
double  soft_tree_output(Decisionnodeptr node, Instanceptr inst);
double* soft_tree_output_K_class(Decisionnodeptr node, Instanceptr inst);

#endif
