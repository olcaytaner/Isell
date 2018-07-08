#include <stdio.h>
#include <limits.h>
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "mixture.h"
#include "statistics.h"

/**
 * Mixture structure constructor. Reads mixture definition from a file and generates a mixture structure. Mixture file has the following structure: \n
 * #of classes (K) \n
 * Prior probability of class 1 Prior probability of class 2 ... Prior probability of class K \n
 * Number of models of class 1 (M_1) \n
 * Mixture probability of model 1 of class 1 Mixture probability of model 2 of class 1 ...Mixture probability of model M_1 of class 1 \n
 * Mean of model 1 of class 1 Standard Deviation of model 1 of class 1 \n
 * Mean of model 2 of class 1 Standard Deviation of model 2 of class 1 \n
 * ... \n
 * Mean of model M_1 of class 1 Standard Deviation of model M_1 of class 1 \n
 * ... \n
 * ... \n
 * Number of models of class K (M_K) \n
 * Mixture probability of model 1 of class K Mixture probability of model 2 of class K ...Mixture probability of model M_K of class K \n
 * Mean of model 1 of class K Standard Deviation of model 1 of class K \n
 * Mean of model 2 of class K Standard Deviation of model 2 of class K \n
 * ... \n
 * Mean of model M_K of class K Standard Deviation of model M_K of class K
 * @param[in] modeldefinition Mixture file name
 * @return Allocated and filled mixture structure
 */
Mixtureptr read_mixture_from_file(char* modeldefinition)
{
	/*!Last Changed 26.01.2005*/
	int i, j;
	FILE* infile;
	Mixtureptr mix;
	mix = safemalloc(sizeof(Mixture), "read_mixture_from_file", 4);
	infile = fopen(modeldefinition, "r");
	if (!infile)
		 return NULL;
	fscanf(infile, "%d", &(mix->classcount));
	mix->classpriors = safemalloc(mix->classcount * sizeof(double), "read_mixture_from_file", 7);
	for (i = 0; i < mix->classcount; i++)
		 fscanf(infile, "%lf", &(mix->classpriors[i]));
	mix->numberofmodels = safemalloc(mix->classcount * sizeof(int), "read_mixture_from_file", 10);
 mix->mixtureprob = safemalloc(mix->classcount * sizeof(double*), "read_mixture_from_file", 11);
 mix->mixturemeans = safemalloc(mix->classcount * sizeof(double*), "read_mixture_from_file", 12);
 mix->mixturedeviations = safemalloc(mix->classcount * sizeof(double*), "read_mixture_from_file", 13);
 for (i = 0; i < mix->classcount; i++)
	 {
		 fscanf(infile, "%d", &(mix->numberofmodels[i]));
   mix->mixtureprob[i] = safemalloc(mix->numberofmodels[i] * sizeof(double), "read_mixture_from_file", 17);
   mix->mixturemeans[i] = safemalloc(mix->numberofmodels[i] * sizeof(double), "read_mixture_from_file", 18);
   mix->mixturedeviations[i] = safemalloc(mix->numberofmodels[i] * sizeof(double), "read_mixture_from_file", 19);
			for (j = 0; j < mix->numberofmodels[i]; j++)
				 fscanf(infile, "%lf", &(mix->mixtureprob[i][j]));
	 }
	for (i = 0; i < mix->classcount; i++)
		 for (j = 0; j < mix->numberofmodels[i]; j++)
				 fscanf(infile, "%lf%lf", &(mix->mixturemeans[i][j]), &(mix->mixturedeviations[i][j]));
	fclose(infile);
	return mix;
}

/**
 * Finds the class which has the maximum mixture gaussian value at point x. For each class, the mixture gaussian value is calculated using the mixture weights and gaussian values of mixture components. The class with the maximum mixture gaussian value is selected.
 * @param[in] mix Mixture structure
 * @param[in] value x
 * @return Class index
 */
