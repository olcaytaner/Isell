#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "lang.h"
#include "libarray.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "matrix.h"
#include "mixture.h"
#include "model.h"
#include "parameter.h"
#include "perceptron.h"
#include "poly.h"
#include "statistics.h"

/**
 * Reads one dimensional data from an input file.
 * @param[in] fname Input file name
 * @param[out] datacount Number of data points read
 * @return The function creates a two-dimensional array. array[i][0] refers to the x value (input value) and array[i][1] refers to the y value (output value) of the i'th data point.
 */
double** readpolydata(char* fname, int* datacount)
{
	/*!Last Changed 22.01.2005*/
	char line[READLINELENGTH];
	FILE* infile;
	double** data;
	int i;
 *datacount = file_length(fname);
 infile = fopen(fname,"r");
	if (!infile)
		 return NULL;
	data = (double**) safemalloc_2d(sizeof(double*), sizeof(double), *datacount, 2, "read_polydata", 6);
 i = 0;
 while (fgets(line, READLINELENGTH, infile))
  {
   sscanf(line, "%lf %lf", &(data[i][0]), &(data[i][1]));
   i++;
  }
 fclose(infile);
	return data;
}

/**
 * Calculates the mean polynom of k polynoms. For example the mean polynom of 2x^3 + 3x^2 + 7 and 5x^2 + 4x + 9 is x^3 + 4x^2 + 2x + 8.
 * @param[in] polynoms An array of polynoms, whose mean will be found
 * @param[in] polynomcount Number of polynoms (size of the polynoms array)
 * @return Mean polynom of k polynoms. Allocated and calculated.
 */
Polynomptr mean_of_polynoms(Polynomptr* polynoms, int polynomcount)
{
	/*!Last Changed 21.01.2005*/
	Polynomptr p;
	Polynodeptr* nodes;
	int i, currentdegree = -1, count;
	double sum;
	p = create_polynom();
	nodes = safemalloc(polynomcount * sizeof(Polynodeptr), "mean_of_polynoms", 6);
 for (i = 0; i < polynomcount; i++)
	 {
		 nodes[i] = polynoms[i]->start;
			if (nodes[i] && nodes[i]->power > currentdegree)
				 currentdegree = nodes[i]->power;
	 }
	while (currentdegree >= 0)
	 {
		 sum = 0;
			count = 0;
		 for (i = 0; i < polynomcount; i++)
				 if (nodes[i] && nodes[i]->power == currentdegree)
					 {
						 sum += nodes[i]->coefficient;
							count++;
						 nodes[i] = nodes[i]->next;
					 }
   if (count > 0)
				 add_polynode_to_polynom(p, currentdegree, sum / count);
		 currentdegree--;
	 }
	safe_free(nodes);
	return p;
}

/**
 * Finds the minimum and maximum boundary values of a partially defined function.
 * @param[in] fx Partially defined function. fx[i] is the i'th partial function.
 * @param[in] functioncount Number of partial functions (Size of the fx array).
 * @param[out] min Minimum boundary value of the function
 * @param[out] max Maximum boundary value of the function
 */
void find_function_bounds(Functionptr fx, int functioncount, double* min, double* max)
{
	/*!Last Changed 21.01.2005*/
	int i;
	*min = fx[0].min;
	*max = fx[0].max;
	for (i = 1; i < functioncount; i++)
	 {
		 if (fx[i].min < *min)
				 *min = fx[i].min;
		 if (fx[i].max > *max)
				 *max = fx[i].max;
	 }
}

/**
 * Generates random data from a given function. 
 * @param[in] datasize Number of data points to generate
 * @param[in] fx Partially defined function. fx[i] is the i'th partial function.
 * @param[in] functioncount Number of partial functions (Size of the fx array).
 * @param[in] noisy If noisy is YES, adds noise with zero mean and epsilon variance to the function output.
 * @return The function creates a two-dimensional array. array[i][0] refers to the x value (input value) and array[i][1] refers to the y value (output value) of the i'th data point.
 */
double** functiondatagenerate(int datasize, Functionptr fx, int functioncount, int noisy)
{
	/*!Last Changed 21.01.2005*/
 int i;
 double x, value, **values, start, end;
	values = (double**) safemalloc_2d(sizeof(double *), sizeof(double), datasize, 2, "functiondatagenerate", 3);
 find_function_bounds(fx, functioncount, &start, &end);
 for (i = 0; i < datasize; i++)
  {
   x = produce_random_value(start, end);
   value = function_value(fx, functioncount, x);
   if (noisy)
     value = value + produce_normal_value(0, get_parameter_f("noiselevel"));
   values[i][0] = x;
   values[i][1] = value;
  }
	return values;
}

/**
 * Generates random data from a given function given its definition in a file. 
 * @param[in] datasize Number of data points to generate
 * @param[in] functiondefinition Name of the file containing the function definition
 * @param[in] noisy If noisy is YES, adds noise with zero mean and epsilon variance to the function output.
 * @return The function creates a two-dimensional array. array[i][0] refers to the x value (input value) and array[i][1] refers to the y value (output value) of the i'th data point.
 */
double** functiondatageneratefile(int datasize, char* functiondefinition, int noisy)
{
	/*!Last Changed 21.01.2005*/
	int functioncount, i;
	double** data;
	Functionptr fx;
	fx = read_function_definition(functiondefinition, &functioncount);
	data = functiondatagenerate(datasize, fx, functioncount, noisy);
 for (i = 0;i < functioncount; i++)
	  free_function(fx[i]);
	safe_free(fx);
	return data;
}

/**
 * Calculates the value of a partially defined function at a point X.
 * @param[in] fx Partially defined function. fx[i] is the i'th partial function.
 * @param[in] functioncount Number of partial functions (Size of the fx array).
 * @param[in] x X
 * @return Output value of the function given the input value
 */
