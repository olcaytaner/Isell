#include <limits.h>
#include <math.h>
#include "clustering.h"
#include "data.h"
#include "dataset.h"
#include "instance.h"
#include "instancelist.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"

extern Datasetptr current_dataset;

/**
 * Find the nearest instance to a given instance from an instance list. Function also returns the index of the nearest instance in the instance list.
 * @param[in] means Instance list
 * @param[in] data The instance
 * @param[out] index Index of the nearest instance in the means instance list
 * @return Nearest instance to data from the instance list means
 */
Instanceptr find_nearest_mean(Instanceptr means, Instanceptr data, int* index)
{
	/*!Last Changed 22.03.2005*/
	double mindist, dist;
	Instanceptr temp, minindex = NULL;
	int i;
 mindist = +INT_MAX;
 i = 0;
 temp = means;
 while (temp)
  {
   dist = distance_between_instances(temp, data);
   if (dist < mindist)
    {
     mindist = dist;
     *index = i;
					minindex = temp;
    }
   temp = temp->next;
   i++;
  }
	return minindex;
}

/**
 * Finds the cluster spreads. Cluster spread is defined as the half of the square root of the maximum distance to the cluster center (calculated only for the instances belonging to that cluster). 
 * @param[in] data Data sample
 * @param[in] means Cluster centers
 * @param[in] k Number of clusters
 * @return Cluster spread array. If there are no instances belonging to a cluster returns 1 for that cluster.
 */
double* cluster_spreads(Instanceptr data, Instanceptr means, int k)
{
	/*!Last Changed 20.08.2007*/
	int i, minindex;
	double* result, dist;
	Instanceptr tmp;
	result = safecalloc(k, sizeof(double), "cluster_spreads", 3);
	tmp = data;
	while (tmp)
	 {
		 find_nearest_mean(means, tmp, &minindex);
			dist = distance_between_instances(tmp, data_x(means, minindex));
			if (dist > result[minindex])
				 result[minindex] = dist;
		 tmp = tmp->next;
	 }
	for (i = 0; i < k; i++)
   if (result[i] == 0)
     result[i] = 1.0;
   else
		   result[i] = sqrt(result[i]) / 2;
	return result;
}

/**
 * Does k-means clustering. The detailed algorithm is given in Page 139 Figure 7.3 of the book "Introduction to Machine Learning" (2004) by Ethem Alpaydin
 * @param[in] data Data sample
 * @param[in] k k in k-means clustering
 * @return Link list of k cluster centers.
 */
Instanceptr k_means_clustering(Instanceptr data, int k)
{
	/*!Last Changed 22.03.2005*/
	Instanceptr result, tmp, tmp2, *b, minindex;
	int i, j, size = data_size(data), changed, dummy, *counts, *randomarray;
	result = empty_link_list_of_instances(current_dataset->multifeaturecount, k);
 b = safecalloc(size, sizeof(Instanceptr), "k_means_clustering", 4);
	counts = safemalloc(k * sizeof(int), "k_means_clustering", 5);
	/*Initialization means*/
	randomarray = random_array(size);
	tmp = result;
	i = 0;
	while (tmp)
	 {
		 tmp2 = data_x(data, randomarray[i]);
		 for (j = 0; j < current_dataset->multifeaturecount; j++)
				 if (j != current_dataset->classdefno)
						 tmp->values[j].floatvalue = tmp2->values[j].floatvalue;
			tmp = tmp->next;
			i++;
	 }
	safe_free(randomarray);
	changed = 1;
	while (changed)
	 {
		 changed = 0;
	  for (i = 0; i < k; i++)
		   counts[i] = 0;
			i = 0;
		 tmp = data;
			/*Finding b_i^t s*/
			while (tmp)
			 {
				 minindex = find_nearest_mean(result, tmp, &dummy);
					if (b[i] != minindex)
					 {
						 changed = 1;
					  b[i] = minindex;
					 }
					counts[dummy]++;
				 tmp = tmp->next;
					i++;
			 }
			/*Update m_i */
			if (changed)
			 {
				 /*Initialize m_i*/
				 tmp = result;
					while (tmp)
					 {
    		 for (j = 0; j < current_dataset->multifeaturecount; j++)
			    	 if (j != current_dataset->classdefno)
						     tmp->values[j].floatvalue = 0.0;
						 tmp = tmp->next;
					 }
					/*Sum up datapoints*/
					i = 0;
					tmp = data;
					while (tmp)
					 {
    		 for (j = 0; j < current_dataset->multifeaturecount; j++)
			    	 if (j != current_dataset->classdefno)
						     b[i]->values[j].floatvalue += tmp->values[j].floatvalue;						 
						 tmp = tmp->next;
							i++;
					 }
					/*Take average*/
				 tmp = result;
					i = 0;
					while (tmp)
					 {
						 if (counts[i] > 0)
      		 for (j = 0; j < current_dataset->multifeaturecount; j++)
	  		    	 if (j != current_dataset->classdefno)
		  				     tmp->values[j].floatvalue /= counts[i];
						 tmp = tmp->next;
							i++;
					 }
			 }
	 }
	safe_free(b);
	safe_free(counts);
	return result;
}

double calculate_mean_of_nearest_distance(Instanceptr data)
{
 Instanceptr temp, mininst;
 double S = 0.0;
 int N;
 int index;
 temp = data;
 N = data_size(data);
 while (temp)
  {
  	mininst = find_nearest_mean(data, temp, &index);
	  S = S + sqrt(distance_between_instances(mininst, temp));
 	 temp = temp->next;
  }
 S = S / N;
 return S;
}
