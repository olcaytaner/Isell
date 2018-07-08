#include<math.h>
#include "instance.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "messages.h"
#include "parameter.h"
#include "svm.h"
#include "svmkernel.h"
#include "svmsolve.h"
#include "clustering.h"

extern Datasetptr current_dataset;

/**
 * Destructor for the SVM_split structure. Deallocates memory allocated for the support vector coefficients, number of support vectors in each class, support vectors and split weights.
 * @param[in] s Svm_split structure
 */
void free_svm_split(Svm_split s)
{
	/*Last Changed 25.07.2005 added weights*/
	/*Last Changed 28.05.2005*/
	int i;
	safe_free(s.sv_coef);
	for (i = 0; i < s.l; i++)
		 safe_free(s.SV[i]);
	safe_free(s.SV);
	safe_free(s.weights);
}

/**
 * Destructor for the Support Vector Machine model structure. Deallocates memory allocated for the number of support vectors in each class, support vectors, rho array, support vector coefficients, A and B probability arrays used in posterior probability estimation.
 * @param[in] model Support Vector Machine model
 */
void free_svm_model(Svm_modelptr model)
{
 /*LAst Changed 04.01.2006 added probA and probB*/
	/*Last Changed 03.05.2005*/
	int i;
	safe_free(model->nSV);
	safe_free(model->rho);
	safe_free(model->SV);
	for (i = 0; i < model->nr_class - 1; i++)
		 safe_free(model->sv_coef[i]);
	safe_free(model->sv_coef);
 safe_free(model->probA);
 safe_free(model->probB);
	safe_free(model);
}

/**
 * Destructor for the SVM problem structure. Deallocates memory allocated for the outputs and input vectors.
 * @param[in] prob SVM problem structure
 */
void free_svm_problem(Svm_problem prob)
{
	/*Last Changed 03.05.2005*/
	int i;
 if (prob.l > 0)
  {
	  safe_free(prob.y);
	  for (i = 0; i < prob.l; i++)
		   safe_free(prob.x[i]);
	  safe_free(prob.x);
  }
}

/**
 * Wrapper function for C-SVM solver. Initializes the output values, alpha values and calls solve function to solve two-class C-SVM.
 * @param[in] prob SVM problem
 * @param[in] param Parameters of the SVM algorithm
 * @param[out] alpha Coefficients of the support vectors
 * @param[out] si Output structure for the SVM solution
 * @param[in] Cp C for positive class
 * @param[in] Cn C for negative class
 */
void solve_c_svc(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si, double Cp, double Cn)
{
	/*Last Changed 29.04.2005*/
	int i, l = prob.l;
	double *minus_ones, sum_alpha = 0.0;
	signed char* y;
	Solverptr s;
 minus_ones = safemalloc(l * sizeof(double), "solve_c_svc", 5);
	y = safemalloc(l * sizeof(signed char), "solve_c_svc", 6);
	s = safemalloc(sizeof(Solver), "solve_c_svc", 7);
	for (i = 0; i < l; i++)
	 {
		 alpha[i] = 0;
		 minus_ones[i] = -1;
		 if (prob.y[i] > 0) 
				 y[i] = +1; 
			else 
				 y[i] = -1;
	 }
	solve(s, l, QC, minus_ones, y, alpha, Cp, Cn, si, prob, param, 0);
	for (i = 0; i < l; i++)
		 sum_alpha += alpha[i];
	for (i = 0; i < l; i++)
		 alpha[i] *= y[i];
	safe_free(s);
	safe_free(minus_ones);
	safe_free(y);
}

/**
 * Wrapper function for nu-SVM solver. Initializes the output values, alpha values and calls nu_solve function to solve two-class nu-SVM.
 * @param[in] prob SVM problem
 * @param[in] param Parameters of the SVM algorithm
 * @param[out] alpha Coefficients of the support vectors
 * @param[out] si Output structure for the SVM solution
 */