double function_value(Functionptr fx, int functioncount, double x)
{
	/*!Last Changed 21.01.2005*/
	int i, which = -1;
	Functionnodeptr tmp;
	double value, sum = 0, polynomvalue;
	for (i = 0; i < functioncount; i++)
		 if (x >= fx[i].min && x <= fx[i].max)
			 {
				 which = i;
				 break;
			 }
 tmp = fx[which].start;
	while (tmp)
	 {
		 value = tmp->coefficient;
			if (tmp->xpower != 0)
				 value *= fpow(x, tmp->xpower);
			if (tmp->sinpower != 0)
			 {
				 polynomvalue = polynom_value(tmp->sinus, x);
					value *= fpow(sin(polynomvalue), tmp->sinpower);
			 }
			if (tmp->cospower != 0)
			 {
				 polynomvalue = polynom_value(tmp->cosinus, x);
					value *= fpow(cos(polynomvalue), tmp->cospower);
			 }
			sum += value;
		 tmp = tmp->next;
	 }
	return sum;
}

/**
 * Deallocates memory allocated for sinus and cosinus parts of the function. If the function does not have any one of them, does nothing.
 * @param[in] fnode One part of the function
 */
void free_functionnode(Functionnodeptr fnode)
{
	/*!Last Changed 19.01.2005*/
	if (fnode->sinpower != 0)
  	free_polynom(fnode->sinus);
	if (fnode->cospower != 0)
  	free_polynom(fnode->cosinus);
}

/**
 * Destructor of the function structure. Deallocates memory allocated its partial functions and itself.
 * @param[in] fx Function structure
 */
void free_function(Function fx)
{
	/*!Last Changed 19.01.2005*/
	Functionnodeptr tmp, next;
	tmp = fx.start;
	while (tmp)
	 {
		 next = tmp->next;
			free_functionnode(tmp);
			safe_free(tmp);
			tmp = next;
	 }
}

/**
 * Divides an input string containing a partial function into 6 parts, where some of them may not exist. For example, if the input string is 3.2x4s2(4x3)c(2x) the parts will be 3.2, 4, 2, 4x3, 1, 2x
 * @param[in] st Input string
 * @param[out] parts[] parts[0] is the coefficient part. It is 3.2 in the example above . \n
 * parts[1] is the exponent of the x. If x does not exist it is NULL. If x part exists but exponent part does not exist, it is 1. It is 4 in the example above. \n 
 * parts[2] is the exponent part of the sinus function. If the sinus part does not exist it is NULL. If sinus part exists but exponent part does not exist, it is 1. It is 2 in the example above. \n
 * parts[3] is the formula of the sinus polynom function. The formula part is separated from other parts by opening and closing parenthesis. If the formula part doest not exist it is NULL. It is 4x3 in the example above. \n
 * parts[4] is the exponent part of the cosinus function. If the cosinus part does not exist it is NULL. If cosinus part exists but exponent part does not exist, it is 1. It is 1 in the example above. \n
 * parts[5] is the formula of the cosinus polynom function. The formula part is separated from other parts by opening and closing parenthesis. If the formula part doest not exist it is NULL. It is 2x in the example above. 
 */
void divide_function_into_parts(char* st, char* parts[6])
{
	/*!Last Changed 19.01.2005*/
	int i, len = strlen(st), mode = 0, pos, parenthesis = 0;
	char line[READLINELENGTH], *data;
	for (i = 0; i < 6; i++)
		 parts[i] = NULL;
	i = 0;
	pos = 0;
	while (i < len)
	 {
		 if ((st[i] >= '0' && st[i] <= '9') || st[i] == '.' || st[i] == '-' || st[i] == '+' || (parenthesis == 1 && st[i] != ')'))
			 {
				 line[pos] = st[i];
					pos++;
					line[pos] = '\0';
			 }
			else
				 if (st[i] == 'x')
					 {
						 if (pos != 0)
							 {
								 data = strcopy(data, line);
									parts[mode] = data;
							 }
						 mode = 1;
							pos = 0;
					 }
					else
						 if (st[i] == 's')
							 {
  						 if (pos != 0)
									 {
							  	 data = strcopy(data, line);
							  		parts[mode] = data;
									 }
								 else
										 if (mode == 1)
											 {
												 data = strcopy(data, "1");
													parts[1] = data;
											 }
								 i += 2;
									mode = 2;
									pos = 0;
							 }
							else
								 if (st[i] == 'c')
									 {
    						 if (pos != 0)
											 {
							    	 data = strcopy(data, line);
							    		parts[mode] = data;
											 }
								   else
									  	 if (mode == 1)
													 {
										  		 data = strcopy(data, "1");
										  			parts[1] = data;
													 }
								   i += 2;
									  mode = 4;
									  pos = 0;
									 }
									else
										 if (st[i] == '(')
											 {
      						 if (pos != 0)
													 {
							      	 data = strcopy(data, line);
							    	  	parts[mode] = data;
													 }
													else
													 {
										  		 data = strcopy(data, "1");
							    	  	parts[mode] = data;
													 }
													mode++;
													pos = 0;
												 parenthesis = 1;
											 }
											else
												 if (st[i] == ')')
													 {
							      	 data = strcopy(data, line);
							    	  	parts[mode] = data;
														 parenthesis = 0;
														 pos = 0;
													 }
		 i++;
	 }
 if (pos != 0)
		{
			data = strcopy(data, line);
			parts[mode] = data;
		}
	else
		 if (mode == 1)
				{
					data = strcopy(data, "1");
					parts[1] = data;
				}
}

/**
 * Divides a function string to an array of function tokens.
 * @param[in] st Function string containing one or more subfunctions. For example 3.2x4s2(4x3)c(2x)+x2+s(2x^4)c3(x) is a function string containing three tokens as 3.2x4s2(4x3)c(2x), x2 and s(2x^4)c3(x).
 * @param[out] tokencount Number of tokens in the function string
 * @return A string array containing the tokens. array[i] is the i'th token. In the example; array[0], array[1] and array[2] are 3.2x4s2(4x3)c(2x), x2 and s(2x^4)c3(x) respectively.
 */
