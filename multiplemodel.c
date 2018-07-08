#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "instance.h"
#include "instancelist.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "messages.h"
#include "model.h"
#include "multiplemodel.h"
#include "regressiontree.h"
#include "statistics.h"

extern Datasetptr current_dataset;

/**
 * Given the values of the features, this function calculates the function value of a multiplicative term. A multiplicative term may be a constant such as 2.4 or a power function such as x1^5 (power to the 5 of the first feature) or sinus, cosinus functions (to the power also) such as sin^4(...).
 * @param[in] term The multiplicative term
 * @param[in] featurevalues Values of the features
 * @return Functional value of a multiplicative term. If the function is not defined, the function returns 1.
 */
double multiplicative_term_value(Mtermptr term, double* featurevalues)
{
	/*!Last Changed 11.04.2005*/
	double value;
	switch (term->functiontype)
	 {
	  case FUNC_CONSTANT:return term->value;
				                  break;
			case FUNC_POWER   :value = featurevalues[term->featureindex - 1];
				                  if (term->power == 1)
											             return value; 
				                  else
											             return fpow(value, term->power);
										            break;
			case FUNC_SINUS   :
			case FUNC_COSINUS :value = additive_term_value(term->interm, featurevalues);
				                  if (term->functiontype == FUNC_SINUS)
											             value = sin(value);
										            else
											             value = cos(value);
										            if (term->power == 1)
											             return value;
										            else
											             return fpow(value, term->power);
										            break;
   default           :printf(m1235, term->functiontype);
                      break;
	 }
	return 1;
}

/**
 * Given the values of the features, this function calculates the functional value of an additive term using the multiplicative term values. An additive term consists of one or more multiplicative terms multipled together such as 2.4sin^2(..)cos(...). The function also considers the sign of the operation.
 * @param[in] term The additive term
 * @param[in] featurevalues Values of the features
 * @return Functional value of an additive term. If there are no terms, returns 1 or -1 according to the sign.
 */
double additive_term_value(Atermptr term, double* featurevalues)
{
	/*!Last Changed 11.04.2005*/
	Mtermptr tmp;
	double value = 1;
	tmp = term->start;
	while (tmp)
	 {
		 value *= multiplicative_term_value(tmp, featurevalues);
		 tmp = tmp->next;
	 }
	return value * term->sign;
}

/**
 * Given the values of the features, this function calculates a single model's value via adding the additive terms of the single model using additive_term_value function. An example model can be 2.4sin(x3)+cos^2(x1)-x1^5x2^4, where the additive terms are 2.4sin(x3), cos^2(x1) and x1^5x2^4. The multiplicative terms are 2.4, sin(x3) for the first additive term, cos^2(x1) for the second additive term and x1^5 and x2^4 for the third additive term.
 * @param[in] m Single model
 * @param[in] featurevalues Values of the features
 * @return The value of the single model
 */
double single_model_value(Singlemodel m, double* featurevalues)
{
	/*!Last Changed 11.04.2005*/
	Atermptr tmp;
	double value = 0, noise;
	tmp = m.start;
	while (tmp)
	 {
		 value += additive_term_value(tmp, featurevalues);
		 tmp = tmp->next;
	 }
	noise = produce_normal_value(0, m.noiselevel);
	return value + noise;
}

/**
 * Extracts the multiplicative terms from an additive term. For example if the additive term is x1^5x2^4, then the function will return two items as x1^5 and x2^4. If the additive term is 2.4sin(x3)cos^2(x1), then the function will return three items as 2.4, sin(x3) and cos^2(x1).
 * @param[in] st String containing the additive term
 * @param[out] count Number of multiplicative terms
 * @return An array of multiplicative terms stored as a string array
 */
char** extract_multiplicative_terms(char* st, int* count)
{
	/*!Last Changed 10.04.2005*/
	int i, len = strlen(st), parcount = 0;
	char** result = NULL;
	char tempst[READLINELENGTH];
	*count = 0;
	strcpy(tempst, "");
	for (i = 0; i < len; i++)
		 switch (st[i])
	   {
		   case 'x':
					case 'c':
					case 's':if (parcount == 0 && i != 0)															 
														 {
						          (*count)++;
		              result = alloc(result, *count * sizeof(char*), *count);
										     	result[*count - 1] = strcopy(result[*count - 1], tempst);
																sprintf(tempst, "%c", st[i]);
														 }
						        else
															 sprintf(tempst, "%s%c", tempst, st[i]);
														break;
					case '(':parcount++;
              sprintf(tempst, "%s%c", tempst, st[i]);
						        break;
					case ')':parcount--;
						        sprintf(tempst, "%s%c", tempst, st[i]);
						        break;
					default :sprintf(tempst, "%s%c", tempst, st[i]);
		 				       break;
    }
	(*count)++;
	result = alloc(result, *count * sizeof(char*), *count);
	result[*count - 1] = strcopy(result[*count - 1], tempst);
	return result;
}

