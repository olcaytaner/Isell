#include <math.h>
#include "distributions.h"
#include "instance.h"
#include "libmath.h"
#include "libmemory.h"

extern Datasetptr current_dataset;

double beta(double* x, int count)
{
 /*Last Changed 28.02.2007*/
 double sum = 0.0, result = 0.0;
 int i;
 for (i = 0; i < count; i++)
  {
   result += gammln(x[i]);
   sum += x[i];
  }
 result -= gammln(sum);
 return result;
}

double chi_square(int freedom, double x)
{
 double  a, y, s;
 double  e, c, z;
 int even;     /* true if df is an even number */
 if (x <= 0.0 || freedom < 1)
   return (1.0);
 a = 0.5 * x;
 even = (2*(freedom/2)) == freedom;
 if (freedom > 1)
   y = ex (-a);
 if (even)
   s = y;
 else
   s = (2.0 * z_normal (-sqrt (x)));
 if (freedom > 2)
  {
   x = 0.5 * (freedom - 1.0);
   if (even)
     z=1.0;
   else
     z=0.5;
   if (a > BIGX)
    {
     if (even)
       e=0.0;
     else
       e=LOG_SQRT_PI;
     c = log (a);
     while (z <= x)
      {
       e = log (z) + e;
       s += ex (c*z-a-e);
       z += 1.0;
      }
     return (s);
    }
   else
    {
     if (even)
       e = 1.0;
     else
       e=(I_SQRT_PI / sqrt (a));
     c = 0.0;
     while (z <= x)
      {
       e = e * (a / z);
       c = c + e;
       z += 1.0;
      }
     return (c * y + s);
    }
  }
 else
   return (s);
}

double chi_square_inverse(double p,int freedom)
{
 double  minchisq = 0.0;
 double  maxchisq = CHI_MAX;
 double  chisqval;
 if (p <= 0.0)
   return (maxchisq);
 else
   if (p >= 1.0)
     return (0.0);
 chisqval = freedom / sqrt (p);    /* fair first value */
 while (maxchisq - minchisq > CHI_EPSILON)
  {
   if (chi_square (freedom , chisqval) < p)
     maxchisq = chisqval;
   else
     minchisq = chisqval;
   chisqval = (maxchisq + minchisq) * 0.5;
  }
 return (chisqval);
}

double dirichlet(Instanceptr x, double* alpha)
{
 /*Last Changed 28.02.2007*/
 int i;
 double result = 1.0;
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
   result *= pow(real_feature(x, i), alpha[i] - 1);
 result /= beta(alpha, current_dataset->multifeaturecount - 1);
 return result;
}

double f_distribution(double F,int freedom1,int freedom2)
{/*Finds the probability*/
 int i, j;
 int a, b;
 double  w, y, z, d, p;
 if (F < F_EPSILON || freedom1 < 1 || freedom2 < 1)
   return (1.0);
 if (freedom1%2)
   a=1;
 else
   a=2;
 if (freedom2%2)
   b=1;
 else
   b=2;
 w = (F * freedom1) / freedom2;
 z = 1.0 / (1.0 + w);
 if (a == 1)
   if (b == 1)
    {
     p = sqrt (w);
     y = I_PI; /* 1 / 3.14159 */
     d = y * z / p;
     p = 2.0 * y * atan (p);
    }
   else
    {
     p = sqrt (w * z);
     d = 0.5 * p * z / w;
    }
 else
   if (b == 1)
    {
     p = sqrt (z);
     d = 0.5 * z * p;
     p = 1.0 - p;
    }
   else
    {
     d = z * z;
     p = w * z;
    }
 y = 2.0 * w / z;
 for (j = b + 2; j <= freedom2; j += 2)
  {
   d *= (1.0 + a / (j - 2.0)) * z;
   if (a==1)
     p = p + d * y / (j - 1.0);
   else
     p = (p + w) * z;
  }
 y = w * z;
 z = 2.0 / z;
 b = freedom2 - 2;
 for (i = a + 2; i <= freedom1; i += 2)
  {
   j = i + b;
   d *= y * j / (i - 2.0);
   p -= z * d / j;
  }
 if (p < 0.0)
   p = 0.0;
 else
   if (p > 1.0)
     p = 1.0;
 return (1.0-p);
}

