#ifndef __hmm_h
#define __hmm_h
#include "matrix.h"
#include "parameter.h"
#include "typedef.h"

#define WEIGHT_DELTA 0.0
#define DELTA 0.01
#define PRIOR 0.01

#define HMM_OPERATORS 3

enum hmm_operator_type{
      NO_CHANGE = -1,
      ADD_ONE_STATE = 0,
      ADD_ONE_COMPONENT,
      ADD_ONE_BOTH};

typedef enum hmm_operator_type HMM_OPERATOR_TYPE;

/*! Emission probability structure for multi-feature discrete HMM.*/
struct multinomial{
  /*! prob[i][j] Emission probability of the j'th value of the feature i.*/
  double** prob;
};

typedef struct multinomial Multinomial;
typedef Multinomial* Multinomialptr;

/*! Emission probability structure for continuous HMM with dirichlet pdf at its states.*/
struct dirichlet{
  /*! Weights of the dirichlet pdf*/
  double* alpha;
};

typedef struct dirichlet Dirichlet;
typedef Dirichlet* Dirichletptr;

/*! Emission probability structure for continuous HMM with multivariate gaussian pdf at its states.*/
struct gaussian{
  /*! Mean vector of the multivariate gaussian pdf*/
	 double* mean;
  /*! Covariance matrix of the multivariate gaussian pdf */
		matrix covariance;
};

typedef struct gaussian Gaussian;
typedef Gaussian* Gaussianptr;

/*! Emission probability structure for continuous HMM with mixture of multivariate Gaussian pdf at its states.*/
struct gaussianmixture{
  /*! Gaussian components of the Gaussian mixture. Stored as an array of Gaussian structures*/
  Gaussianptr components;
  /*! Number of Gaussian components. Size of the components and weights arrays*/
  int componentcount;
  /*! Mixture weights */
  double* weights;
};

typedef struct gaussianmixture Gaussianmixture;
typedef Gaussianmixture* Gaussianmixtureptr;

/*! Emission probability structure for continuous HMM with mixture of dirichlet pdf at its states.*/
struct dirichletmixture{
  /*! Dirichlet components of the Dirichlet mixture. Stored as an array of Dirichlet structures*/
  Dirichletptr components;
  /*! Number of Dirichlet components. Size of the components and weights arrays*/
  int componentcount;
  /*! Mixture weights */
  double* weights;
};

typedef struct dirichletmixture Dirichletmixture;
typedef Dirichletmixture* Dirichletmixtureptr;

/*! Structure for each state of the HMM*/
struct state{
  /*! Incoming state transition probabilities for this node. a_dot_j[i] = A[i][j] where A is state transition probability matrix*/
	 double* a_dot_j; /*Gelen okların a'sı a_{.j} */
  /*! Outgoing state transition probabilities for this node. a_i_dot[j] = A[i][j] where A is state transition probability matrix*/
  double* a_i_dot; /*Giden okların a'sı a_{i.} */
  /*! Initial state probability for this state*/
	 double pi;
  /*! Observation (emission) probability structure. Can be multinomial, Gaussian, mixture of Gaussians, Dirichlet, mixture of Dirichlets*/
		void* b; 
};

typedef struct state State;
typedef State* Stateptr;

/*! Structure of Hidden Markov Model (HMM)*/
struct hmm{
  /*! States in the model*/
	 Stateptr states;
  /*! Number of states in the HMM*/
		int statecount;
  /*! Type of states in the HMM. Can be multinomial, Gaussian, mixture of Gaussians, Dirichlet, mixture of Dirichlets*/
  HMM_STATE_TYPE statetype;
  /*! Structure of the HMM. Can be lefttoright model, where each state is connected only to the next state and itself with exception that there is single connection (to itself) in the last state. Can be lefttorightloop model, where each state is connected to the next state and itself with the last state connected to the first state. Can be full model, where each state is connected to all other states.*/
  HMM_MODEL_TYPE hmmtype;
};

typedef struct hmm Hmm;
typedef Hmm* Hmmptr;

