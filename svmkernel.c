#include <math.h>
#include <stdio.h>
#include "messages.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "svmkernel.h"
#include "typedef.h"

extern Datasetptr current_dataset;

/**
 * Swaps the kernel values of the instance i with instance j. Swaps also the square values if they are als
 * @param[in] k Kernel structure storing kernel values
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 */
void kernel_swap_index(Kernelptr k, int i, int j)
{
	/*Last Changed 26.04.2005*/
	Svm_nodeptr tmp;
	tmp = k->x[i];
	k->x[i] = k->x[j];
	k->x[j] = tmp;
	if (k->x_square != NULL) 
		 swap_double(&(k->x_square[i]), &(k->x_square[j]));
}

/**
 * Calculates the dot product of two instances. Dot product = \sum_i x1_ix2_i.
 * @param[in] px Start pointer of the first instance
 * @param[in] py Start pointer of the second instance
 * @return Dot product of two instances.
 */
double dot(Svm_nodeptr px, Svm_nodeptr py)
{
	/*Last Changed 26.04.2005*/
	double sum = 0;
	while (px->index != -1 && py->index != -1)
	 {
 		if (px->index == py->index)
			 {
		  	sum += px->value * py->value;
		  	px++;
			  py++;
			 }
		 else
			 {
		  	if (px->index > py->index)
			  	 py++;
			  else
				   px++;
			 }			
	}
	return sum;
}

/**
 * Calculates linear kernel value for two instances using dot product.
 * @param[in] k Kernel structure
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 * @return Linear kernel value for two instances.
 */
double kernel_linear(Kernelptr k, int i, int j)
{
	/*Last Changed 26.04.2005*/
	return dot(k->x[i], k->x[j]);
}
	
/**
 * Calculates polynomial kernel value for two instances. (gamma \sum_i x1_ix2_i)^d + x_0
 * @param[in] k Kernel structure
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 * @return Polynomial kernel value for two instances.
 */
double kernel_poly(Kernelptr k, int i, int j)
{
	/*Last Changed 26.04.2005*/
	return pow(k->gamma * dot(k->x[i], k->x[j]) + k->coef0, k->degree);
}

/**
 * Calculates radial basis kernel value for two instances. e^(-gamma * (x1^2 + x2^2 - 2\sum_i x1_ix2_i))
 * @param[in] k Kernel structure
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 * @return Radial basis kernel value for two instances.
 */
double kernel_rbf(Kernelptr k, int i, int j)
{
	/*Last Changed 26.04.2005*/
	return exp(-k->gamma * (k->x_square[i] + k->x_square[j] - 2 * dot(k->x[i], k->x[j])));
}
	
/**
 * Calculates sigmoid kernel value for two instances. tanh(gamma * \sum_i x1_ix2_i + x_0)
 * @param[in] k Kernel structure
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 * @return Sigmoid kernel value for two instances.
 */
double kernel_sigmoid(Kernelptr k, int i, int j)
{
	/*Last Changed 26.04.2005*/
	return tanh(k->gamma * dot(k->x[i], k->x[j]) + k->coef0);
}

double kernel_predefined(Kernelptr k, int i, int j)
{
	return current_dataset->kernel->values[(int)k->x[i]->value][(int)k->x[j]->value];
}

/**
 * Calculates kernel value for two instances according to the kernel function. Calls respective kernel functions such as linear kernel, polynomial kernel, rbf kernel, etc.
 * @param[in] k Kernel structure
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 * @return Kernel value for two instances.
 */
double kernel_function(Kernelptr k, int i, int j)
{
	/*Last Changed 26.04.2005*/
	switch (k->kernel_type)
	 {
	  case    KLINEAR :return kernel_linear(k, i, j);
				                break;
	  case    KPOLY   :return kernel_poly(k, i, j);
				                break;
			case    KRBF    :return kernel_rbf(k, i, j);
				                break;
			case    KSIGMOID:return kernel_sigmoid(k, i, j);
				                break;
   case KPREDEFINED:return kernel_predefined(k, i, j);
                    break;
			default         :printf(m1232, k->kernel_type);
                 return 0;
				             break;
	 }
}

/**
 * Calculates normalized kernel value for two instances according to the kernel function. Calls respective kernel functions such as linear kernel, polynomial kernel, rbf kernel, etc.
 * @param[in] k Kernel structure
 * @param[in] i Index of the first instance
 * @param[in] j Index of the second instance
 * @return Normalized kernel value for two instances.
 */
double kernel_function_normalized(Kernelptr k, int i, int j)
{
 double Kxx, Kyy;
 Kxx = kernel_function(k, i, i);
 Kyy = kernel_function(k, j, j);
 if (Kxx != 0 && Kyy != 0)
   return kernel_function(k, i, j) / sqrt(Kxx * Kyy);
 return kernel_function(k, i, j); 
}

