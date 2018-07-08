#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "graphdata.h"
#include "libarray.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "messages.h"
#include "typedef.h"

extern Bndata train,cv,test;

/**
 * Reads training data corresponding to a Bayesian network from a file. Deallocates previous data also if possible. The input file has the following format: \n
 * Value of Feature1 Value of Feature2 ... Value of Feature d \n
 * Value of Feature1 Value of Feature2 ... Value of Feature d \n
 * ... \n
 * Value of Feature1 Value of Feature2 ... Value of Feature d
 * @param[in] mygraph Bayesian Network
 * @param[out] mydata Structure to hold the training data to be read
 * @param[in] f_name Input file name
 * @return Number of data points read
 */
int load_data(bayesiangraphptr mygraph, Bndata* mydata, char *f_name)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 07.03.2001*/
 FILE *myfile;
 int i, j, k = file_length(f_name);
 char *st;
 char st1[3];
 char myline[READLINELENGTH];
 if (k == 0)
   return 0;
 if (mydata->data)
   free_2d((void**)(void **)mydata->data, mydata->row);
 mydata->row = k;
 mydata->data = (int**) safemalloc_2d(sizeof(int *), sizeof(int), mydata->row, mydata->col, "load_data", 11);
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = '\0';
 myfile = fopen(f_name, "r");
 if (!myfile)
   return 0;
 fgets(myline, READLINELENGTH, myfile);
 st = strtok(myline, st1);
 i = 0;
 while (st)
  {
   i++;
   st = strtok(NULL,st1);
  }
 mydata->col = i;
 fseek(myfile, 0, SEEK_SET);
 j = 0;
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   i = 0;
   while (st)
    {
     mydata->data[j][i] = listindex(st, mygraph->nodes[i].values, mygraph->nodes[i].value_count);
     i++;
     st = strtok(NULL, st1);
    }
   j++;
  }
 fclose(myfile);
 return k;
}

/**
 * Generates random data from a Bayesian Network. The output file has the following format: \n
 * Value of Feature1 Value of Feature2 ... Value of Feature d \n
 * Value of Feature1 Value of Feature2 ... Value of Feature d \n
 * ... \n
 * Value of Feature1 Value of Feature2 ... Value of Feature d
 * @param[in] mygraph Bayesian Network
 * @param[in] data_count Number of instances to create
 * @param[in] f_name Output file name
 */
void create_data(bayesiangraphptr mygraph, int data_count, char *f_name)
{
 /*!Last Changed 17.07.2008 Added find_bin function*/
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 09.02.2001*/
 FILE *myfile;
 BOOLEAN finished, found;
 int i, j, k, pos;
 double *probability;
 int parent[MAXPARENT];
 int *values;
 if ((myfile = fopen(f_name, "w")) == NULL)
  {
   printf(m1268, f_name);
   return;
  }
 probability = (double *)safemalloc(mygraph->nodecount * sizeof(double), "create_data", 11);
 values = (int *)safemalloc(mygraph->nodecount * sizeof(int), "create_data", 12);
 for (i = 0; i < data_count; i++)
  {
   /*Assign random probabilities to each Bayesian node*/
   for (j = 0; j < mygraph->nodecount; j++)
    {
     probability[j] = (double)(produce_random_value(0, 1));
     values[j] = -1;
    }
   finished = BOOLEAN_FALSE;
   while (!finished)
    {
     finished = BOOLEAN_TRUE;
     for (j = 0; j < mygraph->nodecount; j++)
       if (values[j] == -1)
        {
         found = BOOLEAN_TRUE;
         pos = mygraph->nodes[j].pointer;
         while (pos != -1)
          {
           if ((pos != -1) && (values[mygraph->dest[pos]] == -1))
             found = BOOLEAN_FALSE;
           pos = mygraph->arcs[pos].pointer;
          }
         if (found) /*Eger butun parentlarinin degerleri belirlenmisse*/
          {
           finished = BOOLEAN_FALSE;
           if (mygraph->nodes[j].pointer == -1)/*No parent*/
             values[j] = find_bin(mygraph->nodes[j].table[0], mygraph->nodes[j].value_count, probability[j]);
           else
            { /*Parenti varsa*/
             k = 0;
             pos = mygraph->nodes[j].pointer;
             while (pos != -1)
              {
               parent[k] = values[mygraph->dest[pos]];
               pos = mygraph->arcs[pos].pointer;
               k++;
              }
             k = table_pos(mygraph->nodes[j], parent);
             values[j] = find_bin(mygraph->nodes[j].table[k], mygraph->nodes[j].value_count, probability[j]);
            }
          }
        }
    }
   for (j = 0; j < mygraph->nodecount; j++)
     fprintf(myfile, "%s ", mygraph->nodes[j].values[values[j]]);
   fprintf(myfile, "\n");
  }
 safe_free(values);
 safe_free(probability);
 fclose(myfile);
}

