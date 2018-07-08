#include <float.h>
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "svmsolve.h"

double get_C(Solverptr s, int i)
{
 /*Last Changed 28.04.2005*/
	if (s->y[i] > 0)
		 return s->Cp;
	else
		 return s->Cn;
}

void update_alpha_status(Solverptr s, int i)
{
 /*Last Changed 28.04.2005*/
	if (s->alpha[i] >= get_C(s, i))
			s->alpha_status[i] = UPPER_BOUND;
	else 
		 if (s->alpha[i] <= 0)
			  s->alpha_status[i] = LOWER_BOUND;
		 else 
				 s->alpha_status[i] = FREE;
}

int is_upper_bound(Solverptr s, int i) 
{ 
 /*Last Changed 28.04.2005*/
	if (s->alpha_status[i] == UPPER_BOUND)
	  return 1; 
	else
		 return 0;
}

int is_lower_bound(Solverptr s, int i) 
{ 
 /*Last Changed 28.04.2005*/
	if (s->alpha_status[i] == LOWER_BOUND)
	  return 1; 
	else
		 return 0;
}

int is_free(Solverptr s, int i) 
{ 
 /*Last Changed 28.04.2005*/
	if (s->alpha_status[i] == FREE)
	  return 1; 
	else
		 return 0;
}

double* solver_get_Q(Solverptr s, int i, int len)
{
 /*Last Changed 28.04.2005*/
	switch (s->qmatrixtype)
	 {
   case QC  :return svcq_get_Q(s->svcq, i, len);
				         break;
			case QR  :return svrq_get_Q(s->svrq, i, len);
				         break;
			case QONE:return oneclassq_get_Q(s->oneclassq, i, len);
				         break;
			default  :return NULL;
				         break;
	 }
}

void solver_swap_index(Solverptr s, int i, int j)
{
 /*Last Changed 28.04.2005*/
	signed char tmp;
	switch (s->qmatrixtype)
	 {
   case QC  :svcq_swap_index(s->svcq, i, j);
				         break;
			case QR  :svrq_swap_index(s->svrq, i, j);
				         break;
			case QONE:oneclassq_swap_index(s->oneclassq, i, j);
				         break;
	 }
	tmp = s->y[i];
	s->y[i] = s->y[j];
	s->y[j] = tmp;
	swap_double(&(s->G[i]), &(s->G[j]));
	swap_char(&(s->alpha_status[i]), &(s->alpha_status[j]));
	swap_double(&(s->alpha[i]), &(s->alpha[j]));
	swap_double(&(s->b[i]), &(s->b[j]));
	swap_int(&(s->active_set[i]), &(s->active_set[j]));
	swap_double(&(s->G_bar[i]), &(s->G_bar[j]));
}

void reconstruct_gradient(Solverptr s)
{
 /*Last Changed 28.04.2005*/
	int i, j;
	double* Q_i, alpha_i;
	if (s->active_size == s->l) 
		 return;
	for (i = s->active_size; i < s->l; i++)
		 s->G[i] = s->G_bar[i] + s->b[i];	
	for (i = 0; i < s->active_size; i++)
		 if (is_free(s, i))
			 {
			  Q_i = solver_get_Q(s, i, s->l);
			  alpha_i = s->alpha[i];
			  for (j = s->active_size; j < s->l; j++)
				   s->G[j] += alpha_i * Q_i[j];
			 }
}

int select_working_set_general(Solverptr s, int* out_i, int* out_j, int nu)
{
 /*Last Changed 29.04.2005*/
	if (nu)
		 return nu_select_working_set(s, out_i, out_j);
	else
		 return select_working_set(s, out_i, out_j);
}

int select_working_set(Solverptr s, int* out_i, int* out_j)
{
 /*Last Changed 28.04.2005*/
	double Gmax1 = -DBL_MAX, Gmax2 = -DBL_MAX;		
	int i, Gmax1_idx = -1, Gmax2_idx = -1;
	for (i = 0; i < s->active_size; i++)
	 {
	 	if (s->y[i] == +1)	
		  {
		  	if (!is_upper_bound(s, i))	
					 {
				   if (-s->G[i] >= Gmax1)
							 {
					    Gmax1 = -s->G[i];
					    Gmax1_idx = i;
							 }
					 }
			  if (!is_lower_bound(s, i))	
					 {
				   if (s->G[i] >= Gmax2)
							 {
					    Gmax2 = s->G[i];
					    Gmax2_idx = i;
							 }
					 }
			 }
		 else		
			 {
			  if (!is_upper_bound(s, i))	
					 {
				   if (-s->G[i] >= Gmax2)
							 {
					    Gmax2 = -s->G[i];
					    Gmax2_idx = i;
							 }
					 }
			  if (!is_lower_bound(s, i))	
					 {
				   if (s->G[i] >= Gmax1)
							 {
					    Gmax1 = s->G[i];
					    Gmax1_idx = i;
							 }
					 }
			 }
	 }
	if (Gmax1 + Gmax2 < s->eps)
 		return 1;
	*out_i = Gmax1_idx;
	*out_j = Gmax2_idx;
	return 0;
}

