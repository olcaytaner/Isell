#include <math.h>
#include <stdlib.h>
#include "eps.h"
#include "image.h"
#include "libmath.h"
#include "libmemory.h"
#include "libstring.h"
#include "messages.h"
#include "parameter.h"
#include "pstricks.h"

extern Image img;

/**
 * Add a legend value to the image legend.
 * @param[in] ch Represents the type of legend value. 'A' represents the type no 0, 'B' represents the type no 1, etc.
 * @param[in] st Legend text
 */
void add_legend(char ch, char* st)
{
 /*!Last Changed 19.09.2006 legendcount increased*/
 /*!Last Changed 23.01.2005*/
 int count = img.imagelegend.legendcount; 
 img.imagelegend.colors = alloc(img.imagelegend.colors, (count + 1) * sizeof(int), count + 1);
 img.imagelegend.colors[count] = 0;
 img.imagelegend.types = alloc(img.imagelegend.types, (count + 1) * sizeof(char), count + 1);
 img.imagelegend.types[count] = ch;
 img.imagelegend.names = alloc(img.imagelegend.names, (count + 1) * sizeof(char *), count + 1);
 img.imagelegend.names[count] = strcopy(img.imagelegend.names[count], st);
 img.imagelegend.legendcount++;
}

/**
 * Adds a new object to the image using the parameters given.
 * @param[in] type Type of the object
 * @param[in] p1 1. parameter containing two values x and y which store, the starting point of the line for OBJECT_LINE, the position of the string for OBJECT_STRING, the position of the point for OBJECT_NAMEDPOINT, groupindex of the OBJECT_RECTANGLE (with x value), mean and variance values for OBJECT_MVBOX, median and 25 percentile value for OBJECT_BOXPLOT, minimum and maximum values for OBJECT_ERRORHISTOGRAM.
 * @param[in] p2 2. parameter containing two values x and y which store, the ending point of the line for OBJECT_LINE, index of the OBJECT_RECTANGLE (with x value), groupindex and index respectively for OBJECT_MVBOX, 75 percentile value and index for OBJECT_BOXPLOT, group index and index for OBJECT_ERRORHISTOGRAM.
 * @param[in] p3 3. parameter containing two values x and y which store, smallest and largest whisker values for OBJECT_BOXPLOT.
 * @param[in] st 4. parameter that stores, type of the dash for OBJECT_LINE, string to be written for OBJECT_STRING, string to be written on top of the point for OBJECT_NAMEDPOINT, the group index for OBJECT_BOXPLOT, the counts of each histogram value for OBJECT_ERRORHISTOGRAM.
 * @return New object allocated and its parameters set.
 */
