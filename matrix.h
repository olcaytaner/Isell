#ifndef __matrix_H
#define __matrix_H
#include <stdio.h>
#include "interval.h"

/*! Matrix structure*/
struct TMatrix{
       /*! Number of rows in the matrix*/
       int row;
       /*! Number of columns in the matrix*/
       int col;
       /*! Values in the matrix. Stored as a two-dimensional array.*/
       double** values;
       };

typedef struct TMatrix matrix;
typedef matrix* matrixptr;

void      assign_partial_random_weights(matrix* M, int startrow, int endrow, int startcol, int endcol, double randrange);
void      assignTwoDimRandomWeights(matrixptr M, double randrange);
double*   balance(matrix a);
void      balbak(matrix* eigenvectors, double* scale);
int       cholesky_decomposition(matrix a, matrix* b);
void      eigensort(double **d, matrix *v);
int*      elmhes(matrix a);
void      eltran(matrix* eigenvectors, matrix a, int* perm);
double*   find_eigenvalues_eigenvectors(matrix* vector, matrix cov, int* newdimension, double range);
double*   find_eigenvalues_eigenvectors_symmetric(matrix* eigenvectors, matrix a);
double*   find_eigenvalues_eigenvectors_unsymmetric(matrix* eigenvectors, matrix a);
double*   hqr2(matrix* eigenvectors, matrix a);
matrix    identity(int n);
void      initialize_matrix(matrixptr M, double c);
void      initialize_partial_matrix(matrix* M, int startrow, int endrow, int startcol, int endcol, double c);
BOOLEAN   is_symmetric(matrix M);
void      jacobi(matrix matrix2, double **d, matrix *v);
double    mahalanobis_distance(matrix x, matrix y, matrix S);
void      matrix_add(matrix *a,matrix b);
BOOLEAN   matrix_addition_check(matrix a, matrix b);
matrix    matrix_alloc(int row,int col);
double*   matrix_average_columns(matrix a);
double*   matrix_average_rows(matrix a);
void      matrix_cell_power(matrix* a, double power);
void      matrix_cell_square(matrix* a);
void      matrix_columnwise_merge(matrix* a, matrix b);
matrix    matrix_copy(matrix source);
matrix    matrix_correlation(matrix a);
matrix    matrix_covariance(matrix a, double** averages);
double    matrix_determinant(matrix matrix1);
matrix    matrix_diagonal(double* values, int size);
matrix    matrix_difference(matrix a,matrix b);
void      matrix_divide_constant(matrix *a,double cons);
void      matrix_exchange_columns(matrix a, int col1, int col2);
void      matrix_free(matrix mat);
void      matrix_identical(matrix* dest, matrix source);
int       matrix_inverse(matrix *matrix1);
double*   matrix_max_rows(matrix a);
double    matrix_max(matrix a);
matrix    matrix_merge_rows(matrix a, matrix b);
double    matrix_min(matrix a);
int       matrix_multiplication_check(matrix a, matrix b);
matrix    matrix_multiply(matrix a,matrix b);
void      matrix_multiply_constant(matrix *a, double cons);
void      matrix_ones(matrix *a);
matrix    matrix_partial(matrix a,int rowstart,int rowend,int colstart,int colend);
void      matrix_perturbate(matrix matrix1);
matrix    matrix_power(matrix a, int power);
void      matrix_print(matrix m);
void      matrix_product(matrix* a, matrix b);
void      matrix_resize(matrix* a, Intervalptr interval);
void      matrix_rowwise_merge(matrix* a, matrix b);
void      matrix_sort_by_column(matrix a, int index, int sort_type);
double*   matrix_stdev_columns(matrix a);
void      matrix_subtract(matrix *a,matrix b);
matrix    matrix_sum(matrix a,matrix b);
double    matrix_sum_column(matrix a, int index);
double    matrix_sum_of_elements(matrix a);
double    matrix_sum_row(matrix a, int index);
double    matrix_trace(matrix a);
matrix    matrix_transpose(matrix a);
void      matrix_zeros(matrix *a);
matrix    pooled_covariance(matrix a, int na, matrix b, int nb);
matrix    pseudoinverse(matrix a, double range, int* newdim);
matrix    random_matrix(int row, int col, double start, double end);
matrix    read_matrix(char* fname);
matrix    read_matrix_filehandle(FILE* infile);
matrixptr read_matrix_without_header(char* fname);
matrix    reallocate_matrix(matrix M, int newRow, int newColumn);
matrix    svdcmp(matrix a, int m, int n, double* w);
void      write_matrix(char* fname, matrix m, int writeheader);

#endif
