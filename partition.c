#include "libmemory.h"
#include "libmisc.h"
#include "partition.h"

/**
 * Generates a partition where the positive class is on one side, other classes are on the other side (one-vs-all).
 * @param[in] d Dataset
 * @param[in] positive Positive class (single on one side)
 * @return One vs all partition. Positive class is put on the RIGHT side, other classes are put on the LEFT side.
 */
Partition get_two_class_partition_for_one_vs_all(Datasetptr d, int positive)
{
 /*!Last Changed 18.02.2008*/
 Partition result;
 int i;
 result.classes = (int *)safemalloc(d->classno * sizeof(int), "get_two_class_partition_for_one_vs_all", 3);
 for (i = 0; i < d->classno; i++)
   result.classes[i] = i;
 result.count = d->classno;
 result.assignments = (int *)safemalloc(d->classno * sizeof(int), "get_two_class_partition_for_one_vs_all", 7);
 for (i = 0; i < d->classno; i++)
   if (i != positive)
     result.assignments[i] = RIGHT;
   else
     result.assignments[i] = LEFT;
 return result;
}

/**
 * Creates a simple two class partition. One class on the LEFT side, other class on the RIGHT side.
 * @return Two-class partition
 */
Partition get_two_class_partition()
{
	/*!Last Changed 21.06.2005*/
 Partition result;
 result.classes = (int *)safemalloc(2 * sizeof(int), "get_initial_partition", 2);
	result.classes[0] = 0;
	result.classes[1] = 1;
 result.count = 2;
 result.assignments = (int *)safemalloc(2 * sizeof(int), "get_initial_partition", 6);
 result.assignments[0] = LEFT;
 result.assignments[1] = RIGHT;
 return result;
}

/**
 * Checks if all classes are on the same side for the partition p.
 * @param[in] p Partition
 * @return returns TRUE if all classes are on the same side, FALSE otherwise.
 */
BOOLEAN all_same_side(Partition p)
{
/*!Last Changed 16.04.2003*/
	int i, sum = 0;
	for (i = 0; i < p.count; i++)
		 sum += p.assignments[i];
	if ((sum == p.count) || (sum == 0))
		 return BOOLEAN_TRUE;
	return BOOLEAN_FALSE;
}

/**
 * Convert the partition structure to a class assignment array. First finds the class with maximum index. Allocates memory for the class assignment array. Third, set class assignments using the partition structure.
 * @param[in] p Partition
 * @return Array of class assignments. classassignments[i] is the assignment (LEFT or RIGHT) of the class with index i.
 */
int* find_class_assignments(Partition p)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 16.03.2003*/
	int *classassignments, max = -1, i;
 for (i = 0; i < p.count; i++)
   if (p.classes[i] > max)
     max = p.classes[i];
 classassignments = (int *) safemalloc((max + 1) * sizeof(int), "find_class_assignments", 5);
 for (i = 0; i < p.count; i++)
   classassignments[p.classes[i]] = p.assignments[i];
	return classassignments;
}

/**
 * Generates next partition from the given partition.
 * @param[out] p Partition
 */
void increment_partition(Partition* p)
{
/*!Last Changed 04.02.2003*/
	int i = (*p).count - 1;
	while (i >= 0)
	{
		(*p).assignments[i] = 1 - (*p).assignments[i];
		if ((*p).assignments[i] == LEFT)
			 break;
  else
		  i--;
	}
}

/**
 * Creates a new copy of a partition. Allocates memory for the new partition, makes new copies of classes and assignments for the new partition.
 * @param[in] p The partition that will be copied.
 * @return New allocated and produced partition.
 */
Partition create_copy(Partition p)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 26.01.2003*/
 Partition result;
 int i;
 result.count = p.count;
 result.assignments = (int *)safemalloc((p.count)*sizeof(int), "create_copy", 4);
 for (i = 0; i < p.count; i++)
   result.assignments[i] = p.assignments[i];
 result.classes = (int *)safemalloc(p.count*sizeof(int), "create_copy", 7);
 for (i = 0; i<p.count; i++)
   result.classes[i] = p.classes[i];
 return result;
}

/**
 * Desctructor function for the partition structure. Deallocates memory allocated for the partition structure.
 * @param[in] p Partition
 */
void free_partition(Partition p)
{
/*!Last Changed 26.01.2003*/
 safe_free(p.assignments);
 safe_free(p.classes);
}