Objectptr add_object(OBJECT_TYPE type, Point p1, Point p2, Point p3, char* st)
{
 /*!Last Changed 04.02.2008 added namedpoint type*/
 /*!Last Changed 27.12.2007 added error_histogram_plot type*/
 /*!Last Changed 20.09.2006 added dashedtype*/
 /*!Last Changed 11.07.2005 added boxplot and mean variance plot objects*/
 /*!Last Changed 23.01.2005*/
 int i, count = img.objectcount;
 img.objects = alloc(img.objects, (count + 1) * sizeof(Object), count + 1);
 img.objects[count].type = type;
 switch (type)
  {
   case OBJECT_LINE      :initialize_font(&(img.objects[count].fnt), (int) (p3.x), 0, 1.0);
                          if (between(st[0], '0', '9'))
                            img.objects[count].fnt.dashedtype = atoi(st) % MAX_LINE_TYPE;
                          img.objects[count].obje.li.upleft = p1;
                          img.objects[count].obje.li.downright = p2;
                          break;
   case OBJECT_CIRCLE    :initialize_font(&(img.objects[count].fnt), (int) (p3.x), 0, 1.0);
                          img.objects[count].obje.ci.center = p1;
                          img.objects[count].obje.ci.radius = p2.x;
                          break;
   case OBJECT_POINT     :initialize_font(&(img.objects[count].fnt), (int) (p3.x), TIMES, 14.0);
                          img.objects[count].obje.st.p = p1;
                          img.objects[count].obje.st.st = strcopy(img.objects[count].obje.st.st, st);
                          break;
   case OBJECT_STRING    :initialize_font(&(img.objects[count].fnt), (int) (p3.x), TIMES, (int) (p3.y));
                          img.objects[count].obje.st.p = p1;
                          img.objects[count].obje.st.st = strcopy(img.objects[count].obje.st.st, st);
                          break;
   case OBJECT_NAMEDPOINT:initialize_font(&(img.objects[count].fnt), 0, TIMES, 10.0);
                          img.objects[count].obje.st.p = p1;
                          img.objects[count].obje.st.st = strcopy(img.objects[count].obje.st.st, st);
                          break;
   case OBJECT_RECTANGLE :initialize_font(&(img.objects[count].fnt), 0, 0, 1.0);
                          img.objects[count].obje.re.upleft = p1;
                          img.objects[count].obje.re.downright = p2;
                          img.objects[count].groupindex = (int) p1.x;
                          img.objects[count].index = (int) p2.x;
                          break;
   case OBJECT_MVBOX     :initialize_font(&(img.objects[count].fnt), 0, 0, 1.0);
                          img.objects[count].obje.mv.mean = p1.x;
                          img.objects[count].obje.mv.variance = p1.y;
                          img.objects[count].groupindex = (int) p2.x;
                          img.objects[count].index = (int) p2.y;
                          break;
   case OBJECT_BOXPLOT   :initialize_font(&(img.objects[count].fnt), 0, 0, 1.0);
                          img.objects[count].obje.box.median = p1.x;
                          img.objects[count].obje.box.p25 = p1.y;
                          img.objects[count].obje.box.p75 = p2.x;
                          img.objects[count].obje.box.swhisker = p3.x;
                          img.objects[count].obje.box.lwhisker = p3.y;
                          img.objects[count].groupindex = atoi(st);
                          img.objects[count].index = (int) p2.y;
                          break;
   case OBJECT_ERRORHISTOGRAM:initialize_font(&(img.objects[count].fnt), 0, 0, 1.0);
                              img.objects[count].obje.errorhist.min = p1.x;
                              img.objects[count].obje.errorhist.max = p1.y;
                              for (i = 0; i < 10; i++)
                                img.objects[count].obje.errorhist.counts[i] = st[i] - '0';
                              img.objects[count].groupindex = (int) p2.x;
                              img.objects[count].index = (int) p2.y;
                              break;
   default               :printf(m1233, type);
                          break;
  } 
 img.objectcount++;
 return &(img.objects[count]);
}

/**
 * Exchange function used in sorting the objects.
 * @param[in] i Index of the first object
 * @param[in] j Index of the second object
 */
void exchange_objects(int i, int j)
{
 /*!Last Changed 20.09.2006*/
 Object tmp;
 tmp = img.objects[i];
 img.objects[i] = img.objects[j];
 img.objects[j] = tmp;
}