char** plus_minus_token_list(char* st, int* tokencount)
{
	/*!Last Changed 19.01.2005*/
 int i = 0, j, k, len = strlen(st), parcount = 0, pos = 0, add, minus = 0, linelength;
 char line[READLINELENGTH], **tokenlist = NULL;
	*tokencount = 0;
 while (i < len)
	 {
		 add = 0;
		 if (st[i] == '(')
			 {
				 parcount++;
					add = 1;
			 }
			else
				 if (st[i] == ')')
					 {
						 parcount--;
							add = 1;
					 }
					else
						 if (st[i] == '+' || st[i] == '-')
								 if (parcount == 0)
									 {
											tokenlist = alloc(tokenlist, ((*tokencount) + 1) * sizeof(char *), (*tokencount) + 1);
											linelength = strlen(line);
  									tokenlist[*tokencount] = safemalloc(sizeof(char) * (linelength + minus + 1), "plus_minus_token_list", 24);
           for (j = 0; j < minus; j++)
											  tokenlist[*tokencount][j] = '-';
											for (j = minus, k = 0; j < linelength + minus; j++, k++)
												 tokenlist[*tokencount][j] = line[k];
           tokenlist[*tokencount][j] = '\0';
										 (*tokencount)++;
											if (st[i] == '-')
												 minus = 1;
											else
												 minus = 0;
											pos = 0;
									 }
									else
									  add = 1;
							else
							  add = 1;
			if (add)
			 {
					line[pos] = st[i];
					pos++;
					line[pos] = '\0';
			 }
		 i++;
	 }
	if (pos != 0)
	 {
			tokenlist = alloc(tokenlist, ((*tokencount) + 1) * sizeof(char *), (*tokencount) + 1);
			linelength = strlen(line);
  	tokenlist[*tokencount] = safemalloc(sizeof(char) * (linelength + minus + 1), "plus_minus_token_list", 53);
   for (j = 0; j < minus; j++)
					tokenlist[*tokencount][j] = '-';
			for (j = minus, k = 0; j < linelength + minus; j++, k++)
					tokenlist[*tokencount][j] = line[k];
   tokenlist[*tokencount][j] = '\0';
			(*tokencount)++;
	 }
	return tokenlist;
}

/**
 * Constructor for a function. Generates a function structure from a string such as 3.2x4s2(4x3)c(2x)+x2+s(2x^4)c3(x) 2.4 6.7, where 3.2x4s2(4x3)c(2x)+x2+s(2x^4)c3(x) is the string containing the function definition, 2.4 and 6.7 defines the input range of the function such as [2.4, 6.7].
 * @param[in] st String
 * @return Function structure allocated and read
 */
Function create_function(char* st)
{
	/*!Last Changed 19.01.2005*/
	Function tmp;
	Functionnodeptr tmpnode;
	char* functionp, *minp, *maxp, **tokens;
 char* parts[6];
	int i, tokencount, j;
	tmp.min = 0;
	tmp.max = -1;
	functionp = strtok(st, " \n");
	if (!functionp)
		 return tmp;
 minp = strtok(NULL, " \n");
	if (!minp)
		 return tmp;
 maxp = strtok(NULL, " \n");
	if (!maxp)
		 return tmp;
	tmp.min = atof(minp);
	tmp.max = atof(maxp);
	if (tmp.min > tmp.max)
		 return tmp;
	tmp.start = NULL;
	tmp.end = NULL;
	tokens = plus_minus_token_list(functionp, &tokencount);
	for (j = 0; j < tokencount; j++)
	 {
		 tmpnode = safemalloc(sizeof(Functionnode), "create_function", 26);
			tmpnode->next = NULL;
   divide_function_into_parts(tokens[j], parts);
			if (parts[0])
				 tmpnode->coefficient = atof(parts[0]);
			else
			  tmpnode->coefficient = 1;
			if (parts[1]) 
				 tmpnode->xpower = atoi(parts[1]);
			else
				 tmpnode->xpower = 0;
			if (parts[2])
			 {
				 tmpnode->sinpower = atoi(parts[2]);
					if (parts[3])
						 tmpnode->sinus = read_polynom(parts[3]);
					else
					  tmpnode->sinus = NULL;
			 }
			else
				 tmpnode->sinpower = 0;
			if (parts[4])
			 {
				 tmpnode->cospower = atoi(parts[4]);
					if (parts[5])
						 tmpnode->cosinus = read_polynom(parts[5]);
					else
					  tmpnode->cosinus = NULL;
			 }
			else
				 tmpnode->cospower = 0;
			for (i = 0; i < 6; i++)
				 if (parts[i])
						 safe_free(parts[i]);
			if (tmp.end)
			 {
				 tmp.end->next = tmpnode;
					tmp.end = tmpnode;
			 }
			else
			 {
				 tmp.start = tmpnode;
					tmp.end = tmpnode;
			 }
	 }
	free_2d((void**)tokens, tokencount);
	return tmp;
}

/**
 * Reads a partially defined function from a file. The file contains: \n
 * Function string1 Lower bound of the input range of function1 Upper bound of the input range of function1 \n
 * Function string2 Lower bound of the input range of function2 Upper bound of the input range of function2 \n
 * ... \n
 * Function stringK Lower bound of the input range of functionK Upper bound of the input range of functionK
 * @param[in] functiondefinition Name of the input file containing the function definition
 * @param[out] functioncount Number of partial functions in the function
 * @return Structure containing the partially defined function allocated and values are set
 */
Functionptr read_function_definition(char* functiondefinition, int* functioncount)
{
	/*!Last Changed 19.01.2005*/
	FILE* infile;
	Functionptr fx = NULL;
	Function tmp;
	char line[READLINELENGTH];
	*functioncount = 0;
	infile = fopen(functiondefinition, "r");
	if (!infile)
		 return NULL;
 while (fgets(line, READLINELENGTH, infile))
	 {
		 tmp = create_function(line);
			if (tmp.min > tmp.max)
				 return fx;
			fx = alloc(fx, (*functioncount + 1) * sizeof(Function), *functioncount + 1);
			fx[*functioncount] = tmp;
			(*functioncount)++;
	 }
	fclose(infile);
	return fx;
}

