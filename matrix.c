#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include "libarray.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "matrix.h"
#include "messages.h"

/**
 * Merge two matrices (first one has m rows, second one has n rows) together in a row basis. The first m rows come from the first matrix, next n rows come from the second matrix.
 * @param[in] a First matrix
 * @param[in] b Second matrix
 * @warning If two matrices do not have the same number of columns, the minimum column will be copied.
 * @return Merged matrix with m + n rows and min(number of columns of the first matrix, number of columns of the second matrix) columns.
 */
matrix matrix_merge_rows(matrix a, matrix b)
{
 /*!Last Changed 13.02.2005*/
	int mincol, i, j;
	matrix result;
	mincol = imin(a.col, b.col);
	result = matrix_alloc(a.row + b.row, mincol);
 for (i = 0; i < a.row; i++)
		 for (j = 0; j < mincol; j++)
				 result.values[i][j] = a.values[i][j];
	for (i = a.row; i < a.row + b.row; i++)
		 for (j = 0; j < mincol; j++)
				 result.values[i][j] = b.values[i - a.row][j];
	return result;
}

/**
 * Any square matrix A with non-zero pivots can be written as the product of a lower triangular matrix L and an upper triangular matrix U; this is called the LU decomposition. However, if A is symmetric and positive definite, we can choose the factors such that U is the transpose of L, and this is called the Cholesky decomposition.
 * @param[in] a Matrix A
 * @param[out] b Result matrix L (lower diagonal). Upper diagonal of b is L*
 * @return 1 if a is symmetric and positive definite, that is Cholesky decomposition is possible, 0 otherwise.
 */
int cholesky_decomposition(matrix a, matrix* b)
{
 /*!Last Changed 27.04.2007*/
 int i, j, k;
 double sum;
 *b = matrix_alloc(a.row, a.col);
 for (i = 0; i < a.row; i++) 
   for (j = i; j < a.row; j++) 
    {
     sum = a.values[i][j];
     for (k = i - 1; k >= 0; k--) 
       sum -= a.values[i][k] * a.values[j][k];
     if (i == j) 
      {
       if (sum <= 0.0)
         return 0;
       b->values[i][i] = sqrt(sum);
      } 
     else 
       b->values[j][i] = sum / b->values[i][i];
    }
 return 1;
}

/**
 * In mathematics, and in particular linear algebra, the pseudoinverse, A^+ of an m \times n matrix, A, is a generalization of the inverse matrix. The pseudoinverse is defined and unique for all matrices whose entries are real or complex numbers. The pseudoinverse can be computed using the singular value decomposition.
 * @param[in] a Input matrix
 * @param[in] range Threshold for variance explained
 * @param[out] newdim Number of columns in the pseudoinverse. If the matrix A has full column rank, newdim will be equal to the column rank of A, otherwise its columns will be determined according to the threshold range.
 * @return Pseudoinverse of A
 */
matrix pseudoinverse(matrix a, double range, int* newdim)
{
	/*!Last Changed 02.02.2005*/
	int i;
	matrix u, v, s, partv, partu, transu, result, tmpresult;
	double* w, sum, sumnow;
 u = matrix_copy(a);
	w = safemalloc(u.col * sizeof(double), "pseuodoinverse", 5);
	v = svdcmp(u, u.row, u.col, w);   
	sum = 0;
	for (i = 0; i < u.col; i++)
		 sum += w[i];
	*newdim = u.col;
	sumnow = 0;
	for (i = 0; i < u.col; i++)
	 {
		 sumnow += w[i];
			if (sumnow / sum > range)
			 {
				 *newdim = i + 1;
				 break;
			 }
	 }
	partv = matrix_partial(v, 0, a.row - 1, 0, *newdim - 1);
	matrix_free(v);
	partu = matrix_partial(u, 0, a.row - 1, 0, *newdim - 1);
	matrix_free(u);
	transu = matrix_transpose(partu);
	matrix_free(partu);
	s = identity(*newdim);
	for (i = 0; i < *newdim; i++)
		 s.values[i][i] = 1 / w[i];
 tmpresult = matrix_multiply(partv, s);
	matrix_free(partv);
	matrix_free(s);
	safe_free(w);
	result = matrix_multiply(tmpresult, transu);
	matrix_free(tmpresult);
	matrix_free(transu);
	return result;
}

/**
 * Generates random matrix.
 * @param[in] row Number of rows in the output matrix
 * @param[in] col Number of columns in the output matrix
 * @param[in] start Lower threshold for the elements of the matrix
 * @param[in] end Upper threshold for the elements of the matrix
 * @return Random matrix whose elements have values between start and end.
 */
matrix random_matrix(int row, int col, double start, double end)
{
	/*!Last Changed 02.02.2005*/
	int i, j;
	matrix tmp;
	tmp = matrix_alloc(row, col);
 for (i = 0; i < row; i++)
		 for (j = 0; j < col; j++)
				 tmp.values[i][j] = produce_random_value(start, end);
	return tmp;
}

/**
 * Reads matrix from a file.
 * @param[in] infile Input file
 * @pre infile must be opened before this function, and closed after this function.
 * @warning Number of rows and columns are also read from the input file
 * @return Matrix read
 */
matrix read_matrix_filehandle(FILE *infile)
{
 int row, col, i, j;
	matrix tmp;
 if (!infile)
	 {
   tmp = matrix_alloc(1, 1);
			tmp.values[0][0] = 0;
		 return tmp;
	 }
	fscanf(infile, "%d%d", &row, &col);
 tmp = matrix_alloc(row, col);
 for (i = 0; i < row; i++)
		 for (j = 0; j < col; j++)
				 fscanf(infile, "%lf", &(tmp.values[i][j]));
 return(tmp);
}

/**
 * Reads a matrix from a file that does not contain the number of rows and the number of columns.
 * @param[in] fname Name of the input file
 * @return Matrix read
 */
matrixptr read_matrix_without_header(char* fname)
{
 /*!Last Changed 25.01.2009*/
 FILE* infile;
 int row, col, i, j;
 matrixptr tmp;
 row = file_length(fname);
 col = number_of_floats_in_file(fname);
 tmp = safemalloc(sizeof(matrix), "read_matrix_without_header", 6);
 *tmp = matrix_alloc(row, col);
 infile = fopen(fname, "r"); 
 for (i = 0; i < row; i++)
   for (j = 0; j < col; j++)
     fscanf(infile, "%lf", &(tmp->values[i][j]));
 fclose(infile);
 return tmp;
}

/**
 * Reads a matrix from a file using read_matrix_filehandle function.
 * @param[in] fname Name of the input file
 * @return Matrix read
 */
matrix read_matrix(char* fname)
{
	/*!Last Changed 24.03.2006 by Aydin*/
	FILE* infile;
	matrix tmp;
	infile = fopen(fname, "r");
	tmp = read_matrix_filehandle(infile);
	fclose(infile);
	return tmp;
}

/**
 * Prints a matrix to the console.
 * @param[in] m Matrix to be printed
 */
void matrix_print(matrix m)
{
 /*!Last Changed 21.08.2007*/
 int i, j;
 for (i = 0; i < m.row; i++)
  {
   for (j = 0; j < m.col; j++)
     printf("%.6f ", m.values[i][j]);
   printf("\n");
  }
}

/**
 * Writes a matrix to an output file
 * @param[in] fname Name of the output file
 * @param[in] m Matrix to be written
 * @param[in] writeheader If 0, no header (number of rows and columns of the matrix) will be written to the file, otherwise headers will also be written
 */
