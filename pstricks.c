#include <math.h>
#include <stdio.h>
#include "libfile.h"
#include "libmemory.h"
#include "libmisc.h"
#include "messages.h"
#include "parameter.h"
#include "pstricks.h"
#include "tests.h"

char* pstrickscolors[12]={"black", "red", "green", "blue", "purple", "yellow", "cyan", "gray", "orange", "brown", "pink", "white"};

extern Image img;

void write_string_pstricks(FILE* myfile, double posx, double posy, char* st, ALIGN_TYPE align)
{
 switch (align)
  {
   case ALIGN_LEFT         :fprintf(myfile, "\\rput[lb](%f,%f){%s}\n", posx, posy, st);
                            break;
   case ALIGN_ROTATE_LEFT  :fprintf(myfile, "\\rput[lb]{90}(%f,%f){%s}\n", posx, posy, st);
                            break;
   case ALIGN_CENTER       :fprintf(myfile, "\\rput[b](%f,%f){%s}\n", posx, posy, st);
                            break;
   case ALIGN_ROTATE_CENTER:fprintf(myfile, "\\rput[b]{90}(%f,%f){%s}\n", posx, posy, st);
                            break;
   case ALIGN_RIGHT        :fprintf(myfile, "\\rput[rb](%f,%f){%s}\n", posx, posy, st);
                            break;
   case ALIGN_ROTATE_RIGHT :fprintf(myfile, "\\rput[rb]{90}(%f,%f){%s}\n", posx, posy, st);
                            break;
  }
}

