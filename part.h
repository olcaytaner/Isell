#ifndef __part_H
#define __part_H
#include "typedef.h"

BOOLEAN         all_children_expanded_and_are_leaves(Decisionnodeptr rootnode);
void            create_partial_tree(Decisionnodeptr rootnode, Instanceptr validation);
Decisionnodeptr extract_best_leaf_from_partial_tree(Decisionnodeptr rootnode, int* bestscore);
Decisionruleptr extract_rule_from_partial_tree(Decisionnodeptr rootnode);
Ruleset         learn_ruleset_part(Instanceptr* instances, Instanceptr* cvdata);
BOOLEAN         subtree_replacement(Decisionnodeptr rootnode, Decisionnodeptr prunenode, Instanceptr validation);

#endif
