#ifndef __constants_H
#define __constants_H

#define META_CLASSIFICATION_ALGORITHM_COUNT 3

enum meta_classification_algorithm_name{
 NO_META_ALGORITHM = -1,
 ONE_VERSUS_ONE = 100,
 ONE_VERSUS_REST,
 BAGGING_CLASSIFICATION
};

typedef enum meta_classification_algorithm_name META_CLASSIFICATION_ALGORITHM_NAME;

#define META_REGRESSION_ALGORITHM_COUNT 1

enum meta_regression_algorithm_name{
 BAGGING_REGRESSION = 200
};

typedef enum meta_regression_algorithm_name META_REGRESSION_ALGORITHM_NAME;

#define CLASSIFICATION_ALGORITHM_COUNT 41

enum classification_algorithm_name{
 SELECTMAX = 1000,
 KNN,
 C45,
 NEARESTMEAN,
 LDACLASS,
 LDT,
 IREP,
 IREP2,
 RIPPER,
 C45RULES,
 MULTIRIPPER,
 HYBRIDRIPPER,
 MLPGENERIC,
 REXRIPPER,
 MULTILDT,
 OMNILDT,
 LOGISTIC,
 SVM,
 SIMPLEXSVM,
 SVMTREE,
 GAUSSIAN,
 NBTREE,
 HMM,
 DNC,
 QDACLASS,
 NAIVEBAYES,
 RBF,
 RANDOMFOREST,
 RISE,
 DIVS,
 CN2,
 LERILS,
 PART,
 SOFTTREE,
 CART,
 KTREE,
 KFOREST,
 C45STUMP,
 LDTSTUMP,
 MULTILDTSTUMP,
 SOFTSTUMP};

typedef enum classification_algorithm_name CLASSIFICATION_ALGORITHM_NAME;

#define REGRESSION_ALGORITHM_COUNT 17

enum regression_algorithm_name{
 ONEFEATURE = 2000,
 KNNREG,
 REGTREE,
 MLPGENERICREG,
 SELECTAVERAGE,
 LINEARREG,
 QUANTIZEREG,
 REGRULE,
 SVMREG,
 SIMPLEXSVMREG,
 DNCREG,
 RBFREG,
 GPROCESSREG,
 SVMREGTREE,
 SOFTREGTREE,
 REGSTUMP,
 SOFTREGSTUMP};

typedef enum regression_algorithm_name REGRESSION_ALGORITHM_NAME;

enum supervised_algorithm_type{
 UNDEFINED_ALGORITHM = -1,
 CLASSIFICATION = 0,
 REGRESSION,
 AUTOENCODER};

typedef enum supervised_algorithm_type SUPERVISED_ALGORITHM_TYPE;

enum {
 ASCENDING = 0,
 DESCENDING};

enum hmm_state_type{ 
 STATE_DISCRETE = 0,
 STATE_GAUSSIAN,
 STATE_GAUSSIAN_MIXTURE,
 STATE_DIRICHLET,
 STATE_DIRICHLET_MIXTURE};

typedef enum hmm_state_type HMM_STATE_TYPE;

enum hmm_model_type{ 
 FULL_MODEL = 0,
 LR_MODEL,
 LR_LOOP_MODEL};

typedef enum hmm_model_type HMM_MODEL_TYPE;

enum hmm_learning_type{ 
 LEARNING_SEPARATE = 0,
 LEARNING_TOGETHER,
 LEARNING_FIXED};

typedef enum hmm_learning_type HMM_LEARNING_TYPE;

#define PI 3.1415926535897932384626433832795

#define ISELL 123456789

#define MAXCLASSES 50
#define MAXKERNELS 10
#define MAXVALUES 100
#define MAXFEATURES 25000
#define MAXLAYERS 3

#define MAXLENGTH 500000
#define READLINELENGTH MAXLENGTH
#define IMAGEPARTCOUNT 4

#define MAXPARAMS 100

#define MIN_SAMPLE_SIZE 1

enum storage_type{
 STORE_TEXT = 0,
 STORE_DB,
 STORE_SEQUENTIAL,
 STORE_KERNEL,
 STORE_MULTILABEL};

typedef enum storage_type STORAGE_TYPE;

enum class_type{
 POSITIVE_CLASS = 0,
 NEGATIVE_CLASS};

typedef enum class_type CLASS_TYPE;

enum multivariate_type{
 MULTIVARIATE_LINEAR = -2,
 MULTIVARIATE_QUADRATIC = -3};

typedef enum multivariate_type MULTIVARIATE_TYPE;

enum prune_type{
 PREPRUNING = 0,
 VALIDATIONPRUNING,
 MODELSELECTIONPRUNING,
 NOPRUNING,
 SRMPRUNING};

typedef enum prune_type PRUNE_TYPE;

enum prune_model{
 LOCAL = 0,
 GLOBAL};

typedef enum prune_type PRUNE_MODEL;

enum yesno{ 
 NO = 0,
 YES};

