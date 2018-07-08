#ifndef __model_h
#define __model_h

#include "mlp.h"

#define MAX_MODEL_SEARCH 7

struct hmm_model{
                 int states;
                 int mixture;
                 int index;
                 int selected;
                 int tried;
};

typedef struct hmm_model Hmm_model;
typedef Hmm_model* Hmm_modelptr;

enum model_search_direction_type{ 
      FORWARD = 0,
      BACKWARD};

typedef enum model_search_direction_type MODEL_SEARCH_DIRECTION_TYPE;

enum most_search_type{ 
      MULTIMOST = 0,
      ONE_FORWARD,
      ONE_BACKWARD,
      FORWARD_MOST,
      BACKWARD_MOST};

typedef enum most_search_type MOST_SEARCH_TYPE;

double      aic_loglikelihood(double loglikelihood, double d);
double      aic_squared_error(double error, double d, int N);
double      aic_squared_error_with_sigma(double error, double d, int N, double sigma);
double      bic_loglikelihood(double loglikelihood, double d, int N);
double      bic_squared_error(double error, double d, int N);
double      bic_squared_error_with_sigma(double error, double d, int N, double sigma);
Hmm_model   compare_hmm_model_selection_techniques(char* simulationdirectory, int maxstates, int maxmixture);
int         compare_mlp_model_selection_techniques(char *datasetname, char* simulationdirectory, int maxhidden, int starthidden);
int         compare_two_hmm_models(char** two_file, char* simulationdirectory, Hmm_model model1, Hmm_model model2);
int         compare_two_mlp_models(char** two_file, char* simulationdirectory, Datasetptr d, int model1, int model2);
double      epsilon_sensitive_loss(double y, double fx, double epsilon);
int*        generate_candidate_mlp_models(Datasetptr d, int maxhidden, int bestmodel, int*modelcount, MODEL_SEARCH_DIRECTION_TYPE searchdirection);
void        hmm_add_candidate_model(char** filenames, char* directory, int index, Hmm_modelptr models, int* models_searched, int* filecount);
Hmm_model   hmm_model_simulation(MOST_SEARCH_TYPE simulation_algorithm, char* simulationdirectory, int maxstates, int maxmixture, int* tries);
Hmm_model   hmm_model_simulation_most(char* simulationdirectory, int maxstates, int maxmixture, int* tries, MODEL_SEARCH_DIRECTION_TYPE searchdirection);
Hmm_model   hmm_model_simulation_multitest(char* simulationdirectory, int maxstates, int maxmixture, int* tries);
Hmm_model   hmm_model_simulation_one_backward(char* simulationdirectory, int maxstates, int* tries);
Hmm_model   hmm_model_simulation_one_forward(char* simulationdirectory, int maxstates, int* tries);
double      hinge_loss(double y, double fx);
double      mdl(double error, int d, double lambda, int number_of_bits);
double      mlp_model_selection(Instanceptr* data, MODEL_SELECTION_TYPE modelselectionmethod, int* hiddennodes, int maxhidden, double* bestalpha);
Mlpnetwork mlp_model_selection_aic(Instanceptr data, Instanceptr validation, double alpha, int maxhidden, int problemtype);
Mlpnetwork mlp_model_selection_bic(Instanceptr data, Instanceptr validation, double alpha, int maxhidden, int problemtype);
Mlpnetwork mlp_model_selection_crossvalidation(Instanceptr* data, Instanceptr validation, double alpha, int maxhidden, int problemtype);
int         mlp_model_simulation(MOST_SEARCH_TYPE simulation_algorithm, char * datasetname, char* simulationdirectory, int maxhidden, int starthidden, int* tries);
int         mlp_model_simulation_most(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries, MODEL_SEARCH_DIRECTION_TYPE searchdirection);
int         mlp_model_simulation_one_backward(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries);
int         mlp_model_simulation_one_forward(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries);
int         mlp_model_simulation_multitest(Datasetptr d, char* simulationdirectory, int maxhidden, int starthidden, int* tries);
double      number_of_free_parameters_of_hessian(double* eigenvalues, int weightcount, double alpha);
void        search_and_add_candidate_model(char** filenames, char* directory, int model, int* models, int* filecount);
double      srm_classification(double error, int h, int N, double a1, double a2);
double      srm_epsilon(int h, int N, double a1, double a2);
double      srm_regression(double error, int h, int N, double a1, double a2);

#endif
