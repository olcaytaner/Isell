#ifndef __libmemory_H
#define __libmemory_H

#include <stdlib.h>
#include "options.h"

#ifdef HEAP_DEBUG

/*! Structure used in the memory (heap) debug mode. Each allocated memory corresponds to an instance of this structure.*/
struct memorydata{
                  /*! Address of the memory block allocated*/
                  void* p;
                  /*! Size of the allocated memory block*/
                  size_t size;
                  /*! Name of the function that has allocated this memory block*/
                  char functionname[50];
                  /*! Line number of the function that has allocated this memory block*/
                  int line;
                  /*! Is the allocated memory block freed? Has two possible values: YES for dellocated, NO for not deallocated*/
                  int isfreed;
                 };

typedef struct memorydata Memorydata;
typedef Memorydata* Memorydataptr; 

#endif

#ifdef HEAP_DEBUG
void heap_debug_print_heap();
void heap_debug_insert_data(void* p, size_t size, char* functionname, int line);
void heap_debug_delete_data(void* p, int isreallocated);
#endif

void*    alloc(void *p, size_t size, int count);
void     free_2d(void **p,int size);
void     free_3d(void ***p,int size1,int size2);
void     safe_free(void* p);
void*    safecalloc(size_t elementcount, size_t elementsize, char* functionname, int line);
void**   safecalloc_2d(size_t elementsize1, size_t elementsize2, int elementcount1, int elementcount2, char* functionname, int line);
void***  safecalloc_3d(size_t elementsize1, size_t elementsize2, size_t elementsize3, int elementcount1, int elementcount2, int elementcount3, char* functionname, int line);
void*    safemalloc(size_t size, char* functionname, int line);
void**   safemalloc_2d(size_t elementsize1, size_t elementsize2, int elementcount1, int elementcount2, char* functionname, int line);
void***  safemalloc_3d(size_t elementsize1, size_t elementsize2, size_t elementsize3, int elementcount1, int elementcount2, int elementcount3, char* functionname, int line);
void*    saferealloc(void* p, size_t size, char* functionname, int line);

#endif