void do_shrinking_general(Solverptr s, int nu)
{
 /*Last Changed 28.04.2005*/
	if (nu)
		 nu_do_shrinking(s);
	else
		 do_shrinking(s);
}

void do_shrinking(Solverptr s)
{
 /*Last Changed 28.04.2005*/
	int i, j, k;
	double Gm1, Gm2;
	if (select_working_set(s, &i, &j) != 0) 
		 return;
	Gm1 = -s->y[j] * s->G[j];
	Gm2 = s->y[i] * s->G[i];
	for (k = 0; k < s->active_size; k++)
	 {
 		if (is_lower_bound(s, k))
			 {
		  	if (s->y[k] == +1)
					 {
				   if (-s->G[k] >= Gm1) 
								 continue;
					 }
			  else	
						 if (-s->G[k] >= Gm2) 
								 continue;
			 }
		 else 
				 if (is_upper_bound(s, k))
					 {
			    if (s->y[k] == +1)
							 {
				     if (s->G[k] >= Gm2) 
										 continue;
							 }
			    else	
								 if (s->G[k] >= Gm1) 
										 continue;
					 }
		   else 
						 continue;
		 --s->active_size;
		 solver_swap_index(s, k, s->active_size);
		 --k;	
	 }
	/* unshrink, check all variables again before final iterations*/
	if (s->unshrinked || -(Gm1 + Gm2) > s->eps * 10) 
		 return;
	s->unshrinked = 1;
	reconstruct_gradient(s);
	for (k = s->l - 1; k >= s->active_size; k--)
	 {
	 	if (is_lower_bound(s, k))
			 {
		  	if (s->y[k] == +1)
					 {
				   if (-s->G[k] < Gm1) 
								 continue;
					 }
			  else	
						 if (-s->G[k] < Gm2) 
								 continue;
			 }
		 else 
				 if (is_upper_bound(s, k))
					 {
			    if (s->y[k] == +1)
							 {
				     if (s->G[k] < Gm2) 
										 continue;
							 }
			    else	
								 if (s->G[k] < Gm1) 
										 continue;
					 } 
		   else 
						 continue;
		 solver_swap_index(s, k, s->active_size);
		 s->active_size++;
		 ++k;	
	 }
}

double calculate_rho_general(Solverptr s, Solutioninfoptr si, int nu)
{
 /*Last Changed 29.04.2005*/
	if (nu)
		 return nu_calculate_rho(s, si);
	else
		 return calculate_rho(s);
}

double calculate_rho(Solverptr s)
{
 /*Last Changed 28.04.2005*/
	double r, ub = DBL_MAX, lb = -DBL_MAX, sum_free = 0, yG;
	int i, nr_free = 0;
	for (i = 0; i < s->active_size; i++)
	 {
		 yG = s->y[i] * s->G[i];
		 if (is_lower_bound(s, i))
		  {
		  	if (s->y[i] > 0)
				   ub = fmin(ub, yG);
			  else
				   lb = fmax(lb, yG);
			 }
		 else 
				 if (is_upper_bound(s, i))
					 {
			    if (s->y[i] < 0)
				     ub = fmin(ub, yG);
			    else
				     lb = fmax(lb, yG);
					 }
		   else
					 {
			    ++nr_free;
			    sum_free += yG;
					 }
	 }
	if (nr_free > 0)
		 r = sum_free / nr_free;
	else
		 r = (ub + lb) / 2;
	return r;
}

