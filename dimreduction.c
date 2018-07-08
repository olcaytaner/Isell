#include <math.h>
#include <limits.h>
#include "data.h"
#include "dataset.h"
#include "datastructure.h"
#include "dimreduction.h"
#include "instance.h"
#include "instancelist.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "matrix.h"
#include "messages.h"
#include "parameter.h"

extern Datasetptr current_dataset;

Dimension_reduction_algorithm dimension_reduction_algorithms[DIMENSION_REDUCTION_ALGORITHM_COUNT] = {
   {dimension_reduction_pca, "PCA", YES, YES},
   {dimension_reduction_lda, "LDA", YES, YES},
   {dimension_reduction_mds, "MDS", YES, YES},
   {dimension_reduction_isomap, "ISOMAP", YES, YES},
   {dimension_reduction_lle, "LLE", YES, YES}
};

/**
 * Constructor function for the dimension reduction parameter structure. The function prepares the parameters of the algorithm for dimension reduction. Allocates memory for the parameter pointer and sets properties according to the global parameters of the program.
 * @param[in] algorithm Index of the dimension reduction algorithm
 * @return Parameter structure allocated and initialized according to the algorithm
 */
void* prepare_dimension_reduction_algorithm_parameters(DIMENSION_REDUCTION_NAME algorithm)
{
 /*!Last Changed 12.01.2008*/
 Pca_parameterptr ppca;
 Isomap_parameterptr pisomap;
 switch (algorithm)
  {
   case PCA   :
   case LDA   :
   case MDS   :ppca = safemalloc(sizeof(Pca_parameter), "prepare_dimension_reduction_algorithm_parameters", 4);
               ppca->featurecount = get_parameter_i("reducedfeaturecount");
               ppca->variance_explained = get_parameter_f("variance_explained");
               ppca->reduction_type = get_parameter_l("dimensionreducetype");
               return ppca;
               break;
   case ISOMAP:
   case    LLE:pisomap = safemalloc(sizeof(Isomap_parameter), "prepare_dimension_reduction_algorithm_parameters", 12);
               pisomap->ppca.featurecount = get_parameter_i("reducedfeaturecount");
               pisomap->ppca.variance_explained = get_parameter_f("variance_explained");
               pisomap->ppca.reduction_type = get_parameter_l("dimensionreducetype");
               pisomap->epsilon = get_parameter_f("epsilon");
               pisomap->k = get_parameter_i("nearcount");
               pisomap->neighborhood_type = get_parameter_l("neighborhoodtype");
               return pisomap;
               break;
  }
 return NULL;
}

/**
 * Destructor function for the dimension reduction parameter structure. Frees memory allocated to the parameter structure and if necessary its components.
 * @param[in] algorithm Index of the algorithm
 * @param[in] parameters Pointer to the parameter structure
 */
void free_dimension_reduction_algorithm_parameters(DIMENSION_REDUCTION_NAME algorithm, void* parameters)
{
 /*!Last Changed 12.01.2008*/
 switch (algorithm)
  {
   case PCA   :
   case LDA   :
   case MDS   :
   case ISOMAP:
   case LLE   :safe_free(parameters);
               break;
  }
}

/**
 * Checks if there is a dimension reduction algorithm with a given name.
 * @param[in] algname Name of the algorithm to check
 * @return If the algorithm exists returns its index, otherwise returns -1.
 */
int get_dimension_reduction_algorithm_index(char* algname)
{
 /*!Last Changed 12.01.2008*/
 int i;
 for (i = 0; i < DIMENSION_REDUCTION_ALGORITHM_COUNT; i++)
   if (strIcmp(algname, dimension_reduction_algorithms[i].name) == 0)
     return i;
 return -1;
}

/**
 * Finds the minimum k-neighborhood of a dataset. The minimum k that satisfies: If we think the instances of this dataset as nodes of a graph and if we connect each instance in this dataset to its k-nearest neighbors, the graph will be connected; is the minimum k-neighborhood. To make the search faster, we first search an upper bound for k exponentially (we multiply k by 2 by 2 etc.). Then we set the lower bound upper bound/2 and search the minimum k using binary search.
 * @param[in] inst Link list of instances (dataset)
 * @return Minimum k-neighborhood
 */
