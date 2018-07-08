#include <stdlib.h>
#include <limits.h>
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "ordering.h"

/**
 * Generates random ordering for the Bayesian graph. For example if there are 4 nodes, the ordering contains numbers 0 to 3 including randomly (1 3 2 0 for example).
 * @param[in] mygraph Bayesian graph for which random ordering will be produced
 */
void random_ordering(bayesiangraphptr mygraph)
{
 /*!Last Changed 23.12.2000*/
 if (mygraph->ordering)
   safe_free(mygraph->ordering);
 mygraph->ordering = random_array(mygraph->nodecount);
}

/**
 * Generates state ordering for the Bayesian graph. In state ordering, the states are ordered according to their value counts (increasing).
 * @param[in] mygraph Bayesian graph for which random ordering will be produced
 */
void state_ordering(bayesiangraphptr mygraph)
{
 /*!Last Changed 30.05.2008 added random_ordering*/
 /*!Last Changed 23.12.2000*/
 int i, j;
 random_ordering(mygraph);
 for (i = 0; i < mygraph->nodecount; i++)
   for (j = 0; j < mygraph->nodecount; j++)
     if ((mygraph->ordering[i] < mygraph->ordering[j]) && (mygraph->nodes[i].value_count > mygraph->nodes[j].value_count))
       swap_int(&(mygraph->ordering[i]), &(mygraph->ordering[j]));
}

/**
 * Generates minimum table ordering for the Bayesian graph. In minimum table ordering, the states are ordered to minimize the conditional table size of the states. To do that: \n
 * 1) Conditional table size's are determined. Since we do not know the direction of the edges, all edges are counted both ways. \n
 * 2) i = last position in the ordering \n
 * 3) Find the node which has smallest conditional table size. \n
 * 4) Put the node found in step 2 to place i of the ordering. \n
 * 5) Drop the connections of the node found in step 2 and update conditional table sizes accordingly. \n
 * 6) Decrement i and if there are nodes remaining go to step 3.
 * @param[in] mygraph Bayesian graph
 * @param[in] edges Edges of the graph
 * @param[in] edgecount Number of edges in the graph
 */
void min_table_ordering(bayesiangraphptr mygraph, double **edges, int edgecount)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 13.03.2001*/
 int i, j, k, orderno, minindex;
 double *table_counts, min;
	table_counts = safemalloc(mygraph->nodecount * sizeof(double), "min_table_ordering", 3);
 if (mygraph->ordering)
   safe_free(mygraph->ordering);
 mygraph->ordering = safemalloc(mygraph->nodecount * sizeof(int), "min_table_ordering", 6);
 /* Step 1 */
 for (i = 0; i < mygraph->nodecount; i++)
   table_counts[i] = mygraph->nodes[i].value_count;
 for (i = 0; i < edgecount; i++)
  {
   j = (int) edges[i][0];
   k = (int) edges[i][1];
   table_counts[j] *= mygraph->nodes[k].value_count;
   table_counts[k] *= mygraph->nodes[j].value_count;
  }
 /* Step 2 */
 orderno = mygraph->nodecount - 1;
 while (orderno >= 0)
  {
   /* Step 3 */
   min = LONG_MAX;
   for (i = 0; i < mygraph->nodecount; i++)
     if (table_counts[i] < min)
      {
       min = table_counts[i];
       minindex = i;
      }
   /* Step 4 */
   mygraph->ordering[minindex] = orderno;
   table_counts[minindex] = LONG_MAX;
   /* Step 5 */
   for (i = 0; i < edgecount; i++)
    {
     j = (int) edges[i][0];
     k = (int) edges[i][1];
     if (j == minindex)
       table_counts[k] /= mygraph->nodes[j].value_count;
     else
       if (k == minindex)
         table_counts[j] /= mygraph->nodes[k].value_count;
    }
   /* Step 6 */
   orderno--;
  }
	safe_free(table_counts);
}