/**
 * Constructor for kernel structure. Allocates memory for kernel structure, initializes kernel parameters, allocates memory for squares (dot product of the instances with itselves)
 * @param[in] l Number of instances
 * @param[in] x Data set
 * @param[in] kernel_type Kernel type can be KLINEAR for linear, KPOLY for polynomial, KRBF for rbf and KSIGMOID for sigmoid kernels.
 * @param[in] degree Degree of the polynomial kernel
 * @param[in] gamma Multiplication constant for sigmoid, rbf and polynomial kernels.
 * @param[in] coef0 Additive constant for sigmoid and polynomial kernels.
 * @return Allocated and initialized kernel structure
 */
Kernelptr empty_kernel(int l, Svm_node** x, int kernel_type, int degree, double gamma, double coef0)
{
	/*Last Changed 26.04.2005*/
	Kernelptr result;
	int i;
	result = safemalloc(sizeof(Kernel), "empty_kernel", 3);
	result->coef0 = coef0;
	result->degree = degree;
	result->gamma = gamma;
	result->kernel_type = kernel_type;
	result->x = safemalloc(sizeof(Svm_nodeptr) * l, "empty_kernel", 8);
	memcpy(result->x, x, sizeof(Svm_nodeptr) * l);
	if (kernel_type == KRBF)
	 {
		 result->x_square = safemalloc(sizeof(double) * l, "empty_kernel", 12);
			for (i = 0; i < l; i++)
				 result->x_square[i] = dot(x[i], x[i]);
	 }
	else
		 result->x_square = NULL;
	return result;
}

/**
 * Destructor for kernel structure. Deallocates memory allocated for kernel structure, data points.
 * @param[in] k Kernel structure
 */
void free_kernel(Kernelptr k)
{
	/*Last Changed 26.04.2005*/
	safe_free(k->x);
	if (k->kernel_type == KRBF)
  	safe_free(k->x_square);
 safe_free(k);
}

/**
 * Calculates kernel value for two instances x and y according to the kernel. This function is different from kernel_function in the way that kernel_function calculates kernel value for two instances of the data array by specifying the indexes of those two instances. On the other hand, k_function calculates kernel value for two usual instances.
 * @param[in] x First instance
 * @param[in] y Second instance
 * @param[in] param Kernel parameters including kernel type, gamma, x_0 etc.
 * @return Kernel value for two instances
 */
double k_function(Svm_nodeptr x, Svm_nodeptr y, Svm_parameter param)
{
	/*Last Changed 26.04.2005*/
	double sum, d;
	switch (param.kernel_type)
	 {
 		case    KLINEAR :return dot(x, y);
				                break;
		 case    KPOLY   :return pow(param.gamma * dot(x, y) + param.coef0, param.degree);
				                break;
		 case    KRBF    :sum = 0;
               		 	 while (x->index != -1 && y->index !=-1)
					   												 {
             		  	   	if (x->index == y->index)
																   			 {
					                   d = x->value - y->value;
					                   sum += d*d;
					                   ++x;
					                   ++y;
						   													 }
				                  else
		   																	 {
					                   if (x->index > y->index)
																		   			 {	
						                    sum += y->value * y->value;
						                    ++y;
												   									 }
					                   else
								   									  			{
						                    sum += x->value * x->value;
				   		                 ++x;
		   																			 }
   																			 }
																   	 }
			                 while (x->index != -1)
												   					 { 
				                  sum += x->value * x->value;
				                  ++x;
						   											 }
			                 while (y->index != -1)
		   															 {
   				               sum += y->value * y->value;
				                  ++y;
																   	 }
			                 return exp(-param.gamma * sum);
										 	   				 break;
		 case    KSIGMOID:return tanh(param.gamma * dot(x, y) + param.coef0);
			                 break;
   case KPREDEFINED:return current_dataset->kernel->values[(int)x->value][(int)y->value];
                    break;
		 default         :printf(m1232, param.kernel_type);
                    return 0;				            
	 }
}

/**
 * Calculates normalized kernel value for two instances x and y according to the kernel. This function is different from kernel_function in the way that kernel_function calculates kernel value for two instances of the data array by specifying the indexes of those two instances. On the other hand, k_function calculates kernel value for two usual instances.
 * @param[in] x First instance
 * @param[in] y Second instance
 * @param[in] param Kernel parameters including kernel type, gamma, x_0 etc.
 * @return Normalized kernel value for two instances
 */
double k_function_normalized(Svm_nodeptr x, Svm_nodeptr y, Svm_parameter param)
{
 double Kxx, Kyy;
 Kxx = k_function(x, x, param);
 Kyy = k_function(y, y, param);
 if (Kxx != 0 && Kyy != 0)
   return k_function(x, y, param) / sqrt(Kxx * Kyy);
 return k_function(x, y, param);
}
