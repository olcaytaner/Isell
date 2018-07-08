#ifndef __convex_H
#define __convex_H

#include "constants.h"
#include "matrix.h"

enum solution_type{
                    SINGLE_SOLUTION = 0,
                    NO_SOLUTION,
                    UNBOUNDED_SOLUTION,
                    INFINITE_SOLUTION
};

typedef enum solution_type SOLUTION_TYPE;

struct convexsolution{
                      SOLUTION_TYPE solutiontype;
                      double optimalvalue;
                      double* variables;
                      int variablecount;
};

typedef struct convexsolution Convexsolution;
typedef Convexsolution* Convexsolutionptr;

/*! A linear programming problem is defined as
 Minimize f(X) = C^TX
 st.             AX <= B
                 X >= 0
 where number of variables is n and number of constraints is m.
 Example:
 Min -6x-14y-13z
 s.t. 0.5x+2y+z <= 24
        x+2y+4z <= 60
 File:
 3 2
 -6 -14 -13
 0.5 2 1 24
 1 2 4 60
 */
struct linearprogram{
                     /*! n*/
                     int variablecount;
                     /*! m*/
                     int constraintcount;
                     /*! matrices A, B*/
                     matrix constraints;
                     /*! vector C*/
                     double* objectives;
                     BOOLEAN* isbasic;
                     int* basic;
};

typedef struct linearprogram Linearprogram;
typedef Linearprogram* Linearprogramptr;

/*! A quadratic programming problem is defined as
 Minimize f(X) = C^TX + 1/2 X^TQX
 st.             AX <= B
 X >= 0
 where number of variables is n and number of constraints is m.
 Example:
 Min -8x-16y+x^2+4y^2
 s.t. x + y <= 5
      x     <= 3
 File:
 2 2
 -8 -16
 2 0
 0 8
 1 1 5
 1 0 3
 */
struct quadraticprogram{
 /*! n*/
 int n;
 /*! m*/
 int m;
 int constraintcount;
 int variablecount;
 /*! matrices A, B*/
 matrix constraints;
 /*! matrix Q*/
 matrix Q;
 /* vector c*/
 double* c;
 /*! modified objectives vector*/
 double* objectives;
 BOOLEAN* isbasic;
 int* basic;
};

typedef struct quadraticprogram Quadraticprogram;
typedef Quadraticprogram* Quadraticprogramptr;

int                 complementary_variable(int m, int n, int index);
void                drop_artificial_variables(Linearprogramptr program);
int                 find_variable_to_bring_into_basis(Linearprogramptr program);
int                 find_variable_to_bring_into_basis_qp(Quadraticprogramptr program);
int                 find_variable_to_bring_into_basis_qp_random(Quadraticprogramptr program);
int                 find_variable_to_remove_from_basis(matrix constraints, int constraintcount, int newbasic);
Linearprogramptr    formulate_infeasibility_form(Linearprogramptr program);
void                free_convex_solution(Convexsolutionptr solution);
void                free_linear_program(Linearprogramptr program);
void                free_quadratic_program(Quadraticprogramptr program);
BOOLEAN             is_program_in_canonical_form(Linearprogramptr program);
void                pivot_on_the_new_variables(matrix constraints, double* objectives, int newnonbasic, int newbasic);
Quadraticprogramptr prepare_quadratic_program(matrix Q, matrix A, double* c, double* b);
void                print_simplex_tableau(matrix constraints, int* basic, double* objectives);
Linearprogramptr    read_linear_program(char* filename);
Quadraticprogramptr read_quadratic_program(char* filename);
Convexsolutionptr   solve_lp(Linearprogramptr program);
Convexsolutionptr   solve_qp_using_simplex(Quadraticprogramptr program, BOOLEAN random);
Convexsolutionptr   solve_qp_wolfe_method(Quadraticprogramptr program);
Convexsolutionptr   solve_lp_with_basic_feasible_solution_using_simplex(Linearprogramptr program);

#endif
