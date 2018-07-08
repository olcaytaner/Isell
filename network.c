#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "distributions.h"
#include "graphdata.h"
#include "messages.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "network.h"
#include "parameter.h"
#include "readnet.h"
#include "statistics.h"
#include "structure.h"

Bndata train, cv, test;
extern int graphcount;
extern bayesiangraphptr *Graphs;

/**
 * Checks if there is a node in the graph with a given name
 * @param[in] mygraph Bayesian graph
 * @param[in] st Name of the node
 * @return If there is a node, the function returns its index. Otherwise the function returns -1.
 */
int node_in(bayesiangraphptr mygraph, char *st)
{
 int i;
 for (i = 0; i < mygraph->nodecount; i++)
   if (strcmp(mygraph->nodes[i].name, st) == 0)
     return i;
 return -1;
}

/**
 * Given the position index of the parents of a node, returns the index values of the parents of that node. For example if a node has 3 parents having 2, 3 and 5 possible values respectively, then the index values of the parents will go from (0, 0, 0), (0, 0, 1) to (1, 2, 3), (1, 2, 4). The position index of (0, 0, 0) will be 0, (0, 0, 1) will be 1), (1, 2, 3) will be 28 and (1, 2, 4) will be 29.
 * @param[in] mynode Bayesian node of a Bayesian graph
 * @param[in] pos Position index
 * @return An array of index values. array[i] is the index value of the i'th parent of the node
 */
int* table_list(struct node mynode, int pos)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 int i, *result;
	result = (int *) safemalloc(mynode.parents * sizeof(int), "table_list", 2);
 for (i = mynode.parents - 1; i > -1; i--)
  {
   result[i] = pos % (mynode.parent_counts[i]);
   pos = pos / (mynode.parent_counts[i]);
  }
 return result;
}

/**
 * This function is the reverse of table_list function. Given the index values of the parents of a node, this function returns the position index of the parents. For example if a node has 3 parents having 2, 3 and 5 possible values respectively, then the index values of the parents will go from (0, 0, 0), (0, 0, 1) to (1, 2, 3), (1, 2, 4). The position index of (0, 0, 0) will be 0, (0, 0, 1) will be 1), (1, 2, 3) will be 28 and (1, 2, 4) will be 29.
 * @param[in] mynode Bayesian node of a Bayesian graph
 * @param[in] list An array of index values. list[i] is the index value of the i'th parent of the node
 * @return Position index
 */
int table_pos(struct node mynode, int *list)
{
 int i, table_index = 0;
 for (i = 0; i < mynode.parents; i++)
   if (i != 0)
     table_index = list[i] + table_index * mynode.parent_counts[i];
   else
     table_index = list[0];
 return table_index;
}

/**
 * Find the parents of a given node
 * @param[in] mygraph Bayesian graph
 * @param[in] node Bayesian graph node
 * @return An array of node indexes containing the parents of the given node. array[i] stores the index of the i'th parent of the given node.
 */
int* find_parents(bayesiangraphptr mygraph, int node)
{
 /*!Last changed 15.01.2001*/
 int i = 0, where, *result = NULL;
 where = mygraph->nodes[node].pointer;
 while (where != -1)
  {
   i++;
   result = alloc(result, i * sizeof(int), i);
   result[i - 1] = mygraph->dest[where];
   where = mygraph->arcs[where].pointer;
  }
 return result;
}

/**
 * Finds the i'th parent of a given node, where i is given as a parameter.
 * @param[in] mygraph Bayesian graph
 * @param[in] node Bayesian graph node
 * @param[in] pindex Index of the parent
 * @return Index of the parent node 
 */
int parent_index(bayesiangraphptr mygraph, int node, int pindex)
{
 int i = 0, where;
 where = mygraph->nodes[node].pointer;
 while (i < pindex)
  {
   i++;
   where = mygraph->arcs[where].pointer;
  }
 return mygraph->dest[where];
}

/**
 * Constructor of a Bayesian node. Adds a new node with the given name to a Bayesian graph
 * @param[in] mygraph Bayesian graph
 * @param[in] st Name of the node
 * @return The node added
 */