double f_distribution_inverse(double p, int freedom1, int freedom2)
{/*Finds the value*/
 double  fval;
 double  maxf = F_MAX;     /* maximum F ratio */
 double  minf = 0.0;       /* minimum F ratio */
 if (p <= 0.0 || p >= 1.0)
   return (0.0);
 if (freedom1 == freedom2 && freedom1 > 2500)
   return 1 + 4.0 / freedom1;
 fval = 1.0 / p;             /* the smaller the p, the larger the F */
 while (fabs (maxf - minf) > F_EPSILON)
  {
   if (f_distribution (fval, freedom1, freedom2) < p)     /* F too large */
     maxf = fval;
   else                              /* F too small */
     minf = fval;
   fval = (maxf + minf) * 0.5;
  }
 return (fval);
}

double gammln(double x)
{
 /*Last Changed 28.02.2007*/
 /*Returns ln of gamma(x)*/
 double y,tmp,ser;
 double cof[6]={76.18009172947146, -86.50532032941677, 24.01409824083091, -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5};
 int j;
 y = x;
 tmp = x + 5.5;
 tmp -= (x + 0.5) * log(tmp);
 ser = 1.000000000190015;
 for (j = 0; j <= 5; j++) 
   ser += cof[j] / ++y;
 return -tmp + log(2.5066282746310005 * ser / x);
}

double gamma(double x)
{
 /*Last Changed 28.02.2007*/
 return safeexp(gammln(x));
}

double laplacian(double x, double mean, double stdev)
{
 /*Last Changed 20.01.2007*/
 return (sqrt(2) * safeexp(-1 * sqrt(2) * fabs(x - mean) / stdev) / (stdev * 2));
}

double normal(double x,int upper)
{
 double Alnorm,ltone=7.0,utzero=18.66,con=1.28,z,y;
 z=x;
 if (z<0)
  {
   upper=1-upper;
   z=-z;
  }
 if ((z <= ltone) || ((upper) && (z >= utzero)))
  {
   y=0.5*z*z;
   if (z>con)
     Alnorm=0.398942280385*exp(-y)/(z-3.8052*0.00000001+1.00000615302/(z+3.98064794*0.0001+1.98615381364/(z-0.151679116635+5.29330324926/(z+4.8385912808-15.1508972451/(z+0.742380924027+30.789933034/(z+3.99019417011))))));
   else
     Alnorm=0.5-z*(0.398942280444-0.399903438504*y/(y+5.75885480458-29.8213557808/(y+2.62433121679+48.6959930692/(y+5.92885724438))));
  }
 else
   Alnorm=0;
 if (!upper)
   return 1-Alnorm;
 else
   return Alnorm;
}

