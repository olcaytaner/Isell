#ifndef __perceptron_H
#define __perceptron_H

double*  one_dimensional_initialize(int dim,double constant);
double   sigmoid(double r);
void     softmax(double *myarray,int size);
double** two_dimensional_initialize(int dim1,int dim2,double constant);

#endif