void save_image(char* fname)
{
 int i, j;
 FILE* outfile;
 outfile = fopen(fname, "w");
 fprintf(outfile, "%d %d %d %d\n", img.width, img.height, img.objectcount, img.groupcoloring);
 /* Save x axis */
 fprintf(outfile, "%d %d %d\n", img.xaxis.available, img.xaxis.namesavailable, img.xaxis.limitsset);
 fprintf(outfile, "%s\n", img.xaxis.label);
 fprintf(outfile, "%d %d %.6f %.6f\n", img.xaxis.lim.division, img.xaxis.lim.precision, img.xaxis.lim.min, img.xaxis.lim.max);
 fprintf(outfile, "%d %.6f %d %d\n", img.xaxis.labelfnt.dashedtype, img.xaxis.labelfnt.fontsize, img.xaxis.labelfnt.fontname, img.xaxis.labelfnt.fontcolor);
 fprintf(outfile, "%d %.6f %d %d\n", img.xaxis.axisfnt.dashedtype, img.xaxis.axisfnt.fontsize, img.xaxis.axisfnt.fontname, img.xaxis.axisfnt.fontcolor);
 if (img.xaxis.namesavailable)
   for (i = 0; i < img.xaxis.lim.division; i++)
     fprintf(outfile, "%s\n", img.xaxis.names[i]);
 /* Save y axis */
 fprintf(outfile, "%d %d %d\n", img.yaxis.available, img.yaxis.namesavailable, img.yaxis.limitsset);
 if (img.yaxis.available)
  {
   fprintf(outfile, "%s\n", img.yaxis.label);
   fprintf(outfile, "%d %d %.6f %.6f\n", img.yaxis.lim.division, img.yaxis.lim.precision, img.yaxis.lim.min, img.yaxis.lim.max);
   fprintf(outfile, "%d %.6f %d %d\n", img.yaxis.labelfnt.dashedtype, img.yaxis.labelfnt.fontsize, img.yaxis.labelfnt.fontname, img.yaxis.labelfnt.fontcolor);
   fprintf(outfile, "%d %.6f %d %d\n", img.yaxis.axisfnt.dashedtype, img.yaxis.axisfnt.fontsize, img.yaxis.axisfnt.fontname, img.yaxis.axisfnt.fontcolor);
   if (img.yaxis.namesavailable)
     for (i = 0; i < img.yaxis.lim.division; i++)
       fprintf(outfile, "%s\n", img.yaxis.names[i]);
  }
 /* Save Objects */
 for (i = 0; i < img.objectcount; i++)
  {
   fprintf(outfile, "%d %d %d\n", img.objects[i].type, img.objects[i].index, img.objects[i].groupindex);
   fprintf(outfile, "%d %.6f %d %d\n", img.objects[i].fnt.dashedtype, img.objects[i].fnt.fontsize, img.objects[i].fnt.fontname, img.objects[i].fnt.fontcolor);
   switch (img.objects[i].type)
    {
     case       OBJECT_LINE:fprintf(outfile, "%.6f %.6f %.6f %.6f\n", img.objects[i].obje.li.downright.x, img.objects[i].obje.li.downright.y, img.objects[i].obje.li.upleft.x, img.objects[i].obje.li.upleft.y);
                            break;
     case     OBJECT_CIRCLE:fprintf(outfile, "%.6f %.6f %.6f\n", img.objects[i].obje.ci.center.x, img.objects[i].obje.ci.center.y, img.objects[i].obje.ci.radius);
                            break;
     case      OBJECT_POINT:
     case     OBJECT_STRING:
     case OBJECT_NAMEDPOINT:fprintf(outfile, "%.6f %.6f %s\n", img.objects[i].obje.st.p.x, img.objects[i].obje.st.p.y, img.objects[i].obje.st.st);
                            break;
     case  OBJECT_RECTANGLE:fprintf(outfile, "%.6f %.6f %.6f %.6f\n", img.objects[i].obje.re.downright.x, img.objects[i].obje.re.downright.y, img.objects[i].obje.re.upleft.x, img.objects[i].obje.re.upleft.y);
                            break;
     case      OBJECT_MVBOX:fprintf(outfile, "%.6f %.6f\n", img.objects[i].obje.mv.mean, img.objects[i].obje.mv.variance);
                            break;
     case    OBJECT_BOXPLOT:fprintf(outfile, "%.6f %.6f %.6f %.6f %.6f\n", img.objects[i].obje.box.lwhisker, img.objects[i].obje.box.swhisker, img.objects[i].obje.box.p75, img.objects[i].obje.box.p25, img.objects[i].obje.box.median);
                            break;
     case OBJECT_ERRORHISTOGRAM:fprintf(outfile, "%.6f %.6f\n", img.objects[i].obje.errorhist.min, img.objects[i].obje.errorhist.max);
                                 for (j = 0; j < 10; j++)
                                   fprintf(outfile, "%d ", img.objects[i].obje.errorhist.counts[j]);
                                 fprintf(outfile, "\n");
                                 break;
    }
  }
 /* Save Legend */
 fprintf(outfile, "%d %d\n", img.imagelegend.position, img.imagelegend.legendcount);
 fprintf(outfile, "%d %.6f %d %d\n", img.imagelegend.fnt.dashedtype, img.imagelegend.fnt.fontsize, img.imagelegend.fnt.fontname, img.imagelegend.fnt.fontcolor);
 for (i = 0; i < img.imagelegend.legendcount; i++)
   fprintf(outfile, "%s\n", img.imagelegend.names[i]);
 for (i = 0; i < img.imagelegend.legendcount; i++)
   fprintf(outfile, "%d ", img.imagelegend.colors[i]);
 fprintf(outfile, "\n");
 for (i = 0; i < img.imagelegend.legendcount; i++)
   fprintf(outfile, "%c", img.imagelegend.types[i]);
 fprintf(outfile, "\n");
 fclose(outfile);
}

