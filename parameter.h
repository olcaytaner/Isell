#ifndef __parameter_H
#define __parameter_H

#include "constants.h"

#define MAX_PARAMETER_TYPE 6

enum parameter_type{ 
      INTEGER_PARAMETER = 0,
      FLOAT_PARAMETER,
      ONOFF_PARAMETER,
      LIST_PARAMETER,
      STRING_PARAMETER,
      ARRAY_PARAMETER};
   
typedef enum parameter_type PARAMETER_TYPE;

#define MAX_MODEL_STRING_SIZE 100
#define MAX_CLASS_PERMUTATION_SIZE 11

/*! Union storing the value of a parameter. Since a parameter can be one of six different types, we need a union to store the value of the parameter*/
union parametervalue{
       /*! Integer type*/
       int intvalue;
       /*! Float type*/
       double floatvalue;
       /*! List type*/
       int listvalue;
       /*! String type*/
       char* stringvalue;
       /*! On-off type*/
       int onoffvalue;
       /*! Array type*/
       void* arrayvalue;
       };

/*! Structure storing the program parameter information.*/
struct parameter{
                 /*! Name of the parameter*/
                 char* name;
                 /*! Type of the parameter. Can be INTEGER_PARAMETER, FLOAT_PARAMETER, ONOFF_PARAMETER, LIST_PARAMETER, STRING_PARAMETER or ARRAY_PARAMETER*/
                 PARAMETER_TYPE type;
                 /*! If the parameter is an array, this field stores the size of that array. If the parameter is of type list, this field stores the number of possible values of that list type*/
                 int arraysize;
                 /*! If the parameter is of list type, all possible values must be stored for checking possible alterations in the parameter's value.*/
                 char** listvalues;
                 /*! Current value of the parameter. Stored as a union.*/
                 union parametervalue value;
};

typedef struct parameter Parameter;
typedef Parameter* Parameterptr;

/*! Parameters of the KNN algorithm*/
struct knn_parameter{
                     /*! k: Number of neighbors*/
	                    int nearcount;
};

typedef struct knn_parameter Knn_parameter;
typedef Knn_parameter* Knn_parameterptr;

/*! Parameters of the C4.5 algorithm*/
struct c45_parameter{
                     /*! Type of pruning used in decision tree induction. Have values PREPRUNING for pre-pruning, VALIDATIONPRUNING for post-pruning using a validation set, MODELSELECTIONPRUNING for pruning according to the model selection method used such as AIC, BIC etc., NOPRUNING for not doing any pruning and SRMPRUNING to prune according to the SRM model selection algorithm.*/
	                    PRUNE_TYPE prunetype;
                     PRUNE_MODEL prunemodel; 
                     /*! Threshold used for pre-pruning*/
																					double pruningthreshold;
};

typedef struct c45_parameter C45_parameter;
typedef C45_parameter* C45_parameterptr;

/*! Parameters of the Svmtree algorithm*/
struct svmtree_parameter{
                         /*! Pruning parameters*/
                         C45_parameter c45param;
                         /*! Multiclass run type*/
                         int multiclasstype;
};

typedef struct svmtree_parameter Svmtree_parameter;
typedef Svmtree_parameter* Svmtree_parameterptr;

/*! Parameters of the Random Forest algorithm*/
struct randomforest_parameter{
                              /*! Number of decision trees in the forest*/
                              int forestsize;
                              /*! Number of random features used in the tree*/
                              int featuresize;
};

typedef struct randomforest_parameter Randomforest_parameter;
typedef Randomforest_parameter* Randomforest_parameterptr;

/*! Parameters of the K Forest algorithm*/
struct kforest_parameter{
                         /*! Number of decision trees in the forest*/
                         int forestsize;
                         /*! Number of random extracted features used in the tree*/
                         int featuresize;
                         /*! k in the K-forest*/
                         int k;
};

typedef struct kforest_parameter Kforest_parameter;
typedef Kforest_parameter* Kforest_parameterptr;

