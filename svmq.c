#include "libmemory.h"
#include "libmisc.h"
#include "parameter.h"
#include "svm.h"
#include "svmq.h"

Svcqptr empty_svcq(Svm_problem prob, Svm_parameter param, signed char* y)
{
	/*Last Changed 27.04.2005*/
	Svcqptr result;
	result = safemalloc(sizeof(Svcq), "empty_svcq", 2);
	result->k = empty_kernel(prob.l, prob.x, param.kernel_type, param.degree, param.gamma, param.coef0);
	result->y = safemalloc(prob.l * sizeof(signed char), "empty_svcq", 4);
	memcpy(result->y, y, prob.l * sizeof(signed char));
	result->cache = empty_cache(prob.l, (int)(param.cache_size*(1<<20)));
	return result;
}

void free_svcq(Svcqptr s)
{
	/*Last Changed 27.04.2005*/
	safe_free(s->y);
	free_kernel(s->k);
	free_cache(s->cache);
 safe_free(s);
}

double* svcq_get_Q(Svcqptr s, int i, int len)
{
	/*Last Changed 27.04.2005*/
 ON_OFF normalizekernel = get_parameter_o("normalizekernel");
	double *data;
	int start, j;
	start = get_data(s->cache, i, &data, len);
	if (start < len)
	 {
			for (j = start; j < len; j++)
     if (normalizekernel == ON)
				   data[j] = (double)(s->y[i] * s->y[j] * kernel_function_normalized(s->k, i, j));
     else
       data[j] = (double)(s->y[i] * s->y[j] * kernel_function(s->k, i, j));
	 }
	return data;
}

void svcq_swap_index(Svcqptr s, int i, int j)
{
	/*Last Changed 27.04.2005*/
	signed char tmp;
	cache_swap_index(s->cache, i, j);
	kernel_swap_index(s->k, i, j);
	tmp = s->y[i];
	s->y[i] = s->y[j];
	s->y[j] = tmp;
}

Oneclassqptr empty_oneclassq(Svm_problem prob, Svm_parameter param)
{
	/*Last Changed 27.04.2005*/
	Oneclassqptr result;
	result = safemalloc(sizeof(Oneclassq), "empty_oneclassq", 2);
	result->k = empty_kernel(prob.l, prob.x, param.kernel_type, param.degree, param.gamma, param.coef0);
	result->cache = empty_cache(prob.l, (int)(param.cache_size*(1<<20)));
	return result;
}

void free_oneclassq(Oneclassqptr o)
{
	/*Last Changed 27.04.2005*/
	free_kernel(o->k);
	free_cache(o->cache);
	safe_free(o);
}

double* oneclassq_get_Q(Oneclassqptr o, int i, int len)
{
	/*Last Changed 27.04.2005*/
 ON_OFF normalizekernel = get_parameter_o("normalizekernel");
	double *data;
	int start, j;
	start = get_data(o->cache, i, &data, len);
	if (start < len)
	 {
			for (j = start; j < len; j++)
     if (normalizekernel == ON)
				   data[j] = (double)(kernel_function_normalized(o->k, i, j));
     else
       data[j] = (double)(kernel_function(o->k, i, j));
	 }
	return data;
}

void oneclassq_swap_index(Oneclassqptr o, int i, int j)
{
	/*Last Changed 27.04.2005*/
	cache_swap_index(o->cache, i, j);
	kernel_swap_index(o->k, i, j);
}

Svrqptr empty_svrq(Svm_problem prob, Svm_parameter param)
{
	/*Last Changed 27.04.2005*/
	int k;
	Svrqptr result;
	result = safemalloc(sizeof(Svrq), "empty_svrq", 3);
	result->k = empty_kernel(prob.l, prob.x, param.kernel_type, param.degree, param.gamma, param.coef0);
	result->l = prob.l;
	result->cache = empty_cache(prob.l, (int)(param.cache_size*(1<<20)));
	result->sign = safemalloc(2 * prob.l * sizeof(signed char), "empty_svrq", 7);
	result->index = safemalloc(2 * prob.l * sizeof(int), "empty_svrq", 8);
	for(k = 0; k < prob.l; k++)
	 {
			result->sign[k] = 1;
			result->sign[k + prob.l] = -1;
			result->index[k] = k;
			result->index[k + prob.l] = k;
		}
	result->buffer[0] = safemalloc(2 * prob.l * sizeof(double), "empty_svrq", 16);
	result->buffer[1] = safemalloc(2 * prob.l * sizeof(double), "empty_svrq", 17);
	result->next_buffer = 0;
	return result;
}

void free_svrq(Svrqptr s)
{
	/*Last Changed 27.04.2005*/
	free_kernel(s->k);
	free_cache(s->cache);
	safe_free(s->sign);
	safe_free(s->index);
	safe_free(s->buffer[0]);
	safe_free(s->buffer[1]);
	safe_free(s);
}

double* svrq_get_Q(Svrqptr s, int i, int len)
{
	/*Last Changed 27.04.2005*/
 ON_OFF normalizekernel = get_parameter_o("normalizekernel");
	double *data, *buf;
	int real_i, j;
	signed char si;
 real_i = s->index[i];
	if(get_data(s->cache, real_i, &data, s->l) < s->l)
		{
			for (j = 0; j < s->l; j++)
     if (normalizekernel == ON)    
				   data[j] = (double)(kernel_function_normalized(s->k, real_i, j));
     else
       data[j] = (double)(kernel_function(s->k, real_i, j));
		}
	buf = s->buffer[s->next_buffer];
	s->next_buffer = 1 - s->next_buffer;
 si = s->sign[i];
	for (j = 0; j < len; j++)
			 buf[j] = si * s->sign[j] * data[s->index[j]];
	return buf;
}

void svrq_swap_index(Svrqptr s, int i, int j)
{
	/*Last Changed 27.04.2005*/
	signed char tmp;
	tmp = s->sign[i];
	s->sign[i] = s->sign[j];
	s->sign[j] = tmp;
	swap_int(&(s->index[i]), &(s->index[j]));
}
