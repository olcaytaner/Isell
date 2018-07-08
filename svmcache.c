#include "libmemory.h"
#include "libmisc.h"
#include "svmcache.h"

/**
 * Constructor for SVM cache structure. Allocates memory for the cache structure, initializes least recently used. Cache stores the data as a circular link list.
 * @param[in] l Number of instances
 * @param[in] size Size of the cache
 * @return Empty cache
 */
Cacheptr empty_cache(int l, int size)
{
	/*Last Changed 26.04.2005*/
	Cacheptr result;
	result = safemalloc(sizeof(Cache), "empty_cache", 2);
	result->head = (Cachenodeptr) safecalloc(l, sizeof(Cachenode), "empty_cache", 3);	
	result->size = size / sizeof(double);
	result->size -= l * sizeof(Cachenode) / sizeof(double);
	result->lru_head.next = &(result->lru_head);
	result->lru_head.prev = &(result->lru_head);
	return result;
}

/**
 * Destructor for the SVM cache structure. Deallocates memory allocated (i) for the cache structures (ii) elements in the cache (which was stored as a circular link list).
 * @param[in] c Cache
 */
void free_cache(Cacheptr c)
{
	/*Last Changed 26.04.2005*/
	Cachenodeptr h;
	for (h = c->lru_head.next; h != &(c->lru_head); h = h->next)
		 safe_free(h->data);
	safe_free(c->head);
	safe_free(c);
}

/**
 * Remove a cache node from the circular link list. Changes the pointers. prev -> h -> next => prev -> next.
 * @param[in] h Cache node.
 */
void lru_delete(Cachenodeptr h)
{
	/*Last Changed 26.04.2005*/
	h->prev->next = h->next;
	h->next->prev = h->prev;
}

/**
 * Inserts new data to the cache. Updates pointers. prev -> head =>  prev -> h -> head
 * @param[in] c Cache structure.
 * @param[i] h New data (as a cache node)
 */
void lru_insert(Cacheptr c, Cachenodeptr h)
{
	/*Last Changed 26.04.2005*/
	h->next = &(c->lru_head);
	h->prev = c->lru_head.prev;
	h->prev->next = h;
	h->next->prev = h;
}

/**
 * Gets data from the SVM cache.
 * @param[in] c SVM cache
 * @param[in] index Index of the cache node
 * @param[out] data The data returned
 * @param[in] len Length of the data to be returned
 * @return Length of the data returned. 
 */
int get_data(Cacheptr c, int index, double **data, int len)
{
	/*Last Changed 26.04.2005*/
	int more;
	Cachenodeptr h, old;
 h = &(c->head[index]);
	if (h->len) 
		 lru_delete(h);
	more = len - h->len;
	if (more > 0)
	 {
		 while (c->size < more)
			 {
		  	old = c->lru_head.next;
			  lru_delete(old);
			  safe_free(old->data);
			  c->size += old->len;
			  old->data = NULL;
			  old->len = 0;
			 }
		 h->data = (double *)alloc(h->data, sizeof(double) * len, len);
		 c->size -= more;
		 swap_int(&(h->len), &len);
	 }
	lru_insert(c, h);
	*data = h->data;
	return len;
}

/**
 * Swaps i'th cache item with the j'th cache item.
 * @param[in] c Svm Cache
 * @param[in] i first index
 * @param[in] j second index
 */
void cache_swap_index(Cacheptr c, int i, int j)
{
	/*Last Changed 26.04.2005*/
	Cachenodeptr h;
	double* tmp;
	if (i == j) 
		 return;
	if (c->head[i].len) 
		 lru_delete(&(c->head[i]));
	if (c->head[j].len) 
		 lru_delete(&(c->head[j]));
	tmp = c->head[i].data;
 c->head[i].data = c->head[j].data;
	c->head[j].data = tmp;
	swap_int(&(c->head[i].len), &(c->head[j].len));
	if (c->head[i].len) 
		 lru_insert(c, &(c->head[i]));
	if (c->head[j].len) 
		 lru_insert(c, &(c->head[j]));
	if (i > j) 
		 swap_int(&i, &j);
	for (h = c->lru_head.next; h != &(c->lru_head); h = h->next)
	 {
 		if (h->len > i)
			 {
		  	if (h->len > j)
				   swap_double(&(h->data[i]), &(h->data[j]));
			  else
					 {
   				lru_delete(h);
		   		safe_free(h->data);
				   c->size += h->len;
				   h->data = NULL;
				   h->len = 0;
					 }
			 }
	 }
}
