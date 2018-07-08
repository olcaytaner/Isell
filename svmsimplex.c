#include <limits.h>
#include <math.h>
#include "convex.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmath.h" 
#include "libmemory.h"
#include "messages.h"
#include "svmsimplex.h"

extern Datasetptr current_dataset;

double kernel_value(Instanceptr inst1, Instanceptr inst2, Kernel_parameter kernel)
{
	switch (kernel.type)
  {
   case    KLINEAR :return dot_product(inst1, inst2);
                    break;
   case    KPOLY   :return pow(kernel.gamma * dot_product(inst1, inst2) + kernel.coef0, kernel.degree);
                    break;
   case    KRBF    :return exp(-kernel.gamma * distance_between_instances(inst1, inst2));
                    break;
   case    KSIGMOID:return tanh(kernel.gamma * dot_product(inst1, inst2) + kernel.coef0);
                    break;
   case KPREDEFINED:return current_dataset->kernel->values[(int) real_feature(inst1, 0)][(int) real_feature(inst2, 0)];
                    break;
   default         :printf(m1232, kernel.type);
                    return 0;
                    break;
  }
 return 0;
}

matrix prepare_Q_for_epsilon_svm(Instanceptr data, Kernel_parameter kernel)
{
 Instanceptr tmp, tmp1;
 matrix Q;
 int i, j, size = data_size(data);
 Q = matrix_alloc(2 * size, 2 * size);
 tmp = data;
 i = 0;
 while (tmp != NULL)
  {
   tmp1 = data;
   j = 0;
   while (tmp1 != NULL)
    {
     Q.values[i][j] = kernel_value(tmp, tmp1, kernel);
     Q.values[i + size][j + size] = Q.values[i][j];
     Q.values[i + size][j] = -Q.values[i][j];
     Q.values[i][j + size] = -Q.values[i][j];
     tmp1 = tmp1->next;
     j++;
    }
   tmp = tmp->next;
   i++;
  }
 return Q;
}

matrix prepare_Q_for_C_svm(Instanceptr data, Kernel_parameter kernel, int negative)
{
 Instanceptr tmp, tmp1;
 matrix Q;
 int i, j, size = data_size(data);
 Q = matrix_alloc(size, size);
 tmp = data;
 i = 0;
 while (tmp != NULL)
  {
   tmp1 = data;
   j = 0;
   while (tmp1 != NULL)
    {
     Q.values[i][j] = svm_classno(tmp, negative) * svm_classno(tmp1, negative) * kernel_value(tmp, tmp1, kernel);
     tmp1 = tmp1->next;
     j++;
    }
   tmp = tmp->next;
   i++;
  }
 return Q;
}

Quadraticprogramptr prepare_qp_for_epsilon_svm(Instanceptr data, Svm_simplex_parameterptr parameters)
{
 int i, size = data_size(data);
 double *c, *b;
 matrix A, Q;
 Instanceptr tmp;
 Quadraticprogramptr program;
 Q = prepare_Q_for_epsilon_svm(data, parameters->kernel);
 c = safemalloc(2 * size * sizeof(double), "prepare_qp_for_epsilon_svm", 4);
 for (i = 0, tmp = data; i < size; i++, tmp = tmp->next)
  {
   c[i] = parameters->epsilon - give_regressionvalue(tmp);
   c[i + size] = parameters->epsilon + give_regressionvalue(tmp);
  }
 b = safemalloc((2 * size + 2) * sizeof(double), "prepare_qp_for_epsilon_svm", 7);
 for (i = 0; i < 2 * size; i++)
   b[i] = parameters->C;
 b[2 * size] = 0;
 b[2 * size + 1] = 0;
 A = matrix_alloc(2 * size + 2, 2 * size);
 for (i = 0; i < 2 * size; i++)
   A.values[i][i] = 1;
 for (i = 0; i < size; i++)
  {
   A.values[2 * size][i] = 1;
   A.values[2 * size][size + i] = -1;
   A.values[2 * size + 1][i] = -1;
   A.values[2 * size + 1][size + i] = 1;
  }
 program = prepare_quadratic_program(Q, A, c, b);
 matrix_free(Q);
 matrix_free(A);
 safe_free(c);
 safe_free(b);
 return program;
}

Quadraticprogramptr prepare_qp_for_C_svm(Instanceptr data, Svm_simplex_parameterptr parameters, int negative)
{
 int i, size = data_size(data);
 double *c, *b;
 matrix A, Q;
 Instanceptr tmp;
 Quadraticprogramptr program;
 Q = prepare_Q_for_C_svm(data, parameters->kernel, negative);
 c = safemalloc(size * sizeof(double), "prepare_qp_for_C_svm", 4);
 for (i = 0; i < size; i++)
   c[i] = -1;
 b = safemalloc((size + 2) * sizeof(double), "prepare_qp_for_C_svm", 7);
 for (i = 0; i < size; i++)
   b[i] = parameters->C;
 b[size] = 0;
 b[size + 1] = 0;
 A = matrix_alloc(size + 2, size);
 for (i = 0, tmp = data; i < size; i++, tmp = tmp->next)
  {
   A.values[size][i] = svm_classno(tmp, negative);
   A.values[size + 1][i] = -svm_classno(tmp, negative);
   A.values[i][i] = 1;
  }
 program = prepare_quadratic_program(Q, A, c, b);
 matrix_free(Q);
 matrix_free(A);
 safe_free(c);
 safe_free(b);
 return program;
}

