#ifndef __mixture_H
#define __mixture_H

/*! Structure for univarate Gaussian (one-dimensional) mixture for each class*/
struct mixture{
       /*! Number of classes in the problem. Size of the numberofmodels and classpriors array.*/
	      int classcount;
       /*! Number of models (mixture counts) of each class*/
							int* numberofmodels;
       /*! Prior probabilities of the classes*/
							double* classpriors;
       /*! Mixture probabilities (mixture weights) of each class. mixtureprob[i][j] is the mixture weight of the j'th component of class i*/
							double** mixtureprob;
       /*! Mean value of each univariate Gaussian. mixturemeans[i][j] is the mean of the Gaussian for the mixture component j of class i*/
							double** mixturemeans;
       /*! Standard deviation value of each univariate Gaussian. mixturedeviations[i][j] is the standard deviation of the Gaussian for the mixture component j of class i*/
							double** mixturedeviations;
};

typedef struct mixture Mixture;
typedef Mixture* Mixtureptr;

void       free_mixture(Mixtureptr mix);
double**   generate_data_from_mixture_file(char* modeldefinition, int datasize);
double**   generate_data_from_mixture_model(Mixtureptr mix, int datasize);
int        mixture_class_at_value(Mixtureptr mix, double value);
double     mixture_value(Mixtureptr mix, double value, int classno);
Mixtureptr read_mixture_from_file(char* modeldefinition);

#endif
