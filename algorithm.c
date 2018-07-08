#include <limits.h>
#include "algorithm.h"
#include "c45rules.h"
#include "decisiontree.h"
#include "eps.h"
#include "hmm.h"
#include "instance.h"
#include "instancelist.h"
#include "messages.h"
#include "lang.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "mlp.h"
#include "parameter.h"
#include "rbf.h"
#include "regressionrule.h"
#include "regressiontree.h"
#include "rule.h"
#include "savecode.h"
#include "svmprepare.h"
#include "svmsimplex.h"
#include "xmlparser.h"

extern Datasetptr current_dataset;
extern int indentation;
extern Instanceptr cvdata;
extern int totalrun;

/*Algorithms*/
Algorithm classification_algorithms[CLASSIFICATION_ALGORITHM_COUNT] = {{train_select_max, test_select_max, "SELECTMAX", NO, NO, NO},
 {train_knn, test_knn, "KNN", NO, YES, NO},
 {train_c45, test_c45, "C45", NO, NO, NO},
 {train_nearestmean, test_nearestmean, "NEARESTMEAN", NO, YES, NO},
 {train_ldaclass, test_ldaclass, "LDACLASS", YES, YES, NO},
 {train_ldt, test_ldt, "LDT", NO, YES, NO},
 {train_irep, test_irep, "IREP", NO, NO, NO},
 {train_irep2, test_irep2, "IREP2", NO, NO, NO},
 {train_ripper, test_ripper, "RIPPER", NO, NO, NO},
 {train_c45rules, test_c45rules, "C45RULES", NO, NO, NO},
 {train_multiripper, test_multiripper, "MULTIRIPPER", YES, YES, NO},
 {train_hybridripper, test_hybridripper, "HYBRIDRIPPER", YES, YES, NO},
 {train_mlpgeneric, test_mlpgeneric, "MLPGENERIC", YES, YES, YES},
 {train_rexripper, test_rexripper, "REXRIPPER", NO, NO, NO},
 {train_multildt, test_multildt, "MULTILDT", YES, YES, NO},
 {train_omnildt, test_omnildt, "OMNILDT", YES, YES, NO},
 {train_logistic, test_logistic, "LOGISTIC", YES, YES, NO},
 {train_svm, test_svm, "SVM", YES, YES, NO},
 {train_simplexsvm, test_simplexsvm, "NUSVM", YES, YES, NO},
 {train_svmtree, test_svmtree, "SVMTREE", NO, YES, NO},
 {train_gaussian, test_gaussian, "GAUSSIAN", YES, YES, NO},
 {train_nodeboundedtree, test_nodeboundedtree, "NBTREE", YES, YES, NO},
 {train_hmm, test_hmm, "HMM", NO, NO, YES},
 {train_dnc, test_dnc, "DNC", YES, YES, NO},
 {train_qdaclass, test_qdaclass, "QDACLASS", YES, YES, NO},
 {train_naivebayes, test_naivebayes, "NAIVEBAYES", NO, NO, NO},
 {train_rbf, test_rbf, "RBF", YES, YES, YES},
 {train_randomforest, test_randomforest, "RANDOMFOREST", NO, NO, NO},
 {train_rise, test_rise, "RISE", NO, NO, NO},
 {train_divs, test_divs, "DIVS", NO, NO, YES},
 {train_cn2, test_cn2, "CN2", NO, NO, NO},
 {train_lerils, test_lerils, "LERILS", NO, NO, NO},
 {train_part, test_part, "PART", NO, NO, YES},
 {train_softtree, test_softtree, "SOFTTREE", YES, YES, YES},
 {train_cart, test_cart, "CART", YES, YES, YES},
 {train_ktree, test_ktree, "KTREE", NO, NO, NO},
 {train_kforest, test_kforest, "KFOREST", NO, NO, NO},
 {train_c45stump, test_c45stump, "C45STUMP", NO, NO, NO},
 {train_ldtstump, test_ldtstump, "LDTSTUMP", NO, YES, NO},
 {train_multildtstump, test_multildtstump, "MULTILDTSTUMP", YES, YES, NO},
 {train_softstump, test_softtree, "SOFTTREE", YES, YES, NO},
};

/*Algorithms*/
Algorithm regression_algorithms[REGRESSION_ALGORITHM_COUNT] = {{train_onefeature, test_onefeature, "ONEFEATURE", NO, NO, NO},
 {train_knnreg, test_knnreg, "KNNREG", NO, YES, NO},
 {train_regressiontree, test_regressiontree, "REGTREE", YES, YES, NO},
 {train_mlpgenericreg, test_mlpgenericreg, "MLPGENERICREG", YES, YES, YES},
 {train_selectaverage, test_selectaverage, "SELECTAVERAGE", NO, NO, NO},
 {train_linearregression, test_linearregression, "LINEARREG", YES, NO, NO},
 {train_quantizereg, test_quantizereg, "QUANTIZEREG", NO, NO, NO},
 {train_regrule, test_regrule, "REGRULE", YES, YES, NO},
 {train_svmreg, test_svmreg, "SVMREG", YES, YES, NO},
 {train_simplexsvmreg, test_simplexsvmreg, "NUSVMREG", YES, YES, NO},
 {train_dncreg, test_dncreg, "DNCREG", YES, YES, NO},
 {train_rbfreg, test_rbfreg, "RBFREG", YES, YES, YES},
 {train_gprocessreg, test_gprocessreg, "GPROCESSREG", YES, YES, NO},
 {train_svmregtree, test_svmregtree, "SVMREGTREE", NO, YES, NO},
 {train_softregressiontree, test_softregressiontree, "SOFTREGTREE", YES, YES, YES},
 {train_regstump, test_regstump, "REGSTUMP", YES, YES, NO},
 {train_softregstump, test_softregstump, "SOFTREGSTUMP", YES, YES, NO}
};

SUPERVISED_ALGORITHM_TYPE get_algorithm_type(int algorithm)
{
 int i;
 for (i = 0; i < CLASSIFICATION_ALGORITHM_COUNT; i++)
   if (algorithm == SELECTMAX + i)
     return CLASSIFICATION;
 for (i = 0; i < REGRESSION_ALGORITHM_COUNT; i++)
   if (algorithm == ONEFEATURE + i)
     return REGRESSION;
 return UNDEFINED_ALGORITHM;
}

YES_NO must_realize(int algorithm)
{
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
   return classification_algorithms[algorithm - SELECTMAX].mustrealize;
 else
   return regression_algorithms[algorithm - ONEFEATURE].mustrealize;
}

YES_NO must_normalize(int algorithm)
{
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
   return classification_algorithms[algorithm - SELECTMAX].mustnormalize;
 else
   return regression_algorithms[algorithm - ONEFEATURE].mustnormalize;
}

YES_NO cvdata_needed(int algorithm)
{
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
   return classification_algorithms[algorithm - SELECTMAX].cvdata_needed;
 else
   return regression_algorithms[algorithm - ONEFEATURE].cvdata_needed;
}

char* get_algorithm_name(int algorithm)
{
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
   return classification_algorithms[algorithm - SELECTMAX].name;
 else
   return regression_algorithms[algorithm - ONEFEATURE].name;
}

/**
 * Checks if the algorithm matches current dataset
 * @param[in] algorithm Index of the algorithm
 * @param[in] current_dataset current dataset
 * @return 0 if the dataset is a classification dataset but the algorithm is not a classification algorithm or if the dataset is a regression dataset but the algorithm is not a regression algorithm, 1 otherwise.
 */
