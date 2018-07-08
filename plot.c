#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "algorithm.h"
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "eps.h"
#include "instance.h"
#include "lang.h"
#include "libarray.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"
#include "parameter.h"
#include "perceptron.h"
#include "plot.h"
#include "poly.h"
#include "regressiontree.h"
#include "rule.h"
#include "statistics.h"

extern Algorithm classification_algorithms[CLASSIFICATION_ALGORITHM_COUNT];
extern Algorithm regression_algorithms[REGRESSION_ALGORITHM_COUNT];
extern Image img;
extern Datasetptr current_dataset;
extern Instance mean, variance;

extern FILE *output;

char plots[10]="xo+#%-><^?";
Point dummyp = {0.0, 10.0};

void solve_sigmoid(double* observations[2], int obscount, double* besta, double* bestb)
{
 /*!Last Changed 27.10.2004*/
 int i, j, k;
 double sum, a, b, adiff, bdiff, power, bestsum = +LONG_MAX;
 adiff = 10.0 / DIVISIONS;
 bdiff = 10.0 / DIVISIONS;
 for (i = 0; i < DIVISIONS; i++)
   for (j = 0; j < DIVISIONS; j++)
    {
     sum = 0;
     a = -5 + adiff * i;
     b = -5 + bdiff * j;
     for (k = 0; k < obscount; k++)
      {
       power = observations[1][k] - sigmoid(a * observations[0][k] + b);
       sum += power * power;
      }
     if (sum < bestsum)
      {
       bestsum = sum;
       *besta = a;
       *bestb = b;
      }
    }
}

void solve_inv_gauss(double* observations[2], int obscount, double meanstart, double meanend, double stdevstart, double stdevend, double* mean, double* stdev)
{
 /*!Last Changed 27.10.2004*/
 int i, j, k;
 double sum, m, s, mdiff, sdiff, power, bestsum = +LONG_MAX;
 mdiff = (meanend - meanstart) / DIVISIONS;
 sdiff = (stdevend - stdevstart) / DIVISIONS;
 for (i = 0; i < DIVISIONS; i++)
   for (j = 1; j < DIVISIONS; j++)
    {
     sum = 0;
     m = meanstart + mdiff * i;
     s = stdevstart + sdiff * j;
     for (k = 0; k < obscount; k++)
      {
       power = observations[1][k] - 1 + s * sqrt(2 * PI) * gaussian(observations[0][k], m, s);
       sum += power * power;
      }
     if (sum < bestsum)
      {
       bestsum = sum;
       *mean = m;
       *stdev = s;
      }
    }
}

void plotsigmoid(char* names)
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 26.12.2004 added aout1, aout2 array values*/
 /*!Last Changed 27.10.2004*/
 char varvalue[MAXLENGTH];
 char st[2], st2[3], *legendname;
 double *observations[2], a, b;
 int datacount, j, color;
 Point p1, p2;
 FILE *infile;
 Objectptr obj;
 if (!get_parameter_o("hold"))
   clear_image();
 legendname = file_only_name(names);
 color = img.imagelegend.legendcount;
 st[0] = plots[color];
 st[1] = '\0';
 add_legend((char)('A' + color), legendname);
 safe_free(legendname);
 datacount = file_length(names);
 infile = fopen(names, "r");
 if (!infile)
   return;
 observations[0] = (double *)safemalloc(datacount * sizeof(double), "plotsigmoid", 13);
 observations[1] = (double *)safemalloc(datacount * sizeof(double), "plotsigmoid", 14);
 for (j = 0; j < datacount; j++)
   fscanf(infile, "%lf %lf", &(observations[0][j]), &(observations[1][j]));
 fclose(infile);
 for (j = 0; j < datacount; j++)
  {
   p1.x = (double) observations[0][j];
   check_x_range(p1);
   p1.y = (double) observations[1][j];
   add_object(OBJECT_STRING, p1, dummyp, dummyp, st);
  }
 solve_sigmoid(observations, datacount, &a, &b);
 sprintf(varvalue,"%5.2f",a);
 set_variable("aout1[1]", varvalue);
 sprintf(varvalue,"%5.2f",b);
 set_variable("aout2[1]", varvalue);
 safe_free(observations[0]);
 safe_free(observations[1]);
 p1.x = img.xaxis.lim.min;
 p1.y = sigmoid(p1.x * a + b);
 sprintf(st2, "%c", 'A' + color); 
 for (j = 1; j < PARTITIONS; j++)
  {
   p2.x = img.xaxis.lim.min + j * ((img.xaxis.lim.max - img.xaxis.lim.min) / PARTITIONS);
   p2.y = sigmoid(p1.x * a + b);
   obj = add_object(OBJECT_LINE, p1, p2, dummyp, st2);
   obj->fnt.fontcolor = color;
   p1.x = p2.x;
   p1.y = p2.y;
  }
 export_image();
}