void load_image(char* fname)
{
 int i, j, objecttype, legendposition, coloringtype, fonttype, colortype;
 FILE* infile;
 char buffer[READLINELENGTH]; 
 clear_image();
 infile = fopen(fname, "r");
 fscanf(infile, "%d %d %d %d", &(img.width), &(img.height), &(img.objectcount), &coloringtype);
 img.groupcoloring = coloringtype;
 /* Load x axis */
 fscanf(infile, "%d %d %d", &(img.xaxis.available), &(img.xaxis.namesavailable), &(img.xaxis.limitsset));
 fscanf(infile, "%s", buffer);
 img.xaxis.label = strcopy(img.xaxis.label, buffer);
 fscanf(infile, "%d %d %lf %lf", &(img.xaxis.lim.division), &(img.xaxis.lim.precision), &(img.xaxis.lim.min), &(img.xaxis.lim.max));
 fscanf(infile, "%d %lf %d %d", &(img.xaxis.labelfnt.dashedtype), &(img.xaxis.labelfnt.fontsize), &fonttype, &colortype);
 img.xaxis.labelfnt.fontname = fonttype;
 img.xaxis.labelfnt.fontcolor = colortype;
 fscanf(infile, "%d %lf %d %d", &(img.xaxis.axisfnt.dashedtype), &(img.xaxis.axisfnt.fontsize), &fonttype, &colortype);
 img.xaxis.axisfnt.fontname = fonttype;
 img.xaxis.axisfnt.fontcolor = colortype;
 if (img.xaxis.namesavailable)
  {
   img.xaxis.names = safemalloc(img.xaxis.lim.division * sizeof(char *), "load_image", 12);
   for (i = 0; i < img.xaxis.lim.division; i++)
    {
     fscanf(infile, "%s", buffer);
     img.xaxis.names[i] = strcopy(img.xaxis.names[i], buffer);
    }
  }
 else
   img.xaxis.names = NULL;
 /* Load y axis */
 fscanf(infile, "%d %d %d", &(img.yaxis.available), &(img.yaxis.namesavailable), &(img.yaxis.limitsset));
 if (img.yaxis.available)
  {
   fscanf(infile, "%s", buffer);
   img.yaxis.label = strcopy(img.yaxis.label, buffer);
   fscanf(infile, "%d %d %lf %lf", &(img.yaxis.lim.division), &(img.yaxis.lim.precision), &(img.yaxis.lim.min), &(img.yaxis.lim.max));
   fscanf(infile, "%d %lf %d %d", &(img.yaxis.labelfnt.dashedtype), &(img.yaxis.labelfnt.fontsize), &fonttype, &colortype);
   img.yaxis.labelfnt.fontname = fonttype;
   img.yaxis.labelfnt.fontcolor = colortype;
   fscanf(infile, "%d %lf %d %d", &(img.yaxis.axisfnt.dashedtype), &(img.yaxis.axisfnt.fontsize), &fonttype, &colortype);
   img.yaxis.axisfnt.fontname = fonttype;
   img.yaxis.axisfnt.fontcolor = colortype;
   if (img.yaxis.namesavailable)
    {
     img.yaxis.names = safemalloc(img.yaxis.lim.division * sizeof(char *), "load_image", 12);
     for (i = 0; i < img.yaxis.lim.division; i++)
      {
       fscanf(infile, "%s", buffer);
       img.yaxis.names[i] = strcopy(img.yaxis.names[i], buffer);
      }
    }
   else
     img.yaxis.names = NULL;
  }
 else
  {
   img.yaxis.label = NULL;
   img.yaxis.namesavailable = BOOLEAN_FALSE;
   img.yaxis.names = NULL;
  }
 /* Load Objects */
 img.objects = safemalloc(img.objectcount * sizeof(Object), "load_image", 25);
 for (i = 0; i < img.objectcount; i++)
  {
   fscanf(infile, "%d %d %d", &objecttype, &(img.objects[i].index), &(img.objects[i].groupindex));
   img.objects[i].type = objecttype;
   fscanf(infile, "%d %lf %d %d", &(img.objects[i].fnt.dashedtype), &(img.objects[i].fnt.fontsize), &fonttype, &colortype);
   img.objects[i].fnt.fontname = fonttype;
   img.objects[i].fnt.fontcolor = colortype;
   switch (img.objects[i].type)
    {
     case      OBJECT_LINE:fscanf(infile, "%lf %lf %lf %lf", &(img.objects[i].obje.li.downright.x), &(img.objects[i].obje.li.downright.y), &(img.objects[i].obje.li.upleft.x), &(img.objects[i].obje.li.upleft.y));
                           break;
     case    OBJECT_CIRCLE:fscanf(infile, "%lf %lf %lf", &(img.objects[i].obje.ci.center.x), &(img.objects[i].obje.ci.center.y), &(img.objects[i].obje.ci.radius));
                           break;
     case      OBJECT_POINT:
     case     OBJECT_STRING:
     case OBJECT_NAMEDPOINT:fscanf(infile, "%lf %lf", &(img.objects[i].obje.st.p.x), &(img.objects[i].obje.st.p.y));
                           fscanf(infile, "%s", buffer);
                           img.objects[i].obje.st.st = strcopy(img.objects[i].obje.st.st, buffer);
                           break;
     case OBJECT_RECTANGLE:fscanf(infile, "%lf %lf %lf %lf", &(img.objects[i].obje.re.downright.x), &(img.objects[i].obje.re.downright.y), &(img.objects[i].obje.re.upleft.x), &(img.objects[i].obje.re.upleft.y));
                           break;
     case     OBJECT_MVBOX:fscanf(infile, "%lf %lf", &(img.objects[i].obje.mv.mean), &(img.objects[i].obje.mv.variance));
                           break;
     case   OBJECT_BOXPLOT:fscanf(infile, "%lf %lf %lf %lf %lf", &(img.objects[i].obje.box.lwhisker), &(img.objects[i].obje.box.swhisker), &(img.objects[i].obje.box.p75), &(img.objects[i].obje.box.p25), &(img.objects[i].obje.box.median));
                           break;
     case  OBJECT_ERRORHISTOGRAM:fscanf(infile, "%lf %lf\n", &(img.objects[i].obje.errorhist.min), &(img.objects[i].obje.errorhist.max));
                                 for (j = 0; j < 10; j++)
                                   fscanf(infile, "%d", &(img.objects[i].obje.errorhist.counts[j]));
                                 break;
    }
  }
 /* Load Legend */
 fscanf(infile, "%d %d", &legendposition, &(img.imagelegend.legendcount));
 img.imagelegend.position = legendposition;
 fscanf(infile, "%d %lf %d %d", &(img.imagelegend.fnt.dashedtype), &(img.imagelegend.fnt.fontsize), &fonttype, &colortype);
 img.imagelegend.fnt.fontname = fonttype;
 img.imagelegend.fnt.fontcolor = colortype;
 img.imagelegend.names = safemalloc(img.imagelegend.legendcount * sizeof(char *), "load_image", 25);
 for (i = 0; i < img.imagelegend.legendcount; i++)
  {
   fscanf(infile, "%s", buffer);
   img.imagelegend.names[i] = strcopy(img.imagelegend.names[i], buffer);
  } 
 img.imagelegend.colors = safemalloc(img.imagelegend.legendcount * sizeof(COLOR_TYPE), "load_image", 26);
 for (i = 0; i < img.imagelegend.legendcount; i++)
  {
   fscanf(infile, "%d", &colortype);
   img.imagelegend.colors[i] = colortype;
  }
 img.imagelegend.types = safemalloc(img.imagelegend.legendcount * sizeof(char), "load_image", 30);
 for (i = 0; i < img.imagelegend.legendcount; i++)
   fscanf(infile, "%c", &(img.imagelegend.types[i]));
 fclose(infile);
}

