#ifndef __pstricks_H
#define __pstricks_H
#include "image.h"

void   export_image_as_pstricks(char* fname);
double position_in_multi_plot_pstricks(int groupindex, int index, int groupcount, int elementcount, double* barwidth);
void   rangetest_plot_pstricks(double* ranks, int algorithmcount, char** algorithms, double critical_difference);
void   rank_plot_pstricks(char** names, int filecount);
void   save_axes_pstricks(FILE* myfile);
void   set_color_pstricks(FILE* myfile, int color);
void   set_line_width_pstricks(FILE* myfile, double width);
void   write_circle_pstricks(FILE* myfile, double centerx, double centery, double radius);
void   write_dashed_line_pstricks(FILE* myfile, double startx, double starty, double endx, double endy, int dashedtype);
void   write_image_footer_pstricks(FILE* myfile);
void   write_image_header_pstricks(FILE* myfile, double startx, double starty, double endx, double endy);
void   write_line_pstricks(FILE* myfile, double startx, double starty, double endx, double endy);
void   write_rectangle_pstricks(FILE* myfile, double corner1x, double corner1y, double corner2x, double corner2y, FILL_TYPE filltype, int color);
void   write_string_pstricks(FILE* myfile, double posx, double posy, char* st, ALIGN_TYPE align);

#endif