void plot_classification_model(void* model, void* parameters, int algorithm, Instance mean, Instance variance)
{
 ON_OFF changedecisionthreshold = get_parameter_o("changedecisionthreshold");
 int i, j, color;
 char st[2];
 double* posteriors = safemalloc(current_dataset->classno * sizeof(double), "plot_model", 2);
 double meanx, meany, stdevx, stdevy;
 Instanceptr currentinstance;
 Point p, p3, p4;
 Prediction predicted, predictedy, predictedx;
 predictedy.classno = -1;
 if (!get_parameter_o("hold"))
   clear_image();
 color = img.imagelegend.legendcount;
 add_legend((char)('A' + color), classification_algorithms[algorithm - SELECTMAX].name);
 p3.x = color;
 p4.x = GRAY;
 st[0] = '.';
 st[1] = '\0';
 if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
  {
   meanx = mean.values[0].floatvalue;
   meany = mean.values[1].floatvalue;
   stdevx = sqrt(variance.values[0].floatvalue);
   stdevy = sqrt(variance.values[1].floatvalue);
  }
 currentinstance = empty_instance(3);
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.x = img.xaxis.lim.min + i * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {     
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[0].floatvalue = (p.x - meanx) / stdevx;
     else
       currentinstance->values[0].floatvalue = p.x;
     p.y = img.yaxis.lim.min + j * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[1].floatvalue = (p.y - meany) / stdevy;
     else
       currentinstance->values[1].floatvalue = p.y;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     currentinstance->values[2].stringindex = predicted.classno;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     if (changedecisionthreshold == ON && current_dataset->classno == 2)
      {
       if (posteriors[0] > get_parameter_f("decisionthreshold"))
         predicted.classno = 0;
       else
         predicted.classno = 1;
      }
     if (predicted.classno != predictedy.classno && j != 1)
       add_object(OBJECT_POINT, p, dummyp, p3, st);
     if (current_dataset->classno == 2 && j != 1 && in_list(algorithm, 2, SVM, SIMPLEXSVM) && ((predicted.hingeloss == 0.0 && predictedy.hingeloss > 0.0) || (predicted.hingeloss > 0.0 && predictedy.hingeloss == 0.0)))
       add_object(OBJECT_POINT, p, dummyp, p4, st);
     predictedy = predicted;
    }
  }
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.x = img.xaxis.lim.max - i * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {     
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[0].floatvalue = (p.x - meanx) / stdevx;
     else
       currentinstance->values[0].floatvalue = p.x;
     p.y = img.yaxis.lim.max - j * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[1].floatvalue = (p.y - meany) / stdevy;
     else
       currentinstance->values[1].floatvalue = p.y;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     currentinstance->values[2].stringindex = predicted.classno;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     if (changedecisionthreshold == ON && current_dataset->classno == 2)
      {
       if (posteriors[0] > get_parameter_f("decisionthreshold"))
         predicted.classno = 0;
       else
         predicted.classno = 1;
      }
     if (predicted.classno != predictedy.classno && j != 1)
       add_object(OBJECT_POINT, p, dummyp, p3, st);
     if (current_dataset->classno == 2 && j != 1 && in_list(algorithm, 2, SVM, SIMPLEXSVM) && ((predicted.hingeloss == 0.0 && predictedy.hingeloss > 0.0) || (predicted.hingeloss > 0.0 && predictedy.hingeloss == 0.0)))
       add_object(OBJECT_POINT, p, dummyp, p4, st);
     predictedy = predicted;
    }
  }
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.y = img.yaxis.lim.min + i * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {     
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[1].floatvalue = (p.y - meany) / stdevy;
     else
       currentinstance->values[1].floatvalue = p.y;
     p.x = img.xaxis.lim.min + j * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[0].floatvalue = (p.x - meanx) / stdevx;
     else
       currentinstance->values[0].floatvalue = p.x;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     currentinstance->values[2].stringindex = predicted.classno;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     if (changedecisionthreshold == ON && current_dataset->classno == 2)
      {
       if (posteriors[0] > get_parameter_f("decisionthreshold"))
         predicted.classno = 0;
       else
         predicted.classno = 1;
      }
     if (predicted.classno != predictedx.classno && j != 1)
       add_object(OBJECT_POINT, p, dummyp, p3, st);
     if (current_dataset->classno == 2 && j != 1 && in_list(algorithm, 2, SVM, SIMPLEXSVM) && ((predicted.hingeloss == 0.0 && predictedx.hingeloss > 0.0) || (predicted.hingeloss > 0.0 && predictedx.hingeloss == 0.0)))
       add_object(OBJECT_POINT, p, dummyp, p4, st);
     predictedx = predicted;
    }
  }
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.y = img.yaxis.lim.max - i * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {     
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[1].floatvalue = (p.y - meany) / stdevy;
     else
       currentinstance->values[1].floatvalue = p.y;
     p.x = img.xaxis.lim.max - j * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     if (classification_algorithms[algorithm - SELECTMAX].mustnormalize)
       currentinstance->values[0].floatvalue = (p.x - meanx) / stdevx;
     else
       currentinstance->values[0].floatvalue = p.x;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     currentinstance->values[2].stringindex = predicted.classno;
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(currentinstance, model, parameters, posteriors);
     if (changedecisionthreshold == ON && current_dataset->classno == 2)
      {
       if (posteriors[0] > get_parameter_f("decisionthreshold"))
         predicted.classno = 0;
       else
         predicted.classno = 1;
      }
     if (predicted.classno != predictedx.classno && j != 1)
       add_object(OBJECT_POINT, p, dummyp, p3, st);
     if (current_dataset->classno == 2 && j != 1 && in_list(algorithm, 2, SVM, SIMPLEXSVM) && ((predicted.hingeloss == 0.0 && predictedx.hingeloss > 0.0) || (predicted.hingeloss > 0.0 && predictedx.hingeloss == 0.0)))
       add_object(OBJECT_POINT, p, dummyp, p4, st);
     predictedx = predicted;
    }
  }
 safe_free(posteriors);
 export_image();
}
 
void plot_regression_model(void* model, void* parameters, int algorithm, Instance mean, Instance variance, double outputmean, double outputvariance)
{
 int i, color;
 char st[2];
 double meanx, stdevx;
 Instanceptr currentinstance;
 Point p, p3, p4;
 Prediction predicted;
 if (!get_parameter_o("hold"))
   clear_image();
 color = img.imagelegend.legendcount;
 add_legend((char)('A' + color), regression_algorithms[algorithm - ONEFEATURE].name);
 p4.x = GRAY;
 p3.x = color;
 st[0] = '.';
 st[1] = '\0';
 if (regression_algorithms[algorithm - ONEFEATURE].mustnormalize)
  {
   meanx = mean.values[0].floatvalue;
   stdevx = sqrt(variance.values[0].floatvalue);
  }
 currentinstance = empty_instance(2);
 for (i = 0; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.x = img.xaxis.lim.min + i * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   if (regression_algorithms[algorithm - ONEFEATURE].mustnormalize)
     currentinstance->values[0].floatvalue = (p.x - meanx) / stdevx;
   else
     currentinstance->values[0].floatvalue = p.x;
   predicted = regression_algorithms[algorithm - ONEFEATURE].test_algorithm(currentinstance, model, parameters, NULL);
   if (get_parameter_o("normalizeoutput"))
     p.y = predicted.regvalue * sqrt(outputvariance) + outputmean;
   else
     p.y = predicted.regvalue;
   add_object(OBJECT_POINT, p, dummyp, p3, st);
   if (in_list(algorithm, 2, SVMREG, SIMPLEXSVMREG))
    {
     if (get_parameter_o("normalizeoutput"))
       p.y = (predicted.regvalue - get_parameter_f("svmepsilon")) * sqrt(outputvariance) + outputmean;
     else
       p.y = predicted.regvalue - get_parameter_f("svmepsilon");
     add_object(OBJECT_POINT, p, dummyp, p4, st);      
     if (get_parameter_o("normalizeoutput"))
       p.y = (predicted.regvalue + get_parameter_f("svmepsilon")) * sqrt(outputvariance) + outputmean;
     else
       p.y = predicted.regvalue + get_parameter_f("svmepsilon");
     add_object(OBJECT_POINT, p, dummyp, p4, st);      
    }
  }
 export_image();
}

