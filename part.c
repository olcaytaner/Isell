#include "data.h"
#include "decisiontree.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "nodeboundedtree.h"
#include "part.h"
#include "rule.h"
#include "univariatetree.h"
#include "univariaterule.h"

extern Datasetptr current_dataset;

Decisionnodeptr extract_best_leaf_from_partial_tree(Decisionnodeptr rootnode, int* bestscore)
{
 int i;
 Decisionnodeptr best = NULL, current;
 if (rootnode->featureno != LEAF_NODE)
  {
   if (current_dataset->features[rootnode->featureno].type == STRING)
    {
     for (i = 0; i < current_dataset->features[rootnode->featureno].valuecount; i++)
      {
       current = extract_best_leaf_from_partial_tree(&(rootnode->string[i]), bestscore);
       if (current)
         best = current;
      }
    }
   else
    {
     current = extract_best_leaf_from_partial_tree(rootnode->left, bestscore);
     if (current)
       best = current;
     current = extract_best_leaf_from_partial_tree(rootnode->right, bestscore);
     if (current)
       best = current;
    } 
  }
 else
   if (rootnode->expanded && rootnode->covered > *bestscore)
    {
     *bestscore = rootnode->covered;
     return rootnode;    
    }
 return best;
}

Decisionruleptr extract_rule_from_partial_tree(Decisionnodeptr rootnode)
{
 int bestscore = 0;
 Decisionnodeptr bestleaf;
 bestleaf = extract_best_leaf_from_partial_tree(rootnode, &bestscore);
 return extract_rule_from_leaf(bestleaf);
}

BOOLEAN all_children_expanded_and_are_leaves(Decisionnodeptr rootnode)
{
 int i;
 if (current_dataset->features[rootnode->featureno].type == STRING)
  {
   for (i = 0; i < current_dataset->features[rootnode->featureno].valuecount; i++)
     if (rootnode->string[i].featureno != LEAF_NODE || !rootnode->string[i].expanded)
       return BOOLEAN_FALSE;
  }
 else
  {
   if (rootnode->left->featureno != LEAF_NODE || !rootnode->left->expanded)
     return BOOLEAN_FALSE;
   if (rootnode->right->featureno != LEAF_NODE || !rootnode->right->expanded)
     return BOOLEAN_FALSE;
  }
 return BOOLEAN_TRUE;
}

BOOLEAN subtree_replacement(Decisionnodeptr rootnode, Decisionnodeptr prunenode, Instanceptr validation)
{
 int tmp, performancebefore, performance;
 Decisionnodeptr current;
 current = prunenode;
 while (current && all_children_expanded_and_are_leaves(current))
  {
   performancebefore = test_tree(rootnode, validation);
   tmp = current->featureno;
   current->featureno = LEAF_NODE;
   performance = test_tree(rootnode, validation);
   current->featureno = tmp;
   if (performance < performancebefore)
     return BOOLEAN_FALSE;
   accumulate_instances_tree(&(current->instances), current);
   free_children(current);
   make_leaf_node(current);
   current = current->parent;            
  }
 if (current)
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

void create_partial_tree(Decisionnodeptr rootnode, Instanceptr validation)
{
 int i;
 Node_expandedptr header = NULL, nextexpanded;
 Decisionnodeptr expanded_node;
 if (!add_node_to_expanded_list(rootnode, &header, -2, BOOLEAN_TRUE))
  {
   rootnode->expanded = BOOLEAN_TRUE;
   return;  
  }
 while (header)
  {
   nextexpanded = header->next;
   expanded_node = header->node;
   safe_free(header);
   header = nextexpanded;   
   expanded_node->expanded = BOOLEAN_TRUE;
   expanded_node->featureno = find_best_feature(expanded_node->instances, &(expanded_node->split));
   if (make_children(expanded_node))
    {
     if (current_dataset->features[expanded_node->featureno].type == STRING)
      {
       for (i = 0; i < current_dataset->features[expanded_node->featureno].valuecount; i++)
         if (!add_node_to_expanded_list(&(expanded_node->string[i]), &header, -2, BOOLEAN_TRUE))
          {
           expanded_node->string[i].expanded = BOOLEAN_TRUE;
           if (!subtree_replacement(rootnode, expanded_node, validation))
            {
             empty_heap(header);
             return;            
            }
          }
      }
     else
      {
       if (!add_node_to_expanded_list(expanded_node->left, &header, -2, BOOLEAN_TRUE))
        {
         expanded_node->left->expanded = BOOLEAN_TRUE;
         if (!subtree_replacement(rootnode, expanded_node, validation))
          {
           empty_heap(header);
           return;            
          }
        }
       if (!add_node_to_expanded_list(expanded_node->right, &header, -2, BOOLEAN_TRUE))
        {
         expanded_node->right->expanded = BOOLEAN_TRUE; 
         if (!subtree_replacement(rootnode, expanded_node, validation))
          {
           empty_heap(header);
           return;            
          }
        }
      }     
    }   
  }
}

Ruleset learn_ruleset_part(Instanceptr* instances, Instanceptr* cvdata)
{
 int* counts;
 Instanceptr covered = NULL, cvcovered = NULL;
 Ruleset result;
 Decisionruleptr newrule;
 Decisionnodeptr rootnode;
 empty_ruleset(&result);
 counts = find_class_counts(*instances);
 result.defaultclass = find_max_in_list(counts, current_dataset->classno);
 safe_free(counts);
 while (data_size(*instances) > 0)
  {
   rootnode = createnewnode(NULL, 1);
   rootnode->instances = *instances;
   create_partial_tree(rootnode, *cvdata);
   newrule = extract_rule_from_partial_tree(rootnode);
   add_rule(&result, newrule);
   *instances = NULL;
   accumulate_instances_tree(instances, rootnode);
   deallocate_tree(rootnode);
   safe_free(rootnode);
   remove_covered(instances, &covered, newrule);
   remove_covered(cvdata, &cvcovered, newrule);
  }
 merge_instancelist(instances, covered);
 merge_instancelist(cvdata, cvcovered);
 return result; 
}

