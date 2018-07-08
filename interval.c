#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interval.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"

/**
 * Destructor for intervals structure. Deallocates an intervals structure.
 * @param[in] intervals 
 */
void free_intervals(Intervalptr intervals)
{
 /*!Last Changed 09.05.2005*/
 Intervalptr tmp, myptr = intervals;
 if (myptr)
  {
   while (myptr)
    {
     tmp = myptr->next;
     safe_free(myptr);
     myptr = tmp;
    }
  }
}

/**
 * Calculates the length of the interval. Let say the interval is 1-3;4-8;12, the interval length is 3 + 5 + 1 = 9.
 * @param[in] interval The interval
 * @return Length of the interval.
 */
int interval_length(Intervalptr interval)
{
 /*!Last Changed 22.12.2006*/
 int result = 0;
 Intervalptr tmp = interval;
 while (tmp)
  {
   result += tmp->max - tmp->min + 1;
   tmp = tmp->next;
  } 
 return result;
}

/**
 * Checks if the given number is in the interval.
 * @param[in] interval The interval structure
 * @param[in] fno The number
 * @return TRUE if the number is in the interval, FALSE otherwise.
 */
BOOLEAN is_in_interval(Intervalptr interval, int fno)
{
 /*!Last Changed 09.05.2005*/
 Intervalptr tmp;
 tmp = interval;
 while (tmp)
  {
   if (fno >= tmp->min && fno <= tmp->max)
     return BOOLEAN_TRUE;
   tmp = tmp->next;
  }
 return BOOLEAN_FALSE;
}

/**
 * Constructor for the interval structure. Generates interval structure from a string containing the intervals.
 * @param[in] intervalst Input string containing the interval structure. Contains intervals separated via ;'s and each interval is either (i) a number or (ii) two number separated by -. For example 1-3;5 represents the numbers between 1 and 3, and the number 5.
 * @warning The number of intervals may not exceed 1000.
 * @return An interval.
 */
Intervalptr create_intervals(char* intervalst)
{
	/*!Last Changed 23.08.2007 corrected*/
 /*!Last Changed 09.05.2005*/
 Intervalptr result, tmp, current = NULL;
	int i, count;
 char *p, st[READLINELENGTH];
 char *strings[1000];
 if (!strchr(intervalst, ';') && !strchr(intervalst, '-') && !isinteger(intervalst))
   return NULL;
	stringlist(intervalst, ";\n", strings, &count);
 for (i = 0; i < count; i++)
	 {
		 strcpy(st, strings[i]);
   p = strtok(st, "-");
   tmp = safemalloc(sizeof(Interval), "create_intervals", 12);
   tmp->next = NULL;
   tmp->min = atoi(p);
   p = strtok(NULL, "-");
   if (p)
     tmp->max = atoi(p);
   else
     tmp->max = tmp->min;
			if (i != 0)
				 current->next = tmp;
			else
				 result = tmp;
			current = tmp;
			safe_free(strings[i]);
	 }
 return result;
}
