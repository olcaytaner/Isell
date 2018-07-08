#include <stdlib.h>
#include <string.h>
#include "libmemory.h"
#include "libstring.h"

int char_count(char* st, char ch)
{
 /*!Last Changed 10.10.2004*/
 int result = 0, i, len = strlen(st);
 for (i = 0; i < len; i++)
   if (st[i] == ch)
     result++;
 return result;
}

int instr(char* st, char* searchst)
{
 /*!Last Changed 19.01.2005 returns -1 if not found returns position if found*/
 /*!Last Changed 03.02.2004*/
 int i, j, len1, len2;
 if (!st || !searchst)
   return -1;
 len1 = strlen(st);
 len2 = strlen(searchst);
 for (i = 0; i < len1 - len2 + 1; i++)
  {
   for (j = 0; j < len2; j++)
     if (st[i + j] != searchst[j])
       break;
   if (j == len2)
     return i;
  }
 return -1;
}

int listindex(char *element, char *array[], int elementcount)
{
 /*!Last Changed 10.04.2002*/
 int i;
 if (!element)
   return -2;
 for (i = 0; i < elementcount; i++)
   if (strIcmp(element, array[i]) == 0)
     return i;
 return -1;
}

int listindexc(char element, char array[], int elementcount)
{
 /*!Last Changed 03.01.2006*/
 int i;
 for (i = 0; i < elementcount; i++)
   if (element == array[i])
     return i;
 return -1;
}

char* strconcat(char *s, const char *ct)
{
 /*!Last Changed 04.04.2002*/
 s = (char *) alloc (s, strlen(s) + strlen(ct) + 1, strlen(s) + strlen(ct) + 1);
 s = strcat(s, ct);
 return s;
}

char *strcopy(char *s, char *ct)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 s = (char *)safemalloc((strlen(ct)+1)*sizeof(char), "strcopy", 1);
 strcpy(s, ct);
 return s;
}

void stringlist(char* st, char* separator, char* list[], int* count)
{
 /*!Last Changed 29.01.2004*/
 char* p;
 int c = 0;
 p = strtok(st, separator);
 while (p)
  {
   list[c] = strcopy(list[c], p);
   c++;
   p = strtok(NULL, separator);
  }
 *count = c;
}

int strIcmp(const char *s1, const char *s2)
{
/*!Last Changed 12.04.2002*/
 int len1=strlen(s1),len2=strlen(s2),i,min;
 if (len1<len2)
   min=len1;
 else
   min=len2;
 for (i=0;i<min;i++)
  {
   if (upper(s1[i])<upper(s2[i]))
     return -1;
   if (upper(s1[i])>upper(s2[i]))
     return 1;
  }
 if (len1<len2)
   return -1;
 if (len1>len2)
   return 1;
 return 0;
}

char upper(char ch)
{
/*!Last Changed 12.04.2002*/
 if (ch >= 'a' && ch <= 'z')
   return ch + 'A' - 'a';
 return ch;
}
