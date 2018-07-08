#ifndef __libfile_H
#define __libfile_H

#include<stdio.h>

/*! Structure used in the mod of file operation. Contains element and its number of occurence*/
struct fileelement{
                   /*! The element*/
                   int e;
                   /*! Number of occurence*/
                   int count;                  
                  };
typedef struct fileelement Fileelement;

void     concat_file(char* dest,char* src);
void     dividefile(char* fname, double ratio, int howmany, char* directory);
void     editfile(char* filename);
int      file_length(char *file_name);
char*    file_only_name(char *file_name);
double   file_sum(char *fname);
int      fileexists(char* filename);
FILE*    find_string_in_file(FILE *infile, char *substring);
int      modoffile(char* fname, int* howmany);
int      net_extension(char *file_name);
int      number_of_floats_in_file(char* fname);
char**   read_file_into_array(char* filename, int* linecount);

#endif
