#ifndef __libmath_H
#define __libmath_H

#define DBL_DELTA 1e-8

int      between(int x, int left, int right);
int      binom(int m, int n);
double   binomdouble(int m, int n);
double   change_sign(double a, double b);
int      dequal(double x, double y);
int      dgreat(double x, double y);
int      dless(double x, double y);
int      equal(int cr, int cy);
int      factorial(int n);
double   fmax(double a, double b);
double   fmin(double a, double b);
double   fpow(double x,int y);
int      imax(int a, int b);
int      imin(int a, int b);
double   log10(double x);
double   log2(double x);
double   pythag(double a, double b);
double   safeexp(double x);
double   safelog(double x);

#endif

