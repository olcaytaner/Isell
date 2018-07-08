#ifndef __dimreduction_H
#define __dimreduction_H

#include "typedef.h"

#define DIMENSION_REDUCTION_ALGORITHM_COUNT 5

enum dimension_reduction_name{
      PCA = 0,
      LDA,
      MDS,
      ISOMAP,
      LLE};

typedef enum dimension_reduction_name DIMENSION_REDUCTION_NAME;

enum reduction_type{ 
      REDUCTION_TYPE_FEATURE = 0,
      REDUCTION_TYPE_VARIANCE};

typedef enum reduction_type REDUCTION_TYPE;

enum neighborhood_type{
      NEIGHBORHOOD_TYPE_EPSILON = 0,
      NEIGHBORHOOD_TYPE_K};

typedef enum neighborhood_type NEIGHBORHOOD_TYPE;

/*! Algorithm structure for each dimension reduction machine learning algorithm*/
struct dimension_reduction_algorithm{
                                     /*! The function that does the dimension reduction*/
                                     Instanceptr (*algorithm)(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name);
                                     /*! Name of the dimension reduction algorithm*/
                                     char* name;
                                     /*! Do we need conversion from discrete features to numeric features in this machine learning algorithm? Values: YES or NO*/
                                     int mustrealize;
                                     /*! Do we need normalize features in this machine learning algorithm? Values: YES or NO*/
                                     int mustnormalize; 
};

typedef struct dimension_reduction_algorithm Dimension_reduction_algorithm;

/*! Parameter structure for Principal Component Analysis (PCA), Linear Discriminant Analysis (LDA) or Multidimensional scaling (MDS)*/
struct pca_parameter{
                     /*!Number of features after dimension reduction*/
                     int featurecount;
                     /*! The user may also give the variance explained threshold. This threshold will determine the number of features after dimension reduction*/
                     double variance_explained;
                     /*! Type of dimension reduction. Either one can reduce dimension according to the new number of dimensions, or according to a variance explained value*/
                     REDUCTION_TYPE reduction_type;
};

typedef struct pca_parameter Pca_parameter;
typedef Pca_parameter* Pca_parameterptr;

/*! Parameter structure for ISOMAP, LLE*/
struct isomap_parameter{
                     /*! Other parameters will be taken from the PCA parameter structure*/
                     Pca_parameter ppca;
                     /*! Epsilon threshold for epsilon based neighborhood*/
                     double epsilon;
                     /*! k parameter for determining the neighborhood using k-nn algorithm*/
                     int k;
                     /*! Type of neighborhood used in determining the neighbors of each instance. Can be 0 for epsilon threshold based neighborhood or 1 for k-nn based neighborhood*/
                     NEIGHBORHOOD_TYPE neighborhood_type;
};

typedef struct isomap_parameter Isomap_parameter;
typedef Isomap_parameter* Isomap_parameterptr;

matrix      calculate_m_for_lle(matrix w);
matrix      determine_neighborhood_epsilon(Instanceptr data, double epsilon);
matrix      determine_neighborhood_k(Instanceptr data, int k);
Instanceptr dimension_reduction_isomap(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name);
Instanceptr dimension_reduction_lda(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name);
Instanceptr dimension_reduction_lle(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name);
Instanceptr dimension_reduction_mds(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name);
Instanceptr dimension_reduction_pca(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name);
void        free_dimension_reduction_algorithm_parameters(DIMENSION_REDUCTION_NAME algorithm, void* parameters);
int         get_dimension_reduction_algorithm_index(char* algname);
matrix      local_contributions_of_neighbors(Datasetptr d, Instanceptr traindata, matrix neighborhood);
matrix      local_covariance_matrix(Datasetptr d, Instanceptr inst, Instanceptr traindata, matrix neighborhood, int index);
Instanceptr matrix_to_instancelist(matrix m, Instanceptr data);
double      minimum_epsilon_neighborhood(Instanceptr inst);
int         minimum_k_neighborhood(Instanceptr inst);
matrix      multi_dimensional_scaling(matrix distancematrix);
void*       prepare_dimension_reduction_algorithm_parameters(DIMENSION_REDUCTION_NAME algorithm);
void        reduce_dimension_of_covariance_matrix(matrix* covariance, int howmany, matrix eigenvectors);
Instanceptr reduce_dimension_of_instance(Instanceptr inst, int howmany, matrix eigenvectors);
Instanceptr reduce_dimension_of_instancelist(Instanceptr data, int howmany, matrix eigenvectors);
Instanceptr reduce_dimension_of_instancelist_lle(Instanceptr data, int howmany, matrix eigenvectors, double* eigenvalues);
Instanceptr reduce_dimension_of_instancelist_mds(Instanceptr data, int howmany, matrix eigenvectors, double* eigenvalues);
void        reduce_dimension_of_vector(matrix* vector, int howmany, matrix eigenvectors);

#endif
