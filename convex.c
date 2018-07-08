#include <limits.h>
#include <math.h>
#include "convex.h"
#include "datastructure.h"
#include "lang.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "librandom.h"
#include "messages.h"

Quadraticprogramptr copy_of_quadratic_program(Quadraticprogramptr src)
{
 Quadraticprogramptr dst;
 dst = safemalloc(sizeof(Quadraticprogram), "copy_of_quadratic_program", 2);
 dst->constraintcount = src->constraintcount;
 dst->m = src->m;
 dst->n = src->n;
 dst->variablecount = src->variablecount;
 dst->Q = matrix_copy(src->Q);
 dst->constraints = matrix_copy(src->constraints);
 dst->c = (double*) clone_void_array(src->c, src->n * sizeof(double));
 dst->objectives = (double*) clone_void_array(src->objectives, src->constraints.col * sizeof(double));
 dst->basic = (int*) clone_void_array(src->basic, src->constraintcount * sizeof(int));
 dst->isbasic = (BOOLEAN*) clone_void_array(src->isbasic, (src->constraints.col - 1) * sizeof(BOOLEAN));
 return dst;
}

Linearprogramptr read_linear_program(char* filename)
{
 int i, j, totalvariables;
 FILE* infile;
 Linearprogramptr program;
 infile = fopen(filename, "r");
 if (!infile)
  {
   printf(m1274, filename);
   return NULL;  
  }
 program = safemalloc(sizeof(Linearprogram), "read_linear_program", 6);
 fscanf(infile, "%d%d", &(program->variablecount), &(program->constraintcount));
 totalvariables = program->variablecount + program->constraintcount;
 program->objectives = safecalloc(totalvariables + 1, sizeof(double), "read_linear_program", 8);
 program->basic = safemalloc(program->constraintcount * sizeof(int), "read_linear_program", 9);
 program->isbasic = safecalloc(totalvariables, sizeof(BOOLEAN), "read_linear_program", 9);
 for (i = 0; i < program->variablecount; i++)
  {
   fscanf(infile, "%lf", &(program->objectives[i]));
   program->isbasic[i] = BOOLEAN_FALSE;
  }
 for (i = program->variablecount; i < totalvariables; i++)
  {
   program->isbasic[i] = BOOLEAN_TRUE;
   program->basic[i - program->variablecount] = i;
  }
 program->constraints = matrix_alloc(program->constraintcount, totalvariables + 1);
 for (i = 0; i < program->constraints.row; i++)
  {
   for (j = 0; j < program->variablecount; j++)
     fscanf(infile, "%lf", &(program->constraints.values[i][j]));
   program->constraints.values[i][program->variablecount + i] = 1.0;
   fscanf(infile, "%lf", &(program->constraints.values[i][totalvariables]));
  }
 return program;
}