nodeptr add_one_node(bayesiangraphptr mygraph, char *st)
{
/*!Last Changed 15.01.2001*/
 nodeptr current;
 mygraph->nodecount++;
 mygraph->nodes = alloc(mygraph->nodes, mygraph->nodecount * sizeof(struct node), mygraph->nodecount);
 current = &(mygraph->nodes[mygraph->nodecount - 1]);
 current->value_count = 0;
 current->parents = 0;
 current->table_count = 1;
 current->pointer = -1;
 current->name = strcopy(current->name, st);
 return current;
}

/**
 * Gets the properties of a given arc.
 * @param[in] mygraph Bayesian graph
 * @param[in] arc_no Index of the arc
 * @param[out] from The outgoing node of the arc
 * @param[out] to The incoming node of the arc
 * @param[out] from_order The order no of the arc in all arcs going to the same node as this arc.
 * @param[out] before The arc before of this arc in all arcs going to the same node as this arc.
 * @return If the arc is found, the function returns arc_no. Otherwise it returns -1.
 */
int find_arc_with_no(bayesiangraphptr mygraph, int arc_no, int *from, int *to, int *from_order, int *before)
{
/*!Last Changed 25.02.2001*/
 int i, pos1;
 for (i = 0; i < mygraph->nodecount; i++)
  {
   pos1 = mygraph->nodes[i].pointer;
   if (pos1 == -1)
     continue;
   *from_order = 0;
   *before = -1;
   if (pos1 == arc_no)
    {
     *to = i;
     *from = mygraph->dest[arc_no];
     return arc_no;
    }
   else
     while (mygraph->arcs[pos1].pointer != -1)
      {
       (*from_order)++;
       *before = pos1;
       if (mygraph->arcs[pos1].pointer == arc_no)
        {
         *from = mygraph->dest[arc_no];
         *to = i;
         return pos1;
        }
       pos1 = mygraph->arcs[pos1].pointer;
      }
  }
 return -1;
}

/**
 * Reverses the direction of a given arc.
 * @param[in] G Bayesian graph
 * @param[in] arc_no Index of the reversed arc
 * @param[out] from The outgoing node of the arc
 * @param[out] to The incoming node of the arc
 * @param[out] from_order The order no of the arc in all arcs going to the same node as this arc.
 */
void modify_one_arc(bayesiangraphptr G, int arc_no, int *from, int *to, int *from_order)
{
 /*!Last Changed 25.02.2001*/
 int before = -1;
 find_arc_with_no(G, arc_no, from, to, from_order, &before);
 if (before == -1)
   G->nodes[*to].pointer = G->arcs[arc_no].pointer;
 else
   G->arcs[before].pointer = G->arcs[arc_no].pointer;
 before = G->nodes[*from].pointer;
 if (before == -1)
   G->nodes[*from].pointer = arc_no;
 else
  {
   while (G->arcs[before].pointer != -1)
     before = G->arcs[before].pointer;
   G->arcs[before].pointer = arc_no;
  }
 G->arcs[arc_no].pointer = -1;
 G->dest[arc_no] = *to;
}

/**
 * Removes a given arc.
 * @param[in] mygraph Bayesian graph from which a given arc will be deleted
 * @param[in] arc_no Index of the deleted arc
 * @param[out] from The outgoing node of the arc
 * @param[out] to The incoming node of the arc
 * @param[out] from_order The order no of the arc in all arcs going to the same node as this arc.
 */
