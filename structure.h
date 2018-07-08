#ifndef __structure_H
#define __structure_H

#include "network.h"

enum ordering_type{ 
      ORDERING_RANDOM = 0,
      ORDERING_STATE,
      ORDERING_MINTABLE,
      ORDERING_ALARM};

typedef enum ordering_type ORDERING_TYPE;

bayesiangraphptr k2_bn(char *graph_name, char *train_file, char *cv_file, char *test_file, ORDERING_TYPE ordering, int maxparents);
bayesiangraphptr simulated_annealing_bn(char *graph_name, char *train_file, char *cv_file, char *test_file, ORDERING_TYPE ordering, double infthreshold);

#endif