/**
 * Checks if the objects in the image are in the bounds of the image. If not, the function will change the axis limits accordingly.
 */
void check_axis_ranges()
{
 /*!Last Changed 04.02.2008 added object namedpoint*/
 /*!Last Changed 23.01.2005*/
 int i;
 double m, s;
 Point p;
 for (i = 0; i < img.objectcount; i++)
   switch (img.objects[i].type) 
    {
     case OBJECT_LINE      :p = img.objects[i].obje.li.upleft;   
                            if (!img.xaxis.limitsset)
                              check_x_range(p);
                            if (!img.yaxis.limitsset)
                              check_y_range(p);
                            p = img.objects[i].obje.li.downright;
                            if (!img.xaxis.limitsset)
                              check_x_range(p);
                            if (!img.yaxis.limitsset)
                              check_y_range(p);
                            break;
     case OBJECT_CIRCLE    :p = img.objects[i].obje.ci.center;
                            p.x += img.objects[i].obje.ci.radius;
                            if (!img.xaxis.limitsset)
                              check_x_range(p);
                            p.x -= 2 * img.objects[i].obje.ci.radius;
                            if (!img.xaxis.limitsset)
                              check_x_range(p);
                            p = img.objects[i].obje.ci.center;
                            p.y += img.objects[i].obje.ci.radius;
                            if (!img.yaxis.limitsset)
                              check_y_range(p);
                            p.y -= 2 * img.objects[i].obje.ci.radius;
                            if (!img.yaxis.limitsset)
                              check_y_range(p);
                            break;
     case OBJECT_POINT     :
     case OBJECT_STRING    :
     case OBJECT_NAMEDPOINT:p = img.objects[i].obje.st.p;
                            if (!img.xaxis.limitsset)
                              check_x_range(p);
                            if (!img.yaxis.limitsset)
                              check_y_range(p);
                            break;
     case OBJECT_MVBOX     :if (!img.yaxis.limitsset)
                             {
                              img.yaxis.lim.min = 0;
                              m = img.objects[i].obje.mv.mean;
                              s = img.objects[i].obje.mv.variance;
                              if (m - s < img.yaxis.lim.min)
                                img.yaxis.lim.min = m - s;
                              if (m + s > img.yaxis.lim.max)
                                img.yaxis.lim.max = m + s;
                             }
                            break;
     case OBJECT_BOXPLOT   :if (!img.yaxis.limitsset)
                             {
                              img.yaxis.lim.min = 0;
                              if (img.objects[i].obje.box.swhisker < img.yaxis.lim.min)
                                img.yaxis.lim.min = img.objects[i].obje.box.swhisker;
                              if (img.objects[i].obje.box.lwhisker > img.yaxis.lim.max)
                                img.yaxis.lim.max = img.objects[i].obje.box.lwhisker;
                             }
                            break;
     default               :printf("This object type not supported\n");
                            break;
    }
 if (!img.xaxis.limitsset)
   set_limits_automatically(&(img.xaxis.lim));
 if (!img.yaxis.limitsset)
   set_limits_automatically(&(img.yaxis.lim));
}