int minimum_k_neighborhood(Instanceptr inst)
{
 /*!Last Changed 08.04.2008 rewritten*/
 /*!Last Changed 04.04.2008*/
 int k, size = data_size(inst), upper, lower, mean;
 matrix graph;
 for (k = 1; k < size; k *= 2)
  {
   graph = determine_neighborhood_k(inst, k);
   if (is_graph_connected(graph))
    {
     matrix_free(graph);
     break;
    }
   matrix_free(graph);
  }
 if (k > size)
   upper = size;
 else
   upper = k;
 lower = k / 2;
 mean = (upper + lower) / 2;
 while (mean != lower)
  {
   graph = determine_neighborhood_k(inst, mean);
   if (is_graph_connected(graph))
     upper = mean;
   else
     lower = mean;
   mean = (upper + lower) / 2;     
   matrix_free(graph);
  }
 return upper;
}

/**
 * Finds minimum epsilon neighborhood of a dataset. The minimum epsilon that satisfies: If we think the instances of this dataset as nodes of a graph and if we connect each instance in this dataset to the neighbors those are less that epsilon apart, the graph will be connected; is the minimum epsilon-neighborhood. To make search faster, we first set the upper bound for epsilon as the maximum distance between two instances and the lower bound to 0. Then we make binary search to find the minimum epsilon neighborhood.
 * @param[in] inst Link list of instances
 * @return Minimum epsilon neighborhood
 */
double minimum_epsilon_neighborhood(Instanceptr inst)
{
 /*!Last Changed 04.04.2008*/
 int i; 
 double epsilonmin = 0, epsilonmax, epsilonmean, epsilon;
 matrix graph, distancematrix = distance_matrix(inst);
 epsilonmax = matrix_max(distancematrix);
 matrix_free(distancematrix);
 epsilonmean = (epsilonmin + epsilonmax) / 2;
 epsilon = epsilonmax;
 for (i = 0; i < 30; i++)
  {
   graph = determine_neighborhood_epsilon(inst, epsilonmean);
   if (is_graph_connected(graph))
    {
     epsilon = epsilonmean;
     epsilonmax = epsilonmean;
    }
   else
     epsilonmin = epsilonmean;
   epsilonmean = (epsilonmax + epsilonmin) / 2;
   matrix_free(graph);
  }
 return epsilon;
}

/**
 * Generates a new instance from an instance by dimension reduction using the selected eigenvectors. Allocates memory for the new instance and calculates its feature values.
 * @param[in] inst Instance which will be reduced
 * @param[in] howmany Dimension to reduce
 * @param[in] eigenvectors Eigenvector array. eigenvectors[i] is the i'th most important eigenvector.
 * @return New instance allocated and its feature values calculated
 */
Instanceptr reduce_dimension_of_instance(Instanceptr inst, int howmany, matrix eigenvectors)
{
 /*!Last Changed 12.01.2008*/
 Instanceptr newinst;
 double sum;
 int i, j;
 newinst = empty_instance(howmany + 1);
 for (i = 0; i < howmany; i++)
  {
   sum = 0.0;
   for (j = 0; j < eigenvectors.row; j++)
     sum += eigenvectors.values[j][i] * real_feature(inst, j);
   newinst->values[i].floatvalue = sum;
  }
 newinst->values[howmany].stringindex = give_classno(inst);
 return newinst;
}

/**
 * Generates a new link list of instances (a new dataset) from the original dataset using dimension reduction. Uses reduce_dimension_of_instance for dimension reduction of a single instance.
 * @param[in] data Original link list of instances (dataset)
 * @param[in] howmany Dimension to reduce
 * @param[in] eigenvectors Eigenvector array. eigenvectors[i] is the i'th most important eigenvector.
 * @return New link list of instances (new dataset)
 */
