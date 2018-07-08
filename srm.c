#include <math.h>
#include "algorithm.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "lang.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "srm.h"
#include "typedef.h"
#include "vc.h"

#define VC_A 0.16
#define VC_B 1.2
#define VC_K 0.14928

extern Algorithm classification_algorithms[CLASSIFICATION_ALGORITHM_COUNT];
extern Datasetptr current_dataset;

/**
 * With respect to VC-theory, the maximum deviation of error rates between two independently labeled datasets is bounded by \phi(n/h). Given rho, the following function calculates \phi(\rho).
 * @param[in] rho The parameter of the phi function
 * @return \phi(\rho)
 */
double vc_bound(double rho)
{
 /*!Last Changed 11.03.2006*/
 double val1, val2, val3;
 if (rho < 0.5)
   return 1;
 val1 = log(2 * rho) + 1;
 val2 = rho - VC_K;
 val3 = val1 / val2;
 return VC_A * val3 * (sqrt(1 + VC_B / val3) + 1);
}

/**
 * This function calculates the reverse of the \phi function (defined above). Given the \epsilon = \phi(\rho), the function calculates \rho. The reverse of the \phi function is calculated using binary-search.
 * @param[in] epsilon The output of the \phi function
 * @warning Error bound is DBL_DELTA
 * @return The reverse of the \phi function
 */
double solve_vc_bound_equation(double epsilon)
{
 /*!Last Changed 11.03.2006*/
 double start = 0.5, end = 30, middle = 0, lastmiddle, val;
 if (epsilon > 1)
   return 0.5;
 while (epsilon < vc_bound(end))
   end *= 10;
 do
  {
   lastmiddle = middle;
   middle = (start + end) / 2; 
   val = vc_bound(middle);
   if (val > epsilon)
     start = middle;
   else
     end = middle;
  }while (fabs(middle - lastmiddle) > DBL_DELTA);
 return middle;
}

/**
 * Constructor for epsilon estimate. Stores the result of a single experiment. Allocates memory for the epsilon estimate structure.
 * @param[in] epsilon Epsilon value
 * @return Allocated empty epsilon estimate.
 */
Epsilonestimateptr create_epsilon_estimate(double epsilon)
{
 /*!Last Changed 06.04.2006*/
 Epsilonestimateptr newepsilonestimate;
 newepsilonestimate = safemalloc(sizeof(Epsilonestimate), "create_epsilon_estimate", 2);
 newepsilonestimate->epsilon = epsilon;
 newepsilonestimate->next = NULL;
 return newepsilonestimate;
}

/**
 * Constructor for VC-estimate. Stores sample size of the design point and the results of the experiments in a design point. The results of the experiments are stored as a link list.
 * @param[in] n Sample size of the design point
 * @param[in] epsilonptr Link list of epsilon estimates
 * @return Allocated empty VC estimate structure
 */
Vcestimateptr create_vc_estimate(int n, Epsilonestimateptr epsilonptr)
{
 /*!Last Changed 06.04.2006*/
 Vcestimateptr newvcestimate;
 newvcestimate = safemalloc(sizeof(Vcestimate), "create_vc_estimate", 2);
 newvcestimate->n = n;
 newvcestimate->next = NULL;
 newvcestimate->before = NULL;
 newvcestimate->epsilonestimates = epsilonptr;
 newvcestimate->count = 1;
 return newvcestimate;
}

/**
 * Deallocates the link list of epsilon estimates.
 * @param[in] header Header of the link list
 */
void deallocate_epsilon_estimates(Epsilonestimateptr header)
{
 /*!Last Changed 01.04.2006*/
 Epsilonestimateptr tmp, next;
 tmp = header;
 while (tmp)
  {
   next = tmp->next;
   safe_free(tmp);
   tmp = next;
  }
}

/**
 * Deallocates the link list of design points.
 * @param[in] header Header of the link list.
 */
void deallocate_vc_estimates(Vcestimateptr header)
{
 /*!Last Changed 01.04.2006*/
 Vcestimateptr tmp, next;
 tmp = header;
 while (tmp)
  {
   next = tmp->next;
   deallocate_epsilon_estimates(tmp->epsilonestimates);
   safe_free(tmp);
   tmp = next;
  }
}