void write_matrix(char* fname, matrix m, int writeheader)
{
	/*!Last Changed 11.04.2005 added writeheader*/
	/*!Last Changed 02.02.2005*/
	FILE* outfile;
	int i, j;
	char pst[READLINELENGTH];
	outfile = fopen(fname, "w");
	if (!outfile)
		 return;
	if (writeheader)
  	fprintf(outfile, "%d %d\n", m.row, m.col);
	for (i = 0; i < m.row; i++)
	 {
		 for (j = 0; j < m.col; j++)
			 {
     set_precision(pst, NULL, " ");
				 fprintf(outfile, pst, m.values[i][j]);
			 }
			fprintf(outfile, "\n");
	 }
	fclose(outfile);
}

/**
 * Initializes some part of a matrix to a value. Partial matrix elements are M[startrow][startcol] ... M[startrow][endcol] M[startrow + 1][startcol] ... M[endrow][endcol].
 * @param[out] M Matrix to be initialized
 * @param[in] startrow Starting row of the partial matrix (included).
 * @param[in] endrow Ending row of the partial matrix (included).
 * @param[in] startcol Starting column of the partial matrix (included).
 * @param[in] endcol Ending column of the partial matrix (included).
 * @param[in] c Value to be set.
 */
void initialize_partial_matrix(matrix* M, int startrow, int endrow, int startcol, int endcol, double c)
{
	/*!Last Changed 17.04.2007*/
	int i, j;
	for (i = startrow; i <= endrow; i++)
		 for (j = startcol; j <= endcol; j++)
		  	M->values[i][j] = c;
}

/**
 * Initializes all elements of a matrix to a value
 * @param[out] M Matrix to be initialized.
 * @param[in] c Value to be set.
 */
void initialize_matrix(matrix* M, double c)
{
	int i, j;
	for (i = 0; i < M->row; i++)
		 for (j = 0; j < M->col; j++)
		  	M->values[i][j] = c;
}

/**
 * Checks if a matrix is symmetric or not
 * @param[in] M Matrix to be checked for symmetry.
 * @return BOOLEAN_TRUE is the matrix is symmetric, BOOLEAN_FALSE otherwise.
 */
BOOLEAN is_symmetric(matrix M)
{
 int i, j;
 if (M.row != M.col)
   return BOOLEAN_FALSE;
 for (i = 0; i < M.row; i++)
   for (j = i + 1; j < M.col; j++)
     if (M.values[i][j] != M.values[j][i])
       return BOOLEAN_FALSE;
 return BOOLEAN_TRUE;
}

/**
 * Assigns random numbers (in a range) to a part of the matrix.
 * @param[out] M Matrix to be assigned
 * @param[in] startrow Starting row of the partial matrix (included).
 * @param[in] endrow Ending row of the partial matrix (included).
 * @param[in] startcol Starting column of the partial matrix (included).
 * @param[in] endcol Ending column of the partial matrix (included).
 * @param[in] randrange Random range. Random numbers will be assigned between -randrange and +randrange.
 */
void assign_partial_random_weights(matrix* M, int startrow, int endrow, int startcol, int endcol, double randrange)
{
	/*!Last Changed 17.04.2007*/
	int i, j;
	for (i = startrow; i <= endrow; i++)
		 for (j = startcol; j <= endcol; j++)
			  M->values[i][j] = (((double) myrand() / RANDOM_MAX) * randrange * 2) - randrange;
}

/**
 * Assigns random numbers to all elements of the matrix.
 * @param[out] M Matrix to be assigned
 * @param[in] randrange Random range. Random numbers will be assigned between -randrange and +randrange.
 */
void assignTwoDimRandomWeights(matrix* M, double randrange)
{
	int i, j;
	for (i = 0; i < M->row; i++)
		 for (j = 0; j < M->col; j++)
			  M->values[i][j] = (((double) myrand() / RANDOM_MAX) * randrange * 2) - randrange;
}

/**
 * Reallocates memory to the matrix with the corresponding new dimensions (number of rows and number of columns)
 * @param[in] M Old matrix
 * @param[in] newRow Number of rows in the new (output) matrix
 * @param[in] newColumn Number of columns in the new (output) matrix
 * @warning newRow and newColumn can be smaller than the old row and column values
 * @return New (output) matrix
 */
matrix reallocate_matrix(matrix M, int newRow, int newColumn)
{
	/*!Last Changed 17.04.2007*/
 int i;
	if (M.row != newRow)
	 {
		 if (M.row > newRow)
     for (i = newRow; i < M.row; i++)
       safe_free(M.values[i]);
   M.values = alloc(M.values, newRow * sizeof(double *), newRow);
			if (M.row < newRow)
     for (i = M.row; i < newRow; i++)
       M.values[i] = safemalloc(newColumn * sizeof(double), "reallocate_matrix", 28);
	 }
	if (M.col != newColumn)
   for (i = 0; i < M.row; i++)
     M.values[i] = alloc(M.values[i], newColumn * sizeof(double), newColumn);
 M.row = newRow;
	M.col = newColumn;
	return M;
}

/**
 * Constructor function of the matrix structure. Allocates memory (also initializes them to 0) for the elements of a matrix. Initializes the row and column fields.
 * @param[in] row Number of rows in the matrix
 * @param[in] col Number of columns in the matrix
 * @return Allocated and initialized matrix
 */
matrix matrix_alloc(int row, int col)
{ 
 /*!Last Changed 02.02.2004 added safemalloc*/
 matrix temp;
 temp.values = (double **)safecalloc_2d(sizeof(double *), sizeof(double), row, col, "matrix_alloc", 3);
 temp.row = row; 
 temp.col = col; 
 return temp;
}

/**
 * Multiplies the matrix with a constant value. This has the result of multiplying all elements one by one with this value.
 * @param[out] a Matrix to be multiplied
 * @param[in] cons Value
 */
void matrix_multiply_constant(matrix *a, double cons)
{ 
 int i, j; 
 for (i = 0; i < a->row; i++)
   for (j = 0; j < a->col; j++)     
     a->values[i][j] *= cons;
}

/**
 * Divides the matrix by a constant value. This has the result of dividing all elements one by one by this value.
 * @param[out] a Matrix to be divided
 * @param[in] cons Value
 */
void matrix_divide_constant(matrix *a, double cons)
{
 int i, j;
 for (i = 0; i < a->row; i++)
   for (j = 0; j < a->col; j++)     
     a->values[i][j] /= cons;
}

/**
 * Initializes the matrix elements to 1.
 * @param[out] a Matrix to be initialized.
 */
void matrix_ones(matrix *a)
{
 int i, j; 
 for (i = 0; i < a->row; i++)   
   for (j = 0; j < a->col; j++)     
     a->values[i][j] = 1;
}

/**
 * Initializes the matrix elements to 0.
 * @param[out] a Matrix to be initialized.
 */
void matrix_zeros(matrix *a)
{
 int i, j; 
 for (i = 0; i < a->row; i++)   
   for (j = 0; j < a->col; j++)     
     a->values[i][j] = 0;
}

/**
 * Checks if two matrices can be added. Two matrices can be added if and only if their row numbers match and columns numbers match.
 * @param[in] a First matrix to be added
 * @param[in] b Second matrix to be added
 * @return 1 if the matrices can be added, 0 otherwise.
 */
BOOLEAN matrix_addition_check(matrix a, matrix b)
{
 /*!Last Changed 26.12.2006*/
 if (a.row != b.row || a.col != b.col)
  {
   printf(m1363);
   return BOOLEAN_FALSE;
  }
 return BOOLEAN_TRUE;
}
     
/**
 * Checks if two matrices can be multiplied. Two matrices can be multiplied if the first one's column number match with the second one's row number.
 * @param[in] a First matrix to be multiplied
 * @param[in] b Second matrix to be multiplied
 * @return 1 if the matrices can be multiplied, 0 otherwise.
 */
