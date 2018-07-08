#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary.h"
#include "constants.h"
#include "messages.h"
#include "libiteration.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"

/**
 * Constructor for Binaryfunctionptr. Allocates memory for a binary function, initializes and returns it
 * @return Returns allocated binary function.
 */
Binaryfunctionptr empty_binary_function()
{
 /*!Last Changed 19.04.2008*/
 Binaryfunctionptr result;
 result = (Binaryfunctionptr) safemalloc(sizeof(Binaryfunction), "empty_binary_function", 2);
 result->termcount = 0;
 result->terms = NULL;
 return result;
}

/**
 * Destructor of Binaryfunctionptr. Frees memory allocated for a binary function
 * @param[in] bf The binary function
 */
void free_binary_function(Binaryfunctionptr bf)
{
 /*!Last Changed 19.04.2008*/
 int i;
 for (i = 0; i < bf->termcount; i++)
  {
   safe_free(bf->terms[i].complement);
   safe_free(bf->terms[i].variables);
  }
 safe_free(bf);
}

/**
 * Adds a term to Binary function function
 * @param[in] function The binary function
 * @param[in] term The new term that will be added such as x1x3x4 (x1 AND x3 AND x4)
 */
void add_binary_term(Binaryfunctionptr function, Binaryterm term)
{
 /*!Last Changed 19.04.2008*/
 function->termcount++;
 function->terms = (Binarytermptr) alloc(function->terms, function->termcount * sizeof(Binaryterm), function->termcount);
 function->terms[function->termcount - 1] = term;
}

/**
 * Creates and returns a binary function from the string st.
 * @warning The index of the variable starts from 0 ends with 9.
 * @param[in] st String containing the binary function (Example x1x2+x1x3x5' means (x1 AND x2)OR (x1 AND x3 AND NOT x5))
 * @return The binary function
 */
Binaryfunctionptr read_binary_function(char* st)
{
 /*!Last Changed 19.04.2008*/
 int i, index = 0;
 Binaryfunctionptr result = NULL;
 Binaryterm term;
 term.count = 0;
 term.complement = NULL;
 term.variables = NULL;
 for (i = 0; i < strlen(st); i++)
  {
   switch (st[i])
    {
     case  '+':if (result == NULL)
                 result = empty_binary_function();
               add_binary_term(result, term);
               term.count = 0;
               term.complement = NULL;
               term.variables = NULL;
               break;
     case '\'':if (term.count > 0)
                 term.complement[term.count - 1] = BOOLEAN_TRUE;
               break;
     case  'x':term.count++;
               term.variables = (int *) alloc(term.variables, term.count * sizeof(int), term.count);
               term.complement = (BOOLEAN *) alloc(term.complement, term.count * sizeof(BOOLEAN), term.count);
               break;
     case  '0':
     case  '1':
     case  '2':
     case  '3':
     case  '4':
     case  '5':
     case  '6':
     case  '7':
     case  '8':
     case  '9':if (term.count > 0)
                {
                 index = st[i] - '0';
                 term.variables[term.count - 1] = index;
                 term.complement[term.count - 1] = BOOLEAN_FALSE;
                }
               break;
    }
  }
 if (result == NULL)
   result = empty_binary_function();
 add_binary_term(result, term);
 return result;
}

/**
 * Evaluates the binary function by assigning the variables values given in the values array.
 * @param[in] bf The binary function
 * @param[in] values The values of the variables
 * @return True or False according to the values of the variables
 */
BOOLEAN evaluate_binary_function(Binaryfunctionptr bf, int* values)
{
 /*!Last Changed 20.04.2008*/
 int i, j, index, complement;
 for (i = 0; i < bf->termcount; i++)
  {
   for (j = 0; j < bf->terms[i].count; j++)
    {
     index = bf->terms[i].variables[j];
     complement = bf->terms[i].complement[j];
     if ((values[index] && complement) || (!values[index] && !complement))
       break;
    }
   if (j == bf->terms[i].count)
     return BOOLEAN_TRUE;
  }
 return BOOLEAN_FALSE;
}

/**
 * Generates random data from a binary function. The data is exhaustive, the size of the data sample is 2^d
 * @param[in] bf The binary function
 * @param[in] d Number of variables
 * @param[in] fname Name of the output file
 */
void generate_random_data_from_binary_function(Binaryfunctionptr bf, int d, char* fname)
{
 /*!Last Changed 20.04.2008*/
 FILE* outfile;
 int* iterator;
 int result, i;
 outfile = fopen(fname, "w");
 if (!outfile)
  {
   printf(m1306);
   return;
  }
 iterator = first_iterator(d);
 do{
   result = evaluate_binary_function(bf, iterator);
   for (i = 0; i < d; i++)
     switch (iterator[i])
      {
       case BOOLEAN_FALSE:fprintf(outfile, "F ");
                          break;
       case  BOOLEAN_TRUE:fprintf(outfile, "T ");
                          break;
      }
   switch (result)
    {
     case BOOLEAN_FALSE:fprintf(outfile, "F\n");
                        break;
     case  BOOLEAN_TRUE:fprintf(outfile, "T\n");
                        break;
    }
 }while(next_iterator(iterator, d, 1));
 safe_free(iterator);
 fclose(outfile);
}

/**
 * Generates random noisy data from a binary function. The data is exhaustive, the size of the data sample is 2^d. To generate noisy data, the output is flipped if random generated value (between 0 and 1) is less than p.
 * @param[in] bf The binary function
 * @param[in] d Number of variables
 * @param[in] p The noise probability
 * @param[in] fname The output file name
 */
void generate_random_noisy_data_from_binary_function(Binaryfunctionptr bf, int d, double p, char* fname)
{
 /*!Last Changed 21.04.2008*/
 FILE* outfile;
 int* iterator;
 int result, i;
 outfile = fopen(fname, "w");
 if (!outfile)
  {
   printf(m1306);
   return;
  }
 iterator = first_iterator(d);
 do{
   result = evaluate_binary_function(bf, iterator);
   for (i = 0; i < d; i++)
     switch (iterator[i])
      {
       case BOOLEAN_FALSE:fprintf(outfile, "F ");
                          break;
       case  BOOLEAN_TRUE:fprintf(outfile, "T ");
                          break;
      }
   if (produce_random_value(0.0, 1.0) < p)
     result = 1 - result;
   switch (result)
    {
     case BOOLEAN_FALSE:fprintf(outfile, "F\n");
                        break;
     case  BOOLEAN_TRUE:fprintf(outfile, "T\n");
                        break;
    }
 }while(next_iterator(iterator, d, 1));
 safe_free(iterator);
 fclose(outfile);
}