void write_rectangle_pstricks(FILE* myfile, double corner1x, double corner1y, double corner2x, double corner2y, FILL_TYPE filltype, int color)
{
 switch (filltype)
  {
   case FILL_NONE      :fprintf(myfile, "\\psframe(%f, %f)(%f, %f)\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_FLOOD     :fprintf(myfile, "\\psframe[fillstyle=solid,fillcolor=black](%f, %f)(%f, %f)\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_GRAY      :fprintf(myfile, "\\psframe[fillstyle=solid,fillcolor=lightgray](%f, %f)(%f, %f)\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_VERTICAL  :fprintf(myfile, "\\psframe[fillstyle=vlines](%f, %f)(%f, %f)\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_HORIZONTAL:fprintf(myfile, "\\psframe[fillstyle=hlines](%f, %f)(%f, %f)\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_SQUARE    :fprintf(myfile, "\\psframe[fillstyle=crosshatch](%f, %f)(%f, %f)\n", corner1x, corner1y, corner2x, corner2y);
                        break;
   case FILL_COLOR     :fprintf(myfile, "\\psframe[fillstyle=solid,fillcolor=%s](%f, %f)(%f, %f)\n", pstrickscolors[color], corner1x, corner1y, corner2x, corner2y);
                        break;
  }
}

void write_line_pstricks(FILE* myfile, double startx, double starty, double endx, double endy)
{
 fprintf(myfile, "\\psline(%f,%f)(%f,%f)\n", startx, starty, endx, endy);
}

void write_dashed_line_pstricks(FILE* myfile, double startx, double starty, double endx, double endy, int dashedtype)
{
 switch (dashedtype)
  {
   case 0:fprintf(myfile, "\\psline[linestyle=solid](%f,%f)(%f,%f)\n", startx, starty, endx, endy);
          break;
   case 1:fprintf(myfile, "\\psline[linestyle=dashed,dash=4pt 1pt](%f,%f)(%f,%f)\n", startx, starty, endx, endy);
          break;
   case 2:fprintf(myfile, "\\psline[linestyle=dashed,dash=1pt 2pt](%f,%f)(%f,%f)\n", startx, starty, endx, endy);
          break;
   case 3:fprintf(myfile, "\\psline[linestyle=dotted](%f,%f)(%f,%f)\n", startx, starty, endx, endy);
          break;
   case 4:fprintf(myfile, "\\psline[linestyle=dashed,dash=10pt 1pt](%f,%f)(%f,%f)\n", startx, starty, endx, endy);
          break;
  }
}

void write_circle_pstricks(FILE* myfile, double centerx, double centery, double radius)
{
 fprintf(myfile, "\\pscircle(%f,%f){%f}\n", centerx, centery, radius);
}

void write_image_header_pstricks(FILE* myfile, double startx, double starty, double endx, double endy)
{
 fprintf(myfile, "\\begin{pspicture*}(%f,%f)(%f,%f)\n", startx, starty, endx, endy);
}

void write_image_footer_pstricks(FILE* myfile)
{
 fprintf(myfile, "\\end{pspicture*}\n");
}

void set_line_width_pstricks(FILE* myfile, double width)
{
 fprintf(myfile, "\\psset{linewidth=%fpt}\n", width);
}

void set_color_pstricks(FILE* myfile, int color)
{
 fprintf(myfile, "\\psset{linecolor=%s}\n", pstrickscolors[color]);
}

void save_axes_pstricks(FILE* myfile)
{
 double xdivision, ydivision;
 char* ticks_none = "none";
 char* ticks_all = "all";
 char* ticks;
 if (img.xaxis.lim.division == 0)
   xdivision = (img.xaxis.lim.max - img.xaxis.lim.min) / 10;
 else
   xdivision = (img.xaxis.lim.max - img.xaxis.lim.min) / img.xaxis.lim.division;
 if (img.yaxis.lim.division == 0)
   ydivision = (img.yaxis.lim.max - img.yaxis.lim.min) / 10;
 else
   ydivision = (img.yaxis.lim.max - img.yaxis.lim.min) / img.yaxis.lim.division;
 if (get_parameter_o("showticks"))
   ticks = ticks_all;
 else
   ticks = ticks_none;
 if (img.xaxis.available && img.yaxis.available)
  {
   switch (img.xaxis.lim.precision)
    {
      case 0:
     default:switch (img.yaxis.lim.precision)
              {
                case 0:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.0f,Dy=%.0f,Ox=%.0f,Oy=%.0f](%.0f,%.0f)(%.0f,%.0f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
               default:break;
                case 1:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.0f,Dy=%.1f,Ox=%.0f,Oy=%.1f](%.0f,%.1f)(%.0f,%.1f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 2:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.0f,Dy=%.2f,Ox=%.0f,Oy=%.2f](%.0f,%.2f)(%.0f,%.2f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 3:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.0f,Dy=%.3f,Ox=%.0f,Oy=%.3f](%.0f,%.3f)(%.0f,%.3f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
               }
             break;
      case 1:switch (img.yaxis.lim.precision)
              {
                case 0:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.1f,Dy=%.0f,Ox=%.1f,Oy=%.0f](%.1f,%.0f)(%.1f,%.0f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
               default:break;
                case 1:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.1f,Dy=%.1f,Ox=%.1f,Oy=%.1f](%.1f,%.1f)(%.1f,%.1f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 2:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.1f,Dy=%.2f,Ox=%.1f,Oy=%.2f](%.1f,%.2f)(%.1f,%.2f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 3:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.1f,Dy=%.3f,Ox=%.1f,Oy=%.3f](%.1f,%.3f)(%.1f,%.3f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
              }
             break;
      case 2:switch (img.yaxis.lim.precision)
              {
                case 0:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.2f,Dy=%.0f,Ox=%.2f,Oy=%.0f](%.2f,%.0f)(%.2f,%.0f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
               default:break;
                case 1:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.2f,Dy=%.1f,Ox=%.2f,Oy=%.1f](%.2f,%.1f)(%.2f,%.1f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 2:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.2f,Dy=%.2f,Ox=%.2f,Oy=%.2f](%.2f,%.2f)(%.2f,%.2f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 3:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.2f,Dy=%.3f,Ox=%.2f,Oy=%.3f](%.2f,%.3f)(%.2f,%.3f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
              }
             break;
      case 3:switch (img.yaxis.lim.precision)
              {
                case 0:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.3f,Dy=%.0f,Ox=%.3f,Oy=%.0f](%.3f,%.0f)(%.3f,%.0f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
               default:break;
                case 1:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.3f,Dy=%.1f,Ox=%.3f,Oy=%.1f](%.3f,%.1f)(%.3f,%.1f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 2:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.3f,Dy=%.2f,Ox=%.3f,Oy=%.2f](%.3f,%.2f)(%.3f,%.2f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
                case 3:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.3f,Dy=%.3f,Ox=%.3f,Oy=%.3f](%.3f,%.3f)(%.3f,%.3f)\n", ticks, xdivision, ydivision, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.min, img.yaxis.lim.min, img.xaxis.lim.max, img.yaxis.lim.max);
                       break;
              }
             break;
    }
  }
 else
   if (img.xaxis.available)
    {
     switch (img.xaxis.lim.precision)
      {
        case 0:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.0f,Ox=%.0f](%.0f,0)(%.0f,0)\n", ticks, xdivision, img.xaxis.lim.min, img.xaxis.lim.min, img.xaxis.lim.max);
       default:break;
        case 1:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.1f,Ox=%.1f](%.1f,0)(%.1f,0)\n", ticks, xdivision, img.xaxis.lim.min, img.xaxis.lim.min, img.xaxis.lim.max);
               break;
        case 2:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.2f,Ox=%.2f](%.2f,0)(%.2f,0)\n", ticks, xdivision, img.xaxis.lim.min, img.xaxis.lim.min, img.xaxis.lim.max);
               break;
        case 3:fprintf(myfile, "\\psaxes[ticks=%s,Dx=%.3f,Ox=%.3f](%.3f,0)(%.3f,0)\n", ticks, xdivision, img.xaxis.lim.min, img.xaxis.lim.min, img.xaxis.lim.max);
               break;
      }
    }
}

