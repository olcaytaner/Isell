#ifndef __srm_H
#define __srm_H

void               add_single_epsilon_estimate(Vcestimateptr* header, int n, double epsilon);
BOOLEAN            can_ruleset_classify(int* featurelist, int* ruleset, int rulecount, int* datacombination, int* classcombination, int size, int positiveclass);
Epsilonestimateptr create_epsilon_estimate(double epsilon);
Vcestimateptr      create_vc_estimate(int n, Epsilonestimateptr epsilonptr);
void               deallocate_epsilon_estimates(Epsilonestimateptr header);
void               deallocate_vc_estimates(Vcestimateptr header);
BOOLEAN            is_featurelist_valid(int* featurelist, int* ruleset, int rulecount);
int                move_vc_estimates(Vcestimateptr header, Vcestimateptr from, Vcestimateptr to, Datasetptr d, void* parameters, int nodecount, double previousmse);
double             mse_fitting_for_vc_dimension(Vcestimateptr estimates, double vcdimension, int n);
int                number_of_experiments(Vcestimateptr estimates);
double             solve_vc_bound_equation(double epsilon);
double             vc_bound(double tao);
double             vc_dimension_for_a_design(Vcestimateptr estimates, DESIGN_TYPE experimenttype, int n);
int                vc_dimension_of_ruleset(int featurecount, char* values, int randomgenerate, int randomcount);
void               vc_estimate_optimized_of_decision_tree(int featurecount, int nodecount, int experimentcount, char* featuretypes, int* featurevaluecounts);
void               vc_estimate_uniform_of_decision_tree(int featurecount, int nodecount, int experimentcount, char* featuretypes, int* featurevaluecounts);
Vcestimateptr      vc_estimates_for_a_design(void* parameters, int nodecount, Datasetptr d, int experimentcount, double averagevaluecount);
double             vc_single_epsilon_estimate(void* parameters, Datasetptr d, int n, int* nodecount);

#endif
