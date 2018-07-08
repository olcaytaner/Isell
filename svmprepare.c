#include "instance.h"
#include "instancelist.h" 
#include "libmemory.h"
#include "libmisc.h"
#include "partition.h" 
#include "parameter.h"
#include "svmprepare.h"

extern Datasetptr current_dataset;

/**
 * Converts current problem notation the notation used in the SVMsolver. This includes setting the number of classes, data size, the output for each instance (0 or 1 for binary classification problems, class index for K > 2 classification problems, real regression value for regression problems), the input vector for each instance.
 * @param[out] prob The output problem
 * @param[in] probtype The type of the SVM problem. C_SVC for K class C-svm, NU_SVC for K class nu-svm, TWO_CLASS for two class SVM, EPSILON_SVR and NU_SVR for epsilon and nu based regression SVM's, ONE_CLASS for density estimation.
 * @param[in] data Training sample.
 * @param[in] p Partitioning of classes.
 */
void prepare_svm_problem(Svm_problemptr prob, SVM_TYPE probtype, Instanceptr data, Partition p)
{
 /*Last Changed 27.05.2005*/
	int i = 0, *assignments;
	Instanceptr tmp;
	if (probtype == TWO_CLASS)
	 {
			assignments = find_class_assignments(p);
		 prob->nr_class = 2;
	 }
	else
  	prob->nr_class = current_dataset->classno;
	prob->l = data_size(data);
	prob->x = safemalloc(prob->l * sizeof(Svm_nodeptr), "prepare_svm_parameters", 11);
	prob->y = safemalloc(prob->l * sizeof(double), "prepare_svm_parameters", 12);
	tmp = data;
	while (tmp)
	 {
		 if (in_list(probtype, 2, C_SVC, NU_SVC))
		   prob->y[i] = give_classno(tmp);
			else
				 if (probtype == TWO_CLASS)
							prob->y[i] = assignments[give_classno(tmp)];
					else
  				 prob->y[i] = give_regressionvalue(tmp);
			prob->x[i] = instance_to_svmnode(tmp);
		 tmp = tmp->next;
			i++;
	 }
	if (probtype == TWO_CLASS)
			safe_free(assignments);
}

/**
 * Prepares the SVMsolver parameters according to the global parameters.
 * @param[out] param The parameters of the problem to be prepared.
 * @param[in] probtype The type of the SVM problem. C_SVC for K class C-svm, NU_SVC for K class nu-svm, TWO_CLASS for two class SVM, EPSILON_SVR and NU_SVR for epsilon and nu based regression SVM's, ONE_CLASS for density estimation.
 */
void prepare_svm_parameters(Svm_parameterptr param, SVM_TYPE probtype)
{
	/*Last Changed 02.05.2005*/
	param->cache_size = get_parameter_i("svmcache");
	param->C = get_parameter_f("svmC");
	param->coef0 = get_parameter_f("svmcoef0");
	param->degree = get_parameter_i("svmdegree");
	param->eps = 0.001;
	param->gamma = get_parameter_f("svmgamma");
 if (current_dataset->storetype == STORE_KERNEL)
   param->kernel_type = KPREDEFINED;
 else
	  param->kernel_type = get_parameter_l("kerneltype");
	param->nu = get_parameter_f("svmnu");
	param->p = get_parameter_f("svmepsilon");
	param->shrinking = 1;
	if (probtype == TWO_CLASS)
		 probtype = C_SVC;
	param->svm_type = probtype;
	param->cweights = (double *) get_parameter_a("svmcweights");
}