/**
 * Checks if the point is in the x axis limits. If it is not, the limits are changed to include the point accordingly.
 * @param[in] p The point
 */
void check_x_range(Point p)
{
 /*!Last Changed 23.01.2005*/
 if (p.x < img.xaxis.lim.min)
   img.xaxis.lim.min = p.x;
 else
   if (p.x > img.xaxis.lim.max)
     img.xaxis.lim.max = p.x;
}

/**
 * Checks if the point is in the y axis limits. If it is not, the limits are changed to include the point accordingly.
 * @param[in] p The point
 */
void check_y_range(Point p)
{
 /*!Last Changed 23.01.2005*/
 if (p.y < img.yaxis.lim.min)
   img.yaxis.lim.min = p.y;
 else
   if (p.y > img.yaxis.lim.max)
     img.yaxis.lim.max = p.y;
}

/**
 * Clears the image in ISELL. Deallocates memory allocated for the parts of the image, such as x and y axis labels, legend texts, colors, strings of the objects containing strings, etc. This function is different than initialize_image because the latter one is called only once in the beginning of the program whereas the first one is called when a new image is being produced.
 */
void clear_image()
{
 /*!Last Changed 15.09.2006 added namesavailable = 0*/
 /*!Last Changed 13.09.2006 added default position*/
 /*!Last Changed 23.01.2005*/
 int i;
 img.height = epsaxis + 2 * MARGIN;
 img.width = epsaxis + 2 * MARGIN;
 img.groupcoloring = ALL_SAME;
 img.xaxis.available = 1;
 img.yaxis.available = 1;
 img.xaxis.limitsset = 0;
 img.yaxis.limitsset = 0;
 img.xaxis.lim.precision = 2;
 img.yaxis.lim.precision = 2;
 img.xaxis.lim.min = -1;
 img.xaxis.lim.max = -2;
 img.yaxis.lim.min = -1;
 img.yaxis.lim.max = -2;
 if (img.xaxis.label)
  {
   safe_free(img.xaxis.label);
   img.xaxis.label = NULL;
  }
 if (img.xaxis.namesavailable)
  {
   for (i = 0; i < img.xaxis.lim.division; i++)
     safe_free(img.xaxis.names[i]);
   safe_free(img.xaxis.names);
  }
 img.xaxis.namesavailable = 0;
 img.xaxis.names = NULL;
 if (img.yaxis.available)
  {
   if (img.yaxis.label)
    {
     safe_free(img.yaxis.label);
     img.yaxis.label = NULL;
    }
  }
 if (img.yaxis.namesavailable)
  {
   for (i = 0; i < img.yaxis.lim.division; i++)
     safe_free(img.yaxis.names[i]);
   safe_free(img.yaxis.names);
  }
 img.yaxis.namesavailable = 0;
 img.yaxis.names = NULL;
 if (img.imagelegend.legendcount != 0)
  {
   safe_free(img.imagelegend.colors);
   safe_free(img.imagelegend.types);
   for (i = 0; i < img.imagelegend.legendcount; i++)
     safe_free(img.imagelegend.names[i]);  
  }
 img.imagelegend.legendcount = 0;
 img.imagelegend.position = 0;
 if (img.objectcount != 0)
  {
   for (i = 0; i < img.objectcount; i++)
     if (img.objects[i].type == 1) 
       safe_free(img.objects[i].obje.st.st);
   safe_free(img.objects);
  }
 img.objectcount = 0;
}