/*! Parameters of the MULTILDT and OMNILDT algorithms*/
struct multildt_parameter{
                          /*! Pruning parameters*/
	                         C45_parameter c45param;
                          /*! If one wants to remove outliers before finding the linear or quadratic split, this parameters sets the range of outliers.*/
                          double variancerange;
                          /*! Type of model selection used in OMNILDT. Has value MODEL_SELECTION_CV for cross-validation based model selection, MODEL_SELECTION_AIC for Akaike Information Criterion, MODEL_SELECTION_BIC for Bayesian Information Criterion, MODEL_SELECTION_SRM for Structural Risk Minimization, MODEL_SELECTION_MDL for Minimum Description Length, MODEL_SELECTION_UNI for fixed univariate model, MODEL_SELECTION_LIN for fixed multivariate linear model, MODEL_SELECTION_QUA for fixed multivariate quadratic model.*/
																										int modelselectionmethod;
                          /*! Variance explained used in the Principal Component Analysis in Linear Discriminant Analysis based tree induction*/
                          double variance_explained;
                          /*! Type of correction used in the Cross-validation based model selection. BONFERRONI_CORRECTION for Bonferroni type correction, HOLM_CORRECTION for Holm's algorithm based correction, NO_CORRECTION if no correction will be applied*/
                          int correctiontype;
                          /*! Type of condition in the LDT (UNIVARIATE_CONDITION), MULTILDT (MULTIVARIATE_CONDITION) or OMNILDT (HYBRID_CONDITION).*/
																										int conditiontype;
                          /*! a1 constant in SRM based model selection*/
																										double a1;
                          /*! a2 constant in SRM based model selection*/
																										double a2;
};

typedef struct multildt_parameter Multildt_parameter;
typedef Multildt_parameter* Multildt_parameterptr;

/*! Parameters of the Gaussian process regression*/
struct gprocess_parameter{
                          /*!Noise ratio*/
                          double sigma;
                          /*!Type of kernel used in Gaussian process regression*/
                          int kernel_type;
                          /*! Bias for polynom kernel*/
                          double coef0;
                          /*! Degree of the polynom kernel*/
                          int degree;
                          /*! Parameter of the gaussian kernel*/
                          double gamma;
};

typedef struct gprocess_parameter Gprocess_parameter;
typedef Gprocess_parameter* Gprocess_parameterptr;

/*! Parameters of the REGTREE (Regression Tree) algorithm*/
struct regtree_parameter{
                         /*! Type of function in the leaves. Has value CONSTANTLEAF if the function is constant, LINEARLEAF is there is a linear function in the leaves.*/
                         LEAF_TYPE leaftype;
                         /*! Type of pruning used in decision tree induction. Acceptable values are PREPRUNING for pre-pruning, VALIDATIONPRUNING for post-pruning using a validation set and NOPRUNING for not doing any pruning.*/
                         int prunetype;
                         /*! Type of model selection used in REGTREE. Has value MODEL_SELECTION_CV for cross-validation based model selection.*/
                         int modelselectionmethod;
                         /*! Threshold used for pre-pruning*/
                         double pruningthreshold;
                         int nearcount;
                         /*! Type of correction used in the Cross-validation based model selection. BONFERRONI_CORRECTION for Bonferroni type correction, HOLM_CORRECTION for Holm's algorithm based correction, NO_CORRECTION if no correction will be applied*/
                         int correctiontype;
																									double sigma;
};

typedef struct regtree_parameter Regtree_parameter;
typedef Regtree_parameter* Regtree_parameterptr;

/*! Parameters of the SOFTREGTREE (Soft Regression Tree) algorithm*/
struct softregtree_parameter{
 /*! Type of function in the leaves. Has value CONSTANTLEAF if the function is constant, LINEARLEAF is there is a linear function in the leaves.*/
 LEAF_TYPE leaftype;
 /*! Number of epochs in gradient descent learning.*/
 int epochs;
 /*! Learning rate in gradient descent learning.*/
 double learningrate;
 /*! Decrease in the learning rate in gradient descent learning.*/
 double etadecrease;
 /*! Momentum in gradient descent optimization*/
 double alpha;
 REGULARIZATION_TYPE regularization;
};

typedef struct softregtree_parameter Softregtree_parameter;
typedef Softregtree_parameter* Softregtree_parameterptr;

/*! Parameters of the REGRULE (Regression Rule Learner) algorithm*/
struct regrule_parameter{
                         /*! Type of function in the rules. Has value CONSTANTLEAF if the function is constant, LINEARLEAF is there is a linear function in the leaves.*/
                         LEAF_TYPE leaftype;
                         /*! Type of model selection used in REGRULE. Has value MODEL_SELECTION_CV for cross-validation based model selection.*/
                         int modelselectionmethod;
                         /*! Number of optimization passes used in the REGRULE like in RIPPER*/
                         int optimizecount;
                         /*! Type of correction used in the Cross-validation based model selection. BONFERRONI_CORRECTION for Bonferroni type correction, HOLM_CORRECTION for Holm's algorithm based correction, NO_CORRECTION if no correction will be applied*/
                         int correctiontype;
																									double sigma;
};

