#ifndef __omnivariatetree_H
#define __omnivariatetree_H

#include "decisiontree.h"

void        calculate_counts_for_ldt_splits(int univariate, Decisionnodeptr node, int *C, int *U, int *fp, int *fn);
int         create_omnildtchildren(Decisionnodeptr node, Multildt_parameterptr param);
void        decisiontree_node_types(Decisionnodeptr node, char st[READLINELENGTH], int level);
int         find_best_hybrid_ldt_split(Decisionnodeptr node, Multildt_parameterptr param);
int         find_best_hybrid_ldt_split_according_char(Decisionnodeptr node, char model, double variance_explained);
int         find_best_hybrid_ldt_split_aic(Decisionnodeptr node, Multildt_parameterptr param);
int         find_best_hybrid_ldt_split_bic(Decisionnodeptr node, Multildt_parameterptr param);
int         find_best_hybrid_ldt_split_crossvalidation(Decisionnodeptr node, Multildt_parameterptr param);
int         find_best_hybrid_ldt_split_single_model(Decisionnodeptr node, NODE_TYPE model, double variance_explained);
int         find_best_hybrid_ldt_split_srm(Decisionnodeptr node, Multildt_parameterptr param);
int         find_error_and_free_matrix(Instanceptr* train, Instanceptr test, NODE_TYPE node_type, double variance_explained);
void        find_uni_and_multi_splits(Decisionnodeptr node, matrix* qw, double* qw0, double points[3], double variance_explained, int hybridspace);
void        prune_tree_according_to_model_selection_method(Decisionnodeptr root, int modelselectionmethod);
void        prune_tree_global(Decisionnodeptr root, Decisionnodeptr currentnode, int modelselectionmethod);
int         set_best_hybrid_split(double results[3], Decisionnodeptr node, matrix qw, double qw0, int hybridspace);

#endif