/**
 * Constructor function of the polynom structure. Allocates memory for an empty polynom.
 * @return An empty polynom structure allocated.
 */
Polynomptr create_polynom()
{
	/*!Last Changed 02.02.2004 added create_polynom*/
 Polynomptr result = (Polynomptr)safemalloc(sizeof(Polynom), "create_polynom", 1);
 result->start = NULL;
 result->end = NULL;
 return result;
}

/**
 * Prints a polynom to a string.
 * @param[in] p Polynom structure
 * @param[out] st String which will contain the polynom
 */
void print_polynom(Polynomptr p, char* st)
{
	/*!Last Changed 23.01.2005*/
 Polynodeptr tmp;
	char literal[20], pst[READLINELENGTH];
	strcpy(st, "");
 tmp = p->start;
 while (tmp)
  {
   if (tmp->coefficient < 0)
     if (tmp->power > 1)
					 {
    			set_precision(pst, NULL, "x%d");
       sprintf(literal, pst, tmp->coefficient, tmp->power);
					 }
     else
       if (tmp->power == 1)
							 {
			      set_precision(pst, NULL, "x");
         sprintf(literal, pst, tmp->coefficient);
							 }
       else
							 {
      			set_precision(pst, NULL, NULL);
         sprintf(literal, pst, tmp->coefficient);
							 }
   else
     if (tmp->power > 1)
					 {
			    set_precision(pst, "+", "x%d");
       sprintf(literal, pst, tmp->coefficient, tmp->power);
					 }
     else
       if (tmp->power == 1)
							 {
			      set_precision(pst, "+", "x");
         sprintf(literal, pst, tmp->coefficient);
							 }
       else
							 {
			      set_precision(pst, "+", NULL);
         sprintf(literal, pst, tmp->coefficient);
							 }
   strcat(st, literal);
   tmp = tmp->next;
  }
}

/**
 * Destructor function of the polynom structure. Deallocates memory allocated for the polynom.
 * @param[in] p Polynom structure
 */
void free_polynom(Polynomptr p)
{
 Polynodeptr tmp, tmpnxt;
 tmp = p->start;
 while (tmp)
  {
   tmpnxt = tmp->next;
   safe_free(tmp);
   tmp = tmpnxt;
  }
 safe_free(p);
}

/**
 * Calculates the value of the polynom at a point X
 * @param[in] p Polynom structure
 * @param[in] value X
 * @return Value of the polynom at point value
 */
double polynom_value(Polynomptr p, double value)
{
 double result = 0;
 Polynodeptr tmp = p->start;
 while (tmp)
  {
   result += tmp->coefficient * fpow(value,tmp->power);
   tmp = tmp->next;
  }
 return result;
}

/**
 * Calculates the classification error of a polynom fit on a given test set.
 * @param[in] data One-dimensional two-class classification test set. data[i][0] is the value of x (input value), data[i][1] is the class value 0 for the first class 1 for the second class.
 * @param[in] datasize Number of data points in the test set (size of the data array).
 * @param[in] p Polynom fit
 * @return Classification error calculated as: number of misclassifications / number of data points.
 */
double calculate_poly_clserror(double** data, int datasize, Polynomptr p)
{
	/*!Last Changed 26.01.2005*/
 double sum = 0, r, y, o;
	int i;
	for (i = 0; i < datasize; i++)
	 {
		 y = polynom_value(p, data[i][0]);
			if (sigmoid(y) > 0.5)
				 o = 1;
			else
				 o = 0;
			r = data[i][1];
			if (r != o)
     sum++;
	 }
 if (datasize == 0)
   return 0;
 return sum / datasize;
}

/**
 * Calculates log likelihood of a polynom fit on a given test set.
 * @param[in] data One-dimensional two-class classification test set. data[i][0] is the value of x (input value), data[i][1] is the class value 0 for the first class 1 for the second class.
 * @param[in] datasize Number of data points in the test set (size of the data array).
 * @param[in] p Polynom fit
 * @return Log likelihood is calculated as: If the correct class is 0, log(sigmoid(calculated output)), if the correct class is 1, log(1 - sigmoid(calculated output)).
 */
double calculate_poly_loglikelihood(double** data, int datasize, Polynomptr p)
{
	/*!Last Changed 26.01.2005*/
 double sum = 0, y, o;
	int i;
	for (i = 0; i < datasize; i++)
	 {
		 y = polynom_value(p, data[i][0]);
			o = sigmoid(y);
			if (data[i][1] == 1 && o != 0)
				 sum += log(o);
			else
				 if (o != 1)
						 sum += log(1 - o);
	 }
 return sum;
}

/**
 * Calculates the mean square error of a polynom fit on a given test set.
 * @param[in] data One-dimensional two-class regression test set. data[i][0] is the value of x (input value), data[i][1] is the regression value.
 * @param[in] datasize Number of data points in the test set (size of the data array).
 * @param[in] p Polynom fit
 * @return Mean square error calculated as: Total squared error / number of data points.
 */
double calculate_mse(double** data, int datasize, Polynomptr p)
{
	/*!Last Changed 21.01.2005*/
 double sum = 0, r, y;
	int i;
	for (i = 0; i < datasize; i++)
	 {
		 y = polynom_value(p, data[i][0]);
			r = data[i][1];
   sum += (r - y) * (r - y);
	 }
 if (datasize == 0)
   return 0;
 return sum / datasize;
}

/**
 * Calculates the mean square error of a polynom fit on a given test set read from a file.
 * @param[in] fname File name containing the test
 * @param[in] p Polynom fit
 * @return Mean square error calculated as: Total squared error / number of data points.
 */
double calculate_msefile(char* fname, Polynomptr p)
{
	/*!Last Changed 21.01.2005*/
 FILE* infile;
 char line[READLINELENGTH];
 double inp, out, sum = 0, tmp;
 int count = 0;
 infile = fopen(fname, "r");
 if (infile == NULL)
   return +LONG_MAX;
 while (fgets(line, READLINELENGTH, infile))
  {
   sscanf(line, "%lf %lf", &inp, &out);
   tmp = polynom_value(p, inp);
   sum += (tmp - out) * (tmp - out);
   count++;
  }
 fclose(infile);
 if (count == 0)
   return 0;
 return sum / count;
}