Quadraticprogramptr prepare_quadratic_program(matrix Q, matrix A, double* c, double* b)
{
 int i, j, totalvariables;
 Quadraticprogramptr program;
 double sum;
 program = safemalloc(sizeof(Quadraticprogram), "read_quadratic_program", 6);
 program->n = Q.row;
 program->m = A.row;
 totalvariables = 3 * program->n + 3 * program->m;
 program->Q = matrix_copy(Q);
 program->c = clone_array(c, program->n);
 program->objectives = safecalloc(totalvariables + 1, sizeof(double), "read_quadratic_program", 10);
 program->basic = safemalloc((program->n + program->m) * sizeof(int), "read_quadratic_program", 11);
 program->isbasic = safecalloc(totalvariables, sizeof(BOOLEAN), "read_quadratic_program", 12);
 for (i = 0; i < 2 * program->n + 2 * program->m; i++)
   program->isbasic[i] = BOOLEAN_FALSE;
 for (i = 2 * program->n + 2 * program->m; i < 3 * program->n + 3 * program->m; i++)
  {
   program->basic[i - (2 * program->n + 2 * program->m)] = i;
   program->objectives[i] = 1;
   program->isbasic[i] = BOOLEAN_TRUE;
  }
 program->constraints = matrix_alloc(program->n + program->m, totalvariables + 1);
 for (i = 0; i < program->n; i++)
  {
   for (j = 0; j < program->n; j++)
     program->constraints.values[i][j] = Q.values[i][j];
   for (j = 0; j < program->m; j++)
     program->constraints.values[i][program->n + j] = A.values[j][i];
   for (j = 0; j < program->n; j++)
     if (j == i)
       program->constraints.values[i][program->m + program->n + j] = -1;
   for (j = 0; j < program->n; j++)
     if (j == i)
       program->constraints.values[i][2 * program->m + 2 * program->n + j] = 1;
   program->constraints.values[i][totalvariables] = -c[i];
  }
 for (i = 0; i < program->m; i++)
  {
   for (j = 0; j < program->n; j++)
     program->constraints.values[program->n + i][j] = A.values[i][j];
   for (j = 0; j < program->m; j++)
     if (j == i)
       program->constraints.values[program->n + i][program->m + 2 * program->n + j] = 1;
   for (j = 0; j < program->m; j++)
     if (j == i)
       program->constraints.values[program->n + i][2 * program->m + 3 * program->n + j] = 1;
   program->constraints.values[program->n + i][totalvariables] = b[i];
  }
 for (i = 0; i < program->constraints.row; i++)
   if (program->constraints.values[i][totalvariables] < 0)
    {
     for (j = 0; j < 2 * program->m + 2 * program->n; j++)
       program->constraints.values[i][j] *= - 1;
     program->constraints.values[i][totalvariables] *= - 1;
    }
 sum = 0;
 for (i = 0; i < program->m + program->n; i++)
   sum += program->constraints.values[i][totalvariables];
 program->objectives[totalvariables] = -sum;
 for (i = 0; i < 2 * program->m + 2 * program->n; i++)
  {
   sum = 0;
   for (j = 0; j < program->m + program->n; j++)
     sum += program->constraints.values[j][i];
   program->objectives[i] = -sum;
  }
 program->variablecount = 2 * program->m + 2 * program->n;
 program->constraintcount = program->m + program->n;
 return program;
}

Quadraticprogramptr read_quadratic_program(char* filename)
{
 int i, j, m, n;
 FILE* infile;
 double *c, *b;
 matrix Q, A;
 Quadraticprogramptr program;
 infile = fopen(filename, "r");
 if (!infile)
  {
   printf(m1274, filename);
   return NULL;
  }
 fscanf(infile, "%d%d", &n, &m);
 c = safecalloc(n, sizeof(double), "read_quadratic_program", 9);
 b = safecalloc(m, sizeof(double), "read_quadratic_program", 9);
 Q = matrix_alloc(n, n);
 A = matrix_alloc(m, n);
 for (i = 0; i < n; i++)
   fscanf(infile, "%lf", &(c[i]));
 for (i = 0; i < n; i++)
   for (j = 0; j < n; j++)
     fscanf(infile, "%lf", &(Q.values[i][j]));
 for (i = 0; i < m; i++)
  {
   for (j = 0; j < n; j++)
     fscanf(infile, "%lf", &(A.values[i][j]));
   fscanf(infile, "%lf", &(b[i]));
  }
 program = prepare_quadratic_program(Q, A, c, b);
 safe_free(c);
 safe_free(b);
 matrix_free(Q);
 matrix_free(A);
 return program;
}

BOOLEAN is_program_in_canonical_form(Linearprogramptr program)
{
 int i;
 for (i = 0; i < program->constraintcount; i++)
   if (program->constraints.values[i][program->constraints.col - 1] < 0)
     return BOOLEAN_FALSE;
 return BOOLEAN_TRUE;
}

void free_linear_program(Linearprogramptr program)
{
 safe_free(program->objectives);
 safe_free(program->isbasic);
 safe_free(program->basic);
 matrix_free(program->constraints);
 safe_free(program);
}

void free_quadratic_program(Quadraticprogramptr program)
{
 safe_free(program->basic);
 safe_free(program->isbasic);
 safe_free(program->objectives);
 safe_free(program->c);
 matrix_free(program->Q);
}

void free_convex_solution(Convexsolutionptr solution)
{
 safe_free(solution->variables);
 safe_free(solution);
}

int find_variable_to_bring_into_basis(Linearprogramptr program)
{
 int i, index = -2;
 double mincoefficient = +INT_MAX;
 for (i = 0; i < program->variablecount + program->constraintcount; i++)
   if (!program->isbasic[i] && program->objectives[i] < mincoefficient)
    {
     index = i;
     mincoefficient = program->objectives[i];
    }
 if (mincoefficient == 0.0)
   return -1;
 else
   if (mincoefficient > 0.0)
     return -2;
   else
     return index;
}

int complementary_variable(int m, int n, int index)
{
 if (index < m + n)
   return index + m + n;
 else
   return index - m - n;
}