void solve_nu_svc(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si)
{
	/*Last Changed 29.04.2005*/
	Nusolverptr n;
	int i, l = prob.l;
	double nu = param.nu, sum_pos, sum_neg, *zeros, r;
	signed char* y;
	y = safemalloc(l * sizeof(signed char), "solve_nu_svc", 5);
	n = safemalloc(sizeof(Nusolver), "solve_nu_svc", 6);
	for (i = 0; i < l; i++)
		 if (prob.y[i] > 0)
			  y[i] = +1;
		 else
			  y[i] = -1;
	sum_pos = nu * l / 2;
	sum_neg = nu * l / 2;
	for (i = 0; i < l; i++)
		 if (y[i] == +1)
			 {
	  		alpha[i] = fmin(1.0, sum_pos);
			  sum_pos -= alpha[i];
			 }
		 else
			 {
		  	alpha[i] = fmin(1.0, sum_neg);
			  sum_neg -= alpha[i];
			 }
 zeros = safemalloc(l * sizeof(double), "solve_nu_svc", 25);
	for (i = 0; i < l; i++)
		 zeros[i] = 0;
	nu_solve(n, l, QC, zeros, y, alpha, 1.0, 1.0, si, prob, param);
	r = si->r;
	for (i = 0; i < l; i++)
		 alpha[i] *= y[i] / r;
	si->rho /= r;
	si->obj /= (r*r);
	si->upper_bound_p = 1/r;
	si->upper_bound_n = 1/r;
	safe_free(n);
	safe_free(y);
	safe_free(zeros);
}

/**
 * Wrapper function for oneclass-SVM solver. Initializes alpha values and calls solve function to solve oneclass-SVM.
 * @param[in] prob SVM problem
 * @param[in] param Parameters of the SVM algorithm
 * @param[out] alpha Coefficients of the support vectors
 * @param[out] si Output structure for the SVM solution
 */
void solve_one_class(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si)
{
	/*Last Changed 29.04.2005*/
	Solverptr s;
	int l = prob.l, i, n;
	double *zeros; 
	signed char *ones; 
 zeros = safemalloc(l * sizeof(double), "solve_one_class", 5);
	ones = safemalloc(l * sizeof(signed char), "solve_one_class", 6);
	s = safemalloc(sizeof(Solver), "solve_one_class", 7);
 n = (int)(param.nu * prob.l);	
	for (i = 0; i < n; i++)
		 alpha[i] = 1;
	alpha[n] = param.nu * prob.l - n;
	for (i = n + 1; i < l; i++)
		 alpha[i] = 0;
	for (i = 0; i < l; i++)
	 {
 		zeros[i] = 0;
 		ones[i] = 1;
	 }
	solve(s, l, QONE, zeros, ones, alpha, 1.0, 1.0, si, prob, param, 0);
	safe_free(s);
	safe_free(zeros);
	safe_free(ones);
}

/**
 * Wrapper function for C-SVM-regression solver. Initializes alpha, output values and calls solve function to solve C-SVM-regression problem.
 * @param[in] prob SVM problem
 * @param[in] param Parameters of the SVM algorithm
 * @param[out] alpha Coefficients of the support vectors
 * @param[out] si Output structure for the SVM solution
 */
void solve_epsilon_svr(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si)
{
	/*Last Changed 29.04.2005*/
	Solverptr s;
	int l = prob.l, i;
	double *alpha2, *linear_term, sum_alpha;
	signed char* y;
 alpha2 = safemalloc(2 * l * sizeof(double), "solve_epsilon_svr", 5);
 linear_term = safemalloc(2 * l * sizeof(double), "solve_epsilon_svr", 6);
	y = safemalloc(2 * l * sizeof(signed char), "solve_epsilon_svr", 7);
	s = safemalloc(sizeof(Solver), "solve_epsilon_svr", 8);
	for (i = 0; i < l; i++)
	 {
 		alpha2[i] = 0;
	 	linear_term[i] = param.p - prob.y[i];
		 y[i] = 1;
		 alpha2[i + l] = 0;
		 linear_term[i + l] = param.p + prob.y[i];
		 y[i + l] = -1;
	 }
 solve(s, 2 * l, QR, linear_term, y, alpha2, param.C, param.C, si, prob, param, 0);
 sum_alpha = 0;
	for (i = 0; i < l; i++)
	 {
 		alpha[i] = alpha2[i] - alpha2[i+l];
		 sum_alpha += fabs(alpha[i]);
	 }
	safe_free(s);
	safe_free(alpha2);
	safe_free(linear_term);
	safe_free(y);
}

