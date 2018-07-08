#ifndef __screen_H
#define __screen_H

#include <stdio.h>
#include "network.h"

#define SECONDS_IN_A_DAY 24*60*60
#define SECONDS_IN_A_HOUR 60*60
#define SECONDS_IN_A_MINUTE 60

void display_file(char* file_name);
void graph_info(bayesiangraphptr mygraph);
void node_info(bayesiangraphptr mygraph,char *st);
void print_one_independence(bayesiangraphptr mygraph);
void print_time(double t);
void print_zero_independence(bayesiangraphptr mygraph);

#endif