int find_variable_to_bring_into_basis_qp_random(Quadraticprogramptr program)
{
 int i, index = -2, pos, complementary, newnonbasic, *candidate, size;
 Linklistptr candidates;
 candidates = link_list();
 for (i = 0; i < program->variablecount + program->constraintcount; i++)
   if (!program->isbasic[i] && program->objectives[i] < 0)
    {
     complementary = complementary_variable(program->m, program->n, i);
     newnonbasic = find_variable_to_remove_from_basis(program->constraints, program->constraintcount, i);
     if ((i >= 2 * program->m + 2 * program->n) || !program->isbasic[complementary] || (newnonbasic >= 0 && program->basic[newnonbasic] == complementary))
      {
       candidate = safemalloc(sizeof(int), "find_variable_to_bring_into_basis_qp", 10);
       *candidate = i;
       link_list_insert(candidates, candidate);
      }
    }
 size = link_list_size(candidates);
 if (size != 0)
  {
   pos = myrand() % size;
   candidate = (int*) link_list_get_i(candidates, pos);
   index = *candidate;
  }
 else
   index = -1;
 free_link_list(candidates, safe_free);
 return index;
}

int find_variable_to_bring_into_basis_qp(Quadraticprogramptr program)
{
 int i, index = -2, complementary, newnonbasic;
 double mincoefficient = +INT_MAX;
 for (i = 0; i < program->variablecount + program->constraintcount; i++)
   if (!program->isbasic[i] && program->objectives[i] < mincoefficient)
    {
     complementary = complementary_variable(program->m, program->n, i);
     newnonbasic = find_variable_to_remove_from_basis(program->constraints, program->constraintcount, i);
     if ((i >= 2 * program->m + 2 * program->n) || !program->isbasic[complementary] || (newnonbasic >= 0 && program->basic[newnonbasic] == complementary))
      {
       index = i;
       mincoefficient = program->objectives[i];
      }
    }
 if (mincoefficient == 0.0)
   return -1;
 else
   if (mincoefficient > 0.0)
     return -2;
   else
     return index;
}

int find_variable_to_remove_from_basis(matrix constraints, int constraintcount, int newbasic)
{
 int i, index = -1;
 double bi, ais, minratio = +INT_MAX;
 for (i = 0; i < constraintcount; i++)
  {
   bi = constraints.values[i][constraints.col - 1];
   ais = constraints.values[i][newbasic];
   if (ais > 0 && bi / ais < minratio)
    {
     index = i;
     minratio = bi / ais;
    }
  }
 return index;
}

void pivot_on_the_new_variables(matrix constraints, double* objectives, int newnonbasic, int newbasic)
{
 int i, j;
 double ars = constraints.values[newnonbasic][newbasic], factor;
 for (j = 0; j < constraints.col; j++)
   constraints.values[newnonbasic][j] /= ars;
 for (i = 0; i < constraints.row; i++)
   if (i != newnonbasic)
    {
     factor = -constraints.values[i][newbasic];
     if (factor != 0)
       for (j = 0; j < constraints.col; j++)
         constraints.values[i][j] += factor * constraints.values[newnonbasic][j];
    }
 factor = -objectives[newbasic];
 for (j = 0; j < constraints.col; j++)
   objectives[j] += factor * constraints.values[newnonbasic][j];
}

void print_simplex_tableau(matrix constraints, int* basic, double* objectives)
{
 int i, j;
 printf("------------------------------------------------\n");
 for (i = 0; i < constraints.row; i++)
  {
   printf("%3d ", basic[i]);
   for (j = 0; j < constraints.col; j++)
     printf("%7.3f ", constraints.values[i][j]);
   printf("\n");
  }
 printf("  F ");
 for (j = 0; j < constraints.col; j++)
   printf("%7.3f ", objectives[j]);
 printf("\n");
 printf("------------------------------------------------\n");
}