Svm_regression_modelptr solve_svm_regression(Instanceptr data, Svm_simplex_parameterptr parameters)
{
 Quadraticprogramptr program;
 Convexsolutionptr solution;
 Svm_regression_modelptr model;
 int i, j, k, correct, size;
 double sum, lower_bound = +INT_MAX, upper_bound = -INT_MAX;
 Instanceptr tmp;
 size = data_size(data);
 model = safemalloc(sizeof(Svm_regression_model), "solve_svm_regression", 4);
 program = prepare_qp_for_epsilon_svm(data, parameters);
 solution = solve_qp_wolfe_method(program);
 model->support_vector_count = 0;
 for (i = 0; i < size; i++)
   if (solution->variables[i] - solution->variables[i + size] > DBL_DELTA)
     model->support_vector_count++;
 model->alpha = safemalloc(model->support_vector_count * sizeof(double), "solve_svm_regression", 10);
 model->support_vectors = safemalloc(model->support_vector_count * sizeof(Instanceptr), "solve_svm_regression", 11);
 j = 0;
 model->b = 0;
 correct = 0;
 for (i = 0, tmp = data; i < size; i++, tmp = tmp->next)
  {
   if (solution->variables[i] - solution->variables[i + size] > DBL_DELTA)
    {
     model->alpha[j] = solution->variables[i] - solution->variables[i + size];
     model->support_vectors[j] = tmp;
     j++;
    }
   sum = program->c[i];
   for (k = 0; k < solution->variablecount; k++)
     sum += program->Q.values[i][k] * solution->variables[k];
   if (solution->variables[i] > DBL_DELTA && solution->variables[i] < parameters->C - DBL_DELTA)
    {
     model->b += sum;
     correct++;
    }
   else
     if (solution->variables[i] < DBL_DELTA && sum < lower_bound)
       lower_bound = sum;
     else
       if (solution->variables[i] > parameters->C - DBL_DELTA && sum > upper_bound)
         upper_bound = sum;
   sum = program->c[i + size];
   for (k = 0; k < solution->variablecount; k++)
     sum += program->Q.values[i + size][k] * solution->variables[k];
   sum *= -1;
   if (solution->variables[i + size] > DBL_DELTA && solution->variables[i + size] < parameters->C - DBL_DELTA)
    {
     model->b += sum;
     correct++;
    }
   else
     if (solution->variables[i] < DBL_DELTA && sum > upper_bound)
       upper_bound = sum;
     else
       if (solution->variables[i] > parameters->C - DBL_DELTA && sum < lower_bound)
         lower_bound = sum;
  }
 if (correct > 0)
   model->b /= correct;
 else
   model->b = (upper_bound + lower_bound) / 2;
 model->b *= -1;
 free_convex_solution(solution);
 free_quadratic_program(program);
 return model;
}

Svm_binary_modelptr solve_binary_svm(Instanceptr data, Svm_simplex_parameterptr parameters, int negative)
{
 int i, j, k, correct;
 Quadraticprogramptr program;
 Convexsolutionptr solution;
 Svm_binary_modelptr model;
 Instanceptr tmp;
 double sum, upper_bound = -INT_MAX, lower_bound = +INT_MAX;
 model = safemalloc(sizeof(Svm_binary_model), "solve_binary_svm", 4);
 program = prepare_qp_for_C_svm(data, parameters, negative);
 solution = solve_qp_wolfe_method(program);
 model->support_vector_count = 0;
 for (i = 0; i < solution->variablecount; i++)
   if (solution->variables[i] > DBL_DELTA)
     model->support_vector_count++;
 model->alpha_y = safemalloc(model->support_vector_count * sizeof(double), "solve_binary_svm", 10);
 model->support_vectors = safemalloc(model->support_vector_count * sizeof(Instanceptr), "solve_binary_svm", 11);
 j = 0;
 model->b = 0;
 correct = 0;
 for (i = 0, tmp = data; i < solution->variablecount; i++, tmp = tmp->next)
  {
   if (solution->variables[i] > DBL_DELTA)
    {
     model->alpha_y[j] = solution->variables[i] * svm_classno(tmp, negative);
     model->support_vectors[j] = tmp;
     j++;
    }
   sum = program->c[i];
   for (k = 0; k < solution->variablecount; k++)
     sum += program->Q.values[i][k] * solution->variables[k];
   sum *= svm_classno(tmp, negative);
   if (solution->variables[i] > DBL_DELTA && solution->variables[i] < (parameters->C - DBL_DELTA))
    {
     model->b += sum;
     correct++;
    }
   else
     if (solution->variables[i] < DBL_DELTA)
      {
       if (svm_classno(tmp, negative) == -1 && sum > upper_bound)
         upper_bound = sum;
       if (svm_classno(tmp, negative) == 1 && sum < lower_bound)
         lower_bound = sum;
      }
     else
       if (solution->variables[i] > (parameters->C - DBL_DELTA))
        {
         if (svm_classno(tmp, negative) == 1 && sum > upper_bound)
           upper_bound = sum;
         if (svm_classno(tmp, negative) == -1 && sum < lower_bound)
           lower_bound = sum;
        }
  }
 if (correct > 0)
   model->b /= correct;
 else
   model->b = (upper_bound + lower_bound) / 2;
 model->b *= -1;
 free_convex_solution(solution);
 free_quadratic_program(program);
 return model;
}