/**
 * Adds an experiment result to the link list of the corresponding design point.
 * @param[in] header Experiment results. Header of the link list of design points.
 * @param[in] n Sample size
 * @param[in] epsilon Result of the experiment
 */
void add_single_epsilon_estimate(Vcestimateptr* header, int n, double epsilon)
{
 /*!Last Changed 01.04.2006*/
 Vcestimateptr newvcestimate, tmpvc;
 Epsilonestimateptr newepsilonestimate;
 newepsilonestimate = create_epsilon_estimate(epsilon);
 tmpvc = *header;
 /* Finds the design point*/
 while (tmpvc && tmpvc->n != n)
   tmpvc = tmpvc->next;
 /*If there is a design point with the given sample size*/
 if (tmpvc)
  {
   newepsilonestimate->next = tmpvc->epsilonestimates;
   tmpvc->epsilonestimates = newepsilonestimate;
   (tmpvc->count)++;
  }
 /* If not, generate a new design point*/
 else
  {
   newvcestimate = create_vc_estimate(n, newepsilonestimate);
   if (*header)
    {
     newvcestimate->next = *header;
     (*header)->before = newvcestimate;
    }
   *header = newvcestimate;
  }
}

/**
 * Calculates the total number of experiments in an experiment design.
 * @param[in] estimates Experiment results. Header of the link list of design points.
 * @return Number of experiments in an experiment design
 */
int number_of_experiments(Vcestimateptr estimates)
{
 /*!Last Changed 01.04.2006*/
 Vcestimateptr tmpvc = estimates;
 int total = 0;
 while (tmpvc)
  {
   total += tmpvc->count;
   tmpvc = tmpvc->next;
  }
 return total;
}

/**
 * Given the experiment results and the experiment design type, calculates the VC-dimension estimate.
 * @param[in] estimates Experiment results. Header of the link list of design points.
 * @param[in] experimenttype Type of experiment design. UNIFORM_DESIGN for uniform experiment design, OPTIMIZED_DESIGN for optimized experiment design.
 * @param[in] n Sample size
 * @return VC-dimension estimate
 */
double vc_dimension_for_a_design(Vcestimateptr estimates, DESIGN_TYPE experimenttype, int n)
{
 /*!Last Changed 01.04.2006*/
 Vcestimateptr tmpvc = estimates, nextvc;
 Epsilonestimateptr tmpepsilon;
 int count = 0, i;
 double sumvcdimension = 0, sumepsilon, vcdimension, difference, before, after;
 switch (experimenttype)
  {
   case UNIFORM_DESIGN  :while (tmpvc)
                          {
                           tmpepsilon = tmpvc->epsilonestimates;
                           sumepsilon = 0.0;
                           while (tmpepsilon)
                            {
                             sumepsilon += tmpepsilon->epsilon;
                             tmpepsilon = tmpepsilon->next;
                            }
                           vcdimension = tmpvc->n / solve_vc_bound_equation(sumepsilon / tmpvc->count);
                           nextvc = tmpvc->next;
                           tmpvc->next = NULL;
                           for (i = 0; i < 6; i++)
                            {
                             difference = pow(10, -i);
                             before = mse_fitting_for_vc_dimension(tmpvc, vcdimension - difference, -1);
                             after = mse_fitting_for_vc_dimension(tmpvc, vcdimension + difference, -1);
                             if (before < after)
                               while (mse_fitting_for_vc_dimension(tmpvc, vcdimension - difference, -1) < mse_fitting_for_vc_dimension(tmpvc, vcdimension, -1))
                                 vcdimension -= difference;
                             else
                               while (mse_fitting_for_vc_dimension(tmpvc, vcdimension + difference, -1) < mse_fitting_for_vc_dimension(tmpvc, vcdimension, -1))
                                 vcdimension += difference;
                            }
                           tmpvc->next = nextvc;
                           sumvcdimension += vcdimension;
                           count++;
                           tmpvc = tmpvc->next;
                          }                         
                         vcdimension = sumvcdimension / count;
                         break;
   case OPTIMIZED_DESIGN:while (tmpvc)
                          {
                           if (tmpvc->n != n)
                            {
                             tmpepsilon = tmpvc->epsilonestimates;
                             while (tmpepsilon)
                              {
                               sumvcdimension += tmpvc->n / solve_vc_bound_equation(tmpepsilon->epsilon);
                               count++;
                               tmpepsilon = tmpepsilon->next;
                              }
                            }
                           tmpvc = tmpvc->next;
                          }
                         vcdimension = sumvcdimension / count;
                         for (i = 0; i < 6; i++)
                          {
                           difference = pow(10, -i);
                           before = mse_fitting_for_vc_dimension(estimates, vcdimension - difference, n);
                           after = mse_fitting_for_vc_dimension(estimates, vcdimension + difference, n);
                           if (before < after)
                             while (mse_fitting_for_vc_dimension(estimates, vcdimension - difference, n) < mse_fitting_for_vc_dimension(tmpvc, vcdimension, n))
                               vcdimension -= difference;
                           else
                             while (mse_fitting_for_vc_dimension(estimates, vcdimension + difference, n) < mse_fitting_for_vc_dimension(tmpvc, vcdimension, n))
                               vcdimension += difference;
                          }
                         break;
  }
 return vcdimension;
}

