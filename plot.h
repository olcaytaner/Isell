#ifndef __plot_H
#define __plot_H

#include<stdio.h>
#include "image.h"
#include "poly.h"
#include "statistics.h"
#include "typedef.h"

#define DIVISIONS 100
#define MESHDEPTH 1

void      box_plot(char** names, int filecount);
void      error_histogram_plot(char** names, int filecount);
void      functionplot(char* functiondefinition, int color);
void      histogram_plot(char* filename, int bincount);
void      performance_plot(char** names, int filecount, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn));
void      plotay(char* names);
void      plot_covariance(matrix avg, matrix covariance, int contour, int color, double maxdist);
void      plotdata2d(Instanceptr data, Datasetptr d, int fno1, int fno2);
void      plotgauss(char* names);
void      plotgauss2d(char* names, int contour);
void      plotinvgauss(char* names);
void      plot_classification_model(void* model, void* parameters, int algorithm, Instance mean, Instance variance);
void      plot_model_tree(Decisionnodeptr rootnode);
void      plot_regression_model(void* model, void* parameters, int algorithm, Instance mean, Instance variance, double outputmean, double outputvariance);
void      plotmv(char** names, int filecount);
void      plotsigmoid(char* names);
void      plotx(char* names);
void      plotxy(char* names);
void      plotxyline(char* names, int linecolor, double linewidth);
void      plotxynames(char* names);
void      polyplot(double start, double end, char* polynomial, int color, int sig);
double**  read_2d_observations(char* filename, int* counts);
void      solve_inv_gauss(double* observations[2], int obscount, double meanstart, double meanend, double stdevstart, double stdevend, double* mean, double* stdev);
void      solve_sigmoid(double* observations[2], int obscount, double* besta, double* bestb);

#endif
