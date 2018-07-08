#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "decisiontree.h"
#include "eps.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "parameter.h"
#include "perceptron.h"
#include "regressiontree.h"
#include "softregtree.h"

char *availablefonts[4]={"Helvetica", "Courier", "Times", "Arial"};
/*black, red, green, blue, purple, yellow, cyan, gray, orange, brown, pink, white*/
double availablecolors[12][3]={{0.0, 0.0, 0.0},{1.0, 0.0, 0.0},{0.0, 0.5, 0.0},{0.0, 0.0, 1.0},{1.0, 0.0, 1.0},{1.0, 1.0, 0.0},{0.0, 1.0, 1.0},{0.5, 0.5, 0.5},{1.0, 0.7, 0.0},{0.7, 0.3, 0.0},{1.0, 0.75, 0.8},{1.0, 1.0, 1.0}};

extern Image img;
extern Datasetptr current_dataset;
extern Instance mean, variance;

/**
 * Saves current image to a given file
 * @param[in] fname Name of the output file
 */
void export_image_as_eps(char* fname)
{
 /*!Last Changed 04.02.2008 added namedpoint*/
 /*!Last Changed 27.12.2007 added error_histogram_plot*/
 /*!Last Changed 20.09.2006 added save_legend and save_axes*/
 /*!Last Changed 14.09.2006*/
 /*!Last Changed 11.07.2005 added mean variance plot and boxplot*/
 /*!Last Changed 23.01.2005*/
 char st[100];
 FILE* myfile;
 int i, j, posx, posx2, posy, posy2, posy3, posy4, posy5, xgroupcount, xgroupwidth, ygroupcount, ygroupwidth, diff, groupindex, index, elementcount, radius;
 double barwidth, barheight;
 int* groupdone;
 Line l;
 String s;
 Rectangle r;
 Circle ci;
 if (!img.xaxis.limitsset || !img.yaxis.limitsset)
   check_axis_ranges();
 myfile = fopen(fname, "w");
 if (!myfile)
   return;
 write_image_header(myfile, img.width, img.height);
 setcolor(myfile, BLACK);
 save_axes(myfile);
 setcolor(myfile, BLACK);
 save_legend(myfile);
 fprintf(myfile, "stroke\n");
 qsort(img.objects, img.objectcount, sizeof(Object), sort_function_image_object_ascending);
 xgroupcount = img.xaxis.namesavailable;
 ygroupcount = img.yaxis.namesavailable;
 groupdone = safecalloc(ygroupcount, sizeof(int), "export_image_as_eps", 18);
 for (i = 0; i < img.objectcount; i++)
  {   
   groupindex = img.objects[i].groupindex;
   index = img.objects[i].index;
   elementcount = object_count(img.objects[i].type, groupindex);
   switch (img.objects[i].type)
    {
     case OBJECT_LINE      :if (i == 0 || img.objects[i].fnt.fontcolor != img.objects[i - 1].fnt.fontcolor || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                             {
                              setcolor(myfile, img.objects[i].fnt.fontcolor);
                              setlinewidth(myfile, img.objects[i].fnt.fontsize);
                             }
                            l = img.objects[i].obje.li;
                            posx = return_pos(l.upleft.x, MARGIN, epsaxis + MARGIN, img.xaxis.lim.min, img.xaxis.lim.max);
                            posy = return_pos(l.upleft.y, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posx2 = return_pos(l.downright.x, MARGIN, epsaxis + MARGIN, img.xaxis.lim.min, img.xaxis.lim.max);
                            posy2 = return_pos(l.downright.y, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            if (img.objects[i].fnt.dashedtype == SOLID_LINE)
                              write_line(myfile, posx, posy, posx2, posy2);
                            else
                              write_dashed_line(myfile, posx, posy, posx2, posy2, img.objects[i].fnt.dashedtype);
                            break;
     case OBJECT_CIRCLE    :ci = img.objects[i].obje.ci;
                            posx = return_pos(ci.center.x, MARGIN, epsaxis + MARGIN, img.xaxis.lim.min, img.xaxis.lim.max);
                            posy = return_pos(ci.center.y, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            radius = ci.radius;
                            write_circle(myfile, posx, posy, radius);
                            break;
     case OBJECT_POINT     :
     case OBJECT_STRING    :if (i == 0 || img.objects[i].fnt.fontcolor != img.objects[i - 1].fnt.fontcolor || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                              setfont(myfile, img.objects[i].fnt);
                            s = img.objects[i].obje.st;
                            posx = return_pos(s.p.x, MARGIN, epsaxis + MARGIN, img.xaxis.lim.min, img.xaxis.lim.max);
                            if (img.yaxis.available)
                              posy = return_pos(s.p.y, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            else
                              posy = MARGIN;
                            write_string(myfile, posx, posy, s.st, ALIGN_CENTER);
                            break;
     case OBJECT_NAMEDPOINT:if (i == 0 || img.objects[i].fnt.fontcolor != img.objects[i - 1].fnt.fontcolor || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                              setfont(myfile, img.objects[i].fnt);
                            s = img.objects[i].obje.st;
                            posx = return_pos(s.p.x, MARGIN, epsaxis + MARGIN, img.xaxis.lim.min, img.xaxis.lim.max);
                            posy = return_pos(s.p.y, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            write_line(myfile, posx - 2, posy - 2, posx + 2, posy + 2);
                            write_line(myfile, posx - 2, posy + 2, posx + 2, posy - 2);
                            write_string(myfile, posx, posy - 2 * SMALLMARGIN, s.st, ALIGN_CENTER);
                            break;
     case OBJECT_RECTANGLE :if (i == 0 || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                              setlinewidth(myfile, img.objects[i].fnt.fontsize);
                            r = img.objects[i].obje.re;
                            posx = position_in_multi_plot(groupindex, index, xgroupcount, elementcount, &barwidth);
                            posy = return_pos(r.downright.y, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            write_rectangle(myfile, posx, (int) r.upleft.y, (int)(posx + barwidth), posy, index % MAX_FILL_TYPE, BLACK);
                            break;
     case OBJECT_MVBOX     :switch (img.groupcoloring)
                             {
                              case        ALL_SAME:setcolor(myfile, BLACK);
                                                   break;
                              case      GROUP_SAME:setcolor(myfile, img.objects[i].groupindex % COLORCOUNT);
                                                   break;
                              case INDIVIDUAL_SAME:setcolor(myfile, img.objects[i].index % COLORCOUNT);
                                                   break;
                             }
                            posy = return_pos(img.objects[i].obje.mv.mean, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posy2 = return_pos(img.objects[i].obje.mv.mean - img.objects[i].obje.mv.variance, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posy3 = return_pos(img.objects[i].obje.mv.mean + img.objects[i].obje.mv.variance, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posx = position_in_multi_plot(groupindex, index, xgroupcount, elementcount, &barwidth);
                            posx = (int) (posx + barwidth / 2);
                            write_line(myfile, posx, posy - 4, posx, posy2);
                            write_line(myfile, posx, posy + 4, posx, posy3);
                            write_line(myfile, posx - 5, posy2, posx + 5, posy2);
                            write_line(myfile, posx - 5, posy3, posx + 5, posy3);
                            write_circle(myfile, posx, posy, 4);
                            break;
     case OBJECT_BOXPLOT   :posy = return_pos(img.objects[i].obje.box.median, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posy2 = return_pos(img.objects[i].obje.box.p25, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posy3 = return_pos(img.objects[i].obje.box.p75, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posy4 = return_pos(img.objects[i].obje.box.swhisker, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posy5 = return_pos(img.objects[i].obje.box.lwhisker, MARGIN, epsaxis + MARGIN, img.yaxis.lim.min, img.yaxis.lim.max);
                            posx = position_in_multi_plot(groupindex, index, xgroupcount, elementcount, &barwidth);
                            diff = (int)(barwidth / 8);
                            posx = posx + diff;
                            barwidth -= 2 * diff;                           
                            write_rectangle(myfile, posx, posy3, (int) (posx + barwidth), posy2, FILL_COLOR, index + 1);
                            write_line(myfile, posx, posy, (int) (posx + barwidth), posy);
                            if (posy4 < posy2)
                             {
                              write_dashed_line(myfile, (int) (posx + barwidth / 2), posy2, (int) (posx + barwidth / 2), posy4, 4);
                              write_line(myfile, posx, posy4, (int) (posx + barwidth), posy4);
                             }
                            if (posy5 > posy3)
                             {
                              write_dashed_line(myfile, (int) (posx + barwidth / 2), posy3, (int) (posx + barwidth / 2), posy5, 4);
                              write_line(myfile, posx, posy5, (int) (posx + barwidth), posy5);
                             }
                            break;
     case OBJECT_ERRORHISTOGRAM:xgroupwidth = (epsaxis / elementcount);
                                ygroupwidth = (epsaxis / ygroupcount);
                                barheight = (ygroupwidth - 2 * SMALLMARGIN) / 10.0;
                                barwidth = (xgroupwidth - 2 * SMALLMARGIN) / 30.0;
                                posx = MARGIN + index * xgroupwidth + SMALLMARGIN;
                                posy = MARGIN + groupindex * ygroupwidth + SMALLMARGIN;
                                posy2 = MARGIN + (groupindex + 1) * ygroupwidth - SMALLMARGIN;
                                write_line(myfile, posx, posy, posx, posy2);
                                if (!groupdone[groupindex])
                                 {
                                  for (j = 0; j <= 10; j++)
                                   {                                                                  
                                    if (img.yaxis.lim.precision == 0)
                                      sprintf(st, "%.0f", img.objects[i].obje.errorhist.min + ((img.objects[i].obje.errorhist.max - img.objects[i].obje.errorhist.min) / 10) * j);
                                    else
                                      switch (img.yaxis.lim.precision)
                                       {
                                        case  1:sprintf(st, "%.1f", img.objects[i].obje.errorhist.min + ((img.objects[i].obje.errorhist.max - img.objects[i].obje.errorhist.min) / 10) * j);
                                                break;
                                        case  2:sprintf(st, "%.2f", img.objects[i].obje.errorhist.min + ((img.objects[i].obje.errorhist.max - img.objects[i].obje.errorhist.min) / 10) * j);
                                                break;
                                        case  3:sprintf(st, "%.3f", img.objects[i].obje.errorhist.min + ((img.objects[i].obje.errorhist.max - img.objects[i].obje.errorhist.min) / 10) * j);
                                                break;
                                        default:sprintf(st, "%.1f", img.objects[i].obje.errorhist.min + ((img.objects[i].obje.errorhist.max - img.objects[i].obje.errorhist.min) / 10) * j);
                                                break;
                                       }     
                                    write_string(myfile, MARGIN - 5, posy + barheight * j, st, ALIGN_RIGHT);
                                   }
                                  groupdone[groupindex] = YES;
                                 }
                                for (j = 0; j < 10; j++)
                                  if (img.objects[i].obje.errorhist.counts[j] > 0)
                                    write_rectangle(myfile, posx, posy + barheight * j + 1, posx + barwidth * img.objects[i].obje.errorhist.counts[j], posy + barheight * (j + 1) - 1, FILL_COLOR, index % COLORCOUNT);
                                break;
     default               :printf(m1233, img.objects[i].type);
                            break;
    }
  }
 safe_free(groupdone);
 write_image_footer(myfile);
 fclose(myfile);
}

void plot_decision_node(FILE* outfile, Decisionnodeptr rootnode, int maxdepth, int maxwidth)
{
 int i, dx, dy, radius = img.width / (5 * maxwidth), node_width = img.width / maxwidth, node_height = img.height / maxdepth;
 double l;
 Point p1, p2, parent;
 if (rootnode)
  {
   p1.x = (rootnode->x + 0.5) * node_width;
   p1.y = img.height - (rootnode->y + 0.5) * node_height;
   write_circle(outfile, p1.x, p1.y, radius);
   if (rootnode->parent)
    {
     parent.x = (rootnode->parent->x + 0.5) * node_width;
     parent.y = img.height - (rootnode->parent->y + 0.5) * node_height;
     l = point_to_point_distance(p1, parent);
     dx = fabs(parent.x - p1.x) * radius / l;
     dy = fabs(parent.y - p1.y) * radius / l;
     if (rootnode == rootnode->parent->left)
      {  
       p1.x = p1.x + dx;
       p2.x = parent.x - dx;
      }
     else
      {
       p1.x = p1.x - dx;
       p2.x = parent.x + dx;      
      }
     p1.y = p1.y + dy;
     p2.y = parent.y - dy;
     write_line(outfile, p1.x, p1.y, p2.x, p2.y);
    }
   if (rootnode->featureno >= 0 && current_dataset->features[rootnode->featureno].type == STRING)
     for (i = 0; i < current_dataset->features[rootnode->featureno].valuecount; i++)
       plot_decision_node(outfile, &(rootnode->string[i]), maxdepth, maxwidth);
   else
    {
     plot_decision_node(outfile, rootnode->left, maxdepth, maxwidth);
     plot_decision_node(outfile, rootnode->right, maxdepth, maxwidth);    
    }
  } 
}

void plot_decision_tree(Decisionnodeptr rootnode)
{
 FILE* outfile;
 int level = 0, maxdepth = depth_of_tree(rootnode), maxwidth = decisiontree_leaf_count(rootnode) + decisiontree_node_count(rootnode);
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return; 
 write_image_header(outfile, 2 * MARGIN + epsaxis, 2 * MARGIN + epsaxis);  
 level_of_decision_tree(rootnode);
 inorder_traversal_decision_tree(rootnode, &level);
 plot_decision_node(outfile, rootnode, maxdepth, maxwidth);
 write_image_footer(outfile);
 fclose(outfile);
}

void plot_decision_leaf_model(FILE* outfile, Decisionnodeptr rootnode, Point p, int radius)
{
 Prediction predicted, predictedold;
 double output, input1, input2, prevoutput;
 double min1 = current_dataset->features[0].min.floatvalue, max1 = current_dataset->features[0].max.floatvalue;
 double min2 = current_dataset->features[1].min.floatvalue, max2 = current_dataset->features[1].max.floatvalue; 
 int i, j, posx1, posx2, posy1, posy2, width = (2 * WIDTHFACTOR - 0.5) * radius, height = width, left = p.x - width / 2, right = p.x + width / 2, top = p.y - 2 * radius, bottom = p.y - 2 * radius - height;
 write_rectangle(outfile, left, top, right, bottom, FILL_NONE, BLACK);
 setcolor(outfile, RED);
 for (i = 1; i < width; i++)
  {
   input1 = ((min1 + i * ((max1 - min1) / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
   for (j = 1; j < width; j++)
    {
     input2 = ((min2 + j * ((max2 - min2) / width)) - mean.values[1].floatvalue) / sqrt(variance.values[1].floatvalue);      
     output = sigmoid(soft_output_of_subdecisiontree(rootnode, input1, input2));
     if (output > 0.5)
       predicted.classno = 1;
     else
       predicted.classno = 0;
     if (predictedold.classno != predicted.classno && j != 1)
       write_string(outfile, left + i, bottom + j, ".", ALIGN_LEFT);
     predictedold = predicted;
    }
  }
 for (i = width - 1; i > 0; i--)
  {
   input1 = ((min1 + i * ((max1 - min1) / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
   for (j = width - 1; j > 0; j--)
    {
     input2 = ((min2 + j * ((max2 - min2) / width)) - mean.values[1].floatvalue) / sqrt(variance.values[1].floatvalue);      
     output = sigmoid(soft_output_of_subdecisiontree(rootnode, input1, input2));
     if (output > 0.5)
       predicted.classno = 1;
     else
       predicted.classno = 0;
     if (predictedold.classno != predicted.classno && j != width - 1)
       write_string(outfile, left + i, bottom + j, ".", ALIGN_LEFT);
     predictedold = predicted;
    }
  }
 for (i = 1; i < width; i++)
  {
   input2 = ((min2 + i * ((max2 - min2) / width)) - mean.values[1].floatvalue) / sqrt(variance.values[1].floatvalue);
   for (j = 1; j < width; j++)
    {
     input1 = ((min1 + j * ((max1 - min1) / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);      
     output = sigmoid(soft_output_of_subdecisiontree(rootnode, input1, input2));
     if (output > 0.5)
       predicted.classno = 1;
     else
       predicted.classno = 0;
     if (predictedold.classno != predicted.classno && j != 1)
       write_string(outfile, left + j, bottom + i, ".", ALIGN_LEFT);
     predictedold = predicted;
    }
  }
 for (i = width - 1; i > 0; i--)
  {
   input2 = ((min2 + i * ((max2 - min2) / width)) - mean.values[1].floatvalue) / sqrt(variance.values[1].floatvalue);
   for (j = width - 1; j > 0; j--)
    {
     input1 = ((min1 + j * ((max1 - min1) / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);      
     output = sigmoid(soft_output_of_subdecisiontree(rootnode, input1, input2));
     if (output > 0.5)
       predicted.classno = 1;
     else
       predicted.classno = 0;
     if (predictedold.classno != predicted.classno && j != width - 1)
       write_string(outfile, left + j, bottom + i, ".", ALIGN_LEFT);
     predictedold = predicted;
    }
  }
 setcolor(outfile, BLACK); 
 if (rootnode->featureno != LEAF_NODE)
  {  
   for (i = 0; i < width; i++)
    {
     input1 = ((min1 + i * ((max1 - min1) / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
     input2 = -(rootnode->w.values[0][0] + rootnode->w.values[1][0] * input1) / rootnode->w.values[2][0];
     output = input2 * sqrt(variance.values[1].floatvalue) + mean.values[1].floatvalue;
     if (i != 0 && output >= min2 && output <= max2 && prevoutput >= min2 && prevoutput <= max2)
      {
       posy1 = return_pos(prevoutput, bottom, top, min2, max2);
       posy2 = return_pos(output, bottom, top, min2, max2);
       write_line(outfile, left + i - 1, posy1, left + i, posy2);            
      }
     prevoutput = output;
    }
   for (i = 0; i < width; i++)
    {
     input2 = ((min2 + i * ((max2 - min2) / width)) - mean.values[1].floatvalue) / sqrt(variance.values[1].floatvalue);
     input1 = -(rootnode->w.values[0][0] + rootnode->w.values[2][0] * input2) / rootnode->w.values[1][0];
     output = input1 * sqrt(variance.values[0].floatvalue) + mean.values[0].floatvalue;
     if (i != 0 && output >= min1 && output <= max1 && prevoutput >= min1 && prevoutput <= max1)
      {
       posx1 = return_pos(prevoutput, left, right, min1, max1);
       posx2 = return_pos(output, left, right, min1, max1);
       write_line(outfile, posx1, bottom + i - 1, posx2, bottom + i);            
      }
     prevoutput = output;
    }
  }
 else
   if (rootnode->leaftype == CONSTANTLEAF)
    {
     for (i = 0; i < width; i++)
      {
       input1 = ((min1 + i * ((max1 - min1) / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
       for (j = 0; j < width; j++)
        {
         input2 = ((min2 + j * ((max2 - min2) / width)) - mean.values[1].floatvalue) / sqrt(variance.values[1].floatvalue);      
         output = sigmoid(soft_output_of_subdecisiontree(rootnode, input1, input2));
         fprintf(outfile, "%.3f %.3f %.3f setrgbcolor\n", 1 - output, 1 - output, 1 - output);
         write_string(outfile, left + i, bottom + j, ".", ALIGN_CENTER);
        }
      }     
    }
 setcolor(outfile, BLACK);     
}

void plot_regression_node_model(FILE* outfile, Regressionnodeptr rootnode, Point p, int radius, double outputmean, double outputvariance)
{
 double output, prevoutput, input, posy1, posy2; 
 double minoutput = -1.5, maxoutput = 1.5;
 int i, width = (2 * WIDTHFACTOR - 0.5) * radius, height = width, left = p.x - width / 2, right = p.x + width / 2, top = p.y - 2 * radius, bottom = p.y - 2 * radius - height, bottom2 = p.y - 2 * radius - height / 2;
 write_rectangle(outfile, left, top, right, bottom, FILL_NONE, BLACK);
 for (i = 1; i < 4; i++)
  {
   write_line(outfile, left + i * ((right - left) / 4.0), bottom, left + i * ((right - left) / 4.0), bottom + 2);   
   write_line(outfile, left + i * ((right - left) / 4.0), top, left + i * ((right - left) / 4.0), top - 2);   
  }
 for (i = 1; i < 6; i++)
  {
   write_line(outfile, left, bottom + i * ((top - bottom) / 6.0), left + 2, bottom + i * ((top - bottom) / 6.0));   
   write_line(outfile, right - 2, bottom + i * ((top - bottom) / 6.0), right, bottom + i * ((top - bottom) / 6.0));   
  }
 setcolor(outfile, RED);
 for (i = 0; i <= width; i++)
  {
   prevoutput = output;
   input = ((-2 + i * (4.0 / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
   output = soft_output_of_subregressiontree(rootnode, input) * sqrt(outputvariance) + outputmean;
   if (i != 0 && output <= maxoutput && output >= minoutput)
    {
     posy1 = return_pos(prevoutput, bottom, top, minoutput, maxoutput);
     posy2 = return_pos(output, bottom, top, minoutput, maxoutput);
     write_line(outfile, left + i - 1, posy1, left + i, posy2);      
    }
  }
 setcolor(outfile, BLACK);
 write_rectangle(outfile, left + width, top, right + width / 2, bottom2, FILL_NONE, BLACK);
 for (i = 0; i <= width / 2; i++)
  {
   prevoutput = output;
   input = ((-2 + i * (8.0 / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
   output = sigmoid(rootnode->w.values[0][0] + rootnode->w.values[1][0] * input);
   if (i != 0)
    {
     posy1 = return_pos(prevoutput, bottom2, top, 0.0, 1.0);
     posy2 = return_pos(output, bottom2, top, 0.0, 1.0);
     write_line(outfile, left + width + i - 1, posy1, left + width + i, posy2);      
    }
  }
}

void plot_regression_leaf_model(FILE* outfile, Regressionnodeptr rootnode, Point p, int radius, int algorithm, double outputmean, double outputvariance)
{
 double output, prevoutput, output2, prevoutput2, input, posy1, posy2; 
 double minoutput = -1.5, maxoutput = 1.5;
 int i, width = (2 * WIDTHFACTOR - 0.5) * radius, height = width, left = p.x - width / 2, right = p.x + width / 2, top = p.y - 2 * radius, bottom = p.y - 2 * radius - height;
 write_rectangle(outfile, left, top, right, bottom, FILL_NONE, BLACK);
 for (i = 1; i < 4; i++)
  {
   write_line(outfile, left + i * ((right - left) / 4.0), bottom, left + i * ((right - left) / 4.0), bottom + 2);   
   write_line(outfile, left + i * ((right - left) / 4.0), top, left + i * ((right - left) / 4.0), top - 2);   
  } 
 for (i = 1; i < 6; i++)
  {
   write_line(outfile, left, bottom + i * ((top - bottom) / 6.0), left + 2, bottom + i * ((top - bottom) / 6.0));   
   write_line(outfile, right - 2, bottom + i * ((top - bottom) / 6.0), right, bottom + i * ((top - bottom) / 6.0));   
  }
 if (rootnode->leaftype == CONSTANTLEAF)
  {
   output = rootnode->regressionvalue * sqrt(outputvariance) + outputmean;
   posy1 = return_pos(output, bottom, top, minoutput, maxoutput);
   write_line(outfile, left, posy1, right, posy1);
   if (algorithm == SOFTREGTREE)
    {
     setcolor(outfile, RED);
     for (i = 0; i <= width; i++)
      {
       prevoutput2 = output2;
       input = ((-2 + i * (4.0 / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
       output2 = soft_output_of_subregressiontree(rootnode, input) * sqrt(outputvariance) + outputmean;
       if (i != 0 && output2 <= maxoutput && output2 >= minoutput)
        {
         posy1 = return_pos(prevoutput2, bottom, top, minoutput, maxoutput);
         posy2 = return_pos(output2, bottom, top, minoutput, maxoutput);
         write_line(outfile, left + i - 1, posy1, left + i, posy2);      
        }
      }
     setcolor(outfile, BLACK);     
    }
  }
 else
  {
   for (i = 0; i <= width; i++)
    {
     prevoutput = output;
     prevoutput2 = output2;
     input = ((-2 + i * (4.0 / width)) - mean.values[0].floatvalue) / sqrt(variance.values[0].floatvalue);
     output = (rootnode->w.values[0][0] + rootnode->w.values[1][0] * input) * sqrt(outputvariance) + outputmean;
     if (i != 0 && output <= maxoutput && output >= minoutput)
      {
       posy1 = return_pos(prevoutput, bottom, top, minoutput, maxoutput);
       posy2 = return_pos(output, bottom, top, minoutput, maxoutput);
       write_line(outfile, left + i - 1, posy1, left + i, posy2);      
      }
     if (algorithm == SOFTREGTREE)
      {
       setcolor(outfile, RED);
       output2 = soft_output_of_subregressiontree(rootnode, input) * sqrt(outputvariance) + outputmean;
       if (i != 0 && output2 <= maxoutput && output2 >= minoutput)
        {
         posy1 = return_pos(prevoutput2, bottom, top, minoutput, maxoutput);
         posy2 = return_pos(output2, bottom, top, minoutput, maxoutput);
         write_line(outfile, left + i - 1, posy1, left + i, posy2);      
        }
       setcolor(outfile, BLACK);       
      }
    }
  }
}

void plot_regression_node(FILE* outfile, Regressionnodeptr rootnode, int maxdepth, int maxwidth, int algorithm, double outputmean, double outputvariance)
{
 int dx, dy, radius = (epsaxis + MARGIN) / (WIDTHFACTOR * maxwidth), node_width = (epsaxis + MARGIN) / maxwidth, node_height = img.height / maxdepth;
 double l;
 Point p1, p2, parent;
 if (rootnode)
  {
   p1.x = MARGIN / 2 + (rootnode->x + 0.5) * node_width;
   p1.y = img.height - (rootnode->y + 0.5) * node_height + MARGIN / 2;
   write_circle(outfile, p1.x, p1.y, radius);
   if (rootnode->parent)
    {
     parent.x = MARGIN / 2 + (rootnode->parent->x + 0.5) * node_width;
     parent.y = img.height - (rootnode->parent->y + 0.5) * node_height + MARGIN / 2;
     l = point_to_point_distance(p1, parent);
     dx = fabs(parent.x - p1.x) * radius / l;
     dy = fabs(parent.y - p1.y) * radius / l;
     if (rootnode == rootnode->parent->left)
      {  
       p1.x = p1.x + dx;
       p2.x = parent.x - dx;
      }
     else
      {
       p1.x = p1.x - dx;
       p2.x = parent.x + dx;      
      }
     p1.y = p1.y + dy;
     p2.y = parent.y - dy;
     write_line(outfile, p1.x, p1.y, p2.x, p2.y);
    }
   plot_regression_node(outfile, rootnode->left, maxdepth, maxwidth, algorithm, outputmean, outputvariance);
   plot_regression_node(outfile, rootnode->right, maxdepth, maxwidth, algorithm, outputmean, outputvariance);
  }  
}

void plot_regression_tree(Regressionnodeptr rootnode, int algorithm, double outputmean, double outputvariance)
{
 FILE* outfile;
 int level = 0, maxdepth = depth_of_regression_tree(rootnode), maxwidth = regressiontree_leaf_count(rootnode) + regressiontree_node_count(rootnode);
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return; 
 write_image_header(outfile, 2 * MARGIN + epsaxis, 2 * MARGIN + epsaxis);    
 level_of_regression_tree(rootnode);
 inorder_traversal_regression_tree(rootnode, &level);
 plot_regression_node(outfile, rootnode, maxdepth, maxwidth, algorithm, outputmean, outputvariance);
 fclose(outfile);
 write_image_footer(outfile);
}

void plot_decision_model_at_node(FILE* outfile, Decisionnodeptr rootnode, int maxdepth, int maxwidth)
{
 int radius = (epsaxis + MARGIN) / (WIDTHFACTOR * maxwidth), node_width = (epsaxis + MARGIN) / maxwidth, node_height = img.height / maxdepth;
 Point p1, parent;
 if (rootnode)
  {
   p1.x = MARGIN / 2 + (rootnode->x + 0.5) * node_width;
   p1.y = img.height - (rootnode->y + 0.5) * node_height + MARGIN / 2 + 2 * radius;
   plot_decision_leaf_model(outfile, rootnode, p1, radius);
   if (rootnode->parent)
    {
     parent.x = MARGIN / 2 + (rootnode->parent->x + 0.5) * node_width;
     parent.y = img.height - (rootnode->parent->y + 0.5) * node_height + MARGIN / 2 + 2 * radius;
     write_line(outfile, p1.x, p1.y - 2 * radius, parent.x, parent.y - (2 * WIDTHFACTOR + 1.5) * radius);
    }   
   plot_decision_model_at_node(outfile, rootnode->left, maxdepth, maxwidth);
   plot_decision_model_at_node(outfile, rootnode->right, maxdepth, maxwidth);
  }  
}

void plot_regression_model_at_node(FILE* outfile, Regressionnodeptr rootnode, int maxdepth, int maxwidth, double outputmean, double outputvariance)
{
 int radius = (epsaxis + MARGIN) / (WIDTHFACTOR * maxwidth), node_width = (epsaxis + MARGIN) / maxwidth, node_height = img.height / maxdepth;
 Point p1, parent;
 if (rootnode)
  {
   p1.x = MARGIN / 2 + (rootnode->x + 0.5) * node_width;
   p1.y = img.height - (rootnode->y + 0.5) * node_height + MARGIN / 2 + 2 * radius;
   if (rootnode->featureno == LEAF_NODE)
     plot_regression_leaf_model(outfile, rootnode, p1, radius, SOFTREGTREE, outputmean, outputvariance);
   else
     plot_regression_node_model(outfile, rootnode, p1, radius, outputmean, outputvariance);
   if (rootnode->parent)
    {
     parent.x = MARGIN / 2 + (rootnode->parent->x + 0.5) * node_width;
     parent.y = img.height - (rootnode->parent->y + 0.5) * node_height + MARGIN / 2 + 2 * radius;
     write_line(outfile, p1.x, p1.y - 2 * radius, parent.x, parent.y - (2 * WIDTHFACTOR + 1.5) * radius);
    }   
   plot_regression_model_at_node(outfile, rootnode->left, maxdepth, maxwidth, outputmean, outputvariance);
   plot_regression_model_at_node(outfile, rootnode->right, maxdepth, maxwidth, outputmean, outputvariance);
  }  
}

void plot_regression_model_tree(Regressionnodeptr rootnode, double outputmean, double outputvariance)
{
 FILE* outfile;
 int level = 0, maxdepth = depth_of_regression_tree(rootnode), maxwidth = regressiontree_leaf_count(rootnode) + regressiontree_node_count(rootnode);
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return; 
 write_image_header(outfile, 2 * MARGIN + epsaxis, 2 * MARGIN + epsaxis);    
 level_of_regression_tree(rootnode);
 inorder_traversal_regression_tree(rootnode, &level);
 plot_regression_model_at_node(outfile, rootnode, maxdepth, maxwidth, outputmean, outputvariance);
 fclose(outfile);
 write_image_footer(outfile);
}

void plot_model_tree(Decisionnodeptr rootnode)
{
 FILE* outfile;
 int level = 0, maxdepth = depth_of_tree(rootnode), maxwidth = decisiontree_leaf_count(rootnode) + decisiontree_node_count(rootnode);
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return; 
 write_image_header(outfile, 2 * MARGIN + epsaxis, 2 * MARGIN + epsaxis);    
 level_of_decision_tree(rootnode);
 inorder_traversal_decision_tree(rootnode, &level);
 plot_decision_model_at_node(outfile, rootnode, maxdepth, maxwidth);
 fclose(outfile);
 write_image_footer(outfile);
}

void difference_plot(char** files, int filecount)
{
 int i, base, barwidth, bargap;
 int counts[2];
 char st[10];
 double** observations, *difference, max = 0, min = 0;
 FILE* outfile;
 observations = read_observations_for_two_file(files, counts);
 if (counts[0] != counts[1])
   return;
 difference = safemalloc(counts[0] * sizeof(double), "difference_plot", 7);
 for (i = 0; i < counts[0]; i++)
  {
   difference[i] = observations[0][i] - observations[1][i];
   if (difference[i] > max)
     max = difference[i];
   if (difference[i] < min)
     min = difference[i];
  }
 free_2d((void**) observations, 2);
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return; 
 write_image_header(outfile, 2 * MARGIN + epsaxis, 2 * MARGIN + epsaxis);
 base = MARGIN + epsaxis * (-min / (max - min));
 barwidth = 0.8 * (epsaxis / counts[0]);
 bargap = 0.2 * (epsaxis / counts[0]);
 write_line(outfile, MARGIN, MARGIN, MARGIN, epsaxis + MARGIN);
 write_line(outfile, MARGIN, base, epsaxis + MARGIN, base);
 for (i = 0; i <= 10; i++)
  {
   write_line(outfile, MARGIN, MARGIN + i * epsaxis / 10, MARGIN + TICKWIDTH, MARGIN + i * epsaxis / 10);
   sprintf(st, "%.2f", min + ((max - min) / 10) * i);
   write_string(outfile, MARGIN - 20, MARGIN + i * epsaxis / 10, st, ALIGN_CENTER); 
  }
 for (i = 0; i < counts[0]; i++)
   write_rectangle(outfile, MARGIN + bargap + (barwidth + bargap) * i, base, MARGIN + (barwidth + bargap) * (i + 1), base + epsaxis * (difference[i] / (max - min)), FILL_COLOR, difference[i] > 0 ? BLACK : RED);
 write_image_footer(outfile);
 fclose(outfile);
 safe_free(difference);
}

void rank_plot(char** names, int filecount)
{
 FILE* myfile, *outfile;
 int obs, i, barheight = MARGIN, bargap = SMALLMARGIN, gap = SMALLMARGIN, count = file_length(names[0]), barwidth = epsaxis / (count * filecount) - bargap;
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return; 
 write_image_header(outfile, 2 * MARGIN + epsaxis, (barheight + gap) * filecount + gap);
 for (i = 0; i < filecount; i++)
   write_line(outfile, MARGIN, (barheight + gap) * i + gap, epsaxis + MARGIN, (barheight + gap) * i + gap);
 for (i = 0; i < filecount; i++)
  {
   myfile = fopen(names[i], "r");
   if (!myfile)
    { 
     printf(m1274, names[i]);
     return;
    }
   while (fscanf(myfile, "%d\n", &obs) != EOF)
     write_rectangle(outfile, MARGIN + (barwidth + bargap) * (obs - 1), (barheight + gap) * i + gap, MARGIN + (barwidth + bargap) * (obs - 1) + barwidth, (barheight + gap) * i + 0.75 * barheight, FILL_COLOR, i % COLORCOUNT);
   fclose(myfile);
  }  
 write_image_footer(outfile);
 fclose(outfile);
}

void rangetest_plot(double* ranks, int algorithmcount, char** algorithms, double critical_difference)
{
 FILE* outfile;
 char st[3];
 int i, j, oldj, algheight, axisheight, algwidth, leftcount = 0, rightcount = 0, maxcount, linecount;
 outfile = fopen("image.eps", "w");
 if (!outfile)
   return;
 for (i = 0; i < algorithmcount; i++)
   if (ranks[i] < (1.0 + algorithmcount) / 2)
     leftcount++;
   else 
     rightcount++;
 if (leftcount > rightcount)
   maxcount = leftcount;
 else 
   maxcount = rightcount;
 leftcount = 0;
 rightcount = 0;
 clear_image();
 algheight = 0.4 * (MARGIN + SMALLMARGIN);
 algwidth = (epsaxis - MARGIN) / (algorithmcount - 1);
 axisheight = algheight * (maxcount + 1);
 write_image_header(outfile, 2 * MARGIN + epsaxis, axisheight + algheight);
 write_line(outfile, MARGIN, axisheight, epsaxis, axisheight);
 for (i = 1; i <= algorithmcount; i++)
  {
   sprintf(st, "%d", i);
   write_string(outfile, MARGIN + ((epsaxis - MARGIN) / (algorithmcount - 1)) * (i - 1), axisheight + 2 * SMALLMARGIN, st, ALIGN_CENTER);  
   write_line(outfile,  MARGIN + ((epsaxis - MARGIN) / (algorithmcount - 1)) * (i - 1), axisheight - SMALLMARGIN,  MARGIN + ((epsaxis - MARGIN) / (algorithmcount - 1)) * (i - 1), axisheight + SMALLMARGIN);
  }
 for (i = 0; i < algorithmcount; i++)
  {
   if (ranks[i] < (1.0 + algorithmcount) / 2)
    {
     leftcount++;
     write_line(outfile, MARGIN + (ranks[i] - 1) * algwidth, axisheight, MARGIN + (ranks[i] - 1) * algwidth, algheight * leftcount);
     write_line(outfile, MARGIN - 2 * SMALLMARGIN, algheight * leftcount, MARGIN + (ranks[i] - 1) * algwidth, algheight * leftcount);
     write_string(outfile, SMALLMARGIN, algheight * leftcount, algorithms[i], ALIGN_LEFT);
    }
   else
    {
     rightcount++;
     write_line(outfile, MARGIN + (ranks[i] - 1) * algwidth, axisheight, MARGIN + (ranks[i] - 1) * algwidth, algheight * rightcount);
     write_line(outfile, MARGIN + (ranks[i] - 1) * algwidth, algheight * rightcount, epsaxis + 2 * SMALLMARGIN, algheight * rightcount);
     write_string(outfile, epsaxis + 3 * SMALLMARGIN, algheight * rightcount, algorithms[i], ALIGN_LEFT);
    }
  }
 qsort(ranks, algorithmcount, sizeof(double), sort_function_double_ascending);
 linecount = 1;
 oldj = 0;
 for (i = 0; i < algorithmcount - 1; i++)
  {
   j = i + 1;
   if (ranks[j] - ranks[i] < critical_difference)
     while (j < algorithmcount && ranks[j] - ranks[i] < critical_difference)
       j++;
   j--;
   if (i != j && j > oldj)
    {
     setlinewidth(outfile, 4.0);
     write_line(outfile, MARGIN + (ranks[i] - 1) * algwidth - SMALLMARGIN, axisheight - linecount * 1.5 * SMALLMARGIN, MARGIN + (ranks[j] - 1) * algwidth + SMALLMARGIN, axisheight - linecount * 1.5 * SMALLMARGIN);
     setlinewidth(outfile, 1.0);
     linecount++;
    }
   oldj = j;
  }
 write_image_footer(outfile);
 fclose(outfile);
}

/**
 * Save the axis information to the eps file.
 * @param[in] myfile Pointer to the file stream
 * @return Number of divisions (parts) in the x axis
 */
int save_axes(FILE* myfile)
{
 /*!Last Changed 20.09.2006*/
 char st[100];
 int i, xdivision, ydivision, posx, posx2, posy, posy2;
 if (img.xaxis.available)
  {
   if (img.xaxis.lim.division == 0)
     xdivision = 10;
   else
     xdivision = img.xaxis.lim.division;
   if (get_parameter_o("showticks"))
     write_axis(myfile, xdivision, TICKWIDTH, MARGIN, img.width - MARGIN, MARGIN, X_AXIS);
   else
     write_axis(myfile, 0, TICKWIDTH, MARGIN, img.width - MARGIN, MARGIN, X_AXIS);   
  }
 else
   xdivision = 10;
 if (img.yaxis.available)
  {
   if (get_parameter_o("showticks"))
     write_axis(myfile, xdivision, -TICKWIDTH, MARGIN, img.width - MARGIN, img.height - MARGIN, X_AXIS);
   else
     write_axis(myfile, 0, -TICKWIDTH, MARGIN, img.width - MARGIN, img.height - MARGIN, X_AXIS);  
   if (img.yaxis.lim.division == 0)
     ydivision = 10;
   else
     ydivision = img.yaxis.lim.division;
   write_axis(myfile, ydivision, TICKWIDTH, MARGIN, img.height - MARGIN, MARGIN, Y_AXIS);  
   write_axis(myfile, ydivision, -TICKWIDTH, MARGIN, img.height - MARGIN, img.width - MARGIN, Y_AXIS);
  }
 else
   ydivision = 10; 
 fprintf(myfile, "stroke\n");
 if (img.xaxis.available && img.xaxis.label)
  {
   setfont(myfile, img.xaxis.labelfnt);
   write_string(myfile, img.width / 2, 30, img.xaxis.label, ALIGN_CENTER);
  }
 if (img.yaxis.available && img.yaxis.label)
  {
   setfont(myfile, img.yaxis.labelfnt);
   write_string(myfile, 10, img.height / 2, img.yaxis.label, ALIGN_ROTATE_CENTER);
  }
 if (img.xaxis.available)
  {
   setfont(myfile, img.xaxis.axisfnt);
   if (img.xaxis.namesavailable)
     for (i = 0; i < xdivision; i++)
      {
       posx = MARGIN + (i * epsaxis) / xdivision;
       posx2 = MARGIN + ((i + 1) * epsaxis) / xdivision;
       write_string(myfile, (posx + posx2) / 2, MARGIN - 10, img.xaxis.names[i], ALIGN_CENTER);
      }
   else
     for (i = 0; i <= xdivision; i++)
      {
       posx = MARGIN + (i * epsaxis) / xdivision;
       if (img.xaxis.lim.precision == 0)
         sprintf(st, "%.0f", img.xaxis.lim.min + ((img.xaxis.lim.max - img.xaxis.lim.min) / xdivision) * i);
       else
         switch (img.xaxis.lim.precision)
          {
           case 1:sprintf(st, "%.1f", img.xaxis.lim.min + ((img.xaxis.lim.max - img.xaxis.lim.min) / xdivision) * i);
                  break;
           case 2:sprintf(st, "%.2f", img.xaxis.lim.min + ((img.xaxis.lim.max - img.xaxis.lim.min) / xdivision) * i);
                  break;
           case 3:sprintf(st, "%.3f", img.xaxis.lim.min + ((img.xaxis.lim.max - img.xaxis.lim.min) / xdivision) * i);
                  break;
           default:sprintf(st, "%.1f", img.xaxis.lim.min + ((img.xaxis.lim.max - img.xaxis.lim.min) / xdivision) * i);
                   break;
          }     
       write_string(myfile, posx, MARGIN - 15, st, ALIGN_CENTER);
      }
  }
 if (img.yaxis.available)
  {
   setfont(myfile, img.yaxis.axisfnt);
   if (img.yaxis.namesavailable)
     for (i = 0; i < ydivision; i++)
      {
       posy = MARGIN + (i * epsaxis) / ydivision;
       posy2 = MARGIN + ((i + 1) * epsaxis) / ydivision;
       write_string(myfile, MARGIN - 10, (posy + posy2) / 2, img.yaxis.names[i], ALIGN_ROTATE_CENTER);
      }
   else
     for (i = 0; i <= ydivision; i++)
      {
       posy = MARGIN + (i * epsaxis) / ydivision;
       if (img.yaxis.lim.precision == 0)
         sprintf(st, "%.0f", img.yaxis.lim.min + ((img.yaxis.lim.max - img.yaxis.lim.min) / ydivision) * i);
       else
         switch (img.yaxis.lim.precision)
          {
           case 1:sprintf(st, "%.1f", img.yaxis.lim.min + ((img.yaxis.lim.max - img.yaxis.lim.min) / ydivision) * i);
                  break;
           case 2:sprintf(st, "%.2f", img.yaxis.lim.min + ((img.yaxis.lim.max - img.yaxis.lim.min) / ydivision) * i);
                  break;
           case 3:sprintf(st, "%.3f", img.yaxis.lim.min + ((img.yaxis.lim.max - img.yaxis.lim.min) / ydivision) * i);
                  break;
           default:sprintf(st, "%.1f", img.yaxis.lim.min + ((img.yaxis.lim.max - img.yaxis.lim.min) / ydivision) * i);
                   break;
          }     
       write_string(myfile, MARGIN - 5, posy, st, ALIGN_RIGHT);
      }
  }
 return xdivision; 
}

/**
 * Save the legend to the eps file. Legend contains the rectangle, the legend names and the legend subimages (such as dashed lines in different colors, very small rectangles, or different characters etc.)
 * @param[in] myfile Pointer to the file stream
 */
void save_legend(FILE* myfile)
{
 /*!Last Changed 20.09.2006*/
 char st[100], st2[100];
 int posx = 0, posy = 0, direction = 0, i, legendy, legendheight, legendwidth, maxlength = 0;
 for (i = 0; i < img.imagelegend.legendcount; i++)
   if (strlen(img.imagelegend.names[i]) > maxlength)
     maxlength = strlen(img.imagelegend.names[i]);
 legendheight = img.imagelegend.fnt.fontsize;
 legendwidth = 1.1 * img.imagelegend.fnt.fontsize * maxlength;
 if (img.imagelegend.legendcount != 0 && img.imagelegend.position != NONE)
  {
   switch (img.imagelegend.position)
    {
     case DOWN_RIGHT:posx = epsaxis + MARGIN - legendwidth - 10;
                     posy = MARGIN + 20;
                     direction = 1;
                     break;
     case DOWN_LEFT :posx = MARGIN + 10;
                     posy = MARGIN + 20;
                     direction = 1;
                     break;
     case UP_RIGHT  :posx = epsaxis + MARGIN - legendwidth - 10;
                     posy = epsaxis + MARGIN - 20;
                     direction = -1;
                     break;
     case UP_LEFT   :posx = MARGIN + 10;
                     posy = epsaxis + MARGIN - 20;
                     direction = -1;
                     break;
     default        :break;
    }
   write_rectangle(myfile, posx, posy, posx + legendwidth, posy + direction * (img.imagelegend.legendcount + 1) * legendheight, FILL_NONE, BLACK);
   setfont(myfile, img.imagelegend.fnt);   
   for (i = 0; i < img.imagelegend.legendcount; i++)
    {
     setcolor(myfile, img.imagelegend.colors[i]);
     if (direction == 1)      
       legendy = posy + (img.imagelegend.legendcount - i) * legendheight;
     else
       legendy = posy - (i + 1) * legendheight;
     sprintf(st, "%s", img.imagelegend.names[i]);
     if (between(img.imagelegend.types[i], '0', '0' + MAX_FILL_TYPE - 1))
       write_rectangle(myfile, posx + 5, legendy, posx + 20, legendy + 10, img.imagelegend.types[i] - '0', BLACK);
     else
       if (between(img.imagelegend.types[i], 'a', 'z'))
        {
         setcolor(myfile, (img.imagelegend.types[i] - 'a') % COLORCOUNT);
         write_dashed_line(myfile, posx + 5, legendy, posx + 20, legendy, (img.imagelegend.types[i] - 'a') % MAX_LINE_TYPE);
         setcolor(myfile, BLACK);
        }
       else
         if (between(img.imagelegend.types[i], 'A', 'Z'))
          {
           setcolor(myfile, img.imagelegend.types[i] - 'A');
           write_line(myfile, posx + 5, legendy, posx + 20, legendy);
           setcolor(myfile, BLACK);
          } 
         else
          {
           sprintf(st2, "%c   %s", img.imagelegend.types[i], st);
           strcpy(st, st2);
          }
     write_string(myfile, posx + 25, legendy, st, ALIGN_LEFT);
    }  
  }    
}

/**
 * Saves the color information to the eps file
 * @param[in] myfile Pointer to the file stream
 * @param[in] color Color
 */
void setcolor(FILE* myfile, COLOR_TYPE color)
{
 /*!Last Changed 23.01.2005*/
 fprintf(myfile, "%.3f %.3f %.3f setrgbcolor\n", availablecolors[color][0], availablecolors[color][1], availablecolors[color][2]);
}

/**
 * Saves the line width property to the eps file (sets the line width in the eps file)
 * @param[in] myfile Pointer to the file stream
 * @param[in] linewidth Width of the line
 */
void setlinewidth(FILE* myfile, double linewidth)
{
 /*!Last Changed 24.01.2005*/
 fprintf(myfile, "%.3f setlinewidth\n", linewidth);
}

/**
 * Saves the font information to the eps file (sets the font size, font color)
 * @param[in] myfile Pointer to the file stream
 * @param[in] fnt Current text font
 */
void setfont(FILE* myfile, Font fnt)
{
 /*!Last Changed 23.01.2005*/
 /*!Last Changed 20.09.2004*/
 fprintf(myfile, "/%s findfont %f scalefont setfont\n", availablefonts[fnt.fontname], fnt.fontsize);
 fprintf(myfile, "%.3f %.3f %.3f setrgbcolor\n", availablecolors[fnt.fontcolor][0], availablecolors[fnt.fontcolor][1], availablecolors[fnt.fontcolor][2]);
}

/**
 * Write header of the eps file including the dictionary (uses write_dictionary function for this purpose)
 * @param[in] myfile Pointer to the file stream
 * @param[in] width Width of the image
 * @param[in] height Height of the image
 */
void write_image_header(FILE* myfile,int width,int height)
{
 /*!Last Changed 12.09.2006*/ 
 fprintf(myfile, "%c!PS-Adobe-3.0 EPSF-3.0\n", '%');
 fprintf(myfile, "%c%cWritten by ISELL Environment\n", '%', '%');
 fprintf(myfile, "%c%cBoundingBox: 0 0 %d %d\n", '%', '%', width, height);
 fprintf(myfile, "/Times findfont 10 scalefont setfont\n");
 write_dictionary(myfile);
}

/**
 * Writes the dictionary to the eps file. Mainly reads from dictionary.txt file and appends it into the eps file.
 * @param[in] myfile Pointer to the file stream
 */
void write_dictionary(FILE* myfile)
{
 /*!Last Changed 12.09.2006*/
 FILE* infile;
 char myline[READLINELENGTH]; 
 infile = fopen("dictionary.txt", "r");
 if (!infile)
   return;
 while (fgets(myline, READLINELENGTH, infile))
  {
   fprintf(myfile, "%s", myline);
  }
 fclose(infile);
}

/**
 * Writes the footer of the eps file.
 * @param[in] myfile Pointer to the file stream
 */
void write_image_footer(FILE* myfile)
{
 fprintf(myfile, "stroke\n");
 fprintf(myfile, "showpage\n");
}

/**
 * Saves a dashed line object to the eps file.
 * @param[in] myfile Pointer to the file stream
 * @param[in] startx X axis value of the first point of the line
 * @param[in] starty Y axis value of the first point of the line
 * @param[in] endx X axis value of the second point of the line
 * @param[in] endy Y axis value of the second point of the line
 * @param[in] dashedtype Dash type of the dashed line
 */
void write_dashed_line(FILE* myfile, int startx, int starty, int endx, int endy, int dashedtype)
{
 /*!Last Changed 20.09.2006*/
 fprintf(myfile, "%d %d %d %d %d dashedline\n", dashedtype, startx, starty, endx, endy);
}

/**
 * Saves a circle object to the eps file.
 * @param[in] myfile Pointer to the file stream
 * @param[in] centerx X axis value of the circle center
 * @param[in] centery Y axis value of the circle center
 * @param[in] radius radius of the circle
 */
void write_circle(FILE* myfile, int centerx, int centery, int radius)
{
 /*!Last Changed 20.09.2006*/
 fprintf(myfile, "%d %d moveto\n", radius + centerx, centery);
 fprintf(myfile, "%d %d %d circle\n", radius, centerx, centery);
}

/**
 * Saves a regular line object to the eps file.
 * @param[in] myfile Pointer to the file stream
 * @param[in] startx X axis value of the first point of the line
 * @param[in] starty Y axis value of the first point of the line
 * @param[in] endx X axis value of the second point of the line
 * @param[in] endy Y axis value of the second point of the line
 */
void write_line(FILE* myfile, int startx, int starty, int endx, int endy)
{
 /*!Last Changed 20.09.2006*/
 fprintf(myfile, "%d %d %d %d line\n", startx, starty, endx, endy);
}

/**
 * Saves a string object to the eps file.
 * @param[in] myfile Pointer to the file stream
 * @param[in] posx X axis value of the starting coordinate of the string
 * @param[in] posy Y axis value of the starting coordinate of the string
 * @param[in] st String to be put
 * @param[in] align Type of alignment of the string. ALIGN_LEFT for left aligned string, ALIGN_CENTER for center aligned string, ALIGN_RIGHT for right aligned string, ALIGN_ROTATE_LEFT for rotated left aligned string, ALIGN_ROTATE_CENTER for rotated center aligned string and ALIGN_ROTATE_RIGHT for rotated right aligned string.
 */
void write_string(FILE* myfile, int posx, int posy, char* st, ALIGN_TYPE align)
{
 /*!Last Changed 19.09.2006 added clshow and crshow*/
 /*!Last Changed 07.09.2006*/
 fprintf(myfile,"%d %d moveto\n", posx, posy);
 if (in_list(align, 3, ALIGN_ROTATE_CENTER, ALIGN_ROTATE_LEFT, ALIGN_ROTATE_RIGHT))
   fprintf(myfile, "90 rotate\n");
 switch (align)
  {
   case ALIGN_LEFT         :
   case ALIGN_ROTATE_LEFT  :fprintf(myfile,"(%s) clshow\n", st);
                            break;
   case ALIGN_CENTER       :
   case ALIGN_ROTATE_CENTER:fprintf(myfile,"(%s) cshow\n", st);
                            break;
   case ALIGN_RIGHT        :
   case ALIGN_ROTATE_RIGHT :fprintf(myfile,"(%s) crshow\n", st);
                            break;
  }
 if (in_list(align, 3, ALIGN_ROTATE_CENTER, ALIGN_ROTATE_LEFT, ALIGN_ROTATE_RIGHT))
   fprintf(myfile, "-90 rotate\n");
}

/**
 * Saves a rectangle object to the eps file. A rectangle can be defined by two corner points.
 * @param[in] myfile Pointer to the file stream
 * @param[in] corner1x X axis value of the first corner
 * @param[in] corner1y Y axis value of the first corner
 * @param[in] corner2x X axis value of the second corner
 * @param[in] corner2y Y axis value of the second corner
 * @param[in] filltype Type of fill of the rectangle. FILL_NONE for not filling, FILL_FLOOD for flooded filling, FILL_GRAY for gray colored filling, FILL_VERTICAL for filling with vertical lines, FILL_HORIZONTAL for filling with horizontal lines and FILL_SQUARE for filling both with vertical and horizontal lines.
 */
void write_rectangle(FILE* myfile, int corner1x, int corner1y, int corner2x, int corner2y, FILL_TYPE filltype, int color)
{
 /*!Last Changed 18.09.2006*/
 switch (filltype)
  {
   case FILL_NONE      :fprintf(myfile, "%d %d %d %d drawrect\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_FLOOD     :fprintf(myfile, "0.0 %d %d %d %d fillrectgray\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_GRAY      :fprintf(myfile, "0.5 %d %d %d %d fillrectgray\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_VERTICAL  :fprintf(myfile, "4 %d %d %d %d verticalfill\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_HORIZONTAL:fprintf(myfile, "4 %d %d %d %d horizontalfill\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_SQUARE    :fprintf(myfile, "4 %d %d %d %d squarefill\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_COLOR     :fprintf(myfile, "%.3f %.3f %.3f %d %d %d %d fillrectcolor\n", availablecolors[color][0], availablecolors[color][1], availablecolors[color][2], corner1x, corner1y, corner2x, corner2y);
                        break;
  }
}

/**
 * Saves the axis properties to the eps file. An axis can be defined by two points. Starting point of the axis, ending point of the axis. Since axis lines are orthogonal to the image, one need only store three real values to represent any axis. For example x axis can be between (60, 50) and (560, 50) (50, 60, 560), whereas y axis can be between (50, 60) and (50, 560) (50, 60, 560).
 * @param[in] myfile Pointer to the file stream
 * @param[in] tickcount Number of ticks on the axis
 * @param[in] tickwidth Width of each interval separated by two ticks
 * @param[in] p1 The first value to defined the axis (50 for the example)
 * @param[in] p2 The second value to defined the axis (60 for the example)
 * @param[in] p3 The third value to defined the axis (560 for the example)
 * @param[in] axistype Name of the axis. X_AXIS for X axis, Y_AXIS for Y axis.
 */
void write_axis(FILE* myfile, int tickcount, int tickwidth, int p1, int p2, int p3, AXIS_TYPE axistype)
{
 /*!Last Changed 18.09.2006*/
 switch (axistype)
  {   
   case X_AXIS:fprintf(myfile, "%d %d %d %d %d xaxis\n", tickcount, tickwidth, p1, p2, p3);    
               break;
   case Y_AXIS:fprintf(myfile, "%d %d %d %d %d yaxis\n", tickcount, tickwidth, p1, p2, p3);    
               break;
  }
}
