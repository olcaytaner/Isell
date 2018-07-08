#include "libiteration.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"

BOOLEAN bypass_iterator(int* iterator, int size, int range, int bypassno)
{
 /*!Last Changed 21.02.2008*/
 int i, j;
 for (i = bypassno; i >= 0; i--)
  {
   if (iterator[i] < range)
    {
     iterator[i]++;
     for (j = i + 1; j < size; j++)
       iterator[j] = 0;
     return BOOLEAN_TRUE;
    }
   iterator[i] = 0;
  }
 return BOOLEAN_FALSE;
}

BOOLEAN bypass_iterator_from_list(int* iterator, int* list, int* indexlist, int size, int range, int bypassno)
{
 /*!Last Changed 21.02.2008*/
 int i, j;
 for (i = bypassno; i >= 0; i--)
  {
   if (indexlist[i] < range)
    {
     indexlist[i]++;
     iterator[i] = list[indexlist[i]];
     for (j = i + 1; j < size; j++)
      {
       indexlist[j] = 0;
       iterator[j] = list[indexlist[j]];
      }
     return BOOLEAN_TRUE;
    }
   indexlist[i] = 0;
   iterator[i] = list[indexlist[i]];
  }
 return BOOLEAN_FALSE;
}

int* first_combination(int size)
{
 /*!Last Changed 21.02.2008*/
 int i;
 int* result = safemalloc(size * sizeof(int), "first_combination", 2);
 for (i = 0; i < size; i++)
   result[i] = i;
 return result;
}

int* first_combination_from_list(int size, int* list, int** indexlist)
{
 /*!Last Changed 21.02.2008*/
 int i;
 int* result = safemalloc(size * sizeof(int), "first_combination_from_list", 2);
 *indexlist = safemalloc(size * sizeof(int), "first_combination_from_list", 3);
 for (i = 0; i < size; i++)
  {
   (*indexlist)[i] = i;
   result[i] = list[(*indexlist)[i]];
  }
 return result;
}

int* first_iterator(int size)
{
 /*!Last Changed 21.02.2008*/
 int i;
 int* result = safemalloc(size * sizeof(int), "first_iterator", 2);
 for (i = 0; i < size; i++)
   result[i] = 0;
 return result;
}

int* first_mixed_iterator(int size)
{
 /*!Last Changed 14.11.2009*/
 int i;
 int* result = safemalloc(size * sizeof(int), "first_mixed_iterator", 2);
 for (i = 0; i < size; i++)
   result[i] = 0;
 return result;
}

int* first_iterator_from_list(int size, int* list, int** indexlist)
{
 /*!Last Changed 21.02.2008*/
 int i;
 int* result = safemalloc(size * sizeof(int), "first_combination", 2);
 *indexlist = safemalloc(size * sizeof(int), "first_combination", 3);
 for (i = 0; i < size; i++)
  {
   (*indexlist)[i] = 0;
   result[i] = list[(*indexlist)[i]];
  }
 return result;
}

int* first_permutation(int size)
{
 /*!Last Changed 21.02.2008*/
 int i;
 int* result = safemalloc(size * sizeof(int), "first_combination", 2);
 for (i = 0; i < size; i++)
   result[i] = i;
 return result;
}

int** generate_permutations(int howmany, int*total)
{
 /*!Last Changed 27.04.2004*/
 int **result, i, j, k, index;
 if (howmany == 1)
  {
   result = safemalloc(sizeof(int *), "generate_permutations", 4);
   result[0] = safemalloc(sizeof(int), "generate_permutations", 5);
   result[0][0] = 0;
   *total = 1;
  }
 else
  {
   result = generate_permutations(howmany - 1, total);
   result = alloc(result, (*total) * howmany * sizeof(int *), howmany);
   for (i = 0; i < *total; i++)
     for (j = 1; j < howmany; j++)
      {
       index = (*total)*j + i; 
       result[index] = safemalloc(howmany * sizeof(int), "generate_permutations", 17);
       for (k = 0; k < j; k++)
         result[index][k] = result[i][k];
       for (k = j + 1; k < howmany; k++)
         result[index][k] = result[i][k - 1];
       result[index][j] = howmany - 1;
      }
   for (i = 0; i < *total; i++)
    {
     result[i] = alloc(result[i], howmany * sizeof(int), howmany);
     for (j = howmany - 1; j > 0; j--)
       result[i][j] = result[i][j - 1];
     result[i][0] = howmany - 1;
    }
   *total *= howmany;
  }
 return result;
}