void plotinvgauss(char* names)
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 26.12.2004 added aout1, aout2 array values*/
 /*!Last Changed 27.10.2004*/
 char varvalue[MAXLENGTH], pst[READLINELENGTH];
 char st[2], st2[3], *legendname;
 double *observations[2], mean, stdev;
 int datacount, j, color;
 Point p1, p2;
 FILE *infile;
 Objectptr obj;
 if (!get_parameter_o("hold"))
   clear_image();
 legendname = file_only_name(names);
 color = img.imagelegend.legendcount;
 st[0] = plots[color];
 st[1] = '\0';
 add_legend((char)('A' + color), legendname);
 safe_free(legendname);
 datacount = file_length(names);
 infile = fopen(names, "r");
 if (!infile)
   return;
 observations[0] = (double *)safemalloc(datacount * sizeof(double), "plotinvgauss", 13);
 observations[1] = (double *)safemalloc(datacount * sizeof(double), "plotinvgauss", 14);
 for (j = 0; j < datacount; j++)
   fscanf(infile, "%lf %lf", &(observations[0][j]), &(observations[1][j]));
 fclose(infile);
 for (j = 0; j < datacount; j++)
  {
   p1.x = (double) observations[0][j];
   check_x_range(p1);   
   p1.y = (double) observations[1][j];
   add_object(OBJECT_STRING, p1, dummyp, dummyp, st);
  }
 solve_inv_gauss(observations, datacount, img.xaxis.lim.min, img.xaxis.lim.max, 0, img.xaxis.lim.max - img.xaxis.lim.min, &mean, &stdev);
 set_precision(pst, NULL, NULL);
 sprintf(varvalue, pst, mean);
 set_variable("aout1[1]", varvalue);
 set_precision(pst, NULL, NULL);
 sprintf(varvalue, pst, stdev);
 set_variable("aout2[1]", varvalue);
 safe_free(observations[0]);
 safe_free(observations[1]);
 p1.x = img.xaxis.lim.min;
 p1.y = 1 - stdev * sqrt(2 * PI) * gaussian(p1.x, mean, stdev);
 sprintf(st2, "%c", 'A' + color); 
 for (j = 1; j < PARTITIONS; j++)
  {
   p2.x = img.xaxis.lim.min + j * ((img.xaxis.lim.max - img.xaxis.lim.min) / PARTITIONS);
   p2.y = 1 - stdev * sqrt(2 * PI) * gaussian(p1.x, mean, stdev);
   obj = add_object(OBJECT_LINE, p1, p2, dummyp, st2);
   obj->fnt.fontcolor = color;
   p1.x = p2.x;
   p1.y = p2.y;
  }
 export_image();
}

void plot_covariance(matrix avg, matrix covariance, int contour, int color, double maxdist)
{
 int i, j, oldbin, newbin;
 double avgdist, predicted, predictedx, predictedy;
 Point p, dummyp;
 char st[2];
 matrix x;
 st[0] = '.';
 st[1] = '\0';
 dummyp.x = color;
 if (maxdist == 0.0)
   maxdist = 0.5 * pow(matrix_determinant(covariance), 0.25);
 else
   maxdist /= 2.0;
 avgdist = maxdist / contour;
 x = matrix_alloc(1, 2);
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.x = img.xaxis.lim.min + i * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   x.values[0][0] = p.x;
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {
     p.y = img.yaxis.lim.min + j * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     x.values[0][1] = p.y;
     predicted = sqrt(mahalanobis_distance(x, avg, covariance));
     oldbin = ((int)(predictedy / avgdist));
     newbin = ((int)(predicted / avgdist));
     if ((oldbin != newbin) && (newbin < contour) && (j != 1))
       add_object(OBJECT_POINT, p, dummyp, dummyp, st);
     predictedy = predicted;
    }
  }
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.x = img.xaxis.lim.max - i * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   x.values[0][0] = p.x;
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {
     p.y = img.yaxis.lim.max - j * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     x.values[0][1] = p.y;
     predicted = sqrt(mahalanobis_distance(x, avg, covariance));
     oldbin = ((int)(predictedy / avgdist));
     newbin = ((int)(predicted / avgdist));
     if ((oldbin != newbin) && (newbin < contour) && (j != 1))
       add_object(OBJECT_POINT, p, dummyp, dummyp, st);
     predictedy = predicted;
    }
  }
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.y = img.yaxis.lim.min + i * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   x.values[0][1] = p.y;
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {
     p.x = img.xaxis.lim.min + j * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     x.values[0][0] = p.x;
     predicted = sqrt(mahalanobis_distance(x, avg, covariance));
     oldbin = ((int)(predictedx / avgdist));
     newbin = ((int)(predicted / avgdist));
     if ((oldbin != newbin) && (newbin < contour) && (j != 1))
       add_object(OBJECT_POINT, p, dummyp, dummyp, st);
     predictedx = predicted;
    }
  }
 for (i = 1; i < MESHDEPTH * PARTITIONS; i++)
  {
   p.y = img.yaxis.lim.max - i * ((img.yaxis.lim.max - img.yaxis.lim.min) / (MESHDEPTH * PARTITIONS));
   x.values[0][1] = p.y;
   for (j = 1; j < MESHDEPTH * PARTITIONS; j++)
    {
     p.x = img.xaxis.lim.max - j * ((img.xaxis.lim.max - img.xaxis.lim.min) / (MESHDEPTH * PARTITIONS));
     x.values[0][0] = p.x;
     predicted = sqrt(mahalanobis_distance(x, avg, covariance));
     oldbin = ((int)(predictedx / avgdist));
     newbin = ((int)(predicted / avgdist));
     if ((oldbin != newbin) && (newbin < contour) && (j != 1))
       add_object(OBJECT_POINT, p, dummyp, dummyp, st);
     predictedx = predicted;
    }
  }
 matrix_free(x);
}

