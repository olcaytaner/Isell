#include <math.h>
#include "libmath.h"

int between(int x, int left, int right)
{
 if (x >= left && x <= right)
   return 1;
 return 0;
}

/**
 * Changes sign of the first variable according to the sign of the second variable.
 * @param[in] a First variable
 * @param[in] b Second variable
 * @return If the second variable is negative, returns the first variable as negative. If the second variable is positive, returns the the first variable as positive.
 */
double change_sign(double a, double b)
{
 /*!Last Changed 02.02.2005*/
 if (b >= 0)
   return fabs(a);
 else
   return -fabs(a);
}

int dequal(double x, double y)
{
 if (((x - DBL_DELTA) <= y) && ((x + DBL_DELTA) >= y))
   return 1;
 return 0;
}

int dgreat(double x, double y)
{
 if ((x-DBL_DELTA) > y)
   return 1;
 return 0;
}

int dless(double x, double y)
{
 if (x < (y-DBL_DELTA))
   return 1;
 return 0;
}

int equal(int cr, int cy)
{
 if (cr == cy)
  return 1;
 else
  return 0;
}

/**
 * Calculates maximum of two doubles
 * @param[in] a First double
 * @param[in] b Second double
 * @return Maximum of a and b
 */
double fmax(double a, double b)
{
 /*!Last Changed 02.02.2005*/
 if (a > b)
   return a;
 else
   return b;
}

/**
 * Calculates minimum of two doubles
 * @param[in] a First double
 * @param[in] b Second double
 * @return Minimum of a and b
 */
double fmin(double a, double b)
{
 /*!Last Changed 28.04.2005*/
 if (a < b)
   return a;
 else
   return b;
}

double fpow(double x,int y)
{
 double result,t=x;
 if (y%2)
   result=x;
 else
   result=1;
 y/=2;
 while (y>0)
  {
   t=t*t;
   if (y%2)
     result*=t;
   y/=2;
  }
 return result;
}

/**
 * Calculates minimum of two integers
 * @param[in] a First integer
 * @param[in] b Second integer
 * @return Minimum of a and b
 */
int imin(int a, int b)
{
 /*!Last Changed 02.02.2005*/
 if (a < b)
   return a;
 else
   return b;
}

/**
 * Calculates maximum of two integers
 * @param[in] a First integer
 * @param[in] b Second integer
 * @return Maximum of a and b
 */
int imax(int a, int b)
{
 if (a > b)
   return a;
 else
   return b;
}

/**
 * Calculates \sqrt{a^2 + b^2} with less loss of precision. If a > b,
 * @param[in] a a
 * @param[in] b b
 * @return \sqrt{a^2 + b^2}
 */
double pythag(double a, double b)
{
 double absa, absb;
 absa = fabs(a);
 absb = fabs(b);
 if (absa > absb) 
   return absa * sqrt(1.0 + fpow(absb / absa, 2));
 else 
   return (absb == 0.0 ? 0.0 : absb * sqrt(1.0 + fpow(absa / absb, 2)));
}

double safeexp(double x)
{
 if (x>300)
   return exp(300);
 else
   return exp(x);
}

double log10(double x)
{
 return log(x)/log(10);
}

double log2(double x)
{
 return log(x)/log(2);
}

double safelog(double x)
{
 /*Last Changed 07.04.2007*/
 if (dequal(x, 0.0))
   return 0.0;
 return log(x);
}

int factorial(int n)
{
 int i, result = 1;
 for (i = 2; i <= n; i++)
   result *= i;
 return result;
}

int binom(int m, int n)
{
 if (n == 0 || m == n)
   return 1;
 else
   return factorial(m) / (factorial(n) * factorial(m - n));
}

double binomdouble(int m, int n)
{
 int i;
 double sum = 0.0;
 if (m < n)
   return 0;
 if (n == 0 || m == n)
   return 1;
 else
  {
   for (i = 2; i <= m; i++)
     sum += log(i);
   for (i = 2; i <= m - n; i++)
    sum -= log(i);
   for (i = 2; i <= n; i++)
    sum -= log(i);
   return exp(sum);
  }
}