BOOLEAN is_subset(int* set1, int* set2, int size1, int size2)
{
 int i, j;
 BOOLEAN found;
 for (i = 0; i < size1; i++)
  {
   found = BOOLEAN_FALSE;
   for (j = 0; j < size2; j++)
     if (set1[i] == set2[j])
      {
       found = BOOLEAN_TRUE;
       break;
      }
   if (!found)
     return BOOLEAN_FALSE;
  }
 return BOOLEAN_TRUE;
}

int** generate_subsets(int howmany, int *total, int** counts)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 07.05.2001*/
 int i, j, k, **sets, res = 1;
 for (i = 0;i < howmany;i++)
   res *= 2;
 (*counts) = (int *)safemalloc(res * sizeof(int), "generate_subsets", 4);
 sets = (int **)safemalloc(res * sizeof(int *), "generate_subsets", 5);
 for (i = 0;i < res;i++)
  {
   (*counts)[i]=0;
   k=i;
   for (j = 0;j < howmany;j++)
    {
     if (k % 2)
      {
       (*counts)[i]++;
       sets[i] = alloc(sets[i],(*counts)[i] * sizeof(int),(*counts)[i]);
       sets[i][(*counts)[i] - 1] = j;
      }
     k = k / 2;
    }
  }
 (*total) = res;
 return sets;
}

BOOLEAN next_combination(int* combination, int size, int range)
{
 /*Output includes range i.e., range = 3 last element is ... 3*/
 /*!Last Changed 21.02.2008*/
 int i, j;
 for (i = size - 1; i > -1; i--) 
  {
   combination[i]++;
   if (combination[i] <= range - size + i + 1)
     break;
  }
 if (i == -1)
   return BOOLEAN_FALSE;
 for (j = i + 1; j < size; j++)
   combination[j] = combination[j - 1] + 1;
 return BOOLEAN_TRUE;
}

BOOLEAN next_combination_from_list(int* combination, int* list, int* indexlist, int size, int range)
{
 /*!Last Changed 21.02.2008*/
 int i, j;
 for (i = size - 1; i > -1; i--) 
  {
   indexlist[i]++;
   if (indexlist[i] <= range - size + i)
     break;
  }
 if (i == -1)
   return BOOLEAN_FALSE;
 combination[i] = list[indexlist[i]];
 for (j = i + 1; j < size; j++)
  {
   indexlist[j] = indexlist[j - 1] + 1;
   combination[j] = list[indexlist[j]]; 
  }
 return BOOLEAN_TRUE;
}

BOOLEAN next_iterator(int* iterator, int size, int range)
{
 /*Output includes range i.e., range = 3 last element is 3 3 3 ...*/
 /*!Last Changed 21.02.2008*/
 int i;
 for (i = size - 1; i >= 0; i--)
  {
   if (iterator[i] < range)
    {
     iterator[i]++;
     return BOOLEAN_TRUE;
    }
   iterator[i] = 0;
  }
 return BOOLEAN_FALSE;
}

BOOLEAN next_mixed_iterator(int* iterator, int size, int* rangelist)
{
 /*Output does not include range i.e., rangelist[0] = 3, rangelist[1] = 4, rangelist[2] = 2 last element is 2 3 1 */
 /*!Last Changed 14.11.2009*/
 int i;
 for (i = size - 1; i >= 0; i--)
  {
   if (iterator[i] < rangelist[i] - 1)
    {
     iterator[i]++;
     return BOOLEAN_TRUE;
    }
   iterator[i] = 0;
  }
 return BOOLEAN_FALSE;
}

BOOLEAN next_iterator_from_list(int* iterator, int* list, int* indexlist, int size, int range)
{
 /*!Last Changed 21.02.2008*/
 int i;
 for (i = size - 1; i >= 0; i--)
  {
   if (indexlist[i] < range)
    {
     indexlist[i]++;
     iterator[i] = list[indexlist[i]];
     return BOOLEAN_TRUE;
    }
   indexlist[i] = 0;
   iterator[i] = list[indexlist[i]];
  }
 return BOOLEAN_FALSE;
}

BOOLEAN next_permutation(int* permutation, int size)
{
 /*!Last Changed 21.02.2008*/
 int i = size - 2, j, k;
 while (i >= 0 && permutation[i] >= permutation[i + 1])
   i--;
 if (i == -1)
   return BOOLEAN_FALSE;
 j = size - 1;
 while (permutation[i] >= permutation[j])
   j--;
 swap_int(&(permutation[i]), &(permutation[j]));
 k = i + 1;
 j = size - 1;
 while (k < j)
  {
   swap_int(&(permutation[j]), &(permutation[k]));
   k++;
   j--;
  }
 return BOOLEAN_TRUE;
}