Instanceptr reduce_dimension_of_instancelist(Instanceptr data, int howmany, matrix eigenvectors)
{
 /*!Last Changed 12.01.2008*/
 Instanceptr newlist = NULL, prev = NULL, newinst, tmp;
 tmp = data;
 while (tmp)
  {
   newinst = reduce_dimension_of_instance(tmp, howmany, eigenvectors);
   if (newlist)
     prev->next = newinst;
   else
     newlist = newinst;
   prev = newinst;
   tmp = tmp->next;
  }
 return newlist;
}

/**
 * Converts a matrix to a link list of instances taking the class values from another link list of instances. Each row of a matrix will correspond to an instance.
 * @param[in] m Matrix
 * @param[in] data Link list whose class index values will be used for the class values of the output link list
 * @return A link list of instances
 */
Instanceptr matrix_to_instancelist(matrix m, Instanceptr data)
{
 /*!Last Changed 16.01.2008*/
 Instanceptr newinst, newlist = NULL, tmp, prev = NULL; 
 int i, index = 0;
 tmp = data;
 while (tmp)
  {
   newinst = empty_instance(m.col + 1);
   for (i = 0; i < m.col; i++)
     newinst->values[i].floatvalue = m.values[index][i];
   newinst->values[m.col].stringindex = give_classno(tmp);
   if (newlist)
     prev->next = newinst;
   else
     newlist = newinst;
   prev = newinst;
   tmp = tmp->next;
   index++;
  }
 return newlist;
}

void reduce_dimension_of_covariance_matrix(matrix* covariance, int howmany, matrix eigenvectors)
{
 matrix reduced, treduced, S1;
 reduced = matrix_partial(eigenvectors, 0, eigenvectors.row - 1, 0, howmany);
 treduced = matrix_transpose(reduced);
 matrix_free(reduced);
 S1 = matrix_multiply(treduced, *covariance);
 reduced = matrix_transpose(treduced);
 matrix_free(treduced);
 *covariance = matrix_multiply(S1, reduced);
 matrix_free(S1);
 matrix_free(reduced);
}

void reduce_dimension_of_vector(matrix* vector, int howmany, matrix eigenvectors)
{
 matrix reduced, result;
 reduced = matrix_partial(eigenvectors, 0, eigenvectors.row - 1, 0, howmany);
 result = matrix_multiply(*vector, reduced);
 matrix_free(*vector);
 *vector = result;
}

/**
 * Dimension reduce of a link list of instances given the eigenvectors and eigenvalues according to the Multi Dimensional Scaling. In Multi Dimensional Scaling we use the most important k eigenvectors (eigenvectors having the largest k eigenvalues). 
 * @param[in] data Link list of instances whose dimension will be reduced
 * @param[in] howmany Dimension to reduce
 * @param[in] eigenvectors Eigenvector array. eigenvectors[i] is the i'th most important eigenvector.
 * @param[in] eigenvalues Eigenvalues array sorted. eigenvalues[i] is the i'th largest eigenvalue.
 * @return A link list of instances
 */
Instanceptr reduce_dimension_of_instancelist_mds(Instanceptr data, int howmany, matrix eigenvectors, double* eigenvalues)
{
 /*!Last Changed 18.01.2008*/
 matrix c, x, dsqrt;
 Instanceptr newlist;
 c = matrix_partial(eigenvectors, 0, eigenvectors.row - 1, 0, howmany - 1);
 matrix_free(eigenvectors);
 dsqrt = matrix_diagonal(eigenvalues, howmany); 
 safe_free(eigenvalues);
 matrix_cell_power(&dsqrt, 0.5);
 x = matrix_multiply(c, dsqrt);
 matrix_free(dsqrt);
 matrix_free(c);
 newlist = matrix_to_instancelist(x, data);
 matrix_free(x);
 return newlist;
}

/**
 * Dimension reduce of a link list of instances given the eigenvectors and eigenvalues according to the Locally Linear Embedding. In Locally Linear Embedding we use the least important k eigenvectors (eigenvectors having the smallest k eigenvalues distinct from 0). 
 * @param[in] data Link list of instances whose dimension will be reduced
 * @param[in] howmany Dimension to reduce
 * @param[in] eigenvectors Eigenvector array. eigenvectors[i] is the i'th most important eigenvector.
 * @param[in] eigenvalues Eigenvalues array sorted. eigenvalues[i] is the i'th largest eigenvalue.
 * @return A link list of instances
 */
