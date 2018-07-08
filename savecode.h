#ifndef __savecode_H
#define __savecode_H

enum programming_language_type{ 
      C_LANGUAGE = 0,
      CPP_LANGUAGE,
      PASCAL_LANGUAGE,
      JAVA_LANGUAGE,
      MATLAB_LANGUAGE,
      FORTRAN_LANGUAGE};

typedef enum programming_language_type PROGRAMMING_LANGUAGE_TYPE;

enum { 
      INTEGER_VARIABLE = 0,
      REAL_VARIABLE};

#include "typedef.h"
#include "classification.h"
#include "rbf.h"
#include "regression.h"
#include "mlp.h"

void  code_1d_array_constant(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size, void* values, int elementtype);
void  code_1d_array_constant_instance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* arrayname, int size, Instanceptr values);
void  code_1d_array_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int index);
char  code_1d_array_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int index);
void  code_2d_array_constant(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, double** values);
void  code_2d_array_constant_instance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* arrayname, int size1, int size2, Instanceptr values);
void  code_2d_array_constant_matrix(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, matrixptr matrixarray);
void  code_2d_array_constant_sv(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, Svm_nodeptr* values);
void  code_2d_array_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_2d_array_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2);
void  code_3d_array_constant_matrix(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, int size3, matrixptr matrixarray);
void  code_3d_array_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_3d_array_index_variable(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index1, char* index2, char* index3);
void  code_3d_array_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, int size3);
void  code_add_array_variable(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int count, ...);
void  code_add_variable(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int vartype, int count, ...);
void  code_array_index(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int index);
void  code_array_index_variable(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index);
void  code_array_initialize(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size, char* value);
void  code_break(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_calculate_distance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d);
void  code_case(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int* cases, int casecount);
void  code_compare_and_set_bound(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* comparison, char* value, char* bestbound, char* index);
void  code_condition(PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionconditionptr condition, char* st);
void  code_constant_instance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Instanceptr tmp, int index);
void  code_constant_instance_realized(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Instanceptr tmp, int index);
void  code_constant_integer(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* constname, int constvalue);
void  code_constant_real(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* constname, double constvalue);
void  code_constant_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_constant_subarray(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int size, double* values, int index);
void  code_constant_sv(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Svm_nodeptr tmp, int index);
void  code_decisionnode(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* algorithm, char* arrayname, Datasetptr d, Decisionnodeptr m, int level);
void  code_dimension_reduce(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int newdimension);
void  code_dot(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d);
void  code_else(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_end_block(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_endcase(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_endfor(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* forvariable);
void  code_endif(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_endswitch(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_endwhile(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_find_bound_of_list(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* listname, int list_length, char* initialbound, char* bestbound, char* index, char* comparison);
void  code_for(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* variable, int start, int end, int difference);
void  code_forward_propagation(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int row, int col, char* weightname, char* inputname, char* outputname, int sigmoid);
void  code_forward_propagation_regression_output(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int col, char* weightname, char* inputname, char* outputname);
void  code_function_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_function_header(FILE* testcodefile, char* functionname, int featurecount, PROGRAMMING_LANGUAGE_TYPE language);
void  code_function_return_classification(FILE* testcodefile, char* functionname, PROGRAMMING_LANGUAGE_TYPE language, int returnvalue);
void  code_function_return_regression(FILE* testcodefile, char* functionname, PROGRAMMING_LANGUAGE_TYPE language, double returnvalue);
void  code_function_return_string(FILE* testcodefile, char* functionname, PROGRAMMING_LANGUAGE_TYPE language, char* returnvalue);
void  code_if(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* operator, char* operand1, char* operand2);
void  code_if_core(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* st);
void  code_increment(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index);
void  code_indentation(FILE* testcodefile);
void  code_kernel_function(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Svm_parameter param);
void  code_normalize(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d);
void  code_realize(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int normalize);
void  code_regressionnode(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Regressionnodeptr m, int level);
void  code_rule(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithm, Decisionruleptr rule);
void  code_set_variable(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* varname, char* expression);
void  code_simple_logical_expression(PROGRAMMING_LANGUAGE_TYPE language, char* operator, char* operand1, char* operand2, char* st2);
void  code_sort_according_to_distances(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int size, int nearcount);
void  code_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_start_block(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_switch(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* expression);
void  code_variable_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language);
void  code_while(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* operator, char* operand1, char* operand2);
void  code_while_core(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* st);
void  generate_test_code_c45(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m);
void  generate_test_code_c45rules(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rulesetptr m);
void  generate_test_code_gaussian(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_gaussianptr m);
void  generate_test_code_knn(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int nearcount, Instanceptr m);
void  generate_test_code_knnreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int nearcount, Instanceptr m);
void  generate_test_code_ldaclass(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_ldaclassptr m);
void  generate_test_code_ldt(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m);
void  generate_test_code_linearreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_linearregressionptr m);
void  generate_test_code_logistic(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, double** m);
void  generate_test_code_mlpgeneric(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Mlpnetworkptr m);
void  generate_test_code_mlpgenericreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Mlpnetworkptr m);
void  generate_test_code_multildt(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m, char* algorithmname);
void  generate_test_code_multiripper(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithm, Rulesetptr m);
void  generate_test_code_naivebayes(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_naivebayesptr m);
void  generate_test_code_nearestmean(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Instanceptr m);
void  generate_test_code_onefeature(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_onefeatureptr m);
void  generate_test_code_qdaclass(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_qdaclassptr m);
void  generate_test_code_quantizereg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_quantizeregptr m);
void  generate_test_code_rbf(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rbfnetworkptr m);
void  generate_test_code_rbfreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rbfnetworkptr m);
void  generate_test_code_regtree(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Regressionnodeptr m);
void  generate_test_code_ripper(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithm, Rulesetptr m);
void  generate_test_code_rise(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rulesetptr m);
void  generate_test_code_selectaverage(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, double* m);
void  generate_test_code_selectmax(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_selectmaxptr m);
void  generate_test_code_svm(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithmname, Svm_modelptr m);
void  generate_test_code_svmreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithmname, Svm_modelptr m);
void generate_test_code_svmtree(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m);
char* test_code_file_name_with_extension(char* testcodefilename, PROGRAMMING_LANGUAGE_TYPE language);

#endif
