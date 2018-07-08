#ifndef __readnet_H
#define __readnet_H
#include "network.h"

bayesiangraphptr read_network_bif(char *f_name);
bayesiangraphptr read_network_hugin(char *f_name);
bayesiangraphptr read_network_net(char *f_name);
bayesiangraphptr read_network_xml(char *f_name);

#endif