int matrix_multiplication_check(matrix a, matrix b)
{
 /*!Last Changed 26.12.2006*/
 if (a.col != b.row)
  {
   printf(m1364);
   return 0;
  }
 return 1;
}

/**
 * Adds second matrix to the first matrix (a += b).
 * @param[out] a First matrix
 * @warning If two matrices can not be added, makes no change.
 * @param[in] b Second matrix
 */
void matrix_add(matrix *a, matrix b)
{
 int i, j; 
 if (!matrix_addition_check(*a, b))
   return;
 for (i = 0; i < a->row; i++)
   for (j = 0; j < a->col; j++)
     a->values[i][j] += b.values[i][j];
}

/**
 * Sums two matrices (c = a + b).
 * @param[in] a First matrix
 * @param[in] b Second matrix
 * @warning If two matrices can not be added, returns a.
 * @return Sum of the two matrices as a new matrix.
 */
matrix matrix_sum(matrix a, matrix b)
{
 int i, j;
	matrix result;
 if (!matrix_addition_check(a, b))
   return a;
 result = matrix_alloc(a.row, a.col);
 for (i = 0; i < a.row; i++)
   for (j = 0; j < a.col; j++)
     result.values[i][j] = a.values[i][j] + b.values[i][j];
 return result;
}

/**
 * Subtracts second matrix from the first matrix (a -= b).
 * @param[out] a First matrix
 * @warning If two matrices can not be subtracted, makes no change.
 * @param[in] b Second matrix
 */
void matrix_subtract(matrix *a, matrix b)
{
 int i, j;
 if (!matrix_addition_check(*a, b))
   return; 
 for (i = 0; i < a->row; i++)
   for (j = 0; j < a->col; j++)
     a->values[i][j] -= b.values[i][j];
}

/**
 * Takes the difference of two matrices (c = a - b).
 * @param[in] a First matrix
 * @param[in] b Second matrix
 * @warning If two matrices can not be subtracted, returns a.
 * @return Difference of the two matrices as a new matrix.
 */
matrix matrix_difference(matrix a, matrix b)
{
 int i, j;
	matrix result;
 if (!matrix_addition_check(a, b))
   return a; 
 result = matrix_alloc(a.row,a.col);
 for (i = 0; i < a.row; i++)
   for (j = 0; j < a.col; j++)
     result.values[i][j] = a.values[i][j] - b.values[i][j];
 return result;    
}

/**
 * Multiplies second matrix with the first matrix (a *= b).
 * @param[out] a First matrix
 * @warning If two matrices can not be multiplied, makes no change.
 * @param[in] b Second matrix
 */
void matrix_product(matrix* a, matrix b)
{
	/*!Last Changed 23.12.2006*/
	matrix result;
	result = matrix_multiply(*a, b);
	matrix_free(*a);
	*a = result;
}

matrix matrix_power(matrix a, int power)
{
 int i;
	matrix result;
 result = matrix_copy(a);
 for (i = 1; i < power; i++)
   matrix_product(&result, a);
 return result;
}

/**
 * Takes the exponent of each element of the matrix
 * @param[out] a Matrix whose elements will get exponentiated
 * @param[in] power Power
 */
void matrix_cell_power(matrix* a, double power)
{
 /*!Last Changed 17.01.2008*/
 int i, j;
 for (i = 0; i < a->row; i++)
   for (j = 0; j < a->col; j++)
     a->values[i][j] = pow(a->values[i][j], power);
}

/**
 * Takes the square of each element of the matrix
 * @param[out] a Matrix whose elements will be squared
 */
void matrix_cell_square(matrix* a)
{
 /*!Last Changed 17.01.2008*/
 int i, j;
 for (i = 0; i < a->row; i++)
   for (j = 0; j < a->col; j++)
     a->values[i][j] = a->values[i][j] * a->values[i][j];
}

/**
 * Generates a diagonal matrix whose diagonal elements are given as parameters
 * @param[in] values Diagonal elements.
 * @param[in] size Number of rows and columns of the resulting matrix (Size of the values array).
 * @return Diagonal matrix whose diagonal elements come from values array.
 */
matrix matrix_diagonal(double* values, int size)
{
 /*!Last Changed 17.01.2008*/
 int i;
 matrix result;
 result = matrix_alloc(size, size);
 for (i = 0; i < size; i++)
   result.values[i][i] = values[i];
 return result;
}

/**
 * Multiplies two matrices (c = a * b).
 * @param[in] a First matrix
 * @param[in] b Second matrix
 * @warning If two matrices can not be multiplied, returns a.
 * @return Product of the two matrices as a new matrix.
 */
matrix matrix_multiply(matrix a, matrix b)
{
 int i, j, k;
 matrix result;
 if (!matrix_multiplication_check(a, b))
   return a;
 result = matrix_alloc(a.row, b.col);
 for (i = 0; i < a.row; i++)
   for (k = 0; k < b.col; k++)
    {
     result.values[i][k] = 0;
     for (j = 0; j < a.col; j++)
       result.values[i][k] += a.values[i][j] * b.values[j][k];
    }
 return result;
}

double matrix_sum_of_elements(matrix a)
{
	/*!Last Changed 23.12.2006*/
	int i, j;
	double sum = 0.0;
 for (i = 0; i < a.row; i++)
		 for (j = 0; j < a.col; j++)
				 sum += a.values[i][j];
	return sum;
}

double matrix_trace(matrix a)
{
	/*!Last Changed 23.12.2006*/
	int i;
	double sum = 0.0;
 if (a.row != a.col)
  {
   printf(m1365, "Trace");
   return 0;
  }
 for (i = 0; i < a.row; i++)
		 sum += a.values[i][i];
	return sum;
}

void matrix_sort_by_column(matrix a, int index, int sort_type)
{
 quicksort_array_2d(a.values, a.col, index, sort_type, 0, a.row - 1);
}

double matrix_sum_column(matrix a, int index)
{
 /*!Last Changed 19.01.2008*/
 int i;
 double sum = 0.0;
 for (i = 0; i < a.row; i++)
   sum += a.values[i][index];
 return sum;
}

double* matrix_average_columns(matrix a)
{
	/*!Last Changed 23.12.2006*/
	int i, j;
	double* result = safecalloc(a.col, sizeof(double), "matrix_average_columns", 2);
	for (i = 0; i < a.row; i++)
		 for (j = 0; j < a.col; j++)
				 result[j] += a.values[i][j];
	for (j = 0; j < a.col; j++)
		 result[j] /= a.row;
	return result;
}

double* matrix_max_rows(matrix a)
{
	int i, j;
	double* result = safecalloc(a.row, sizeof(double), "matrix_average_columns", 2);
	for (i = 0; i < a.row; i++)
  {
   result[i] = 0;
   for (j = 1; j < a.col; j++)
     if (a.values[i][(int)result[i]] < a.values[i][j])
       result[i] = j;
  }
	return result;
}

double* matrix_stdev_columns(matrix a)
{
 /*!Last Changed 28.02.2009*/
 int i, j;
 double* average = matrix_average_columns(a);
 double* result = safecalloc(a.col, sizeof(double), "matrix_stdev_columns", 2);
 for (i = 0; i < a.row; i++)
   for (j = 0; j < a.col; j++)
     result[j] += (a.values[i][j] - average[j]) * (a.values[i][j] - average[j]);
 for (j = 0; j < a.col; j++)
   result[j] = sqrt(result[j] / (a.row - 1));
 safe_free(average);
 return result;
}

double matrix_sum_row(matrix a, int index)
{
 /*!Last Changed 19.01.2008*/
 int j;
 double sum = 0.0;
 for (j = 0; j < a.col; j++)
   sum += a.values[index][j];
 return sum;
}

