#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include "messages.h"
#include "libfile.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "options.h"
#include "parameter.h"
#include "statistics.h"
#include "tests.h"

/**
 * Kronecker Delta function. 
 * @param[in] i First number
 * @param[in] j Second number
 * @return If two numbers are equal returns 1, otherwise returns 0.
 */
int kronecker_delta(int i, int j)
{
 /*!Last Changed 19.01.2008*/
 if (i == j)
   return 1;
 else
   return 0;
}

/**
 * Checks the string for being comment.
 * @param[in] line String which is going to be checked
 * @return If the first character is #, then the line is a commented line.
 */
int is_line_comment(char* line)
{
 /*!Last Changed 29.12.2005*/
 if (line[0] == '#')
   return 1;
 return 0;
}

/**
 * Sets the format string according to the precision (for double type). The precision is set using the precision parameter. The function takes also into account the format string before, and after. For example, if current precision is 5, the format string will be %4.5lf
 * @param[out] precisionst Output format string
 * @param[in] before Format string before 
 * @param[in] after Format string after
 */
void set_precision(char* precisionst, char* before, char* after)
{
 /*!Last Changed 05.07.2005*/
 char st[10];
 strcpy(precisionst, "");
 if (before)
   strcat(precisionst, before);
 sprintf(st, "%%4.%dlf", get_parameter_i("precision"));
 strcat(precisionst, st);
 if (after)
   strcat(precisionst, after);
}

/**
 * Swaps two character variables.
 * @param[out] i First character variable
 * @param[out] j Second character variable
 */
void swap_char(char* i, char* j)
{
 /*!Last Changed 28.04.2005*/
 char tmp;
 tmp = *i;
 *i = *j;
 *j = tmp;
}

/**
 * Swaps two integer variables
 * @param[out] i First integer variable
 * @param[out] j Second integer variable
 */
void swap_int(int* i, int* j)
{
 /*!Last Changed 26.04.2005*/
 int tmp;
 tmp = *i;
 *i = *j;
 *j = tmp;
}

/**
 * Swaps two double variables
 * @param[out] i First double variable
 * @param[out] j Second double variable
 */
