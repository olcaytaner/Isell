#ifndef __image_H
#define __image_H
#include "typedef.h"

#define epsaxis 500
#define PARTITIONS 500
#define TICKWIDTH 6

enum align_type{
      ALIGN_LEFT = 0,
      ALIGN_RIGHT,
      ALIGN_CENTER,
      ALIGN_ROTATE_LEFT,
      ALIGN_ROTATE_RIGHT,
      ALIGN_ROTATE_CENTER};

typedef enum align_type ALIGN_TYPE;

#define MAX_FILL_TYPE 7

enum fill_type{ 
      FILL_NONE = 0,      
      FILL_FLOOD,
      FILL_GRAY,
      FILL_SQUARE,
      FILL_COLOR,      
      FILL_VERTICAL,
      FILL_HORIZONTAL};

typedef enum fill_type FILL_TYPE;

enum axis_type{ 
      X_AXIS = 0,
      Y_AXIS};

typedef enum axis_type AXIS_TYPE;

#define SOLID_LINE 0
#define MAX_LINE_TYPE 5

enum part_type {
               DATA = 0, 
               LEGEND, 
               AXIS, 
               LABEL};

typedef enum part_type PART_TYPE;

void      add_legend(char ch, char* st);
Objectptr add_object(OBJECT_TYPE type, Point p1, Point p2, Point p3, char* st);
void      check_axis_ranges();
void      check_x_range(Point p);
void      check_y_range(Point p);
void      clear_image();
void      exchange_objects(int i, int j);
void      export_image();
void      initialize_font(Font* f, int color, FONT_TYPE name, double size);
void      initialize_image();
void      load_image(char* fname);
int       object_count(int type, int groupindex);
double    point_to_point_distance(Point p1, Point p2);
int       position_in_multi_plot(int groupindex, int index, int groupcount, int elementcount, double* barwidth);
int       return_pos(double value,int start,int end,double min,double max);
void      save_image(char* fname);
void      set_limits_automatically(Limits* ax);

#endif