double* matrix_average_rows(matrix a)
{
	/*!Last Changed 23.12.2006*/
	int i, j;
	double* result = safecalloc(a.row, sizeof(double), "matrix_average_rows", 2);
	for (i = 0; i < a.row; i++)
		 for (j = 0; j < a.col; j++)
				 result[i] += a.values[i][j];
	for (i = 0; i < a.row; i++)
		 result[i] /= a.col;
	return result;
}

double matrix_max(matrix a)
{
 /*!Last Changed 07.04.2008*/
 int i, j;
 double result = a.values[0][0];
 for (i = 0; i < a.row; i++)
   for (j = 0; j < a.col; j++)
     if (a.values[i][j] > result)
       result = a.values[i][j];
 return result;
}

double matrix_min(matrix a)
{
 /*!Last Changed 07.04.2008*/
 int i, j;
 double result = a.values[0][0];
 for (i = 0; i < a.row; i++)
   for (j = 0; j < a.col; j++)
     if (a.values[i][j] < result)
       result = a.values[i][j];
 return result;
}

matrix matrix_transpose(matrix a)
{
 int i, j;
 matrix result = matrix_alloc(a.col, a.row);
 for (i = 0; i < a.row; i++)
   for (j = 0; j < a.col; j++)
     result.values[j][i] = a.values[i][j];
 return result;    
}

void matrix_columnwise_merge(matrix* a, matrix b)
{
	/*!Last Changed 23.12.2006*/
	int i, j;
 if (a->row != b.row)
  {
   printf(m1366);
   return;
  }
	for (i = 0; i < a->row; i++)
	 {
		 a->values[i] = realloc(a->values[i], (a->col + b.col) * sizeof(double));
			for (j = 0; j < b.col; j++)
				 a->values[i][a->col + j] = b.values[i][j];
	 }
	a->col += b.col;
}

void matrix_rowwise_merge(matrix* a, matrix b)
{
	int i, j;
 if (a->col != b.col)
  {
   printf(m1484);
   return;
  }
 a->values = realloc(a->values, (a->row + b.row) * sizeof(double*));
	for (i = 0; i < b.row; i++)
  {
   a->values[a->row + i] = safemalloc(a->col * sizeof(double), "matrix_rowwise_merge", 10);
   for (j = 0; j < b.col; j++)
     a->values[a->row + i][j] = b.values[i][j];
  }
	a->row += b.row;
}

void matrix_resize(matrix* a, Intervalptr interval)
{
 /*!Last Changed 22.12.2006*/
 int i, j, k; 
 for (i = 0; i < a->row; i++)
  {
   k = 0;
   for (j = 0; j < a->col; j++)
     if (is_in_interval(interval, j))
      {
       a->values[i][k] = a->values[i][j];
       k++;
      }
   a->values[i] = realloc(a->values[i], k * sizeof(double));
  }
 a->col = k;
}

matrix matrix_covariance(matrix a, double** averages)
{
	/*!Last Changed 23.12.2006*/
	matrix result;
	int i, j, k;
	result = matrix_alloc(a.col, a.col);
	*averages = matrix_average_columns(a);
 for (i = 0; i < a.row; i++)
		 for (j = 0; j < a.col; j++)
				 for (k = 0; k < a.col; k++)
						 result.values[j][k] += (a.values[i][j] - (*averages)[j]) * (a.values[i][k] - (*averages)[k]);
 for (j = 0; j < a.col; j++)
		 for (k = 0; k < a.col; k++)
				 result.values[j][k] /= (a.row - 1);
	return result;
}

matrix pooled_covariance(matrix a, int na, matrix b, int nb)
{
 /*!Last Changed 12.01.2009*/
 matrix result, tmp;
 result = matrix_copy(a);
 tmp = matrix_copy(b);
 matrix_multiply_constant(&result, na - 1);
 matrix_multiply_constant(&tmp, nb - 1);
 matrix_add(&result, tmp);
 matrix_divide_constant(&result, na + nb - 2);
 matrix_free(tmp);
 return result;
}

matrix matrix_correlation(matrix a)
{
	/*!Last Changed 23.12.2006*/
	matrix result;
	double* dummy;
	int i, j;
	result = matrix_covariance(a, &dummy);
	safe_free(dummy);
 for (i = 0; i < a.col; i++)
	 {
		 for (j = i + 1; j < a.col; j++)
			 {
     if (result.values[i][i] != 0.0 && result.values[j][j] != 0.0)
  				 result.values[i][j] /= sqrt(result.values[i][i] * result.values[j][j]);
     else
       result.values[i][j] = 0.0;
			  result.values[j][i] = result.values[i][j];
			 }
			result.values[i][i] = 1.0;
	 }
	return result;
}

matrix matrix_partial(matrix a, int rowstart, int rowend, int colstart, int colend)
{
 int i,j;
 matrix result = matrix_alloc(rowend - rowstart + 1, colend - colstart + 1);
 for (i = rowstart; i <= rowend; i++)
   for (j = colstart; j <= colend; j++)
     result.values[i - rowstart][j - colstart] = a.values[i][j];
 return result;
}

void matrix_identical(matrix* dest, matrix source)
{
	int i;
 if (dest->row != source.row || dest->col != source.col)
   printf(m1363);
 for (i = 0; i < source.row; i++)
		 memcpy(dest->values[i], source.values[i], source.col * sizeof(double));
}

matrix matrix_copy(matrix source)
{
 int i;
 matrix temp = matrix_alloc(source.row, source.col);
 for (i = 0; i < source.row; i++)
		 memcpy(temp.values[i], source.values[i], source.col * sizeof(double));
 return temp;
}

void matrix_free(matrix mat)
{
	free_2d((void**)mat.values, mat.row);
}

matrix identity(int n)
{
 int i;
 matrix result = matrix_alloc(n, n);
 for (i = 0; i < n; i++)
   result.values[i][i] = 1;
 return result;
}

void matrix_perturbate(matrix matrix1)
{
	/*!Last Changed 06.08.2007*/
	int i;
	for (i = 0; i < matrix1.row; i++)
		 matrix1.values[i][i] += produce_random_value(0, 0.00001);
}

double matrix_determinant(matrix matrix1)
{
 /*!Last Changed 25.02.2007*/
 int i, j, k;
 matrix mat;
 double ratio, det = 1.0;
 if (matrix1.row != matrix1.col)
  {
   printf(m1365, "Determinant");
   return 0;
  } 
 mat = matrix_copy(matrix1);
 for (i = 0; i < mat.row; i++) 
  {
   det *= mat.values[i][i];
   if (det == 0.0)
     break;
   for (j = i + 1; j < mat.row; j++)
    {
     ratio = mat.values[j][i] / mat.values[i][i];
     for (k = i; k < mat.col; k++)
       mat.values[j][k] = mat.values[j][k] - mat.values[i][k] * ratio;
    } 
  }
 matrix_free(mat);
 return det;
}

double mahalanobis_distance(matrix x, matrix y, matrix S)
{
 /*!Last Changed 12.01.2009*/
 int reversed;
 matrix diff, difftranspose, covinverse;
 double result;
 /* (x - mu) */
 diff = matrix_difference(x, y); 
 /* (x - mu)T */
 difftranspose = matrix_transpose(diff); 
 covinverse = matrix_copy(S);
 reversed = matrix_inverse(&covinverse); 
 if (!reversed)
  {
   matrix_free(covinverse);
   matrix_free(difftranspose);
   matrix_free(diff);
   printf("Matrix inverse problem\n");
   return -1;
  }
 /*(x - mu) sigma-1 (x - mu)T*/
 matrix_product(&diff, covinverse);
 matrix_free(covinverse);
 matrix_product(&diff, difftranspose); 
 matrix_free(difftranspose);
 result = diff.values[0][0];
 matrix_free(diff);
 return result;
}