/**
 * Initializes the font structure.
 * @param[out] f Font structure
 * @param[in] color Color of the font
 * @param[in] name Name of the font
 * @param[in] size Size of the font
 */
void initialize_font(Font* f, int color, FONT_TYPE name, double size)
{
 /*!Last Changed 23.01.2005*/
 f->fontcolor = color;
 f->fontname = name;
 f->fontsize = size;
}

/**
 * Initializes the image in ISELL. Sets the values of the fields of the img structure to initial values.
 */
void initialize_image()
{
 /*!Last Changed 23.01.2005*/
 img.height = epsaxis + 2 * MARGIN;
 img.width = epsaxis + 2 * MARGIN;
 img.groupcoloring = ALL_SAME;
 img.xaxis.limitsset = 0;
 img.yaxis.limitsset = 0;
 img.xaxis.label = NULL;
 img.yaxis.label = NULL;
 img.xaxis.available = 1;
 img.yaxis.available = 1;
 img.xaxis.namesavailable = 0;
 img.yaxis.namesavailable = 0;
 img.imagelegend.legendcount = 0;
 img.imagelegend.colors = NULL;
 img.imagelegend.types = NULL;
 img.imagelegend.names = NULL;
 img.objectcount = 0;
 img.xaxis.lim.min = -1;
 img.xaxis.lim.max = -2;
 img.yaxis.lim.min = -1;
 img.yaxis.lim.max = -2;
 initialize_font(&(img.xaxis.axisfnt), 0, TIMES, 10.0);
 initialize_font(&(img.yaxis.axisfnt), 0, TIMES, 10.0);
 initialize_font(&(img.xaxis.labelfnt), 0, TIMES, 10.0);
 initialize_font(&(img.yaxis.labelfnt), 0, TIMES, 10.0);
 initialize_font(&(img.imagelegend.fnt), 0, TIMES, 10.0);
}

/**
 * Counts the number objects having a specific group index
 * @param[in] type Type of the object. 
 * @param[in] groupindex Group index of the object
 * @return Number of objects with type type and having the group index groupindex
 */
