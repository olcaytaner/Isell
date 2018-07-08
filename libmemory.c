#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libmemory.h"
#include "messages.h"
#include "options.h"

#ifdef HEAP_DEBUG
size_t totalallocated = 0;
int heapcount;
#define MAXHEAPCOUNT 100000
Memorydata memoryheap[MAXHEAPCOUNT];
#endif

void *alloc(void *p, size_t size, int count)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 if (count == 1)
   return safemalloc(size, "alloc", 2);
 else
   return saferealloc(p, size, "alloc", 4);
}

void free_2d(void **p,int size)
{
/*!Last Changed 23.02.2001*/
 int i;
 for (i = 0; i < size; i++)
   if (p[i])
     safe_free(p[i]);
 safe_free(p);
}

void free_3d(void ***p,int size1,int size2)
{
/*!Last Changed 23.02.2001*/
 int i,j;
 for (i = 0; i < size1; i++)
   for (j = 0; j < size2; j++)
     safe_free(p[i][j]);
 for (i = 0; i < size1; i++)
   safe_free(p[i]);
 safe_free(p);
}

void safe_free(void* p)
{
 /*!Last Changed 28.11.2004*/
 #ifdef HEAP_DEBUG
   heap_debug_delete_data(p, 0);
 #endif
 free(p);
}

void* safecalloc(size_t elementcount, size_t elementsize, char* functionname, int line)
{
 /*!Last Changed 02.02.2004*/
 void* result;
 result = calloc(elementcount, elementsize);
 if (!result)
  {
   printf(m1305, functionname, line);
   exit(0);
  }
 else
  {
   #ifdef HEAP_DEBUG
     heap_debug_insert_data(result, elementcount * elementsize, functionname, line);
   #endif
   return result;
  }
}

void** safecalloc_2d(size_t elementsize1, size_t elementsize2, int elementcount1, int elementcount2, char* functionname, int line)
{
 /*!Last Changed 06.02.2005*/
 void** result;
 int i;
 result = safemalloc(elementsize1 * elementcount1, functionname, line);
 for (i = 0; i < elementcount1; i++)
   result[i] = safecalloc(elementcount2, elementsize2, functionname, line);
 return result;
}

void*** safecalloc_3d(size_t elementsize1, size_t elementsize2, size_t elementsize3, int elementcount1, int elementcount2, int elementcount3, char* functionname, int line)
{
 /*!Last Changed 06.02.2005*/
 void*** result;
 int i, j;
 result = safemalloc(elementsize1 * elementcount1, functionname, line);
 for (i = 0; i < elementcount1; i++)
   result[i] = safemalloc(elementsize2 * elementcount2, functionname, line);
 for (i = 0; i < elementcount1; i++)
   for (j = 0; j < elementcount2; j++)
     result[i][j] = safecalloc(elementcount3, elementsize3, functionname, line);
 return result;
}

void* safemalloc(size_t size, char* functionname, int line)
{
 /*!Last Changed 02.02.2004*/
 void* result;
 result = malloc(size);
 if (!result)
  {
   printf(m1305, functionname, line);
   exit(0);
  }
 else
  {
   #ifdef HEAP_DEBUG
     heap_debug_insert_data(result, size, functionname, line);
   #endif
   return result;
  }
}

void** safemalloc_2d(size_t elementsize1, size_t elementsize2, int elementcount1, int elementcount2, char* functionname, int line)
{
 /*!Last Changed 06.02.2005*/
 void** result;
 int i;
 result = safemalloc(elementsize1 * elementcount1, functionname, line);
 for (i = 0; i < elementcount1; i++)
   result[i] = safemalloc(elementsize2 * elementcount2, functionname, line);
 return result;
}

void*** safemalloc_3d(size_t elementsize1, size_t elementsize2, size_t elementsize3, int elementcount1, int elementcount2, int elementcount3, char* functionname, int line)
{
 /*!Last Changed 06.02.2005*/
 void*** result;
 int i, j;
 result = safemalloc(elementsize1 * elementcount1, functionname, line);
 for (i = 0; i < elementcount1; i++)
   result[i] = safemalloc(elementsize2 * elementcount2, functionname, line);
 for (i = 0; i < elementcount1; i++)
   for (j = 0; j < elementcount2; j++)
     result[i][j] = safemalloc(elementsize3 * elementcount3, functionname, line);
 return result;
}

void* saferealloc(void* p, size_t size, char* functionname, int line)
{
 /*!Last Changed 28.11.2004*/
 void* result;
 #ifdef HEAP_DEBUG
   heap_debug_delete_data(p, 1);
 #endif
 result = realloc(p, size);
 if (!result)
  {
   printf(m1305, functionname, line);
   exit(0);
  }
 else
  {
   #ifdef HEAP_DEBUG
     heap_debug_insert_data(result, size, functionname, line);
   #endif
   return result;
  }
}

#ifdef HEAP_DEBUG

void heap_debug_print_heap()
{
 /*!Last Changed 18.12.2005*/
 /*!Last Changed 13.12.2005*/
 /*!Last Changed 29.11.2004*/
 int i;
 FILE* outfile;
 outfile = fopen("heap_contents.txt","w");
 if (!outfile)
  {
   printf(m1306);
   return;
  }
 fprintf(outfile, "%d\n", heapcount);
 for (i = 0; i < heapcount; i++)
   if (memoryheap[i].isfreed)
     fprintf(outfile,"Freed;%ld;%ld;%s;%d\n", memoryheap[i].p, memoryheap[i].size, memoryheap[i].functionname, memoryheap[i].line);
   else
     fprintf(outfile,"Not freed;%ld;%ld;%s;%d\n", memoryheap[i].p, memoryheap[i].size, memoryheap[i].functionname, memoryheap[i].line);
 fclose(outfile);
}

void heap_debug_insert_data(void* p, size_t size, char* functionname, int line)
{
 /*!Last Changed 18.12.2005*/
 /*!Last Changed 29.11.2004*/
 totalallocated += size;
 memoryheap[heapcount].p = p;
 strcpy(memoryheap[heapcount].functionname, functionname);
 memoryheap[heapcount].line = line;
 memoryheap[heapcount].size = size;
 memoryheap[heapcount].isfreed = 0;
 heapcount++;
 if (heapcount > MAXHEAPCOUNT)
   exit(0);
}

void heap_debug_delete_data(void* p, int isreallocated)
{
 /*!Last Changed 18.12.2005*/
 /*!Last Changed 29.11.2004*/
 int i;
 for (i = 0; i < heapcount; i++)
   if (memoryheap[i].p == p && !memoryheap[i].isfreed)
    {
     memoryheap[i].isfreed = 1;
     return;
    }
 if (!isreallocated)
   printf(m1307);
}

#endif