void add_polynode_to_polynom(Polynomptr p, int power, double coefficient)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 Polynodeptr result, tmp, tmpbefore;
 result = (Polynodeptr)safemalloc(sizeof(Polynode), "add_polynode_to_polynom", 2);
 result->power=power;
 result->coefficient=coefficient;
 result->next=NULL;
 if (p->start==NULL)/*If the list is empty*/
  {
   p->start=result;
   p->end=result;
   return;
  }
 tmp=p->start;
 tmpbefore=NULL;/*Finds the position to put the new node*/
 while (tmp && tmp->power>power)
  {
   tmpbefore=tmp;
   tmp=tmp->next;
  }
 if (tmp)/*If the new element has a higher power than at least one of the the existing powers*/
  {
   if (tmp->power==power)/*If the power already exists return from function*/
    {
     safe_free(result);
     return;
    }
   result->next=tmp;
   if (tmpbefore)
     tmpbefore->next=result;
   else
     p->start=result;
  }
 else
  {
   p->end->next=result;
   p->end=result;
  }
}

Polynomptr read_polynom(char* st)
{
	/*!Last Changed 19.01.2005*/
 int i = 0,power = 0,j = 0, type = 0, len, point = 0;
	double coefficient = 0, multiplier;
 Polynomptr result = create_polynom();
	len = strlen(st);
 while (i < len)
  {
   if (st[i]=='x')
    {
     j = 1;
					point = 0;
     if (coefficient==0)
       coefficient = 1;
    }
   else
     if (st[i]=='+' || st[i]=='-')
      {
       if (i == 0)
							 {
								 if (st[i]== '-')
           type = 1;
         i++;
								 continue;
							 }
       if (j==1 && power==0)
         power = 1;
       if (type == 1)
         coefficient*=-1;
       add_polynode_to_polynom(result,power,coefficient);
       if (st[i] == '+')
         type = 0;
       else
         type = 1;
       power = 0;
       coefficient = 0;
       j = 0;
							point = 0;
      }
     else
       if (st[i] >= '0' && st[i] <= '9')
         if (j == 0)
										 if (point == 0)
             coefficient = coefficient * 10 + st[i] - '0';
											else
											 {
												 coefficient = coefficient + (st[i] - '0') * multiplier;
												 multiplier /= 10;
											 }
         else
           power = power * 10 + st[i] - '0';
							else
								 if (st[i] == '.')
									 {
										 point = 1;
											multiplier = 0.1;
									 }
   i++;
  }
 if (type == 1)
   coefficient*=-1;
	if (j == 1 && power == 0)
		 power = 1;
 if (power!=0)
   add_polynode_to_polynom(result,power,coefficient);
 else
   add_polynode_to_polynom(result,j,coefficient);
 return result;
}

Polynomptr polyfit(double** data, int datasize, int degree)
{
	/*!Last Changed 21.01.2005*/
 double pow1, pow2;
 int i, j, k;
 matrix A, w, y;
 Polynomptr p;
 A = matrix_alloc(degree + 1, degree + 1);
 y = matrix_alloc(degree + 1, 1);
 for (k = 0; k < datasize; k++)
  {
   pow1 = 1;
   for (i = 0; i < degree + 1; i++)
    {
     pow2 = pow1;
     for (j = 0; j < degree + 1; j++)
      {
       A.values[i][j] += pow2;                              /*fpow(inp[count-1],i+j);*/
       pow2 *= data[k][0];
      }
     y.values[i][0] += data[k][1] * pow1;                  /*out[count-1]*fpow(inp[count-1],i);*/
     pow1 *= data[k][0];
    }
  }
 p = create_polynom();
 matrix_inverse(&A);
 w = matrix_multiply(A,y);
 for (i = 0; i < degree + 1; i++)
   add_polynode_to_polynom(p, i, w.values[i][0]);
 matrix_free(A);
 matrix_free(w);
 matrix_free(y);
 return p;
}

Polynomptr polyfitfile(char* fname, int degree, double* error)
{
	/*!Last Changed 22.01.2005 added read_polydata*/
	/*!Last Changed 21.01.2004 rewritten*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 double **data;
 int datacount;
 Polynomptr p;
 data = readpolydata(fname, &datacount);
 p = polyfit(data, datacount, degree);
 (*error) = calculate_msefile(fname, p);
	free_2d((void**)data, datacount);
 return p;
}

Polynomptr polyfitclsfile(char* fname, int degree, double* error)
{
	/*!Last Changed 26.01.2005*/
 double **data;
 int datacount;
 Polynomptr p;
 data = readpolydata(fname, &datacount);
 p = polyfitcls(data, datacount, degree);
 (*error) = calculate_poly_clserror(data, datacount, p);
	free_2d((void**)data, datacount);
 return p;
}

double** polydatagenerate(int datasize, double start, double end, char* polynomial, int noisy)
{
	/*!Last Changed 21.01.2005 rewritten*/
 int i;
 double x, value, **values;
 Polynomptr p = read_polynom(polynomial);
	values = (double **) safemalloc_2d(sizeof(double *), sizeof(double), datasize, 2, "polydatagenerate", 4);
 for (i = 0; i < datasize; i++)
  {
   x = produce_random_value(start, end);
   value = polynom_value(p, x);
   if (noisy)
     value = value + produce_normal_value(0, get_parameter_f("noiselevel"));
   values[i][0] = x;
   values[i][1] = value;
  }
	return values;
}

void savepolydata(char* fname, double** polydata, int datasize)
{
	/*!Last Changed 21.01.2005*/
 FILE* outfile;
	int i;
	char pst[READLINELENGTH];
 outfile = fopen(fname,"w");
 if (outfile == NULL)
   return;
 for (i = 0; i < datasize; i++)
	 {
			set_precision(pst, NULL, NULL);
			fprintf(outfile, pst, polydata[i][0]);
			set_precision(pst, " ", "\n");
		 fprintf(outfile, pst, polydata[i][1]);
	 }
 fclose(outfile);
}