Instanceptr reduce_dimension_of_instancelist_lle(Instanceptr data, int howmany, matrix eigenvectors, double* eigenvalues)
{
 /*!Last Changed 19.01.2008*/
 matrix M;
 Instanceptr newlist;
 int i = eigenvectors.row - 1;
 while (eigenvalues[i] == 0.0)
   i--;
 M = matrix_partial(eigenvectors, 0, eigenvectors.row - 1, i - howmany + 1, i);
 matrix_free(eigenvectors);
 safe_free(eigenvalues);
 newlist = matrix_to_instancelist(M, data);
 matrix_free(M);
 return newlist;
}

/**
 * Determine epsilon neighborhood matrix between instances in the dataset.
 * @param[in] data Dataset as a link list of instances
 * @param[in] epsilon Epsilon threshold
 * @return matrix[i][j] is 1 if the distance between the instance i and the instance j is smaller than epsilon, 0 otherwise. matrix[i][i] = 0 for all i's.
 */
matrix determine_neighborhood_epsilon(Instanceptr data, double epsilon)
{
 /*!Last Changed 19.01.2008*/
 int i, j, size;
 Instanceptr temp1, temp2;
 matrix result;
 size = data_size(data);
 result = matrix_alloc(size, size);
 for (i = 0, temp1 = data; i < size; i++, temp1 = temp1->next)
   for (j = 0, temp2 = data; j < size; j++, temp2 = temp2->next)
     if (i != j && distance_between_instances(temp1, temp2) < epsilon)
       result.values[i][j] = 1;
     else
       result.values[i][j] = 0;
 return result;
}

/**
 * Determine k neighborhood matrix between instances in the dataset.
 * @param[in] data Dataset as a link list of instances
 * @param[in] k Number of neighbors
 * @return matrix[i][j] is 1, where instance j is one of the k nearest neighbors of instance i, 0 otherwise. To make the matrix symmetric, we make matrix[i][j] 1 if matrix[j][i] is 1.
 */
matrix determine_neighborhood_k(Instanceptr data, int k)
{
 /*!Last Changed 08.04.2008*/
 /*!Last Changed 19.01.2008*/
 int i, j, size;
 double threshold;
 Instanceptr temp1, temp2;
 matrix result;
 size = data_size(data);
 result = matrix_alloc(size, size);
 for (i = 0, temp1 = data; i < size; i++, temp1 = temp1->next)
  {
   threshold = find_k_nearest_neighbor_distance(data, temp1, k);  
   for (j = 0, temp2 = data; j < size; j++, temp2 = temp2->next)
     if (i != j && distance_between_instances(temp1, temp2) < threshold + DBL_DELTA)
       result.values[i][j] = 1;
     else
       result.values[i][j] = 0;
  }
 for (i = 0; i < size; i++)
   for (j = i + 1; j < size; j++)
     if (result.values[i][j] != result.values[j][i])
       result.values[i][j] = result.values[j][i] = 1;
 return result;
}

/**
 * ISOMAP dimension reduction algorithm.
 * @param[in] d Pointer to the dataset (properties) on which dimension reduction will be applied
 * @param[in] traindata Data as a link list of instances on which dimension reduction will be applied
 * @param[in] parameters Parameters of the ISOMAP dimension reduction algorithm. 
 * @param[out] new_dataset Pointer to the new dataset structure which will be allocated and the values will be set
 * @param[in] new_dataset_name Name of the dimension reduced dataset
 * @return New link list of instances (as dimensions reduced using ISOMAP)
 */