void delete_one_arc(bayesiangraphptr mygraph, int arc_no, int *from, int *to, int *from_order)
{
 /*!Last Changed 07.03.2001*/
 int i, dummy;
/*Once silinecek arcin yeri bulunuyor ve ondan sonra gelen parentlarin yerleri bir one aliniyor*/
 find_arc_with_no(mygraph, arc_no, from, to, from_order, &dummy);
 if (mygraph->nodes[*to].pointer == arc_no)
   mygraph->nodes[*to].pointer = mygraph->arcs[arc_no].pointer;
 else
   mygraph->arcs[dummy].pointer = mygraph->arcs[arc_no].pointer;
/*Silinecek arcdan sonrakileri teker teker kaydir*/
 for (i = arc_no; i < mygraph->arccount - 1; i++)
  {
   mygraph->arcs[i].pointer = mygraph->arcs[i + 1].pointer;
   mygraph->dest[i] = mygraph->dest[i + 1];
  }
/*Silinecek arcdan daha yuksek arc numarasi olanlarin arc numarasini bir azalt*/
 for (i = 0; i < mygraph->nodecount; i++)
   if (mygraph->nodes[i].pointer > arc_no)
     mygraph->nodes[i].pointer--;
 for (i = 0; i < mygraph->arccount; i++)
   if (mygraph->arcs[i].pointer > arc_no)
     mygraph->arcs[i].pointer--;
/*Arc ve desti yeniden realloc et*/
 mygraph->arccount--;
 mygraph->arcs = alloc(mygraph->arcs, mygraph->arccount * sizeof(struct arc), mygraph->arccount);
 mygraph->dest = alloc(mygraph->dest, mygraph->arccount * sizeof(int), mygraph->arccount);
}

/**
 * Adds one arc to a Bayesian graph
 * @param mygraph Bayesian graph
 * @return Address of the new arc
 */
Arcptr add_one_arc(bayesiangraphptr mygraph)
{
/*!Last Changed 25.02.2001*/
 mygraph->arccount++;
 mygraph->arcs = alloc(mygraph->arcs, mygraph->arccount * sizeof(struct arc), mygraph->arccount);
 mygraph->dest = alloc(mygraph->dest, mygraph->arccount * sizeof(int), mygraph->arccount);
 mygraph->arcs[mygraph->arccount - 1].pointer = -1;
 return &mygraph->arcs[mygraph->arccount - 1];
}

/**
 * Arranges the properties of the arc (in this case the from node and the destination node)
 * @param[in] mygraph Bayesian graph
 * @param[in] src Source node of the arc
 * @param[in] dest Index of the destination node of the arc
 */
void add_one_pointer(bayesiangraphptr mygraph, nodeptr src, int dest)
{
 /*!Last Changed 23.12.2000*/
 int where;
 if (src->pointer == -1)
   src->pointer = mygraph->arccount - 1;
 else
  {
   where = src->pointer;
   while (mygraph->arcs[where].pointer != -1)
     where = mygraph->arcs[where].pointer;
   mygraph->arcs[where].pointer = mygraph->arccount - 1;
  }
 mygraph->dest[mygraph->arccount - 1] = dest;
}

void delete_parent_counts(nodeptr Node, int parent_order)
{ /*Delete one parent from the parent_counts array*/
/*!Last Changed 25.02.2001*/
 int i;
 Node->table_count/=Node->parent_counts[parent_order];
 for (i=parent_order;i<Node->parents-1;i++)
   Node->parent_counts[i]=Node->parent_counts[i+1];
 Node->parents--;/*Yer kucultmede alloc yapma*/
 Node->parent_counts = alloc(Node->parent_counts, Node->parents * sizeof(int), Node->parents);
}

void add_parent_counts(bayesiangraphptr mygraph,nodeptr Node,int parent)
{ /*Add one parent in the parent_counts array*/
/*!Last Changed 23.12.2000*/
 Node->parents++;
 Node->parent_counts=alloc(Node->parent_counts,Node->parents*sizeof(int),Node->parents);
 Node->parent_counts[Node->parents-1]=mygraph->nodes[parent].value_count;
 Node->table_count*=mygraph->nodes[parent].value_count;
}

void alloc_conditional_table(nodeptr Node)
{
	/*Allocate conditional table*/
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 23.02.2001*/
 Node->table = (double **)safecalloc_2d(sizeof(double*), sizeof(double), Node->table_count, Node->value_count, "alloc_conditional_table", 2);
}

void add_one_value(nodeptr current,char *st)
{/*Verilmis bir node a bir deger ekliyor*/
/*!Last Changed 15.01.2001*/
 current->value_count++;
 current->values=alloc(current->values,current->value_count*sizeof(char *),current->value_count);
 current->values[current->value_count-1]=strcopy(current->values[current->value_count-1],st);
}