Polynomptr polyfitcls(double** data, int datasize, int degree)
{
	/*!Last Changed 26.01.2005*/
	int i, j, *array;
	double random_value, o, y, eta, *deltaw, update;
	Polynomptr p = create_polynom();
	Polynodeptr node;
 for (i = 0; i <= degree; i++)
	 {
		 random_value = produce_random_value(-0.01, 0.01);
		 add_polynode_to_polynom(p, i, random_value);
	 }
	eta = get_parameter_f("learning_rate");
	deltaw = safecalloc((degree + 1), sizeof(double), "polyfitcls", 11);
	for (j = 0; j < get_parameter_i("perceptronrun"); j++)
	 {
			array = random_array(datasize);
			for (i = 0; i < datasize; i++)
			 {
				 o = polynom_value(p, data[array[i]][0]);
					y = sigmoid(o);
     node = p->start;
					while (node)
					 {
						 update = (eta * (data[array[i]][1] - y) * fpow(data[array[i]][0], node->power) + get_parameter_f("alpha") * deltaw[node->power]);
						 node->coefficient += update;
							deltaw[node->power] = update;
						 node = node->next;
					 }
			 }
			safe_free(array);
			eta *= get_parameter_f("etadecrease");
	 }
	safe_free(deltaw);
 return p;
}

Polynomptr polylearn(char* trainfname, char* testfname, int maxdegree, double* trainerror, double* testerror, int regression)
{
	/*!Last Changed 22.01.2005 rewritten*/
	/*!Last Changed 02.02.2004 added safemalloc*/
 char varname[MAXLENGTH];
 char varvalue[MAXLENGTH], pst[READLINELENGTH];
	FILE* errorfile;
 Polynomptr p;
	double** traindata, **testdata, bestgeneralization = +LONG_MAX, error, generalizationerror, calcsigma;
	double*** trainfolds, ***testfolds, loglikelihood;
	char fname[20], **fnames, line[READLINELENGTH];
	int traincount, testcount, i, bestdegree = -1, d, j, k, *trainortest;
 int foldcount, foldtraincount, dummy1, dummy2, *ordering;
	traindata = readpolydata(trainfname, &traincount);
	testdata = readpolydata(testfname, &testcount);
	if (get_parameter_l("modelselectionmethod") != 0)
	 {
   for (i = 0; i <= maxdegree; i++)
			 {
				 if (regression)
					 {
  	  	 p = polyfit(traindata, traincount, i);
							error = calculate_mse(traindata, traincount, p);
					 }
					else
					 {
						 p = polyfitcls(traindata, traincount, i);
							error = calculate_poly_clserror(traindata, traincount, p);
							loglikelihood = calculate_poly_loglikelihood(traindata, traincount, p);
					 }
		  	d = i + 1;
			  switch (get_parameter_l("modelselectionmethod"))
					 {
			  		case MODEL_SELECTION_AIC:if (regression)
															                   if (get_parameter_f("sigma") <= 0.0)
																                   {
								                            generalizationerror = aic_squared_error(error, d, traincount);
															  	                  calcsigma = (error * traincount) / (traincount - d);
	                                   sprintf(varname,"aout1[%d]", d);
			                                 set_precision(pst, NULL, NULL);
	                                   sprintf(varvalue, pst, calcsigma);
	                                   set_variable(varname, varvalue);
																                   }
																                  else
																	                   generalizationerror = aic_squared_error_with_sigma(error, d, traincount, get_parameter_f("sigma"));
								                        else
															                   generalizationerror = aic_loglikelihood(loglikelihood, d);
														                  break;
			  		case MODEL_SELECTION_BIC:if (regression)
															                   if (get_parameter_f("sigma") <= 0.0)
																                   {
														  	                   generalizationerror = bic_squared_error(error, d, traincount);
														  		                  calcsigma = (error * traincount) / (traincount - d);
	                                   sprintf(varname,"aout1[%d]", d);
			                                 set_precision(pst, NULL, NULL);
	                                   sprintf(varvalue, pst, calcsigma);
	                                   set_variable(varname, varvalue);
																                   }
																                  else
																	                   generalizationerror = bic_squared_error_with_sigma(error, d, traincount, get_parameter_f("sigma"));
								                        else
															                   generalizationerror = bic_loglikelihood(loglikelihood, d, traincount);
								                        break;
			  		case MODEL_SELECTION_SRM:if (regression) 
															                   generalizationerror = srm_regression(error, d, traincount, 1.0, 1.0);
								                        else
															                   generalizationerror = srm_classification(error, d, traincount, 1.0, 1.0);
														                  break;
				  	case MODEL_SELECTION_MDL:if (regression)
  								                        generalizationerror = mdl(error, d, 1.0 / traincount, 6);
								                        else
															                   generalizationerror = mdl(error, d, 0.01, 6);
														                  break;
					 }
					if (generalizationerror < bestgeneralization)
					 {
						 bestdegree = i;
       bestgeneralization = generalizationerror;
					 }
			  free_polynom(p);
			 }
	 }
	else
	 {
		 if (traincount < 50) /*Leave one out*/
			 {
				 foldcount = traincount;
					foldtraincount = traincount - 1;
				 trainfolds = (double***) safemalloc_3d(sizeof(double**), sizeof(double*), sizeof(double), traincount, traincount - 1, 2, "polylearn", 37);
					testfolds = (double***) safemalloc_3d(sizeof(double**), sizeof(double*), sizeof(double), traincount, 1, 2, "polylearn", 38);
					for (i = 0; i < traincount; i++)
					 { /*Prepare fold i*/
							k = 0;
							for (j = 0; j < traincount; j++)
								 if (i != j)
									 {
										 trainfolds[i][k][0] = traindata[j][0];
										 trainfolds[i][k][1] = traindata[j][1];
											k++;
									 }
									else
									 {
										 testfolds[i][0][0] = traindata[j][0];
										 testfolds[i][0][1] = traindata[j][1];
									 }       
					 }
			 }
			else /*5x2 Cross validation*/
			 {
				 foldcount = 10;
					foldtraincount = traincount / 2;
				 trainfolds = (double***) safemalloc_3d(sizeof(double**), sizeof(double*), sizeof(double), 10, traincount / 2, 2, "polylearn", 63);
					testfolds = (double***) safemalloc_3d(sizeof(double**), sizeof(double*), sizeof(double), 10, traincount / 2, 2, "polylearn", 64);
					for (i = 0; i < 5; i++)
					 {
       trainortest = random_array(traincount);
							k = 0;
       for (j = 0; j < traincount; j++)
								 if (trainortest[j] % 2 == 0)
									 {
										 trainfolds[2 * i][k][0] = traindata[j][0];
										 trainfolds[2 * i][k][1] = traindata[j][1];
											testfolds[2 * i + 1][k][0] = traindata[j][0];
											testfolds[2 * i + 1][k][1] = traindata[j][1];
											k++;
									 }
									else
									 {
										 trainfolds[2 * i + 1][j - k][0] = traindata[j][0];
										 trainfolds[2 * i + 1][j - k][1] = traindata[j][1];
											testfolds[2 * i][j - k][0] = traindata[j][0];
											testfolds[2 * i][j - k][1] = traindata[j][1];										 
									 }
					  safe_free(trainortest);
					 }
			 }
			fnames = safemalloc((maxdegree + 1) * sizeof(char *), "polylearn", 138); 
   for (i = 0; i <= maxdegree; i++)
			 { 
				 sprintf(fname, "%s/error%d.txt", get_parameter_s("tempdirectory"), i);
					fnames[i] = strcopy(fnames[i], fname);
				 errorfile = fopen(fname, "w");
					if (!errorfile)
						 return NULL;
				 for (j = 0; j < foldcount; j++)
					 {
						 if (regression)
							 {
  						 p = polyfit(trainfolds[j], foldtraincount, i);
							  error = calculate_mse(testfolds[j], traincount - foldtraincount, p);
							 }
							else
							 {
								 p = polyfitcls(trainfolds[j], foldtraincount, i);
									error = calculate_poly_clserror(testfolds[j], traincount - foldtraincount, p);
							 }
			    set_precision(pst, NULL, "\n");
							fprintf(errorfile, pst, error);
							free_polynom(p);
					 }
					fclose(errorfile);
 			}
			free_3d((void***)trainfolds, foldcount, foldtraincount);
			free_3d((void***)testfolds, foldcount, traincount - foldtraincount);
			sprintf(line, "-");
			ordering = multiple_comparison(fnames, maxdegree + 1, get_parameter_l("testtype"), line, &dummy1, &dummy2, get_parameter_l("correctiontype"));
			bestdegree = ordering[0] - 1;
			safe_free(ordering);
   free_2d((void**)fnames, maxdegree + 1);
	 }
	if (regression)
	 {
  	p = polyfit(traindata, traincount, bestdegree);
	  *trainerror = calculate_mse(traindata, traincount, p);
	  *testerror = calculate_mse(testdata, testcount, p);
	 }
 else
	 {
		 p = polyfitcls(traindata, traincount, bestdegree);
			*trainerror = calculate_poly_clserror(traindata, traincount, p);
			*testerror = calculate_poly_clserror(testdata, testcount, p);
	 }
	free_2d((void**)traindata, traincount);
	free_2d((void**)testdata, testcount);
 return p;
}