void plotgauss2d(char* names, int contour)
{
 int i;
 double obs1, obs2, dist, maxdist;
 double *observations1 = NULL, *observations2 = NULL, *averages;
 matrix observations, covariance, avg, x;
 Point p, dummyp, p3;
 char st2[2], *legendname;
 FILE* myfile;
 int datacount, color;
 if (!get_parameter_o("hold"))
   clear_image();
 legendname = file_only_name(names);
 color = img.imagelegend.legendcount;
 p3.x = color;
 add_legend((char)('A' + color), legendname);
 safe_free(legendname);
 st2[0] = plots[color];
 st2[1] = '\0';
 datacount = 0;
 myfile = fopen(names, "r");
 if (!myfile)
   return;
 while (fscanf(myfile,"%lf %lf\n", &obs1, &obs2) != EOF)
  {
   datacount++;
   observations1 = alloc(observations1, datacount * sizeof(double), datacount);
   observations1[datacount - 1] = obs1;
   observations2 = alloc(observations2, datacount * sizeof(double), datacount);
   observations2[datacount - 1] = obs2;
   p.x = (double) obs1;
   check_x_range(p);
   p.y = (double) obs2;
   check_y_range(p);
   add_object(OBJECT_STRING, p, dummyp, p3, st2);
  }
 fclose(myfile);
 observations = matrix_alloc(datacount, 2);
 avg = matrix_alloc(1, 2);
 for (i = 0; i < datacount; i++)
  {
   observations.values[i][0] = observations1[i];
   observations.values[i][1] = observations2[i];
  }
 covariance = matrix_covariance(observations, &averages);
 matrix_free(observations);
 memcpy(avg.values[0], averages, 2 * sizeof(double));
 safe_free(averages);
 x = matrix_alloc(1, 2);
 maxdist = -INT_MAX;
 for (i = 0; i < datacount; i++)
  {
   x.values[0][0] = observations1[i];
   x.values[0][1] = observations2[i];
   dist = mahalanobis_distance(x, avg, covariance);
   if (dist == -1)
    {
     matrix_free(x);
     safe_free(observations1);
     safe_free(observations2);
     matrix_free(covariance);
     matrix_free(avg);
     return;
    }
   dist = sqrt(dist);
   if (dist > maxdist)
     maxdist = dist;
  }
 matrix_free(x);
 safe_free(observations1);
 safe_free(observations2);
 plot_covariance(avg, covariance, contour, color, maxdist);
 matrix_free(covariance);
 matrix_free(avg);
 export_image();
}

void plotgauss(char* names)
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 07.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 double obs, sum, divisionx, startx;
 double means;
 double variances;
 double* observations = NULL;
 Point p1, p2;
 char st2[3], *legendname;
 FILE* myfile;
 int datacount, j, color;
 Objectptr obj;
 if (!get_parameter_o("hold"))
   clear_image();
 legendname = file_only_name(names);
 color = img.imagelegend.legendcount;
 add_legend((char)('A' + color), legendname);
 safe_free(legendname);
 datacount = 0;
 sum = 0;
 myfile = fopen(names, "r");
 if (!myfile)
   return;
 while (fscanf(myfile,"%lf\n",&obs)!=EOF)
  {
   datacount++;
   observations = alloc(observations,datacount*sizeof(double),datacount);
   observations[datacount-1]=obs;
   sum += obs;
  }
 if (datacount != 0)
   means = sum / datacount;
 else
   means = 0;
 sum = 0;
 for (j = 0;j < datacount;j++)
   sum += (observations[j] - means) * (observations[j] - means);
 if (datacount > 1)
   variances = sqrt(sum / (datacount - 1));
 else
   variances = 0;
 safe_free(observations);
 fclose(myfile);
 img.yaxis.available = 0;
 img.height = (img.imagelegend.legendcount + 1) * 20 + MARGIN + 20;
 if (img.yaxis.lim.min > img.yaxis.lim.max)
  {
   img.yaxis.lim.min = 0;
   img.yaxis.lim.max = 1;
  }
 divisionx = variances * 6 / PARTITIONS;
 startx = means - 3 * variances;
 p1.x = startx;
 p1.y = gaussian(p1.x, means, variances);
 sprintf(st2, "%c", 'A' + color);
 for (j = 1; j < PARTITIONS; j++)
  {
   p2.x = startx + divisionx * j;
   p2.y = gaussian(p1.x, means, variances);
   obj = add_object(OBJECT_LINE, p1, p2, dummyp, st2);
   obj->fnt.fontcolor = color;
   p1.x = p2.x;
   p1.y = p2.y;
  }
 export_image();
}

void functionplot(char* functiondefinition, int color)
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 21.01.2005*/
 int i, functioncount;
 double minvaluey = +LONG_MAX, maxvaluey = -LONG_MAX, valuey, valuex, start, end;
 char st2[3], *legendname; 
 Point p1, p2;
 Functionptr fx = read_function_definition(functiondefinition, &functioncount);
 Objectptr obj;
 if (!get_parameter_o("hold"))
   clear_image();
 legendname = file_only_name(functiondefinition);
 add_legend((char)('A' + color), legendname);
 safe_free(legendname);
 img.yaxis.available = 1;
 find_function_bounds(fx, functioncount, &start, &end);
 for (i = 0; i <= PARTITIONS; i++)
  {
   valuex = start + i * ((end - start) / PARTITIONS);
   valuey = function_value(fx, functioncount, valuex);
   if (img.yaxis.lim.min >= img.yaxis.lim.max)
    {
     if (valuey < minvaluey)
       minvaluey = valuey;
     if (valuey > maxvaluey)
       maxvaluey = valuey;
    }
  }
 if (img.yaxis.lim.min >= img.yaxis.lim.max)
  {
   img.yaxis.lim.min = minvaluey;
   img.yaxis.lim.max = maxvaluey;
  }
 if (img.xaxis.lim.min >= img.xaxis.lim.max)
  {
   img.xaxis.lim.min = start;
   img.xaxis.lim.max = end;
  }
 p1.x = (double) start;
 p1.y = (double) (function_value(fx, functioncount, p1.x));
 sprintf(st2, "%c", 'A' + color);
 for (i = 1; i <= PARTITIONS; i++)
  {
   p2.x = (double) (start + i * ((end - start) / PARTITIONS));
   p2.y = (double) (function_value(fx, functioncount, p1.x));
   obj = add_object(OBJECT_LINE, p1, p2, dummyp, st2);
   obj->fnt.fontcolor = color;
   p1.x = p2.x;
   p1.y = p2.y;
  }
 for (i = 0;i < functioncount; i++)
   free_function(fx[i]);
 safe_free(fx);
 export_image();
}

