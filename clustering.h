#ifndef __clustering_H
#define __clustering_H

#include "typedef.h"

double      calculate_mean_of_nearest_distance(Instanceptr data);
double*     cluster_spreads(Instanceptr data, Instanceptr means, int k);
Instanceptr find_nearest_mean(Instanceptr means, Instanceptr data, int* index);
Instanceptr k_means_clustering(Instanceptr data, int k);

#endif