int find_parent_order(bayesiangraphptr G,int parent,int node)
{
/* parentin nodeun kacinci parenti olduguna doner,
   eger parent bulunamazsa -1 e doner */
/*!Last Changed 03.01.2001*/
 int where=G->nodes[node].pointer,order=-1;
 while (where!=-1)
  {
   order++;
   if (G->dest[where]==parent)
     return order;
   where=G->arcs[where].pointer;
  }
 return -1;
}

int find_arc_no(bayesiangraphptr G,int from, int to, int *arc_no, int *before)
{
/* eger belirtilen arc yoksa 0 a varsa 1 e donsun....*/
/* arc_no from-->to arcinin numarasi, before bundan bir oncekinin numarasi*/
/*!Last Changed 03.01.2001*/
 *before = -1;
 if ((to<0)||(to>=G->nodecount))
   return 0;
 if ((from<0)||(from>=G->nodecount))
   return 0;
 *arc_no=G->nodes[to].pointer;
 while ((*arc_no!=-1)&&(G->dest[*arc_no]!=from))
  {
   *before=*arc_no;
   *arc_no=G->arcs[*arc_no].pointer;
  }
 if (*arc_no==-1)
   return 0;
 else
   return 1;
}

bayesiangraphptr create_graph(char *graph_name)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 07.03.2001*/
 bayesiangraphptr result;
 result = (bayesiangraphptr)safemalloc(sizeof(bayesiangraph), "create_graph", 2);
 result->nodecount=0;
 result->arccount=0;
 result->ordering=NULL;
 result->independence=NULL;
 result->one_independence=NULL;
 result->name=strcopy(result->name,graph_name);
 return result;
}

void deallocate_graph(bayesiangraphptr G)
{
 /*!Last Changed 07.03.2001*/
 int i;
 safe_free(G->name);
 for (i = 0; i < G->nodecount; i++)
  {
   safe_free(G->nodes[i].name);
			free_2d((void**)G->nodes[i].values, G->nodes[i].value_count);
   if (G->nodes[i].parents)
     safe_free(G->nodes[i].parent_counts);
			free_2d((void**)G->nodes[i].table, G->nodes[i].table_count);
  }
 if (G->independence!=NULL)
		 free_2d((void**)G->independence, G->nodecount);
 if (G->one_independence!=NULL)
		 free_3d((void***)G->one_independence, G->nodecount, G->nodecount);
 safe_free(G->dest);
 safe_free(G->arcs);
 safe_free(G->nodes);
 safe_free(G);
}

void swap_independence(double **result,int i,int j)
{
/*!Last Changed 11.02.2001*/
 double temp;
 int k;
 for (k=0;k<3;k++)
  {
   temp=result[i][k];
   result[i][k]=result[j][k];
   result[j][k]=temp;
  }
}

void qsort_independence(double **result,int left,int right)
{
/*!Last Changed 11.02.2001*/
 int i,last;
 if (left>=right)
   return;
 swap_independence(result,left,(left+right)/2);
 last=left;
 for (i=left+1;i<=right;i++)
   if (result[i][2]>result[left][2])
    {
     last++;
     swap_independence(result,last,i);
    }
 swap_independence(result,left,last);
 qsort_independence(result,left,last-1);
 qsort_independence(result,last+1,right);
}

double **sort_independence(bayesiangraphptr mygraph)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 21.01.2001*/
 int total=(mygraph->nodecount)*(mygraph->nodecount-1)/2,i,j,k;
 double **result;
 result = (double **)safemalloc_2d(sizeof(double *), sizeof(double), total, 3, "sort_independence", 3);
 k=0;
 for (i=0;i<mygraph->nodecount;i++)
   for (j=i+1;j<mygraph->nodecount;j++)
    {
     result[k][0]=i;
     result[k][1]=j;
     result[k][2]=mygraph->independence[i][j];
     k++;
    }
 qsort_independence(result,0,total-1);
 return result;
}