double studentized_range(double q, double v, double r)
{
 double pj,ehj,hj,x,pz,w0,pk,gk,pk1,pk2,gstep,gmid,c,v2,h,r1,g,vw[30],qw[30],pcutj=0.00003,pcutk=0.0001,step=0.45,vmax=12000.0,fifth=0.2,cv1=0.193064705,cv2=0.293525326,cvmax=0.39894228,cv[4]={0.318309886,-0.00268132716,-0.00347222222,0.0833333333},result;
 int j,k,jmin=3,jmax=15,kmin=7,kmax=15,jump,jj,op,op2;
 result=0;
 if (q<=0)
   return result;
 g=step*pow(r,-fifth);
 gmid=0.5*log(r);
 r1=r-1;
 c=log(r*g*cvmax);
 if (c<vmax)
  {
   h=step*pow(v,-0.5);
   v2=v*0.5;
   if (v==1)
     c=cv1;
   else
     if (v==2)
       c=cv2;
     else
       c=sqrt(v2)*cv[0]/(1+((cv[1]/v2+cv[2])/v2+cv[3])/v2);
   c=log(c*g*r*h);
  }
 gstep=g;
 qw[0]=-1;
 qw[jmax]=-1;
 pk1=1;
 pk2=1;
 for (k=1;k<=kmax;k++)
  {
   gstep=gstep-g;
   op2=0;
   while ((gstep>0)||(op2==0))
    {
     gstep=-gstep;
     gk=gmid+gstep;
     pk=0;
     if ((pk2>pcutk)||(k<=kmin))
      {
       w0=c-0.5*gk*gk;
       pz=normal(gk,1);
       x=normal(gk-q,1)-pz;
       if (x>0)
         pk=exp(w0+r1*log(x));
       if (v<=vmax)
        {
         jump=-jmax;
         op=0;
         while ((h<0)||(op==0))
          {
           jump=jump+jmax;
           for (j=1;j<=jmax;j++)
            {
             jj=j+jump;
             if (qw[jj-1]<=0)
              {
               hj=h*j;
               if (j<jmax)
                 qw[jj]=-1;
               ehj=exp(hj);
               qw[jj-1]=q*ehj;
               vw[jj-1]=v*(hj+0.5-0.5*ehj*ehj);
              }
             pj=0;
             x=normal(gk-qw[jj-1],1)-pz;
             if (x>0)
               pj=exp(w0+vw[jj-1]+r1*log(x));
             pk=pk+pj;
             if (pj<=pcutj)
               if ((jj>jmin)||(k>kmin))
                 break;
            }
           h=-h;
           op++;
          }
        }
      }
     result=result+pk;
     if ((k>kmin)&&(pk<=pcutk)&&(pk1<=pcutk))
       return result;
     pk2=pk1;
     pk1=pk;
     op2++;
    }
  }
 return result;
}

double qtrng0(double p, double v, double r)
{
 double q,t,vmax=12000,c1=0.8843,c2=0.2368,c3=1.214,c4=1.208,c5=1.4142;
 t=z_inverse(0.5+0.5*p);
 if (v<vmax)
   t=t+(t*t*t+t)/v/4;
 q=c1-c2*t;
 if (v<vmax)
   q=q-c3/v+c4*t/v;
 return t*(q*log(r-1)+c5);
}

double studentized_range_inverse(double p, double v, double r)
{
 double pcut=0.001,result,q1,p1 = 0.0,p2 = 0.0,q2,e1,e2;
 int j,jmax=8;
 q1=qtrng0(p,v,r);
 p1=studentized_range(q1,v,r);
 result=q1;
 if (fabs(p1-p)<pcut)
   return result;
 if (p1>p)
   p1=1.75*p-0.75*p1;
 if (p1<p)
   p2=p+(p-p1)*(1-p)/(1-p1)*0.75;
 if (p2<0.80)
   p2=0.80;
 else
   if (p2>0.995)
     p2=0.995;
 q2=qtrng0(p2,v,r);
 for (j=2;j<=jmax;j++)
  {
   p2=studentized_range(q2,v,r);
   e1=p1-p;
   e2=p2-p;
   result=(e2*q1-e1*q2)/(e2-e1);
   if (fabs(e1)>=fabs(e2))
    {
     q1=q2;
     p1=p2;
    }
   if (fabs(p1-p)<5*pcut)
     return result;
   q2=result;
  }
 return result;
}

double t_distribution(double T,int freedom)
{
 if (T>=0)
   return (f_distribution(T*T,1,freedom)/2);
 else
   return 1-(f_distribution(T*T,1,freedom)/2);
}

double t_distribution_inverse(double p,int freedom)
{
 if (p<0.5)
   return sqrt(f_distribution_inverse(p*2,1,freedom));
 else
   return -sqrt(f_distribution_inverse((1-p)*2,1,freedom));
}