void polyplot(double start, double end, char* polynomial, int color, int sig) 
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 08.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 int i;
 double minvaluey = +LONG_MAX, maxvaluey = -LONG_MAX, valuey, valuex;
 Point p1, p2;
 char st2[3]; 
 Polynomptr p = read_polynom(polynomial);
 Objectptr obj;
 if (!get_parameter_o("hold"))
   clear_image();
 add_legend((char)('A' + color), polynomial);
 img.yaxis.available = 1;
 for (i = 0; i <= PARTITIONS; i++)
  {
   valuex = start + i * ((end - start) / PARTITIONS);
   valuey = polynom_value(p, valuex);
   if (img.yaxis.lim.min >= img.yaxis.lim.max)
    {
     if (valuey < minvaluey)
       minvaluey = valuey;
     if (valuey > maxvaluey)
       maxvaluey = valuey;
    }
  }
 if (img.yaxis.lim.min >= img.yaxis.lim.max)
  {
   img.yaxis.lim.min = minvaluey;
   img.yaxis.lim.max = maxvaluey;
  }
 if (img.xaxis.lim.min >= img.xaxis.lim.max)
  {
   img.xaxis.lim.min = start;
   img.xaxis.lim.max = end;
  }
 p1.x = start;
 p1.y = (polynom_value(p, p1.x));
 if (sig)
   p1.y = sigmoid(p1.y);
 sprintf(st2, "%c", 'A' + color);
 for (i = 1; i <= PARTITIONS; i++)
  {
   p2.x = (start + i * ((end - start) / PARTITIONS));
   p2.y = (polynom_value(p, p1.x));
   if (sig)
     p2.y = sigmoid(p2.y);
   obj = add_object(OBJECT_LINE, p1, p2, dummyp, st2);
   obj->fnt.fontcolor = color;
   p1.x = p2.x;
   p1.y = p2.y;
  }
 free_polynom(p);
 export_image();
}

void histogram_plot(char* filename, int bincount)
{
 /*!Last Changed 11.10.2010*/
 Point p1, p2;
 char st[MAXLENGTH], pst[READLINELENGTH];
 double prev, max = -INT_MAX, min = +INT_MAX, obs, binwidth, *observations, *distinct_observations = NULL;
 FILE* myfile;
 int i, j, bin, *counts, filesize = file_length(filename), distinctcount;
 observations = safemalloc(filesize * sizeof(double), "histogram_plot", 20);
 if (!get_parameter_o("hold"))
   clear_image();
 myfile = fopen(filename, "r");
 if (!myfile)
  {
   printf(m1274, filename);
   return;
  }
 i = 0;
 while (fscanf(myfile, "%lf\n", &obs) != EOF)
  {
   if (obs > max)
     max = obs;
   if (obs < min)
     min = obs;
   observations[i] = obs;
   i++;
  }
 fclose(myfile);
 if (bincount == 0)
  {
   qsort(observations, filesize, sizeof(double), sort_function_double_ascending);
   distinctcount = 1;
   prev = observations[0];
   for (i = 1; i < filesize; i++)
     if (observations[i] != prev)
      {
       prev = observations[i];
       distinctcount++;
      }
   distinct_observations = safemalloc(distinctcount * sizeof(double), "histogram_plot", 35);
   counts = safecalloc(distinctcount, sizeof(int), "histogram_plot", 36);
   distinct_observations[0] = observations[0];
   prev = observations[0];
   counts[0] = 1;
   j = 0;
   for (i = 1; i < filesize; i++)
     if (observations[i] != prev)
      {
       j++;
       distinct_observations[j] = observations[i];
       prev = observations[i];
       counts[j] = 1;
      }
     else
       counts[j]++;
  }
 else
  {
   counts = safecalloc(bincount, sizeof(int), "histogram_plot", 3);
   binwidth = (max - min) / bincount;
   for (i = 0; i < filesize; i++)
    {
     bin = (int)((observations[i] - min) / binwidth);
     if (bin == bincount)
       bin--;
      counts[bin]++;
    }
  }
 img.xaxis.limitsset = 1;
 if (bincount != 0)
   img.xaxis.namesavailable = bincount;
 else 
   img.xaxis.namesavailable = distinctcount;
 img.xaxis.lim.division = img.xaxis.namesavailable;
 img.xaxis.lim.min = 0;
 img.xaxis.lim.max = img.xaxis.namesavailable;
 img.xaxis.lim.precision = 0;
 if (bincount != 0)
   img.xaxis.names = safemalloc(bincount * sizeof(char*), "histogram_plot", 24); 
 else 
   img.xaxis.names = safemalloc(distinctcount * sizeof(char*), "histogram_plot", 24); 
 img.yaxis.limitsset = 1;
 img.yaxis.lim.division = 10;
 img.yaxis.lim.min = 0;
 if (bincount != 0)
   img.yaxis.lim.max = counts[find_max_in_list(counts, bincount)];
 else
   img.yaxis.lim.max = counts[find_max_in_list(counts, distinctcount)];  
 img.yaxis.namesavailable = 0;
 img.yaxis.lim.precision = 0;
 if (bincount != 0)
  {
   for (i = 0; i < bincount; i++)
    {
     set_precision(pst, NULL, NULL);
     sprintf(st, pst, min + binwidth * i);  
     img.xaxis.names[i] = strcopy(img.xaxis.names[i], st);
     p1.x = i;
     p1.y = MARGIN;
     p2.x = 0;
     p2.y = counts[i];
     add_object(OBJECT_RECTANGLE, p1, p2, dummyp, "");
    }
  }
 else 
  {
   for (i = 0; i < distinctcount; i++)
    {
     set_precision(pst, NULL, NULL);
     sprintf(st, pst, distinct_observations[i]);  
     img.xaxis.names[i] = strcopy(img.xaxis.names[i], st);
     p1.x = i;
     p1.y = MARGIN;
     p2.x = 0;
     p2.y = counts[i];
     add_object(OBJECT_RECTANGLE, p1, p2, dummyp, "");
    }
   safe_free(distinct_observations);
  }
 safe_free(observations);
 safe_free(counts);
 export_image();
}