int cycle_now(bayesiangraphptr mygraph,int from,int to)
{
/*!Last Changed 25.02.2001*/
/*Daha onceden fromdan toya bir arc vard. Bu arc yon degistirince cycle oluyor mu?*/
 int pos;
 pos=mygraph->nodes[to].pointer;
 while (pos!=-1)
  {
   if (from==mygraph->dest[pos])
     return 1;
   if (cycle_now(mygraph,from,mygraph->dest[pos]))
     return 1;
   pos=mygraph->arcs[pos].pointer;
  }
 return 0; 
}

int connected(bayesiangraphptr mygraph)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 25.02.2001*/
 int found=0,i,j,k,l,**connected_components,*component_counts;
 int pos,pos1,pos2,ind1,ind2;
 connected_components = safemalloc (mygraph->nodecount * sizeof(int *), "connected", 3);
 component_counts = safemalloc (mygraph->nodecount * sizeof(int), "connected", 4);
 for (i = 0; i < mygraph->nodecount; i++)
  {
   component_counts[i]=1;
   connected_components[i] = (int *) safemalloc (sizeof(int), "connected", 8);
   connected_components[i][0]=i;
  }
 for (i=0;i<mygraph->nodecount;i++)
  {
   pos=mygraph->nodes[i].pointer;
   while (pos!=-1)
    {
     ind1=i;
     ind2=mygraph->dest[pos];
     pos1=-1;
     pos2=-1;
     j=0;
     l=0;
     while (j<mygraph->nodecount)
      {
       for (k=0;k<component_counts[j];k++)
        {
         if (connected_components[j][k]==ind1)
           pos1=j;
         if (connected_components[j][k]==ind2)
           pos2=j;
        }
       if (component_counts[j]>0)
         l++;
       j++;
      }
     if (l==1)
      {
       found=1;
       break;
      }
     if (pos1!=pos2)
      {
       connected_components[pos1] = alloc(connected_components[pos1],(component_counts[pos1] + component_counts[pos2])*sizeof(int), component_counts[pos1] + component_counts[pos2]);
       for (j=0;j<component_counts[pos2];j++)
         connected_components[pos1][component_counts[pos1]+j] = connected_components[pos2][j];
       component_counts[pos1]+=component_counts[pos2];
       component_counts[pos2]=0;
      }
     pos=mygraph->arcs[pos].pointer;
    }
  }
 if (!found)
  {
   k=0;
   for (i=0;i<mygraph->nodecount;i++)
     if (component_counts[i]>0)
       k++;
   if (k==1)
     found=1;
  }
	free_2d((void**)connected_components, mygraph->nodecount);
 safe_free(component_counts);
 return found;
}

