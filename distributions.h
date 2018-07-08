#ifndef __distributions_H
#define __distributions_H

#include "typedef.h"

#define Z_MAX 6.0
#define Z_EPSILON 0.000001
#define CHI_EPSILON 0.000001
/* accuracy of critchi approximation */
#define CHI_MAX 99999.0
/* maximum chi square value */
#define LOG_SQRT_PI 0.5723649429247000870717135
/* log (sqrt (pi)) */
#define I_SQRT_PI 0.5641895835477562869480795
/* 1 / sqrt (pi) */
#define BIGX 200.0
/* max value to represent exp (x) */
#define ex(x) (((x) < -BIGX) ? 0.0 : exp (x))
#define I_PI 0.3183098861837906715377675
#define F_EPSILON 0.000001
/* accuracy of critf approximation */
#define F_MAX 9999.0
/* maximum F ratio */

double    beta(double* x, int count);
double    chi_square(int freedom, double x);
double    chi_square_inverse (double p,int freedom);
double    dirichlet(Instanceptr x, double* alpha);
double    f_distribution(double F,int freedom1,int freedom2);
double    f_distribution_inverse(double p, int freedom1, int freedom2);
double    gamma(double x);
double    gammln(double x);
double    kolmogorov_one_sample(int n, double d);
double    kolmogorov_two_sample(int m, int n, double D);
double    laplacian(double x, double mean, double stdev);
double    normal(double x,int upper);
double    qtrng0(double p, double v, double r);
double    studentized_range(double q, double v, double r);
double    studentized_range_inverse(double p, double v, double r);
double    t_distribution(double T,int freedom);
double    t_distribution_inverse(double p,int freedom);
double    z_inverse (double p);
double    z_normal(double z);

#endif
