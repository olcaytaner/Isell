#include <limits.h>
#include "constants.h"
#include "datastructure.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"

/**
 * Checks if the graph is connected from all-to-all shortest path matrix. If one of the distances in the all-to-all shortest path matrix is infinity, that is one node is unreachable from another node, the graph is not connected.
 * @param[in] distancematrix All-to-all shortest path matrix. distancematrix[i][j] is the minimum distance from node i to node j.
 * @pre All-to-all distance matrix must have been calculated before.
 * @return TRUE if the graph is connected, FALSE otherwise.
 */
BOOLEAN graph_connected(matrix distancematrix)
{
 int i, j;
 for (i = 0; i < distancematrix.row; i++)
   for (j = i + 1; j < distancematrix.col; j++)
     if (distancematrix.values[i][j] == +LONG_MAX)
       return BOOLEAN_FALSE;
 return BOOLEAN_TRUE;
}

/**
 * Makes recursive depth first search starting from the node v.
 * @param[in] adjacencymatrix Adjacency matrix of the graph. adjacencymatrix[i][j] is 1, if there is an edge from node i to node j, 0 otherwise.
 * @param[out] discovered Array for checking if there are nodes that are already visited. If discovered[i] is 1, the node is is already visited, 0 otherwise.
 * @param[in] v Starting node index.
 */
void simple_dfs(matrix adjacencymatrix, BOOLEAN* discovered, int v)
{
 /*!Last Changed 04.04.2008*/
 int i;
 discovered[v] = BOOLEAN_TRUE;
 for (i = 0; i < adjacencymatrix.row; i++)
   if (discovered[i] == BOOLEAN_FALSE && adjacencymatrix.values[v][i] == BOOLEAN_TRUE)
     simple_dfs(adjacencymatrix, discovered, i);
}

/**
 * Checks the graph connectivity using recursive depth first search.
 * @param[in] adjacencymatrix Adjacency matrix of the graph. adjacencymatrix[i][j] is 1, if there is an edge from node i to node j, 0 otherwise.
 * @return TRUE if the graph is connected, FALSE otherwise.
 */
int is_graph_connected(matrix adjacencymatrix)
{
 /*!Last Changed 04.04.2008*/
 int i; 
 BOOLEAN* discovered = safemalloc(adjacencymatrix.row * sizeof(BOOLEAN), "is_graph_connected", 1);
 BOOLEAN connected = BOOLEAN_TRUE;
 simple_dfs(adjacencymatrix, discovered, 0);
 for (i = 0; i < adjacencymatrix.row; i++)
   if (!discovered[i])
    {
     connected = BOOLEAN_FALSE;
     break;
    } 
 safe_free(discovered);
 return connected;
}

/**
 * Finds the shortest paths from all nodes to all nodes using Floyd-Warshall's shortest path algorithm.
 * @param[out] distancematrix The adjacency matrix containing pairwise distances between nodes. distancematrix[i][j] represents the length of the edge between node i and node j. If there is no edge between those nodes, distancematrix[i][j] is +LONG_MAX. After this function call, the matrix will now contain pairwise shortest paths from all nodes to all nodes.
 */
void floyds_all_pairs_shortest_path_algorithm(matrix* distancematrix)
{
 int i, j, k, N = distancematrix->row;
 for (k = 0; k < N; k++)
   for (i = 0; i < N; i++)
     for (j = 0; j < N; j++)
       if (distancematrix->values[i][j] > distancematrix->values[i][k] + distancematrix->values[k][j])
         distancematrix->values[i][j] = distancematrix->values[i][k] + distancematrix->values[k][j];
}

/**
 * Constructor for the graph structure. Allocates an empty graph and returns it.
 * @return Graph with no nodes and edges.
 */
Graphptr graph(int vertexcount)
{
 int i;
 Graphptr g;
 g = malloc(sizeof(Graph));
 g->nedges = 0;
 g->nvertices = vertexcount;
 for (i = 0; i < vertexcount; i++)
  {
   g->visitorder[i] = 0;
   g->height[i] = 0;
   g->degree[i] = 0;
  }
 return g;
}