double **find_edges(bayesiangraphptr mygraph,double threshold,int *edgecount)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 13.03.2001*/
 int i,j,k,l,found,total=(mygraph->nodecount)*(mygraph->nodecount-1)/2;
 double **sorted=sort_independence(mygraph),thrhold,**edges=NULL;
 int ind1,ind2,pos1,pos2,**connected_components,*component_counts;
 connected_components = safemalloc (mygraph->nodecount * sizeof(int *), "find_edges", 4);
 component_counts = safemalloc (mygraph->nodecount * sizeof(int), "find_edges", 5);
 *edgecount=0;
 for (i = 0; i < mygraph->nodecount; i++)
  {
   component_counts[i]=1;
   connected_components[i] = (int *) safemalloc (sizeof(int), "find_edges", 10);
   connected_components[i][0]=i;
  }
 for (i=0;i<total;i++)
  {
   j = (int) sorted[i][0];
   k = (int) sorted[i][1];
   found=0;
   for (l=0;l<mygraph->nodecount;l++)/*One independence lara bakiyorum*/
     if ((l!=j)&&(l!=k)&&(mygraph->one_independence[j][k][l]<1-threshold))
      {
       found=1;
       break;
      }
   if (!found)
    {
     (*edgecount)++;
     edges=alloc(edges,(*edgecount)*sizeof(double *),*edgecount);
     edges[(*edgecount)-1] = safemalloc(2*sizeof(double), "find_edges", 28);
     edges[(*edgecount)-1][0]=j;
     edges[(*edgecount)-1][1]=k;
     ind1=j;
     ind2=k;
     pos1=-1;
     pos2=-1;
     j=0;
     l=0;
     while (j<mygraph->nodecount)
      {
       for (k=0;k<component_counts[j];k++)
        {
         if (connected_components[j][k]==ind1)
           pos1=j;
         if (connected_components[j][k]==ind2)
           pos2=j;
        }
       if (component_counts[j]>0)
         l++;
       j++;
      }
     if (l==1)
      {
       thrhold = sorted[i][2];
       break;
      }
     if (pos1!=pos2)
      {
       connected_components[pos1] = alloc(connected_components[pos1], (component_counts[pos1] + component_counts[pos2]) * sizeof(int), component_counts[pos1] + component_counts[pos2]);
       for (j=0;j<component_counts[pos2];j++)
         connected_components[pos1][component_counts[pos1]+j]=connected_components[pos2][j];
       component_counts[pos1]+=component_counts[pos2];
       component_counts[pos2]=0;
      }
    }
  }
 i++;  
 while (i < total && sorted[i][2] == thrhold)
  {
   j = (int) sorted[i][0];
   k = (int) sorted[i][1];
   found = 0;
   for (l = 0; l < mygraph->nodecount; l++)
     if ((l != j) && (l != k) && (mygraph->one_independence[j][k][l] < 1-threshold)){
       found = 1;
       break;
     }
   if (!found)
    {
     (*edgecount)++;
     edges=alloc(edges,(*edgecount)*sizeof(double *),*edgecount);
     edges[(*edgecount)-1] = safemalloc(2*sizeof(double), "find_edges", 80);
     edges[(*edgecount)-1][0]=j;
     edges[(*edgecount)-1][1]=k;
    }
   i++;
  }
	free_2d((void**)connected_components, mygraph->nodecount);
 safe_free(component_counts);
	free_2d((void**)sorted, total);
 return edges;
}

void add_arcs_according_to_ordering(bayesiangraphptr mygraph,double **edges,int edgecount)
{
/*!Last Changed 13.03.2001*/
 int i,j,k;
 for (i = 0; i < edgecount; i++)
  {
   j = (int) edges[i][0];
   k = (int) edges[i][1];
   add_one_arc(mygraph);
   if (mygraph->ordering[j]>mygraph->ordering[k])
    {
     add_one_pointer(mygraph,&mygraph->nodes[j],k);
     add_parent_counts(mygraph,&mygraph->nodes[j],k);
    }
   else
    {
     add_one_pointer(mygraph,&mygraph->nodes[k],j);
     add_parent_counts(mygraph,&mygraph->nodes[k],j);
    }
  }
}

void make_one_conditional_table(bayesiangraphptr mygraph,int node)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 09.03.2001*/
 int i,j,k,value_pos,pos;
 int *list1,*parents;
 double sum;
 alloc_conditional_table(&mygraph->nodes[node]);
 if (mygraph->nodes[node].parents)
  {
   parents=find_parents(mygraph,node);
   list1 = safemalloc(mygraph->nodes[node].parents*sizeof(int), "make_one_conditional_table", 8);
  }
 for (i=0;i<train.row;i++)
  {
   value_pos=train.data[i][node];
   if (mygraph->nodes[node].parents)
    {
     for (k=0;k<mygraph->nodes[node].parents;k++)
       list1[k]=train.data[i][parents[k]];
     pos=table_pos(mygraph->nodes[node],list1);
    }
   else/*No parents*/
     pos=0;
   mygraph->nodes[node].table[pos][value_pos]++;
  }
 if (mygraph->nodes[node].parents)
  {
   safe_free(parents);
   safe_free(list1);
  } 
 for (j=0;j<mygraph->nodes[node].table_count;j++)
  {
   sum=0;
   for (k=0;k<mygraph->nodes[node].value_count;k++)
     sum+=mygraph->nodes[node].table[j][k];
   if (sum!=0)
    {
     for (k=0;k<mygraph->nodes[node].value_count;k++)
       mygraph->nodes[node].table[j][k]/=sum;
    }
   else
     for (k=0;k<mygraph->nodes[node].value_count;k++)
       mygraph->nodes[node].table[j][k] = (double) 1.0 / mygraph->nodes[node].value_count;
  }
}

