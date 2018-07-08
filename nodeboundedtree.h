#ifndef __nodeboundedtree_H
#define __nodeboundedtree_H

#include "parameter.h"

int             add_node_to_expanded_list(Decisionnodeptr node, Node_expandedptr* list, int nodeindex, int usediscrete);
void            add_node_to_heap(Decisionnodeptr node, Node_expandedptr* list, int nodeindex);
Decisionnodeptr create_binary_tree_according_model_string(char* model);
int             create_node_bounded_tree(Decisionnodeptr node, Nodeboundedtree_parameterptr param);
void            empty_heap(Node_expandedptr header);
void            prune_node_bounded_tree(Decisionnodeptr root, Nodeboundedtree_parameterptr param, int datasize);
void            prune_node_bounded_tree_nodes(Decisionnodeptr root, Decisionnodeptr toberemoved, Nodeboundedtree_parameterptr param, Decisionnodeptr* nodes, double* generalizationerror, int* index, int datasize);

#endif
