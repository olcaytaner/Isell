#ifndef __regressiontree_H
#define __regressiontree_H

enum child_type{
      LEFTCHILD = 0,
      RIGHTCHILD};

typedef enum child_type CHILD_TYPE;
      
#include "decisiontree.h"
#include "typedef.h"
#include "options.h"
#include "parameter.h"

void              accumulate_instances_regression_leaf(Instanceptr* data, Instanceptr* cvdata, Regressionnodeptr leaf);
void              accumulate_instances_regression_tree(Instanceptr* data, Instanceptr* cvdata, Regressionnodeptr root);
int               best_hybrid_regression_tree_split(Regressionnodeptr node, Regtree_parameterptr param);
int               best_hybrid_regression_tree_split_crossvalidation(Regressionnodeptr node, Regtree_parameterptr param);
int               best_hybrid_regression_tree_split_aic_or_bic(Regressionnodeptr node, Regtree_parameterptr param);
int               best_regression_feature(Instanceptr instances,double* split, char* comparison);
int               best_regression_feature_linear_leaves(Instanceptr instances, double* split, char* comparison);
int               best_single_regression_tree_split(Regressionnodeptr node, int nodetype);
int               can_be_divided_regression(Regressionnodeptr node, Regtree_parameterptr param);
int               complexity_of_regression_tree(Regressionnodeptr node);
double            constant_or_linear_model_mse(Instanceptr train, Instanceptr test, LEAF_TYPE leaftype);
void              create_regression_matrices(Instanceptr list, matrix* X, matrix* r);
void              create_regression_matrices_for_array_of_instances(Instanceptr list, int start, int end, matrix* X, matrix* r);
int               create_regtree(Regressionnodeptr node, Regtree_parameterptr param);
Regressionnodeptr createnewregnode(Regressionnodeptr parentnode, int howmany, LEAF_TYPE leaftype);
void              deallocate_regression_tree(Regressionnodeptr node);
int               depth_of_regression_tree(Regressionnodeptr node);
double            estimate_sigma_square_with_heuristic(Instanceptr data);
double            estimate_sigma_square_with_knn(Instanceptr data, int nearcount);
int               find_linear_regression_split(Instanceptr data, Instanceptr* left, Instanceptr* right);
int               find_quadratic_regression_split(Instanceptr data, Instanceptr* left, Instanceptr* right);
void              find_uni_and_multi_regression_splits(Regressionnodeptr node, Instanceptr* leftlincenter, Instanceptr* rightlincenter, Instanceptr* leftquacenter, Instanceptr* rightquacenter);
void              inorder_traversal_regression_tree(Regressionnodeptr node, int* index);
double            instance_regression_error(matrix w, double regressionvalue, Instanceptr instance, LEAF_TYPE leaftype);
int               level_of_regression_node(Regressionnodeptr node);
void              level_of_regression_tree(Regressionnodeptr node);
double            linear_fit_value(matrix w, Instanceptr inst);
void              make_leaf_regression_node(Regressionnodeptr node, LEAF_TYPE leaftype);
int               make_regression_children(Regressionnodeptr node, LEAF_TYPE leaftype);
double            mse_error_of_regression_tree(Regressionnodeptr node);
matrix            multivariate_regression(matrix X, matrix r);
CHILD_TYPE        nearest_center_in_regression_tree(Instanceptr tmp, NODE_TYPE nodetype, Instanceptr leftcenter, Instanceptr rightcenter);
Regressionnodeptr next_regression_node(Regressionnodeptr currentnode, Instanceptr tmp);
void              prune_regression_tree(Regressionnodeptr root, Regtree_parameterptr param);
int               regressiontree_leaf_count(Regressionnodeptr node);
double            regression_tree_model_selection_error(Instanceptr* train, Instanceptr *test, NODE_TYPE nodetype, LEAF_TYPE leaftype);
int               regressiontree_node_count(Regressionnodeptr node);
void              regressiontree_node_types(Regressionnodeptr node, char st[READLINELENGTH], int level);
void              set_best_model_in_regression_tree(Regressionnodeptr node, int bestmodel, Instanceptr leftl, Instanceptr rightl, Instanceptr leftq, Instanceptr rightq);
void              setup_regressionnode_properties(Regressionnodeptr node, LEAF_TYPE leaftype);
void              soft_regtree_dimension_reduction(Regressionnodeptr node, char st[READLINELENGTH], int level);
double            test_regtree(Regressionnodeptr root,Instanceptr testingdata);
double            variance_of_linear_regression(matrix w, Instanceptr list, LEAF_TYPE leaftype);
double            variance_of_linear_regression_for_array_of_instances(matrix w, Instanceptr list, int start, int end, LEAF_TYPE leaftype);

#endif
