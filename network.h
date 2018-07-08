#ifndef __network_H
#define __network_H

#define MAXPARENT 20

/*! Structure for a Bayesian Network node.*/
struct node{
/*!Last Changed 10.03.2001*/
       /*! Name of the Bayesian Network node*/
       char *name;
       /*! Possible values of the node.*/
       char **values;
       /*! Number of possible values of this node. Size of the first dimension of values array*/
       int value_count;
       /*! Number of possible values of the parents of this node. If there are no parents of this node this array is NULL.*/
       int *parent_counts;
       /*! Number of parents of this node. Size of the parent_counts array*/
       int parents;
       /*! Size (Length) of the conditional table. Equals to parent_counts[0] * parentcounts[1] * ... parent_counts[parents-1]*/
       long table_count;
       /*! Pointer to the first parent of this node. If there are no parents, its equal to -1.*/
       int pointer;
       /*! Conditional table of this node. Stored as a two dimensional array. First dimension has size table_count, second dimension has size value_count*/
       double **table;
       /*! If this graph is plotted, x position (axis value) of the center point of this node.*/
       int xpos;
       /*! If this graph is plotted, y position (axis value) of the center point of this node.*/
       int ypos;
       };


/*! Structure for a Bayesian arc. The arcs are stored in the dest array, pointer value of the nodes and pointer value of the arcs.*/
struct arc{
/*!Last Changed 25.02.2001*/
       /*! Index of the next fromnode. */
       int pointer;
       };

typedef struct arc Arc;
typedef struct node* nodeptr;
typedef Arc* Arcptr;

struct BayesianGraph{
/*!Last Changed 04.02.2001*/
       /*!Name of the Bayesian Network*/
       char* name;
       /*!Array storing the nodes of the Bayesian Network*/
       nodeptr nodes;
       /*!Number of nodes in the Bayesian Network. Size of the nodes array*/
       int nodecount;
       /*!Array storing the arcs of the Bayesian Network*/
       Arcptr arcs;
       /*!Number of arcs in the Bayesian Network. Size of the arcs array*/
       int arccount;
       /*!Array storing the incoming nodes*/
       int *dest;
       /*!Zero independence values*/
       double **independence;
       /*!One independence values*/
       double ***one_independence;
       /*!Ordering of this Bayesian Network. If ordering[i]>ordering[j] and there is an edge from i to j, j is the parent of i.*/
       int *ordering;
       };

/*!Structure for the Bayesian Network data*/
struct bndata{
/*!Last Changed 09.03.2001*/
       /*!Data. Stored as two dimensional array. data[i][j] is the value of the feature j (stored as the index corresponding to all possible values) of the instance i.*/
       int **data;
       /*!Number of instances. Size of the first dimension of the data array.*/
       int row;
       /*!Number of features. Size of the second dimension of the data array*/
       int col;
       };

typedef struct BayesianGraph bayesiangraph;
typedef bayesiangraph* bayesiangraphptr;
typedef struct bndata Bndata;

void     add_arcs_according_to_ordering(bayesiangraphptr mygraph,double **edges,int edgecount);
void     add_network(bayesiangraphptr newnetwork);
Arcptr   add_one_arc(bayesiangraphptr mygraph);
nodeptr  add_one_node(bayesiangraphptr mygraph,char *st);
void     add_one_pointer(bayesiangraphptr mygraph,nodeptr src,int dest);
void     add_one_value(nodeptr current,char *st);
void     add_parent_counts(bayesiangraphptr mygraph,nodeptr Node,int parent);
int      connected(bayesiangraphptr mygraph);
bayesiangraphptr create_graph(char *graph_name);
int      cycle_now(bayesiangraphptr mygraph,int from,int to);
void     alloc_conditional_table(nodeptr Node);
void     deallocate_graph(bayesiangraphptr G);
void     delete_one_arc(bayesiangraphptr mygraph,int arc_no,int *from,int *to,int *from_order);
void     delete_parent_counts(nodeptr Node,int parent_order);
int      find_arc_no(bayesiangraphptr G,int from, int to, int *arc_no, int *before);
int      find_arc_with_no(bayesiangraphptr mygraph,int arc_no,int *from,int *to,int *from_order,int *before);
double** find_edges(bayesiangraphptr mygraph,double threshold,int *edgecount);
bayesiangraphptr find_network(char *name);
int      find_parent_order(bayesiangraphptr G,int parent,int node);
int*     find_parents(bayesiangraphptr mygraph,int node);
void     learn_network(char *graph_name,char *train_file,char *cv_file,char *test_file,int algorithm);
void     load_network(char *filename);
void     make_conditional_tables(bayesiangraphptr mygraph);
void     make_one_conditional_table(bayesiangraphptr mygraph,int node);
void     modify_one_arc(bayesiangraphptr G,int arc_no,int *from,int *to,int *from_order);
int      node_in(bayesiangraphptr mygraph,char *st);
double   one_independence_test_indexed(bayesiangraphptr mygraph,int node1, int node2, int given);
int      parent_index(bayesiangraphptr mygraph,int node,int pindex);
void     print_independences(void (*print)(bayesiangraphptr mygraph), char* networkname);
void     qsort_independence(double **result,int left,int right);
double** sort_independence(bayesiangraphptr mygraph);
void     swap_independence(double **result,int i,int j);
int*     table_list(struct node mynode,int pos);
int      table_pos(struct node mynode, int *list);

#endif
