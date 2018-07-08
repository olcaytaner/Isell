#ifndef __svmkernel_H
#define __svmkernel_H
#include "svm.h"

struct kernel{
	             Svm_node **x;
	             double *x_square;
	             KERNEL_TYPE kernel_type;
	             int degree;
	             double gamma;
	             double coef0;
};

typedef struct kernel Kernel;
typedef Kernel* Kernelptr;

double    dot(Svm_nodeptr px, Svm_nodeptr py);
Kernelptr empty_kernel(int l, Svm_node** x, int kernel_type, int degree, double gamma, double coef0);
void      free_kernel(Kernelptr k);
double    k_function(Svm_nodeptr x, Svm_nodeptr y, Svm_parameter param);
double    k_function_normalized(Svm_nodeptr x, Svm_nodeptr y, Svm_parameter param);
double    kernel_function(Kernelptr k, int i, int j);
double    kernel_function_normalized(Kernelptr k, int i, int j);
double    kernel_linear(Kernelptr k, int i, int j);
double    kernel_poly(Kernelptr k, int i, int j);
double    kernel_rbf(Kernelptr k, int i, int j);
double    kernel_sigmoid(Kernelptr k, int i, int j);
void      kernel_swap_index(Kernelptr k, int i, int j);

#endif