void print_graph(Graphptr g)
{
 int i, j;
 for (i = 0; i < g->nvertices; i++)
   for (j = 0; j < g->degree[i]; j++)
     printf("%d -> %d (%.2f)\n", i, g->edges[i][j].v, g->edges[i][j].weight);
}

void graph_insert_edge(Graphptr g, int x, int y, double weight)
{
 if (g->degree[x] >= MAXDEGREE - 1)
  {
   printf(m1443, x, y);
   return;
  }
 g->edges[x][g->degree[x]].v = y;
 g->edges[x][g->degree[x]].weight = weight;
 g->degree[x]++;
 g->nedges++;
}

void compute_indegrees(Graphptr g, int* in)
{
 int i, j;
 for (i = 0; i < g->nvertices; i++)
   in[i] = 0;
 for (i = 0; i < g->nvertices; i++)
   for (j = 0; j < g->degree[i]; j++)
     in[g->edges[i][j].v]++;
}

Graphptr minimum_spanning_tree_prim(Graphptr g)
{
 int i, v, w;
 int intree[MAXV];
 double distance[MAXV], weight, dist;
 int parent[MAXV];
 Graphptr mst = graph(g->nvertices);
 for (i = 0; i < g->nvertices; i++)
  {
   intree[i] = BOOLEAN_FALSE;
   distance[i] = +LONG_MAX;
   parent[i] = -1;
  }
 distance[0] = 0;
 v = 0;
 while (!intree[v])
  {
   intree[v] = BOOLEAN_TRUE;
   for (i = 0; i < g->degree[v]; i++)
    {
     w = g->edges[v][i].v;
     weight = g->edges[v][i].weight;
     if (distance[w] > weight && !intree[w])
      {
       distance[w] = weight;
       parent[w] = v;
      }
    }
   v = 0;
   dist = +LONG_MAX;
   for (i = 1; i < g->nvertices; i++)
     if (!intree[i] && dist > distance[i])
      {
       dist = distance[i];
       v = i;
      }
  }
 for (i = 1; i < g->nvertices; i++)
   if (parent[i] != -1)
    {
     graph_insert_edge(mst, parent[i], i, 1);
     graph_insert_edge(mst, i, parent[i], 1);
    }
 return mst;
}

double* shortest_path(Graphptr g, int start)
{
 int i, v, w;
 int intree[MAXV];
 double* distance, weight, dist;
 distance = safemalloc(g->nvertices * sizeof(double), "shortest_path", 3);
 for (i = 0; i < g->nvertices; i++)
  {
   intree[i] = BOOLEAN_FALSE;
   distance[i] = +LONG_MAX;
  }
 distance[start] = 0;
 v = start;
 g->parents[v] = -1;
 while (!intree[v])
  {
   intree[v] = BOOLEAN_TRUE;
   for (i = 0; i < g->degree[v]; i++)
    {
     w = g->edges[v][i].v;
     weight = g->edges[v][i].weight;
     if (distance[w] > distance[v] + weight)
      {
       distance[w] = distance[v] + weight;
       g->parents[w] = v;
      }
    }
   v = 0;
   dist = +LONG_MAX;
   for (i = 1; i < g->nvertices; i++)
     if (!intree[i] && dist > distance[i])
      {
       dist = distance[i];
       v = i;
      }
  }
 return distance;
}

int eccentricity(Graphptr g, int node)
{
 int i, eccentricity = 0;
 double* path_lengths = shortest_path(g, node);
 for (i = 0; i < g->nvertices; i++)
   if (path_lengths[i] > eccentricity)
     eccentricity = path_lengths[i];
 safe_free(path_lengths);
 return eccentricity;
}