Instanceptr dimension_reduction_isomap(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name)
{
 /*!Last Changed 18.01.2008*/
 int howmany;
 matrix eigenvectors, distancematrix, b;
 double *eigenvalues;
 Isomap_parameterptr pisomap = (Isomap_parameterptr) parameters;
 Instanceptr newlist;
 switch (pisomap->neighborhood_type)
  {
   case NEIGHBORHOOD_TYPE_EPSILON:distancematrix = distance_matrix_with_threshold(traindata, pisomap->epsilon);
                                  break;
   case       NEIGHBORHOOD_TYPE_K:distancematrix = distance_matrix_with_k(traindata, pisomap->k);
                                  break;
  } 
 floyds_all_pairs_shortest_path_algorithm(&distancematrix);
 if (!graph_connected(distancematrix))
  {
   printf(m1393);
   matrix_free(distancematrix);
   return NULL;
  }
 b = multi_dimensional_scaling(distancematrix);
 matrix_free(distancematrix);
 switch (pisomap->ppca.reduction_type)
  {
   case REDUCTION_TYPE_FEATURE :eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, b);
                                howmany = pisomap->ppca.featurecount;
                                break;
   case REDUCTION_TYPE_VARIANCE:eigenvalues = find_eigenvalues_eigenvectors(&eigenvectors, b, &howmany, pisomap->ppca.variance_explained);
                                break;
  }
 matrix_free(b);
 newlist = reduce_dimension_of_instancelist_mds(traindata, howmany, eigenvectors, eigenvalues);
 *new_dataset = dimension_reduced_dataset(d, howmany + 1, newlist, new_dataset_name);
 return newlist;
}

/**
 * (Linear Discriminant Analysis) LDA dimension reduction algorithm.
 * @param[in] d Pointer to the dataset (properties) on which dimension reduction will be applied
 * @param[in] traindata Data as a link list of instances on which dimension reduction will be applied
 * @param[in] parameters Parameters of the LDA dimension reduction algorithm. 
 * @param[out] new_dataset Pointer to the new dataset structure which will be allocated and the values will be set
 * @param[in] new_dataset_name Name of the dimension reduced dataset
 * @return New link list of instances (as dimensions reduced using LDA)
 */
Instanceptr dimension_reduction_lda(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name)
{
 /*!Last Changed 16.01.2008*/
 int howmany = 0, reversed;
 matrix eigenvectors, sw, sb, W;
 double *eigenvalues;
 Pca_parameterptr ppca = (Pca_parameterptr) parameters;
 Instanceptr newlist;
 sw = within_class_matrix(traindata);
 reversed = matrix_inverse(&sw);
 if (!reversed)
   printf("Matrix inverse problem\n");
 sb = between_class_matrix(traindata);
 W = matrix_multiply(sw, sb);
 matrix_free(sw);
 matrix_free(sb);
 switch (ppca->reduction_type)
  {
   case REDUCTION_TYPE_FEATURE :if (is_symmetric(W))
                                  eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, W);
                                else
                                  eigenvalues = find_eigenvalues_eigenvectors_unsymmetric(&eigenvectors, W);
                                if (ppca->featurecount > d->classno - 1)
                                  howmany = d->classno - 1;
                                else
                                  howmany = ppca->featurecount;
                                break;
   case REDUCTION_TYPE_VARIANCE:if (is_symmetric(W))
                                  eigenvalues = find_eigenvalues_eigenvectors(&eigenvectors, W, &howmany, ppca->variance_explained);
                                else
                                  eigenvalues = find_eigenvalues_eigenvectors_unsymmetric(&eigenvectors, W);
                                break;
  }
 safe_free(eigenvalues);
 newlist = reduce_dimension_of_instancelist(traindata, howmany, eigenvectors);
 *new_dataset = dimension_reduced_dataset(d, howmany + 1, newlist, new_dataset_name);
 matrix_free(eigenvectors);
 matrix_free(W);
 return newlist;
}

matrix local_covariance_matrix(Datasetptr d, Instanceptr inst, Instanceptr traindata, matrix neighborhood, int index)
{
 /*!Last Changed 19.01.2008*/
 matrix C;
 int i, j, t, k, p, size = matrix_sum_row(neighborhood, index);
 Instanceptr temp1, temp2;
 double sum;
 C = matrix_alloc(size, size);
 for (i = 0, j = 0, temp1 = traindata; i < neighborhood.row; i++, temp1 = temp1->next)
   if (neighborhood.values[index][i] == 1)
    {
     for (t = 0, k = 0, temp2 = traindata; t < neighborhood.row; t++, temp2 = temp2->next)
       if (neighborhood.values[index][t] == 1)
        {
         sum = 0.0;
         for (p = 0; p < d->multifeaturecount - 1; p++)
           sum += (real_feature(inst, p) - real_feature(temp1, p)) * (real_feature(inst, p) - real_feature(temp2, p));
         C.values[j][k] = sum;
         k++;
        }
     j++;
    }
 return C;
}