void               add_component(Hmmptr hmm, int count, Instanceptr traindata);
double*            add_probability_conserving(double* prob, int count, int added);
void               add_state(Hmmptr hmm, int count, Instanceptr traindata);
void               alloc_dirichlet(Dirichletptr d);
void               alloc_gaussian(Gaussianptr g);
Hmmptr             alloc_hmm(int statecount, HMM_STATE_TYPE statetype, int componentcount, HMM_MODEL_TYPE hmmtype);
void*              alloc_state_model(HMM_STATE_TYPE statetype, int componentcount);
Hmmptr             apply_operator(Hmmptr hmm, int operatortype, Hmm_parameterptr p, Instanceptr traindata);
double             baumwelch(Hmmptr hmm, Instanceptr traindata, int numiteration);
Hmmptr             best_hmm_for_mixture(Instanceptr traindata, Instanceptr cvdata, Hmm_parameterptr p);
Hmmptr             best_hmm_for_single(Instanceptr traindata, Instanceptr cvdata, Hmm_parameterptr p);
void               best_hmm_for_together(Hmmptr* model, Instanceptr* trdata, Instanceptr cvdata, Hmm_parameterptr p);
void               calc_bwd(Hmmptr hmm, Instanceptr traindata, int seqlen, double **beta, double *scale);
void               calc_fwd(Hmmptr hmm, Instanceptr traindata, double **alpha, double * loglikelihood, double *scale);
void               calc_gamma(Hmmptr hmm, int seqlen, double **alpha, double **beta, double **gamma);
void               calc_gammaMix(Hmmptr hmm, Instanceptr traindata, int seqlen, double **gamma, double ***gammaMix);
double             calc_obslik(Hmmptr hmm, Instanceptr inst, int stateindex);
double             calc_obslikMix(Hmmptr hmm, Instanceptr inst, int stateindex, int componentindex);
void               calc_xi(Hmmptr hmm, Instanceptr traindata, int seqlen, double **alpha, double **beta, double ***xi);
Hmmptr             calloc_hmm(int statecount, HMM_STATE_TYPE statetype, int componentcount, HMM_MODEL_TYPE hmmtype);
void*              calloc_state_model(HMM_STATE_TYPE statetype, int componentcount);
int                check_hmm(char* name, Hmmptr hmm);
Hmmptr             clone_hmm(Hmmptr srchmm);
void*              clone_state_model(void* model, HMM_STATE_TYPE statetype);
int                complexity_of_hmm(Hmmptr hmm);
void               copy_hmm(Hmmptr desthmm, Hmmptr srchmm);
void               copy_state_model(void *destmodel, void* srcmodel, HMM_STATE_TYPE statetype);
double             dirichlet_mixture_multivariate(Instanceptr inst, Dirichletmixtureptr dm);
Hmmptr             empty_hmm(int statecount, HMM_STATE_TYPE statetype, HMM_MODEL_TYPE hmmtype);
void               free_hmm(Hmmptr hmm);
void               free_state_model(void* model, HMM_STATE_TYPE statetype);
double             gaussian_mixture_multivariate(Instanceptr inst, Gaussianmixtureptr gm);
void               generate_gaussian_mixture_data(Gaussianmixtureptr gm, int* classes, int datacount, char* outfilename);
int                get_component_count(State s, HMM_STATE_TYPE statetype);
double             get_dirichlet_mixture_multivariate_component(Instanceptr inst, Dirichletmixtureptr dm, int component);
double             get_gaussian_mixture_multivariate_component(Instanceptr inst, Gaussianmixtureptr gm, int component);
double             hmm_error_rate(Hmmptr* model, Instanceptr data);
void               init_hmm(Hmmptr srchmm);
void               init_state_model(void* model, HMM_STATE_TYPE statetype);
double             iterate_em_hmm(Hmmptr hmm, Instanceptr traindata, double *accumA, double *accumB, double **accumMix);
double             loglikelihood_of_hmm(Hmmptr hmm, Instanceptr data);
double             multinomial_multivariate(Instanceptr inst, Multinomialptr m);
void               multiply_constant_state_model(void *destmodel, double mul, HMM_STATE_TYPE statetype);
void               random_dirichlet(Dirichletptr d, Instanceptr traindata);
void               random_gaussian(Gaussianptr g, Instanceptr traindata);
Hmmptr             random_hmm(int statecount, HMM_STATE_TYPE statetype, int componentcount, HMM_MODEL_TYPE hmmtype, Instanceptr traindata);
void*              random_state_model(HMM_STATE_TYPE statetype, int componentcount, Instanceptr traindata);
Gaussianmixtureptr read_gaussian_mixture(char* filename, int** classes);
void               sum_hmm(Hmmptr desthmm, Hmmptr srchmm);
void               sum_state_model(void *destmodel, void* srcmodel, HMM_STATE_TYPE statetype);
int                test_hmm_instance(Instanceptr inst, Hmmptr* hmmarray);
void               update_state_model(void *model, double accum, double *accumMix, HMM_STATE_TYPE statetype);
int*               viterbi(Hmmptr hmm, Instanceptr traindata); 
void               printcontinstance(FILE *controlfile, Instanceptr inst);
void               printcontobservation(FILE *controlfile, Instanceptr inst);
void               printgamma(FILE *controlfile, double **gamma, Hmmptr hmm, int seqlen);
void   		          printgaussianhmm(FILE *controlfile, Hmmptr hmm);
void               printgaussianmixtureemission(FILE *controlfile, Gaussianmixtureptr g, int numberofcomponents);

#endif
