#include "graphdata.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "operator.h"

extern Bndata cv;

/**
 * Applies removing arc operator to the Bayesian Graph. The arc is selected randomly.
 * @param[in] mygraph Bayesian graph
 * @return TRUE if the operator successfully increases the likelihood of the graph, FALSE otherwise. If there are not enough arcs in the graph, the function also returns FALSE.
 */
BOOLEAN remove_operator(bayesiangraphptr mygraph)
{
/*!Last Changed 02.03.2001*/
 int arc_no, from, to, from_order;
 double before, after;
 if (mygraph->arccount == mygraph->nodecount - 1)
   return BOOLEAN_FALSE;
 arc_no = myrand() % mygraph->arccount;
 before = likelihood_of_data(mygraph, cv);
 delete_one_arc(mygraph, arc_no, &from, &to, &from_order);
 delete_parent_counts(&mygraph->nodes[to], from_order);
 free_2d((void **) mygraph->nodes[to].table, mygraph->nodes[to].table_count);
 make_one_conditional_table(mygraph, to);
 if (connected(mygraph))
  {
   after = likelihood_of_data(mygraph, cv);
   if (after > before)
     return BOOLEAN_TRUE;
  }
 free_2d((void **) mygraph->nodes[to].table, mygraph->nodes[to].table_count);
 add_parent_counts(mygraph, &mygraph->nodes[to], from);
 add_one_arc(mygraph);
 add_one_pointer(mygraph, &mygraph->nodes[to], from);
 make_one_conditional_table(mygraph, to);
 return BOOLEAN_FALSE;
}

/**
 * Applies modifying the direction of the arc operator to the Bayesian Graph. The arc is selected randomly.
 * @param[in] mygraph Bayesian graph.
 * @return TRUE if the operator successfully increases the likelihood of the graph, FALSE otherwise. If modifying the arcs direction creates a cycle, the function also returns FALSE.
 */
BOOLEAN modify_operator(bayesiangraphptr mygraph)
{
/*!Last Changed 02.03.2001*/
 int arc_no, from, to, from_order;
 double before, after;
 arc_no = myrand() % mygraph->arccount;
 before = likelihood_of_data(mygraph, cv);
 modify_one_arc(mygraph, arc_no, &from, &to, &from_order);
 free_2d((void **) mygraph->nodes[to].table, mygraph->nodes[to].table_count);
 free_2d((void **) mygraph->nodes[from].table, mygraph->nodes[from].table_count);
 delete_parent_counts(&mygraph->nodes[to], from_order);
 add_parent_counts(mygraph, &mygraph->nodes[from], to);
 make_one_conditional_table(mygraph, to);
 make_one_conditional_table(mygraph, from);
 if (!cycle_now(mygraph, from, to))
  {
   after = likelihood_of_data(mygraph, cv);
   if (after > before)
     return BOOLEAN_TRUE;
  }
 modify_one_arc(mygraph, arc_no, &from, &to, &from_order);
 free_2d((void **) mygraph->nodes[to].table, mygraph->nodes[to].table_count);
 free_2d((void **) mygraph->nodes[from].table, mygraph->nodes[from].table_count);
 delete_parent_counts(&mygraph->nodes[to], from_order);
 add_parent_counts(mygraph, &mygraph->nodes[from], to);
 make_one_conditional_table(mygraph, to);
 make_one_conditional_table(mygraph, from);
 return BOOLEAN_FALSE;
}