/**
 * Calculates the mean square error (MSE) of the VC-dimension estimate.
 * @param[in] estimates Experiment results. Header of the link list of design points.
 * @param[in] vcdimension VC-dimension estimate
 * @param[in] n Sample size
 * @return MSE of the VC-dimension estimate
 */
double mse_fitting_for_vc_dimension(Vcestimateptr estimates, double vcdimension, int n)
{
 /*!Last Changed 01.04.2006*/
 Vcestimateptr tmpvc = estimates;
 Epsilonestimateptr tmpepsilon;
 int count = 0;
 double mse = 0, add;
 while (tmpvc)
  {
   if (tmpvc->n != n)
    {
     tmpepsilon = tmpvc->epsilonestimates;
     while (tmpepsilon)
      {
       add = tmpepsilon->epsilon - vc_bound(tmpvc->n / vcdimension);
       mse += add * add;
       count++;
       tmpepsilon = tmpepsilon->next;
      }
    }
   tmpvc = tmpvc->next;
  }
 return mse / count;
}

/**
 * Calculates the epsilon estimate in a single experiment used for VC-dimension estimation of a decision tree with the given number of nodes. 
 * @param[in] parameters Parameters of the decision tree algorithm
 * @param[in] d The dataset
 * @param[in] n Sample size of the design point
 * @param[out] nodecount Number of nodes of the decision tree. The decision tree algorithm tries to generate the tree with nodecount number of nodes. But if it is unsuccessful, it returns the number of nodes in the tree.
 * @return Epsilon estimate of a single experiment
 */
double vc_single_epsilon_estimate(void* parameters, Datasetptr d, int n, int* nodecount)
{
 Instanceptr data, tmp, traindata = NULL, testdata = NULL;
 int misclassified1, misclassified2, realclassno;
 void* model;
 double pos[2]; 
 Prediction predicted;
 traindata = generate_random_data(d, n);
 data = generate_random_data(d, n);
 tmp = data;
 while (tmp)
  {
   tmp->isexception = YES;
   tmp = tmp->next;
  }
 merge_instancelist(&traindata, data);
 model = classification_algorithms[NBTREE - SELECTMAX].train_algorithm(&traindata, NULL, parameters);
 *nodecount = decisiontree_node_count((Decisionnodeptr) model);
 accumulate_instances_tree(&testdata, (Decisionnodeptr) model);
 if (data_size(testdata) != (2 * n))
   printf("Test data is not correctly accumulated %d %d\n", data_size(testdata), 2 * n);
 tmp = testdata;
 misclassified1 = 0;
 misclassified2 = 0;
 while (tmp)
  {
   realclassno = give_classno(tmp);
   predicted = classification_algorithms[NBTREE - SELECTMAX].test_algorithm(tmp, model, parameters, pos);
   if (realclassno != predicted.classno && !(tmp->isexception))
     misclassified1++;
   if (realclassno == predicted.classno && tmp->isexception)
     misclassified2++; 
   tmp = tmp->next;
  }
 deallocate(testdata);
 testdata = NULL;
 free_model_of_supervised_algorithm(NBTREE, model, current_dataset);
 return fabs((misclassified1 - misclassified2) / (n + 0.0));
}

