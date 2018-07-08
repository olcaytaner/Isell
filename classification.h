#ifndef __classification_H
#define __classification_H

#include "typedef.h"

/*!Model learned in the SELECTMAX classification algorithm*/
struct model_selectmax{
                       /*!Since SELECTMAX algorithm assigns the class with the maximum prior probability, we only need that class index*/
                       int classno;
                       /*!To produce posterior probabilities we need prior probabilities. Posterior probabilities are equal to prior probabilities in the SELECTMAX algorithm*/
                       double* priors;
};

typedef struct model_selectmax Model_selectmax;
typedef Model_selectmax* Model_selectmaxptr;

/*!Model learned in the NAIVE BAYES classification algorithm*/
struct model_naivebayes{
                        /*!Mean vector of each continuous feature. Stored as a two dimensional array. m[0] is the mean vector of the first feature, m[1] is the mean vector of the second feature, etc.*/
	                       double** m;
                        /*!Standard deviation of each continuous feature. s[0] is the standard deviation of the first feature, s[1] is the standard deviation of the second feature, etc.*/
																								double* s;
                        /*!Prior probabilities of each class. priors[i] is the prior probability of the feature i*/
																								double* priors;
                        /*!Maximum likelihood estimates of each discrete feature value of each class. Stored as a three dimensional array. p[i][j][k] is the maximum likelihood estimate of k'th possible value of the j'th feature of the class i*/
																								double*** p;
};

typedef struct model_naivebayes Model_naivebayes;
typedef Model_naivebayes* Model_naivebayesptr;

/*!Model learned in the QUADRATIC DISCRIMINANT ANALYSIS classification algorithm. Quadratic discrimant is defined as g_i(x) = x^TW_ix + w_i^Tx + w_{i0} (Alpaydin 2005 pp. 93)*/
struct model_qdaclass{
                      /*! W_i = -1/2S_i^{-1} where S_i is the covariance matrix of class i*/
	                     matrix* W;
                      /*! w_i = S_i^{-1}m_i where S_i is the covariance matrix and m_i is the mean vector of class i*/
																						matrix* w;
                      /*! w_{i0} = -1/2m_i^TS_i^{-1}m_i - 1/2log|S_i| + log P(C_i)*/
																						double* w0;
				                  double* priors;
};

typedef struct model_qdaclass Model_qdaclass;
typedef Model_qdaclass* Model_qdaclassptr;

/*!Model learned in the LINEAR DISCRIMINANT ANALYSIS (LDA). Since we require the inverse of the covariance matrix in LDA, where the inverse is not defined in some problems, Principal Component Analysis is used to reduce dimension with a given threshold epsilon*/
struct model_ldaclass{
                      /*!Number of features after PCA*/
	                     int newfeaturecount;
                      /*!Number of features before PCA*/
																						int oldfeaturecount;
                      /*!Bias weight of the linear discriminant for class i*/
																						double* w0;
                      /*!Weight vector of the linear discriminant for class i. It is stored as 1 row matrix.*/
																						matrix* w;
                      /*!Eigenvectors of the covariance matrix. Mainly used for applying dimension reduction for the test data*/
																						matrix eigenvectors;
                      /*!Eigenvalues of the covariance matrix*/
																						double* eigenvalues;
};

typedef struct model_ldaclass Model_ldaclass;
typedef Model_ldaclass* Model_ldaclassptr;

/*!Model learned in the simple GAUSSIAN classifier*/
struct model_gaussian{
                      /*!Mean vector of the training data*/
                      Instanceptr mean;
                      /*!Standard deviation of each class*/
                      double* variance;
                      /*!Prior probabilities of each class. priors[i] is the prior probability of the feature i*/
                      double* priors;
};

typedef struct model_gaussian Model_gaussian;
typedef Model_gaussian* Model_gaussianptr;

/*!Model learned in the REXRIPPER rule learning algorithm. REXRIPPER is the same as RIPPER but also has the exception data, where if a test instance is not classified using the ruleset, it is classified using 1nearest neighbor algorithm on the exception data*/
struct model_rexripper{
                       /*!Ruleset learned using RIPPER*/
	                      Ruleset r;
                       /*!Exception data for 1 nearest neighbor classification*/
																							Instanceptr exceptiondata;
};

typedef struct model_rexripper Model_rexripper;
typedef Model_rexripper* Model_rexripperptr;

struct condition{
                 int featureindex;
                 char comparison;
                 double value;
};

typedef struct condition Condition;

/*!Model learned in the DIVS rule learning algorithm. */
struct model_divs{
                  /*!Each hypothesis of an instance is represented as a ruleset*/
                  Condition*** hypotheses;
                  /*!Number of rules in each ruleset*/
                  int* rulecounts;
                  /*!Classno of each ruleset*/
                  int* classno;
                  /*!Number of conditions in each rule of each ruleset*/
                  int** conditioncounts;
                  /*!Number of hypotheses in the model*/
                  int datacount;
                  /*!specificity*/
                  int M;
                  /*!consistency*/
                  int epsilon;
};

typedef struct model_divs Model_divs;
typedef Model_divs* Model_divsptr;

/*!Model learned in the RISE rule learning algorithm. */
struct model_rise{
                  Ruleset rules;
                  matrix* svdm;
                  int* counts;
};

typedef struct model_rise Model_rise;
typedef Model_rise* Model_riseptr;

void                    free_performance(Classificationresultptr result);
void                    make_tree_stump(Decisionnodeptr rootnode);
Classificationresultptr performance_initialize(int testsize);
void                    print_classification_results(Instanceptr testdata, Classificationresultptr result, double runtime, int printbinary, int displayruntime, int accuracy, int displayclassresulton);
void                    print_confusion(FILE* outfile, int** confusionmatrix, int classno);
void                    print_confusion_2d(FILE* outfile, int** confusionmatrix);
void                    print_posterior(FILE* outfile, double** posteriors, int testsize, Instanceptr data);
matrixptr               read_posteriors(int numfolds, char *fname);
Prediction              test_c45(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_c45rules(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_c45stump(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_cart(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_cn2(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_divs(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_dnc(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_gaussian(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_hmm(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_hybridripper(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_irep(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_irep2(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_knn(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_kforest(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_ktree(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_ldaclass(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_lerils(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_ldt(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_ldtstump(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_logistic(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_mlpgeneric(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_multildt(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_multildtstump(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_multiripper(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_naivebayes(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_nearestmean(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_nodeboundedtree(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_omnildt(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_part(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_qdaclass(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_randomforest(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_rbf(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_rexripper(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_ripper(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_rise(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_select_max(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_simplexsvm(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_softstump(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_softtree(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_svm(Instanceptr data, void* model, void* parameters, double* posterior);
Prediction              test_svmtree(Instanceptr data, void* model, void* parameters, double* posterior);
void*                   train_c45(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_c45rules(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_c45stump(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_cart(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_cn2(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_divs(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_dnc(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_gaussian(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_hmm(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_hybridripper(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_irep(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_irep2(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_kforest(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_knn(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_ktree(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_ldaclass(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_lerils(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_ldt(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_ldtstump(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_logistic(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_mlpgeneric(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_multiripper(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_multildt(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_multildtstump(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_naivebayes(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_nearestmean(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_nodeboundedtree(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_omnildt(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_part(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_qdaclass(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_randomforest(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_rbf(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_rexripper(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_ripper(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_rise(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_select_max(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_simplexsvm(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_softstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_softtree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters);
void*                   train_svm(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);
void*                   train_svmtree(Instanceptr* trdata, Instanceptr* cvdata, void* parameters);

#endif