matrix local_contributions_of_neighbors(Datasetptr d, Instanceptr traindata, matrix neighborhood)
{
 /*!Last Changed 19.01.2008*/
 matrix w, C;
 Instanceptr tmp = traindata;
 double denominator, nominator;
 int i = 0, j, t;
 w = matrix_alloc(neighborhood.row, neighborhood.col);
 while (tmp)
  {
   C = local_covariance_matrix(d, tmp, traindata, neighborhood, i);
   matrix_inverse(&C);
   denominator = matrix_sum_of_elements(C);
   t = 0;
   for (j = 0; j < neighborhood.col; j++)
     if (neighborhood.values[i][j])
      {
       nominator = matrix_sum_row(C, t);
       w.values[i][j] = nominator / denominator;
       t++;
      }
     else
       w.values[i][j] = 0.0;
   i++;
   tmp = tmp->next;
   matrix_free(C);
  }
 return w;
}

matrix calculate_m_for_lle(matrix w)
{
 /*!Last Changed 19.01.2008*/
 int i, j, k;
 double sum;
 matrix M = matrix_alloc(w.row, w.col);
 for (i = 0; i < w.row; i++)
   for (j = 0; j < w.col; j++)
    {
     sum = kronecker_delta(i, j) - w.values[i][j] - w.values[j][i];
     for (k = 0; k < w.row; k++)
       sum += w.values[k][i] * w.values[k][j];
     M.values[i][j] = sum;
    }
 return M;
}

/**
 * (Locally Linear Embedding) LLE dimension reduction algorithm.
 * @param[in] d Pointer to the dataset (properties) on which dimension reduction will be applied
 * @param[in] traindata Data as a link list of instances on which dimension reduction will be applied
 * @param[in] parameters Parameters of the LLE dimension reduction algorithm. 
 * @param[out] new_dataset Pointer to the new dataset structure which will be allocated and the values will be set
 * @param[in] new_dataset_name Name of the dimension reduced dataset
 * @return New link list of instances (as dimensions reduced using LLE)
 */
Instanceptr dimension_reduction_lle(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name)
{
 /*!Last Changed 19.01.2008*/
 int howmany;
 matrix eigenvectors, neighborhood, w, M;
 double *eigenvalues;
 Isomap_parameterptr pisomap = (Isomap_parameterptr) parameters;
 Instanceptr newlist; 
 switch (pisomap->neighborhood_type)
  {
   case NEIGHBORHOOD_TYPE_EPSILON:neighborhood = determine_neighborhood_epsilon(traindata, pisomap->epsilon);
                                  break;
   case       NEIGHBORHOOD_TYPE_K:neighborhood = determine_neighborhood_k(traindata, pisomap->k);
                                  break;
  }
 w = local_contributions_of_neighbors(d, traindata, neighborhood);
 matrix_free(neighborhood);
 M = calculate_m_for_lle(w);
 matrix_free(w);
 switch (pisomap->ppca.reduction_type)
  {
   case REDUCTION_TYPE_FEATURE :eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, M);
                                howmany = pisomap->ppca.featurecount;
                                break;
   case REDUCTION_TYPE_VARIANCE:eigenvalues = find_eigenvalues_eigenvectors(&eigenvectors, M, &howmany, 1 - pisomap->ppca.variance_explained);
                                break;
  }
 matrix_free(M);
 newlist = reduce_dimension_of_instancelist_lle(traindata, howmany, eigenvectors, eigenvalues);
 *new_dataset = dimension_reduced_dataset(d, howmany + 1, newlist, new_dataset_name);
 return newlist;
}

