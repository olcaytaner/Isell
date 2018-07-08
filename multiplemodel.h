#ifndef __multiplemodel_h
#define __multiplemodel_h

#include "matrix.h"
#include "typedef.h"

enum function_type{ 
      FUNC_CONSTANT = 0,
      FUNC_POWER,
      FUNC_SINUS,
      FUNC_COSINUS};

typedef enum function_type FUNCTION_TYPE;

/*! Structure for a multiplicative term. Multiplicative terms are stored as a link list. This structure holds one node information of that link list. A multiplicative term may be a constant such as 2.4 or a power function such as x1^5 (power to the 5 of the first feature) or sinus, cosinus functions (to the power also) such as sin^4(...).*/
struct multiplicativeterm{
                          /*! Exponent part of the multiplicative term. default value 1.*/
	                         int power;
                          /*! Type of the function of the multiplicative term. 0 for constant, 1 for polynom, 2 for sinus, 3 for cosinus*/
                          FUNCTION_TYPE functiontype; 
                          /*! Index of the feature. */
																										int featureindex;
                          /*! If the multiplicative term is constant, value of this constant*/
																										double value;
                          /*! If the multiplicative term contains an interm, this structure holds that interm*/
																										struct additiveterm* interm;
                          /*! Pointer to the next multiplicative term*/
	                         struct multiplicativeterm* next;
                         };

typedef struct multiplicativeterm Mterm;
typedef Mterm* Mtermptr;

/*! Structure for an additive term. Additive terms are stored as a link list. This structure holds one node information of that link list. An additive term consists of one or more multiplicative terms multipled together such as 2.4sin^2(..)cos(...).*/
struct additiveterm{
                    /*! Sign of the additive term. +1 for plus sign, -1 for minus sign*/
	                   int sign;
                    /*! Pointer to the first node of the multiplicative term link list*/
	                   Mtermptr start;
                    /*! Pointer to the next additive term*/
																				struct additiveterm* next;
                   };

typedef struct additiveterm Aterm;
typedef Aterm* Atermptr;

/*! Structure for a single model. An example model can be 2.4sin(x3)+cos^2(x1)-x1^5x2^4, where the additive terms are 2.4sin(x3), cos^2(x1) and x1^5x2^4. The multiplicative terms are 2.4, sin(x3) for the first additive term, cos^2(x1) for the second additive term and x1^5 and x2^4 for the third additive term.*/
struct singlemodel{
                   /*! Pointer to the first node of the additive term link list*/
	                  Atermptr start;
                   /*! Number of data points for this model*/
																			int datacount;
                   /*! Level of noise to add this model */
																			double noiselevel;
                  };

typedef struct singlemodel Singlemodel;
typedef Singlemodel* Singlemodelptr;

/*! Structure for a multiple model. Contains an array of single models*/
struct multiplemodel{
                     /*! An array of single models*/
	                    Singlemodelptr models;
                     /*! Number of single models. Size of the models array.*/
																					int modelcount;
                     /*! Number of features used in the multiple model. Size of the min and max arrays*/
																					int featurecount;
                     /*! Minimum value of each feature. Stored as an array. min[i] is the minimum of feature i*/
																					double* min;
                     /*! Maximum value of each feature. Stored as an array. max[i] is the minimum of feature i*/
																					double* max;
                    };

typedef struct multiplemodel Multiplemodel;
typedef Multiplemodel* Multiplemodelptr;

double           additive_term_value(Atermptr term, double* featurevalues);
int              data_count_of_multiple_model(Multiplemodelptr m);
char**           extract_additive_terms(char* st, int* count);
char**           extract_multiplicative_terms(char* st, int* count);
void             free_additive_term(Atermptr a);
void             free_multiple_model(Multiplemodelptr m);
void             free_multiplicative_term(Mtermptr m);
void             free_single_model(Singlemodel s);
matrix           generate_data_from_multiple_model(Multiplemodelptr m);
matrix           generate_data_from_single_model(Singlemodel s, int featurecount, double* min, double* max);
int              index_and_power_from_string(char* st, int* power);
char*            interm_and_power_from_string(char* st, int* power);
int              mmelin1(Instanceptr* traindata);
double           multiplicative_term_value(Mtermptr term, double* featurevalues);
Atermptr         read_additive_term_from_string(char* st);
Multiplemodelptr read_multiple_model_from_file(char* modelfile);
Mtermptr         read_multiplicative_term_from_string(char* st);
void             read_single_model_from_string(Singlemodelptr s, char* modelst);
void             remove_instances_according_to_threshold(Instanceptr* parentlist, Instanceptr* removed, double threshold, matrix w, char which);
double           single_model_value(Singlemodel m, double* featurevalues);

#endif
