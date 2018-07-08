#ifndef __savenet_H
#define __savenet_H

#include "network.h"

void save_graph_bif(bayesiangraphptr mygraph,char *file_name);
void save_graph_hugin(bayesiangraphptr mygraph,char *file_name);
void save_graph_net(bayesiangraphptr mygraph,char *file_name);
void save_graph_xml(bayesiangraphptr mygraph,char *file_name);

#endif