void swap_double(double* i, double* j)
{
 /*!Last Changed 26.04.2005*/
 double tmp;
 tmp = *i;
 *i = *j;
 *j = tmp;
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting integer objects in ascending order.
 * @param[in] a Pointer to the first integer object
 * @param[in] b Pointer to the second integer object
 * @return 0 if two integers are equal, -1 if the first integer is smaller than the second one, +1 if the first integer is larger that the second one.
 */
int sort_function_int_ascending(const void *a, const void *b)
{
 /*!Last Changed 21.05.2005*/
 int num1, num2;
 num1 = *((int*) a);
 num2 = *((int*) b);
 if (num1 < num2)
   return -1;
 else
   if (num1 > num2)
     return 1;
 return 0;
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting integer objects in descending order.
 * @param[in] a Pointer to the first integer object
 * @param[in] b Pointer to the second integer object
 * @return 0 if two integers are equal, +1 if the first integer is smaller than the second one, -1 if the first integer is larger that the second one.
 */
int sort_function_int_descending(const void *a, const void *b)
{
 /*!Last Changed 21.05.2005*/
 return -sort_function_int_ascending(a, b);
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting double objects in ascending order.
 * @param[in] a Pointer to the first double object
 * @param[in] b Pointer to the second double object
 * @return 0 if two doubles are equal, -1 if the first double is smaller than the second one, +1 if the first double is larger that the second one.
 */
int sort_function_double_ascending(const void *a, const void *b)
{
 /*!Last Changed 02.03.2005*/
 double num1, num2;
 num1 = *((double*) a);
 num2 = *((double*) b);
 if (num1 < num2)
   return -1;
 else
   if (num1 > num2)
     return 1;
 return 0;
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting double objects in descending order.
 * @param[in] a Pointer to the first double object
 * @param[in] b Pointer to the second double object
 * @return 0 if two doubles are equal, +1 if the first double is smaller than the second one, -1 if the first double is larger that the second one.
 */
int sort_function_double_descending(const void *a, const void *b)
{
 /*!Last Changed 02.03.2005*/
 return -sort_function_double_ascending(a, b);
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting observation objects in ascending order.
 * @param[in] a Pointer to the first observation object
 * @param[in] b Pointer to the second observation object
 * @return 0 if two observations are equal, -1 if the first observation is smaller than the second one, +1 if the first observation is larger that the second one.
 */
int sort_function_observation_ascending(const void *a, const void *b)
{
 /*!Last Changed 13.01.2009*/
 Obs obs1, obs2;
 obs1 = *((Obs*) a);
 obs2 = *((Obs*) b);
 if (obs1.observation < obs2.observation)
   return -1;
 else
   if (obs1.observation > obs2.observation)
     return 1;
 return 0;
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting observation objects in descending order.
 * @param[in] a Pointer to the first observation object
 * @param[in] b Pointer to the second observation object
 * @return 0 if two observations are equal, +1 if the first observation is smaller than the second one, -1 if the first observation is larger that the second one.
 */
int sort_function_observation_descending(const void *a, const void *b)
{
 /*!Last Changed 13.01.2009*/
 return -sort_function_observation_ascending(a, b);
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting string objects in ascending order.
 * @param a Pointer to the first string object
 * @param b Pointer to the second string object
 * @return 0 if two string are equal, -1 if the first string comes before the second one in the dictionary, +1 if the first integer comes after the second one.
 */
int sort_function_string_ascending(const void *a, const void *b)
{
 return strcmp(a, b);
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting string objects in ascending order.
 * @param a Pointer to the first string object
 * @param b Pointer to the second string object
 * @return 0 if two string are equal, -1 if the first string comes after the second one in the dictionary, +1 if the first integer comes before the second one.
 */
int sort_function_string_descending(const void *a, const void *b)
{
 return -strcmp(a, b);
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting image objects in ascending order.
 * @param[in] a Pointer to the first image object
 * @param[in] b Pointer to the second image object
 * @return 0 if two image objects are equal, -1 if the first image object is smaller than the second one, +1 if the first image object is larger that the second one.
 */
int sort_function_image_object_ascending(const void *a, const void *b)
{
 /*!Last Changed 21.05.2005*/
 Object obj1, obj2;
 obj1 = *((Objectptr) a);
 obj2 = *((Objectptr) b);
 if (obj1.type > obj2.type)
   return 1;
 else
   if (obj1.type < obj2.type)
     return -1;
   else
     if (obj1.fnt.fontcolor > obj2.fnt.fontcolor)
       return 1;
     else
       if (obj1.fnt.fontcolor < obj2.fnt.fontcolor)
         return -1;
       else
         if (obj1.fnt.fontsize > obj2.fnt.fontsize)
           return 1;
         else
           if (obj1.fnt.fontsize < obj2.fnt.fontsize)
             return -1;
           else
             if (obj1.fnt.dashedtype > obj2.fnt.dashedtype)
               return 1;
             else
               if (obj1.fnt.dashedtype < obj2.fnt.dashedtype)
                 return -1;
 return 0;
}

/**
 * Qsort in the standard ANSI C library needs a comparison function for sorting any two objects. This comparison function is used in sorting image objects in descending order.
 * @param[in] a Pointer to the first image object
 * @param[in] b Pointer to the second image object
 * @return 0 if two image objects are equal, +1 if the first image object is smaller than the second one, -1 if the first image object is larger that the second one.
 */
int sort_function_image_object_descending(const void *a, const void *b)
{
 /*!Last Changed 21.05.2005*/
 return -sort_function_image_object_ascending(a, b);
}

int* findmincounts(char** names, int count)
{
 /*!Last Changed 25.01.2005*/
 int i, j, length, *result = safecalloc(count, sizeof(int), "findmincounts", 1);
 double** data, min;
 FILE* infile;
 length = file_length(names[0]);
 data = (double**) safemalloc_2d(sizeof(double*), sizeof(double), count, length, "findmincounts", 5);
 for (i = 0; i < count; i++)
  {
   infile = fopen(names[i], "r");
   if (!infile)
    {
     printf(m1274, names[i]);
     return NULL;
    }
   for (j = 0; j < length; j++)
     fscanf(infile, "%lf", &(data[i][j]));
   fclose(infile);
  }
 for (j = 0; j < length; j++)
  {
   min = data[0][j];
   for (i = 1; i < count; i++)
     if (data[i][j] < min)
       min = data[i][j];
   for (i = 0; i < count; i++)
     if (data[i][j] == min)
       (result[i])++;
  }
 free_2d((void**)data, count);
 return result;
}

void sort_two_arrays(double* array1, int* array2, int size)
{
 /*!Last Changed 21.10.2004*/
 int i, j, tmpi;
 double tmp;
 for (i = 0; i < size; i++)
   for (j = i + 1; j < size; j++)
     if (array1[i] > array1[j])
      {
       tmpi = array2[i];
       array2[i] = array2[j];
       array2[j] = tmpi;
       tmp = array1[i];
       array1[i] = array1[j];
       array1[j] = tmp;
      }
}

void check_and_put_in_between(int* x, int left, int right)
{
 /*!Last Changed 29.12.2005*/
 if (*x < left)
  *x = left;
 else
   if (*x > right)
     *x = right;
}

int isfloat(char* st)
{
 /*!Last Changed 29.09.2004 added - sign*/
 /*!Last Changed 29.01.2004*/
 int i, len = strlen(st);
 for (i = 0; i < len; i++)
   if ((st[i] < '0' || st[i] > '9') && st[i] != '.' && st[i] != '-' && st[i] != 'E' && st[i] != 'e' && st[i] != '+')
     return 0;
 return 1;
}

int isinteger(char* st)
{
 /*!Last Changed 29.09.2004 added - sign*/
 /*!Last Changed 29.01.2004*/
 int i, len = strlen(st);
 for (i = 0; i < len; i++)
   if ((st[i] < '0' || st[i] > '9') && st[i] != '-' && st[i] != '+')
     return 0;
 return 1;
}

/**
 * Checks if the character is an english letter
 * @param[in] ch Character to be checked
 * @return BOOLEAN_TRUE if the character is an english letter, BOOLEAN_FALSE otherwise
 */
BOOLEAN is_letter(char ch)
{
 /*!Last Changed 27.02.2009*/
 if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
   return BOOLEAN_TRUE;
 else
   return BOOLEAN_FALSE;
}

int atoi_check(char* st, int* value, int min, int max)
{
 /*!Last Changed 25.07.2006*/
 int tmp;
 if (isinteger(st))
  {
   tmp = atoi(st);
   if (tmp < min)
    {
     printf(m1405, tmp, min);
     return NO;
    }
   else
     if (tmp > max)
      {
       printf(m1406, tmp, max);
       return NO;
      }
     else
      {
       *value = tmp;
       return YES;
      }
  }
 else
   printf(m1341, st);
 return NO;
}

int atof_check(char* st, double* value, double min, double max)
{
 /*!Last Changed 25.07.2006*/
 double tmp;
 if (isfloat(st))
  {
   tmp = atof(st);
   if (tmp < min)
    {
     printf(m1407, tmp, min);
     return NO;
    }
   else
     if (tmp > max)
      {
       printf(m1408, tmp, max);
       return NO;
      }
     else
      {
       *value = tmp;
       return YES;
      }
  }
 else
   printf(m1340, st);
 return NO;
}

int in_list(int searchvalue, int count, ...)
{
 /*!Last Changed 29.08.2005*/
 int i, compvalue;
 va_list ap;
 va_start(ap, count);
 for (i = 0; i < count; i++)
  {
   compvalue = va_arg(ap, int);
   if (searchvalue == compvalue)
     return 1;
  }
 return 0;
}

int checkparams(int paramcount, int must, ...)
{
 int i;
 char* line;
 va_list ap;
 va_start(ap, must);
 if (paramcount < must)
  {
   i=0;
   line = va_arg(ap,char *);
   while (i < paramcount)
    {
     line=va_arg(ap,char *);
     i++;
    }
   printf("%s",line);
   return 0;
  }
 return 1;
}

void check_and_set_mean(double* mean, int count)
{
 /*!Last Changed 07.05.2004*/
 if (count != 0)
   *mean = *mean / count;
 else
   *mean = 0.0;
}

void check_and_set_meani(int* mean, int count)
{
 /*!Last Changed 07.05.2004*/
 if (count != 0)
   *mean = *mean / count;
 else
   *mean = 0;
}

void find_mse_and_sse(char* inname, char* outname1, char* outname2)
{
 /*!Last Changed 18.12.2004*/
 FILE* infile;
 FILE* outfile1, *outfile2;
 char buffer[450000];
 char number[10];
 double numbers[5000], meansum, msesum, ssesum;
 int read, i, j, k, count, totalread, first = 1;
 infile = fopen(inname, "r");
 if (!infile)
   return;
 outfile1 = fopen(outname1, "w");
 if (!outfile1)
   return;
 outfile2 = fopen(outname2, "w");
 if (!outfile2)
   return;
 read = fread(buffer, 1, 450000, infile);
 i = 0;
 j = 0;
 totalread = -1;
 while (i < read)
  {
   if (buffer[i] != 10)
    {
     number[j] = buffer[i];
     j++;
    }
   else
    {
     number[j] = 0;
     j = 0;
     if (totalread == -1)
      {
       count = atoi(number);
       totalread = 0;
       meansum = 0;
       msesum = 0;
      }
     else
      {
       numbers[totalread] = (double) atof(number);
       meansum += numbers[totalread];
       msesum += numbers[totalread] * numbers[totalread];
       totalread++;
       if (totalread == count)
        {
         if (first)
           fprintf(outfile2, "%d\n", count);
         fprintf(outfile1, "%5.4f\n", msesum / count);
         ssesum = 0;
         meansum /= count;
         for (k = 0; k < count; k++)
           ssesum += (numbers[k] - meansum) * (numbers[k] - meansum);
         fprintf(outfile2, "%5.4f\n", ssesum / (count - 1));
         totalread = -1;
         first = 0;
        }
      }   
    }
   i++;
  }
 fclose(infile);
 fclose(outfile1);
 fclose(outfile2);
}