/**
 * Calculates the epsilon estimates in an experiment design used for VC-dimension estimation of a decision tree with the given number of nodes.
 * @param[in] parameters Parameters of the decision tree algorithm
 * @param[in] nodecount Number of nodes of the decision tree. 
 * @param[in] d The dataset
 * @param[in] experimentcount Number of experiments in a design point
 * @param[in] averagevaluecount Average of the number of distinct values of the features in the dataset
 * @return Link list of epsilon estimates in an experiment design
 */
Vcestimateptr vc_estimates_for_a_design(void* parameters, int nodecount, Datasetptr d, int experimentcount, double averagevaluecount)
{
 /*!Last Changed 06.11.2007 added preestimation value*/
 /*!Last Changed 01.10.2006*/
 /*!Last Changed 01.04.2006*/
 int multiplier, n, k, ncount;
 double p, epsilon;
 Vcestimateptr header = NULL;
 ((Nodeboundedtree_parameterptr) parameters)->nodecount = nodecount;
 if (in_list(get_parameter_l("modelselectionmethod"), 3, MODEL_SELECTION_UNI, MODEL_SELECTION_LIN, MODEL_SELECTION_QUA))
   if (((Nodeboundedtree_parameterptr) parameters)->usediscrete)
     multiplier = decisiontree_vc_dimension_preestimation_discrete(d, (int) (nodecount * log2(averagevaluecount)));
   else
     multiplier = decisiontree_vc_dimension_preestimation(d, nodecount, get_parameter_l("modelselectionmethod") - 5);
 else
   multiplier = decisiontree_vc_dimension_preestimation(d, nodecount, MODEL_QUA);
 for (p = 1.5; p <= 30; p += 1.5)
  {
   n = (int) (multiplier * p);
   for (k = 0; k < experimentcount; k++)
    {
     epsilon = vc_single_epsilon_estimate(parameters, d, n, &ncount);
     if (ncount == nodecount)
       add_single_epsilon_estimate(&header, n, epsilon);
    }
  }
 return header;
}

/**
 * Estimates the VC-dimension of a decision tree with a given number of nodes for a given dataset using uniform experiment design.
 * @param[in] featurecount Number of features in the dataset
 * @param[in] nodecount Number of nodes in the decision tree
 * @param[in] experimentcount Number of experiments in the uniform design
 * @param[in] featuretypes The types of features in the dataset. Stored as an array of characters. featuretypes[i] is the character that represents the type of the feature i. 'R' stands for continuous features, 'D' stands for discrete features. 
 * @param[in] featurevaluecounts If some or all of the features are discrete features, this array stores the number of distinct values of those features. Stored as an array of integer. featurevaluecounts[i] is the number of distinct values of the feature i can take.
 */
void vc_estimate_uniform_of_decision_tree(int featurecount, int nodecount, int experimentcount, char* featuretypes, int* featurevaluecounts)
{
 /*!Last Changed 24.09.2006 added different feature types*/
 /*!Last Changed 03.05.2006 run for a specific featurecount and nodecount*/
 /*!Last Changed 01.04.2006*/
 /*!Last Changed 11.03.2006*/
 void* parameters;
 double vcdimension, averagecount;
 Vcestimateptr estimates;
 parameters = prepare_supervised_algorithm_parameters(NBTREE);
 ((Nodeboundedtree_parameterptr) parameters)->prunetype = NOPRUNING;
 if (featurevaluecounts[0] > 1)
   ((Nodeboundedtree_parameterptr) parameters)->usediscrete = BOOLEAN_TRUE;
 else
   ((Nodeboundedtree_parameterptr) parameters)->usediscrete = BOOLEAN_FALSE;
 current_dataset = generate_random_dataset("VC-Estimate", featurecount, featuretypes, featurevaluecounts, 2);
 averagecount = average_valuecount(current_dataset);
 estimates = vc_estimates_for_a_design(parameters, nodecount, current_dataset, experimentcount, averagecount);
 vcdimension = vc_dimension_for_a_design(estimates, UNIFORM_DESIGN, -1);
 set_default_variable("out1", vcdimension);
 deallocate_vc_estimates(estimates);
 free_dataset(current_dataset);
 free_supervised_algorithm_parameters(NBTREE, parameters);
}