/**
 * Wrapper function for nu-SVM-regression solver. Initializes alpha, output values and calls nu_solve function to solve nu-SVM-regression problem.
 * @param[in] prob SVM problem
 * @param[in] param Parameters of the SVM algorithm
 * @param[out] alpha Coefficients of the support vectors
 * @param[out] si Output structure for the SVM solution
 */
void solve_nu_svr(Svm_problem prob, Svm_parameter param, double *alpha, Solutioninfoptr si)
{
	/*Last Changed 29.04.2005*/
	Nusolverptr n;
	int l = prob.l, i;
	double C = param.C, *alpha2, *linear_term, sum;
	signed char *y;
 alpha2 = safemalloc(2 * l * sizeof(double), "solve_nu_svr", 5);
 linear_term = safemalloc(2 * l * sizeof(double), "solve_nu_svr", 6);
	y = safemalloc(2 * l * sizeof(signed char), "solve_nu_svr", 7);
	n = safemalloc(sizeof(Nusolver), "solve_nu_svr", 8);
	sum = C * param.nu * l / 2;
	for (i = 0; i < l; i++)
	 {
	 	alpha2[i] = fmin(sum, C);
			alpha2[i + l] = fmin(sum, C);
		 sum -= alpha2[i];
		 linear_term[i] = -prob.y[i];
		 y[i] = 1;
		 linear_term[i+l] = prob.y[i];
		 y[i+l] = -1;
	 }
 nu_solve(n, 2 * l, QR, linear_term, y, alpha2, C, C, si, prob, param);
	for (i = 0; i < l; i++)
		 alpha[i] = alpha2[i] - alpha2[i + l];
	safe_free(n);
	safe_free(alpha2);
 safe_free(linear_term);
	safe_free(y);
}

/**
 * Wrapper function for SVM solver. Calls appropriate solver function according to the problem.
 * @param[in] prob SVM problem
 * @param[in] param Parameters of the SVM algorithm
 * @param[in] Cp C for positive class
 * @param[in] Cn C for negative class
 * @return Solution to the SVM problem
 */
Svm_weights svm_train_one(Svm_problem prob, Svm_parameter param, double Cp, double Cn)
{
	/*Last Changed 29.04.2005*/
	Svm_weights result;
	double *alpha;
	Solutioninfo si;
	int nSV = 0, nBSV = 0, i;
 alpha = safemalloc(sizeof(double) * prob.l, "svm_train_one", 5);
	switch (param.svm_type)
	 {
	 	case C_SVC      :solve_c_svc(prob, param, alpha, &si, Cp, Cn);
			                 break;
		 case NU_SVC     :solve_nu_svc(prob, param, alpha, &si);
			                 break;
		 case ONE_CLASS  :solve_one_class(prob, param, alpha, &si);
			                 break;
		 case EPSILON_SVR:solve_epsilon_svr(prob, param, alpha, &si);
			                 break;
		 case NU_SVR     :solve_nu_svr(prob, param, alpha, &si);
			                 break;
   default         :printf(m1234, param.svm_type);
                    break;
	 }
	for (i = 0; i < prob.l; i++)
	 {
	 	if (fabs(alpha[i]) > 0)
			 {
		  	++nSV;
			  if (prob.y[i] > 0)
					 {
				   if (fabs(alpha[i]) >= si.upper_bound_p)
					    ++nBSV;
					 }
			  else
					 {
			   	if (fabs(alpha[i]) >= si.upper_bound_n)
					    ++nBSV;
					 }
			 }
	 }
	result.alpha = alpha;
	result.rho = si.rho;
	result.nSV = nSV;
	result.nBSV = nBSV;
	return result;
}

int svm_group_classes(Svm_problem prob, int **start_ret, int **count_ret, int *perm)
{
	/*Last Changed 02.05.2005*/
	int l,  *count, *data_label, i, classno, *start, classes[MAXCLASSES], nr_class = 0;
 l = prob.l;
 for (i = 0; i < l; i++)
	  search_and_add_into_list(classes, &nr_class, (int)prob.y[i]);
 count = safecalloc(nr_class, sizeof(int), "svm_group_classes", 5);
	data_label = safemalloc(l * sizeof(int), "svm_group_classes", 6);
	for (i = 0; i < l; i++)
	 {
		 classno = search_list(classes, nr_class, (int)prob.y[i]);
			(count[classno])++;
		 data_label[i] = classno;
	 }
	start = safemalloc(nr_class * sizeof(int), "svm_group_classes", 11);
	start[0] = 0;
	for (i = 1;i < nr_class; i++)
		 start[i] = start[i - 1] + count[i - 1];
	for (i = 0; i < l; i++)
	 {
 		perm[start[data_label[i]]] = i;
		 ++start[data_label[i]];
	 }
	start[0] = 0;
	for (i = 1;i < nr_class; i++)
		 start[i] = start[i - 1] + count[i - 1];
	*start_ret = start;
	*count_ret = count;
	safe_free(data_label);
 return nr_class;
}

