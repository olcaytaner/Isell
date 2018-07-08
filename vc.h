#ifndef __vc_H
#define __vc_H

#define VC_GENERAL 0
#define VC_INDEPTH 1

double decisiontree_vc_dimension_continuous_recursive(Datasetptr current_dataset, Decisionnodeptr rootnode);
double decisiontree_vc_dimension_discrete_old(Datasetptr d, Decisionnodeptr rootnode);
double decisiontree_vc_dimension_discrete_recursive(Datasetptr current_dataset, int d, Decisionnodeptr rootnode);
double decisiontree_vc_dimension_discrete_recursive_old(int d, Decisionnodeptr rootnode);
double decisiontree_vc_dimension_linear1(double* testdata);
double decisiontree_vc_dimension_linear2(double* testdata);
int    decisiontree_vc_dimension_preestimation(Datasetptr d, int nodecount, MODEL_TYPE nodetype);
int    decisiontree_vc_dimension_preestimation_discrete(Datasetptr d, int nodecount);
double decisiontree_vc_dimension_quadratic1(double* testdata);
double decisiontree_vc_dimension_quadratic2(double* testdata);
int    decisiontree_vc_dimension_recursive(Datasetptr current_dataset, Decisionnodeptr root);
int    decisiontree_vc_dimension_wrt_level(Datasetptr d, Decisionnodeptr rootnode, int nodetype);
int    decisiontree_vc_dimension_wrt_nodetype(Datasetptr d, int nodecount, MODEL_TYPE nodetype);
double decisiontree_vc_dimension_univariate1(double* testdata);
double decisiontree_vc_dimension_univariate2(double* testdata);
int    omnivariatetree_vc_dimension(Datasetptr d, Decisionnodeptr node);
int    ruleset_vc_dimension(Datasetptr current_dataset, Rulesetptr ruleset);
int    ruleset_vc_dimension_continuous(Datasetptr current_dataset, Rulesetptr ruleset);
int    ruleset_vc_dimension_discrete(Datasetptr current_dataset, Rulesetptr ruleset);
int    ruleset_vc_dimension_discrete_recursive(int* condition_counts, int k, int d);

#endif