void error_histogram_plot(char** names, int filecount)
{
 /*!Last Changed 27.12.2007*/
 int i, j, index, counts[10], count;
 Point p1, p2, p3;
 char* plotname, st2[11];
 char* pos;
 double obs, min = +INT_MAX, max = -INT_MAX;
 FILE* myfile;
 if (!get_parameter_o("hold"))
   clear_image();
 for (i = 0; i < filecount; i++)
  {
   myfile = fopen(names[i], "r");
   if (!myfile)
    {
     printf(m1274, names[i]);
     return;
    }
   while (fscanf(myfile, "%lf\n", &obs) != EOF)
    {
     if (obs > max)
       max = obs;
     if (obs < min)
       min = obs;
    }
   fclose(myfile);
  } 
 for (i = 0; i < filecount; i++)
  {
   p1.x = min;
   p1.y = max;
   p2.x = img.yaxis.namesavailable;
   p2.y = i;
   myfile = fopen(names[i], "r");
   for (j = 0; j < 10; j++)
     counts[j] = 0;
   if (!myfile)
    {
     printf(m1274, names[i]);
     return;
    }
   count = 0;
   while (fscanf(myfile, "%lf\n", &obs) != EOF)
    {
     if (max != min)
      {
       index = (int) (10 * (obs - min) / (max - min));
       if (index == 10)
         index--;
      }
     else
       index = 5;
     counts[index]++;
     count++;
    }
   fclose(myfile);
   for (j = 0; j < 10; j++)
     st2[j] = '0' + (30 * counts[j]) / count;
   st2[10] = '\0';
   add_object(OBJECT_ERRORHISTOGRAM, p1, p2, p3, st2);
   if (img.yaxis.namesavailable == 0)
    {
     plotname = file_only_name(names[i]);     
     pos = strchr(plotname, '-');
     img.xaxis.names = alloc(img.xaxis.names, (i + 1) * sizeof(char *), (i + 1)); 
     if (pos)
       img.xaxis.names[i] = strcopy(img.xaxis.names[i], (pos + 1));
     else
       img.xaxis.names[i] = strcopy(img.xaxis.names[i], "Try Again");
     safe_free(plotname);
    }   
  }
 img.yaxis.names = alloc(img.yaxis.names, (img.yaxis.namesavailable + 1) * sizeof(char *), (img.yaxis.namesavailable + 1));
 plotname = file_only_name(names[0]);
 img.yaxis.names[img.yaxis.namesavailable] = strcopy(img.yaxis.names[img.yaxis.namesavailable], plotname);
 safe_free(plotname);
 img.xaxis.namesavailable = filecount;
 img.yaxis.namesavailable++;
 img.xaxis.limitsset = 1;
 img.xaxis.lim.division = img.xaxis.namesavailable;
 img.xaxis.lim.min = 0;
 img.xaxis.lim.max = img.xaxis.namesavailable;
 img.xaxis.lim.precision = 0;
 img.yaxis.limitsset = 1;
 img.yaxis.lim.division = img.yaxis.namesavailable;
 img.yaxis.lim.min = 0;
 img.yaxis.lim.max = 10 * img.yaxis.namesavailable;
 export_image();
}

void performance_plot(char** names, int filecount, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn))
{
 int count = get_parameter_i("runcount") * get_parameter_i("foldcount"), i, j, k, number_of_points;
 matrixptr posteriors;
 Performance_pointptr points;
 Objectptr obj;
 char st[2];
 Point p1, p2, dummyp;
 st[1] = '\0';
 clear_image();
 for (i = 0; i < filecount; i++)
  {
   st[0] = '0';
   posteriors = read_posteriors(count, names[i]);
   for (j = 0; j < count; j++)
    {
     points = find_performance_points(posteriors[j], &number_of_points, 0, 1, calculate_performance);
     for (k = 0; k < number_of_points - 1; k++)
      {
       p1.x = points[k].x;
       p1.y = points[k].y;
       p2.x = points[k + 1].x;
       p2.y = points[k + 1].y;
       obj = add_object(OBJECT_LINE, p1, p2, dummyp, st);
       obj->fnt.fontcolor = i;
       if (get_parameter_o("displayequalloss") && points[k].equal_loss)
        {
         st[0] = plots[i];
         obj = add_object(OBJECT_STRING, p1, dummyp, dummyp, st);
         obj->fnt.fontcolor = i;
         obj->fnt.fontsize = 20;
        }
      }
     safe_free(points);
     matrix_free(posteriors[j]);
    }
   safe_free(posteriors);
  }
 img.xaxis.limitsset = 1;
 img.xaxis.lim.division = 10;
 img.xaxis.lim.min = 0.0;
 img.xaxis.lim.max = 1.0;
 img.xaxis.lim.precision = 1;
 img.yaxis.limitsset = 1;
 img.yaxis.lim.division = 10;
 img.yaxis.lim.min = 0.0;
 img.yaxis.lim.max = 1.0;
 img.yaxis.lim.precision = 1;
 export_image();
}

void box_plot(char** names, int filecount)
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 11.07.2005*/
 int count, i;
 Point p1, p2, p3;
 char* plotname, st2[3];
 plotname = file_only_name(names[0]); 
 if (!get_parameter_o("hold"))
   clear_image();
 for (i = 0; i < filecount; i++)
  {
   p1.x = file_percentile(names[i], 50);
   p1.y = file_percentile(names[i], 25);
   p2.x = file_percentile(names[i], 75);
   p2.y = i;
   file_whiskers(names[i], p1.y, p2.x, &(p3.x), &(p3.y));
   sprintf(st2, "%d", img.xaxis.namesavailable);
   add_object(OBJECT_BOXPLOT, p1, p2, p3, st2);
  }
 img.xaxis.namesavailable++;
 count = img.xaxis.namesavailable;
 img.xaxis.names = alloc(img.xaxis.names, count * sizeof(char *), count); 
 img.xaxis.names[count - 1] = strcopy(img.xaxis.names[count - 1], plotname);
 safe_free(plotname);
 img.xaxis.limitsset = 1;
 img.xaxis.lim.division = img.xaxis.namesavailable;
 img.xaxis.lim.min = 0;
 img.xaxis.lim.max = img.xaxis.namesavailable;
 img.xaxis.lim.precision = 0;
 export_image();
}