int matrix_inverse(matrix *matrix1)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 double big;
 double dum,pivinv;
 int i,icol,irow,j,k,l,ll,n=matrix1->row;
 matrix b;
 int *indxc,*indxr,*ipiv;
 if (matrix1->row != matrix1->col)
  {
   printf(m1365, "Inverse");
   return 0;
  } 
 b = identity(n);
 indxc = (int *)safemalloc(n*sizeof(int), "matrix_inverse", 7);
 indxr = (int *)safemalloc(n*sizeof(int), "matrix_inverse", 8);
 ipiv = (int *)safemalloc(n*sizeof(int), "matrix_inverse", 9);
 for (j=1;j<=n;j++)
   ipiv[j-1] = 0;
 for (i=1;i<=n;i++)
  {
   big = 0.0;
   irow = -1;
   icol = -1;
   for (j=1;j<=n;j++)
     if (ipiv[j-1]!=1)
       for (k=1;k<=n;k++)
         if (ipiv[k-1]==0)
           if (fabs(matrix1->values[j-1][k-1])>=big)
            {
             big = fabs(matrix1->values[j-1][k-1]);
             irow = j;
             icol = k;
            }
   if (irow == -1 || icol == -1)
     return 0;         
   ipiv[icol-1]=ipiv[icol-1]+1;
   if (irow!=icol)
    {
     for (l=1;l<=n;l++)
      {
       dum=matrix1->values[irow-1][l-1];
       matrix1->values[irow-1][l-1]=matrix1->values[icol-1][l-1];
       matrix1->values[icol-1][l-1]=dum;
      }
     for (l=1;l<=n;l++)
      {
       dum = b.values[irow-1][l-1];
       b.values[irow-1][l-1] = b.values[icol-1][l-1];
       b.values[icol-1][l-1] = dum;
      }
    }
   indxr[i-1]=irow;
   indxc[i-1]=icol;
			if (matrix1->values[icol-1][icol-1] == 0)
			 {
     matrix_free(b);
     safe_free(ipiv);
     safe_free(indxr);
     safe_free(indxc);
				 return 0;
			 }
   pivinv = (1.0)/(matrix1->values[icol-1][icol-1]);
   matrix1->values[icol-1][icol-1]=1.0;
   for (l=1;l<=n;l++)
     matrix1->values[icol-1][l-1]=matrix1->values[icol-1][l-1]*pivinv;
   for (l=1;l<=n;l++)
     b.values[icol-1][l-1]=b.values[icol-1][l-1]*pivinv;
   for (ll=1;ll<=n;ll++)
     if (ll!=icol)
      {
       dum = matrix1->values[ll-1][icol-1];
       matrix1->values[ll-1][icol-1] = 0.0;
       for (l=1;l<=n;l++)
         matrix1->values[ll-1][l-1]=matrix1->values[ll-1][l-1]-matrix1->values[icol-1][l-1]*dum;
       for (l=1;l<=n;l++)
         b.values[ll-1][l-1]=b.values[ll-1][l-1]-dum*b.values[icol-1][l-1];
      }
  }
 for (l=n;l>=1;l--)
   if (indxr[l-1]!=indxc[l-1])
     for (k=1;k<=n;k++)
      {
       dum = matrix1->values[k-1][indxr[l-1]-1];
       matrix1->values[k-1][indxr[l-1]-1] = matrix1->values[k-1][indxc[l-1]-1];
       matrix1->values[k-1][indxc[l-1]-1] = dum;
      }
 matrix_free(b);
 safe_free(ipiv);
 safe_free(indxr);
 safe_free(indxc);
	return 1;
}

void jacobi(matrix matrix2,double **d,matrix *v)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*Computes the eigenvectors and eigenvalues of a SYMMETRIC matrix*/
 int j,iq,ip,i,n=matrix2.row;
 double tresh,theta,tau,t,sm,s,h,g,c;
 double *b;
 double *z;
 matrix matrix1 = matrix_copy(matrix2);
 (*v) = matrix_alloc(matrix2.row,matrix2.col);
 (*d) = (double *)safemalloc(n*sizeof(double), "jacobi", 7);
 b = (double *)safemalloc(n*sizeof(double), "jacobi", 8);
 z = (double *)safemalloc(n*sizeof(double), "jacobi", 9);
 for (ip=1;ip<=n;ip++)
  {
   for (iq=1;iq<=n;iq++)
     (*v).values[ip-1][iq-1]=0.0;
   (*v).values[ip-1][ip-1]=1.0;
  }
 for (ip=1;ip<=n;ip++)
  {
   b[ip-1]=matrix1.values[ip-1][ip-1];
   (*d)[ip-1]=b[ip-1];
   z[ip-1]=0.0;
  }
 for (i=1;i<=50;i++)
  {
   sm = 0.0;
   for (ip=1;ip<=n-1;ip++)
     for (iq=ip+1;iq<=n;iq++)
       sm += (double)fabs(matrix1.values[ip-1][iq-1]);
   if (sm==0.0)
     goto devam;
   if (i<4)
     tresh=0.2*sm/pow(n,2);
   else
     tresh=0.0;
   for (ip=1;ip<=n-1;ip++)
    {
     for (iq=ip+1;iq<=n;iq++)
      {
       g = 100.0*(double)fabs(matrix1.values[ip-1][iq-1]);
       if ((i > 4)&&((double)fabs((*d)[ip-1])+g==(double)fabs((*d)[ip-1]))&&((double)fabs((*d)[iq-1])+g==(double)fabs((*d)[iq-1])))
         matrix1.values[ip-1][iq-1]=0.0;
       else
         if ((double)fabs(matrix1.values[ip-1][iq-1])>tresh)
          {
           h = (*d)[iq-1]-(*d)[ip-1];
           if ((double)fabs(h)+g==(double)fabs(h))
             t=matrix1.values[ip-1][iq-1]/h;
           else
            {
             theta=0.5*h/matrix1.values[ip-1][iq-1];
             t = 1.0/((double)fabs(theta)+sqrt(1.0+pow(theta,2)));
             if (theta < 0.0)
               t = -t;
            }
           c= 1.0/sqrt(1+pow(t,2));
           s = t*c;
           tau = s/(1.0+c);
           h = t*matrix1.values[ip-1][iq-1];
           z[ip-1] -= h;
           z[iq-1] += h;
           (*d)[ip-1] -= h;
           (*d)[iq-1] += h;
           matrix1.values[ip-1][iq-1]=0.0;
           for (j=1;j<=ip-1;j++)
            {
             g=matrix1.values[j-1][ip-1];
             h=matrix1.values[j-1][iq-1];
             matrix1.values[j-1][ip-1]=g-s*(h+g*tau);
             matrix1.values[j-1][iq-1]=h+s*(g-h*tau);
            }
           for (j=ip+1;j<=iq-1;j++)
            {
             g=matrix1.values[ip-1][j-1];
             h=matrix1.values[j-1][iq-1];
             matrix1.values[ip-1][j-1] = g-s*(h+g*tau);
             matrix1.values[j-1][iq-1] = h+s*(g-h*tau);
            }
           for (j=iq+1;j<=n;j++)
            {
             g = matrix1.values[ip-1][j-1];
             h = matrix1.values[iq-1][j-1];
             matrix1.values[ip-1][j-1] = g-s*(h+g*tau);
             matrix1.values[iq-1][j-1] = h+s*(g-h*tau);
            }
           for (j=1;j<=n;j++)
            {
             g = (*v).values[j-1][ip-1];
             h = (*v).values[j-1][iq-1];
             (*v).values[j-1][ip-1] = g-s*(h+g*tau);
             (*v).values[j-1][iq-1] = h+s*(g-h*tau);
            }
          }
      }
    }
   for (ip=1;ip<=n;ip++)
    {
     b[ip-1] = b[ip-1]+z[ip-1];
     (*d)[ip-1] = b[ip-1];
     z[ip-1] = 0.0;
    }
  }