int check_supervised_algorithm_type(int algorithm, Datasetptr current_dataset)
{
 /*!Last Changed 05.03.2006*/
 if (in_list(algorithm, 1, HMM) && current_dataset->storetype != STORE_SEQUENTIAL)
  {
   printf(m1471);
   return 0;
  }
 if (current_dataset->storetype == STORE_KERNEL && !in_list(algorithm, 6, SVM, SIMPLEXSVM, SVMREG, SIMPLEXSVMREG, KNN, KNNREG))
  {
   printf(m1472);
   return 0;
  }
 if (get_algorithm_type(algorithm) == CLASSIFICATION && current_dataset->classno == 0)
  {
   printf(m1132);
   return 0;
  }
 if (get_algorithm_type(algorithm) == REGRESSION && current_dataset->classno != 0)
  {
   printf(m1133);
   return 0;
  }
 return 1;
}

/**
 * Returns total number of parameters of the algorithm considering the output model of the algorithm
 * @param[in] algorithm Index of the algorithm
 * @param[in] model Model of the algorithm
 * @param[in] parameters Parameters of the algorithm
 * @param[in] current_dataset Current dataset
 * @return Total complexity (number of parameters) of the supervised algorithm
 */
int complexity_of_supervised_algorithm(int algorithm, void* model, void* parameters, Datasetptr current_dataset)
{
 /*!Last Changed 19.08.2007 added RBF and RBFREG algorithms*/
 /*!Last Changed 11.08.2007 added NAIVEBAYES algorithm*/
 /*!Last Changed 02.04.2007 added HMM algorithm*/
 /*!Last Changed 18.12.2005*/
 Ruleset r; 
 Regressionruleset regr;
 Quantizereg_parameterptr paramqua; 
 Model_ldaclassptr mlda;
 Decisionnodeptr* forest;
 Decisionnodeptr dtrootnode;
 Regressionnodeptr rtrootnode;
 Mlpparameters mlpparams;
 Rbf_parameter rbfparams;
 Svm_modelptr msvm;
 Hmmptr* hmmarray;
 Model_divsptr mdivs;
 int vectsize, i, j, numparameters, nSV, treecount, complexity;
 switch (algorithm)
  {
   case SELECTMAX     :return 1;/*Classno*/
   case ONEFEATURE    :return 4;/*Split feature, point, left and right mean*/
   case NEARESTMEAN   :return (current_dataset->featurecount - 1) * current_dataset->classno; /*Mean vectors for each class. Discrete features are NOT converted*/
   case GAUSSIAN      :return current_dataset->multifeaturecount * current_dataset->classno; /*Mean and vector and single variance for each class. Discrete features are converted.*/
   case SELECTAVERAGE :return 1;/*Mean regression value of trainset*/
   case LINEARREG     :return current_dataset->multifeaturecount; /*Weights for each feature and b. Discrete features are converted.*/
   case QUANTIZEREG   :paramqua = (Quantizereg_parameterptr) parameters;
                       if (current_dataset->featurecount > 2)
                         return 5 + paramqua->partitioncount * paramqua->partitioncount; /*minx, binx, miny, biny, partitioncount*/
                       else
                         return 3 + paramqua->partitioncount; /*minx, binx, partitioncount*/
   case KNN           :
   case KNNREG        :return data_size((Instanceptr)model) * current_dataset->featurecount + 1;/*number of data times number of features + 1 memory space to store k. Discrete features are not converted.*/
   case GPROCESSREG   :return ((Model_gprocessregressionptr)model)->size * current_dataset->multifeaturecount;/*N data samples + weights array of size N*/
   case LDT           :
   case C45           :
   case KTREE         :dtrootnode = (Decisionnodeptr)model;
                       return decisiontree_node_count(dtrootnode) * 2 + decisiontree_leaf_count(dtrootnode);/*For each node, univariate split, for each leaf, class label*/
   case RANDOMFOREST  :
   case KFOREST       :forest = (Decisionnodeptr*) model;
                       treecount = get_parameter_i("forestsize");
                       complexity = 0;
                       for (i = 0; i < treecount; i++)
                        {
                         dtrootnode = forest[i];
                         complexity += decisiontree_node_count(dtrootnode) * 2 + decisiontree_leaf_count(dtrootnode);
                        }
                       return complexity;
                       break;
   case SVMTREE       :forest = (Decisionnodeptr*) model;
                       if (get_parameter_l("multiclasstype") == ONE_VS_ONE)
                         treecount = current_dataset->classno * (current_dataset->classno - 1) / 2;
                       else
                         treecount = current_dataset->classno;
                       complexity = 0;
                       for (i = 0; i < treecount; i++)
                        {
                         dtrootnode = forest[i];
                         complexity += decisiontree_node_count(dtrootnode) * 2 + decisiontree_leaf_count(dtrootnode);
                        }
                       return complexity;
   case QDACLASS      :return current_dataset->classno * ((current_dataset->multifeaturecount - 1) * (current_dataset->multifeaturecount) / 2 + (current_dataset->multifeaturecount - 1) + 1);
   case LDACLASS      :mlda = (Model_ldaclassptr)model;
                       vectsize = (mlda->newfeaturecount - 1) * (current_dataset->multifeaturecount - 1);/*Size of eigenvectors plus*/
                       return vectsize + current_dataset->classno * mlda->newfeaturecount; /*Weight vector and bias for each class*/
   case NAIVEBAYES    :numparameters = current_dataset->classno;
                       for (i = 0; i < current_dataset->featurecount; i++)
                         switch (current_dataset->features[i].type)
                          {
                           case INTEGER:
                           case    REAL:numparameters += current_dataset->classno + 1;
                                        break;
                           case  STRING:numparameters += current_dataset->classno * current_dataset->features[i].valuecount;
                                        break;
                          }
                       return numparameters;
   case LOGISTIC      :return current_dataset->multifeaturecount * current_dataset->classno; /*Mean vectors and bias for each class. Discrete features are converted*/
   case IREP          :
   case IREP2         :
   case RIPPER        :
   case C45RULES      :
   case MULTIRIPPER   :
   case HYBRIDRIPPER  :
   case REXRIPPER     :
   case RISE          :
   case CN2           :
   case LERILS        :
   case PART          :r = *((Ruleset*) model);
                       return ruleset_complexity(r) + r.rulecount + 1; /*Complexity of conditions + classno of each rule + default class*/
   case DIVS          :mdivs = (Model_divsptr) model;
                       numparameters = mdivs->datacount; /*Classno of each ruleset*/
                       for (i = 0; i < mdivs->datacount; i++)
                         for (j = 0; j < mdivs->rulecounts[i]; j++)
                           numparameters += mdivs->conditioncounts[i][j]; /*Complexity of conditions*/
                       return numparameters;
   case RBF           :
   case RBFREG        :rbfparams = *((Rbf_parameterptr) parameters);
                       return rbfparams.input * rbfparams.hidden + rbfparams.hidden + rbfparams.output * (rbfparams.hidden + 1); /*m, s, w*/
   case MLPGENERIC    :
   case MLPGENERICREG :
   case DNC           :
   case DNCREG        :mlpparams = *((Mlpparameters*) parameters);
                       if (mlpparams.layernum != 0)
                        {
                         numparameters = (mlpparams.dimnum + 1) * mlpparams.hidden[0];
                         for (i = 0; i < mlpparams.layernum - 1; i++)
                           numparameters += (mlpparams.hidden[i] + 1) * mlpparams.hidden[i + 1]; /*weights from input to hiddens*/
                         numparameters += (mlpparams.hidden[i] + 1) * mlpparams.classnum; /*weights from last hidden to output*/
                        }
                       else
                         numparameters = (mlpparams.dimnum + 1) * mlpparams.classnum;
                       return numparameters;
   case SVMREGTREE    :
   case REGTREE       :
   case SOFTREGTREE   :rtrootnode = (Regressionnodeptr)model;
                       return complexity_of_regression_tree(rtrootnode);
   case REGRULE       :regr = *((Regressionruleset*) model);
                       return complexity_of_regression_ruleset(regr);
   case MULTILDT      :
   case OMNILDT       :
   case NBTREE        :
   case SOFTTREE      :
   case CART          :dtrootnode = (Decisionnodeptr)model;
                       return decisiontree_complexity_count(dtrootnode) + decisiontree_leaf_count(dtrootnode); /*Include class at the leaves*/
   case SVM           :nSV = 0;
                       msvm = (Svm_modelptr) model;
                       for (i = 0; i < msvm->nr_class; i++)
                         nSV += msvm->nSV[i];
                       return nSV * (current_dataset->multifeaturecount - 1);/*number of support vectors * number of features (realized)*/
   case SVMREG        :msvm = (Svm_modelptr) model;
                       nSV = msvm->l;
                       return nSV * (current_dataset->multifeaturecount - 1);/*number of support vectors * number of features (realized)*/
   case SIMPLEXSVM    :
   case SIMPLEXSVMREG :return number_of_support_vectors(model) * (current_dataset->multifeaturecount - 1);
   case HMM           :hmmarray = (Hmmptr*) model;
                       numparameters = 0;
                       for (i = 0; i < current_dataset->classno; i++)
                         numparameters += complexity_of_hmm(hmmarray[i]);
                       return numparameters;
  }
 return 0;
}