void plotmv(char** names, int filecount)
{
 /*!Last Changed 20.09.2006*/
 /*!Last Changed 11.07.2005*/
 int count, i;
 Point p1, p2;
 char* plotname;
 if (!get_parameter_o("hold"))
   clear_image();
 plotname = file_only_name(names[0]);
 for (i = 0; i < filecount; i++)
  {
   p1.x = file_mean(&(names[i]), 1);
   p1.y = file_variance(&(names[i]), 1);
   p2.x = img.xaxis.namesavailable;
   p2.y = i;
   add_object(OBJECT_MVBOX, p1, p2, dummyp, NULL);
  }
 img.xaxis.namesavailable++;
 count = img.xaxis.namesavailable;
 img.xaxis.names = alloc(img.xaxis.names, count * sizeof(char *), count); 
 img.xaxis.names[count - 1] = strcopy(img.xaxis.names[count - 1], plotname);
 safe_free(plotname);
 img.xaxis.lim.division = count;
 img.xaxis.lim.min = 0;
 img.xaxis.lim.max = count;
 img.xaxis.lim.precision = 0;
 export_image();
}

void plotx(char* names)
{
 /*!Last Changed 06.09.2006 incremented legendcount*/
 /*!Last Changed 07.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 double* observations;
 FILE* myfile;
 double obs, minobs = +LONG_MAX, maxobs = -LONG_MAX;
 int counts, j, color;
 char st[2], st2[10];
 Point p1, p3;
 if (!get_parameter_o("hold"))
   clear_image();
 color = img.imagelegend.legendcount;
 st[0] = plots[color];
 st[1] = '\0';
 p3.x = color;
 sprintf(st2, "Class %d", img.imagelegend.legendcount + 1);
 add_legend(st[0], st2);
 myfile = fopen(names, "r");
 if (!myfile)
   return;
 counts = 0;
 observations = NULL;
 while (fscanf(myfile, "%lf\n", &obs) != EOF)
  {
   counts++;
   observations = alloc(observations, counts * sizeof(double), counts);
   observations[counts - 1] = obs;
   if (obs > maxobs)
     maxobs = obs;
   if (obs < minobs)
     minobs = obs;
  }
 fclose(myfile);
 if (maxobs > img.xaxis.lim.max)
   img.xaxis.lim.max = maxobs;
 if (minobs < img.xaxis.lim.min)
   img.xaxis.lim.min = minobs; 
 img.yaxis.available = 0;
 for (j = 0; j < counts; j++)
  {
   p1.x = (double) observations[j];
   p1.y = 0;
   add_object(OBJECT_STRING, p1, dummyp, p3, st);
  }
 safe_free(observations);
 export_image();
}

double** read_2d_observations(char* filename, int* counts)
{
 double** observations;
 FILE* myfile;
 double obs1, obs2, minobsx = +LONG_MAX, minobsy = +LONG_MAX, maxobsx = -LONG_MAX, maxobsy = -LONG_MAX;
 *counts = 0;
 myfile = fopen(filename, "r");
 if (!myfile)
   return NULL;
 observations = safecalloc(2, sizeof(double*), "read_2d_observations", 7);
 while (fscanf(myfile, "%lf %lf\n", &obs1, &obs2) != EOF)
  {
   (*counts)++;
   observations[0] = alloc(observations[0], (*counts) * sizeof(double), *counts);
   observations[1] = alloc(observations[1], (*counts) * sizeof(double), *counts);
   observations[0][(*counts) - 1] = obs1;
   observations[1][(*counts) - 1] = obs2;
   if (obs1 > maxobsx)
     maxobsx = obs1;
   if (obs1 < minobsx)
     minobsx = obs1;
   if (obs2 > maxobsy)
     maxobsy = obs2;
   if (obs2 < minobsy)
     minobsy = obs2;
  }
 fclose(myfile);
 if (maxobsx > img.xaxis.lim.max)
   img.xaxis.lim.max = maxobsx;
 if (minobsx < img.xaxis.lim.min)
   img.xaxis.lim.min = minobsx;
 if (maxobsy > img.yaxis.lim.max)
   img.yaxis.lim.max = maxobsy;
 if (minobsy < img.yaxis.lim.min)
   img.yaxis.lim.min = minobsy;
 return observations;
}

void plotxy(char* names)
{
 /*!Last Changed 06.09.2006 incremented legendcount*/
 /*!Last Changed 08.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 double** observations;
 int counts, j, color;
 char st[2], st2[10];
 Point p1, p3;
 if (!get_parameter_o("hold"))
   clear_image();
 img.yaxis.available = YES;
 color = img.imagelegend.legendcount;
 p3.x = color;
 st[0] = plots[color];
 st[1] = '\0';
 sprintf(st2, "Class %d", img.imagelegend.legendcount + 1);
 add_legend(st[0], st2);
 observations = read_2d_observations(names, &counts);
 for (j = 0; j < counts; j++)
  {
   p1.x = (double) observations[0][j];
   p1.y = (double) observations[1][j];
   add_object(OBJECT_STRING, p1, dummyp, p3, st);
  }
 safe_free(observations[0]);
 safe_free(observations[1]);
 safe_free(observations);
 export_image();
}

void plotxynames(char* names)
{
 /*!Last Changed 04.02.2008*/
 double* observations[2];
 char** obsnames;
 FILE* myfile;
 char st[READLINELENGTH];
 double obs1, obs2, minobsx = +LONG_MAX, minobsy = +LONG_MAX, maxobsx = -LONG_MAX, maxobsy = -LONG_MAX;
 int counts, j;
 Point p1;
 if (!get_parameter_o("hold"))
   clear_image();
 img.yaxis.available = YES;
 observations[0] = NULL;
 observations[1] = NULL;
 obsnames = NULL;
 myfile = fopen(names, "r");
 if (!myfile)
   return;
 counts = 0;
 while (fscanf(myfile, "%s %lf %lf\n", st, &obs1, &obs2) != EOF)
  {
   counts++;
   obsnames = alloc(obsnames, counts * sizeof(char *), counts);
   obsnames[counts - 1] = strcopy(obsnames[counts - 1], st);
   observations[0] = alloc(observations[0], counts * sizeof(double), counts);
   observations[1] = alloc(observations[1], counts * sizeof(double), counts);
   observations[0][counts - 1] = obs1;
   observations[1][counts - 1] = obs2;
   if (obs1 > maxobsx)
     maxobsx = obs1;
   if (obs1 < minobsx)
     minobsx = obs1;
   if (obs2 > maxobsy)
     maxobsy = obs2;
   if (obs2 < minobsy)
     minobsy = obs2;
  }
 fclose(myfile);
 if (maxobsx > img.xaxis.lim.max)
   img.xaxis.lim.max = maxobsx;
 if (minobsx < img.xaxis.lim.min)
   img.xaxis.lim.min = minobsx;
 if (maxobsy > img.yaxis.lim.max)
   img.yaxis.lim.max = maxobsy;
 if (minobsy < img.yaxis.lim.min)
   img.yaxis.lim.min = minobsy;
 for (j = 0; j < counts; j++)
  {
   p1.x = (double) observations[0][j];
   p1.y = (double) observations[1][j];
   add_object(OBJECT_NAMEDPOINT, p1, dummyp, dummyp, obsnames[j]);
  }
 safe_free(observations[0]);
 safe_free(observations[1]);
 free_2d((void **)obsnames, counts);
 export_image();
}