/**
 * Estimates the VC-dimension of a decision tree with a given number of nodes for a given dataset using optimized experiment design.
 * @param[in] featurecount Number of features in the dataset
 * @param[in] nodecount Number of nodes in the decision tree
 * @param[in] experimentcount Number of experiments in the uniform design
 * @param[in] featuretypes The types of features in the dataset. Stored as an array of characters. featuretypes[i] is the character that represents the type of the feature i. 'R' stands for continuous features, 'D' stands for discrete features. 
 * @param[in] featurevaluecounts If some or all of the features are discrete features, this array stores the number of distinct values of those features. Stored as an array of integer. featurevaluecounts[i] is the number of distinct values of the feature i can take.
 */
void vc_estimate_optimized_of_decision_tree(int featurecount, int nodecount, int experimentcount, char* featuretypes, int* featurevaluecounts)
{
 /*!Last Changed 24.09.2006 added different feature types*/
 /*!Last Changed 03.05.2006 run for a specific featurecount and nodecount*/
 /*!Last Changed 01.04.2006*/
 int positivecount, positivenotsaturated, total, totalmove;
 void* parameters;
 double vcdimension, vcdimensionremove, msefitting, msefittingremove, averagecount;
 Vcestimateptr estimates, tmpvc, nextvc, tmpvc2, vc1, vc2, vc3, vc4, removed, added;
 parameters = prepare_supervised_algorithm_parameters(NBTREE);
 ((Nodeboundedtree_parameterptr) parameters)->prunetype = NOPRUNING; 
 if (featurevaluecounts[0] > 1)
   ((Nodeboundedtree_parameterptr) parameters)->usediscrete = BOOLEAN_TRUE;
 else
   ((Nodeboundedtree_parameterptr) parameters)->usediscrete = BOOLEAN_FALSE;
 current_dataset = generate_random_dataset("VC-Estimate", featurecount, featuretypes, featurevaluecounts, 2);
 averagecount = average_valuecount(current_dataset);
 estimates = vc_estimates_for_a_design(parameters, nodecount, current_dataset, experimentcount, averagecount);
 total = number_of_experiments(estimates);
 totalmove = 0;
 while (1)
  {
   vcdimension = vc_dimension_for_a_design(estimates, OPTIMIZED_DESIGN, -1);
   msefitting = mse_fitting_for_vc_dimension(estimates, vcdimension, -1);
   positivecount = 0;
   positivenotsaturated = 0;
   /*Calculate contributions*/
   tmpvc = estimates;
   while (tmpvc)
    {
     vcdimensionremove = vc_dimension_for_a_design(estimates, OPTIMIZED_DESIGN, tmpvc->n);
     msefittingremove = mse_fitting_for_vc_dimension(estimates, vcdimensionremove, tmpvc->n);
     tmpvc->contribution = (msefittingremove - msefitting) / tmpvc->count;
     if (tmpvc->contribution > 0)
      {
       if (tmpvc->count / (total + 0.0) < 0.25)
        {
         positivenotsaturated++;
         tmpvc->issaturated = BOOLEAN_FALSE;
        }
       else
         tmpvc->issaturated = BOOLEAN_TRUE;
       positivecount++;
      }
     else
       tmpvc->issaturated = BOOLEAN_FALSE;
     tmpvc = tmpvc->next;
    }
   if (!positivecount || !positivenotsaturated)
     break;
   /*Sort contributions*/
   tmpvc = estimates->next;
   while (tmpvc)
    {
     nextvc = tmpvc->next;
     tmpvc2 = tmpvc;
     while (tmpvc2->before && tmpvc2->contribution > tmpvc2->before->contribution)
      {
       vc1 = tmpvc2->before->before;
       vc2 = tmpvc2->before;
       vc3 = tmpvc2;
       vc4 = tmpvc2->next;
       if (vc1)
         vc1->next = vc3;
       else
         estimates = vc3;
       if (vc4)
         vc4->before = vc2;
       vc2->before = vc3;
       vc2->next = vc4;
       vc3->before = vc1;
       vc3->next = vc2;
      }
     tmpvc = nextvc;
    }
   /*Move one experiment from the worst design point to the best design point, if not succesful to the second best etc.*/
   removed = estimates;
   while (removed->next)
     removed = removed->next;
   added = estimates;
   totalmove++;
   while (added && !(move_vc_estimates(estimates, removed, added, current_dataset, parameters, nodecount, msefitting)))
     added = added->next;
   if (!added)
     break;
  }
 deallocate_vc_estimates(estimates);
 set_default_variable("out1", vcdimension);
 set_default_variable("out2", totalmove);
 free_dataset(current_dataset);
 free_supervised_algorithm_parameters(NBTREE, parameters);
}