/**
 * Returns the index of the algorithm given its name
 * @param[in] algname Name of the supervised algorithm
 * @return Index of the algorithm
 */
int get_supervised_algorithm_index(char* algname)
{
 /*!Last Changed 11.03.2006*/
 int i;
 for (i = 0; i < CLASSIFICATION_ALGORITHM_COUNT; i++)
   if (strIcmp(algname, classification_algorithms[SELECTMAX + i].name) == 0)
     return SELECTMAX + i;
 for (i = 0; i < REGRESSION_ALGORITHM_COUNT; i++)
   if (strIcmp(algname, regression_algorithms[ONEFEATURE + i].name) == 0)
     return ONEFEATURE + i;
 return -1;
}

/**
 * Calls corresponding function with respect to the algorithm to generate test code file for testing trained algorithms on other datasets using testcode file
 * @param[in] testcodefile Name of the output file
 * @param[in] algorithm Index of the algorithm
 * @param[in] model Output model of the algorithm
 * @param[in] parameters Parameters of the algorithm
 * @param[in] language Output language, can be C, CPP, Java, Ada, Fortran, Matlab etc.
 * @param[in] current_dataset Current dataset
 */
void generate_test_code(FILE* testcodefile, int algorithm, void* model, void* parameters, int language, Datasetptr current_dataset)
{
 /*!Last Changed 21.08.2007 added RBF and RBFREG*/
 /*!Last Changed 07.08.2007 added QDACLASS*/
 /*!Last Changed 15.05.2006*/
 indentation = 0;
 switch (algorithm)
  {
   case SELECTMAX    :generate_test_code_selectmax(testcodefile, language, current_dataset, (Model_selectmaxptr) model);
                      break;
   case ONEFEATURE   :generate_test_code_onefeature(testcodefile, language, current_dataset, (Model_onefeatureptr) model);
                      break;
   case SELECTAVERAGE:generate_test_code_selectaverage(testcodefile, language, current_dataset, (double*) model);
                      break;
   case GAUSSIAN     :generate_test_code_gaussian(testcodefile, language, current_dataset, (Model_gaussianptr) model);
                      break;
   case NEARESTMEAN  :generate_test_code_nearestmean(testcodefile, language, current_dataset, (Instanceptr) model);
                      break;
   case LINEARREG    :generate_test_code_linearreg(testcodefile, language, current_dataset, (Model_linearregressionptr) model);
                      break;                      
   case QUANTIZEREG  :generate_test_code_quantizereg(testcodefile, language, current_dataset, (Model_quantizeregptr) model);
                      break;                      
   case KNN          :generate_test_code_knn(testcodefile, language, current_dataset, ((Knn_parameterptr) parameters)->nearcount, (Instanceptr) model);
                      break;
   case KNNREG       :generate_test_code_knnreg(testcodefile, language, current_dataset, ((Knn_parameterptr) parameters)->nearcount, (Instanceptr) model);
                      break;
   case C45          :generate_test_code_c45(testcodefile, language, current_dataset, (Decisionnodeptr) model);
                      break;
   case LDT          :generate_test_code_ldt(testcodefile, language, current_dataset, (Decisionnodeptr) model);
                      break;
   case SVMTREE      :generate_test_code_svmtree(testcodefile, language, current_dataset, (Decisionnodeptr) model);
                      break;
   case SVMREGTREE   :
   case REGTREE      :generate_test_code_regtree(testcodefile, language, current_dataset, (Regressionnodeptr) model);
                      break;
   case IREP         :generate_test_code_ripper(testcodefile, language, current_dataset, "Irep", (Rulesetptr) model);
                      break;
   case IREP2        :generate_test_code_ripper(testcodefile, language, current_dataset, "Irep2", (Rulesetptr) model);
                      break;
   case RIPPER       :generate_test_code_ripper(testcodefile, language, current_dataset, "Ripper", (Rulesetptr) model);
                      break;
   case CN2          :generate_test_code_ripper(testcodefile, language, current_dataset, "Cn2", (Rulesetptr) model);
                      break;
   case LERILS       :generate_test_code_ripper(testcodefile, language, current_dataset, "Lerils", (Rulesetptr) model);
                      break;
   case PART         :generate_test_code_ripper(testcodefile, language, current_dataset, "Part", (Rulesetptr) model);
                      break;
   case RISE         :generate_test_code_rise(testcodefile, language, current_dataset, (Rulesetptr) model);
                      break;
   case MULTIRIPPER  :generate_test_code_multiripper(testcodefile, language, current_dataset, "Multiripper", (Rulesetptr) model);
                      break;
   case C45RULES     :generate_test_code_c45rules(testcodefile, language, current_dataset, (Rulesetptr) model);
                      break;
   case LDACLASS     :generate_test_code_ldaclass(testcodefile, language, current_dataset, (Model_ldaclassptr) model);
                      break;
   case NAIVEBAYES   :generate_test_code_naivebayes(testcodefile, language, current_dataset, (Model_naivebayesptr) model);
                      break;
   case QDACLASS     :generate_test_code_qdaclass(testcodefile, language, current_dataset, (Model_qdaclassptr) model);
                      break;
   case LOGISTIC     :generate_test_code_logistic(testcodefile, language, current_dataset, (double**) model);
                      break;
   case RBF          :generate_test_code_rbf(testcodefile, language, current_dataset, (Rbfnetworkptr) model);
                      break;
   case RBFREG       :generate_test_code_rbfreg(testcodefile, language, current_dataset, (Rbfnetworkptr) model);
                      break;
   case MLPGENERIC   :
   case DNC          :generate_test_code_mlpgeneric(testcodefile, language, current_dataset, (Mlpnetworkptr) model);
                      break;
   case MLPGENERICREG:
   case DNCREG       :generate_test_code_mlpgenericreg(testcodefile, language, current_dataset, (Mlpnetworkptr) model);
                      break;
   case MULTILDT     :generate_test_code_multildt(testcodefile, language, current_dataset, (Decisionnodeptr) model, "multildt");
                      break;
   case NBTREE       :generate_test_code_multildt(testcodefile, language, current_dataset, (Decisionnodeptr) model, "nodeboundedtree");
                      break;
   case SVM          :generate_test_code_svm(testcodefile, language, current_dataset, "Svm", (Svm_modelptr) model);
                      break;
   case SVMREG       :generate_test_code_svmreg(testcodefile, language, current_dataset, "Svmreg", (Svm_modelptr) model);
                      break;
  }
}