devam:
 safe_free(z);
 safe_free(b);
 matrix_free(matrix1);
}

void matrix_exchange_columns(matrix a, int col1, int col2)
{
 /*!Last Changed 02.02.2005*/
	double tmp;
	int i;
	for (i = 0; i < a.row; i++)
	 {
		 tmp = a.values[i][col1];
   a.values[i][col1] = a.values[i][col2];
			a.values[i][col2] = tmp;
	 }
}

void eigensort(double **eigenvalues, matrix *eigenvectors)
{
 int k, j, i, n = (*eigenvectors).row;
 double p;
 for (i = 0; i < n - 1; i++)
  {
   k = i;
   p = (*eigenvalues)[i];
   for (j = i + 1; j < n; j++)
    {
     if ((*eigenvalues)[j] >= p)
      {
       k = j;
       p = (*eigenvalues)[j];
      }
    }
   if (k != i)
    {
     (*eigenvalues)[k] = (*eigenvalues)[i];
     (*eigenvalues)[i] = p;
     matrix_exchange_columns(*eigenvectors, i, k);
    }
  }
}

double* balance(matrix a)
{
 int i, j;
 BOOLEAN done = BOOLEAN_FALSE; 
 double sqrdx = 4, r, c, g, f, s, *scale;
 scale = safemalloc(a.row * sizeof(double), "balance", 3);
 for (i = 0; i < a.row; i++)
   scale[i] = 1.0;
 while (!done) 
  {
   done = BOOLEAN_TRUE; 
   for (i = 0; i < a.row; i++) 
    {
     r = 0.0;
     c = 0.0; 
     for (j = 0; j < a.col; j++)
       if (j != i) 
        { 
         c += fabs(a.values[j][i]); 
         r += fabs(a.values[i][j]);
        } 
     if (c != 0.0 && r != 0.0) 
      {
       g = r / 2; 
       f = 1.0; 
       s = c + r; 
       while (c < g) 
        {
         f *= 2; 
         c *= sqrdx;
        } 
       g = r * 2; 
       while (c > g) 
        {
         f /= 2; 
         c /= sqrdx;
        } 
       if ((c + r) / f < 0.95 * s) 
        {
         done = BOOLEAN_FALSE; 
         g = 1.0 / f; 
         scale[i] *= f; 
         for (j = 0; j < a.row; j++) 
           a.values[i][j] *= g; 
         for (j = 0; j < a.row; j++) 
           a.values[j][i] *= f;
        }
      }
    }
  }
 return scale;
}

void balbak(matrix* eigenvectors, double* scale)
{
 int i, j;
 for (i = 0; i < eigenvectors->row; i++) 
   for (j = 0; j < eigenvectors->col; j++)
     eigenvectors->values[i][j] *= scale[i];
}

int* elmhes(matrix a)
{
 int m, i, j, *perm;
 double x, y;
 perm = safemalloc(a.row * sizeof(int), "elmhes", 3);
 for (m = 1; m < a.row - 1; m++) 
  {	
   x = 0.0;
   i = m;
   for (j = m; j < a.row; j++) 
    {	
     if (fabs(a.values[j][m-1]) > fabs(x)) 
      { 
       x = a.values[j][m-1];
       i = j;   
      }
    }
   perm[m] = i; 
   if (i != m) 
    {
     for (j = m - 1; j < a.row; j++) 
       swap_double(&(a.values[i][j]), &(a.values[m][j])); 
     for (j = 0; j < a.row; j++) 
       swap_double(&(a.values[j][i]), &(a.values[j][m]));
    } 
   if (x != 0.0) 
    {
     for (i = m + 1; i < a.row; i++) 
      { 
       y = a.values[i][m-1]; 
       if (y != 0.0) 
        {
         y /= x; 
         a.values[i][m - 1] = y; 
         for (j = m; j < a.row; j++) 
           a.values[i][j] -= y * a.values[m][j]; 
         for (j = 0; j < a.row; j++) 
           a.values[j][m] += y * a.values[j][i];
        }
      }
    }
  }
 return perm;
}

void eltran(matrix* eigenvectors, matrix a, int* perm)
{
 int mp, k, i, j;
 for (mp = a.row - 2; mp > 0; mp--)
  {
   for (k = mp + 1; k < a.row; k++)
     eigenvectors->values[k][mp] = a.values[k][mp - 1];
   i = perm[mp];
   if (i != mp)
    {
     for (j = mp; j < a.row; j++)
      {
       eigenvectors->values[mp][j] = eigenvectors->values[i][j];
       eigenvectors->values[i][j] = 0.0;
      }
     eigenvectors->values[i][mp] = 1.0;
    }
  }
}

