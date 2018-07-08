#ifndef __eps_H
#define __eps_H

#include<stdio.h>
#include "image.h"
#include "poly.h"
#include "statistics.h"
#include "typedef.h"

#define WIDTHFACTOR 2

void      difference_plot(char** files, int filecount);
void      export_image_as_eps(char* fname);
void      plot_decision_node(FILE* outfile, Decisionnodeptr rootnode, int maxdepth, int maxwidth);
void      plot_decision_tree(Decisionnodeptr rootnode);
void      plot_decision_leaf_model(FILE* outfile, Decisionnodeptr rootnode, Point p, int radius);
void      plot_decision_model_at_node(FILE* outfile, Decisionnodeptr rootnode, int maxdepth, int maxwidth);
void      plot_regression_leaf_model(FILE* outfile, Regressionnodeptr rootnode, Point p, int radius, int algorithm, double outputmean, double outputvariance);
void      plot_regression_model_at_node(FILE* outfile, Regressionnodeptr rootnode, int maxdepth, int maxwidth, double outputmean, double outputvariance);
void      plot_regression_model_tree(Regressionnodeptr rootnode, double outputmean, double outputvariance);
void      plot_regression_node(FILE* outfile, Regressionnodeptr rootnode, int maxdepth, int maxwidth, int algorithm, double outputmean, double outputvariance);
void      plot_regression_node_model(FILE* outfile, Regressionnodeptr rootnode, Point p, int radius, double outputmean, double outputvariance);
void      plot_regression_tree(Regressionnodeptr rootnode, int algorithm, double outputmean, double outputvariance);
void      rangetest_plot(double* ranks, int algorithmcount, char** algorithms, double critical_difference);
void      rank_plot(char** names, int filecount);
int       save_axes(FILE* myfile);
void      save_legend(FILE* myfile);
void      setcolor(FILE* myfile, COLOR_TYPE color);
void      setfont(FILE* myfile, Font fnt);
void      setlinewidth(FILE* myfile, double linewidth);
void      write_axis(FILE* myfile, int tickcount, int tickwidth, int p1, int p2, int p3, AXIS_TYPE axistype);
void      write_circle(FILE* myfile, int centerx, int centery, int radius);
void      write_dashed_line(FILE* myfile, int startx, int starty, int endx, int endy, int dashedtype);
void      write_dictionary(FILE* myfile);
void      write_image_footer(FILE* myfile);
void      write_image_header(FILE* myfile,int width,int height);
void      write_line(FILE* myfile,int startx,int starty,int endx,int endy);
void      write_plot_names(FILE* myfile);
void      write_rectangle(FILE* myfile,int corner1x,int corner1y,int corner2x,int corner2y, FILL_TYPE filltype, int color);
void      write_string(FILE* myfile,int posx,int posy,char* st, ALIGN_TYPE align);

#endif