/**
 * Extracts the additive terms from a string of expression. For example if the expression is x1^5x2^4+2.4sin(x3)cos^2(x1), then the function will return two items as x1^5x2^4 and 2.4sin(x3)cos^2(x1). If the expression is 2.4sin(x3)+cos^2(x1)-x1^5x2^4, then the function will return three items as 2.4sin(x3), cos^2(x1) and -x1^5x2^4.
 * @param[in] st String containing the expression
 * @param[out] count Number of additive terms
 * @return An array of additive terms stored as a string array
 */
char** extract_additive_terms(char* st, int* count)
{
	/*!Last Changed 10.04.2005*/
	int i, len = strlen(st), parcount = 0;
	char tempst[READLINELENGTH];
	char** result = NULL;
	*count = 0;
	strcpy(tempst, "");
	for (i = 0; i < len; i++)
		 switch (st[i])
	   {
		   case '+':
					case '-':if (parcount == 0 && i != 0)
														 { 
						          (*count)++;
		              result = alloc(result, *count * sizeof(char*), *count);
										     	result[*count - 1] = strcopy(result[*count - 1], tempst);
																sprintf(tempst, "%c", st[i]);
														 }
			           else
									       sprintf(tempst, "%s%c", tempst, st[i]);
			           break;
					case '(':parcount++;
						        sprintf(tempst, "%s%c", tempst, st[i]);
			           break;
					case ')':parcount--;
						        sprintf(tempst, "%s%c", tempst, st[i]);
		            break;
					default :sprintf(tempst, "%s%c", tempst, st[i]);
		 				       break;
	   }
	(*count)++;
	result = alloc(result, *count * sizeof(char*), *count);
	result[*count - 1] = strcopy(result[*count - 1], tempst);
	return result;
}

/**
 * Extracts the power and the feature index from a string. For example if the string is x1^4 then the index is 1 and the power is 4. If the string is x3 then the index is 3 and the power is 1.
 * @param[in] st String containing the expression
 * @param[out] power Exponent term of the feature
 * @return Index of the feature
 */
int index_and_power_from_string(char* st, int* power)
{
	/*!Last Changed 10.04.2005*/
	int i, len = strlen(st), pos = 0, index;
	char tmp[READLINELENGTH];
	strcpy(tmp, "");
	for (i = 1; i < len; i++)
		 if (st[i] == '^')
			 {
				 pos = 1;
     index = atoi(tmp);
	    strcpy(tmp, ""); 
			 }
			else
					sprintf(tmp, "%s%c", tmp, st[i]);
	if (pos == 0)
	 {
		 index = atoi(tmp);
			*power = 1;
	 }
	else
		 *power = atoi(tmp);
	return index;
}

/**
 * Extracts the term and the power of an sinus or cosinus expression. For example if the expression is cos^3(3x1), then the power is 3 and the output string is 3x1. If the expression is sin(6x2^4) then the power is 1 and the output string is 6x2^4.
 * @param[in] st String containing the expression without sinus or cosinus. For the above examples, 3(3x1) and (6x2^4).
 * @param[out] power Exponent term of the expression
 * @return String containing the term without exponent part
 */
char* interm_and_power_from_string(char* st, int* power)
{
	/*!Last Changed 10.04.2005*/
	char* result = NULL, tmp[READLINELENGTH];
	int i, len = strlen(st);
	strcpy(tmp, "");
	for (i = 0; i < len; i++)
		 if (st[i] == '(')
			 {
				 if (strlen(tmp) != 0)
  				 *power = atoi(tmp);
					else
					  *power = 1;
    	strcpy(tmp, "");
			 }
			else
			  if (st[i] != ')')
						 sprintf(tmp, "%s%c", tmp, st[i]);
	result = strcopy(result, tmp);
	return result;
}