int polylearnvalidation(char* definitionfile, int maxdegree, int regression)
{
 /*!Last Changed 23.01.2005*/
	char polyst[MAXLENGTH];
 char varname[MAXLENGTH];
 char varvalue[MAXLENGTH], pst[READLINELENGTH];
	double** traindata, **testdata, besterror = +LONG_MAX, beststdev, trainerror, *testerror, *mean, *stdev, bayeserror;
	int i, bestdegree, functioncount, j, traincount, testcount;
	Functionptr fx;
	Mixtureptr mix;
	Polynomptr p;
	if (regression)
		 fx = read_function_definition(definitionfile, &functioncount);
	else
		 mix = read_mixture_from_file(definitionfile);
	testerror = safemalloc(100 * sizeof(double), "polylearnvalidation", 14);
	mean = safemalloc(maxdegree * sizeof(double), "polylearnvalidation", 15);
	stdev = safemalloc(maxdegree * sizeof(double), "polylearnvalidation", 16);
	traincount = 100;
	testcount = 1000;
	if (!regression)
	 {
   testdata = generate_data_from_mixture_model(mix, testcount);
			bayeserror = 0.0;
			for (i = 0; i < testcount; i++)
				 if (mixture_class_at_value(mix, testdata[i][0]) != testdata[i][1])
						 bayeserror++;
			set_default_variable("out2", bayeserror / testcount);
	 }
	for (i = 1; i <= maxdegree; i++)
	 {
		 trainerror = 0;
		 for (j = 0; j < 100; j++)
			 {
				 if (regression)
					 {
						 traindata = functiondatagenerate(traincount, fx, functioncount, 1);
							testdata = functiondatagenerate(testcount, fx, functioncount, 1);
		     p = polyfit(traindata, traincount, i);
					 }
					else
					 {
						 traindata = generate_data_from_mixture_model(mix, traincount);
						 testdata = generate_data_from_mixture_model(mix, testcount);
		     p = polyfitcls(traindata, traincount, i);
					 }
					if (j == 0)
					 {
    			print_polynom(p, polyst);
      	sprintf(varname,"aout3[%d]", i);
	    		set_variable(varname, polyst);
					 }
					if (regression)
					 {
			    trainerror += calculate_mse(traindata, traincount, p);
  	  		testerror[j] = calculate_mse(testdata, testcount, p);
					 }
					else
					 {
						 trainerror += calculate_poly_clserror(traindata, traincount, p);
							testerror[j] = calculate_poly_clserror(testdata, testcount, p);
					 }
			  free_polynom(p);
					free_2d((void**)traindata, traincount);
					free_2d((void**)testdata, testcount);
			 }
  	sprintf(varname,"aout1[%d]", i);
			set_precision(pst, NULL, NULL);
  	sprintf(varvalue, pst, trainerror / traincount);
  	set_variable(varname, varvalue); 
			mean[i - 1] = find_mean_stdev_of_list(testerror, 100, &(stdev[i - 1]));
			if (mean[i - 1] < besterror)
			 {
				 besterror = mean[i - 1];
					beststdev = stdev[i - 1];
			 }
  	sprintf(varname,"aout2[%d]", i);
			set_precision(pst, NULL, NULL);
  	sprintf(varvalue, pst, mean[i - 1]);
  	set_variable(varname, varvalue); 
  	sprintf(varname,"aout4[%d]", i);
			set_precision(pst, NULL, NULL);
  	sprintf(varvalue, pst, stdev[i - 1]);
  	set_variable(varname, varvalue); 
	 }
	bestdegree = 0;
	while (mean[bestdegree] - stdev[bestdegree] > besterror + beststdev)
		 bestdegree++;
	safe_free(mean);
	safe_free(stdev);
 safe_free(testerror);
	if (regression)
	 {
  	for (i = 0;i < functioncount; i++)
	  	 free_function(fx[i]);
	  safe_free(fx);
	 }
	else
		 free_mixture(mix);
	return bestdegree;
}

