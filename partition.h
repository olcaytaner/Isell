#ifndef __partition_H
#define __partition_H

#include "typedef.h"
#define LEFT 0
#define RIGHT 1

/*! Structure defining the binary partition (division) of classes. Classes are divided into two sets, namely LEFT set and RIGHT set.*/
struct partition{
                 /*! Indices of the classes. classes[i] is the index of class with position i*/
                 int *classes;
                 /*! Assignment of the class in the position i.*/
                 int *assignments;
                 /*! Number of classes in the problem. Size of the classes and assignments array.*/
                 int count;
                };

typedef struct partition Partition;

BOOLEAN   all_same_side(Partition p);
Partition create_copy(Partition p);
int*      find_class_assignments(Partition p);
void      free_partition(Partition p);
Partition get_two_class_partition();
Partition get_two_class_partition_for_one_vs_all(Datasetptr d, int positive);
void      increment_partition(Partition* p);

#endif