int* topological_sort(Graphptr g)
{
 int x, y, i, j;
 int *node;
 int* sorted = safemalloc(sizeof(int) * g->nvertices, "topological_sort", 1);
 int indegree[MAXV];
 Queueptr zeroin = queue();
 compute_indegrees(g, indegree);
 for (i = 0; i < g->nvertices; i++)
   if (indegree[i] == 0)
    {
     node = safemalloc(sizeof(int), "topological_sort", 11);
     node[0] = i;
     enqueue(zeroin, node);
    }
 j = 0;
 while (!queue_empty(zeroin))
  {
   node = dequeue(zeroin);
   x = node[0];
   safe_free(node);
   sorted[j] = x;
   j++;
   for (i = 0; i < g->degree[x]; i++)
    {
     y = g->edges[x][i].v;
     indegree[y]--;
     if (indegree[y] == 0)
      {
       node = safemalloc(sizeof(int), "topological_sort", 29);
       node[0] = y;
       enqueue(zeroin, node);
      }
    }
  }
 free_queue(zeroin);
 return sorted;
}

void free_graph(Graphptr g)
{
 safe_free(g);
}

Graphptr graph_from_distancematrix(matrix distancematrix)
{
 int i, j;
 Graphptr g;
 g = graph(distancematrix.row);
 for (i = 0; i < distancematrix.row; i++)
   for (j = 0; j < distancematrix.row; j++)
     if (distancematrix.values[i][j] != -1)
      {
       (g->nedges)++;
       g->edges[i][g->degree[i]].v = j;
       g->edges[i][g->degree[i]].weight = distancematrix.values[i][j];
       (g->degree[i])++;
      }
 return g;
}

/**
 * Constructor for the stack structure. Allocates an empty stack and returns it.
 * @return Stack with no elements.
 */
Stackptr stack()
{
	/*!Last Changed 01.06.2007*/
	Stackptr s;
	s = safemalloc(sizeof(Stack), "stack", 2);
	s->top = -1;
	return s;
}

/**
 * Pushes an integer value to the stack.
 * @param[in] s Stack
 * @param[in] value Element
 */
void push(Stackptr s, int value)
{
	/*!Last Changed 01.06.2007*/
	if (s->top < STACKSIZE - 1)
	 {
  	s->top++;
			s->elements[s->top] = value;
	 }
}

/**
 * Pops an integer value from the stack.
 * @param[in] s Stack
 * @return Popped element.
 */
int pop(Stackptr s)
{
	/*!Last Changed 01.06.2007*/
	int value;
	if (s->top > -1)
	 {
	  value = s->elements[s->top];
	  s->top--;
	  return value;
	 }
	return -1;
}

/**
 * Destructor function of the stack structure. Deallocates memory allocated for the stack.
 * @param[in] s Stack
 */
void free_stack(Stackptr s)
{
	/*!Last Changed 01.06.2007*/
	safe_free(s);
}

/**
 * Constructor function for the link list item structure
 * @param item Item to be itemized
 * @return Initialized list item structure
 */
Listitemptr list_item(void* item)
{
 /*!Last Changed 21.01.2009*/
 Listitemptr l;
 l = safemalloc(sizeof(Listitem), "list_item", 2);
 l->item = item;
 l->next = NULL;
 return l;
}

/**
 * Constructor function for the link list structure.
 * @return Initialized empty link list
 */
Linklistptr link_list()
{
 /*!Last Changed 21.01.2009*/
 Linklistptr l;
 l = safemalloc(sizeof(Linklist), "link_list", 2);
 l->head = NULL;
 l->tail = NULL;
 return l;
}

/**
 * Adds an item to the link list
 * @param l Link list
 * @param item Item to be inserted
 */
void link_list_insert(Linklistptr l, void* item)
{
 /*!Last Changed 21.01.2009*/
 Listitemptr listitem = list_item(item);
 if (l->head == NULL)
   l->head = listitem;
 else
   l->tail->next = listitem;
 l->tail = listitem;
}

/**
 * Removes an item from the link list
 * @param l Link list
 * @param item Item to be deleted
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
BOOLEAN link_list_remove(Linklistptr l, void* item, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 Listitemptr tmp = l->head, tmpbefore = NULL;
 while (tmp && compare(item, tmp->item) != 0)
  {
   tmpbefore = tmp;
   tmp = tmp->next;
  }
 if (tmp != NULL)
  {
   if (tmpbefore == NULL) /*first element is deleted*/
     l->head = l->head->next;
   else
     tmpbefore->next = tmp->next;
   if (l->tail == tmp) /*last element is deleted*/
     l->tail = tmpbefore;
   safe_free(tmp);
   return BOOLEAN_TRUE;
  }
 else
   return BOOLEAN_FALSE;
}