double z_normal(double z)
{/*Returns the probability*/
 double  y, x, w;
 if (z == 0.0)
   x = 0.0;
 else
  {
   y = 0.5 * fabs (z);
   if (y >= (Z_MAX * 0.5))
     x = 1.0;
   else
     if (y < 1.0)
      {
       w = y*y;
       x = ((((((((0.000124818987 * w-0.001075204047) * w +0.005198775019) * w-0.019198292004) * w +0.059054035642) * w-0.151968751364) * w +0.319152932694) * w-0.531923007300) * w +0.797884560593) * y * 2.0;
      }
     else
      {
       y -= 2.0;
       x = (((((((((((((-0.000045255659 * y+0.000152529290) * y -0.000019538132) * y-0.000676904986) * y +0.001390604284) * y-0.000794620820) * y -0.002034254874) * y+0.006549791214) * y -0.010557625006) * y+0.011630447319) * y -0.009279453341) * y+0.005353579108) * y -0.002141268741) * y+0.000535310849) * y +0.999936657524;
      }
  }
 if (z > 0.0)
   return ((x + 1.0) * 0.5);
 else
   return ((1.0 - x) * 0.5);
}

double z_inverse(double p)
{/*Returns the critical value given the probability*/
 double  minz = -Z_MAX;    /* minimum of range of z */
 double  maxz = Z_MAX;     /* maximum of range of z */
 double  zval = 0.0;       /* computed/returned z value */
 double  pval;     /* prob (z) function, pval := poz (zval) */
 if (p <= 0.0 || p >= 1.0)
   return (0.0);
 while (maxz - minz > Z_EPSILON)
  {
   pval = z_normal (zval);
   if (pval > p)
     maxz = zval;
   else
     minz = zval;
   zval = (maxz + minz) * 0.5;
  }
 return (zval);
}

double kolmogorov_two_sample(int m, int n, double D)
{
 int i;
 double N, lambda, pvalue;
 N = m * n / (m + n + 0.0);
 lambda = (sqrt(N) + 0.12 + 0.11 / sqrt(N)) * D;
 pvalue = 0.0;
 for (i = 1; i <= 101; i++)
  {
   if ((i - 1) % 2 == 0)
     pvalue += 2 * exp(-2 * lambda * lambda * i * i);
   else
     pvalue -= 2 * exp(-2 * lambda * lambda * i * i);
  }
 return pvalue;
}

double kolmogorov_one_sample(int n, double d) 
{ 
 int k, m, i, j, g;
 double h, s;
 matrix H, Q;
 s = d * d * n; 
 if (s > 7.24 || (s > 3.76 && n > 99)) 
   return 1 - 2 * exp(-(2.000071 + .331 / sqrt(n) + 1.409 / n) * s);
 k = (int)(n * d) + 1; 
 m = 2 * k - 1; 
 h = k - n * d; 
 H = matrix_alloc(m, m); 
 for (i = 0; i < m; i++) 
   for (j = 0; j < m; j++)
     if (i - j + 1 < 0) 
       H.values[i][j] = 0; 
     else 
       H.values[i][j] = 1; 
 for (i = 0; i < m; i++) 
  {
   H.values[i][0] -= pow(h, i + 1); 
   H.values[m - 1][i] -= pow(h, (m - i));
  } 
 H.values[m - 1][0] += (2 * h - 1 > 0 ? pow(2 * h - 1, m):0); 
 for (i = 0; i < m; i++) 
   for (j = 0; j < m; j++)
     if (i - j + 1 > 0) 
      for (g = 1; g <= i - j + 1; g++) 
        H.values[i][j] /= g; 
 Q = matrix_power(H, n); 
 s = Q.values[k - 1][k - 1]; 
 for (i = 1;i <= n; i++) 
   s = (s * i) / n; 
 matrix_free(H); 
 matrix_free(Q); 
 return s;
}
