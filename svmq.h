#ifndef __svmq_H
#define __svmq_H

#include "svmcache.h"
#include "svmkernel.h"

struct svcq{
	           Kernelptr k;
	           signed char* y;
												Cacheptr cache;
};

typedef struct svcq Svcq;
typedef Svcq* Svcqptr;

struct oneclassq{
	           Kernelptr k;
												Cacheptr cache;
};

typedef struct oneclassq Oneclassq;
typedef Oneclassq* Oneclassqptr;

struct svrq{
	           Kernelptr k;
	           int l;
	           Cacheptr cache;
	           signed char* sign;
	           int *index;
	           int next_buffer;
	           double* buffer[2];
};

typedef struct svrq Svrq;
typedef Svrq* Svrqptr;

Oneclassqptr empty_oneclassq(Svm_problem prob, Svm_parameter param);
Svcqptr      empty_svcq(Svm_problem prob, Svm_parameter param, signed char* y);
Svrqptr      empty_svrq(Svm_problem prob, Svm_parameter param);
void         free_oneclassq(Oneclassqptr o);
void         free_svcq(Svcqptr s);
void         free_svrq(Svrqptr s);
double*      oneclassq_get_Q(Oneclassqptr o, int i, int len);
void         oneclassq_swap_index(Oneclassqptr o, int i, int j);
double*      svcq_get_Q(Svcqptr s, int i, int len);
double*      svrq_get_Q(Svrqptr s, int i, int len);
void         svcq_swap_index(Svcqptr s, int i, int j);
void         svrq_swap_index(Svrqptr s, int i, int j);

#endif