/**
 * Tries to move one experiment from a design point to another design point. If the exchange yields a better mean square error, the change is accepted, otherwise it is rejected.
 * @param[in] header Experiment results. Header of the link list of design points.
 * @param[in] from Experiment results in a design point from which an experiment will be moved.
 * @param[in] to Experiment results in a design point to which an experiment will be moved.
 * @param[in] d The dataset
 * @param[in] parameters Parameters of the decision tree algorithm
 * @param[in] nodecount Number of nodes in the decision tree
 * @param[in] previousmse The previous mean square error
 * @return If the exchange yields an MSE which is smaller than previousmse, the exchange is accepted and the function returns SUCCESS, otherwise it returns FAILURE.
 */
int move_vc_estimates(Vcestimateptr header, Vcestimateptr from, Vcestimateptr to, Datasetptr d, void* parameters, int nodecount, double previousmse)
{
 /*!Last Changed 05.04.2006*/
 Epsilonestimateptr deleted, added;
 double epsilon, vcdimension, mse;
 int ncount;
 if (to->issaturated)
   return FAILURE;
 epsilon = vc_single_epsilon_estimate(parameters, d, to->n, &ncount);
 if (ncount == nodecount)
  {
   deleted = from->epsilonestimates;
   from->epsilonestimates = from->epsilonestimates->next;
   (from->count)--;
   added = create_epsilon_estimate(epsilon);
   added->next = to->epsilonestimates;
   to->epsilonestimates = added;
   (to->count)++;
   vcdimension = vc_dimension_for_a_design(header, OPTIMIZED_DESIGN, -1);
   mse = mse_fitting_for_vc_dimension(header, vcdimension, -1);   
   if (mse < previousmse)
    {
     safe_free(deleted);
     if (from->count == 0)
      {
       from->before->next = from->next;
       if (from->next)
         from->next->before = from->before;
       safe_free(from);
      }
     return SUCCESS;
    }
   else
    {
     (to->count)--;
     to->epsilonestimates = to->epsilonestimates->next;
     (from->count)++;
     deleted->next = from->epsilonestimates;
     from->epsilonestimates = deleted;
     safe_free(added);
     return FAILURE;
    }
  }
 else
   return FAILURE;
}

BOOLEAN is_featurelist_valid(int* featurelist, int* ruleset, int rulecount)
{
 /*!Last Changed 21.02.2008*/
 int i, j = 0, k, available[10], fno;
 for (i = 0; i < rulecount; i++)
  {
   if (ruleset[i] > 1)
    {
     for (k = 0; k < ruleset[i]; k++)
       available[k] = BOOLEAN_FALSE;
     for (k = 0; k < ruleset[i]; k++)
      {
       fno = featurelist[j + k] / 10;
       if (available[fno])
         return BOOLEAN_FALSE;
       available[fno] = BOOLEAN_TRUE;
      }
    }
   j += ruleset[i];
  }
 return BOOLEAN_TRUE;
}