/**
 * Searches an item in a link list
 * @param l Link list
 * @param item Item to be searched
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 * @return The item if found, NULL otherwise
 */
void* link_list_search(Linklistptr l, void* item, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 Listitemptr tmp = l->head;
 while (tmp && compare(item, tmp->item) != 0)
   tmp = tmp->next;
 if (tmp)
   return tmp->item;
 else
   return NULL;
}

/**
 * Number of elements in the link list structure
 * @param l Link list to be searched for its size
 */
int link_list_size(Linklistptr l)
{
 int count = 0;
 Listitemptr tmp = l->head;
 while (tmp != NULL)
 {
  tmp = tmp->next;
  count++;
 }
 return count;
}

/**
 * Get i'th element in the link list structure
 * @param l Link list to be searched for the item
 * @param i index of item
 */
void* link_list_get_i(Linklistptr l, int i)
{
 int count = 0;
 Listitemptr tmp = l->head;
 while (tmp != NULL && count < i)
  {
   tmp = tmp->next;
   count++;
  }
 return tmp->item;
}

/**
 * Destructor function for the link list structure
 * @param l Link list to be freed 
 */
void free_link_list(Linklistptr l, void (*free_node_contents) (void* a))
{
 /*!Last Changed 21.01.2009*/
 Listitemptr tmp = l->head, next = NULL;
 while (tmp != NULL)
  {
   next = tmp->next;
   free_node_contents(tmp->item);
   safe_free(tmp);
   tmp = next;
  }
 safe_free(l);
}

/**
 * Constructor for the queue structure
 * @return Initialized an empty queue
 */
Queueptr queue()
{
 /*!Last Changed 21.01.2009*/
 Queueptr q;
 q = safemalloc(sizeof(Queue), "queue", 2);
 q->first = 0;
 q->last = QUEUESIZE - 1;
 q->count = 0;
 return q; 
}

/**
 * Add an element to the LIFO queue
 * @param q The queue
 * @param x The element to be inserted
 */
void enqueue(Queueptr q, void* x)
{
 /*!Last Changed 21.01.2009*/
 if (q->count >= QUEUESIZE)
   printf("Queue overflow\n");
 else
  {
   q->last = (q->last + 1) % QUEUESIZE;
   q->elements[q->last] = x;
   q->count = q->count + 1;
  }
}

/**
 * Removes an element from the queue
 * @param q The queue
 * @return The element in the beginning of the queue
 */
void* dequeue(Queueptr q)
{
 /*!Last Changed 21.01.2009*/
 void* x = NULL;
 if (q->count <= 0)
   printf("Warning: Empty queue dequeue\n");
 else
  {
   x = q->elements[q->first];
   q->first = (q->first + 1) % QUEUESIZE;
   q->count = q->count - 1;
  }
 return x;
}

/**
 * Checks if the queue is empty
 * @param q The queue
 * @return Returns BOOLEAN_TRUE if the queue is empty, BOOLEAN_FALSE otherwise.
 */