/**
 * Constructor for multiplicative term. Generates a multiplicative term structure (allocates and fills the structure) from a string of expression such as cos^2(x1) or sin(3x1) or 1.4. Since a multiplicative term may contain additive term inside of it such as x1 and 3x2^3, the function also calls read_additive_term_from_string function.
 * @param[in] st String containing the multiplicative term
 * @return Multiplicative term structure
 */
Mtermptr read_multiplicative_term_from_string(char* st)
{
	/*!Last Changed 10.04.2005*/
	Mtermptr result;
	char* termst;
	result = safemalloc(sizeof(Mterm), "read_multiplicative_term_from_string", 3);
	result->next = NULL;
	switch (st[0])
	 {
	  case 'x':result->functiontype = FUNC_POWER;
				        result->featureindex = index_and_power_from_string(&(st[1]), &(result->power));
												result->interm = NULL;
				        break;
			case 's':
			case 'c':if (st[0] == 's')
  				        result->functiontype = FUNC_SINUS;
				        else 
													 result->functiontype = FUNC_COSINUS;
												termst = interm_and_power_from_string(&(st[1]), &(result->power)); 
												result->interm = read_additive_term_from_string(termst);
												safe_free(termst);
				        break;
			default :result->functiontype = FUNC_CONSTANT;
				        result->value = atof(st);
												result->interm = NULL;
				        break;
	 }
	return result;
}

/**
 * Constructor for additive term. Generates an additive term structure (allocates and fills the structure) from a string of expression such as x1^5x2^4 or 2.4sin(x3)cos^2(x1). It uses extract_multiplicative_terms function for this purpose.
 * @param[in] st String containing the expression
 * @return Additive term structure
 */
Atermptr read_additive_term_from_string(char* st)
{
	/*!Last Changed 10.04.2005*/
	Atermptr result;
	Mtermptr tmp, before;
	char** terms;
	int mulcount, i;
	result = safemalloc(sizeof(Aterm), "read_additive_term_from_string", 5);
	result->next = NULL;
	if (st[0] == '-')
		 result->sign = -1;
 else
		 result->sign = 1;
	if (st[0] == '+' || st[0] == '-')
	  terms = extract_multiplicative_terms(&(st[1]), &mulcount);
	else
   terms = extract_multiplicative_terms(st, &mulcount);
 result->start = read_multiplicative_term_from_string(terms[0]);
	before = result->start;
	for (i = 1; i < mulcount; i++)
	 {
		 tmp = read_multiplicative_term_from_string(terms[i]);
			before->next = tmp;
			before = tmp;
	 }
	free_2d((void**)terms, mulcount);
	return result;
}

/**
 * Generates a single model from a mathematical expression such as x1^5x2^4+2.4sin(x3)cos^2(x1) or 2.4sin(x3)+cos^2(x1)-x1^5x2^4.
 * @param[out] s Output single model
 * @param[in] modelst String containing the expression
 */
void read_single_model_from_string(Singlemodelptr s, char* modelst)
{
	/*!Last Changed 10.04.2005*/
	int addcount, i;
	char** terms;
	Atermptr tmp, before;
 terms = extract_additive_terms(modelst, &addcount);
	s->start = read_additive_term_from_string(terms[0]);
	before = s->start;
 for (i = 1; i < addcount; i++)
	 {
		 tmp = read_additive_term_from_string(terms[i]);
			before->next = tmp;
			before = tmp;
	 }
	free_2d((void**)terms, addcount);
}

/**
 * Reads and generates multiple model from an input file. Multiple model file has the following format: \n
 * Number of features (d) \n
 * Minimum value of feature1 Maximum value of feature 1 \n
 * Minimum value of feature2 Maximum value of feature 2 \n
 * ... \n
 * Minimum value of featured Maximum value of feature d \n 
 * Number of models (m) \n
 * Data count for model 1 Noise level for model 1 \n
 * Model 1 expression \n
 * Data count for model 2 Noise level for model 2 \n
 * Model 2 expression \n
 * ... \n
 * ... \n
 * Data count for model m Noise level for model m \n
 * Model m expression
 * @param[in] modelfile Input file containing multiple model information
 * @return Multiple model structure allocated and filled
 */
