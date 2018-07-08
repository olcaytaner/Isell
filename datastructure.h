#ifndef __datastructure_H
#define __datastructure_H

#include "constants.h"
#include "matrix.h"

#define STACKSIZE 1000
#define QUEUESIZE 1000
#define DICTIONARYSIZE 1000
#define HASHTABLESIZE 1009
#define HEAPSIZE 1000
#define MAXV 1000
#define MAXDEGREE 1000

/*!Stack data structure containing integers as elements*/
struct stack{
             /*! Stack array*/
	            int elements[STACKSIZE];
             /*! Top of the stack. -1 if the stack is empty.*/
													int top;
};

typedef struct stack Stack;
typedef Stack* Stackptr;

/*!Link list item data structure void* as items*/
struct listitem{
                /*! Item itself*/
                void* item;
                /*! Pointer to the next item*/
                struct listitem* next;
};

typedef struct listitem Listitem;
typedef Listitem* Listitemptr;

/*!Link list data structure*/
struct linklist{
                /*! Header of the link list*/
                Listitemptr head;
                /*! Tail of the link list*/
                Listitemptr tail;
};

typedef struct linklist Linklist;
typedef Linklist* Linklistptr;

/*!Queue data structure containing void* as elements*/
struct queue{
             /*! body of the queue*/
             void* elements[QUEUESIZE + 1];
             /*! position of the first element*/
             int first;
             /*! position of the last element*/
             int last;
             /*! number of queue elements*/
             int count;
};

typedef struct queue Queue;
typedef Queue* Queueptr;

struct dictionary{
                  /*! body of the dictionary*/
                  void* elements[DICTIONARYSIZE];
                  /*! number of elements in the dictionary*/
                  int count;
};

typedef struct dictionary Dictionary;
typedef Dictionary* Dictionaryptr;

struct hashtable{
                 /*! body of the hash table*/
                 Linklistptr elements[HASHTABLESIZE];
};

typedef struct hashtable Hashtable;
typedef Hashtable* Hashtableptr;

/*!Tree node data structure void* as items*/
struct treenode{
                /*! Item itself*/
                void* item;
                /*! Pointer to the left child*/
                struct treenode* left;
                /*! Pointer to the right child*/
                struct treenode* right;
};

typedef struct treenode Treenode;
typedef Treenode* Treenodeptr;

/*!Binary search tree structure Treenode's as the nodes of the tree*/
struct binarytree{
                  /*! Root node of the binary tree*/
                  Treenodeptr root;
};

typedef struct binarytree Binarytree;
typedef Binarytree* Binarytreeptr;

struct heap{
            /*!Body of the heap*/
            void* elements[HEAPSIZE];
            /*!Number of elements in the heap*/
           int count;
};

typedef struct heap Heap;
typedef Heap* Heapptr;

typedef struct{
                    /*!Neighboring vertex*/
                    int v;
                    /*!Edge weight*/
                    double weight;
}Edge;

typedef Edge* Edgeptr;

typedef struct{
                     /*!Adjacency info*/
                     Edge edges[MAXV][MAXDEGREE];
                     /*!Outdegree of each vertex*/
                     int degree[MAXV];
                     /*!The order of visit of the nodes*/
                     int visitorder[MAXV];
                     /*!The height of each node*/
                     int height[MAXV];
                     /*!If the structure is a tree, this number shows the parent of the node*/
                     int parents[MAXV];
                     /*!Number of vertices in graph*/
                     int nvertices;
                     /*!Number of edges in graph*/
                     int nedges;
}Graph;

typedef Graph* Graphptr;

Binarytreeptr binary_tree();
void          binary_tree_insert(Binarytreeptr b, void* x, int (*compare)(const void *a, const void *b));
void          binary_tree_node_insert(Treenodeptr* t, void* x, int (*compare)(const void *a, const void *b));
void          binary_tree_node_remove(Treenodeptr* t, void* x, int (*compare)(const void *a, const void *b));
void*         binary_tree_node_search(Treenodeptr t, void* x, int (*compare)(const void *a, const void *b));
void          binary_tree_remove(Binarytreeptr b, void* x, int (*compare)(const void *a, const void *b));
void*         binary_tree_search(Binarytreeptr b, void* x, int (*compare)(const void *a, const void *b));
void          compute_indegrees(Graphptr g, int* in);
void*         delete_min(Heapptr h, int (*compare)(const void *a, const void *b));
void*         dequeue(Queueptr q);
Dictionaryptr dictionary();
void          dictionary_delete(Dictionaryptr d, void* x, int (*compare)(const void *a, const void *b));
void          dictionary_insert(Dictionaryptr d, void* x, int (*compare)(const void *a, const void *b));
int           dictionary_search(Dictionaryptr d, void* x, int (*compare)(const void *a, const void *b));
int           eccentricity(Graphptr g, int node);
void          enqueue(Queueptr q, void* x);
Treenodeptr   find_max(Treenodeptr t);
Treenodeptr   find_min(Treenodeptr t);
void          floyds_all_pairs_shortest_path_algorithm(matrix* distancematrix);
void          free_binary_tree(Binarytreeptr b);
void          free_binary_tree_node(Treenodeptr t);
void          free_dictionary(Dictionaryptr d);
void          free_graph(Graphptr g);
void          free_hash_table(Hashtableptr h, void (*free_node_contents) (void* a));
void          free_heap(Heapptr h);
void          free_link_list(Linklistptr l, void (*free_node_contents) (void* a));
void          free_queue(Queueptr q);
void          free_stack(Stackptr s);
Graphptr      graph(int vertexcount);
BOOLEAN       graph_connected(matrix distancematrix);
void          graph_insert_edge(Graphptr g, int x, int y, double weight);
Graphptr      graph_from_distancematrix(matrix distancematrix);
int           hash_function_double(const void *a, int tablesize);
int           hash_function_int(const void *a, int tablesize);
Hashtableptr  hash_table();
void          hash_table_insert(Hashtableptr h, void* x, int (*hash_function)(const void *a, int tablesize));
int           hash_table_remove(Hashtableptr h, void* x, int (*hash_function)(const void *a, int tablesize), int (*compare)(const void *a, const void *b));
void*         hash_table_search(Hashtableptr h, void* x, int (*hash_function)(const void *a, int tablesize), int (*compare)(const void *a, const void *b));
Heapptr       heap();
BOOLEAN       heap_empty(Heapptr h);
BOOLEAN       heap_full(Heapptr h);
void          heap_insert(Heapptr h, void* x, int (*compare)(const void *a, const void *b));
int           is_graph_connected(matrix connectionmatrix);
Linklistptr   link_list();
void*         link_list_get_i(Linklistptr l, int i);
void          link_list_insert(Linklistptr l, void* item);
BOOLEAN       link_list_remove(Linklistptr l, void* item, int (*compare)(const void *a, const void *b));
void*         link_list_search(Linklistptr l, void* item, int (*compare)(const void *a, const void *b));
int           link_list_size(Linklistptr l);
Listitemptr   list_item(void* item);
Graphptr      minimum_spanning_tree_prim(Graphptr g);
void          percolate_down(Heapptr h, int hole, int (*compare)(const void *a, const void *b));
int           pop(Stackptr s);
void          print_graph(Graphptr g);
void          push(Stackptr s, int value);
Queueptr      queue();
BOOLEAN       queue_empty(Queueptr q);
double*       shortest_path(Graphptr g, int start);
void          simple_dfs(matrix adjacencymatrix, BOOLEAN* discovered, int v);
Stackptr      stack();
Treenodeptr   tree_node(void* item);

#endif
