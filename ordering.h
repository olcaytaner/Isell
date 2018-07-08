#ifndef __ordering_H
#define __ordering_H

#include "network.h"

void alarm_ordering(bayesiangraphptr mygraph);
int  find_nodecount_before(bayesiangraphptr mygraph,int node);
int* find_nodes_before(bayesiangraphptr mygraph,int node);
void manual_ordering(bayesiangraphptr mygraph, int *given_ordering);
void min_table_ordering(bayesiangraphptr mygraph,double **edges,int edgecount);
void random_ordering(bayesiangraphptr mygraph);
void state_ordering(bayesiangraphptr mygraph);

#endif
