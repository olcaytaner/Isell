#ifndef __svm_H
#define __svm_H

enum svm_type{ 
      C_SVC = 0,
      NU_SVC,
      ONE_CLASS,
      EPSILON_SVR,
      NU_SVR,
      TWO_CLASS};

typedef enum svm_type SVM_TYPE;

enum kernel_type{ 
 KLINEAR = 0,
 KPOLY,
 KRBF,
 KSIGMOID,
 KPREDEFINED};

typedef enum kernel_type KERNEL_TYPE;

struct solutioninfo{
		                  double obj;
		                  double rho;
		                  double upper_bound_p;
		                  double upper_bound_n;
		                  double r;	
};


typedef struct solutioninfo Solutioninfo;
typedef Solutioninfo* Solutioninfoptr;

struct svm_node{
	int index;
	double value;
};

typedef struct svm_node Svm_node;
typedef Svm_node* Svm_nodeptr;

struct svm_problem{
	int l;
	double *y;
	Svm_nodeptr *x;
	int nr_class;
};

typedef struct svm_problem Svm_problem;
typedef Svm_problem* Svm_problemptr;

struct svm_parameter
{
	SVM_TYPE svm_type;
	KERNEL_TYPE kernel_type;
	int degree;
	double gamma;
	double coef0;
	double cache_size; 
	double eps;	
	double C;	
	double* cweights;
	double nu;	
	double p;	
	int shrinking;	
};

typedef struct svm_parameter Svm_parameter;
typedef Svm_parameter* Svm_parameterptr;

struct svm_weights{
	                  double *alpha;
	                  double rho;	
																			int nSV;
																			int nBSV;
};

typedef struct svm_weights Svm_weights;
typedef Svm_weights* Svm_weightsptr;

struct svm_split{
	Svm_parameter param;
	int l;
	Svm_node **SV;
	double* sv_coef;
	double rho;
	double* weights;
	int nSV;
};

typedef struct svm_split Svm_split;
typedef Svm_split* Svm_splitptr;

struct svm_model
{
	Svm_parameter param;	
	int nr_class;		
	int l;			
	Svm_node **SV;		
	double **sv_coef;	
	double *rho;		
	int *nSV;
	Svm_problem prob;
 double *probA;         
 double *probB; 
};

typedef struct svm_model Svm_model;
typedef Svm_model* Svm_modelptr;

void         free_svm_model(Svm_modelptr model);
void         free_svm_problem(Svm_problem prob);
void         free_svm_split(Svm_split s);
void         multiclass_probability(int k, double **r, double *p);
double       sigmoid_predict(double decision_value, double A, double B);
void         sigmoid_train(int l, double *dec_values, double *labels, double* A, double* B);
void         solve_c_svc(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si, double Cp, double Cn);
void         solve_epsilon_svr(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si);
void         solve_nu_svc(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si);
void         solve_nu_svr(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si);
void         solve_one_class(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si);
void         svm_binary_svc_probability(Svm_problem prob, Svm_parameter param, double Cp, double Cn, double* probA, double* probB);
int          svm_group_classes(Svm_problem prob, int **start_ret, int **count_ret, int *perm);
double       svm_predict(Svm_modelptr model, Svm_nodeptr x);
void         svm_predict_probability(Svm_modelptr model, Svm_nodeptr x, double* prob_estimates);
void         svm_predict_values(Svm_modelptr model, Svm_nodeptr x, double* dec_values);
Svm_modelptr svm_train(Svm_problem prob, Svm_parameter param, int calculate_probability);
Svm_weights  svm_train_one(Svm_problem prob, Svm_parameter param, double Cp, double Cn);


#endif