Svm_modelptr svm_train(Svm_problem prob, Svm_parameter param, int calculate_probability)
{
 /*Last Changed 04.01.2006 added probA, probB*/
	/*Last Changed 01.05.2005*/
	int i, j, k, l, nr_class, *start, *count, *perm, *nonzero, p = 0, si, sj, ci, cj, total_sv, *nz_count, nSV, *nz_start, q;
	Svm_weights f, *weights;
	Svm_modelptr model;
	Svm_node** x;
	Svm_problem sub_prob;
 model = safemalloc(sizeof(Svm_model), "svm_train", 6);
	model->prob = prob;
	model->param = param;
	if (in_list(param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
	 {
 		model->nr_class = 2;
   model->probA = NULL; 
   model->probB = NULL;
	 	model->nSV = NULL;
		 model->sv_coef = safemalloc(sizeof(double*), "svm_train", 12);
		 f = svm_train_one(prob, param, 0, 0);
		 model->rho = safemalloc(sizeof(double), "svm_train", 14);
		 model->rho[0] = f.rho;
		 model->l = f.nSV;
		 model->SV = safemalloc(sizeof(Svm_nodeptr) * f.nSV, "svm_train", 17);
		 model->sv_coef[0] = safemalloc(sizeof(double) * f.nSV, "svm_train", 18);
		 j = 0;
		 for (i = 0; i < prob.l; i++)
			  if (fabs(f.alpha[i]) > 0)
					 {
				   model->SV[j] = prob.x[i];
				   model->sv_coef[0][j] = f.alpha[i];
				   ++j;
					 }		
		 safe_free(f.alpha);
	 }
	else
	 {
 		l = prob.l;
 		start = NULL;
 		count = NULL;
 		perm = safemalloc(l * sizeof(int), "svm_train", 34);
 		nr_class = svm_group_classes(prob, &start, &count, perm);		
 		x = safemalloc(l * sizeof(Svm_nodeptr), "svm_train", 37);
 		for (i = 0; i < l; i++)
 			 x[i] = prob.x[perm[i]];
 		nonzero = safemalloc(l * sizeof(int), "svm_train", 40);
 		for (i = 0; i < l; i++)
 			 nonzero[i] = 0;
 		weights = safemalloc(sizeof(Svm_weights) * nr_class * (nr_class - 1) / 2, "svm_train", 43);
   if (calculate_probability)
    {
     model->probA = safemalloc(sizeof(double) * nr_class * (nr_class - 1) / 2, "svm_train", 44);
     model->probB = safemalloc(sizeof(double) * nr_class * (nr_class - 1) / 2, "svm_train", 44);
    }
   else
    {
     model->probA = NULL;
     model->probB = NULL;
    }
 		for (i = 0; i < nr_class; i++)
 			 for (j = i + 1; j < nr_class; j++)
					 {
			 	  si = start[i];
			 			sj = start[j];
			 	  ci = count[i];
			 			cj = count[j];
			 	  sub_prob.l = ci + cj;
			 	  sub_prob.x = safemalloc(sizeof(Svm_nodeptr) * sub_prob.l, "svm_train", 52);
			 	  sub_prob.y = safemalloc(sizeof(double) * sub_prob.l, "svm_train", 53);
			 	  for (k = 0; k < ci; k++)
							 {
				 	   sub_prob.x[k] = x[si + k];
				 	   sub_prob.y[k] = +1;
							 }
				   for (k = 0; k < cj; k++)
							 {
					    sub_prob.x[ci + k] = x[sj + k];
					    sub_prob.y[ci + k] = -1;
							 }
       if (calculate_probability)
         svm_binary_svc_probability(sub_prob, param, param.C * param.cweights[i], param.C * param.cweights[j], &(model->probA[p]), &(model->probB[p]));
			 	  weights[p] = svm_train_one(sub_prob, param, param.C * param.cweights[i], param.C * param.cweights[j]);
			 	  for (k = 0; k < ci; k++)
			 		   if (!nonzero[si + k] && fabs(weights[p].alpha[k]) > 0)
			 			    nonzero[si + k] = 1;
			 	  for (k = 0; k < cj; k++)
    					if (!nonzero[sj + k] && fabs(weights[p].alpha[ci+k]) > 0)
			 			    nonzero[sj + k] = 1;
  	 			safe_free(sub_prob.x);
	   			safe_free(sub_prob.y);
			 	  ++p;
					 }
		 model->nr_class = nr_class;
		 model->rho = safemalloc(sizeof(double) * nr_class * (nr_class - 1) / 2, "svm_train", 76);
		 for (i = 0;i < nr_class * (nr_class - 1) / 2; i++)
 	 		model->rho[i] = weights[i].rho;
		 total_sv = 0;
		 nz_count = safemalloc(nr_class * sizeof(int), "svm_train", 80);
		 model->nSV = safemalloc(nr_class * sizeof(int), "svm_train", 81);
		 for (i = 0; i < nr_class; i++)
			 {
	 		 nSV = 0;
	 		 for (j = 0; j < count[i]; j++)
	 			  if (nonzero[start[i] + j])
							 {	
				    	++nSV;
				 	   ++total_sv;
							 }
		 	 model->nSV[i] = nSV;
		 	 nz_count[i] = nSV;
			 }
 		model->l = total_sv;
 		model->SV = safemalloc(sizeof(Svm_nodeptr) * total_sv, "svm_train", 95);
 		p = 0;
 		for (i = 0; i < l; i++)
 			 if (nonzero[i]) 
				 	 model->SV[p++] = x[i];
   nz_start = safemalloc(nr_class * sizeof(int), "svm_train", 100);
 		nz_start[0] = 0;
 		for (i = 1; i < nr_class; i++)
 			 nz_start[i] = nz_start[i - 1] + nz_count[i - 1];
 		model->sv_coef = safemalloc(sizeof(double *) * (nr_class-1), "svm_train", 104);
 		for (i = 0; i < nr_class - 1; i++)
 			 model->sv_coef[i] = safemalloc(sizeof(double) * total_sv, "svm_train", 106);
 		p = 0;
 		for (i = 0; i < nr_class; i++)
 			 for (j = i + 1; j < nr_class; j++)
					 {
		 		  si = start[i];
		 		  sj = start[j];
		 		  ci = count[i];
		 		  cj = count[j];				
		 		  q = nz_start[i];
   				for (k = 0; k < ci; k++)
    					if (nonzero[si + k])
		 				    model->sv_coef[j - 1][q++] = weights[p].alpha[k];
   				q = nz_start[j];
		 		  for (k = 0; k < cj; k++)
    					if (nonzero[sj + k])
     						model->sv_coef[i][q++] = weights[p].alpha[ci+k];
   				++p;
					 }		
		 safe_free(count);
		 safe_free(perm);
		 safe_free(start);
		 safe_free(x);
		 safe_free(nonzero);
		 for (i = 0; i < nr_class * (nr_class - 1) / 2; i++)
 	 		safe_free(weights[i].alpha);
		 safe_free(weights);
		 safe_free(nz_count);
		 safe_free(nz_start);
	 }
	return model;
}

void svm_predict_values(Svm_modelptr model, Svm_nodeptr x, double* dec_values)
{
	/*Last Changed 02.05.2005*/
 ON_OFF normalizekernel = get_parameter_o("normalizekernel");
	int i, j, k, nr_class, l, *start, p = 0, pos = 0, si, sj, ci, cj;
	double* sv_coef, sum = 0.0, *kvalue, *coef1, *coef2;
	if (in_list(model->param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
	 {
	 	sv_coef = model->sv_coef[0];
		 for (i = 0; i < model->l; i++)
     if (normalizekernel == ON)
  			  sum += sv_coef[i] * k_function_normalized(x, model->SV[i], model->param);
     else
       sum += sv_coef[i] * k_function(x, model->SV[i], model->param);
		 sum -= model->rho[0];
		 *dec_values = sum;
	 }
	else
	 {
 		nr_class = model->nr_class;
 		l = model->l;
 		kvalue = safemalloc(sizeof(double) * l, "svm_predict_values", 15);
 		for (i = 0; i < l; i++)
     if (normalizekernel == ON)
   			 kvalue[i] = k_function_normalized(x, model->SV[i], model->param);
     else
       kvalue[i] = k_function(x, model->SV[i], model->param);
 		start = safemalloc(sizeof(int) * nr_class, "svm_predict_values", 18);
 		start[0] = 0;
 		for (i = 1; i < nr_class; i++)
 			 start[i] = start[i - 1] + model->nSV[i - 1];
 		for (i = 0; i < nr_class; i++)
 			 for (j = i + 1; j < nr_class; j++)
					 {
						 sum = 0;
			 	  si = start[i];
			 	  sj = start[j];
			 	  ci = model->nSV[i];
			 	  cj = model->nSV[j];				
			 	  coef1 = model->sv_coef[j-1];
			 	  coef2 = model->sv_coef[i];
  	 			for (k = 0; k < ci; k++)
  	  				sum += coef1[si + k] * kvalue[si + k];
  	 			for (k = 0; k < cj; k++)
    					sum += coef2[sj + k] * kvalue[sj + k];
  	 			sum -= model->rho[p++];
  	 			dec_values[pos++] = sum;
					 }
		 safe_free(kvalue);
		 safe_free(start);
	 }
}

void sigmoid_train(int l, double *dec_values, double *labels, double* A, double* B)
{
 double prior1 = 0, prior0 = 0, min_step = 1e-10, sigma = 1e-3, eps = 1e-5, hiTarget, loTarget, *t;
 double fApB, p, q, h11, h22, h21, g1, g2, det, dA, dB, gd, stepsize, newA, newB, newf, d1, d2, fval = 0.0;
 int i, max_iter = 100, iter;
 for (i = 0; i < l; i++)
   if (labels[i] > 0) 
     prior1 += 1;
   else 
     prior0 += 1;
 hiTarget = (prior1 + 1.0) / (prior1 + 2.0);
 loTarget = 1 / (prior0 + 2.0);
 t = safemalloc(l * sizeof(double), "sigmoid_train", 10);
 /* Initial Point and Initial Fun Value*/
 *A = 0.0; 
 *B = log((prior0 + 1.0) / (prior1 + 1.0));
 for (i = 0; i < l; i++)
  {
   if (labels[i] > 0) 
     t[i] = hiTarget;
   else 
     t[i] = loTarget;
   fApB = dec_values[i] * (*A) + (*B);
   if (fApB >= 0)
     fval += t[i] * fApB + log(1 + exp(-fApB));
   else
     fval += (t[i] - 1) * fApB + log(1 + exp(fApB));
  }
 for (iter = 0; iter < max_iter; iter++)
  {
   h11 = sigma; 
   h22 = sigma;
   h21 = 0.0;
   g1 = 0.0;
   g2 = 0.0;
   for (i = 0; i < l; i++)
    {
     fApB = dec_values[i] * (*A) + (*B);
     if (fApB >= 0)
      {
       p = exp(-fApB) / (1.0 + exp(-fApB));
       q = 1.0 / (1.0 + exp(-fApB));
      }
     else
      {
       p = 1.0 / (1.0 + exp(fApB));
       q = exp(fApB) / (1.0 + exp(fApB));
      }
     d2 = p * q;
     h11 += dec_values[i] * dec_values[i] * d2;
     h22 += d2;
     h21 += dec_values[i] * d2;
     d1 = t[i] - p;
     g1 += dec_values[i] * d1;
     g2 += d1;
    }
   if (fabs(g1) < eps && fabs(g2) < eps)
     break;
   /* Finding Newton direction: -inv(H') * g*/
   det = h11 * h22 - h21 * h21;
   dA = -(h22 * g1 - h21 * g2) / det;
   dB = -(-h21 * g1 + h11 * g2) / det;
   gd = g1 * dA + g2 * dB;
   stepsize = 1;   /* Line Search*/
   while (stepsize >= min_step)
    {
     newA = (*A) + stepsize * dA;
     newB = (*B) + stepsize * dB;
     /* New function value*/
     newf = 0.0;
     for (i = 0; i < l; i++)
      {
       fApB = dec_values[i] * newA + newB;
       if (fApB >= 0)
         newf += t[i] * fApB + log(1 + exp(-fApB));
       else
         newf += (t[i] - 1)*fApB + log(1 + exp(fApB));
      }
     /* Check sufficient decrease*/
     if (newf < fval + 0.0001 * stepsize * gd)
      {
       *A = newA;
       *B = newB;
       fval = newf;
       break;
      }
     else
       stepsize = stepsize / 2.0;
    }
   if (stepsize < min_step)
     break;
 }
 safe_free(t);
}

double sigmoid_predict(double decision_value, double A, double B)
{
 double fApB = decision_value * A + B;
 if (fApB >= 0)
   return exp(-fApB) / (1.0 + exp(-fApB));
 else
   return 1.0 / (1 + exp(fApB)) ;
}

/* Method 2 from the multiclass_prob paper by Wu, Lin, and Weng*/
void multiclass_probability(int k, double **r, double *p)
{
 int t, j;
 int iter = 0, max_iter = 100;
 double **Q, *Qp, pQp, eps=0.005 / k, max_error, error, diff;
 Q = (double**)safemalloc_2d(sizeof(double*), sizeof(double), k, k, "multiclass_probability", 4);
 Qp = safemalloc(k * sizeof(double), "multiclass_proability", 5);
 for (t = 0; t < k; t++)
  {
   p[t] = 1.0 / k;  /* Valid if k = 1*/
   Q[t][t] = 0;
   for (j = 0; j < t; j++)
    {
     Q[t][t] += r[j][t] * r[j][t];
     Q[t][j] = Q[j][t];
    }
   for (j = t + 1; j < k; j++)
    {
     Q[t][t] += r[j][t] * r[j][t];
     Q[t][j] = -r[j][t] * r[t][j];
    }
  }
 for (iter = 0; iter < max_iter; iter++)
  {
   /* stopping condition, recalculate QP,pQP for numerical accuracy*/
   pQp = 0;
   for (t = 0; t < k; t++)
    {
     Qp[t] = 0;
     for (j = 0; j < k; j++)
       Qp[t] += Q[t][j] * p[j];
     pQp += p[t] * Qp[t];
    }
   max_error = 0;
   for (t = 0; t < k; t++)
    {
     error = fabs(Qp[t] - pQp);
     if (error > max_error)
       max_error = error;
    }
   if (max_error < eps) 
     break;
   for (t = 0; t < k; t++)
    {
     diff = (-Qp[t] + pQp) / Q[t][t];
     p[t] += diff;
     pQp = (pQp + diff * (diff * Q[t][t] + 2 * Qp[t])) / (1 + diff) / (1 + diff);
     for (j = 0; j < k; j++)
      {
       Qp[j] = (Qp[j] + diff * Q[t][j]) / (1 + diff);
       p[j] /= (1 + diff);
      }
    }
  }
 free_2d((void**)Q, k);
 safe_free(Qp);
}

/* Cross-validation decision values for probability estimates*/
void svm_binary_svc_probability(Svm_problem prob, Svm_parameter param, double Cp, double Cn, double* probA, double* probB)
{
 int i, nr_fold = 5, *perm, begin, end, j, k, p_count, n_count;
 double* dec_values, firstlabel;
 Svm_problem subprob;
 Svm_parameter subparam;
 Svm_modelptr submodel;
 dec_values = safemalloc(prob.l * sizeof(double), "svm_binary_svc_probability", 3);
 perm = random_array(prob.l);
 for (i = 0; i < nr_fold; i++)
  {
   begin = i * prob.l / nr_fold;
   end = (i + 1) * prob.l / nr_fold;
   subprob.l = prob.l - (end - begin);
   subprob.x = safemalloc(subprob.l * sizeof(Svm_nodeptr), "svm_binary_svc_probability", 10);
   subprob.y = safemalloc(subprob.l * sizeof(double), "svm_binary_svc_probability", 11);
   k = 0;
   for (j = 0; j < begin; j++)
    {
     if (k == 0)
       firstlabel = prob.y[perm[j]];
     subprob.x[k] = prob.x[perm[j]];
     subprob.y[k] = prob.y[perm[j]];
     k++;
    }
   for (j = end; j < prob.l; j++)
    {
     if (k == 0)
       firstlabel = prob.y[perm[j]];
     subprob.x[k] = prob.x[perm[j]];
     subprob.y[k] = prob.y[perm[j]];
     k++;
    }
   p_count = 0;
   n_count = 0;
   for (j = 0; j < k; j++)
    {
     if (subprob.y[j] > 0)
       p_count++;
     else
       n_count++;
    }
   if (p_count == 0 && n_count == 0)
     for (j = begin; j < end; j++)
       dec_values[perm[j]] = 0;
   else 
     if (p_count > 0 && n_count == 0)
       for (j = begin; j < end; j++)
         dec_values[perm[j]] = 1;
     else 
       if (p_count == 0 && n_count > 0)
         for (j = begin; j < end; j++)
           dec_values[perm[j]] = -1;
       else
        {
         subparam = param;
         subparam.C = 1.0;
         subparam.cweights = safemalloc(2 * sizeof(double), "svm_binary_svc_probability", 40);
         subparam.cweights[0] = Cp;
         subparam.cweights[1] = Cn;
         submodel = svm_train(subprob, subparam, 0);
         for (j = begin; j < end; j++)
          {
           svm_predict_values(submodel, prob.x[perm[j]], &(dec_values[perm[j]])); 
           dec_values[perm[j]] *= -1;
          }
         free_svm_model(submodel);
         safe_free(subparam.cweights);
         safe_free(subprob.x);
         safe_free(subprob.y);
        }
  }
 sigmoid_train(prob.l, dec_values, prob.y, probA, probB);
 safe_free(dec_values);
 safe_free(perm);
}

void svm_predict_probability(Svm_modelptr model, Svm_nodeptr x, double* prob_estimates)
{
 /*Last Changed 04.01.2006*/
 int i, j, nr_class, k = 0;
 double *dec_values, min_prob = 1e-7, **pairwise_prob;
 nr_class = model->nr_class;
 dec_values = safemalloc((nr_class * (nr_class - 1) / 2) * sizeof(double), "svm_predict_probability", 4);
 svm_predict_values(model, x, dec_values);
 pairwise_prob = (double **) safemalloc_2d(sizeof(double*), sizeof(double), nr_class, nr_class, "svm_predict_probability", 6);
 for (i = 0; i < nr_class; i++)
   for (j = i + 1; j < nr_class; j++)
    {
     pairwise_prob[i][j] = fmin(fmax(sigmoid_predict(dec_values[k], model->probA[k], model->probB[k]), min_prob), 1 - min_prob);
     pairwise_prob[j][i] = 1 - pairwise_prob[i][j];
     k++;
    }
 multiclass_probability(nr_class, pairwise_prob, prob_estimates);
 free_2d((void**)pairwise_prob, nr_class);
 safe_free(dec_values);
}

double svm_predict(Svm_modelptr model, Svm_nodeptr x)
{
	/*Last Changed 02.05.2005*/
 int i, j, nr_class, *vote, pos = 0, vote_max_idx;
	double res, *dec_values;
	if (in_list(model->param.svm_type, 3, ONE_CLASS, EPSILON_SVR, NU_SVR))
	 {
		 svm_predict_values(model, x, &res);		
		 if (model->param.svm_type == ONE_CLASS)
				 if (res > 0)
						 return 1;
					else
						 return -1;
		 else
			  return res;
	 }
	else
	 {
		 nr_class = model->nr_class;
		 dec_values = safemalloc(sizeof(double) * nr_class * (nr_class-1) / 2, "svm_predict", 17);
		 svm_predict_values(model, x, dec_values);
		 vote = safemalloc(sizeof(int) * nr_class, "svm_predict", 19);
		 for (i = 0; i < nr_class; i++)
			  vote[i] = 0;
		 for (i = 0; i < nr_class; i++)
  			for (j = i + 1; j < nr_class; j++)
					 {
				   if (dec_values[pos] > 0)
    					++vote[i];
			   	else
					    ++vote[j];
				   pos++;
					 }  
		 vote_max_idx = 0;
		 for (i = 1; i < nr_class; i++)
			  if (vote[i] > vote[vote_max_idx])
				   vote_max_idx = i;
		 safe_free(vote);
		 safe_free(dec_values);
		 return vote_max_idx;
	 }
}