void bias_variance_polynom(char* definitionfile, int maxdegree, int M, int N, int regression)
{
 /*!Last Changed 23.01.2005*/
 /*!Last Changed 21.01.2005*/
 char varname[MAXLENGTH];
 char varvalue[MAXLENGTH];
 char polyst[MAXLENGTH], pst[READLINELENGTH];
 int functioncount, i, degree, j, bestdegree = 0, polynomindex;
 Functionptr fx;
 Mixtureptr mix;
 double ***samples, bias, variance, gxt, fxt, besterror = +LONG_MAX, error, **testsample, *ghat, sigmo;
 Polynomptr *g, gmean;
 if (regression)
  {
   fx = read_function_definition(definitionfile, &functioncount);
   if (!fx)
     return;
  }
 else
   mix = read_mixture_from_file(definitionfile);
 samples = safemalloc(M * sizeof(double **), "bias_variance_polynom", 17);
 g = safemalloc(M * sizeof(Polynomptr), "bias_variance_polynom", 18);
 if (regression)
  {
   for (i = 0; i < M; i++)
     samples[i] = functiondatagenerate(N, fx, functioncount, 1);
   testsample = functiondatagenerate(1000, fx, functioncount, 1);
  }
 else
  {
   for (i = 0; i < M; i++)
     samples[i] = generate_data_from_mixture_model(mix, N);
   testsample = generate_data_from_mixture_model(mix, 1000);
  }
 for (degree = 1; degree <= maxdegree; degree++)
  {
   ghat = safecalloc(1000, sizeof(double), "bias_variance_polynom", 33);
   for (i = 0; i < M; i++)
    {
     if (regression)
       g[i] = polyfit(samples[i], N, degree);
     else
       g[i] = polyfitcls(samples[i], N, degree);
     for (j = 0; j < 1000; j++)
       ghat[j] += polynom_value(g[i], testsample[j][0]);
     polynomindex = maxdegree + (degree - 1) * M + i + 1;
     if (polynomindex < 256)
      {
       print_polynom(g[i], polyst);
       sprintf(varname,"aout3[%d]", polynomindex);
       set_variable(varname, polyst); 
      }
    }
   gmean = mean_of_polynoms(g, M);
   print_polynom(gmean, polyst);
   sprintf(varname,"aout3[%d]", degree);
   set_variable(varname, polyst);
   for (j = 0; j < 1000; j++)
     ghat[j] /= M;
   /*Calculate bias*/
   bias = 0; 
   for (j = 0; j < 1000; j++)
     if (regression)
      {
       fxt = function_value(fx, functioncount, testsample[j][0]);
       bias += (ghat[j] - fxt) * (ghat[j] - fxt);
      }
     else
      {
       fxt = mixture_value(mix, testsample[j][0], (int) (testsample[j][1]));
       if (testsample[j][1] == 0)
         sigmo = 1 - sigmoid(ghat[j]);
       else
         sigmo = sigmoid(ghat[j]);
       bias += (sigmo - fxt) * (sigmo - fxt);
      }
   bias /= 1000;
   sprintf(varname,"aout1[%d]", degree);
   set_precision(pst, NULL, NULL);
   sprintf(varvalue, pst, bias);
   set_variable(varname, varvalue); 
   /*Calculate variance*/
   variance = 0;
   for (i = 0; i < M; i++)
     for (j = 0; j < 1000; j++)
      {
       gxt = polynom_value(g[i], testsample[j][0]);
       if (regression)
         variance += (gxt - ghat[j]) * (gxt - ghat[j]);
       else
        {
         sigmo = sigmoid(gxt) - sigmoid(ghat[j]);
         variance += sigmo * sigmo;
        }
      }
   variance /= (M * 1000);
   sprintf(varname,"aout2[%d]", degree);
   set_precision(pst, NULL, NULL);
   sprintf(varvalue, pst, variance);
   set_variable(varname, varvalue); 
   free_polynom(gmean);
   safe_free(ghat);
   for (i = 0; i < M; i++)
     free_polynom(g[i]);
   error = bias + variance;
   if (error < besterror)
    {
     besterror = error;
     bestdegree = degree;
    }
  }
 set_default_variable("out1", bestdegree + 0.0);
 if (regression)
  {
   for (i = 0;i < functioncount; i++)
     free_function(fx[i]);
   safe_free(fx);
  }
 else
   free_mixture(mix);
 free_3d((void***)samples, M, N);
 free_2d((void**)testsample, 1000);
 safe_free(g);
}