/**
 * Generates an initial graph with no arcs between the nodes. Loads also data for training, testing and validating purposes to train, test, cv variables.
 * @param[in] graph_name Name of the Bayesian network. This name will be used in the subsequent references.
 * @param[in] train_file Name of the file for training purposes
 * @param[in] cv_file Name of the file for validation purposes
 * @param[in] test_file Name of the file for testing purposes
 * @return Allocated, initialized, no arc including graph
 */
bayesiangraphptr initial_graph_from_data(char *graph_name, char *train_file, char *cv_file, char *test_file)
{
/*!Last Changed 09.03.2001*/
 FILE *myfile;
 int i, k = file_length(train_file);
 char *st;
 char tmp[10];
 char st1[3];
 char myline[READLINELENGTH];
 bayesiangraphptr mygraph;
 if (k == 0)
   return NULL;
 mygraph = create_graph(graph_name);
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = '\0';
 myfile = fopen(train_file, "r");
 if (!myfile)
   return NULL;
 fgets(myline, READLINELENGTH, myfile);
 st = strtok(myline, st1);
 i = 0;
 while (st)
  {
   i++;
   sprintf(tmp, "Feature%d", i);
   add_one_node(mygraph, tmp);
   st = strtok(NULL, st1);
  }
 fseek(myfile, 0, SEEK_SET);
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   i = 0;
   while (st)
    {
     k = listindex(st, mygraph->nodes[i].values, mygraph->nodes[i].value_count);
     if (k == -1)
       add_one_value(&mygraph->nodes[i], st);
     i++;
     st = strtok(NULL, st1);
    }
  }
 fclose(myfile);
 load_data(mygraph, &train, train_file);
 load_data(mygraph, &cv, cv_file);
 load_data(mygraph, &test, test_file);
 return mygraph;
}

/**
 * Counts the number of instances in the training set obeying the following condition: \n
 * For i 1 to list_count \n
 * Feature list[i] will have the value value_list[i]
 * @param[in] list The list containing the feature numbers
 * @param[in] value_list The list containing the values of the features (as integers representing indexes) in the list list
 * @param[in] list_count Number of items in the list and value_list
 * @return Number of instances following the condition above
 */
int count_data(int *list, int *value_list, int list_count)
{
 /*!Last Changed 09.03.2001*/
 int i, j, count = 0;
 BOOLEAN found;
 for (i = 0; i < train.row; i++)
  {
   found = BOOLEAN_TRUE;
   for (j = 0; j < list_count; j++)
     if (train.data[i][list[j]] != value_list[j])
      {
       found = BOOLEAN_FALSE;
       break;
      }
   if (found)
     count++;
  }
 return count;
}

/**
 * Counts the number of instances in a file obeying the following condition: \n
 * For i 1 to list_count \n
 * Feature list[i] will have the value value_list[i] in the file file_name
 * @param[in] file_name Input file name
 * @param[in] list The list containing the feature numbers
 * @param[in] value_list The list containing the values of the features (as strings) in the list list
 * @param list_count Number of items in the list and value_list
 * @return Number of instances following the condition above
 */
int count_data_from_file(char *file_name, int *list, char **value_list, int list_count)
{
/*!Last Changed 13.02.2001*/
 FILE *myfile;
 int i, j, count = 0;
 BOOLEAN found;
 char *st;
 char st1[3];
 char myline[READLINELENGTH];
 if ((myfile = fopen(file_name, "r")) == NULL)
   return 0;
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = '\0';
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   found = BOOLEAN_TRUE;
   i = 0;
   while (st)
    {
     for (j = 0; j < list_count; j++)
       if (list[j] == i)
         if (strcmp(value_list[j], st) != 0)
          {
           found = BOOLEAN_FALSE;
           break;
          }
     if (!found)
       break;
     i++;
     st = strtok(NULL, st1);
    }
   if (found)
     count++;
  }
 fclose(myfile);
 return count;
}