typedef struct regrule_parameter Regrule_parameter;
typedef Regrule_parameter* Regrule_parameterptr;

/*! Parameters of the Linear Discriminant Analysis algorithm*/
struct ldaclass_parameter{
                          /*! Variance explained used in the Principal Component Analysis in Linear Discriminant Analysis to get orthogonal eigenvectors*/
	                         double variance_explained;
};

typedef struct ldaclass_parameter Ldaclass_parameter;
typedef Ldaclass_parameter* Ldaclass_parameterptr;

/*! Parameters of the Logistic Discrimination algorithm*/
struct logistic_parameter{
                          /*! Number of epochs*/
	                         int perceptron_run;
                          /*! Learning rate for update equations*/
																										double learning_rate;
                          /*! Decrease in the learning rate. This value is multiplied with current learning rate after each epoch*/
																										double etadecrease;
};

typedef struct logistic_parameter Logistic_parameter;
typedef Logistic_parameter* Logistic_parameterptr;

/*! Parameters of the RIPPER rule learning system including multivariate and omnivariate rule learners*/
struct ripper_parameter{
                        /*! In rule learning systems, when the rules are ordered, the first ... rules are for the first class, the second ... rules are for the second class, etc. In the default mode, the classes (which class will be the first class etc.) are ordered from the least prevalent to the most prevalent one. On the other hand, if classpermutation is given, the classes are ordered from the classpermutation[0] to classpermutation[C - 1], where C is the number of classes. The classes are represented as digits '0' for the first class, '1' for the second class etc.*/
                        char classpermutation[MAX_CLASS_PERMUTATION_SIZE];
                        /*! If rulesordered = YES, there is a default class for which there are no rules. If a test instance failed to accepted by any rules, that instance is classified with the default class. If rulesordered = NO, there is no default class.*/
																								int rulesordered;
                        /*! Number of optimization passes in the RIPPER*/
																								int optimizecount;
                        /*! If usefisher is ON and withoutoutliers is YES, the univariate split is found without using the outliers.*/
																								int usefisher;
                        /*! If smallset is ON and number of instances falling into the rule is smaller than smallsetsize no split is found.*/
																								int smallset;
                        /*! If smallset is ON and number of instances falling into the rule is smaller than smallsetsize no split is found.*/
																								int smallsetsize;
                        /*! If usefisher is ON and withoutoutliers is YES, the univariate split is found without using the outliers.*/
																								int withoutoutliers;
                        /*! If rexenabled is ON and a test instance failed to accepted by any rules, that instance is classified using KNN algorithm using exceptional instances*/
																								int rexenabled;
                        /*! Type of condition in the RIPPER (UNIVARIATE_CONDITION), MULTIRIPPER (MULTIVARIATE_CONDITION) or OMNIRIPPER (HYBRID_CONDITION).*/
																								int conditiontype;
                        /*! Type of the linear discriminant algorithm for separating two classes. Possible values are LINEAR_LDA for Linear Discriminant Analysis, LINEAR_LP for linear perceptron and LINEAR_SVM for Support Vector Machines with linear kernel*/
                        int multivariatealgorithm;
                        /*! Confidence level used in the statistical tests for cross-validation based model selection*/
                        double confidencelevel;
                          /*! Type of model selection used in OMNIRIPPER. Has value MODEL_SELECTION_CV for cross-validation based model selection, MODEL_SELECTION_AIC for Akaike Information Criterion, MODEL_SELECTION_BIC for Bayesian Information Criterion, MODEL_SELECTION_SRM for Structural Risk Minimization, MODEL_SELECTION_MDL for Minimum Description Length, MODEL_SELECTION_UNI for fixed univariate model, MODEL_SELECTION_LIN for fixed multivariate linear model, MODEL_SELECTION_QUA for fixed multivariate quadratic model.*/
                        int modelselectionmethod;
                         /*! Type of correction used in the Cross-validation based model selection. BONFERRONI_CORRECTION for Bonferroni type correction, HOLM_CORRECTION for Holm's algorithm based correction, NO_CORRECTION if no correction will be applied*/
                        int correctiontype;
                        /*! k: Number of neighbors used when Rex is enabled*/
																								int nearcount;
                        /*! If one wants to remove outliers before finding the linear or quadratic split, this parameters sets the range of outliers.*/
																								double variancerange;
                        /*! Variance explained used in the Principal Component Analysis in Linear Discriminant Analysis to get orthogonal eigenvectors*/
																								double variance_explained;
                        /*! If classorderoptimize is ON then the order of classes (rules are ordered according to class normally first by the least occuring class to the most occuring class) will be optimized*/
                        int classorderoptimize;
};

