#ifndef __svmsolve_H
#define __svmsolve_H
#include "svmq.h"

enum { 
      LOWER_BOUND = 0,
      UPPER_BOUND,
      FREE};

enum svm_q_matrix_type{ 
      QC = 0,
      QR,
      QONE};

typedef enum svm_q_matrix_type SVM_Q_MATRIX_TYPE;

struct solver{
	             int active_size;
	             signed char *y;
	             double *G;		
	             char *alpha_status;	
	             double *alpha;
														SVM_Q_MATRIX_TYPE qmatrixtype;
														Svcqptr svcq;
	             Svrqptr svrq;
														Oneclassqptr oneclassq;
	             double eps;
	             double Cp;
														double Cn;
	             double *b;
	             int *active_set;
	             double *G_bar;		
	             int l;
	             int unshrinked;
};

typedef struct solver Solver;
typedef Solver* Solverptr;

struct nusolver{
	               Solver s;
																Solutioninfoptr si;
};

typedef struct nusolver Nusolver;
typedef Nusolver* Nusolverptr;

double  calculate_rho(Solverptr s);
double  calculate_rho_general(Solverptr s, Solutioninfoptr si, int nu);
void    do_shrinking(Solverptr s);
void    do_shrinking_general(Solverptr s, int nu);
double  get_C(Solverptr s, int i);
int     is_free(Solverptr s, int i);
int     is_lower_bound(Solverptr s, int i); 
int     is_upper_bound(Solverptr s, int i);
double  nu_calculate_rho(Solverptr s, Solutioninfoptr si);
void    nu_do_shrinking(Solverptr s);
int     nu_select_working_set(Solverptr s, int* out_i, int* out_j);
void    nu_solve(Nusolverptr n, int l, SVM_Q_MATRIX_TYPE qmatrixtype, double *b, signed char *y, double *alpha, double Cp, double Cn, Solutioninfoptr si, Svm_problem prob, Svm_parameter param);
void    reconstruct_gradient(Solverptr s);
int     select_working_set(Solverptr s, int* out_i, int* out_j);
int     select_working_set_general(Solverptr s, int* out_i, int* out_j, int nu);
void    solve(Solverptr s, int l, SVM_Q_MATRIX_TYPE qmatrixtype, double* b, signed char *y, double *alpha, double Cp, double Cn, Solutioninfoptr si, Svm_problem prob, Svm_parameter param, int nu);
double* solver_get_Q(Solverptr s, int i, int len);
void    solver_swap_index(Solverptr s, int i, int j);
void    update_alpha_status(Solverptr s, int i);


#endif