/**
 * Frees memory allocated to the parameters of the algorithm
 * @param[in] algorithm Index of the algorithm
 * @param[in] parameters Parameters of the algorithm
 */
void free_supervised_algorithm_parameters(int algorithm, void* parameters)
{
 /*!Last Changed 02.04.2007 added HMM algorithm*/
 /*!Last Changed 14.08.2005*/
 switch (algorithm)
  {
   case RIPPER        :
   case IREP2         :
   case IREP          :
   case MULTIRIPPER   :
   case HYBRIDRIPPER  :
   case REXRIPPER     :
   case LDACLASS      :
   case KNN           :
   case KNNREG        :
   case GPROCESSREG   :
   case REGTREE       :
   case SVMREGTREE    :
   case SOFTREGTREE   :
   case SOFTTREE      :
   case LDT           :
   case NBTREE        :
   case RBF           :
   case RBFREG        :
   case MLPGENERIC    :
   case MLPGENERICREG :
   case DNC           :
   case DNCREG        :
   case MULTILDT      :
   case OMNILDT       :
   case QUANTIZEREG   :
   case REGRULE       :
   case SVM           :
   case SIMPLEXSVM    :
   case SVMREG        :
   case SIMPLEXSVMREG :
   case HMM           :
   case LOGISTIC      :
   case SVMTREE       :
   case RANDOMFOREST  :
   case KFOREST       :
   case C45           :
   case CN2           :
   case LERILS        :
   case CART          :safe_free(parameters);
                       break;
  }
}

/**
 * Frees memory allocated to the model of the algorithm
 * @param[in] algorithm Index of the algorithm
 * @param[in] model Output model of the algorithm
 * @param[in] current_dataset Current dataset
 */
void free_model_of_supervised_algorithm(int algorithm, void* model, Datasetptr current_dataset)
{
 /*!Last Changed 19.08.2007 added RBF and RBFREG algorithms*/
 /*!Last Changed 11.08.2007 added NAIVEBAYES algorithm*/
 /*!Last Changed 05.08.2007 added QDACLASS algorithm*/
 /*!Last Changed 02.04.2007 added HMM algorithm*/
 /*!Last Changed 14.08.2005*/
 int i, j, treecount;
 Model_ldaclassptr mlda;
 Model_qdaclassptr mqda;
 Model_quantizeregptr mqua;
 Model_gaussianptr mgau;
 Model_selectmaxptr msel;
 Model_naivebayesptr mnaive;
 Model_divsptr mdivs;
 Model_riseptr mrise;
 Hmmptr* mhmm;
 Decisionnodeptr* forest;
 switch (algorithm)
  {
   case ONEFEATURE    :
   case SELECTAVERAGE :safe_free(model);
                       break;
   case SELECTMAX     :msel = (Model_selectmaxptr)model;
                       safe_free(msel->priors);
                       safe_free(msel);
                       break;
   case LOGISTIC      :free_2d((void**)((double**)model), current_dataset->classno);
                       break;
   case LINEARREG     :matrix_free(((Model_linearregressionptr) model)->w);
                       safe_free(model);
                       break;
   case NEARESTMEAN   :deallocate((Instanceptr)model);
                       break;
   case GAUSSIAN      :mgau = (Model_gaussianptr)model;
                       deallocate(mgau->mean);
                       safe_free(mgau->variance);
                       safe_free(mgau->priors);
                       safe_free(mgau);
                       break;
   case LDT           :
   case C45           :
   case MULTILDT      :
   case OMNILDT       :
   case NBTREE        :
   case SOFTTREE      :
   case CART          :
   case KTREE         :
   case C45STUMP      :
   case LDTSTUMP      :
   case MULTILDTSTUMP :
   case SOFTSTUMP     :deallocate_tree((Decisionnodeptr)model);
                       safe_free((Decisionnodeptr)model);
                       break;
   case SVMTREE       :forest = (Decisionnodeptr*) model;
                       if (get_parameter_l("multiclasstype") == ONE_VS_ONE)
                        treecount = (current_dataset->classno * (current_dataset->classno - 1))/ 2;
                       else
                         treecount = current_dataset->classno;
                       for (i = 0; i < treecount; i++)
                        {
                         deallocate_tree(forest[i]);
                         safe_free(forest[i]);
                        }
                       safe_free(forest);
                       break;
   case SVMREGTREE    :
   case REGTREE       :
   case SOFTREGTREE   :
   case REGSTUMP      :
   case SOFTREGSTUMP  :deallocate_regression_tree((Regressionnodeptr)model);
                       cvdata = NULL;
                       safe_free((Regressionnodeptr)model);
                       break;
   case REGRULE       :free_regression_ruleset(*((Regressionrulesetptr) model));
                       safe_free(model);
                       break;
   case NAIVEBAYES    :mnaive = (Model_naivebayesptr) model;
                       safe_free(mnaive->priors);
                       safe_free(mnaive->s);
                       free_2d((void **) mnaive->m, current_dataset->classno);
                       for (i = 0; i < current_dataset->classno; i++)
                        {
                         for (j = 0; j < current_dataset->featurecount; j++)
                           if (current_dataset->features[j].type == STRING)
                             safe_free(mnaive->p[i][j]);
                         safe_free(mnaive->p[i]);
                        }
                       safe_free(mnaive->p);
                       safe_free(mnaive);
                       break;
   case QDACLASS      :mqda = (Model_qdaclassptr) model;
                       safe_free(mqda->w0);
                       for (i = 0; i < current_dataset->classno; i++)
                         matrix_free(mqda->W[i]);
                       safe_free(mqda->W);
                       for (i = 0; i < current_dataset->classno; i++)
                         matrix_free(mqda->w[i]);
                       safe_free(mqda->w);
                       safe_free(mqda->priors);
                       safe_free(mqda);
                       break;
   case LDACLASS      :mlda = (Model_ldaclassptr) model;
                       safe_free(mlda->eigenvalues);
                       safe_free(mlda->w0);
                       matrix_free(mlda->eigenvectors);
                       for (i = 0; i < current_dataset->classno; i++)
                         matrix_free(mlda->w[i]);
                       safe_free(mlda->w);
                       safe_free(mlda);
                       break;
   case QUANTIZEREG   :mqua = (Model_quantizeregptr) model;
                       if (current_dataset->featurecount > 2)
                        {
                         free_2d((void**)(mqua->meanstwo), mqua->partitioncount);
                         matrix_free(mqua->eigenvectors);
                        }
                       else
                         safe_free(mqua->meansone);                      
                       safe_free(mqua);
                       break;
   case RISE          :mrise = (Model_riseptr) model;
                       free_ruleset(mrise->rules);
                       safe_free(mrise->counts);
                       for (i = 0; i < current_dataset->featurecount; i++)
                         if (current_dataset->features[i].type == STRING)
                           matrix_free(mrise->svdm[i]);
                       safe_free(mrise->svdm);
                       safe_free(mrise);
                       break;
   case IREP          :
   case IREP2         :
   case RIPPER        :
   case MULTIRIPPER   :
   case HYBRIDRIPPER  :
   case CN2           :
   case LERILS        :
   case PART          :free_ruleset(*((Ruleset*)model));
                       safe_free(model);
                       break;
   case DIVS          :mdivs = (Model_divsptr)model;
                       for (i = 0; i < mdivs->datacount; i++)
                        {
                         for (j = 0; j < mdivs->rulecounts[i]; j++)
                           safe_free(mdivs->hypotheses[i][j]);
                         safe_free(mdivs->hypotheses[i]);
                         safe_free(mdivs->conditioncounts[i]);                        
                        }
                       safe_free(mdivs->hypotheses);
                       safe_free(mdivs->conditioncounts);
                       safe_free(mdivs->classno);
                       safe_free(mdivs->rulecounts);
                       safe_free(mdivs);
                       break;
   case REXRIPPER     :free_ruleset(((Model_rexripperptr)model)->r);
                       if (((Model_rexripperptr)model)->exceptiondata)
                         deallocate(((Model_rexripperptr)model)->exceptiondata);
                       safe_free(model);
                       break;
   case C45RULES      :free_ruleset(*((Ruleset*)model));
                       safe_free(model);
                       break;
   case RBF           :
   case RBFREG        :free_rbfnetwork((Rbfnetworkptr) model);
                       break;
   case MLPGENERIC    :
   case MLPGENERICREG :
   case DNC           :
   case DNCREG        :free_mlpnnetwork((Mlpnetworkptr) model);
                       safe_free(model);
                       break;
   case GPROCESSREG   :safe_free(((Model_gprocessregressionptr) model)->weights);
                       safe_free(model);
                       break;
   case SVM           :
   case SVMREG        :free_svm_problem(((Svm_modelptr)model)->prob);
                       free_svm_model((Svm_modelptr)model);
                       break;
   case SIMPLEXSVM    :if (current_dataset->classno == 2)
                         free_svm_binary_model((Svm_binary_modelptr) model);
                       else
                         free_svm_simplex_model((Svm_simplex_modelptr) model);
                       break;
   case SIMPLEXSVMREG :free_svm_regression_model((Svm_regression_modelptr) model);
                       break;
   case HMM           :mhmm = (Hmmptr*) model;
                       for (i = 0; i < current_dataset->classno; i++)
                         free_hmm(mhmm[i]);
                       safe_free(mhmm);
                       break;
   case RANDOMFOREST  :
   case KFOREST       :forest = (Decisionnodeptr*) model;
                       treecount = get_parameter_i("forestsize");
                       for (i = 0; i < treecount; i++)
                        {
                         deallocate_tree(forest[i]);
                         safe_free(forest[i]);
                        }
                       safe_free(forest);
                       break;
  }
}