int object_count(int type, int groupindex)
{
 /*!Last Changed 21.09.2006 added groupindex*/
 /*!Last Changed 11.07.2005*/
 int i, count = 0;
 for (i = 0; i < img.objectcount; i++)
   if (img.objects[i].type == type && img.objects[i].groupindex == groupindex)
     count++;
 return count;
}

double point_to_point_distance(Point p1, Point p2)
{
 /*!Last Changed 19.09.2004*/
 return (double) sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

/**
 * Calculates the x position of an object, which occurs in groups in the image.
 * @param[in] groupindex Index of the group of this object belongs to
 * @param[in] index Index of the object in its group
 * @param[in] groupcount Number of groups in the image
 * @param[in] elementcount Number of elements in each group
 * @param[out] barwidth Calculated width of the object in the image
 * @return X position of the object given its group index and its index in its group
 */
int position_in_multi_plot(int groupindex, int index, int groupcount, int elementcount, double* barwidth)
{
 /*!Last Changed 28.03.2007 fixed error by replacing groupcount with elementcount*/
 /*!Last Changed 21.09.2006*/
 double singlewidth, emptywidth;
 singlewidth = epsaxis / (groupcount + 0.0);
 emptywidth = singlewidth * ((sqrt(5) - 1) / ((elementcount + 2) * sqrt(5) + (elementcount - 2))); 
 *barwidth = singlewidth * ((sqrt(5) + 1) / ((elementcount + 2) * sqrt(5) + (elementcount - 2)));
 return MARGIN + (int) (singlewidth * groupindex + emptywidth + index * (*barwidth));
}

int return_pos(double value, int start, int end, double min, double max)
{
 return start + (int)((end - start) * (value - min) / (max - min));
}

/**
 * Set the limits of an axis automatically. The minimum, maximum and the corresponding divisions are calculated using heuristic principles. The minimum and maximum values will be the mutliples of 10's powers, such as 200, 300, etc.
 * @param[out] ax Current and will be changed limits
 */
void set_limits_automatically(Limits* ax)
{
 /*!Last Changed 13.09.2006*/
 /*!Last Changed 25.01.2005*/
 double difference, power;
 int digits, firstdigit, seconddigit, digitsp, digitsm;
 if (ax->max < 0)
   return;
 if (ax->min < 0)
  {
   digitsm = (int) log10(-ax->min);
   digitsp = (int) log10(ax->max);
   if (digitsp >= digitsm)
     digits = digitsp;
   else
     digits = digitsm;
   power = fpow(10, digits);
   if (((int) (ax->max / power)) * power != ax->max)
     ax->max = ((int) (ax->max / power + 1)) * power;
   if (((int) (-ax->min / power)) * power != -ax->min)
     ax->min = -(((int) (-ax->min / power + 1)) * power);
   ax->division = (int) (ax->max / power) + (int) (-ax->min / power);
   if (digits >= 0)
     ax->precision = 0;
   else
     ax->precision = abs(digits);
   return;
  }
 difference = ax->max - ax->min;
 firstdigit = (int) log10(ax->min);
 seconddigit = (int) log10(ax->max);
 digits = (int) log10(difference);
 if (firstdigit == seconddigit)
  {
   power = fpow(10, firstdigit);
   ax->min = ((int) (ax->min / power)) * power;
   if (ax->max != (int) (ax->max))
     ax->max = ((int) (ax->max / power + 1)) * power;
   else
     ax->max = ((int) (ax->max / power)) * power;
   ax->division = (int) ((ax->max - ax->min) / power);
  }
 else
  {
   ax->min = 0;
   power = fpow(10, seconddigit);
   if (((int) (ax->max / power)) * power != ax->max)
     ax->max = ((int) (ax->max / power + 1)) * power;
   ax->division = (int) (ax->max / power);
  }
 if (digits >= 0)
   ax->precision = 0;
 else
   ax->precision = abs(digits);
}

void export_image()
{
 switch (get_parameter_l("imagetype"))
  {
        case IMAGE_EPS:export_image_as_eps("image.eps");
                       break;
   case IMAGE_PSTRICKS:export_image_as_pstricks("image.txt");
                       break;
  }
}