/**
 * Produces ordering based on the Alarm dataset.
 * @param[in] mygraph Bayesian graph
 */
void alarm_ordering(bayesiangraphptr mygraph)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 if (mygraph->ordering)
   safe_free(mygraph->ordering);
 mygraph->ordering = (int *)safemalloc(sizeof(int) * mygraph->nodecount, "alarm_ordering", 3);
 mygraph->ordering[0] = 18;
 mygraph->ordering[1] = 27;
 mygraph->ordering[2] = 3;
 mygraph->ordering[3] = 5;
 mygraph->ordering[4] = 13;
 mygraph->ordering[5] = 12;
 mygraph->ordering[6] = 22;
 mygraph->ordering[7] = 24;
 mygraph->ordering[8] = 16;
 mygraph->ordering[9] = 26;
 mygraph->ordering[10] = 4;
 mygraph->ordering[11] = 6;
 mygraph->ordering[12] = 7;
 mygraph->ordering[13] = 10;
 mygraph->ordering[14] = 23;
 mygraph->ordering[15] = 28;
 mygraph->ordering[16] = 1;
 mygraph->ordering[17] = 2;
 mygraph->ordering[18] = 0;
 mygraph->ordering[19] = 14;
 mygraph->ordering[20] = 21;
 mygraph->ordering[21] = 29;
 mygraph->ordering[22] = 25;
 mygraph->ordering[23] = 30;
 mygraph->ordering[24] = 17;
 mygraph->ordering[25] = 31;
 mygraph->ordering[26] = 19;
 mygraph->ordering[27] = 32;
 mygraph->ordering[28] = 20;
 mygraph->ordering[29] = 15;
 mygraph->ordering[30] = 33;
 mygraph->ordering[31] = 34;
 mygraph->ordering[32] = 35;
 mygraph->ordering[33] = 8;
 mygraph->ordering[34] = 9;
 mygraph->ordering[35] = 11;
 mygraph->ordering[36] = 36;
}


/**
 * Sets the graph ordering manually using the given ordering.
 * @param[in] mygraph Bayesian graph
 * @param[in] given_ordering Manual ordering
 */
void manual_ordering(bayesiangraphptr mygraph, int *given_ordering)
{
 /*!Created 01.06.2001*/
 int i;
 if (mygraph->ordering)
   safe_free(mygraph->ordering);
 for (i = 0; i < mygraph->nodecount; i++)
	  mygraph->ordering[i] = given_ordering[i];
}

/**
 * For a given node, determines the nodes that are before that node in the ordering. 
 * @param[in] mygraph Bayesian graph containing the ordering
 * @param[in] node Given node
 * @return Since there can be many nodes before the given node in the ordering, the function returns an array of integers containing the indexes of the nodes. If there are node nodes before the given node, returns NULL.
 */
int* find_nodes_before(bayesiangraphptr mygraph, int node)
{
 /*!Verilmis bir node icin orderingde ondan once gelen tum nodelari donduruyor*/
 /*!Last changed 01.06.2001*/
 int i = 0, where, *result = NULL;
 where = mygraph->ordering[i];
 while (where != node)
  {
   i++;
   result = alloc(result, i * sizeof(int), i);
   result[i - 1] = mygraph->ordering[i - 1];
   where = mygraph->ordering[i];
  }
 return result;
}

/**
 * Similar to find_nodes_before, counts the number of nodes that comes before a given node in the ordering.
 * @param[in] mygraph Bayesian graph containing the ordering.
 * @param[in] node Given node
 * @return Number of nodes that comes before a given node in the ordering. If there are no nodes returns 0.
 */
int find_nodecount_before(bayesiangraphptr mygraph,int node)
{
 /*!Verilmis bir node icin orderingde ondan once gelen tum nodelarin sayisini donduruyor*/
 /*!Last changed 01.06.2001*/
 int count = 0, where;
 where = mygraph->ordering[count];
 while (where != node)
  {
   count++;
   where = mygraph->ordering[count];
  }
 return count;
}