double test_svm_regression(Svm_regression_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst)
{
 int i;
 double sum = 0;
 for (i = 0; i < model->support_vector_count; i++)
   sum += model->alpha[i] * kernel_value(inst, model->support_vectors[i], parameters->kernel);
 sum += model->b;
 return sum;
}

double test_binary_svm(Svm_binary_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst)
{
 int i;
 double sum = 0;
 for (i = 0; i < model->support_vector_count; i++)
   sum += model->alpha_y[i] * kernel_value(inst, model->support_vectors[i], parameters->kernel);
 sum += model->b;
 return sum;
}

void free_svm_binary_model(Svm_binary_modelptr model)
{
 safe_free(model->alpha_y);
 safe_free(model->support_vectors);
 safe_free(model);
}

void free_svm_regression_model(Svm_regression_modelptr model)
{
 safe_free(model->alpha);
 safe_free(model->support_vectors);
 safe_free(model);
}

int number_of_support_vectors(void* model)
{
 int i, sv;
 if (current_dataset->classno == 2)
   return ((Svm_binary_modelptr) model)->support_vector_count;
 else
   if (current_dataset->classno == 0)
     return ((Svm_regression_modelptr) model)->support_vector_count;
   else
    {
     sv = 0;
     for (i = 0; i < ((Svm_simplex_modelptr)model)->problemcount; i++)
       sv += ((Svm_simplex_modelptr)model)->models[i]->support_vector_count;
     return sv;
    }
}

Svm_simplex_modelptr solve_one_vs_one_svm(Instanceptr* data, Svm_simplex_parameterptr parameters)
{
 int i, j, k;
 Instanceptr list;
 Svm_simplex_modelptr model;
 model = safemalloc(sizeof(Svm_simplex_model), "solve_one_vs_one_svm", 4);
 model->problemcount = current_dataset->classno * (current_dataset->classno - 1) / 2;
 model->models = safemalloc(model->problemcount * sizeof(Svm_binary_modelptr), "solve_one_vs_one_svm", 6);
 k = 0;
 for (i = 0; i < current_dataset->classno; i++)
   for (j = i + 1; j < current_dataset->classno; j++)
    {
     divide_instancelist_one_vs_one(data, &list, i, j);
     model->models[k] = solve_binary_svm(list, parameters, i);
     k++;
     merge_instancelist(data, list);
    }
 return model;
}

int test_one_vs_one_svm(Svm_simplex_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst)
{
 int i, j, k, *votes, maxVote;
 double vote;
 votes = safecalloc(current_dataset->classno, sizeof(int), "test_one_vs_one_svm", 2);
 k = 0;
 for (i = 0; i < current_dataset->classno; i++)
   for (j = i + 1; j < current_dataset->classno; j++)
    {
     vote = test_binary_svm(model->models[k], parameters, inst);
     if (vote < 0)
       votes[i]++;
     else
       votes[j]++;
    }
 maxVote = find_max_in_list(votes, current_dataset->classno);
 safe_free(votes);
 return maxVote;
}

Svm_simplex_modelptr solve_one_vs_rest_svm(Instanceptr data, Svm_simplex_parameterptr parameters)
{
 int i;
 Svm_simplex_modelptr model;
 model = safemalloc(sizeof(Svm_simplex_model), "solve_one_vs_one_svm", 3);
 model->problemcount = current_dataset->classno;
 model->models = safemalloc(model->problemcount * sizeof(Svm_binary_modelptr), "solve_one_vs_one_svm", 5);
 for (i = 0; i < current_dataset->classno; i++)
   model->models[i] = solve_binary_svm(data, parameters, i);
 return model;
}

void free_svm_simplex_model(Svm_simplex_modelptr model)
{
 int i;
 for (i = 0; i < model->problemcount; i++)
   free_svm_binary_model(model->models[i]);
 safe_free(model->models);
 safe_free(model);
}

int test_one_vs_rest_svm(Svm_simplex_modelptr model, Svm_simplex_parameterptr parameters, Instanceptr inst)
{
 int i, maxVote;
 double *votes;
 votes = safecalloc(current_dataset->classno, sizeof(double), "test_one_vs_rest_svm", 3);
 for (i = 0; i < current_dataset->classno; i++)
   votes[i] = test_binary_svm(model->models[i], parameters, inst);
 maxVote = find_min_in_list_double(votes, current_dataset->classno);
 safe_free(votes);
 return maxVote;
}