double* hqr2(matrix* eigenvectors, matrix a)
{
 int nn, m, l, k, j, its, i, mmin, na; 
 double z, y, x, w, v, u, t, s, r, q, p, anorm = 0.0, *eigenvalues;
 double EPS = 2.22045e-016;
 eigenvalues = safemalloc(a.row * sizeof(double), "hqr2", 5);
 for (i = 0; i < a.row; i++) 
   for (j = imax(i - 1,0); j < a.row; j++)
     anorm += fabs(a.values[i][j]);
 nn = a.row - 1; 
 t = 0.0; 
 while (nn >= 0) 
  {
   its = 0; 
   do {
     for (l = nn; l > 0; l--) 
      { 
       s = fabs(a.values[l - 1][l - 1]) + fabs(a.values[l][l]);	
       if (s == 0.0) 
         s = anorm; 
       if (fabs(a.values[l][l - 1]) <= EPS * s) 
        {
         a.values[l][l - 1] = 0.0; 
         break;
        }
      } 
      x = a.values[nn][nn]; 
      if (l == nn) 
       {	
        eigenvalues[nn] = a.values[nn][nn] = x + t;
        nn--; 
       } 
      else 
       {
        y = a.values[nn - 1][nn-1]; 
        w = a.values[nn][nn-1] * a.values[nn-1][nn]; 
        if (l == nn - 1) 
         {	
          p = 0.5 * (y - x); 
          q = p * p + w;
          z = sqrt(fabs(q)); 
          x += t; 
          a.values[nn][nn] = x; 
          a.values[nn - 1][nn - 1] = y + t; 
          if (q >= 0.0) 
           {
            z = p + change_sign(z,p); 
            eigenvalues[nn - 1] = eigenvalues[nn] = x + z; 
            if (z != 0.0) 
              eigenvalues[nn] = x - w / z; 
            x = a.values[nn][nn - 1]; 
            s = fabs(x) + fabs(z); 
            p = x / s; 
            q = z / s; 
            r = sqrt(p * p + q * q); 
            p /= r; 
            q /= r; 
            for (j = nn - 1; j < a.row; j++) 
             {	
              z = a.values[nn - 1][j]; 
              a.values[nn - 1][j] = q * z + p * a.values[nn][j]; 
              a.values[nn][j] = q * a.values[nn][j] - p * z;
             } 
            for (i = 0; i <= nn; i++) 
             {	
              z = a.values[i][nn - 1]; 
              a.values[i][nn - 1] = q * z + p * a.values[i][nn]; 
              a.values[i][nn] = q * a.values[i][nn] - p * z;
             } 
            for (i = 0; i < a.row; i++) 
             {	
              z = eigenvectors->values[i][nn - 1]; 
              eigenvectors->values[i][nn - 1] = q * z + p * eigenvectors->values[i][nn]; 
              eigenvectors->values[i][nn] = q * eigenvectors->values[i][nn] - p * z;
             } 
           } 
          else 
           {
            printf("Complex eigenvalues. Exiting\n");
            return NULL;
           }
          nn -= 2; 
         } 
        else 
         {	
          if (its == 30) 
           {
            printf("Too many iterations. Exiting\n");
            return NULL;
           } 
          if (its == 10 || its == 20) 
           {	
            t += x; 
            for (i = 0; i < nn + 1; i++) 
              a.values[i][i] -= x; 
            s = fabs(a.values[nn][nn - 1]) + fabs(a.values[nn - 1][nn - 2]); 
            y = x = 0.75 * s; 
            w = -0.4375 * s * s;
           } 
          its++; 
          for (m = nn - 2; m >= l; m--) 
           {
            z = a.values[m][m]; 
            r = x - z; 
            s = y - z; 
            p = (r * s - w) / a.values[m + 1][m] + a.values[m][m + 1]; 
            q = a.values[m + 1][m + 1] - z - r - s; 
            r = a.values[m + 2][m + 1]; 
            s = fabs(p) + fabs(q) + fabs(r);
            p /= s; 
            q /= s; 
            r /= s; 
            if (m == l) 
              break; 
            u = fabs(a.values[m][m - 1]) * (fabs(q) + fabs(r)); 
            v = fabs(p) * (fabs(a.values[m - 1][m - 1]) + fabs(z) + fabs(a.values[m + 1][m + 1]));
            if (u <= EPS * v) 
              break;
           } 
          for (i = m; i < nn - 1; i++) 
           {
            a.values[i + 2][i] = 0.0; 
            if (i != m) 
              a.values[i + 2][i - 1] = 0.0;
           } 
          for (k = m; k < nn; k++) 
           { 
            if (k != m) 
             { 
              p = a.values[k][k - 1];	
              q = a.values[k + 1][k - 1];	
              r = 0.0; 
              if (k + 1 != nn) 
                r = a.values[k + 2][k - 1]; 
              if ((x = fabs(p) + fabs(q) + fabs(r)) != 0.0) 
               {
                p /= x;	
                q /= x;	
                r /= x;
               }
             } 
            if ((s = change_sign(sqrt(p * p + q * q + r * r), p)) != 0.0) 
             {
              if (k == m) 
               { 
                if (l != m)
                  a.values[k][k - 1] = -a.values[k][k - 1]; 
               } 
              else
                a.values[k][k - 1] = -s * x; 
              p += s;
              x = p / s; 
              y = q / s; 
              z = r / s; 
              q /= p; 
              r /= p; 
              for (j = k; j < a.row; j++) 
               {
                p = a.values[k][j] + q * a.values[k+1][j]; 
                if (k+1 != nn) 
                 {
                  p += r * a.values[k + 2][j]; 
                  a.values[k + 2][j] -= p * z;
                 } 
                a.values[k + 1][j] -= p * y; 
                a.values[k][j] -= p * x;
               } 
              mmin = nn < k + 3 ? nn : k + 3; 
              for (i = 0; i < mmin + 1; i++) 
               {
                p = x * a.values[i][k] + y * a.values[i][k + 1]; 
                if (k + 1 != nn) 
                 {
                  p += z * a.values[i][k + 2]; 
                  a.values[i][k + 2] -= p * r;
                 } 
                a.values[i][k + 1] -= p * q; 
                a.values[i][k] -= p;
               } 
              for (i = 0; i < a.row; i++) 
               {
                p = x * eigenvectors->values[i][k] + y * eigenvectors->values[i][k + 1]; 
                if (k + 1 != nn) 
                 {
                  p += z * eigenvectors->values[i][k + 2]; 
                  eigenvectors->values[i][k + 2] -= p * r;
                 } 
                eigenvectors->values[i][k + 1] -= p * q; 
                eigenvectors->values[i][k] -= p;
               }
             } 
           } 
         }
       }
   }while (l + 1 < nn);
 }
 if (anorm != 0.0) 
  { 
   for (nn = a.row - 1; nn >= 0; nn--) 
    {
     p = eigenvalues[nn];
     na = nn - 1; 
     m = nn; 
     a.values[nn][nn] = 1.0; 
     for (i = nn - 1; i >= 0;i--) 
      {
       w = a.values[i][i] - p; 
       r = 0.0; 
       for (j = m; j <= nn; j++)
         r += a.values[i][j] * a.values[j][nn]; 
       m = i;
       t = w;
       if (t == 0.0) 
         t = EPS * anorm;
       a.values[i][nn] = -r / t; 
       t = fabs(a.values[i][nn]); 
       if (EPS * t * t > 1)
         for (j = i; j <= nn; j++) 
           a.values[j][nn] /= t;
      }
    }
   for (j = a.row - 1; j >= 0; j--)
     for (i = 0; i < a.row; i++)
      {
       z = 0.0;
       for (k = 0; k <= j; k++)
         z += eigenvectors->values[i][k] * a.values[k][j];
       eigenvectors->values[i][j] = z;
      }
  }
 return eigenvalues;
}

double* find_eigenvalues_eigenvectors_unsymmetric(matrix* eigenvectors, matrix a)
{
 double *eigenvalues = NULL, *scale;
 int i, *permutation;
 (*eigenvectors) = matrix_alloc(a.row, a.col); 
 scale = balance(a); 
 permutation = elmhes(a); 
 for (i = 0; i < a.row; i++) 
   eigenvectors->values[i][i] = 1.0;
 eltran(eigenvectors, a, permutation); 
 eigenvalues = hqr2(eigenvectors, a);
 if (eigenvalues == NULL)
   return NULL; 
 balbak(eigenvectors, scale); 
 eigensort(&eigenvalues, eigenvectors);
	return eigenvalues;
}

double* find_eigenvalues_eigenvectors_symmetric(matrix* eigenvectors, matrix a)
{
 double *eigenvalues = NULL;
 jacobi(a, &eigenvalues, eigenvectors);
 eigensort(&eigenvalues, eigenvectors);
	return eigenvalues;
}

double* find_eigenvalues_eigenvectors(matrix* eigenvectors, matrix cov, int* newdimension, double range)
{
 int i, howmany;
 double total = 0 ,sum = 0, *eigenvalues = NULL;
 jacobi(cov, &eigenvalues, eigenvectors);
 eigensort(&eigenvalues, eigenvectors);
 for (i = 0; i < eigenvectors->row; i++)
   total += eigenvalues[i];
	howmany = eigenvectors->row - 1;
 for (i = 0; i < eigenvectors->row; i++)
  {
   sum += eigenvalues[i];
   if ((sum / total) > range)
    {
     howmany = i;
     break;
    }
  }
	*newdimension = howmany;
	return eigenvalues;
}