void solve(Solverptr s, int l, SVM_Q_MATRIX_TYPE qmatrixtype, double* b, signed char *y, double *alpha, double Cp, double Cn, Solutioninfoptr si, Svm_problem prob, Svm_parameter param, int nu)
{
 /*Last Changed 28.04.2005*/
	double *Q_i, *Q_j, alpha_i, C_i, C_j, old_alpha_i, old_alpha_j, delta, diff, sum, delta_alpha_i, delta_alpha_j, v;
	int i, j, iter, counter, ui, uj, k;
	s->l = l;
	s->qmatrixtype = qmatrixtype;
	switch (qmatrixtype)
	 {
	  case QC  :s->svcq = empty_svcq(prob, param, y);
				         break;
			case QR  :s->svrq = empty_svrq(prob, param);
				         break;
			case QONE:s->oneclassq = empty_oneclassq(prob, param);
				         break;
	 }
 s->b = safemalloc(s->l * sizeof(double), "solve", 14);
	memcpy(s->b, b, s->l * sizeof(double));
 s->y = safemalloc(s->l * sizeof(signed char), "solve", 16);
	memcpy(s->y, y, s->l * sizeof(signed char));
 s->alpha = safemalloc(s->l * sizeof(double), "solve", 18);
	memcpy(s->alpha, alpha, s->l * sizeof(double));
	s->Cp = Cp;
	s->Cn = Cn;
	s->eps = param.eps;
	s->unshrinked = 0;
	s->alpha_status = safemalloc(s->l * sizeof(char), "solve", 24);
	for (i = 0; i < s->l; i++)
			update_alpha_status(s, i);
	s->active_set = safemalloc(s->l * sizeof(int), "solve", 27);
	for (i = 0; i < s->l; i++)
			s->active_set[i] = i;
	s->active_size = s->l;
	s->G = safemalloc(s->l * sizeof(double), "solve", 31);
	s->G_bar = safemalloc(s->l * sizeof(double), "solve", 32);
	for (i = 0; i < s->l; i++)
	 {
			s->G[i] = b[i];
			s->G_bar[i] = 0;
		}
	for (i = 0; i < s->l; i++)
			if (!is_lower_bound(s, i))
			 {
				 Q_i = solver_get_Q(s, i, s->l);
				 alpha_i = s->alpha[i];
				 for (j = 0; j < s->l; j++)
					  s->G[j] += alpha_i * Q_i[j];
				 if (is_upper_bound(s, i))
					  for (j = 0; j < s->l; j++)
						   s->G_bar[j] += get_C(s, i) * Q_i[j];
			 }
	iter = 0;
	counter = imin(s->l, 1000) + 1;
	while (1)
	 {
		 if(--counter == 0)
			 {
			  counter = imin(s->l, 1000);
			  if (param.shrinking) 
						 do_shrinking(s);
			 }
		 if (select_working_set_general(s, &i, &j, nu) != 0)
			 {
			  reconstruct_gradient(s);
			  s->active_size = s->l;
			  if (select_working_set_general(s, &i, &j, nu) != 0)
				   break;
			  else
				   counter = 1;	
			 }		
		++iter;
		Q_i = solver_get_Q(s, i, s->active_size);
		Q_j = solver_get_Q(s, j, s->active_size);
		C_i = get_C(s, i);
		C_j = get_C(s, j);
		old_alpha_i = s->alpha[i];
		old_alpha_j = s->alpha[j];
		if (s->y[i] != s->y[j])
		 {
			 delta = (-s->G[i] - s->G[j]) / fmax(Q_i[i] + Q_j[j] + 2 * Q_i[j], 0.0);
			 diff = s->alpha[i] - s->alpha[j];
			 s->alpha[i] += delta;
			 s->alpha[j] += delta;			
			 if (diff > 0)
			  {
		  		if (s->alpha[j] < 0)
						 {
				   	s->alpha[j] = 0;
					   s->alpha[i] = diff;
						 }
				 }
			 else
				 {
				  if (s->alpha[i] < 0)
						 {
					   s->alpha[i] = 0;
					   s->alpha[j] = -diff;
						 }
				 }
			 if (diff > C_i - C_j)
				 {
			  	if (s->alpha[i] > C_i)
						 {
					   s->alpha[i] = C_i;
					   s->alpha[j] = C_i - diff;
						 }
				 }
			 else
			  {
				  if (s->alpha[j] > C_j)
						 {
					   s->alpha[j] = C_j;
					   s->alpha[i] = C_j + diff;
						 } 
				 }
		 }
		else
		 {
			 delta = (s->G[i] - s->G[j]) / fmax(Q_i[i] + Q_j[j] - 2 * Q_i[j], 0.0);
			 sum = s->alpha[i] + s->alpha[j];
			 s->alpha[i] -= delta;
			 s->alpha[j] += delta;
			 if (sum > C_i)
				 {
			  	if (s->alpha[i] > C_i)
						 {
					   s->alpha[i] = C_i;
					   s->alpha[j] = sum - C_i;
						 }
				 }
			 else
			  {
			  	if (alpha[j] < 0)
						 {
					   s->alpha[j] = 0;
					   s->alpha[i] = sum;
						 }
				 }
			 if (sum > C_j)
				 {
			  	if (s->alpha[j] > C_j)
						 {
					   s->alpha[j] = C_j;
					   s->alpha[i] = sum - C_j;
						 }
				 }
			 else
				 {
				  if (s->alpha[i] < 0)
						 {
					   s->alpha[i] = 0;
					   s->alpha[j] = sum;
						 }
				 }
		 }
		delta_alpha_i = s->alpha[i] - old_alpha_i;
		delta_alpha_j = s->alpha[j] - old_alpha_j;	
		for (k = 0; k < s->active_size; k++)
			 s->G[k] += Q_i[k] * delta_alpha_i + Q_j[k] * delta_alpha_j;
  ui = is_upper_bound(s, i);
		uj = is_upper_bound(s, j);
		update_alpha_status(s, i);
		update_alpha_status(s, j);
		if (ui != is_upper_bound(s, i))
		 {
				Q_i = solver_get_Q(s, i, s->l);
				if (ui)
					 for (k = 0; k < s->l; k++)
						  s->G_bar[k] -= C_i * Q_i[k];
				else
					 for (k = 0;k < s->l; k++)
						  s->G_bar[k] += C_i * Q_i[k];
		 }
		if (uj != is_upper_bound(s, j))
		 {
				Q_j = solver_get_Q(s, j, s->l);
				if (uj)
					 for (k = 0; k < s->l; k++)
						  s->G_bar[k] -= C_j * Q_j[k];
				else
					 for (k = 0; k < s->l; k++)
						  s->G_bar[k] += C_j * Q_j[k];
		 }
	 }
	si->rho = calculate_rho_general(s, si, nu);
	v = 0;
	for (i = 0; i < s->l; i++)
			v += s->alpha[i] * (s->G[i] + s->b[i]);
	si->obj = v / 2;
 for (i = 0; i < s->l; i++)
			alpha[s->active_set[i]] = s->alpha[i];
	si->upper_bound_p = Cp;
	si->upper_bound_n = Cn;
	safe_free(s->b);
	safe_free(s->y);
	safe_free(s->alpha);
	safe_free(s->alpha_status);
	safe_free(s->active_set);
	safe_free(s->G);
	safe_free(s->G_bar);
	switch (qmatrixtype)
	 {
	  case QC  :free_svcq(s->svcq);
				         break;
			case QR  :free_svrq(s->svrq);
				         break;
			case QONE:free_oneclassq(s->oneclassq);
				         break;
	 }
}