matrix multi_dimensional_scaling(matrix distancematrix)
{
 /*!Last Changed 17.01.2008*/
 double *ddots, *drdot, ddotdot;
 matrix b;
 int r, s, N = distancematrix.row;
 b = matrix_alloc(N, N);
 matrix_cell_square(&distancematrix);
 ddotdot = matrix_sum_of_elements(distancematrix) / (N * N);
 ddots = matrix_average_columns(distancematrix);
 drdot = matrix_average_rows(distancematrix);
 for (r = 0; r < N; r++)
   for (s = 0; s < N; s++)
     b.values[r][s] = 0.5 * (drdot[r] + ddots[s] - ddotdot - distancematrix.values[r][s]);
 safe_free(ddots);
 safe_free(drdot);
 return b;
}

/**
 * (Multi Dimensional Scaling) MDS dimension reduction algorithm.
 * @param[in] d Pointer to the dataset (properties) on which dimension reduction will be applied
 * @param[in] traindata Data as a link list of instances on which dimension reduction will be applied
 * @param[in] parameters Parameters of the MDS dimension reduction algorithm. 
 * @param[out] new_dataset Pointer to the new dataset structure which will be allocated and the values will be set
 * @param[in] new_dataset_name Name of the dimension reduced dataset
 * @return New link list of instances (as dimensions reduced using MDS)
 */
Instanceptr dimension_reduction_mds(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name)
{
 /*!Last Changed 17.01.2008*/
 int howmany;
 matrix eigenvectors, distancematrix, b;
 double *eigenvalues;
 Pca_parameterptr ppca = (Pca_parameterptr) parameters;
 Instanceptr newlist;
 distancematrix = distance_matrix(traindata);
 b = multi_dimensional_scaling(distancematrix);
 matrix_free(distancematrix);
 switch (ppca->reduction_type)
  {
   case REDUCTION_TYPE_FEATURE :eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, b);
                                howmany = ppca->featurecount;
                                break;
   case REDUCTION_TYPE_VARIANCE:eigenvalues = find_eigenvalues_eigenvectors(&eigenvectors, b, &howmany, ppca->variance_explained);
                                break;
  }
 matrix_free(b);
 newlist = reduce_dimension_of_instancelist_mds(traindata, howmany, eigenvectors, eigenvalues);
 *new_dataset = dimension_reduced_dataset(d, howmany + 1, newlist, new_dataset_name);
 return newlist;
}

/**
 * Principal Component Analysis (PCA) dimension reduction algorithm.
 * @param[in] d Pointer to the dataset (properties) on which dimension reduction will be applied
 * @param[in] traindata Data as a link list of instances on which dimension reduction will be applied
 * @param[in] parameters Parameters of the PCA dimension reduction algorithm. 
 * @param[out] new_dataset Pointer to the new dataset structure which will be allocated and the values will be set
 * @param[in] new_dataset_name Name of the dimension reduced dataset
 * @return New link list of instances (as dimensions reduced using PCA)
 */
Instanceptr dimension_reduction_pca(Datasetptr d, Instanceptr traindata, void* parameters, Datasetptr* new_dataset, char* new_dataset_name)
{
 /*!Last Changed 16.01.2008*/
 int howmany = 0;
 matrix eigenvectors, cov;
 double *eigenvalues;
 Instance mean;
 Pca_parameterptr ppca = (Pca_parameterptr) parameters;
 Instanceptr newlist;
 mean = find_mean(traindata);
 cov = covariance(traindata, mean);
 safe_free(mean.values);
 switch (ppca->reduction_type)
  {
   case REDUCTION_TYPE_FEATURE :eigenvalues = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, cov);
                                if (ppca->featurecount <= d->multifeaturecount - 1)
                                  howmany = ppca->featurecount;
                                break;
   case REDUCTION_TYPE_VARIANCE:eigenvalues = find_eigenvalues_eigenvectors(&eigenvectors, cov, &howmany, ppca->variance_explained);
                                break;
  }
 safe_free(eigenvalues);
 newlist = reduce_dimension_of_instancelist(traindata, howmany, eigenvectors);
 *new_dataset = dimension_reduced_dataset(d, howmany + 1, newlist, new_dataset_name);
 matrix_free(eigenvectors);
 matrix_free(cov);
 return newlist;
}