Multiplemodelptr read_multiple_model_from_file(char* modelfile)
{
	/*!Last Changed 10.04.2005*/
	FILE* infile;
	int i;
	char modelst[READLINELENGTH];
	Multiplemodelptr result;
 infile = fopen(modelfile, "r");
	if (!infile)
		 return NULL;
	result = safemalloc(sizeof(Multiplemodel), "read_multiple_model_from_file", 8);
	fscanf(infile, "%d", &(result->featurecount));
	result->min = safemalloc(result->featurecount * sizeof(double), "read_multiple_model_from_file", 10);
	result->max = safemalloc(result->featurecount * sizeof(double), "read_multiple_model_from_file", 11);
	for (i = 0; i < result->featurecount; i++)
		 fscanf(infile, "%lf%lf", &(result->min[i]), &(result->max[i]));
	fscanf(infile, "%d", &(result->modelcount));
	result->models = safemalloc(result->modelcount * sizeof(Singlemodel), "read_multiple_model_from_file", 15);
	for (i = 0; i < result->modelcount; i++)
	 {
		 fscanf(infile, "%d%lf", &(result->models[i].datacount), &(result->models[i].noiselevel));
			fscanf(infile, "%s", modelst);
   read_single_model_from_string(&(result->models[i]), modelst);
	 }
	fclose(infile);
	return result;
}

/**
 * Destructor function for multiplicative term. Deallocates memory allocated for a multiplicative term structure
 * @param[in] m Multiplicative term structure
 */
void free_multiplicative_term(Mtermptr m)
{
	/*!Last Changed 11.04.2005*/
	if (m->functiontype > 1)
	 {
		 free_additive_term(m->interm);
			safe_free(m->interm);
	 }
}

/**
 * Destructor function for additive term. Deallocates memory allocated for an additive term structure
 * @param[in] a Additive term structure
 */
void free_additive_term(Atermptr a)
{
	/*!Last Changed 11.04.2005*/
	Mtermptr tmp, next;
	tmp = a->start;
	while (tmp)
	 {
		 next = tmp->next;
			free_multiplicative_term(tmp);
			safe_free(tmp);
			tmp = next;
	 }
}

/**
 * Destructor function for a single model. Deallocates memory allocated for a single model.
 * @param[in] s Single model structure
 */
void free_single_model(Singlemodel s)
{
	/*!Last Changed 11.04.2005*/
	Atermptr tmp, next;
 tmp = s.start;
	while (tmp)
	 {
		 next = tmp->next;
			free_additive_term(tmp);
			safe_free(tmp);
			tmp = next;
	 }
}

/**
 * Destructor function for a multiple model structure. Deallocates memory allocated for a multiple model.
 * @param[in] m Multiple model structure
 */
void free_multiple_model(Multiplemodelptr m)
{
	/*!Last Changed 11.04.2005*/
	int i;
	safe_free(m->min);
	safe_free(m->max);
	for (i = 0; i < m->modelcount; i++)
		 free_single_model(m->models[i]);
	safe_free(m->models);
	safe_free(m);
}

/**
 * Total number of data points that must be generated for a multiple model.
 * @param[in] m Multiple model structure
 * @return Number of data points to be generated
 */
int data_count_of_multiple_model(Multiplemodelptr m)
{
	/*!Last Changed 11.04.2005*/
	int result = 0, i;
	for (i = 0; i < m->modelcount; i++)
		 result += m->models[i].datacount;
	return result;
}

/**
 * Generates random data from a single model. 
 * @param[in] s Single model structure
 * @param[in] featurecount Number of features in the output dataset
 * @param[in] min Minimum values of each feature. Stored as an array. min[i] is the minimum value of the feature i.
 * @param[in] max Maximum values of each feature. Stored as an array. max[i] is the maximum value of the feature i.
 * @return Matrix of random data. Stored in a row major structure. Each row corresponds to a data point.
 */
matrix generate_data_from_single_model(Singlemodel s, int featurecount, double* min, double* max)
{
	/*!Last Changed 11.04.2005*/
	int i, j;
	matrix result;
	result = matrix_alloc(s.datacount, featurecount + 1);
	for (i = 0; i < s.datacount; i++)
	 {
		 for (j = 0; j < featurecount; j++)
				 result.values[i][j] = produce_random_value(min[j], max[j]);
			result.values[i][j] = single_model_value(s, result.values[i]);
	 }
	return result;
}

