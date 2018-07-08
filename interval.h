#ifndef __interval_H
#define __interval_H

#include "constants.h"

#define RANDOM_MAX 0x7FFFFFFFUL

/*! Node for the interval link list data structure. An interval starts from an integer and ends with an integer, where both of them may be the same number.*/
struct interval{
       /*! Lower bound of the interval*/
       int min;
       /*! Upper bound of the interval*/
       int max;
       /*! Pointer to the next interval structure node*/
       struct interval* next;
       };

typedef struct interval Interval;
typedef Interval* Intervalptr;

Intervalptr create_intervals(char* intervalst);
void        free_intervals(Intervalptr intervals);
int         interval_length(Intervalptr interval);
BOOLEAN     is_in_interval(Intervalptr interval, int fno);

#endif