int mixture_class_at_value(Mixtureptr mix, double value)
{
	/*!Last Changed 26.01.2005*/
	int i, j, index;
	double maxprob = -1, prob;
	for (i = 0; i < mix->classcount; i++)
	 {
		 prob = 0;
			for (j = 0; j < mix->numberofmodels[i]; j++)
				 prob += mix->mixtureprob[i][j] * gaussian(value, mix->mixturemeans[i][j], mix->mixturedeviations[i][j]);
			if (prob > maxprob)
			 {
				 maxprob = prob;
					index = i;
			 }
	 }
	return index;
}

/**
 * Calculates the probability of a class at point x. Finds mixture gaussian values of each class, sum them in order to normalize mixture gaussian values to get probabilities.
 * @param[in] mix Mixture structure
 * @param[in] value x
 * @param[in] classno Class index 
 * @return The probability of class with index classno at point value.
 */
double mixture_value(Mixtureptr mix, double value, int classno)
{
	/*!Last Changed 26.01.2005*/
	int i, j;
	double totalprob = 0, prob, realprob;
	for (i = 0; i < mix->classcount; i++)
	 {
		 prob = 0;
			for (j = 0; j < mix->numberofmodels[i]; j++)
				 prob += mix->mixtureprob[i][j] * gaussian(value, mix->mixturemeans[i][j], mix->mixturedeviations[i][j]);
			if (i == classno)
				 realprob = prob;
			totalprob += prob;
	 }
	return realprob / totalprob;
}

/**
 * Destructor for the mixture structure. Deallocates memory allocated for class priors, number of models in each class, mixture probabilities, mixture means etc.
 * @param[in] mix Mixture structure
 */
void free_mixture(Mixtureptr mix)
{
	/*!Last Changed 26.01.2005*/
	safe_free(mix->numberofmodels);
	safe_free(mix->classpriors);
	free_2d((void**)mix->mixtureprob, mix->classcount);
	free_2d((void**)mix->mixturemeans, mix->classcount);
	free_2d((void**)mix->mixturedeviations, mix->classcount);
	safe_free(mix);
}

/**
 * Generates random data from a mixture file. Constructs a mixture structure and uses generate_data_from_mixture_model to generate data.
 * @param[in] modeldefinition Mixture structure definition file
 * @param[in] datasize Number of data points to generate.
 * @return Two dimensional array of random data. result[i][0] is the input feature value of the instance i, results[i][1] is the class index of the instance i.
 */
double** generate_data_from_mixture_file(char* modeldefinition, int datasize)
{
	/*!Last Changed 26.01.2005*/
	Mixtureptr mix;
	double** data;
	mix = read_mixture_from_file(modeldefinition);
 data = generate_data_from_mixture_model(mix, datasize);
	free_mixture(mix);
	return data;
}

/**
 * Generates random data from a mixture structure. First it produces the class index of the random instance using the prior probabilities of the classes. Second, given its class, selects the mixture model of the random instance using the mixture probabilities of that class. Third, given its class and its mixture model, generates random normal value from the mean and deviation of the model.
 * @param[in] mix Mixture structure.
 * @param[in] datasize Number of data points to generate.
 * @return Two dimensional array of random data. result[i][0] is the input feature value of the instance i, results[i][1] is the class index of the instance i.
 */
double** generate_data_from_mixture_model(Mixtureptr mix, int datasize)
{
	/*!Last Changed 26.01.2005*/
	int i, whichclass, whichmodel;
	double** data, prob;
 data = (double**) safemalloc_2d(sizeof(double*), sizeof(double), datasize, 2, "generate_data_from_mixture_model", 5); 
	for (i = 0; i < datasize; i++)
	 {
		 prob = produce_random_value(0, 1);
   whichclass = find_bin(mix->classpriors, mix->classcount, prob);
			data[i][1] = whichclass;
			prob = produce_random_value(0, 1);
   whichmodel = find_bin(mix->mixtureprob[whichclass], mix->numberofmodels[whichclass], prob);
			data[i][0] = produce_normal_value(mix->mixturemeans[whichclass][whichmodel], mix->mixturedeviations[whichclass][whichmodel]);
	 }
	return data;
}