Convexsolutionptr solve_lp_with_basic_feasible_solution_using_simplex(Linearprogramptr program)
{
 Convexsolutionptr solution;
 SOLUTION_TYPE solutiontype = NO_SOLUTION;
 int i, newbasic, newnonbasic;
 solution = safemalloc(sizeof(Convexsolution), "solve_lp_with_basic_feasible_solution_using_simplex", 4);
 while (BOOLEAN_TRUE)
  {
   /*print_simplex_tableau(program->constraints, program->basic, program->objectives);*/
   newbasic = find_variable_to_bring_into_basis(program);
   if (newbasic == -1)
    {
     solutiontype = INFINITE_SOLUTION;
     break;
    }
   if (newbasic == -2)
    {
     solutiontype = SINGLE_SOLUTION;
     break;    
    }
   newnonbasic = find_variable_to_remove_from_basis(program->constraints, program->constraintcount, newbasic);
   if (newnonbasic == -1)
    {
     solutiontype = UNBOUNDED_SOLUTION;
     break;
    }
   pivot_on_the_new_variables(program->constraints, program->objectives, newnonbasic, newbasic);
   program->isbasic[newbasic] = BOOLEAN_TRUE;
   program->isbasic[program->basic[newnonbasic]] = BOOLEAN_FALSE;
   program->basic[newnonbasic] = newbasic;
  }
 solution->optimalvalue = program->objectives[program->variablecount + program->constraintcount];
 solution->variablecount = program->variablecount;
 solution->solutiontype = solutiontype;
 solution->variables = safecalloc(solution->variablecount, sizeof(double), "solve_lp_with_basic_feasible_solution_using_simplex", 23);
 for (i = 0; i < program->constraintcount; i++)
   if (program->basic[i] < program->variablecount)
     solution->variables[program->basic[i]] = program->constraints.values[i][program->variablecount + program->constraintcount];
 return solution;
}

Linearprogramptr formulate_infeasibility_form(Linearprogramptr program)
{
 int i, j, totalvariables;
 double sum;
 Linearprogramptr newprogram;
 newprogram = safemalloc(sizeof(Linearprogram), "formulate_infeasibility_form", 2);
 newprogram->variablecount = program->variablecount + program->constraintcount;
 newprogram->constraintcount = program->constraintcount;
 totalvariables = program->variablecount + 2 * program->constraintcount;
 newprogram->isbasic = safecalloc(totalvariables, sizeof(BOOLEAN), "formulate_infeasibility_form", 7);
 newprogram->basic = safemalloc(program->constraintcount * sizeof(int), "formulate_infeasibility_form", 8);
 for (i = 0; i < newprogram->variablecount; i++)
   newprogram->isbasic[i] = BOOLEAN_FALSE;
 for (i = newprogram->variablecount; i < totalvariables; i++)
  {
   newprogram->isbasic[i] = BOOLEAN_TRUE;
   newprogram->basic[i - newprogram->variablecount] = i;
  }
 newprogram->constraints = matrix_alloc(program->constraintcount + 1, totalvariables + 1);
 for (i = 0; i < program->constraintcount; i++)
  {
   newprogram->constraints.values[i][newprogram->variablecount + i] = 1.0;
   if (program->constraints.values[i][program->constraints.col - 1] < 0)
    {
     for (j = 0; j < newprogram->variablecount; j++)
       newprogram->constraints.values[i][j] = -program->constraints.values[i][j];
     newprogram->constraints.values[i][newprogram->constraints.col - 1] = -program->constraints.values[i][program->constraints.col - 1];
    }
   else
    {
     for (j = 0; j < newprogram->variablecount; j++)
       newprogram->constraints.values[i][j] = program->constraints.values[i][j];
     newprogram->constraints.values[i][newprogram->constraints.col - 1] = program->constraints.values[i][program->constraints.col - 1];     
    }
  }
 for (j = 0; j < program->variablecount; j++)
   newprogram->constraints.values[program->constraintcount][j] = program->objectives[j];
 newprogram->objectives = safecalloc(totalvariables + 1, sizeof(double), "formulate_infeasibility_form", 26);
 for (i = 0; i < newprogram->variablecount; i++)
  {
   sum = 0.0;
   for (j = 0; j < program->constraintcount; j++)
     sum += newprogram->constraints.values[j][i];
   newprogram->objectives[i] = -sum;
  }
 sum = 0.0;
 for (j = 0; j < program->constraintcount; j++)
   sum += newprogram->constraints.values[j][newprogram->constraints.col - 1];
 newprogram->objectives[totalvariables] = -sum;
 return newprogram;
}

