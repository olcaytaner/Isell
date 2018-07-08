#include <stdio.h>
#include <stdlib.h>
#include "graphdata.h"
#include "indtests.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "messages.h"
#include "operator.h"
#include "ordering.h"
#include "structure.h"

/**
 * Learn Bayesian graph from training data using simulated annealing algorithm. \n
 * Algorithm: \n
 * 1. Create initial graph using zero and one independence tests. \n
 * 2. Put the arc directions according to the ordering \n
 * 3. Apply remove arc and modify arc operators. \n
 * 4. Stop if none of the operators are successful for 5 times.
 * @param[in] graph_name Name of the Bayesian graph. To call some function about this graph, one must use this name.
 * @param[in] train_file Name of the file containing the train set.
 * @param[in] cv_file Name of the file containing the validation set.
 * @param[in] test_file Name of the file containing the test set.
 * @param[in] ordering Index of the ordering type. ORDERING_RANDOM for random produced orderings, ORDERING_STATE orders the state according to the value counts, ORDERING_MINTABLE orders the nodes so that, for each node the size of the conditional table is minimum, ORDERING_ALARM is the ordering used for Alarm dataset.
 * @param[in] infthreshold Threshold used for setting the edges.
 * @return Bayesian graph
 */
bayesiangraphptr simulated_annealing_bn(char *graph_name, char *train_file, char *cv_file, char *test_file, ORDERING_TYPE ordering, double infthreshold)
{/*Last Changed 09.03.2001*/
 int i = 0, j, edgecount;
 bayesiangraphptr newnetwork;
 double **edges;
 newnetwork = (bayesiangraphptr) initial_graph_from_data(graph_name, train_file, cv_file, test_file);
 if (!newnetwork)
   return NULL;
 newnetwork->independence = zero_independence_test(newnetwork);
 newnetwork->one_independence = one_independence_test(newnetwork);
 edges = find_edges(newnetwork, infthreshold, &edgecount);
 switch (ordering)
  {
   case ORDERING_RANDOM  :random_ordering(newnetwork);
                          break;
   case ORDERING_STATE   :state_ordering(newnetwork);
                          break;
   case ORDERING_MINTABLE:min_table_ordering(newnetwork, edges, edgecount);
                          break;
   case ORDERING_ALARM   :alarm_ordering(newnetwork);
                          break;
   default               :printf(m1240, ordering);
                          break;
  }
 add_arcs_according_to_ordering(newnetwork, edges, edgecount);
 make_conditional_tables(newnetwork);
 while (i < 5)
  {
   j = myrand() % 2;
   if (j)
     if (!remove_operator(newnetwork))
       i++;
     else
       i = 0;
   else
     if (!modify_operator(newnetwork))
       i++;
     else
       i = 0;
  }
 return newnetwork;
}

/**
 * Learn Bayesian graph from training data using K2 algorithm. \n
 * Algorithm: \n
 * 1. Initialize Bayesian graph using the training data. \n
 * 2. For each node of the Bayesian graph, select the node that maximizes the K2 metric to add as a parent. \n
 * 3. Repeat step 2 until there is no more improvement. \n
 * 4. Generate conditional tables.
 * @param[in] graph_name Name of the Bayesian graph. To call some function about this graph, one must use this name.
 * @param[in] train_file Name of the file containing the train set.
 * @param[in] cv_file Name of the file containing the validation set.
 * @param[in] test_file Name of the file containing the test set.
 * @param[in] ordering Index of the ordering type. ORDERING_RANDOM for random produced orderings, ORDERING_STATE orders the state according to the value counts, ORDERING_MINTABLE orders the nodes so that, for each node the size of the conditional table is minimum, ORDERING_ALARM is the ordering used for Alarm dataset.
 * @param[in] maxparents Maximum number of parents that is allowed.
 * @return Bayesian graph
 */
bayesiangraphptr k2_bn(char *graph_name, char *train_file, char *cv_file, char *test_file, ORDERING_TYPE ordering, int maxparents)
{/*Last Changed 04.06.2001*/
 int i, j, nodecount;
 BOOLEAN proceed;
 bayesiangraphptr newnetwork;
 int *current_parents, *modified_parents;
 int selected_node;
 double p_old, p_new;
 int *nodeset;
 newnetwork = initial_graph_from_data(graph_name, train_file, cv_file, test_file);
 if (!newnetwork)
   return NULL;
 switch (ordering)
  {
   case ORDERING_RANDOM  :random_ordering(newnetwork);
                          break;
   case ORDERING_STATE   :state_ordering(newnetwork);
                          break;
   case ORDERING_MINTABLE:break;
   case ORDERING_ALARM   :alarm_ordering(newnetwork);
		                        break;
   default               :printf(m1240, ordering);
                          break;
  }
 for (j = 0; j < newnetwork->nodecount; j++)
  {
   i = newnetwork->ordering[j];
   proceed = BOOLEAN_TRUE;
   while ((proceed) && ((newnetwork->nodes[i].parents) < maxparents))
    {
     current_parents = find_parents(newnetwork, i);
     p_old = calculate_K2_metric(newnetwork, i, current_parents, newnetwork->nodes[i].parents);
     modified_parents = find_parents(newnetwork, i);
     nodeset = find_nodes_before(newnetwork, i);
     nodecount = find_nodecount_before(newnetwork, i);
     selected_node = select_node_maximizes_K2_metric(newnetwork, i, current_parents, newnetwork->nodes[i].parents, nodeset, nodecount);
     if (selected_node == -1)
       proceed = BOOLEAN_FALSE;
     else
      {
       modified_parents = alloc(modified_parents, ((newnetwork->nodes[i].parents + 1) * sizeof(int)), newnetwork->nodes[i].parents + 1);
       modified_parents[newnetwork->nodes[i].parents] = selected_node;
       p_new = calculate_K2_metric(newnetwork, i, modified_parents, newnetwork->nodes[i].parents + 1);
       if (p_new > p_old)
        {
         p_old = p_new;
         add_one_arc(newnetwork);
         add_one_pointer(newnetwork,&newnetwork->nodes[i],selected_node);
         add_parent_counts(newnetwork,&newnetwork->nodes[i],selected_node);
        }
       else
         proceed = BOOLEAN_FALSE;
      }
					safe_free(current_parents);
					safe_free(modified_parents);
					safe_free(nodeset);
		  }
   printf(m1317, i);
  }
 make_conditional_tables(newnetwork);
 return newnetwork;
}