void plotay(char* names)
{
 /*!Last Changed 15.09.2006*/
 /*!Last Changed 08.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 FILE* myfile;
 double obs, maxobs = -LONG_MAX, minobs = 0;
 int count2;
 char st[MAXLENGTH], *legendname;
 Point p1, p2;
 if (!get_parameter_o("hold"))
   clear_image();
 legendname = file_only_name(names);
 add_legend((char) (img.xaxis.namesavailable + '0'), legendname);
 safe_free(legendname);
 img.yaxis.available = 1;
 myfile = fopen(names, "r");
 if (!myfile)
   return;
 count2 = 0;
 while (fscanf(myfile,"%s %lf\n", st, &obs) != EOF)
  {
   count2++;
   if (!img.xaxis.namesavailable)
    {
     img.xaxis.names = alloc(img.xaxis.names, count2 * (sizeof(char*)), count2);
     img.xaxis.names[count2 - 1] = strcopy(img.xaxis.names[count2 - 1], st);
     p2.x = count2 - 1;
    }
   else
     p2.x = listindex(st, img.xaxis.names, img.xaxis.lim.division);
   p1.x = img.xaxis.namesavailable;
   p1.y = MARGIN;
   p2.y = (double) obs;
   add_object(OBJECT_RECTANGLE, p1, p2, dummyp, "");   
   if (!img.xaxis.namesavailable)
    {
     if (obs > maxobs)
       maxobs = obs;
    }
   else
     if (obs > img.yaxis.lim.max)
       img.yaxis.lim.max = obs;
  }
 fclose(myfile);
 if (!img.xaxis.namesavailable)
  {
   img.xaxis.lim.division = count2;
   img.xaxis.limitsset = 1;
   img.xaxis.lim.precision = 0;
   img.yaxis.lim.min = minobs;
   img.yaxis.lim.max = maxobs;
  }
 img.xaxis.namesavailable++;
 export_image();
}

void plotxyline(char* names, int linecolor, double linewidth)
{
 /*!Last Changed 20.09.2006 added line types*/
 /*!Last Changed 06.09.2006 incremented legendcount*/
 /*!Last Changed 08.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 double** observations;
 int j, counts;
 char st2[3], *legendname;
 Point p1, p2;
 Objectptr obj;
 if (!get_parameter_o("hold"))
   clear_image();
 img.yaxis.available = 1;
 legendname = file_only_name(names); 
 add_legend((char) ('a' + img.imagelegend.legendcount), legendname);
 safe_free(legendname);
 observations = read_2d_observations(names, &counts);
 sprintf(st2, "%d", img.imagelegend.legendcount - 1);
 for (j = 0; j < counts - 1; j++)
  {
   p1.x = (double) (observations[0][j]);
   p1.y = (double) (observations[1][j]);
   p2.x = (double) (observations[0][j+1]);
   p2.y = (double) (observations[1][j+1]);
   obj = add_object(OBJECT_LINE, p1, p2, dummyp, st2);
   obj->fnt.fontcolor = linecolor;
   obj->fnt.fontsize = linewidth;
  }
 safe_free(observations[0]);
 safe_free(observations[1]);
 safe_free(observations);
 export_image();
}

void plotdata2d(Instanceptr data, Datasetptr d, int fno1, int fno2)
{
 /*!Last Changed 21.09.2006 added save_axes*/
 /*!Last Changed 08.10.2004 added img.xaxis.lim and img.yaxis.lim*/
 /*!Last Changed 29.09.2004 added is_class_plotted*/
 /*!Last Changed 26.09.2004*/
 char st[2];
 int classno;
 Instanceptr tmp;
 Point p1;
 if (!get_parameter_o("hold"))
   clear_image();
 img.yaxis.available = 1;
 if (img.xaxis.lim.min >= img.xaxis.lim.max)
   find_bounds_of_feature(current_dataset, data, fno1, &(img.xaxis.lim.min), &(img.xaxis.lim.max));
 if (img.yaxis.lim.min >= img.yaxis.lim.max)
   find_bounds_of_feature(current_dataset, data, fno2, &(img.yaxis.lim.min), &(img.yaxis.lim.max));
 tmp = data;
 while (tmp)
  {
   if (d->features[fno1].type == INTEGER)
     p1.x = (double) tmp->values[fno1].intvalue;
   else
     p1.x = tmp->values[fno1].floatvalue;
   if (d->features[fno2].type == INTEGER)
     p1.y = (double) tmp->values[fno2].intvalue;
   else
     p1.y = tmp->values[fno2].floatvalue;
   if (d->classno != 0)
    {
     classno = give_classno(tmp);
     if (((int *)get_parameter_a("is_class_plotted"))[classno])
      {
       st[0] = plots[classno];
       st[1] = '\0';
       add_object(OBJECT_STRING, p1, dummyp, dummyp, st);
      }     
    }
   else
    { 
     st[0] = '.';
     st[1] = '\0';
     add_object(OBJECT_STRING, p1, dummyp, dummyp, st);
    }
   tmp = tmp->next;
  }
 export_image();
}
