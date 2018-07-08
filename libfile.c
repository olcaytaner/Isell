#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libfile.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "messages.h"

void concat_file(char* dest,char* src)
{
/*!Last Changed 27.04.2002*/
 FILE* myfile1;
 FILE* myfile2;
 char st[500];
 if ((myfile1=fopen(dest,"a"))==NULL)
  {
   printf(m1274, dest);
   return;
  }
 if ((myfile2=fopen(src,"r"))==NULL)
  {
   printf(m1274, src);
   fclose(myfile1);
   return;
  }
 while (fgets(st,500,myfile2))
   fputs(st,myfile1);
 fclose(myfile1);
 fclose(myfile2);
}

void dividefile(char* fname, double ratio, int howmany, char* directory)
{
 int *tmp, size, i, j;
 char filename[100];
 char myline[READLINELENGTH];
 FILE *infile,*outfile1,*outfile2;
 size = file_length(fname);
 for (i=0;i<howmany;i++)
  {
   tmp = random_array(size);
   sprintf(filename,"%s/run%d.tra",directory,i+1);
   outfile1 = fopen(filename,"w");
   if (!outfile1)
     return;
   sprintf(filename,"%s/run%d.tes",directory,i+1);
   outfile2 = fopen(filename,"w");
   if (!outfile2)
     return;
   infile = fopen(fname,"r");
   if (!infile)
     return;
   for (j=0;j<size;j++)
    {
     fgets(myline,READLINELENGTH,infile);
     if (tmp[j]<(ratio*size))
       fprintf(outfile1,"%s",myline);
     else
       fprintf(outfile2,"%s",myline);
    }
   fclose(infile);
   fclose(outfile1);
   fclose(outfile2);
   safe_free(tmp);
  }
}

void editfile(char* filename)
{
/*Last changed 18.03.2002*/
 FILE* outputfile;
 char st[500];
 outputfile=fopen(filename,"w");
 if (!outputfile)
  {
   printf(m1068);
   return;
  }
 while (fgets(st,500,stdin))
   fputs(st,outputfile);
 fclose(outputfile);
}

int file_length(char *file_name)
{
 int i = 0;
 FILE *myfile;
 char myline[READLINELENGTH];
 if ((myfile = fopen(file_name, "r")) == NULL)
   return 0;
 while (fgets(myline, READLINELENGTH, myfile))
  {
   i++;
  }
 fclose(myfile); 
 return i;
}

char *file_only_name(char *file_name)
{
 /*!Last Changed 09.01.2007 added linux file style*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 06.02.2001*/
 char *st;
 int i,j,end,start,len = strlen(file_name);
 for (i = 0; i < len; i++)
   if (file_name[i] == '.')
     break;
 end = i;
 for (j = i - 1; j >= 0; j--)
   if (file_name[j] == '\\' || file_name[j] == '/')
     break;
 start = j;
 st = safemalloc((end - start) * sizeof(char), "file_only_name", 11);
 for (j = start + 1; j < end; j++)
   st[j - start - 1] = file_name[j];
 st[end - start - 1] = '\0';
 return st;
}

int fileexists(char* filename)
{
/*!Last Changed 18.03.2002*/
 FILE* file;
 if ((file=fopen(filename,"r")) != NULL)
  {
   fclose(file);
   return 1;
  }
 else
   return 0;
}

int number_of_floats_in_file(char* fname)
{
 /*!Last Changed 14.01.2009*/
 char myline[READLINELENGTH], *p;
 int count = 0;
 FILE* myfile;
 myfile = fopen(fname, "r");
 if (!myfile)
   return -1;
 fgets(myline, READLINELENGTH, myfile);
 p = strtok(myline, " ;\n");
 while (p)
  {
   count++;
   p = strtok(NULL, " ;\n");
  }
 fclose(myfile);
 return count;
}

FILE* find_string_in_file(FILE *infile, char *substring)
{
/*!Last Changed 24.03.2006 Aydin*/ 
 char temp[READLINELENGTH];
 if (!infile)
   return infile;
 while (infile)
  {
   fgets(temp, READLINELENGTH, infile);
   if (strstr(temp, substring))
    return infile;
  }
 return(infile);
}

int modoffile(char* fname, int* howmany)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 FILE* myfile;
 Fileelement* l;
 double tmp;
 int count = 0, i, tmpi, max, maxindex;
 l = (Fileelement*)safemalloc(file_length(fname)*sizeof(Fileelement), "modoffile", 5);
 myfile = fopen(fname, "r");
 if (!myfile)
   return -1;
 while (fscanf(myfile,"%lf",&tmp)!=EOF)
  {
   tmpi = (int)tmp;
   for (i = 0; i < count; i++)
     if (l[i].e==tmpi)
       break;
   if (i<count)
     l[i].count++;
   else
    {
     l[count].e = tmpi;
     l[count].count = 1;
     count++;
    }
  }
 max = -1;
 maxindex = -1;
 for (i = 0; i < count; i++)
   if (l[i].count > max)
    {
     max = l[i].count;
     maxindex = i;
    }
 tmpi = l[maxindex].e;
 *howmany = max;
 safe_free(l);
 fclose(myfile);
 return tmpi;
}

int net_extension(char *file_name)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 06.02.2001*/
 int i, j, len = strlen(file_name);
 char *st;
 int result;
 j= len - 1;
 while ((j>=0)&&(file_name[j]!='.'))
   j--;
 if (j<0)
   return -1;
 st = safemalloc((len - j)*sizeof(char), "net_extension", 9);
 st[len - j - 1]='\0';
 for (i=j+1;i<len;i++)
   st[i-j-1]=file_name[i];
 if (strcmp(st,"net")==0)
   result = 0;
 else
   if (strcmp(st,"xml")==0)
     result = 1;
   else
     if (strcmp(st,"hugin")==0)
       result = 2;
     else
       if (strcmp(st,"bif")==0)
         result = 3;
       else
         result = -1;
 safe_free(st);
 return result;
}

char** read_file_into_array(char* filename, int* linecount)
{
 /*!Last Changed 11.10.2006*/
 FILE *myfile;
 char myline[READLINELENGTH], **lines = NULL;
 int i;
 *linecount = 0;
 if ((myfile = fopen(filename, "r")) == NULL)
  {
   printf(m1274, filename);
   return NULL;
  }
 while (fgets(myline, READLINELENGTH, myfile))
   (*linecount)++;
 fclose(myfile);
 lines = (char **)safemalloc((*linecount) * sizeof(char *), "read_file_into_array", 13);
 myfile = fopen(filename, "r");
 i = 0;
 while (fgets(myline, READLINELENGTH, myfile))
  {
   lines[i] = strcopy(lines[i], myline);
   i++;
  }
 fclose(myfile);
 return lines;
}

double file_sum(char *fname)
{
 /*Last Changed 04.02.2005*/
 FILE *myfile;
 double sum = 0, temp;
 if ((myfile = fopen(fname, "r")) == NULL)
  {
   printf(m1274, fname);
   return 0;
  }
 while (fscanf(myfile, "%lf", &temp) != EOF)
   sum += temp;
 fclose(myfile);
 return sum;
}