BOOLEAN can_ruleset_classify(int* featurelist, int* ruleset, int rulecount, int* datacombination, int* classcombination, int size, int positiveclass)
{
 /*Last Changed 26.02.2008*/
 int pow2[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
 int i, j, k, fno, value, p, control, last_positive = -1, last_negative = rulecount;
 BOOLEAN covered_by_rule, covered_by_ruleset;
 for (p = 0; p < size; p++)
  {
   j = 0;
   covered_by_ruleset = BOOLEAN_FALSE;
   for (i = 0; i < rulecount; i++)
    {
     covered_by_rule = BOOLEAN_TRUE;
     for (k = 0; k < ruleset[i]; k++)
      {
       fno = featurelist[j + k] / 10;
       value = featurelist[j + k] % 2;
       control = datacombination[p] & pow2[fno];
       if ((control > 0 && value == 0) || (control == 0 && value != 0))
        {
         covered_by_rule = BOOLEAN_FALSE;
         break;
        } 
      }
     if (covered_by_rule)
      {
       if (classcombination[p] != positiveclass)
        {
         if (i < last_negative)
          {
           last_negative = i;
           if (last_positive >= last_negative)
             return BOOLEAN_FALSE;
          }
        }
       else
        {
         if (i > last_positive)
          {
           last_positive = i;
           if (last_positive >= last_negative)
             return BOOLEAN_FALSE;
          }
         covered_by_ruleset = BOOLEAN_TRUE;
        }
       break;
      }
     j += ruleset[i];
    }
   if (!covered_by_ruleset && classcombination[p] == positiveclass)
     return BOOLEAN_FALSE;
  }
 return BOOLEAN_TRUE;
}

int vc_dimension_of_ruleset(int featurecount, char* values, int randomgenerate, int randomcount)
{
 /*Last Changed 24.02.2008*/
 int i, vcdimension = 1, bound = (int) pow(2, featurecount), conditioncount = 0, notfinished, classcounts[2];
 BOOLEAN successful, classifiedsinglecombination, classifiedallcombinations;
 int *datacombination, *classiterator, *ruleset, *indexlist, *list, *featurelist, valuecount, count = 0;
 valuecount = strlen(values);
 ruleset = safemalloc(valuecount * sizeof(int), "vc_dimension_of_ruleset", 3);
 for (i = 0; i < valuecount; i++)
  {
   ruleset[i] = values[i] - '0';
   conditioncount += ruleset[i];
  }
 list = safemalloc(2 * featurecount * sizeof(int), "vc_dimension_of_ruleset", 9);
 for (i = 0; i < featurecount; i++)
  {
   list[2 * i] = 10 * i;
   list[2 * i + 1] = 10 * i + 1;
  } 
 while (vcdimension <= bound)
  {
   datacombination = first_combination(vcdimension);
   count = 0;
   successful = BOOLEAN_FALSE;
   while (BOOLEAN_TRUE)
    {
     classifiedallcombinations = BOOLEAN_TRUE;
     classiterator = first_iterator(vcdimension);
     do{
        classcounts[0] = classcounts[1] = 0;
        for (i = 0; i < vcdimension; i++)
          classcounts[classiterator[i]]++;
        if (classcounts[0] * classcounts[1] == 0)
          continue;
        featurelist = first_iterator_from_list(conditioncount, list, &indexlist);
        classifiedsinglecombination = BOOLEAN_FALSE;
        notfinished = BOOLEAN_TRUE;
        while (!classifiedsinglecombination && notfinished)
         {
          if (can_ruleset_classify(featurelist, ruleset, valuecount, datacombination, classiterator, vcdimension, 0) ||
              can_ruleset_classify(featurelist, ruleset, valuecount, datacombination, classiterator, vcdimension, 1))
           {
            classifiedsinglecombination = BOOLEAN_TRUE;
            break;
           }
          notfinished = next_iterator_from_list(featurelist, list, indexlist, conditioncount, 2 * featurecount - 1);
         }
        safe_free(indexlist);
        safe_free(featurelist);
        if (!classifiedsinglecombination)
         {
          classifiedallcombinations = BOOLEAN_FALSE;
          break;
         }
     }while(next_iterator(classiterator, vcdimension, 1));
     safe_free(classiterator);
     if (classifiedallcombinations)
      {
       successful = BOOLEAN_TRUE;
       break;
      }
     count++;
     if (randomgenerate)
       if (count < randomcount)
         random_combination(datacombination, vcdimension, bound - 1);
       else
         break;
     else
       if (!next_combination(datacombination, vcdimension, bound - 1))
         break;
   }
   safe_free(datacombination);
   if (successful)
     vcdimension++;
   else
     break;
  }
 safe_free(list);
 safe_free(ruleset);
 return vcdimension - 1;
}