void* tune_hyperparameters_of_the_supervised_algorithm(int algorithm, Instanceptr* traindata, Instanceptr* cvdata, Instanceptr* tunedata, void* parameters)
{
 /*!Last Changed 12.03.2009*/
 Xmldocumentptr doc;
 Xmlelementptr element, child;
 void* bestmodel = NULL;
 void* model;
 int valuecount, i, classno, bestnear;
 Knn_parameterptr pknn;
 C45_parameterptr pc45;
 Multildt_parameterptr pmultildt;
 Logistic_parameterptr plogistic;
 Rbf_parameterptr prbf;
 Mlpparametersptr pmlp;
 Svm_parameterptr psvm;
 Svm_simplex_parameterptr ssvm;
 Randomforest_parameterptr prf;
 Kforest_parameterptr pkf;
 double* posteriors = NULL;
 Instanceptr test, next;
 Prediction predicted;
 double performance, bestperformance, real;
 if (in_list(algorithm, 18, KNN, KNNREG, LOGISTIC, RBF, RBFREG, MLPGENERIC, MLPGENERICREG, DNC, DNCREG, SVM, SIMPLEXSVM, SVMREG, SIMPLEXSVMREG, SVMTREE, MULTIRIPPER, HYBRIDRIPPER, RANDOMFOREST, KFOREST) || (in_list(algorithm, 4, C45, LDT, MULTILDT, OMNILDT) && (get_parameter_l("prunetype") == PREPRUNING)))
  {
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
     posteriors = safemalloc(current_dataset->classno * sizeof(double), "tune_hyperparameters_of_the_supervised_algorithm", 20);   
   doc = xml_document("hyperparameters.xml");
   if (!doc)
    {
     printf(m1428);
     return NULL;
    }
   xml_parse(doc);
   if (in_list(algorithm, 2, KNN, KNNREG))
     element = xml_get_child_with_specific_attribute_value(doc->root, "name", "k");
   else
     if (in_list(algorithm, 4, C45, LDT, MULTILDT, OMNILDT))
       element = xml_get_child_with_specific_attribute_value(doc->root, "name", "pruningthreshold");
     else
       if (in_list(algorithm, 7, LOGISTIC, RBF, RBFREG, MLPGENERIC, MLPGENERICREG, DNC, DNCREG))
         element = xml_get_child_with_specific_attribute_value(doc->root, "name", "learningrate");
       else
         if (in_list(algorithm, 7, SVM, SIMPLEXSVM, SVMREG, SIMPLEXSVMREG, SVMTREE, MULTIRIPPER, HYBRIDRIPPER))
           element = xml_get_child_with_specific_attribute_value(doc->root, "name", "C");
         else
           if (in_list(algorithm, 2, RANDOMFOREST, KFOREST))
             element = xml_get_child_with_specific_attribute_value(doc->root, "name", "featuresize");
           else
            {
             printf(m1451);
             if (get_algorithm_type(algorithm) == CLASSIFICATION)
               return classification_algorithms[algorithm - SELECTMAX].train_algorithm(traindata, cvdata, parameters);
             else
               return regression_algorithms[algorithm - ONEFEATURE].train_algorithm(traindata, cvdata, parameters);
            }
   valuecount = xml_number_of_children(element);
   child = element->child;
   for (i = 0; i < valuecount; i++)
    {
     switch (algorithm){
       case KNN          :
       case KNNREG       :pknn = (Knn_parameterptr) parameters;
                          pknn->nearcount = atoi(child->pcdata);
                          break;
       case C45          :
       case LDT          :
       case SVMTREE      :pc45 = (C45_parameterptr) parameters;
                          pc45->pruningthreshold = atof(child->pcdata);
                          break;
       case MULTILDT     :
       case OMNILDT      :pmultildt = (Multildt_parameterptr) parameters;
                          pmultildt->c45param.pruningthreshold = atof(child->pcdata);
                          break;
       case LOGISTIC     :plogistic = (Logistic_parameterptr) parameters;
                          plogistic->learning_rate = atof(child->pcdata);
                          break;
       case RBF          :
       case RBFREG       :prbf = (Rbf_parameterptr) parameters;
                          prbf->eta = atof(child->pcdata);
                          break;
       case MLPGENERIC   :
       case MLPGENERICREG:
       case DNC          :
       case DNCREG       :pmlp = (Mlpparametersptr) parameters;
                          pmlp->eta = atof(child->pcdata);
                          break;
       case SVM          :
       case SVMREG       :psvm = (Svm_parameterptr) parameters;
                          psvm->C = atof(child->pcdata);
                          break;
       case SIMPLEXSVM   :
       case SIMPLEXSVMREG:ssvm = (Svm_simplex_parameterptr) parameters;
                          ssvm->C = atof(child->pcdata);
                          break;
						 case MULTIRIPPER  :
						 case HYBRIDRIPPER :set_parameter_f("svmC", child->pcdata, -INT_MAX, +INT_MAX);
							                   break;
       case RANDOMFOREST :prf = (Randomforest_parameterptr) parameters;
                          prf->featuresize = atoi(child->pcdata);
                          if (prf->featuresize >= current_dataset->featurecount - 1)
                            continue;
                          break;
       case KFOREST      :pkf = (Kforest_parameterptr) parameters;
                          pkf->featuresize = atoi(child->pcdata);
                          break;
     }
     if (get_algorithm_type(algorithm) == CLASSIFICATION)
       model = classification_algorithms[algorithm - SELECTMAX].train_algorithm(traindata, cvdata, parameters);
     else
       model = regression_algorithms[algorithm - ONEFEATURE].train_algorithm(traindata, cvdata, parameters);
     test = *tunedata;
     performance = 0.0;
     while (test)
      {
       next = test->next;
       test->next = NULL;
       if (get_algorithm_type(algorithm) == CLASSIFICATION)
         predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(test, model, parameters, posteriors);
       else
         predicted = regression_algorithms[algorithm - ONEFEATURE].test_algorithm(test, model, parameters, NULL);
       test->next = next;
       if (get_algorithm_type(algorithm) == CLASSIFICATION)
        {
         classno = give_classno(test);
         if (classno != predicted.classno)
           performance++;
        }
       else
        {
         real = give_regressionvalue(test);
         performance += (real - predicted.regvalue) * (real - predicted.regvalue);
        }
       test = test->next;
      }
     if (i == 0 || performance < bestperformance)
      {
       bestperformance = performance;
       write_array_variables("aout", totalrun, 15, "f", atof(child->pcdata));
       if (algorithm == KNN || algorithm == KNNREG)
         bestnear = atoi(child->pcdata);
       if (i != 0)
         free_model_of_supervised_algorithm(algorithm, bestmodel, current_dataset);
       bestmodel = model;
      }
     else
       free_model_of_supervised_algorithm(algorithm, model, current_dataset);
     child = child->sibling;
    }
   if (algorithm == KNN || algorithm == KNNREG)
    {    
     pknn = (Knn_parameterptr) parameters;
     pknn->nearcount = bestnear;    
    }
   xml_free_document(doc);
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
     safe_free(posteriors);
   return bestmodel;
  }
 else
  {
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
     return classification_algorithms[algorithm - SELECTMAX].train_algorithm(traindata, cvdata, parameters);
   else
     return regression_algorithms[algorithm - ONEFEATURE].train_algorithm(traindata, cvdata, parameters);
  }
}

