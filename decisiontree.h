#ifndef __decisiontree_H
#define __decisiontree_H

#include "typedef.h"
#include "options.h"
#include "partition.h"
#include "parameter.h"

#ifdef MPI
#include "mpi.h"
#endif

#define BITS_FOR_REAL_NUMBER 32
#define DUMMY_MODEL -1000

enum node_type{
      LEAF_NODE = -1,
      UNIVARIATE = 0,
      LINEAR = -2,
      QUADRATIC = -3,
      SVMLIN = -4,
      DISCRETEMIX = -5,
      SOFTLIN = -6};

typedef enum node_type NODE_TYPE;
      
enum condition_type{
      LINEARRULE = LINEAR,
      QUADRATICRULE = QUADRATIC,
      SVMRULELIN = SVMLIN,
      UNIVARIATE_CONDITION = 0,
      MULTIVARIATE_CONDITION,
      HYBRID_CONDITION};

typedef enum condition_type CONDITION_TYPE;
      
void            accumulate_instances_leaf(Instanceptr* data, Decisionnodeptr leaf);
void            accumulate_instances_tree(Instanceptr* data, Decisionnodeptr root);
void            addcondition(Decisionnodeptr node, Decisioncondition condition);
int             can_be_divided(Decisionnodeptr node, C45_parameterptr p);
Decisionnodeptr createnewnode(Decisionnodeptr parentnode, int howmany);
void            deallocate_korderingsplit(Korderingsplit ksplit);
void            deallocate_tree(Decisionnodeptr node);
void            deallocate_tree_rule(Decisionnodeptr node);
int             decisiontree_complexity_count(Decisionnodeptr node);
int             decisiontree_condition_count(Decisionnodeptr node);
int             decisiontree_falsepositive_count(Decisionnodeptr node);
int             decisiontree_leaf_count(Decisionnodeptr node);
double          decisiontree_loglikelihood(Decisionnodeptr root);
int             decisiontree_node_count(Decisionnodeptr node);
int             decisiontree_rule_count(Decisionnodeptr node);
int             depth_of_tree(Decisionnodeptr node);
double          entropy(int* counts);
double          error_of_split(int counts[2][MAXCLASSES], int N);
void            find_occurences_for_discrete_feature(Instanceptr instances, int fno, int ratio[MAXVALUES][MAXCLASSES]);
void            find_subtree_class_counts(Decisionnodeptr subtree, int* counts);
void            free_children(Decisionnodeptr node);
Partition       get_empty_partition(Instanceptr instances);
Partition       get_initial_partition(Instanceptr instances);
Partition       get_start_partition(Instanceptr instances, int leftclass);
double          information_gain(int ratio[2][MAXCLASSES]);
double          information_gain_from_ratios(int ratio[MAXVALUES][MAXCLASSES]);
void            inorder_traversal_decision_tree(Decisionnodeptr node, int* index);
int             instance_value_equal_to_split(Decisionnodeptr currentnode, Instanceptr tmp);
int             is_smaller_discrete_order(Instanceptr inst, Korderingsplit ksplit);
void            leaf_distribution(Decisionnodeptr rootnode, int* counts);
void            level_of_decision_tree(Decisionnodeptr node);
int             level_of_node(Decisionnodeptr node);
double          log_likelihood_for_classification(int classcounts[]);
void            make_leaf_node(Decisionnodeptr node);
Decisionnodeptr next_node(Decisionnodeptr currentnode, Instanceptr tmp);
void            prune_tree(Decisionnodeptr root, Decisionnodeptr toberemoved, Instanceptr cvdata);
BOOLEAN         setup_node_properties(Decisionnodeptr node);
double          soft_output_of_subdecisiontree(Decisionnodeptr subtree, double input1, double input2);
void            subtree_covered_and_false_positives(Decisionnodeptr root, int* C, int* fp);
double          svm_split_value(Instanceptr inst, Svm_split s);
int             test_tree(Decisionnodeptr root,Instanceptr testingdata);

#endif