typedef struct ripper_parameter Ripper_parameter;
typedef Ripper_parameter* Ripper_parameterptr;

struct quantizereg_parameter{
	                            int partitioncount;
};

typedef struct quantizereg_parameter Quantizereg_parameter;
typedef Quantizereg_parameter* Quantizereg_parameterptr;

struct nodeboundedtree_parameter{
	                                int nodecount;
                                 int partitioncount;
																																	int model;
																																	int usediscrete;
                                 int vccalculationtype;
                                 int prunetype;
                                 double a1;
                                 double a2;
																																	double variance_explained;
                                 char modelstring[MAX_MODEL_STRING_SIZE];
};

typedef struct nodeboundedtree_parameter Nodeboundedtree_parameter;
typedef Nodeboundedtree_parameter* Nodeboundedtree_parameterptr;

struct hmm_parameter{
                     /*! Structure of the HMM. Can be lefttoright model, where each state is connected only to the next state and itself with exception that there is single connection (to itself) in the last state. Can be lefttorightloop model, where each state is connected to the next state and itself with the last state connected to the first state. Can be full model, where each state is connected to all other states.*/
                     HMM_MODEL_TYPE hmmtype;
                     /*! Type of states in the HMM. Can be multinomial, Gaussian, mixture of Gaussians, Dirichlet, mixture of Dirichlets*/
                     HMM_STATE_TYPE statetype;
                     /*! Number of epochs in Baum-Welch training*/
																					int iteration;
                     /*! If weightfreezing is YES, the weights are freezed while adding a new state to the HMM*/
                     int weightfreezing;
																					HMM_LEARNING_TYPE learningtype;
                     /*! Number of states for each class's HMM model*/
                     int* statecounts;
                     /*! Number of Gaussian or Dirichlet mixture components for each class's HMM model*/
                     int* componentcounts;
};

typedef struct hmm_parameter Hmm_parameter;
typedef Hmm_parameter* Hmm_parameterptr;

struct cn2_parameter{
                     /*! Maximum number of complexes retained after each iteration in CN2*/ 
                     int maxstar;
                     /*! Confidence level for the test that checks whether current rule is statistically significantly better than current best rule*/
                     double confidencelevel;
                     /*! In rule learning systems, when the rules are ordered, the first ... rules are for the first class, the second ... rules are for the second class, etc. In the default mode, the classes (which class will be the first class etc.) are ordered from the least prevalent to the most prevalent one. On the other hand, if classpermutation is given, the classes are ordered from the classpermutation[0] to classpermutation[C - 1], where C is the number of classes. The classes are represented as digits '0' for the first class, '1' for the second class etc.*/
                     char classpermutation[MAX_CLASS_PERMUTATION_SIZE];
};

typedef struct cn2_parameter Cn2_parameter;
typedef Cn2_parameter* Cn2_parameterptr;

struct lerils_parameter{
                        /*Flip probability during random walk*/
                        double flip_probability;
                        /*Number of rules in the pool*/
                        int pool_size;
                        /*Number of restarts for each ruleset*/
                        int restarts;
};

typedef struct lerils_parameter Lerils_parameter;
typedef Lerils_parameter* Lerils_parameterptr;

void    free_program_parameters();
void*   get_parameter_a(char* name);
double  get_parameter_f(char* name);
int     get_parameter_i(char* name);
int     get_parameter_l(char* name);
int     get_parameter_o(char* name);
char*   get_parameter_s(char* name);
int     parameter_index(char* name);
BOOLEAN read_program_parameters();
void    set_float_parameter(char* name, double value);
void    set_integer_parameter(char* name, int value);
void    set_parameter_af(char* name, char** values, int arraysize);
void    set_parameter_ai(char* name, char** values, int arraysize);
void    set_parameter_f(char* name, char* value, double min, double max);
void    set_parameter_i(char* name, char* value, int min, int max);
void    set_parameter_l(char* name, char* value, char* errormessage);
void    set_parameter_o(char* name, char* value);
void    set_parameter_s(char* name, char* value);

#endif
