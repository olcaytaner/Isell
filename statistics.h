#ifndef __statistics_H
#define __statistics_H
#include "tests.h"
#include "typedef.h"

enum correction_type{ 
      BONFERRONI_CORRECTION = 0,
      HOLM_CORRECTION,
      NO_CORRECTION};

typedef enum correction_type CORRECTION_TYPE;

/*! Link node structure in the MultiTest graph. The links are stored as a link list*/
struct link{
             /*! The node the link is coming from*/
             int from;
             /*! The node the link is going to*/
             int to;
             /*! Pointer to the next link*/
             struct link* next;
            };

typedef struct link Link;
typedef Link* Linkptr;

/*! Comparisons those made in the MultiTest algorithm*/
struct comparison{
                   /*! Index of the first algorithm compared*/
                   int alg1;
                   /*! Index of the second algorithm compared*/
                   int alg2;
                   /*! Confidence value of the statistical test between two algorithms*/
                   double confidence;
                   /*! Results of the statistical test. Has two values, YES if the test not rejects, NO if the test rejects*/
                   int accepted;
                  };

typedef struct comparison Comparison;
typedef Comparison* Comparisonptr;

struct confusion_matrix{
                        int false_positives;
                        int true_positives;
                        int false_negatives;
                        int true_negatives;
};

typedef struct confusion_matrix Confusion_matrix;
typedef Confusion_matrix* Confusion_matrixptr;

struct performance_point{
                         /*!x axis value*/
                         double x;
                         /*!y axis value*/
                         double y;
                         /*! if posterior probability is near 0.5, this is the equal loss point and equaloss is 1, otherwise 0*/
                         int equal_loss;
                        };
                 
typedef struct performance_point Performance_point;
typedef Performance_point* Performance_pointptr;

Comparisonptr        all_pairwise_comparisons(char** files, int filecount, int correctiontype, TEST_TYPE testtype);
void                 binary_comparison(int (*algorithm)(char** files,int *which,double confidence,double *real_confidence), char** params, int paramcount);
void                 binary_tests_compare(char** names,int count,int equality, FILE* output);
void                 bonferroni_correction(Comparisonptr comparisons, int compcount, double confidence);
void                 bootstrap(char* infile, char* outfile);
int                  compare_two_files_using_test(TEST_TYPE testtype, char **files, int *which, double confidence, double *real_confidence);
int                  connected_components(Linkptr edges, int nodecount);
Confusion_matrixptr  construct_confusion_matrix_from_file(char* posterior_file_name, double decision_threshold);
Confusion_matrix     construct_confusion_matrix_from_posteriors(matrix posteriors, double decision_threshold);
matrix               construct_posterior_matrix(double** posteriors, Instanceptr data);
double               file_mean(char **files,int filecount);
double               file_percentile(char *file, double percentile);
double               file_variance(char **files,int filecount);
void                 file_whiskers(char* file, double p25, double p75, double* swhisker, double* lwhisker);
double               find_area_under_the_curve_2class(matrix posteriors, int posclass, int negclass, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn));
double               find_area_under_the_curve_kclass(matrix posteriors, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn));
int                  find_best_classifier(char**files, int filecount, TEST_TYPE testtype);
void                 find_cliques(char** files, int filecount);
Performance_pointptr find_performance_points(matrix posteriors, int* number_of_points, int posclass, int negclass, Performance_point (*calculate_performance)(int tp, int fp, int tn, int fn));
double**             find_true_false_positives(double**, int datasize, int*);
double               gaussian(double x, double mean, double stdev);
double               gaussian_multivariate(Instanceptr inst, double* mean, matrix covariance);
double               gaussian_vector(Instanceptr x, Instanceptr mean, double stdev);
void                 generate_statistical_data(char* filename, int datacount, int zeroonecount, double errorrate);
int                  generate_tests(char **files, int filecount);
void                 holm_correction(Comparisonptr comparisons, int compcount, double confidence);
int*                 multiple_comparison(char **files,int filecount,TEST_TYPE testtype, char* st, int* monotone, int* componentcount, CORRECTION_TYPE correctiontype);
void                 multiple_tests(double (*algorithm)(char** files), char** params, int paramcount);
void                 no_correction(Comparisonptr comparisons, int compcount, double confidence);
Performance_point    precision_recall_performance(int tp, int fp, int tn, int fn);
void                 produce_distribution(double probability, int howmany, int samplesize, char *f_name);
void                 produce_normal_distribution(char* filename, int N, double mean, double stdev);
double               produce_normal_value(double mean, double stdev);
Performance_point    tpr_fpr_performance(int tp, int fp, int tn, int fn);

#endif
