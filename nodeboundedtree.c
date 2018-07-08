#include "decisiontree.h"
#include "data.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "model.h"
#include "multivariatetree.h"
#include "nodeboundedtree.h"
#include "omnivariatetree.h"
#include "srm.h"
#include "univariatetree.h"
#include "vc.h"

extern Datasetptr current_dataset;

void prune_node_bounded_tree_nodes(Decisionnodeptr root, Decisionnodeptr toberemoved, Nodeboundedtree_parameterptr param, Decisionnodeptr* nodes, double* generalizationerror, int* index, int datasize)
{
 /*!Last Changed 30.09.2007*/
 int tmp, h;
 double error;
 if (toberemoved->featureno == LEAF_NODE)
   return;
 (*index)++;
 nodes[*index] = toberemoved;
 tmp = toberemoved->featureno;
 toberemoved->featureno = LEAF_NODE;
 error = decisiontree_falsepositive_count(root) / (datasize + 0.0);
 h = decisiontree_vc_dimension_wrt_nodetype(current_dataset, decisiontree_node_count(root), param->model - 5);
 generalizationerror[*index] = srm_classification(error, h, datasize, param->a1, param->a2);
 toberemoved->featureno = tmp;
 prune_node_bounded_tree_nodes(root, toberemoved->left, param, nodes, generalizationerror, index, datasize);
 prune_node_bounded_tree_nodes(root, toberemoved->right, param, nodes, generalizationerror, index, datasize);
}

void prune_node_bounded_tree(Decisionnodeptr root, Nodeboundedtree_parameterptr param, int datasize)
{
 /*!Last Changed 30.09.2007*/
 int finished = NO, count, h, index, best;
 double *generalizationerror, error;
 Decisionnodeptr *nodes, toberemoved;
 while (!finished)
  {
   count = decisiontree_node_count(root) + 1;
   nodes = safemalloc(count * sizeof(Decisionnodeptr), "prune_node_bounded_tree", 7);
   generalizationerror = safemalloc(count * sizeof(double), "prune_node_bounded_tree", 8);
   error = decisiontree_falsepositive_count(root) / (datasize + 0.0);
   h = decisiontree_vc_dimension_wrt_nodetype(current_dataset, count - 1, param->model - 5);
   generalizationerror[0] = srm_classification(error, h, datasize, param->a1, param->a2);
   index = 0;
   prune_node_bounded_tree_nodes(root, root, param, nodes, generalizationerror, &index, datasize);
   best = find_min_in_list_double(generalizationerror, count);
   finished = YES;
   if (best != 0)
    {
     toberemoved = nodes[best];
     finished = NO;
     if (in_list(toberemoved->featureno, 2, LINEAR, QUADRATIC))
       matrix_free(toberemoved->w);
     accumulate_instances_tree(&(toberemoved->instances), toberemoved);
     toberemoved->featureno = LEAF_NODE;
     free_children(toberemoved);
    }
   safe_free(nodes);
   safe_free(generalizationerror);
  }
}

void add_node_to_heap(Decisionnodeptr node, Node_expandedptr* list, int nodeindex)
{
 /*!Last Changed 11.07.2006*/
 Node_expandedptr newexpanded, tmp, tmpbefore = NULL;
 newexpanded = safemalloc(sizeof(Node_expanded), "add_node_to_expanded_list", 8);
 newexpanded->next = NULL;
 newexpanded->node = node;
 if (nodeindex == -1)
   newexpanded->value = -log_likelihood_for_classification(node->counts);
 else
   if (nodeindex == -2)
     newexpanded->value = -entropy(node->counts);
   else
     newexpanded->value = -nodeindex;
 if (!(*list))
   *list = newexpanded;
 else
  {
   tmp = *list;
   while (tmp && tmp->value > newexpanded->value)
    {
     tmpbefore = tmp;
     tmp = tmp->next;
    }
   if (tmpbefore)
    {
     newexpanded->next = tmpbefore->next;
     tmpbefore->next = newexpanded;
    }
   else
    {
     newexpanded->next = *list;
     *list = newexpanded;
    }
  }
}

int add_node_to_expanded_list(Decisionnodeptr node, Node_expandedptr* list, int nodeindex, int usediscrete)
{
 /*!Last Changed 01.07.2006 added nodeindex*/
 /*!Last Changed 19.03.2006*/
 if (!usediscrete)
   node->conditiontype = HYBRID_CONDITION;
 else
   node->conditiontype = UNIVARIATE_CONDITION;
 if (!can_be_divided(node, NULL))
  {
   make_leaf_node(node);
   return 0;
  }
 add_node_to_heap(node, list, nodeindex);
 return 1;
}