BOOLEAN queue_empty(Queueptr q)
{
 /*!Last Changed 21.01.2009*/
 if (q->count <= 0)
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

/**
 * Destructor function of the queue structure. Deallocates memory allocated for the queue.
 * @param[in] s Queue
 */
void free_queue(Queueptr q)
{
 /*!Last Changed 21.01.2009*/
 safe_free(q);
}

/**
 * Constructor for the dictionary structure. Allocates an empty dictionary and returns it.
 * @return Dictionary with no elements.
 */
Dictionaryptr dictionary()
{
 /*!Last Changed 21.01.2009*/
 Dictionaryptr d;
 d = safemalloc(sizeof(Dictionary), "dictionary", 2);
 d->count = 0;
 return d;
}

/**
 * Inserts an elements to a static sorted dictionary
 * @param d Dictionary
 * @param x The element to be inserted
 * @param compare(const void * a, const void * b) Comparison function to compare two items
 */
void dictionary_insert(Dictionaryptr d, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 int i, j;
 if (d->count == DICTIONARYSIZE)
  {
   printf("Dictionary is full\n");
   return;
  }
 if (d->count == 0)
   d->elements[0] = x;
 else
   if (compare(x, d->elements[d->count - 1]) >= 0)
     d->elements[d->count] = x;
   else
    {
     i = 0;
     while (compare(x, d->elements[i]) < 0)
       i++;
     for (j = d->count - 1; j >= i; j--)
       d->elements[j + 1] = d->elements[j];
     d->elements[i] = x;
    }
 d->count++;
}

/**
 * Deletes an item from a static sorted dictionary
 * @param d The dictionary
 * @param x Item to be deleted
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void dictionary_delete(Dictionaryptr d, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 int i, j;
 for (i = 0; i < d->count; i++)
   if (compare(x, d->elements[i]) == 0)
    {
     for (j = i; j < d->count - 1; j++)
       d->elements[j] = d->elements[j + 1];
     break;
    }
 d->count--;
}

/**
 * Searches an item in a static sorted dictionary
 * @param d The dictionary
 * @param x Item to be search
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 * @return If the item is found, the index of the item. Otherwise the function returns -1.
 */
int dictionary_search(Dictionaryptr d, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 int i;
 for (i = 0; i < d->count; i++)
   if (compare(x, d->elements[i]) == 0)
     return i;
 return -1;
}

/**
 * Destructor for the dictionary structure
 * @param d The dictionary
 */
void free_dictionary(Dictionaryptr d)
{
 /*!Last Changed 21.01.2009*/
 safe_free(d);
}

/**
 * This hash function find the location of an integer object in an hash table.
 * @param a Pointer to the integer object
 * @param tablesize Size of the hash table
 * @return Position of the integer object
 */
int hash_function_int(const void *a, int tablesize)
{
 /*!Last Changed 23.01.2009*/
 int num;
 num = *((int*) a);
 return num % tablesize;
}

/**
 * This hash function find the location of a double object in an hash table.
 * @param a Pointer to the double object
 * @param tablesize Size of the hash table
 * @return Position of the double object
 */
int hash_function_double(const void *a, int tablesize)
{
 /*!Last Changed 23.01.2009*/
 double num;
 num = *((double*) a);
 return ((int) num) % tablesize;
}

/**
 * Constructor for the hash table structure. 
 * @return Initialized empty hash table
 */
Hashtableptr hash_table()
{
 /*!Last Changed 21.01.2009*/
 int i;
 Hashtableptr h;
 h = safemalloc(sizeof(Hashtable), "hash_table", 2);
 for (i = 0; i < HASHTABLESIZE; i++)
   h->elements[i] = link_list();
 return h;
}

/**
 * Inserts an item to an hash table
 * @param h Hash table
 * @param x Item to be inserted
 * @param (* hash_function)(const void * a, int tablesize) Hash function to find the position of an item
 */
void hash_table_insert(Hashtableptr h, void* x, int (*hash_function)(const void *a, int tablesize))
{
 /*!Last Changed 21.01.2009*/
 int location;
 location = hash_function(x, HASHTABLESIZE);
 link_list_insert(h->elements[location], x);
}

/**
 * Removes an item from an hash table
 * @param h Hash table
 * @param x Item to be removed
 * @param (* hash_function)(const void * a, int tablesize) Hash function to find the position of an item
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
int hash_table_remove(Hashtableptr h, void* x, int (*hash_function)(const void *a, int tablesize), int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 int location;
 location = hash_function(x, HASHTABLESIZE);
 return link_list_remove(h->elements[location], x, compare);
}

/**
 * Searches an item in an hash table
 * @param h Hash table
 * @param x Item to be removed
 * @param (* hash_function)(const void * a, int tablesize) Hash function to find the position of an item
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 * @return The item if found, NULL otherwise
 */
void* hash_table_search(Hashtableptr h, void* x, int (*hash_function)(const void *a, int tablesize), int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 21.01.2009*/
 int location;
 location = hash_function(x, HASHTABLESIZE);
 return link_list_search(h->elements[location], x, compare);
}

/**
 * Destructor function for an hash table
 * @param h Hash table
 */
void free_hash_table(Hashtableptr h, void (*free_node_contents) (void* a))
{
 /*!Last Changed 21.01.2009*/
 int i;
 for (i = 0; i < HASHTABLESIZE; i++)
   free_link_list(h->elements[i], free_node_contents);
 safe_free(h);
}

/**
 * Constructor function for a binary tree node
 * @param item Item in the tree node
 * @return Initialized empty tree node
 */
Treenodeptr tree_node(void* item)
{
 /*!Last Changed 22.01.2009*/
 Treenodeptr t;
 t = safemalloc(sizeof(Treenode), "tree_node", 2);
 t->item = item;
 t->left = NULL;
 t->right = NULL;
 return t;
}

/**
 * Constructor function for a binary search tree
 * @return Initialized empty binary search tree
 */
Binarytreeptr binary_tree()
{
 /*!Last Changed 22.01.2009*/
 Binarytreeptr b;
 b = safemalloc(sizeof(Binarytree), "binary_search_tree", 2);
 b->root = NULL;
 return b;
}

/**
 * Searches an item in a binary search tree
 * @param b Binary search tree
 * @param x Item to be searched
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 * @return The item if it is found, NULL otherwise.
 */
void* binary_tree_search(Binarytreeptr b, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 return binary_tree_node_search(b->root, x, compare);
}

/**
 * Internal method to find an item in a subtree
 * @param t The node that roots the tree
 * @param x Item to search for
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 * @return The item if it is found, NULL otherwise.
 */
void* binary_tree_node_search(Treenodeptr t, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 if (t == NULL)
   return NULL;
 else
   if (compare(x, t->item) < 0)
     return binary_tree_node_search(t->left, x, compare);
   else
     if (compare(x, t->item) > 0)
       return binary_tree_node_search(t->right, x, compare);
     else
       return t->item;
}

/**
 * Internal method to find the smallest item in a subtree t
 * @param t The node that roots the tree
 * @return The node containing the smallest item
 */
Treenodeptr find_min(Treenodeptr t)
{
 /*!Last Changed 22.01.2009*/
 if (t == NULL)
   return NULL;
 if (t->left == NULL)
   return t;
 return find_min(t->left);
}

/**
 * Internal method to find the largest item in a subtree t
 * @param t The node that roots the tree
 * @return The node containing the largest item
 */
Treenodeptr find_max(Treenodeptr t)
{
 /*!Last Changed 22.01.2009*/
 if (t != NULL)
   while (t->right != NULL)
     t = t->right;
 return t;
}

/**
 * Inserts an item into a binary search tree
 * @param b Binary search tree
 * @param x Item to be inserted
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void binary_tree_insert(Binarytreeptr b, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 binary_tree_node_insert(&(b->root), x, compare);
}

/**
 * Internal method to insert into a subtree
 * @param t The node that roots the tree
 * @param x The item to insert
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void binary_tree_node_insert(Treenodeptr* t, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 if (*t == NULL)
   *t = tree_node(x);
 else
   if (compare(x, (*t)->item) < 0)
     binary_tree_node_insert(&((*t)->left), x, compare);
   else
     if (compare(x, (*t)->item) > 0)
       binary_tree_node_insert(&((*t)->right), x, compare);
     else
       ;
}

/**
 * Removes an item from a binary search tree
 * @param b Binary tree
 * @param x Item to be deleted
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void binary_tree_remove(Binarytreeptr b, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 binary_tree_node_remove(&(b->root), x, compare);
}

/**
 * Internal method to remove from a subtree
 * @param t The node that roots the tree
 * @param x Item to remove
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void binary_tree_node_remove(Treenodeptr* t, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 Treenodeptr oldnode;
 if (*t == NULL)
   return;
 if (compare(x, (*t)->item) < 0)
   binary_tree_node_remove(&((*t)->left), x, compare);
 else
   if (compare(x, (*t)->item) > 0)
     binary_tree_node_remove(&((*t)->right), x, compare);
   else
     if ((*t)->left != NULL && (*t)->right != NULL)
      {
       (*t)->item = find_min((*t)->right)->item;
       binary_tree_node_remove(&((*t)->right), x, compare);
      }
     else
      {
       oldnode = *t;
       *t = ((*t)->left != NULL) ? (*t)->left : (*t)->right;
       safe_free(oldnode);
      }
}

/**
 * Deallocate the memory allocated for the binary tree
 * @param b Binary tree
 */
void free_binary_tree(Binarytreeptr b)
{
 /*!Last Changed 22.01.2009*/
 free_binary_tree_node(b->root);
 safe_free(b);
}

/**
 * Internal method to deallocate the memory allocatesd for the subtree
 * @param t The node that roots the tree
 */
void free_binary_tree_node(Treenodeptr t)
{
 /*!Last Changed 22.01.2009*/
 if (t != NULL)
  {
   free_binary_tree_node(t->left);
   free_binary_tree_node(t->right);
   safe_free(t);
  }
}

/**
 * Constructor of the heap structure
 * @return Initialized empty heap
 */
Heapptr heap()
{
 /*!Last Changed 22.01.2009*/
 Heapptr h;
 h = safemalloc(sizeof(Heap), "heap", 2);
 h->count = 0;
 return h;
}

/**
 * Deallocate the memory allocated for heap
 * @param h The heap
 */
void free_heap(Heapptr h)
{
 /*!Last Changed 22.01.2009*/
 safe_free(h);
}

/**
 * Checks if the heap is full
 * @param h The heap
 * @return If the heap is full returns TRUE, FALSE otherwise
 */
BOOLEAN heap_full(Heapptr h)
{
 /*!Last Changed 22.01.2009*/
 if (h->count == HEAPSIZE - 1)
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

/**
 * Checks if the heap is empty
 * @param h The heap
 * @return If the heap is empty returns TRUE, FALSE otherwise
 */
BOOLEAN heap_empty(Heapptr h)
{
 /*!Last Changed 22.01.2009*/
 if (h->count == 0)
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

/**
 * Inserts an item to an heap
 * @param h The heap
 * @param x Item to insert
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void heap_insert(Heapptr h, void* x, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 int hole;
 if (heap_full(h))
  {
   printf("Heap is full\n");
   return;
  }
 hole = ++(h->count);
 for (; hole > 1 && compare(x, h->elements[hole / 2]) < 0; hole /= 2)
   h->elements[hole] = h->elements[hole / 2];
 h->elements[hole] = x;
}

/**
 * Removes the smallest item from the priority queue and returns it.
 * @param h The heap
 * @return Smallest item in the heap
 */
void* delete_min(Heapptr h, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 void* item;
 if (heap_empty(h))
  {
   printf("Heap is empty\n");
   return NULL;
  }
 item = h->elements[1];
 h->elements[1] = h->elements[(h->count)--];
 percolate_down(h, 1, compare);
 return item;
}

/**
 * Internal method to percolate down in the heap.
 * @param h The heap
 * @param hole The index at which the percolate begins
 * @param (* compare)(const void * a, const void * b) Comparison function to compare two items
 */
void percolate_down(Heapptr h, int hole, int (*compare)(const void *a, const void *b))
{
 /*!Last Changed 22.01.2009*/
 int child;
 void* tmp = h->elements[hole];
 for (; hole * 2 <= h->count; hole = child)
  {
   child = hole * 2;
   if (child != h->count && compare(h->elements[child + 1], h->elements[child]) < 0)
     child++;
   if (compare(h->elements[child], tmp) < 0)
     h->elements[hole] = h->elements[child];
   else
     break;
  }
 h->elements[hole] = tmp;
}