/**
 * Tries to guess the value of each feature given the values of other features using the given Bayesian Graph. This is done for all test data.
 * @param[in] mydata Test data
 * @param[in] mygraph Bayesian Graph
 * @return Total number of features correctly guessed.
 */
int guess_data(Bndata mydata, bayesiangraphptr mygraph)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 07.03.2001*/
 int i, j, k, pos, value_pos, *parents, *list1, result = 0, maxindex;
 double max;
 for (i = 0; i < mydata.row; i++)
   for (j = 0; j < mydata.col; j++)
    {
     value_pos = mydata.data[i][j];
     if (mygraph->nodes[j].parents)
      {
       parents = find_parents(mygraph, j);
       list1 = safemalloc(mygraph->nodes[j].parents * sizeof(int), "guess_data", 10);
       for (k = 0; k < mygraph->nodes[j].parents; k++)
         list1[k] = mydata.data[i][parents[k]];
       pos = table_pos(mygraph->nodes[j], list1);
       safe_free(parents);
       safe_free(list1);
      }
     else /*No parents*/
       pos = 0;
     max = 0;
     maxindex = -1;
     for (k = 0; k < mygraph->nodes[j].parents; k++)
       if (mygraph->nodes[j].table[pos][k] > max)
        {
         max = mygraph->nodes[j].table[pos][k];
         maxindex = k;
        }
     if (maxindex == value_pos)
       result++;   
    }
 return result;   
}

/**
 * Calculates the log-likelihood of the input data according to the Bayesian Graph. Since multiplication of small numbers will seize to zero, log-likelihood is calculated.
 * @param[in] mygraph Bayesian Graph
 * @param[in] mydata Input data
 * @return Log-likelihood of the input data
 */
double likelihood_of_data(bayesiangraphptr mygraph, Bndata mydata)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 25.02.2001*/
 int i, j, k, pos, value_pos, *parents, *list1;
 double result = 0;
 for (i = 0; i < mydata.row; i++)
   for (j = 0; j < mydata.col; j++)
    {
     value_pos = mydata.data[i][j];
     if (mygraph->nodes[j].parents)
      {
       parents = find_parents(mygraph,j);
       list1 = safemalloc(mygraph->nodes[j].parents * sizeof(int), "likelihood_of_data", 10);
       for (k = 0; k < mygraph->nodes[j].parents; k++)
         list1[k] = mydata.data[i][parents[k]];
       pos = table_pos(mygraph->nodes[j], list1);
       safe_free(parents);
       safe_free(list1);
      }
     else/*No parents*/
       pos = 0;
     if (mygraph->nodes[j].table[pos][value_pos] > 0)
       result += log2(mygraph->nodes[j].table[pos][value_pos]);
    }
 return result;
}

/**
 * Generates all possible value combinations for the parents of a Bayesian node. For example, if a node has three parents with 2, 4 and 3 values respectively, then the result will contain 2 * 4 * 3 = 24 possible combinations starting from (0, 0, 0), (0, 0, 1), ... to (1, 3, 1), (1, 3, 2).
 * @param[in] mygraph Bayesian Graph
 * @param[in] parents Parent indexes of the Bayesian node
 * @param[in] parentcount Number of parents of the Bayesian node (number of items in the parents list).
 * @param[in] total_combinations Total number of combinations. Although this can be calculated in this function, to fasten the operations, the function takes this value as an argument.
 * @return All possible value combinations for the parents of a Bayesian node stored as a two-dimensional array. First dimension is the index of the combination, second index is the values of that combination.
 */
int **parent_value_combinations(bayesiangraphptr mygraph, int *parents, int parentcount, int total_combinations)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 int i, j;
 int repeat, value, num_values, current_value;
 int **combinations;
 combinations = (int **) safemalloc (total_combinations * sizeof(int *), "parent_value_combinations", 4);
 for (i = 0; i < total_combinations; i++)
   combinations[i] = (int *) safemalloc (parentcount * sizeof(int), "parent_value_combinations", 6);
 num_values = 1;
 for (i = 0; i < parentcount; i++)
  {
   current_value = mygraph->nodes[parents[i]].value_count;
   num_values = num_values * current_value;
   repeat = (int) total_combinations / num_values;
   value = 0;
   for (j = 0; j < total_combinations; j++)
    {
     if (repeat > 0)
      {
       combinations[j][i] = value;
       repeat = repeat - 1;
      }
     else
      {
       repeat = (int) total_combinations / num_values;
       value = (value + 1) % current_value;
       combinations[j][i] = value;
       repeat = repeat -1;
      }
    }
  }
 return combinations;
}

