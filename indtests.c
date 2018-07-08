#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "distributions.h"
#include "graphdata.h"
#include "indtests.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "statistics.h"

extern Bndata train;

/**
 * Calculates 0 independence test results for all pairs of nodes. \sum_{i, j} P(i and j) log P(i and j) / (P(i) P(j))
 * @param[in] mygraph Bayesian Graph for which 0 independence test run
 * @return Two dimensional array containing 0 independence test results. array[i][j] is P(i, j).
 */
double **zero_independence_test(bayesiangraphptr mygraph)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 09.03.2001*/
 double time;
	char pst[READLINELENGTH];
 int flength, i, j, k, l, total = (mygraph->nodecount) * (mygraph->nodecount - 1) / 2;
 double p1, p2, p3, walk = total / ((double)80.0), c;
 int list1[2], list2[1], list3[1];
 int value_list1[2], value_list2[1], value_list3[1];
 double **result;
 flength = train.row;
 if (flength == 0)
   return NULL;
 result = (double **)safemalloc_2d(sizeof(double *), sizeof(double), mygraph->nodecount, mygraph->nodecount, "zero_independence_test", 10);
 printf(m1269);
 time = -clock();
 c = 0;
 for (i = 0; i < mygraph->nodecount; i++)
   for (j = i + 1; j < mygraph->nodecount; j++)
    {
     c++;
     while (c >= walk)
      {
       printf(".");
       c -= walk;
      }
     result[i][j] = 0;
     for (k = 0; k < mygraph->nodes[i].value_count; k++)
      {
       list2[0] = i;
       value_list2[0] = k;
       p2 = count_data(list2, value_list2, 1) / (double)(flength + 0.0);
       if (p2 != 0)
         for (l = 0; l < mygraph->nodes[j].value_count; l++)
          {
           list1[0] = i;
           list1[1] = j;
           value_list1[0] = k;
           value_list1[1] = l;
           p1 = count_data(list1, value_list1, 2) / (double)(flength + 0.0);
           list3[0] = j;
           value_list3[0] = l;
           p3 = count_data(list3, value_list3, 1) / (double)(flength + 0.0);
           if ((p1 != 0) && (p3 != 0))
             result[i][j] += p1 * log2(p1 / (p2 * p3));
          }
      }
     if (result[i][j] != 0)
       result[i][j] = 1 - chi_square((mygraph->nodes[i].value_count - 1) * (mygraph->nodes[j].value_count - 1), 2 * result[i][j] * flength);
     result[j][i] = result[i][j];
    }
 time += clock();
	set_precision(pst, "Time for 0 independence calculations:", "\n");
 printf(pst, time / CLOCKS_PER_SEC);
 return result;
}

/**
 * Calculates 1 independence test results for all triples of nodes. Uses one_independence_test_indexed function.
 * @param[in] mygraph Bayesian Graph for which 1 independence test run
 * @return Three dimensional array containing 1 independence test results. array[i][j][k] is P(i, j | k).
 */
double ***one_independence_test(bayesiangraphptr mygraph)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 27.02.2001*/
 double time;
 int i, j, k, total = mygraph->nodecount * (mygraph->nodecount - 1) * (mygraph->nodecount - 2) / 2;
 double ***result, walk = total / 80.0, c;
	char pst[READLINELENGTH];
 printf(m1270);
 time = -clock();
 result = (double***) safemalloc_3d(sizeof(double **), sizeof(double *), sizeof(double), mygraph->nodecount, mygraph->nodecount, mygraph->nodecount, "one_independence_test", 6);
 c = 0;
 for (i = 0; i < mygraph->nodecount; i++)
   for (j = 0; j < mygraph->nodecount; j++)
     for (k = 0; k < mygraph->nodecount; k++)
       if ((i != j) && (i != k) && (j != k))
        {
         if (i < j)
          {
           result[i][j][k] = one_independence_test_indexed(mygraph, i, j, k);
           c++;
          }
         else
           result[i][j][k] = result[j][i][k];
         while (c >= walk)
          {
           printf(".");
           c -= walk;
          }
        }
       else
         result[i][j][k] = 0;
 time += clock();
	set_precision(pst, "Time for 1 independence calculations:", "\n");
 printf(pst, time / CLOCKS_PER_SEC);
 return result;
}