/**
 * Generates data from a multiple model.
 * @param[in] m Multiple model structure
 * @return Matrix of random data. Stored in a row major structure. Each row corresponds to a data point.
 */
matrix generate_data_from_multiple_model(Multiplemodelptr m)
{
	/*!Last Changed 11.04.2005*/
	matrix result;
	int datacount, i, j, k, p;
 datacount = data_count_of_multiple_model(m);
	result = matrix_alloc(datacount, m->featurecount + 1);
	k = 0;
	for (i = 0; i < m->modelcount; i++)
		 for (j = 0; j < m->models[i].datacount; j++)
			 {
		   for (p = 0; p < m->featurecount; p++)
				   result.values[k][p] = produce_random_value(m->min[p], m->max[p]);
     result.values[k][p] = single_model_value(m->models[i], result.values[k]);
			  k++;
			 }
	return result;
}

void remove_instances_according_to_threshold(Instanceptr* parentlist, Instanceptr* removed, double threshold, matrix w, char which)
{
	/*!Last Changed 11.04.2005*/
	double value;
	Instanceptr last, tmp, before, next;
	last = *removed;
	if (last)
		 while (last->next != NULL)
				 last = last->next;
	tmp = *parentlist;
	before = NULL;
	while (tmp)
	 {
		 next = tmp->next;
		 value = fabs(linear_fit_value(w, tmp) - give_regressionvalue(tmp));
			if ((value > threshold && which == '>') || (value < threshold && which == '<'))
			 {
				 if (last)
  				 last->next = tmp;
					else
						 *removed = tmp;
					last = tmp;
					if (before)
						 before->next = tmp->next;
					else
						 *parentlist = tmp->next;
			 }
			else
				 before = tmp;
			tmp = next;
	 }
	last->next = NULL;
}

int mmelin1(Instanceptr* traindata)
{
	/*!Last Changed 11.04.2005*/
	matrix X, r, w, bestw, *models;
	Instanceptr removed = NULL, modelremoved = NULL, tmp;
	int i, finished = 1, iteration, size, j, mindatasize, modelcount, first;
	double totalerror, *residuals, threshold, upperbound, upperbefore, bestthreshold, ratio, bestratio;
	size = data_size(*traindata);
 mindatasize = size / 20;
	modelcount = 0;
	while (finished)
	 {
		 iteration = size / mindatasize;
			bestratio = 0;
			first = 1;
		 for (i = 0; i < iteration - 2; i++)
			 {
     create_regression_matrices(*traindata, &X, &r);
    	w = multivariate_regression(X, r);
	  		matrix_free(X);
		  	matrix_free(r);
    	size = data_size(*traindata);
					residuals = safemalloc(size * sizeof(double), "mmelin", 20);
					j = 0;
					totalerror = 0;
					tmp = *traindata;
					while (tmp)
					 {
						 residuals[j] = fabs(linear_fit_value(w, tmp) - give_regressionvalue(tmp));
							totalerror += residuals[j] * residuals[j]; 
						 tmp = tmp->next;
							j++;
					 }
					totalerror /= j;
     upperbound = srm_regression(totalerror, current_dataset->multifeaturecount, size, 1.0, 1.0);
					if (i != 0)
						 ratio = upperbefore / upperbound;
					else
						 ratio = 0;
					upperbefore = upperbound;
     if (ratio > bestratio)
					 {
						 if (!first)
								 matrix_free(bestw);
       bestw = matrix_copy(w);
							bestratio = ratio;
							bestthreshold = threshold;
					 }
     qsort(residuals, size, sizeof(double), sort_function_double_ascending);
					threshold = (residuals[size - mindatasize - 1] + residuals[size - mindatasize]) / 2;
     remove_instances_according_to_threshold(traindata, &modelremoved, threshold, w, '>');
					matrix_free(w);
			 }
			merge_instancelist(traindata, modelremoved);
   remove_instances_according_to_threshold(traindata, &removed, bestthreshold, bestw, '<');
			modelremoved = NULL;
			modelcount++;
			models = alloc(models, modelcount * sizeof(matrix), modelcount);
			models[modelcount - 1] = matrix_copy(bestw);
			matrix_free(bestw);
			size = data_size(*traindata);
			if (size <= 2 * mindatasize)
				 finished = 0;
	 }
	merge_instancelist(traindata, removed);
	removed = NULL;
	return 0;
}