void nu_solve(Nusolverptr n, int l, SVM_Q_MATRIX_TYPE qmatrixtype, double *b, signed char *y, double *alpha, double Cp, double Cn, Solutioninfoptr si, Svm_problem prob, Svm_parameter param)
{
	/*Last Changed 29.04.2005*/
	n->si = si;
	solve(&(n->s), l, qmatrixtype, b, y, alpha, Cp, Cn, si, prob, param, 1);
}

int nu_select_working_set(Solverptr s, int* out_i, int* out_j)
{
	/*Last Changed 29.04.2005*/
	double Gmax1 = -DBL_MAX, Gmax2 = -DBL_MAX, Gmax3 = -DBL_MAX, Gmax4 = -DBL_MAX;	
	int Gmax1_idx = -1, Gmax2_idx = -1, Gmax3_idx = -1, Gmax4_idx = -1, i;
	for (i = 0; i < s->active_size;i++)
	 {
		 if (s->y[i] == +1)	
			 {
			  if (!is_upper_bound(s, i))	
					 {
				   if (-s->G[i] >= Gmax1)
							 {
					    Gmax1 = -s->G[i];
					    Gmax1_idx = i;
							 }
					 }
			  if (!is_lower_bound(s, i))	
					 {
				   if (s->G[i] >= Gmax2)
							 {
					    Gmax2 = s->G[i];
					    Gmax2_idx = i;
							 }
					 }
			 }
		 else		
			 {
			  if (!is_upper_bound(s, i))	
					 {
				   if (-s->G[i] >= Gmax3)
							 {  
					    Gmax3 = -s->G[i];
					    Gmax3_idx = i;
							 }
					 }
			  if (!is_lower_bound(s, i))	
					 {
				   if (s->G[i] >= Gmax4)
							 {
					    Gmax4 = s->G[i];
					    Gmax4_idx = i;
							 }
					 }
			 }
	 }
	if (fmax(Gmax1 + Gmax2, Gmax3 + Gmax4) < s->eps)
 		return 1;
	if (Gmax1 + Gmax2 > Gmax3 + Gmax4)
	 {
 		*out_i = Gmax1_idx;
 		*out_j = Gmax2_idx;
	 }
	else
	 {
		 *out_i = Gmax3_idx;
		 *out_j = Gmax4_idx;
	 }
	return 0;
}