matrix svdcmp(matrix a, int m, int n, double* w)
{
/*Given a matrix a[1..m][1..n], this routine computes its singular value decomposition, A = UWV T. Thematrix U replaces a on output. The diagonal matrix of singular values W is output as a vector w[1..n]. Thematrix V (not the transpose V T ) is output as v[1..n][1..n].*/
 int flag, i, its, j, jj, k, l, nm;
 double anorm, c, f, g, h, s, scale, x, y, z, *rv1, tmp;
	matrix v;
 rv1 = safemalloc(n * sizeof(double), "svdcmp", 4);
	v = matrix_alloc(n, n);
 g = 0.0;
	scale = 0.0;
	anorm = 0.0; 
 for (i = 1; i <= n; i++) 
	 {
   l = i + 1;
   rv1[i - 1] = scale * g;
   g = 0.0;
			s = 0.0;
			scale = 0.0;
   if (i <= m) 
			 {
     for (k = i; k <= m; k++) 
						 scale += fabs(a.values[k - 1][i - 1]);
     if (scale) 
					 {
       for (k = i; k <= m; k++) 
							 {
         a.values[k - 1][i - 1] /= scale;
         s += a.values[k - 1][i - 1] * a.values[k - 1][i - 1];
						  }
       f = a.values[i - 1][i - 1];
       g = change_sign(sqrt(s), f);
       h = f * g - s;
       a.values[i - 1][i - 1] = f - g;
       for (j = l; j <= n; j++) 
							 {
         for (s = 0.0, k = i; k <= m; k++) 
										 s += a.values[k - 1][i - 1]*a.values[k - 1][j - 1];
         f = s / h;
         for (k = i; k <= m; k++) 
										 a.values[k - 1][j - 1] += f*a.values[k - 1][i - 1];
							 }
       for (k = i; k <= m; k++) 
								 a.values[k - 1][i - 1] *= scale;
					 }
			 }
   w[i - 1] = scale * g;
   g = 0.0;
			s = 0.0;
			scale = 0.0;
   if (i <= m && i != n) 
			 {
     for (k = l; k <= n; k++) 
						 scale += fabs(a.values[i - 1][k - 1]);
     if (scale) 
					 {
       for (k = l; k <= n; k++) 
							 {
         a.values[i - 1][k - 1] /= scale;
         s += a.values[i - 1][k - 1] * a.values[i - 1][k - 1];
						  }
       f = a.values[i - 1][l - 1];
       g = -change_sign(sqrt(s), f);
       h = f * g - s;
       a.values[i - 1][l - 1] = f - g;
       for (k = l; k <= n; k++) 
								 rv1[k - 1] = a.values[i - 1][k - 1] / h;
       for (j = l; j <= m; j++) 
							 {
         for (s = 0.0, k = l; k <= n; k++) 
										 s += a.values[j - 1][k - 1] * a.values[i - 1][k - 1];
         for (k = l; k <= n; k++) 
										 a.values[j - 1][k - 1] += s * rv1[k - 1];
							 }
       for (k = l; k <= n; k++) 
								 a.values[i - 1][k - 1] *= scale;
					 }
			 }
   anorm = fmax(anorm, (fabs(w[i - 1]) + fabs(rv1[i - 1])));
	 }
 for (i = n; i >= 1; i--) 
	 { 
   if (i < n) 
			 {
     if (g) 
					 {
       for (j = l; j <= n; j++) 
         v.values[j - 1][i - 1] = (a.values[i - 1][j - 1] / a.values[i - 1][l - 1]) / g;
       for (j = l; j <= n; j++) 
							 {
         for (s = 0.0, k = l; k <= n; k++) 
										 s += a.values[i - 1][k - 1] * v.values[k - 1][j - 1];
         for (k = l; k <= n; k++) 
										 v.values[k - 1][j - 1] += s * v.values[k - 1][i - 1];
							 }
					 }
     for (j = l; j <= n; j++) 
					 {
						 v.values[i - 1][j - 1] = 0.0;
							v.values[j - 1][i - 1] = 0.0;
					 }
			 }
   v.values[i - 1][i - 1] = 1.0;
   g = rv1[i - 1];
   l = i;
	 }
 for (i = imin(m, n); i >= 1; i--) 
	 { 
   l = i + 1;
   g = w[i - 1];
   for (j = l; j <= n; j++) 
				 a.values[i - 1][j - 1] = 0.0;
   if (g) 
			 {
     g = 1.0 / g;
     for (j = l; j <= n; j++) 
					 {
       for (s = 0.0, k = l; k <= m; k++) 
								 s += a.values[k - 1][i - 1] * a.values[k - 1][j - 1];
       f = (s / a.values[i - 1][i - 1]) * g;
       for (k = i; k <= m; k++) 
								 a.values[k - 1][j - 1] += f * a.values[k - 1][i - 1];
					 }
     for (j = i; j <= m; j++) 
						 a.values[j - 1][i - 1] *= g;
			 } 
		 else 
				 for (j = i; j <= m; j++) 
						 a.values[j - 1][i - 1] = 0.0;
   ++(a.values[i - 1][i - 1]);
	 }
 for (k = n; k >= 1; k--) 
	 {  
		 for (its = 1; its <= 30; its++) 
			 {
     flag = 1;
     for (l = k; l >= 1; l--) 
					 { 
       nm = l - 1; 
       if ((double)(fabs(rv1[l - 1]) + anorm) == anorm) 
							 {
         flag = 0;
         break;
							 }
       if ((double)(fabs(w[nm - 1]) + anorm) == anorm) 
								 break;
					 }
     if (flag) 
					 {
       c = 0.0; 
       s = 1.0;
       for (i = l; i <= k; i++) 
							 {
         f = s * rv1[i - 1];
         rv1[i - 1] = c * rv1[i - 1];
         if ((double)(fabs(f) + anorm) == anorm) 
										 break;
         g = w[i - 1];
         h = pythag(f,g);
         w[i - 1] = h;
         h = 1.0 / h;
         c = g * h;
         s = -f * h;
         for (j = 1; j <= m; j++) 
									 {
           y = a.values[j - 1][nm - 1];
           z = a.values[j - 1][i - 1];
           a.values[j - 1][nm - 1] = y * c + z * s;
           a.values[j - 1][i - 1] = z * c - y * s;
									 }
							 }
					 }
     z = w[k - 1];
     if (l == k) 
					 { 
       if (z < 0.0) 
							 {
         w[k - 1] = -z;
         for (j = 1; j <= n; j++) 
										 v.values[j - 1][k - 1] = -v.values[j - 1][k - 1];
							 }
       break;
					 }
     if (its == 30) 
						 printf(m1304);
     x = w[l - 1]; 
     nm = k - 1;
     y = w[nm - 1];
     g = rv1[nm - 1];
     h = rv1[k - 1];
     f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
     g = pythag(f, 1.0);
     f = ((x - z) * (x + z) + h * ((y / ( f + change_sign(g, f))) - h)) / x;
     c = 1.0; 
					s = 1.0; 
     for (j=l;j<=nm;j++) 
					 {
       i = j + 1;
       g = rv1[i - 1];
       y = w[i - 1];
       h = s * g;
       g = c * g;
       z = pythag(f, h);
       rv1[j - 1] = z;
       c = f / z;
       s = h / z;
       f = x * c + g * s;
       g = g * c - x * s;
       h = y * s;
       y *= c;
       for (jj = 1; jj <= n; jj++) 
							 {
         x = v.values[jj - 1][j - 1];
         z = v.values[jj - 1][i - 1];
         v.values[jj - 1][j - 1] = x * c + z * s;
         v.values[jj - 1][i - 1] = z * c - x * s;
							 }
       z = pythag(f, h);
       w[j - 1] = z; 
       if (z) 
							 {
         z = 1.0 / z;
         c = f * z;
         s = h * z;
							 }
       f = c * g + s * y;
       x = c * y - s * g;
       for (jj = 1; jj <= m; jj++) 
							 {
         y = a.values[jj - 1][j - 1];
         z = a.values[jj - 1][i - 1];
         a.values[jj - 1][j - 1] = y * c + z * s;
         a.values[jj - 1][i - 1] = z * c - y * s;
							 }
					 }
     rv1[l - 1] = 0.0;
     rv1[k - 1] = f;
     w[k - 1] = x;
    }
  }
 safe_free(rv1);
	for (i = 0; i < n; i++)
		 for (j = i + 1; j < n; j++)
				 if (w[i] < w[j])
					 {
       tmp = w[i];
						 w[i] = w[j];
							w[j] = tmp;
						 matrix_exchange_columns(a, i, j);
						 matrix_exchange_columns(v, i, j);
					 }
 return v;
}
