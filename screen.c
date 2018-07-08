#include <stdlib.h>
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "screen.h"
#include "typedef.h"

extern FILE* output;

/**
 * Displays the information about the graph to the screen (output file). The information includes name of the graph, number of nodes, number of arcs, for each node the nodes those are connected etc.
 * @param[in] mygraph Graph whose information will be printed
 */
void graph_info(bayesiangraphptr mygraph)
{
 int i, j, pos;
 fprintf(output, "Graph Name:%s\n", mygraph->name);
 fprintf(output, "Node Count:%d\n", mygraph->nodecount);
 fprintf(output, "Arc Count:%d\n", mygraph->arccount);
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(output, "Node %d: %s\n", i + 1, mygraph->nodes[i].name);
   fprintf(output, "Parents:");
   pos = mygraph->nodes[i].pointer;
   while (pos != -1)
    {
     fprintf(output, "%s ", mygraph->nodes[mygraph->dest[pos]].name);
     pos = mygraph->arcs[pos].pointer;
    }
   fprintf(output, "\n");
   fprintf(output, "Values:");
   for (j = 0; j < mygraph->nodes[i].value_count; j++)
     fprintf(output, "%s ", mygraph->nodes[i].values[j]);
   fprintf(output, "\n");
  }
}

/**
 * Displays the node information, names of the node, values that this node can have, conditional table for this node.
 * @param[in] mygraph Graph
 * @param[in] st node name
 */
void node_info(bayesiangraphptr mygraph,char *st)
{
 int *list, *parents, j, k, i = node_in(mygraph, st);
 if (i == -1)
  {
   fprintf(output, "Node %s does not exist\n", st);
   return;
  }
 parents = find_parents(mygraph, i);
 fprintf(output, "Node Name:%s\n", mygraph->nodes[i].name);
 fprintf(output, "Values: ");
 for (k = 0; k < mygraph->nodes[i].value_count; k++)
   fprintf(output, "%s ", mygraph->nodes[i].values[k]);
 fprintf(output, "\n---------------Conditional Table--------------\n");
 for (k = 0; k < mygraph->nodes[i].table_count; k++)
  {
   list = table_list(mygraph->nodes[i], k);
   for (j = 0; j < mygraph->nodes[i].parents; j++)
     fprintf(output, "%8s ", mygraph->nodes[parents[j]].values[list[j]]);
   for (j = 0; j < mygraph->nodes[i].value_count; j++)
     fprintf(output, "%.4f ", mygraph->nodes[i].table[k][j]);
   fprintf(output, "\n");
   safe_free(list);
  }
 safe_free(parents);
}

/**
 * Displays zero independences for this graph. Node i is zero independent from node j: P(i, j)
 * @param[in] mygraph Graph
 */
void print_zero_independence(bayesiangraphptr mygraph)
{
/*Last Changed 04.02.2001*/
 int i, j;
 for (i = 0; i < mygraph->nodecount; i++)
  {
   for (j = 0; j < mygraph->nodecount; j++)
     fprintf(output, "%8.4f", mygraph->independence[i][j]);
   fprintf(output, "\n");
  }
}

/**
 * Displays one independences for this graph. Node i is one independent from node j given node k: P(i, j | k) 
 * @param[in] mygraph Graph
 */
void print_one_independence(bayesiangraphptr mygraph)
{
/*Last Changed 04.02.2001*/
 int i, j, k, l = 0;
 for (i = 0; i < mygraph->nodecount; i++)
   for (j = 0; j < mygraph->nodecount; j++)
     for (k = 0; k < mygraph->nodecount; k++)
       if ((i != j) && (i != k) && (j != k))
        {
         fprintf(output, "%d %d %d %8.4f\n", i, j, k, mygraph->one_independence[i][j][k]);
         l++;
         if (l % 23 == 0)
          {
           fprintf(output, "Press any key to continue\n");
           getc(stdin);
          }
        }
}

/**
 * Prints a file to the screen.
 * @param[in] file_name Name of the file
 */
void display_file(char* file_name)
{
 FILE* infile;
 char line[READLINELENGTH];
 infile = fopen(file_name, "r");
 if (!infile)
  {
   printf(m1052);
   return;
  }
 while (fgets(line, READLINELENGTH, infile))
   printf("%s", line);
}

/**
 * Prints time formatted 3 days 4 hours 12 min 24 secs.
 * @param[in] t time
 */
void print_time(double t)
{
 int tmp;
 if (t > SECONDS_IN_A_DAY)
  {
   tmp = (int)(t / SECONDS_IN_A_DAY);
   printf(m1310, tmp);
   t = t - tmp * SECONDS_IN_A_DAY;
  }
 if (t > SECONDS_IN_A_HOUR)
  {
   tmp = (int)(t / SECONDS_IN_A_HOUR);
   printf(m1311, tmp);
   t = t - tmp * SECONDS_IN_A_HOUR;
  }
 if (t > SECONDS_IN_A_MINUTE)
  {
   tmp = (int)(t / SECONDS_IN_A_MINUTE);
   printf(m1312, tmp);
   t = t - tmp * SECONDS_IN_A_MINUTE;
  }
 printf(m1313, t);
}