void nu_do_shrinking(Solverptr s)
{
	/*Last Changed 29.04.2005*/
	double Gmax1 = -DBL_MAX, Gmax2 = -DBL_MAX, Gmax3 = -DBL_MAX, Gmax4 = -DBL_MAX, Gm1, Gm2, Gm3, Gm4;	
	int k;
	for (k = 0; k < s->active_size; k++)
	 {
	 	if (!is_upper_bound(s, k))
			 {
			  if (s->y[k] == +1)
				 	{
				   if (-s->G[k] > Gmax1) 
								 Gmax1 = -s->G[k];
					 }
			  else	
						 if (-s->G[k] > Gmax3) 
								 Gmax3 = -s->G[k];
			 }
		 if (!is_lower_bound(s, k))
			 {
		  	if (s->y[k] == +1)
					 {	
				   if (s->G[k] > Gmax2) 
								 Gmax2 = s->G[k];
					 }
			  else	
						 if (s->G[k] > Gmax4) 
								 Gmax4 = s->G[k];
			 }
	 }
 Gm1 = -Gmax2;
 Gm2 = -Gmax1;
	Gm3 = -Gmax4;
	Gm4 = -Gmax3;
	for (k = 0; k < s->active_size; k++)
	 {
		 if (is_lower_bound(s, k))
			 {
		  	if (s->y[k] == +1)
					 {
				   if (-s->G[k] >= Gm1) 
								 continue;
					 }
			  else	
						 if (-s->G[k] >= Gm3)  
								 continue;
			 }
		 else 
				 if (is_upper_bound(s, k))
					 {
			    if (s->y[k] == +1)
							 {
				     if (s->G[k] >= Gm2) 
										 continue;
							 }
			    else	
								 if (s->G[k] >= Gm4) 
										 continue;
					 }
		   else 
						 continue;
		 --s->active_size;
		 solver_swap_index(s, k, s->active_size);
		 --k;	
	 }
	if (s->unshrinked || fmax(-(Gm1+Gm2), -(Gm3+Gm4)) > s->eps * 10) 
		 return;
	s->unshrinked = 1;
	reconstruct_gradient(s);
	for (k = s->l - 1; k >= s->active_size; k--)
	 {
	 	if (is_lower_bound(s, k))
			 {
			  if (s->y[k] == +1)
					 {
				   if (-s->G[k] < Gm1) 
								 continue;
					 }
			  else	
						 if (-s->G[k] < Gm3) 
								 continue;
			 }
		 else 
				 if (is_upper_bound(s, k))
					 {
			    if (s->y[k] == +1)
							 {
				     if (s->G[k] < Gm2) 
										 continue;
							 }
			    else	
								 if (s->G[k] < Gm4) 
										 continue;
					 }
		   else 
						 continue;
		 solver_swap_index(s, k, s->active_size);
		 s->active_size++;
		 ++k;	
	 }
}

double nu_calculate_rho(Solverptr s, Solutioninfoptr si)
{
	/*Last Changed 29.04.2005*/
	int nr_free1 = 0, nr_free2 = 0, i;
	double ub1 = DBL_MAX, ub2 = DBL_MAX, lb1 = -DBL_MAX, lb2 = -DBL_MAX, sum_free1 = 0, sum_free2 = 0, r1, r2;
	for (i = 0; i < s->active_size; i++)
	 {
	 	if (s->y[i] == +1)
			 {
			  if (is_lower_bound(s, i))
				   ub1 = fmin(ub1, s->G[i]);
			  else 
						 if (is_upper_bound(s, i))
				     lb1 = fmax(lb1, s->G[i]);
			    else
							 {
				     ++nr_free1;
				     sum_free1 += s->G[i];
							 }
			 }
		 else
			 {
		  	if (is_lower_bound(s, i))
				   ub2 = fmin(ub2, s->G[i]);
			  else 
						 if (is_upper_bound(s, i))
				     lb2 = fmax(lb2, s->G[i]);
			    else
							 {
				     ++nr_free2;
				     sum_free2 += s->G[i];
							 }
			 }
	 }
	if (nr_free1 > 0)
		 r1 = sum_free1 / nr_free1;
	else
		 r1 = (ub1 + lb1) / 2;	
	if (nr_free2 > 0)
		 r2 = sum_free2 / nr_free2;
	else
		 r2 = (ub2 + lb2) / 2;	
	si->r = (r1 + r2) / 2;
	return (r1 - r2) / 2;
}