void empty_heap(Node_expandedptr header)
{
 /*!Last Changed 01.07.2006*/
 Node_expandedptr nextexpanded;
 while (header)
  {
   nextexpanded = header->next;
   make_leaf_node(header->node);
   safe_free(header);
   header = nextexpanded;
  }
}

Decisionnodeptr create_binary_tree_according_model_string(char* model)
{
 /*!Last Changed 11.07.2006*/
 Node_expandedptr header = NULL, nextexpanded;
 int nodeindex = 0;
 char ch;
 Decisionnodeptr rootnode, expanded_node, leftnode, rightnode;
 rootnode = createnewnode(NULL, 1);
 add_node_to_heap(rootnode, &header, nodeindex);
 nodeindex++;
 while (header)
  {
   nextexpanded = header->next;
   expanded_node = header->node;
   ch = model[-2 * (int) (header->value) + 1];
   safe_free(header);
   header = nextexpanded;
   expanded_node->featureno = DUMMY_MODEL;
   leftnode = createnewnode(expanded_node, 1);
   expanded_node->left = leftnode;
   rightnode = createnewnode(expanded_node, 1);
   expanded_node->right = rightnode; 
   switch (ch)
    {
     case '1':add_node_to_heap(leftnode, &header, nodeindex);
              nodeindex++;
              break;
     case '2':add_node_to_heap(leftnode, &header, nodeindex);
              nodeindex++;
              add_node_to_heap(rightnode, &header, nodeindex);
              nodeindex++;
              break;
    }
  }
 return rootnode;
}

