#ifndef __svmsimplex_H
#define __svmsimplex_H
#include "convex.h"
#include "typedef.h"

struct kernel_parameter{
                        KERNEL_TYPE type;
                        int degree;
                        double gamma;
                        double coef0;
};

typedef struct kernel_parameter Kernel_parameter;

struct svm_simplex_parameter{
                             Kernel_parameter kernel;
                             PROBLEM_TYPE problem_type;
                             double C;
                             double epsilon;
};

typedef struct svm_simplex_parameter Svm_simplex_parameter;
typedef Svm_simplex_parameter* Svm_simplex_parameterptr;

struct svm_binary_model{
                         int support_vector_count;
                         Instanceptr* support_vectors;
                         double* alpha_y;
                         double b;
};

typedef struct svm_binary_model Svm_binary_model;
typedef Svm_binary_model* Svm_binary_modelptr;

struct svm_simplex_model{
                         int problemcount;
                         Svm_binary_modelptr* models;
};

typedef struct svm_simplex_model Svm_simplex_model;
typedef Svm_simplex_model* Svm_simplex_modelptr;

struct svm_regression_model{
                            int support_vector_count;
                            Instanceptr* support_vectors;
                            double* alpha;
                            double b;
};

typedef struct svm_regression_model Svm_regression_model;
typedef Svm_regression_model* Svm_regression_modelptr;

void                    free_svm_binary_model(Svm_binary_modelptr model);
void                    free_svm_regression_model(Svm_regression_modelptr model);
void                    free_svm_simplex_model(Svm_simplex_modelptr model);
double                  kernel_value(Instanceptr inst1, Instanceptr inst2, Kernel_parameter kernel);
int                     number_of_support_vectors(void* model);
matrix                  prepare_Q_for_C_svm(Instanceptr data, Kernel_parameter kernel, int negative);
matrix                  prepare_Q_for_epsilon_svm(Instanceptr data, Kernel_parameter kernel);
Quadraticprogramptr     prepare_qp_for_C_svm(Instanceptr data, Svm_simplex_parameterptr parameters, int negative);
Quadraticprogramptr     prepare_qp_for_epsilon_svm(Instanceptr data, Svm_simplex_parameterptr parameters);
Svm_binary_modelptr     solve_binary_svm(Instanceptr data, Svm_simplex_parameterptr parameters, int negative);
Svm_simplex_modelptr    solve_one_vs_one_svm(Instanceptr* data, Svm_simplex_parameterptr parameters);
Svm_simplex_modelptr    solve_one_vs_rest_svm(Instanceptr data, Svm_simplex_parameterptr parameters);
Svm_regression_modelptr solve_svm_regression(Instanceptr data, Svm_simplex_parameterptr parameters);
double                  test_binary_svm(Svm_binary_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst);
int                     test_one_vs_one_svm(Svm_simplex_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst);
int                     test_one_vs_rest_svm(Svm_simplex_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst);
double                  test_svm_regression(Svm_regression_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst);

#endif