void make_conditional_tables(bayesiangraphptr mygraph)
{
/*!Last Changed 25.02.2001*/
 int i;
 for (i=0;i<mygraph->nodecount;i++)
   make_one_conditional_table(mygraph,i);
}

double one_independence_test_indexed(bayesiangraphptr mygraph,int node1, int node2, int given)
{
/*!Last Changed 09.03.2001*/
 int flength=train.row, i, j, k;
 double p1, p2, p3;
 int list1[1], list2[2], list3[3];
 int value_list1[1], value_list2[2], value_list3[3];
 double denom;
	double result = 0;
 double temp;
 if (flength == 0)
   return 0;
 list1[0] = given;
 list3[0] = node1;
 list3[1] = node2;
 list3[2] = given;
 for (k = 0; k < mygraph->nodes[given].value_count; k++)
  {
   value_list1[0]=k;
   denom = count_data(list1,value_list1,1) + (double) 0.0;
   if (denom!=0)
     for (i = 0; i < mygraph->nodes[node1].value_count; i++)
      {
       value_list2[0]=i;
       value_list2[1]=k;
       list2[0]=node1;
       list2[1]=given;
       temp = count_data(list2,value_list2,2) + (double) 0.0;
       p2=temp/denom;
       if (p2!=0)
         for (j = 0; j < mygraph->nodes[node2].value_count; j++)
          {
           value_list3[0]=i;
           value_list3[1]=j;
           value_list3[2]=k;
           temp=count_data(list3,value_list3,3) + (double) 0.0;
           p1 = temp/denom;
           value_list2[0]=j;
           list2[0]=node2;
           temp=count_data(list2,value_list2,2) + (double) 0.0;
           p3=temp/denom;
           if ((p1 != 0) && (p3 != 0))
             result += p1 * log2(p1 / (p2 * p3));
          }
      }
  }
 if (result != 0)
   result = 1 - chi_square((mygraph->nodes[node1].value_count - 1) * (mygraph->nodes[node2].value_count - 1) * (mygraph->nodes[given].value_count - 1) , 2 * result * flength);
 return result;
}

void add_network(bayesiangraphptr newnetwork)
{
 graphcount++;
 Graphs=alloc(Graphs,graphcount*sizeof(bayesiangraphptr),graphcount);
 Graphs[graphcount-1]=newnetwork;
}

void load_network(char *filename)
{
 int i = net_extension(filename);
 bayesiangraphptr newnetwork = NULL;
 switch (i)
  {
   case NET_FILE  :newnetwork = (bayesiangraphptr)read_network_net(filename);
                   break;
   case XML_FILE  :newnetwork = (bayesiangraphptr)read_network_xml(filename);
                   break;
   case HUGIN_FILE:newnetwork = (bayesiangraphptr)read_network_hugin(filename);
                   break;
   case BIF_FILE  :newnetwork = (bayesiangraphptr)read_network_bif(filename);
                   break;
 };
 if (newnetwork==NULL)
  {
   printf(m1044);
   return;
  }
 add_network(newnetwork);
}

void print_independences(void (*print)(bayesiangraphptr mygraph), char* networkname)
{
 bayesiangraphptr G;
 if (networkname)
  {
   G = find_network(networkname);
   if (G)
     print(G);
   else
     printf(m1273, networkname);
  }
 else
   printf(m1020);
}

bayesiangraphptr find_network(char *name)
{
 int i;
 for (i=0;i<graphcount;i++)
   if (strcmp(Graphs[i]->name,name)==0)
     return Graphs[i];
 return NULL;
}

void learn_network(char *graph_name, char *train_file, char *cv_file, char *test_file, int algorithm)
{
 bayesiangraphptr newnetwork;
 switch (algorithm){
   case 0:newnetwork = (bayesiangraphptr)simulated_annealing_bn(graph_name, train_file, cv_file, test_file, get_parameter_l("ordering"), get_parameter_f("infthreshold"));
          break;
   case 1:newnetwork = (bayesiangraphptr)k2_bn(graph_name, train_file, cv_file, test_file, get_parameter_l("ordering"), 2);
          break;
 }
 if (newnetwork)
   add_network(newnetwork);
}