void drop_artificial_variables(Linearprogramptr program)
{
 int i, totalvariables;
 double tmp;
 program->variablecount -= program->constraintcount;
 totalvariables = program->variablecount + program->constraintcount;
 program->isbasic = alloc(program->isbasic, totalvariables * sizeof(BOOLEAN), totalvariables);
 program->objectives = alloc(program->objectives, (totalvariables + 1) * sizeof(double), totalvariables + 1);
 for (i = 0; i < totalvariables; i++)
   program->objectives[i] = program->constraints.values[program->constraintcount][i];
 program->objectives[totalvariables] = program->constraints.values[program->constraintcount][program->constraints.col - 1];
 for (i = 0; i < program->constraintcount; i++)
  {
   tmp = program->constraints.values[i][program->constraints.col - 1];
   program->constraints.values[i] = alloc(program->constraints.values[i], (totalvariables + 1) * sizeof(double), totalvariables + 1);
   program->constraints.values[i][totalvariables] = tmp;
  }
 safe_free(program->constraints.values[program->constraintcount]);
 (program->constraints.row)--;
 program->constraints.values = alloc(program->constraints.values, program->constraints.row * sizeof(double*), program->constraints.row);
 program->constraints.col -= program->constraintcount;
}

Convexsolutionptr solve_lp(Linearprogramptr program)
{
 Convexsolutionptr solution;
 Linearprogramptr newprogram;
 if (is_program_in_canonical_form(program))
   return solve_lp_with_basic_feasible_solution_using_simplex(program);
 newprogram = formulate_infeasibility_form(program);
 solution = solve_lp_with_basic_feasible_solution_using_simplex(newprogram);
 if (solution->optimalvalue <= DBL_DELTA)
  {
   drop_artificial_variables(newprogram);
   return solve_lp_with_basic_feasible_solution_using_simplex(newprogram);
  }
 return NULL;
}

Convexsolutionptr solve_qp_wolfe_method(Quadraticprogramptr program)
{
 int iteration = 0;
 Convexsolutionptr solution, best;
 Quadraticprogramptr copy;
 double optimal;
 copy = copy_of_quadratic_program(program);
 best = solve_qp_using_simplex(copy, BOOLEAN_FALSE);
 optimal = fabs(best->optimalvalue);
 while (optimal > DBL_DELTA && iteration < 10)
  {
   free_quadratic_program(copy);
   copy = copy_of_quadratic_program(program);
   solution = solve_qp_using_simplex(copy, BOOLEAN_TRUE);
   iteration++;
   if (fabs(solution->optimalvalue) < optimal)
    {
     optimal = fabs(solution->optimalvalue);
     free_convex_solution(best);
     best = solution;
    }
   else
     free_convex_solution(solution);
  }
 free_quadratic_program(copy);
 return best;
}

Convexsolutionptr solve_qp_using_simplex(Quadraticprogramptr program, BOOLEAN random)
{
 Convexsolutionptr solution;
 SOLUTION_TYPE solutiontype = NO_SOLUTION;
 int i, newbasic, newnonbasic;
 solution = safemalloc(sizeof(Convexsolution), "solve_qp_using_simplex", 4);
 while (BOOLEAN_TRUE)
  {
   /*print_simplex_tableau(program->constraints, program->basic, program->objectives);*/
   if (random)
     newbasic = find_variable_to_bring_into_basis_qp_random(program);
   else
     newbasic = find_variable_to_bring_into_basis_qp(program);
   if (newbasic == -1)
    {
     solutiontype = INFINITE_SOLUTION;
     break;
    }
   if (newbasic == -2)
    {
     solutiontype = SINGLE_SOLUTION;
     break;
    }
   newnonbasic = find_variable_to_remove_from_basis(program->constraints, program->constraintcount, newbasic);
   if (newnonbasic == -1)
    {
     solutiontype = UNBOUNDED_SOLUTION;
     break;
    }
   pivot_on_the_new_variables(program->constraints, program->objectives, newnonbasic, newbasic);
   program->isbasic[newbasic] = BOOLEAN_TRUE;
   program->isbasic[program->basic[newnonbasic]] = BOOLEAN_FALSE;
   program->basic[newnonbasic] = newbasic;
  }
 solution->optimalvalue = program->objectives[program->variablecount + program->constraintcount];
 printf("Optimal:%f\n", solution->optimalvalue);
 solution->variablecount = program->n;
 solution->solutiontype = solutiontype;
 solution->variables = safecalloc(solution->variablecount, sizeof(double), "solve_qp_using_simplex", 23);
 for (i = 0; i < program->constraintcount; i++)
   if (program->basic[i] < program->n)
     solution->variables[program->basic[i]] = program->constraints.values[i][program->variablecount + program->constraintcount];
 return solution;
}