double position_in_multi_plot_pstricks(int groupindex, int index, int groupcount, int elementcount, double* barwidth)
{
 double emptywidth;
 emptywidth = ((sqrt(5) - 1) / ((elementcount + 2) * sqrt(5) + (elementcount - 2)));
 *barwidth = ((sqrt(5) + 1) / ((elementcount + 2) * sqrt(5) + (elementcount - 2)));
 return groupindex + emptywidth + index * (*barwidth);
}

void export_image_as_pstricks(char* fname)
{
 char st[100];
 FILE* myfile;
 int i, j, posy, xgroupcount, xgroupwidth, ygroupcount, ygroupwidth, diff, groupindex, index, elementcount;
 double posx, posy2, barwidth, barheight, epsilonx, epsilony;
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
 write_image_header_pstricks(myfile, img.xaxis.lim.min - 1, img.yaxis.lim.min - 1, img.xaxis.lim.max + 1, img.yaxis.lim.max + 1);
 epsilonx = (img.xaxis.lim.max - img.xaxis.lim.min) / 200;
 epsilony = (img.yaxis.lim.max - img.yaxis.lim.min) / 200;
 set_color_pstricks(myfile, BLACK);
 save_axes_pstricks(myfile);
 set_color_pstricks(myfile, BLACK);
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
                                  set_color_pstricks(myfile, img.objects[i].fnt.fontcolor);
                                  set_line_width_pstricks(myfile, img.objects[i].fnt.fontsize);
                                 }
                                l = img.objects[i].obje.li;
                                if (img.objects[i].fnt.dashedtype == SOLID_LINE)
                                  write_line_pstricks(myfile, l.upleft.x, l.upleft.y, l.downright.x, l.downright.y);
                                else
                                  write_dashed_line_pstricks(myfile, l.upleft.x, l.upleft.y, l.downright.x, l.downright.y, img.objects[i].fnt.dashedtype);
                                break;
         case OBJECT_CIRCLE    :ci = img.objects[i].obje.ci;
                                write_circle_pstricks(myfile, ci.center.x, ci.center.y, ci.radius);
                                break;
         case OBJECT_POINT     :
         case OBJECT_STRING    :if (i == 0 || img.objects[i].fnt.fontcolor != img.objects[i - 1].fnt.fontcolor || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                                 {
                                  set_color_pstricks(myfile, img.objects[i].fnt.fontcolor);
                                  set_line_width_pstricks(myfile, img.objects[i].fnt.fontsize);
                                 }
                                s = img.objects[i].obje.st;
                                write_string_pstricks(myfile, s.p.x, s.p.y, s.st, ALIGN_CENTER);
                                break;
         case OBJECT_NAMEDPOINT:if (i == 0 || img.objects[i].fnt.fontcolor != img.objects[i - 1].fnt.fontcolor || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                                 {
                                  set_color_pstricks(myfile, img.objects[i].fnt.fontcolor);
                                  set_line_width_pstricks(myfile, img.objects[i].fnt.fontsize);
                                 }
                                s = img.objects[i].obje.st;
                                write_line_pstricks(myfile, s.p.x - epsilonx, s.p.y - epsilony, s.p.x + epsilonx, s.p.y + epsilony);
                                write_line_pstricks(myfile, s.p.x - epsilonx, s.p.y + epsilony, s.p.x + epsilonx, s.p.y - epsilony);
                                write_string_pstricks(myfile, s.p.x, s.p.y - 4 * epsilony, s.st, ALIGN_CENTER);
                                break;
         case OBJECT_RECTANGLE :if (i == 0 || img.objects[i].fnt.fontsize != img.objects[i - 1].fnt.fontsize)
                                  set_line_width_pstricks(myfile, img.objects[i].fnt.fontsize);
                                r = img.objects[i].obje.re;
                                posx = position_in_multi_plot_pstricks(groupindex, index, xgroupcount, elementcount, &barwidth);
                                write_rectangle_pstricks(myfile, posx, r.upleft.y, posx + barwidth, r.downright.y, index % MAX_FILL_TYPE, BLACK);
                                break;
         case OBJECT_MVBOX     :switch (img.groupcoloring)
                                 {
                                  case        ALL_SAME:set_color_pstricks(myfile, BLACK);
                                                       break;
                                  case      GROUP_SAME:set_color_pstricks(myfile, img.objects[i].groupindex % COLORCOUNT);
                                                       break;
                                  case INDIVIDUAL_SAME:set_color_pstricks(myfile, img.objects[i].index % COLORCOUNT);
                                                       break;
                                 }
                                posx = position_in_multi_plot_pstricks(groupindex, index, xgroupcount, elementcount, &barwidth) + barwidth / 2;
                                write_line_pstricks(myfile, posx, img.objects[i].obje.mv.mean - 4 * epsilony, posx, img.objects[i].obje.mv.mean - img.objects[i].obje.mv.variance);
                                write_line_pstricks(myfile, posx, img.objects[i].obje.mv.mean + 4 * epsilony, posx, img.objects[i].obje.mv.mean + img.objects[i].obje.mv.variance);
                                write_line_pstricks(myfile, posx - 5 * epsilonx, img.objects[i].obje.mv.mean - img.objects[i].obje.mv.variance, posx + 5 * epsilonx, img.objects[i].obje.mv.mean - img.objects[i].obje.mv.variance);
                                write_line_pstricks(myfile, posx - 5 * epsilonx, img.objects[i].obje.mv.mean + img.objects[i].obje.mv.variance, posx + 5 * epsilonx, img.objects[i].obje.mv.mean + img.objects[i].obje.mv.variance);
                                write_circle_pstricks(myfile, posx, img.objects[i].obje.mv.mean, 4);
                                break;
         case OBJECT_BOXPLOT   :posx = position_in_multi_plot_pstricks(groupindex, index, xgroupcount, elementcount, &barwidth);
                                diff = barwidth / 8;
                                posx = posx + diff;
                                barwidth -= 2 * diff;
                                write_rectangle_pstricks(myfile, posx, img.objects[i].obje.box.p75, posx + barwidth, img.objects[i].obje.box.p25, FILL_COLOR, index + 1);
                                write_line_pstricks(myfile, posx, img.objects[i].obje.box.median, posx + barwidth, img.objects[i].obje.box.median);
                                if (img.objects[i].obje.box.swhisker < img.objects[i].obje.box.p25)
                                 {
                                  write_dashed_line_pstricks(myfile, posx + barwidth / 2, img.objects[i].obje.box.p25, posx + barwidth / 2, img.objects[i].obje.box.swhisker, 4);
                                  write_line_pstricks(myfile, posx, img.objects[i].obje.box.swhisker, posx + barwidth, img.objects[i].obje.box.swhisker);
                                 }
                                if (img.objects[i].obje.box.lwhisker > img.objects[i].obje.box.p75)
                                 {
                                  write_dashed_line_pstricks(myfile, posx + barwidth / 2, img.objects[i].obje.box.p75, posx + barwidth / 2, img.objects[i].obje.box.lwhisker, 4);
                                  write_line_pstricks(myfile, posx, img.objects[i].obje.box.lwhisker, posx + barwidth, img.objects[i].obje.box.lwhisker);
                                 }
                                break;
     case OBJECT_ERRORHISTOGRAM:xgroupwidth = 1;
                                ygroupwidth = ygroupcount * 10;
                                barheight = 1.0;
                                barwidth = xgroupwidth / 30.0;
                                posx = index * xgroupwidth;
                                posy = groupindex * 10;
                                posy2 = (groupindex + 1) * 10;
                                write_line_pstricks(myfile, posx, posy, posx, posy2);
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
                                     write_string_pstricks(myfile, 0, posy + barheight * j, st, ALIGN_RIGHT);
                                   }
                                  groupdone[groupindex] = YES;
                                 }
                                for (j = 0; j < 10; j++)
                                  if (img.objects[i].obje.errorhist.counts[j] > 0)
                                    write_rectangle_pstricks(myfile, posx, posy + barheight * j + 1, posx + barwidth * img.objects[i].obje.errorhist.counts[j], posy + barheight * (j + 1) - 1, FILL_COLOR, index % COLORCOUNT);
                                break;
         default               :printf(m1233, img.objects[i].type);
                                break;
    }
  }
 safe_free(groupdone);
 write_image_footer_pstricks(myfile);
 fclose(myfile);
}