int create_node_bounded_tree(Decisionnodeptr node, Nodeboundedtree_parameterptr param)
{
 /*!Last Changed 01.10.2006 added second vc calculation type*/
 /*!Last Changed 24.09.2006 discrete type is also added*/
 /*!Last Changed 01.07.2006 added nodeindex with MODEL_SELECTION_FIX*/
 /*!Last Changed 31.05.2006 added a1 and a2 to parameters*/
 /*!Last Changed 24.05.2006 added free_children and continue*/
 /*!Last Changed 19.03.2006*/
 Node_expandedptr header = NULL, nextexpanded;
 BOOLEAN isexpanded;
 int *countsleft, *countsright, expandedcount = 0, numinstances = data_size(node->instances);
 int i, error = 0, h, parenterror, lefterror, righterror, level, len, nodeindex = 0, childrenproduced;
 double bestupperbound, upperbound;
 Decisionnodeptr expanded_node;
 if (param->prunetype == PREPRUNING)
  {
   error = find_error_as_leaf(node->instances);
   bestupperbound = srm_classification((error + 0.0) / numinstances, 1, numinstances, param->a1, param->a2);
  }
 if (param->model != MODEL_SELECTION_FIX)
   add_node_to_expanded_list(node, &header, -1, param->usediscrete);
 else
  {
   add_node_to_expanded_list(node, &header, nodeindex, param->usediscrete);
   nodeindex++;
  }
 while (header && expandedcount < param->nodecount)
  {
   if (param->prunetype == PREPRUNING)     
     parenterror = find_error_as_leaf(header->node->instances);
   nextexpanded = header->next;
   expanded_node = header->node;
   safe_free(header);
   header = nextexpanded;
   switch (param->model)
    {
     case MODEL_SELECTION_UNI:if (param->usediscrete)
                               {
                                expanded_node->featureno = find_best_ldt_feature(expanded_node->instances, &(expanded_node->split));
                                if (expanded_node->featureno == -1)
                                  isexpanded = BOOLEAN_FALSE;
                                else
                                  isexpanded = BOOLEAN_TRUE;
                               }
                              else
                                isexpanded = find_best_hybrid_ldt_split_single_model(expanded_node, UNIVARIATE, param->variance_explained);
                              break;
     case MODEL_SELECTION_LIN:isexpanded = find_best_hybrid_ldt_split_single_model(expanded_node, LINEAR, param->variance_explained);
                              break;
     case MODEL_SELECTION_QUA:isexpanded = find_best_hybrid_ldt_split_single_model(expanded_node, QUADRATIC, param->variance_explained);
                              break;
     case MODEL_SELECTION_LEV:level = level_of_node(expanded_node);
                              len = strlen(param->modelstring);
                              if (level >= len)
                                level = len - 1;
                              find_best_hybrid_ldt_split_according_char(expanded_node, param->modelstring[level], param->variance_explained);
                              break;
     case MODEL_SELECTION_FIX:isexpanded = find_best_hybrid_ldt_split_according_char(expanded_node, param->modelstring[2 * expandedcount], param->variance_explained);
                              break;
     default                 :printf(m1325);
                              isexpanded = BOOLEAN_FALSE;
                              break;
    }
   if (param->usediscrete)
     childrenproduced = make_children(expanded_node);
   else
     childrenproduced = make_multivariate_children(expanded_node);
   if (isexpanded && childrenproduced)
    {     
     if (param->prunetype == PREPRUNING)     
      {
       switch (param->vccalculationtype)
        {
         case VC_GENERAL:h = decisiontree_vc_dimension_wrt_nodetype(current_dataset, expandedcount + 1, param->model - 5);
                         break;
        }
       lefterror = find_error_as_leaf(expanded_node->left->instances);
       righterror = find_error_as_leaf(expanded_node->right->instances);
       error += lefterror + righterror - parenterror;
       upperbound = srm_classification((error + 0.0) / numinstances, h, numinstances, param->a1, param->a2);
       if (upperbound < bestupperbound)
         bestupperbound = upperbound;
       else
        {
         expanded_node->instances = expanded_node->left->instances;
         merge_instancelist(&(expanded_node->instances), expanded_node->right->instances);
         expanded_node->left->instances = NULL;
         expanded_node->right->instances = NULL;
         free_children(expanded_node);
         make_leaf_node(expanded_node);
         continue;
        }
      }
     if (param->model == MODEL_SELECTION_FIX)
      {
       switch (param->modelstring[2 * expandedcount + 1]){
         case '1':countsleft = find_class_counts(expanded_node->left->instances);
                  countsright = find_class_counts(expanded_node->right->instances);
                  if (log_likelihood_for_classification(countsleft) < log_likelihood_for_classification(countsright))
                   {
                    can_be_divided(expanded_node->right, NULL);
                    make_leaf_node(expanded_node->right);
                    if (!add_node_to_expanded_list(expanded_node->left, &header, nodeindex, param->usediscrete))
                     {
                      empty_heap(header);
                      safe_free(countsleft);
                      safe_free(countsright);                      
                      return 0;
                     }
                    nodeindex++;
                   }
                  else
                   {
                    can_be_divided(expanded_node->left, NULL);
                    make_leaf_node(expanded_node->left);
                    if (!add_node_to_expanded_list(expanded_node->right, &header, nodeindex, param->usediscrete))
                     { 
                      empty_heap(header);
                      safe_free(countsleft);
                      safe_free(countsright);                      
                      return 0;
                     }
                    nodeindex++;
                   }
                  safe_free(countsleft);
                  safe_free(countsright);
                  break;
         case '2':if (!add_node_to_expanded_list(expanded_node->left, &header, nodeindex, param->usediscrete))
                   {
                    can_be_divided(expanded_node->right, NULL);
                    make_leaf_node(expanded_node->right);
                    empty_heap(header);
                    return 0;
                   }
                  nodeindex++;
                  if (!add_node_to_expanded_list(expanded_node->right, &header, nodeindex, param->usediscrete))
                   {
                    empty_heap(header);
                    return 0;
                   }
                  nodeindex++;
                  break;
         case '0':can_be_divided(expanded_node->left, NULL);
                  make_leaf_node(expanded_node->left);
                  can_be_divided(expanded_node->right, NULL);
                  make_leaf_node(expanded_node->right);
                  break;
       }
      }
     else
      {
       if (param->usediscrete && current_dataset->features[expanded_node->featureno].type == STRING)
        {
         for (i = 0; i < current_dataset->features[expanded_node->featureno].valuecount; i++)
           add_node_to_expanded_list(&(expanded_node->string[i]), &header, -1, BOOLEAN_TRUE); 
        }
       else
        {
         add_node_to_expanded_list(expanded_node->left, &header, -1, param->usediscrete);
         add_node_to_expanded_list(expanded_node->right, &header, -1, param->usediscrete);
        }
      }
     expandedcount++;      
    }
   else
    {
     make_leaf_node(expanded_node);
     if (param->model == MODEL_SELECTION_FIX)
      {
       empty_heap(header);
       return 0;
      }
    }
  }
 empty_heap(header);
 return 1;
}
