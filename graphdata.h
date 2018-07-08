#ifndef __graphdata_H
#define __graphdata_H

#include "network.h"

double   calculate_K2_metric(bayesiangraphptr mygraph, int node, int *parents, int parentcount);
int      count_data(int *list,int *value_list,int list_count);
int      count_data_from_file(char *file_name,int *list,char **value_list,int list_count);
void     create_data(bayesiangraphptr mygraph,int data_count,char *f_name);
bayesiangraphptr initial_graph_from_data(char *graph_name,char *train_file,char *cv_file,char *test_file);
int      guess_data(Bndata mydata,bayesiangraphptr mygraph);
double   likelihood_of_data(bayesiangraphptr mygraph,Bndata mydata);
double   lnfact(int n);
int      load_data(bayesiangraphptr mygraph, Bndata* mydata, char *f_name);
int**    parent_value_combinations(bayesiangraphptr mygraph, int *parents, int parentcount, int total_combinations);
int      select_node_maximizes_K2_metric(bayesiangraphptr mygraph, int node, int *parents, int parentcount, int *nodeset, int nodecount);

#endif