/**
 * Not every supervised algorithm needs validation data. According to the global parameters of the program, this function checks if the corresponding algorithm needs validation data
 * @param[in] algorithm Index of the algorithm
 * @return 1 if the algorithm needs validation data, 0 otherwise.
 */
int is_cvdata_needed(int algorithm)
{
 /*!Last Changed 16.08.2007*/
 /*!Last Changed 04.03.2006*/
 switch (algorithm)
  {
   case C45          :
   case LDT          :
   case MULTILDT     :if (in_list(get_parameter_l("prunetype"), 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
                        return YES;
                      else
                        if (get_parameter_l("prunetype") == PREPRUNING && get_parameter_o("hyperparametertune") == ON)
                          return YES;
                        else
                          return NO;
                      break;
   case KTREE        :if (in_list(get_parameter_l("prunetype"), 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
                        return YES;
                      else
                        return NO;
   case RANDOMFOREST :
   case KFOREST      :if (get_parameter_o("hyperparametertune") == ON)
                        return YES;
                      else
                        return NO;
   case SVMTREE      :if (in_list(get_parameter_l("prunetype"), 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
                        return YES;
                      else
                        return NO;
                      break;
   case NBTREE       :if (in_list(get_parameter_l("prunetype"), 3, PREPRUNING, VALIDATIONPRUNING, MODELSELECTIONPRUNING))
                        return YES;
                      else
                        return NO;
                      break;
   case MULTIRIPPER  :if ((get_parameter_i("optimizecount") > 0) || (get_parameter_o("hyperparametertune") == ON))
                        return YES;
                      else
                        return NO;
                      break;
   case HYBRIDRIPPER :if ((get_parameter_i("optimizecount") > 0 && get_parameter_l("modelselectionmethod") == 0) || (get_parameter_o("hyperparametertune") == ON))
                        return YES;
                      else
                        return NO;
                      break;
   case HMM          :if (get_parameter_l("hmmlearningtype") != LEARNING_FIXED)
                        return YES;
                      else
                        return NO;
   case SVMREGTREE   :
   case REGTREE      :
   case OMNILDT      :
   case REGRULE      :if (get_parameter_l("prunetype") == VALIDATIONPRUNING || (get_parameter_l("prunetype") == MODELSELECTIONPRUNING && get_parameter_l("modelselectionmethod") == 0))
                        return YES;
                      else
                        return NO;
                      break;
   case SVM          :
   case SIMPLEXSVM   :
   case SVMREG       :
   case SIMPLEXSVMREG:
   case KNN          :
   case KNNREG       :
   case LOGISTIC     :if (get_parameter_o("hyperparametertune") == ON)
                        return YES;
                      else 
                        return NO;
                      break;
  }
 return cvdata_needed(algorithm);
}

/**
 * Given the global parameters of the program, this function prepares the parameters of the supervised algorithm. In some cases such as Neural Networks and Support Vector Machines, it calls parameter preparing functions.
 * @param[in] algorithm Index of the algorithm
 * @return Parameters of the algorithm
 */
void* prepare_supervised_algorithm_parameters(int algorithm)
{
 /*!Last Changed 11.08.2007 added SVMTREE algorithm*/
 /*!Last Changed 11.08.2007 added NAIVEBAYES algorithm*/
 /*!Last Changed 05.08.2007 added QDACLASS algorithm*/
 /*!Last Changed 02.04.2007 added HMM algorithm*/
 /*!Last Changed 14.08.2005*/
 Knn_parameterptr pknn;
 C45_parameterptr pc45;
 Ldaclass_parameterptr plda;
 Ripper_parameterptr prip;
 Mlpparametersptr pmlp;
 Softregtree_parameterptr psoftregtree;
 Regtree_parameterptr pregtree;
 Multildt_parameterptr pmultildt;
 Quantizereg_parameterptr pqua;
 Regrule_parameterptr pregrule;
 Logistic_parameterptr plogistic;
 Svm_parameterptr psvm;
 Svmtree_parameterptr psvmtree;
 Nodeboundedtree_parameterptr pnbtree;
 Hmm_parameterptr phmm;
 Rbf_parameterptr prbf;
 Gprocess_parameterptr pgprocess;
 Randomforest_parameterptr prf;
 Kforest_parameterptr pkf;
 Cn2_parameterptr pcn2;
 Lerils_parameterptr plerils;
 Svm_simplex_parameterptr ssvm;
 switch (algorithm)
  {
   case SELECTMAX     :
   case ONEFEATURE    :
   case NEARESTMEAN   :
   case GAUSSIAN      :
   case SELECTAVERAGE :
   case LINEARREG     :
   case NAIVEBAYES    :
   case QDACLASS      :
   case C45RULES      :return NULL;
   case QUANTIZEREG   :pqua = safemalloc(sizeof(Quantizereg_parameter), "prepare_supervised_algorithm_parameters", 4);
                       pqua->partitioncount = get_parameter_i("partitioncount");
                       return pqua;
   case KNN           :
   case KNNREG        :pknn = safemalloc(sizeof(Knn_parameter), "prepare_supervised_algorithm_parameters", 6);
                       pknn->nearcount = get_parameter_i("nearcount");
                       return pknn;
   case GPROCESSREG   :pgprocess = safemalloc(sizeof(Gprocess_parameter), "prepare_supervised_algorithm_parameters", 6);
                       pgprocess->sigma = get_parameter_f("sigma");
                       pgprocess->kernel_type = get_parameter_l("kerneltype");
                       pgprocess->coef0 = get_parameter_f("svmcoef0");
                       pgprocess->degree = get_parameter_i("svmdegree");
                       pgprocess->gamma = get_parameter_f("svmgamma");
                       return pgprocess;
   case LDT           :
   case C45           :
   case CART          :
   case KTREE         :pc45 = safemalloc(sizeof(C45_parameter), "prepare_supervised_algorithm_parameters", 9);
                       pc45->prunetype = get_parameter_l("prunetype");
                       pc45->prunemodel = get_parameter_l("prunemodel");
                       pc45->pruningthreshold = get_parameter_f("pruningthreshold");
                       return pc45;
   case RANDOMFOREST  :prf = safemalloc(sizeof(Randomforest_parameter), "prepare_supervised_algorithm_parameters", 12);
                       prf->featuresize = get_parameter_i("featuresize");
                       prf->forestsize = get_parameter_i("forestsize");
                       return prf;
   case KFOREST       :pkf = safemalloc(sizeof(Kforest_parameter), "prepare_supervised_algorithm_parameters", 12);
                       pkf->featuresize = get_parameter_i("featuresize");
                       pkf->forestsize = get_parameter_i("forestsize");
                       pkf->k = get_parameter_i("smallsetsize");
                       return pkf;
   case SVMTREE       :psvmtree = safemalloc(sizeof(Svmtree_parameter), "prepare_supervised_algorithm_parameters", 9);
                       psvmtree->c45param.prunetype = get_parameter_l("prunetype");
                       psvmtree->c45param.pruningthreshold = get_parameter_f("pruningthreshold");
                       psvmtree->multiclasstype = get_parameter_l("multiclasstype");
                       return psvmtree;
   case NBTREE        :pnbtree = safemalloc(sizeof(Nodeboundedtree_parameter), "prepare_supervised_algorithm_parameters", 13);
                       pnbtree->nodecount = get_parameter_i("nodecount");
                       pnbtree->model = get_parameter_l("modelselectionmethod");
                       if (get_parameter_s("model_per_level") != NULL)
                        {
                         if (strlen(get_parameter_s("model_per_level")) > MAX_MODEL_STRING_SIZE)
                           printf(m1338);
                         strcpy(pnbtree->modelstring, get_parameter_s("model_per_level"));
                        }
                       pnbtree->variance_explained = get_parameter_f("variance_explained");
                       pnbtree->prunetype = get_parameter_l("prunetype");    
                       pnbtree->partitioncount = get_parameter_i("partitioncount");
                       pnbtree->usediscrete = get_parameter_o("usediscrete");
                       pnbtree->vccalculationtype = get_parameter_l("vccalculationtype");
                       return pnbtree;
   case LDACLASS      :plda = safemalloc(sizeof(Ldaclass_parameter), "prepare_supervised_algorithm_parameters", 13);
                       plda->variance_explained = get_parameter_f("variance_explained");
                       return plda;
   case LOGISTIC      :plogistic = safemalloc(sizeof(Logistic_parameter), "prepare_supervised_algorithm_parameters", 16);
                       plogistic->etadecrease = get_parameter_f("etadecrease");
                       plogistic->learning_rate = get_parameter_f("learning_rate");
                       plogistic->perceptron_run = get_parameter_i("perceptronrun");
                       return plogistic;
   case IREP          :
   case IREP2         :
   case RIPPER        :
   case MULTIRIPPER   :
   case HYBRIDRIPPER  :
   case REXRIPPER     :prip = safemalloc(sizeof(Ripper_parameter), "prepare_supervised_algorithm_parameters", 16);
                       if (get_parameter_s("classpermutation") != NULL)
                        {
                         if (strlen(get_parameter_s("classpermutation")) > MAX_CLASS_PERMUTATION_SIZE)
                           printf(m1339);
                         strcpy(prip->classpermutation, get_parameter_s("classpermutation"));
                        }
                       else
                         strcpy(prip->classpermutation, "");
                       prip->optimizecount = get_parameter_i("optimizecount");
                       prip->rexenabled = NO;
                       prip->rulesordered = YES;
                       prip->smallset = get_parameter_o("smallset");
                       prip->smallsetsize = get_parameter_i("smallsetsize");
                       prip->usefisher = get_parameter_o("usefisher");
                       prip->withoutoutliers = get_parameter_o("withoutoutliers");
                       prip->confidencelevel = get_parameter_f("confidencelevel");
                       prip->correctiontype = get_parameter_l("correctiontype");
                       prip->conditiontype = UNIVARIATE_CONDITION;
                       prip->modelselectionmethod = get_parameter_l("modelselectionmethod");
                       prip->nearcount = get_parameter_i("nearcount");
                       prip->multivariatealgorithm = get_parameter_l("multivariatealgorithm");
                       prip->variancerange = get_parameter_f("variancerange");
                       prip->variance_explained = get_parameter_f("variance_explained");
                       prip->classorderoptimize = get_parameter_o("classorderoptimize");
                       break;
   case CN2           :pcn2 = safemalloc(sizeof(Cn2_parameter), "prepare_supervised_algorithm_parameters", 31);
                       pcn2->maxstar = get_parameter_i("beamsize");
                       pcn2->confidencelevel = get_parameter_f("confidencelevel");
                       if (get_parameter_s("classpermutation") != NULL)
                        {
                         if (strlen(get_parameter_s("classpermutation")) > MAX_CLASS_PERMUTATION_SIZE)
                           printf(m1339);
                         strcpy(pcn2->classpermutation, get_parameter_s("classpermutation"));
                        }
                       else
                         strcpy(pcn2->classpermutation, "");
                       return pcn2;
                       break;
   case LERILS        :plerils = safemalloc(sizeof(Lerils_parameter), "prepare_supervised_algorithm_parameters", 36);
                       plerils->flip_probability = get_parameter_f("mutationprobability");
                       plerils->pool_size = get_parameter_i("populationsize");
                       plerils->restarts = get_parameter_i("restarts");
                       return plerils;
   case RBF           :
   case RBFREG        :prbf = safemalloc(sizeof(Rbf_parameter), "prepare_supervised_algorithm_parameters", 34);
                       if (current_dataset->classno == 0)
                         prbf->output = 1;
                       else
                         prbf->output = current_dataset->classno;
                       prbf->input = current_dataset->multifeaturecount - 1;
                       prbf->hidden = ((int *) get_parameter_a("hiddennodes"))[0];
                       prbf->epochs = get_parameter_i("perceptronrun");
                       prbf->alpha = get_parameter_f("alpha");
                       prbf->eta = get_parameter_f("learning_rate");
                       prbf->initialeta = prbf->eta;
                       prbf->etadecrease = get_parameter_f("etadecrease");
                       prbf->randrange = 0.01;
                       return prbf;
   case MLPGENERIC    :
   case MLPGENERICREG :pmlp = safemalloc(sizeof(Mlpparameters), "prepare_supervised_algorithm_parameters", 34);
                       *pmlp = prepare_Mlpparameters(NULL, NULL, get_parameter_o("weightdecay"), get_parameter_f("decayalpha"), get_parameter_i("hiddenlayers"), (int *) get_parameter_a("hiddennodes"), 0, 0.0, 0.0, get_parameter_i("maxhidden"), get_algorithm_type(algorithm));
                       return pmlp;
   case DNC           :
   case DNCREG        :pmlp = safemalloc(sizeof(Mlpparameters), "prepare_supervised_algorithm_parameters", 38);
                       *pmlp = prepare_Mlpparameters(NULL, NULL, get_parameter_o("weightdecay"), get_parameter_f("decayalpha"), 1, NULL, get_parameter_i("windowsize"), get_parameter_f("errorthreshold"), get_parameter_f("errordropratio"), get_parameter_i("maxhidden"), get_algorithm_type(algorithm));
                       return pmlp;
   case SVMREGTREE    :
   case REGTREE       :
   case REGSTUMP      :pregtree = safemalloc(sizeof(Regtree_parameter), "prepare_supervised_algorithm_parameters", 37);
                       pregtree->correctiontype = get_parameter_l("correctiontype");
                       pregtree->leaftype = get_parameter_l("leaftype");
                       pregtree->modelselectionmethod = get_parameter_l("modelselectionmethod");
                       pregtree->nearcount = get_parameter_i("nearcount");
                       pregtree->prunetype = get_parameter_l("prunetype");
                       pregtree->pruningthreshold = get_parameter_f("pruningthreshold");
                       pregtree->sigma = get_parameter_f("sigmaestimate");
                       return pregtree;
   case SOFTREGTREE   :
   case SOFTTREE      :
   case SOFTSTUMP     :
   case SOFTREGSTUMP  :psoftregtree = safemalloc(sizeof(Softregtree_parameter), "prepare_supervised_algorithm_parameters", 37);
                       psoftregtree->leaftype = get_parameter_l("leaftype");
                       psoftregtree->epochs = get_parameter_i("perceptronrun");
                       psoftregtree->learningrate = get_parameter_f("learning_rate");
                       psoftregtree->etadecrease = get_parameter_f("etadecrease");
                       psoftregtree->alpha = get_parameter_f("alpha");
                       psoftregtree->regularization = get_parameter_l("regularization");
                       return psoftregtree;
   case REGRULE       :pregrule = safemalloc(sizeof(Regrule_parameter), "prepare_supervised_algorithm_parameters", 45);
                       pregrule->correctiontype = get_parameter_l("correctiontype");
                       pregrule->leaftype = get_parameter_l("leaftype");
                       pregrule->modelselectionmethod = get_parameter_l("modelselectionmethod");
                       pregrule->optimizecount = get_parameter_i("optimizecount");
                       pregrule->sigma = get_parameter_f("sigmaestimate");
                       return pregrule;
   case MULTILDT      :
   case OMNILDT       :pmultildt = safemalloc(sizeof(Multildt_parameter), "prepare_supervised_algorithm_parameters", 45);
                       pmultildt->c45param.prunetype = get_parameter_l("prunetype");
                       pmultildt->c45param.pruningthreshold = get_parameter_f("pruningthreshold");
                       pmultildt->conditiontype = MULTIVARIATE_CONDITION;
                       pmultildt->correctiontype = get_parameter_l("correctiontype");
                       pmultildt->modelselectionmethod = get_parameter_l("modelselectionmethod");
                       pmultildt->variance_explained = get_parameter_f("variance_explained");
                       pmultildt->variancerange = get_parameter_f("variancerange");
                       pmultildt->a1 = get_parameter_f("srma1");
                       pmultildt->a2 = get_parameter_f("srma2");
                       break;
   case SVM           :psvm = safemalloc(sizeof(Svm_parameter), "prepare_supervised_algorithm_parameters", 46);
                       prepare_svm_parameters(psvm, C_SVC);
                       return psvm;
   case SIMPLEXSVM    :
   case SIMPLEXSVMREG :ssvm = safemalloc(sizeof(Svm_simplex_parameter), "prepare_supervised_algorithm_parameters", 46);
                       ssvm->C = get_parameter_f("svmC");
                       ssvm->epsilon = get_parameter_f("svmepsilon");
                       ssvm->kernel.type = get_parameter_l("kerneltype");
                       ssvm->kernel.coef0 = get_parameter_f("svmcoef0");
                       ssvm->kernel.degree = get_parameter_i("svmdegree");
                       ssvm->kernel.gamma = get_parameter_f("svmgamma");
                       return ssvm;
   case SVMREG        :psvm = safemalloc(sizeof(Svm_parameter), "prepare_supervised_algorithm_parameters", 46);
                       prepare_svm_parameters(psvm, EPSILON_SVR);
                       return psvm;
   case HMM           :phmm = safemalloc(sizeof(Hmm_parameter), "prepare_supervised_algorithm_parameters", 64);                      
                       phmm->hmmtype = get_parameter_l("hmmtype");
                       phmm->statetype = get_parameter_l("hmmstatetype");
                       phmm->iteration = get_parameter_i("perceptronrun");
                       phmm->weightfreezing = get_parameter_o("weightfreezing");
                       phmm->learningtype = get_parameter_l("hmmlearningtype");
                       phmm->statecounts = get_parameter_a("statecounts");
                       phmm->componentcounts = get_parameter_a("componentcounts");
                       return phmm;
  }
 switch (algorithm)
  {
   case IREP         :
   case IREP2        :
   case RIPPER       :return prip;
   case MULTIRIPPER  :prip->conditiontype = MULTIVARIATE_CONDITION;
                      prip->multivariatealgorithm = get_parameter_l("multivariatealgorithm");
                      return prip;
   case HYBRIDRIPPER :prip->conditiontype = HYBRID_CONDITION;
                      prip->multivariatealgorithm = get_parameter_l("multivariatealgorithm");
                      return prip;
   case REXRIPPER    :prip->rexenabled = YES;
                      prip->rulesordered = NO;
                      return prip;
   case MULTILDT     :return pmultildt;
   case OMNILDT      :pmultildt->conditiontype = HYBRID_CONDITION;
                      return pmultildt;
  }
 return NULL;
}

/**
 * Prints the model user-friendly to screen. For example, rules generated via rule learning algorithms are printed to the screen.
 * @param[in] algorithm Index of the algorithm
 * @param[in] model Output model of the algorithm
 */
void display_output(int algorithm, void* model)
{
 /*!Last Changed 29.08.2005*/
 Ruleset r;
 Decisionnodeptr rootnode;
 switch (algorithm)
  {
   case C45          :rootnode = (Decisionnodeptr) model;
                      if (get_parameter_o("writerules"))
                       {
                        r = create_ruleset_from_decision_tree(rootnode);
                        write_ruleset(r);
                        free_ruleset(r);
                       }
                      break;
   case LDT          :
   case MULTILDT     :rootnode = (Decisionnodeptr) model;
                      if (get_parameter_o("writerules"))
                       {
                        r = create_ruleset_from_decision_tree(rootnode);
                        write_ruleset(r);
                        free_ruleset(r);
                       }
                      break;
   case OMNILDT      :rootnode = (Decisionnodeptr) model;
                      if (get_parameter_o("writerules"))
                       {
                        r = create_ruleset_from_decision_tree(rootnode);
                        write_ruleset(r);
                        free_ruleset(r);
                       }
                      break;
   case IREP         :
   case IREP2        :
   case RIPPER       :
   case C45RULES     :
   case MULTIRIPPER  :r = *((Ruleset*) model);
                      if (get_parameter_o("writerules"))
                        write_ruleset(r);
                      break;
   case HYBRIDRIPPER :r = *((Ruleset*) model);
                      if (get_parameter_o("writerules"))
                        write_ruleset(r);
                      break; 
   case REXRIPPER    :r = ((Model_rexripperptr) model)->r;
                      if (get_parameter_o("writerules"))
                        write_ruleset(r);
                      break;
  }
}