void rangetest_plot_pstricks(double* ranks, int algorithmcount, char** algorithms, double critical_difference)
{
 FILE* outfile;
 char st[3];
 int i, j, oldj, leftcount = 0, rightcount = 0, maxcount, linecount;
 double algheight, axisheight, algwidth;
 outfile = fopen("image.txt", "w");
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
 algheight = 0.5;
 algwidth = 1.0;
 axisheight = algheight * (maxcount + 1);
 write_image_header_pstricks(outfile, -2, -1, algorithmcount + algwidth, axisheight + algheight);
 write_line_pstricks(outfile, 0, axisheight, algorithmcount - 1, axisheight);
 for (i = 1; i <= algorithmcount; i++)
  {
   sprintf(st, "%d", i);
   write_string_pstricks(outfile, (i - 1), axisheight + .2, st, ALIGN_CENTER);
   write_line_pstricks(outfile, i - 1, axisheight - 0.1, i - 1, axisheight + 0.1);
  }
 for (i = 0; i < algorithmcount; i++)
  {
   if (ranks[i] < (1.0 + algorithmcount) / 2)
    {
     leftcount++;
     write_line_pstricks(outfile, (ranks[i] - 1) * algwidth, axisheight, (ranks[i] - 1) * algwidth, algheight * leftcount);
     write_line_pstricks(outfile, 0, algheight * leftcount, (ranks[i] - 1) * algwidth, algheight * leftcount);
     write_string_pstricks(outfile, -1.9, algheight * leftcount, algorithms[i], ALIGN_LEFT);
    }
   else
    {
     rightcount++;
     write_line_pstricks(outfile, (ranks[i] - 1) * algwidth, axisheight, (ranks[i] - 1) * algwidth, algheight * rightcount);
     write_line_pstricks(outfile, (ranks[i] - 1) * algwidth, algheight * rightcount, algorithmcount - .8, algheight * rightcount);
     write_string_pstricks(outfile, algorithmcount - .7, algheight * rightcount, algorithms[i], ALIGN_LEFT);
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
     set_line_width_pstricks(outfile, 2.0);
     write_line_pstricks(outfile, (ranks[i] - 1) * algwidth - .1, axisheight - linecount * 0.15, (ranks[j] - 1) * algwidth + 0.1, axisheight - linecount * .15);
     set_line_width_pstricks(outfile, 1.0);
     linecount++;
    }
   oldj = j;
  }
 write_image_footer_pstricks(outfile);
 fclose(outfile);
}

void rank_plot_pstricks(char** names, int filecount)
{
 FILE* myfile, *outfile;
 int obs, i, count = file_length(names[0]), barwidth = count * filecount / 10;
 double barheight = 0.5, gap = 0.5;
 outfile = fopen("image.txt", "w");
 if (!outfile)
   return;
 write_image_header_pstricks(outfile, 0, 0, barwidth, (barheight + gap) * filecount);
 for (i = 0; i < filecount; i++)
   write_line_pstricks(outfile, 0, (barheight + gap) * i + gap, barwidth, (barheight + gap) * i + gap);
 for (i = 0; i < filecount; i++)
  {
   myfile = fopen(names[i], "r");
   if (!myfile)
    {
     printf(m1274, names[i]);
     return;
    }
   while (fscanf(myfile, "%d\n", &obs) != EOF)
     write_rectangle_pstricks(outfile, 0.1 * obs, (barheight + gap) * i + gap, 0.1 * (obs + 1), (barheight + gap) * i + gap + barheight, FILL_COLOR, i % COLORCOUNT);
   fclose(myfile);
  }
 write_image_footer_pstricks(outfile);
 fclose(outfile);
}