typedef enum yesno YES_NO;

enum boolean{ 
 BOOLEAN_FALSE = 0,
 BOOLEAN_TRUE};

typedef enum boolean BOOLEAN;

enum { 
 FAILURE = 0,
 SUCCESS};

enum { 
 COVERED = 0,
 UNCOVERED};

enum onoff{ 
 OFF = 0,
 ON};

typedef enum onoff ON_OFF;

#define MARGIN 80
#define SMALLMARGIN 5

enum model_selection_type{ 
 MODEL_SELECTION_CV = 0,
 MODEL_SELECTION_AIC,
 MODEL_SELECTION_BIC,
 MODEL_SELECTION_SRM,
 MODEL_SELECTION_MDL,
 MODEL_SELECTION_UNI,
 MODEL_SELECTION_LIN,
 MODEL_SELECTION_QUA,
 MODEL_SELECTION_LEV,
 MODEL_SELECTION_FIX};

typedef enum model_selection_type MODEL_SELECTION_TYPE;

enum model_type{ 
 MODEL_UNI = 0,
 MODEL_LIN,
 MODEL_QUA};

typedef enum model_type MODEL_TYPE;

enum linear_model_type{ 
 LINEAR_LDA = 0,
 LINEAR_LP,
 LINEAR_SVM};

typedef enum linear_model_type LINEAR_MODEL_TYPE;

enum bayesian_network_save_type{ 
 NET_FILE = 0,
 XML_FILE,
 HUGIN_FILE,
 BIF_FILE};

typedef enum bayesian_network_save_type BAYESIAN_NETWORK_SAVE_TYPE;

enum design_type{ 
 UNIFORM_DESIGN = 0,
 OPTIMIZED_DESIGN};

typedef enum design_type DESIGN_TYPE;

enum problem_type{
 ONE_VS_ONE = 0,
 ONE_VS_REST};

typedef enum problem_type PROBLEM_TYPE;

enum object_type{
 OBJECT_LINE = 0,
 OBJECT_STRING,
 OBJECT_RECTANGLE,
 OBJECT_MVBOX,
 OBJECT_BOXPLOT,
 OBJECT_ERRORHISTOGRAM,
 OBJECT_NAMEDPOINT,
 OBJECT_CIRCLE,
 OBJECT_POINT};

typedef enum object_type OBJECT_TYPE;

#define TESTCOUNT 8

enum feature_type{
 INTEGER = 0, 
 STRING, 
 REAL, 
 CLASSDEF, 
 REGDEF};

typedef enum feature_type FEATURE_TYPE;

enum normalization_type{
 Z_NORMALIZATION = 0,
 STANDARD_NORMALIZATION,
 VECTOR_NORMALIZATION};

typedef enum normalization_type NORMALIZATION_TYPE;

enum hingeloss_type{
 REGULAR_LOSS = 0,
 HINGELOSS_01,
 HINGELOSS_02};

typedef enum hingeloss_type HINGELOSS_TYPE;

enum RType {TRAIN = 1, TEST};

enum leaf_type{ 
 CONSTANTLEAF = 0,
 LINEARLEAF};

typedef enum leaf_type LEAF_TYPE;

enum font_type{ 
 HELVETICA = 0,
 COURIER,
 TIMES,
 ARIAL};

typedef enum font_type FONT_TYPE;

enum color_type{ 
 BLACK = 0,
 RED,
 GREEN,
 BLUE,
 PURPLE,
 YELLOW,
 CYAN,
 GRAY,
 ORANGE,
 BROWN,
 PINK,
 WHITE};

typedef enum color_type COLOR_TYPE;

#define COLORCOUNT 12

enum legend_position_type{ 
 DOWN_RIGHT = 0,
 DOWN_LEFT,
 UP_RIGHT,
 UP_LEFT,
 NONE};

typedef enum legend_position_type LEGEND_POSITION_TYPE;

enum group_coloring_type{
 ALL_SAME = 0,
 GROUP_SAME,
 INDIVIDUAL_SAME};

typedef enum group_coloring_type GROUP_COLORING_TYPE;

#define GROUP_COLORING_COUNT 3

enum discretization_type{
 EQUALWIDTH = 0,
 ENTROPY};
 
typedef enum discretization_type DISCRETIZATION_TYPE;

enum regularization_type{
 NO_REGULARIZATION = 0,
 L1_REGULARIZATION,
 L2_REGULARIZATION
};

typedef enum regularization_type REGULARIZATION_TYPE;

enum variable_type{ 
 NO_TYPE = -1,
 INTEGER_TYPE = 0,
 REAL_TYPE,
 STRING_TYPE,
 ARRAY_TYPE,
 FILE_TYPE,
 MATRIX_TYPE};

typedef enum variable_type VARIABLE_TYPE;

enum image_type{
 IMAGE_EPS = 0,
 IMAGE_PSTRICKS
};

typedef enum image_type IMAGE_TYPE;

#endif
