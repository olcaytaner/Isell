#ifndef __svmcache_H
#define __svmcache_H

struct cachenode{
		struct cachenode *prev;
		struct cachenode *next;	
		double *data;
		int len;		
};

typedef struct cachenode Cachenode;
typedef Cachenode* Cachenodeptr;

struct cache{
	 Cachenodeptr head;
	 Cachenode lru_head;
	 int l;
		int size;
};

typedef struct cache Cache;
typedef Cache* Cacheptr;

void  cache_swap_index(Cacheptr c, int i, int j);
Cacheptr empty_cache(int l, int size);
void  free_cache(Cacheptr c);
int   get_data(Cacheptr c, int index, double **data, int len);
void  lru_delete(Cachenodeptr h);
void  lru_insert(Cacheptr c, Cachenodeptr h);

#endif