/**
 * Calculates log(n!).
 * @param[in] n n
 * @return log(n!).
 */
double lnfact(int n)
{
 int i;
 double result;
 result = 0;
 for (i = n; i > 0; i--)
   result = result + log(i);
 return result;
}

/**
 * Calculates K2 metric for a Bayesian graph node in a Bayesian graph.
 * @param[in] mygraph Bayesian graph
 * @param[in] node Bayesian node
 * @param[in] parents Parent indexes of the Bayesian graph node
 * @param[in] parentcount Number of parents of the Bayesian graph node (number of items in the parents list).
 * @return K2 metric value for the Bayesian graph node
 */
double calculate_K2_metric(bayesiangraphptr mygraph, int node, int *parents, int parentcount)
{
 /*!Last Changed 02.02.2004*/
 int i, j, k;
 int ri, qi, xijk, Nij;
 double lnsum, result;
 int *list, *value;
 int **combinations;
 ri = mygraph->nodes[node].value_count;
 qi = 1;/*qi is the number of different instantiations of the parents of node*/
 for (i = 0; i < parentcount; i++)
   qi = qi * mygraph->nodes[parents[i]].value_count;
 list = (int *) safemalloc ((parentcount + 1) * sizeof(int), "calculate_K2_metric", 10);	/*sizes are for node+parents*/
 value = (int *) safemalloc ((parentcount + 1) * sizeof(int), "calculate_K2_metric", 11);
 combinations = parent_value_combinations(mygraph, parents, parentcount, qi);
 list[0] = node;
 for (i = 0; i < parentcount; i++)
   list[i + 1] = parents[i];
 result = 0;
 for (j = 0; j < qi; j++)
  {
   Nij = 0;
   lnsum = 0;
   for (k = 0; k < ri; k++)
    {
     value[0] = k;
     for (i = 0; i < parentcount; i++)
       value[i + 1] = combinations[j][i];
     xijk = count_data(list, value, parentcount + 1);
     Nij = Nij + xijk;
     lnsum = lnsum + lnfact(xijk);
    }
   result = result + (lnfact(ri - 1) - lnfact(Nij + ri - 1) + lnsum);
  }
 return result;
}

/**
 * Finds the node within the candidate parent nodes that maximizes the K2 metric. This node will be added to the parent node list of the given Bayesian graph node.
 * @param[in] mygraph Bayesian graph
 * @param[in] node Bayesian graph node
 * @param[in] parents Current parent list of the Bayesian graph node
 * @param[in] parentcount Current number of parent nodes of the Bayesian graph node (number of items in the parents list).
 * @param[in] nodeset Candidate parent nodes list of the Bayesian graph node
 * @param[in] nodecount Number of candidates parent nodes (number of items in the nodeset list)
 * @return The index of node which maximizes the K2 metric
 */
int select_node_maximizes_K2_metric(bayesiangraphptr mygraph, int node, int *parents, int parentcount, int *nodeset, int nodecount)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 double metricvalue, maxvalue;
 int i, j, maxnode;
 int *currentparents;
 BOOLEAN found;
 currentparents = (int *) safemalloc((parentcount + 1) * sizeof(int), "select_node_maximizes_K2_metric", 4);
 maxvalue = -LONG_MAX;
 maxnode = -1;
 for (i = 0; i < parentcount; i++)
   currentparents[i] = parents[i];
 for (i = 0; i < nodecount; i++)
  {
   found = BOOLEAN_FALSE;
   for (j = 0; j < parentcount; j++)
     if (nodeset[i] == parents[j])
      {
       found = BOOLEAN_TRUE;
       break;
      }
   currentparents[parentcount] = nodeset[i];
   if (!found)
    {
     metricvalue = calculate_K2_metric(mygraph, node, currentparents, parentcount + 1);
     if (metricvalue > maxvalue)
      {
       maxvalue = metricvalue;
       maxnode = nodeset[i];
      }
    }
  }
	safe_free(currentparents);
 return maxnode;
}
