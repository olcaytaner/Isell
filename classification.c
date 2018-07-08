#include<stdlib.h>
#include<limits.h>
#include<float.h>
#include "cart.h"
#include "c45rules.h"
#include "cn2.h"
#include "classification.h"
#include "clustering.h"
#include "data.h"
#include "dataset.h"
#include "decisionforest.h"
#include "decisiontree.h"
#include "dimreduction.h"
#include "divs.h"
#include "hmm.h"
#include "instance.h"
#include "instancelist.h"
#include "ktree.h"
#include "lang.h"
#include "math.h"
#include "matrix.h"
#include "messages.h"
#include "lerils.h"
#include "libarray.h"
#include "libfile.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "mlp.h"
#include "model.h"
#include "multivariaterule.h"
#include "multivariatetree.h"
#include "nodeboundedtree.h"
#include "omnivariaterule.h"
#include "omnivariatetree.h"
#include "parameter.h"
#include "part.h"
#include "perceptron.h"
#include "ripper.h"
#include "rise.h"
#include "rbf.h"
#include "rule.h"
#include "softtree.h"
#include "statistics.h"
#include "svm.h"
#include "svmprepare.h"
#include "svmsimplex.h"
#include "svmtree.h"
#include "typedef.h"
#include "univariaterule.h"
#include "univariatetree.h"

extern Datasetptr current_dataset;
extern FILE* output;
extern Instance testmean;
extern Instance testvariance;

/**
 * Reads  file and returns an array of matrices containing posteriors. \n
 * Posterior matrix: There is a row for each test data. \n
 * posterior probability of 1. class posterior probability of 2. class ... posterior probability of c. class Real class of the test instance \n
 * posterior probability of 1. class posterior probability of 2. class ... posterior probability of c. class Real class of the test instance \n
 * ... 
 * @warning Posterior file must contain "####### CURRENT RUN COUNT" string
 * @param[in] numfolds Number of folds in the posterior file
 * @param[in] fname Name of the file containing the posterior probabilities
 * @return Array of posterior matrices
 */
matrixptr read_posteriors(int numfolds, char *fname)
{
 /*!Last Changed 24.03.2006 Aydin*/
 FILE *infile;
 int i;
 char searchstring[100];
 matrixptr tmp = NULL;
 infile = fopen(fname, "r");
 if (infile)
   tmp = safemalloc(numfolds * sizeof(matrix), "read_posteriors", 6);
	else 
		 return NULL;
 for (i = 0 ; i < numfolds ; i++)
  {
   sprintf(searchstring, "####### CURRENT RUN COUNT %d", i + 1);
   infile = find_string_in_file(infile, searchstring);
   fscanf(infile, "%c", &searchstring[99]);
   tmp[i] = read_matrix_filehandle(infile);
  }
 fclose(infile);
 return(tmp);
}

/**
 * Prints confusion matrix to the output file
 * @param[in] outfile Name of the output file
 * @param[in] confusionmatrix Confusion matrix
 * @param[in] classno Number of classes in the dataset (number of rows and columns in the confusion matrix)
 */
void print_confusion(FILE* outfile, int** confusionmatrix, int classno)
{
 /*!Last Changed 02.08.2007*/
 int i, j;
 fprintf(outfile, "%d %d\n", classno, classno);
 for (i = 0; i < classno; i++)
  {
   for (j = 0; j < classno; j++)
     fprintf(outfile, "%d ", confusionmatrix[i][j]);
   fprintf(outfile, "\n");
  }
}

/**
 * Prints 2d confusion matrix to the output file
 * @param[in] outfile Name of the output file
 * @param[in] confusionmatrix Confusion matrix
 */
void print_confusion_2d(FILE* outfile, int** confusionmatrix)
{
 fprintf(outfile, "%d %d %d %d\n", confusionmatrix[0][0], confusionmatrix[0][1], confusionmatrix[1][1], confusionmatrix[1][0]);
}

/**
 * Prints posterior probabities to the output file
 * Posterior matrix: There is a row for each test data.
 * posterior probability of 1. class posterior probability of 2. class ... posterior probability of c. class Real class of the test instance
 * posterior probability of 1. class posterior probability of 2. class ... posterior probability of c. class Real class of the test instance
 * ...
 * @param[in] outfile Name of the output file
 * @param[in] posteriors Posterior probabilities 1.dimension: index of the test instance 2.dimension: index of the class
 * @param[in] testsize Number of test instances
 * @param[in] data Test data
 */
void print_posterior(FILE* outfile, double** posteriors, int testsize, Instanceptr data)
{
 /*!Last Changed 01.11.2005*/
 int i = 0, j, classno;
 char pst[READLINELENGTH];
 Instanceptr tmp = data;
 fprintf(outfile, "# %d %d\n", testsize, current_dataset->classno + 1);
 while (tmp)
  {
   classno = give_classno(tmp);
   for (j = 0; j < current_dataset->classno; j++)
    {
     set_precision(pst, NULL, " ");
     fprintf(outfile, pst, posteriors[i][j]);
    }
   fprintf(outfile, "%d\n", classno);
   tmp = tmp->next;
   i++;
  }
}

/**
 * Prints accuracy or error results, runtime for a single run to the output file
 * @param[in] testdata Link list containing the test data
 * @param[in] result Structure containing the results
 * @param[in] runtime Runtime of the algorithm in seconds
 * @param[in] printbinary If printbinary is YES, the output for each test instance will be printed, otherwise only the error or accuracy will be printed
 * @param[in] displayruntime If displayruntime is YES, runtime will be prined, otherwise not printed
 * @param[in] accuracy If accuracy is YES, the accuracy of the algorithm will be printed, otherwise the error rate of the algorithm will be printed
 * @param[in] displayclassresulton If displayclassresulton is YES, the accuracy or error results for each class will also be printed
 */
void print_classification_results(Instanceptr testdata, Classificationresultptr result, double runtime, int printbinary, int displayruntime, int accuracy, int displayclassresulton)
{
 /*!Last Changed 22.05.2005*/
 /*!Last Changed 17.08.2003 Displays accuracy or error results*/
 int i;
 char pst[READLINELENGTH];
 Instanceptr tmp = testdata;
 if (result->datasize == 0)
   return;

 if (printbinary)
  {
   fprintf(output,"%d\n", result->datasize);
   while (tmp)
    {
     fprintf(output,"%d", tmp->classified);
     tmp = tmp->next;
    }
  }
 else
  {
   if (displayruntime)
    {
     set_precision(pst, NULL, " %.3lf\n");
     if (accuracy)
       fprintf(output, pst, 100 * (result->performance + 0.0) / result->datasize, runtime); 
     else
       fprintf(output, pst, 100 - 100 * (result->performance + 0.0) / result->datasize, runtime);   
	printf("error %f\n",100 - 100 * (result->performance + 0.0) / result->datasize);     
    }
   else
    {
     set_precision(pst, NULL, "\n");
     if (accuracy)
       fprintf(output, pst, 100 * (result->performance + 0.0) / result->datasize); 
     else
       fprintf(output, pst, 100 - 100 * (result->performance + 0.0) / result->datasize); 
    }
   if (displayclassresulton)
    {
     set_precision(pst, NULL, "\n");
     fprintf(output, "----------\n");
     if (accuracy)
       for (i = 0; i < current_dataset->classno; i++)
         if (result->class_counts[i] != 0)
           fprintf(output, pst, 100 * (result->class_performance[i] + 0.0) / result->class_counts[i]);
         else
           fprintf(output, pst, 0.0);
     else
       for (i = 0; i < current_dataset->classno; i++)
         if (result->class_counts[i] != 0)
           fprintf(output, pst, 100 - 100 * (result->class_performance[i] + 0.0) / result->class_counts[i]);
         else
           fprintf(output, pst, 100.0);
     fprintf(output, "----------\n");
    }
  }
}

/**
 * Constructor of classification result struct. Allocates memory for confusion matrix, class counts, performances of classes, posterior probabilities.
 * @param[in] testsize Number of test instances in the dataset
 * @return Empty classification result struct.
 */
Classificationresultptr performance_initialize(int testsize)
{
 /*!Last Changed 01.08.2007 added confusion matrix*/
 /*!Last Changed 02.02.2004 added safecalloc*/
 /*!Last Changed 19.03.2002*/
 int i;
 Classificationresultptr result;
 result = safemalloc(sizeof(Classificationresult), "performance_initialize", 2);
 result->performance = 0;
 result->confusion_matrix = (int**) safecalloc_2d(sizeof(int*), sizeof(int), current_dataset->classno, current_dataset->classno, "performance_initialize", 4);
 result->class_performance = safecalloc(current_dataset->classno, sizeof(int), "performance_initialize", 5);
 result->class_counts = safecalloc(current_dataset->classno, sizeof(int), "performance_initialize", 6);
 result->classno = current_dataset->classno;
 result->datasize = testsize;
 result->posteriors = (double **) safecalloc_2d(sizeof(double *), sizeof(double), testsize, current_dataset->classno, "performance_initialize", 8);
 result->hingeloss = 0.0;
 for (i = 0; i < 3; i++)
   result->knnloss[i] = 0.0;
 return result;
}

/**
 * Destructor of classification result struct. Frees memory allocated for confusion matrix, class counts, performances of classes, posterior probabilities and the result itself.
 * @param[in] result Classification result struct
 */
void free_performance(Classificationresultptr result){
 /*!Last Changed 01.08.2007*/
 free_2d((void **) result->confusion_matrix, result->classno);
 safe_free(result->class_performance);
 safe_free(result->class_counts);
 free_2d((void **) result->posteriors, result->datasize);
 safe_free(result);
}

/**
 * Training algorithm for HMM. Three learning methodologies exists. In LEARNING_SEPARATE, HMM model for each class is learnt separately. In LEARNING_TOGETHER, the performances of other classes are also considered while learning each class. Therefore, all models of all classes are learnt together. In LEARNING_FIXED, the HMM model parameters are fixed, i.e. 3 HMM states with 2 gaussian mixtures in each. For each class, that model is trained using Baum-Welch algorithm.
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the HMM algorithm
 * @return Model of the HMM algorithm. Contains one HMM model for each class.
 */
void* train_hmm(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 16.08.2007*/
 /*!Last Changed 02.04.2007*/
 Hmm_parameterptr p;
 Hmmptr* model;
 Instanceptr *trclasses, *cvclasses = NULL;
 int i;
 p = (Hmm_parameterptr) parameters;
 trclasses = divide_instancelist_into_classes(trdata);
 if (p->learningtype == LEARNING_SEPARATE)
   cvclasses = divide_instancelist_into_classes(cvdata);
 model = safemalloc(current_dataset->classno * sizeof(Hmmptr), "train_hmm", 9);
 switch (p->learningtype)
  {
   case LEARNING_SEPARATE:for (i = 0; i < current_dataset->classno; i++)
                           {
                            switch (p->statetype)
                             {
                              case          STATE_DISCRETE:
                              case          STATE_GAUSSIAN:
                              case         STATE_DIRICHLET:model[i] = best_hmm_for_single(trclasses[i], cvclasses[i], p);
                                                           break;
                              case  STATE_GAUSSIAN_MIXTURE:
                              case STATE_DIRICHLET_MIXTURE:model[i] = best_hmm_for_mixture(trclasses[i], cvclasses[i], p);
                                                           break;
                             }
                            printf("Class %d is finished\n", i + 1);
                           }
                          break;
   case LEARNING_TOGETHER:best_hmm_for_together(model, trclasses, *cvdata, p);
                          break;
   case    LEARNING_FIXED:for (i = 0; i < current_dataset->classno; i++)
                           {
                            model[i] = random_hmm(p->statecounts[i], p->statetype, p->componentcounts[i], p->hmmtype, trclasses[i]);
                            baumwelch(model[i], trclasses[i], p->iteration);
                           }
                          break;
  }
 merge_instancelist_groups(trdata, trclasses, current_dataset->classno);
 safe_free(trclasses);
 if (p->learningtype == LEARNING_SEPARATE)
  {
   merge_instancelist_groups(cvdata, cvclasses, current_dataset->classno);
   safe_free(cvclasses);
  }
 return model;
}

/**
 * Testing algorithm for HMM.
 * @param[in] data Test instance.
 * @param[in] model Model of the HMM algorithm.
 * @param[in] parameters Parameters of the HMM algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_hmm(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 14.04.2007 added test_hmm_instance*/
 /*!Last Changed 02.04.2007*/
 Hmmptr* hmmarray;
 Prediction predicted;
 hmmarray = (Hmmptr*) model;
 predicted.classno = test_hmm_instance(data, hmmarray);
 return predicted;
}

/**
 * Training algorithm for KNN. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the KNN algorithm
 * @return Model of the KNN algorithm.
 */
void* train_knn(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 25.10.2004*/
 /*!Last Changed 21.03.2002*/
 return *trdata;
}

/**
 * Testing algorithm for KNN.
 * @param[in] data Test instance.
 * @param[in] model Model of the KNN algorithm.
 * @param[in] parameters Parameters of the KNN algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_knn(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 05.01.2009 added normalize_array*/
 /*!Last Changed 01.11.2005 added posteriors*/
 /*!Last Changed 25.10.2004*/
 /*!Last Changed 21.03.2002*/
 int *ccounts, j, classno, sum;
 Prediction predicted;
 Instanceptr *nearestneighbors, train;
 Knn_parameterptr p;
 p = (Knn_parameterptr) parameters;
 train = (Instanceptr) model;
 ccounts = safemalloc(current_dataset->classno * sizeof(int), "test_knn", 7);
 for (j = 0; j < current_dataset->classno; j++)
   ccounts[j] = 0;
 if (current_dataset->storetype == STORE_KERNEL)
   nearestneighbors = find_all_neighbors(train, data, &(p->nearcount));
 else
   nearestneighbors = find_nearest_neighbors(train, data, p->nearcount);
 predicted.knnloss[0] = 0;
 predicted.knnloss[1] = 0;
 predicted.knnloss[2] = p->nearcount;
 classno = give_classno(data);
 sum = 0;
 for (j = 0; j < p->nearcount; j++)
  {
   if (classno == give_classno(nearestneighbors[j]))
    {
     predicted.knnloss[0] += 1 / (j + 1.0);
     sum--;
     if (sum > 0)
       predicted.knnloss[2]++;
     else
       if (sum < 0)
         predicted.knnloss[2]--;
    }
   else
    {
     predicted.knnloss[0] -= 1 / (j + 1.0);
     sum++;
     if (sum > 0)
       predicted.knnloss[2]++;
     else
       if (sum < 0)
         predicted.knnloss[2]--;
    }
   predicted.knnloss[1] -= sum / (j + 1.0);
   ccounts[give_classno(nearestneighbors[j])]++;
  }
 predicted.classno = find_max_in_list(ccounts, current_dataset->classno);
 normalize_array(ccounts, posterior, current_dataset->classno);
 safe_free(nearestneighbors);
 safe_free(ccounts);
 return predicted;
}

/**
 * Training algorithm for the simplest classifier. It only returns most occuring class. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the SELECTMAX algorithm
 * @return Model of the SELECTMAX algorithm.
 */
void* train_select_max(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 04.08.2005*/
 /*!Last Changed 14.07.2001*/
 int i, *train_counts, datacount = data_size(*trdata);
 Model_selectmaxptr m;
 m = safemalloc(sizeof(Model_selectmax), "train_select_max", 3); 
 m->priors = safemalloc(current_dataset->classno * sizeof(double), "train_select_max", 4);
 train_counts = find_class_counts(*trdata);
 for (i = 0; i < current_dataset->classno; i++)
   m->priors[i] = train_counts[i] / (datacount + 0.0);
 m->classno = find_max_in_list(train_counts, current_dataset->classno);
 safe_free(train_counts);
 return m;
}

/**
 * Testing algorithm for SELECTMAX.
 * @param[in] data Test instance.
 * @param[in] model Model of the SELECTMAX algorithm.
 * @param[in] parameters Parameters of the SELECTMAX algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_select_max(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 01.11.2005*/
 /*!Last Changed 04.08.2005*/
 /*!Last Changed 14.07.2001*/
 Prediction predicted;
 int i;
 Model_selectmaxptr m;
 m = (Model_selectmaxptr)model;
 predicted.classno = m->classno;
 for (i = 0; i < current_dataset->classno; i++)
   posterior[i] = m->priors[i];
 return predicted;
}

void* train_randomforest(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 int i;
 Randomforest_parameterptr p;
 Decisionnodeptr* forest;
 Decisionnodeptr rootnode;
 Instanceptr data;
 p = (Randomforest_parameterptr) parameters;
 forest = safemalloc(p->forestsize * sizeof(Decisionnodeptr), "train_randomforest", 4);
 for (i = 0; i < p->forestsize; i++)
  {
   rootnode = createnewnode(NULL, 1);
   rootnode->instances = bootstrap_sample_without_stratification(*trdata);
   create_randomforestchildren(rootnode, p);
   data = NULL;
   accumulate_instances_tree(&data, rootnode);
   deallocate(data);
   forest[i] = rootnode;
  }
 return forest;
}

Prediction test_randomforest(Instanceptr data, void* model, void* parameters, double* posterior)
{
 int i;
 double* votes;
 Randomforest_parameterptr p;
 Decisionnodeptr* forest;
 Prediction predicted, predictedc45;
 forest = (Decisionnodeptr*) model;
 p = (Randomforest_parameterptr) parameters;
 votes = safecalloc(current_dataset->classno, sizeof(double), "test_randomforest", 5);
 for (i = 0; i < p->forestsize; i++) 
  {
   predictedc45 = test_c45(data, forest[i], parameters, posterior);
   votes[predictedc45.classno]++;
  }
 normalize_array_of_doubles(votes, current_dataset->classno);
 memcpy(posterior, votes, current_dataset->classno * sizeof(double));
 safe_free(votes);
 predicted.classno = findMaxOutput(posterior, current_dataset->classno);
 return predicted;
}

void* train_kforest(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 int i;
 Kforest_parameterptr p;
 Decisionnodeptr* forest;
 Decisionnodeptr rootnode;
 Instanceptr data;
 p = (Kforest_parameterptr) parameters;
 forest = safemalloc(p->forestsize * sizeof(Decisionnodeptr), "train_randomforest", 4);
 for (i = 0; i < p->forestsize; i++)
  {
   rootnode = createnewnode(NULL, 1);
   rootnode->instances = bootstrap_sample_without_stratification(*trdata);
   create_kforestchildren(rootnode, p);
   data = NULL;
   accumulate_instances_tree(&data, rootnode);
   deallocate(data);
   forest[i] = rootnode;
  }
 return forest;
}

Prediction test_kforest(Instanceptr data, void* model, void* parameters, double* posterior)
{
 int i;
 double* votes;
 Kforest_parameterptr p;
 Decisionnodeptr* forest;
 Prediction predicted, predictedc45;
 forest = (Decisionnodeptr*) model;
 p = (Kforest_parameterptr) parameters;
 votes = safecalloc(current_dataset->classno, sizeof(double), "test_randomforest", 5);
 for (i = 0; i < p->forestsize; i++)
  {
   predictedc45 = test_c45(data, forest[i], parameters, posterior);
   votes[predictedc45.classno]++;
  }
 normalize_array_of_doubles(votes, current_dataset->classno);
 memcpy(posterior, votes, current_dataset->classno * sizeof(double));
 safe_free(votes);
 predicted.classno = findMaxOutput(posterior, current_dataset->classno);
 return predicted;
}

void make_tree_stump(Decisionnodeptr rootnode)
{
 int count, i;
 if (!make_children(rootnode))
  {
   setup_node_properties(rootnode);
   make_leaf_node(rootnode);
   return;
  }
 switch (current_dataset->features[rootnode->featureno].type)
  {
   case INTEGER:
   case REAL   :setup_node_properties(rootnode->left);
                make_leaf_node(rootnode->left);
                setup_node_properties(rootnode->right);
                make_leaf_node(rootnode->right);
                break;
   case STRING :count = current_dataset->features[rootnode->featureno].valuecount;
                for (i = 0; i < count; i++)
                 {
                  setup_node_properties(&(rootnode->string[i]));
                  make_leaf_node(&(rootnode->string[i]));
                 }
                break;
  }
}

void* train_c45stump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Decisionnodeptr rootnode;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 rootnode->conditiontype = UNIVARIATE_CONDITION;
 rootnode->featureno = find_best_feature(rootnode->instances, &(rootnode->split));
 make_tree_stump(rootnode);
 *trdata = NULL;
 return rootnode;
}

Prediction test_c45stump(Instanceptr data, void* model, void* parameters, double* posterior)
{
 Decisionnodeptr currentnode, root = (Decisionnodeptr) model;
 Prediction predicted;
 currentnode = root;
 while (currentnode->featureno != LEAF_NODE)
   currentnode = next_node(currentnode, data);
 normalize_array(currentnode->counts, posterior, current_dataset->classno);
 predicted.classno = currentnode->classno;
 return predicted;
}

void* train_ldtstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Decisionnodeptr rootnode;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 rootnode->conditiontype = UNIVARIATE_CONDITION;
 rootnode->featureno = find_best_ldt_feature(rootnode->instances, &(rootnode->split));
 make_tree_stump(rootnode);
 *trdata = NULL;
 return rootnode;
}

Prediction test_ldtstump(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_c45stump(data, model, parameters, posterior);
}

void* train_multildtstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Decisionnodeptr rootnode;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 rootnode->conditiontype = MULTIVARIATE_CONDITION;
 rootnode->lineard = find_best_multi_ldt_split(&(rootnode->instances), &(rootnode->w), &(rootnode->w0), MULTIVARIATE_LINEAR, 0.99);
 if (!(rootnode->lineard))
  {
   matrix_free(rootnode->w);
   setup_node_properties(rootnode);
   make_leaf_node(rootnode);
  }
 else
  {
   rootnode->featureno = LINEAR;
   make_multivariate_children(rootnode);
   setup_node_properties(rootnode->left);
   make_leaf_node(rootnode->left);
   setup_node_properties(rootnode->right);
   make_leaf_node(rootnode->right);
  }
 *trdata = NULL;
 return rootnode;
}

Prediction test_multildtstump(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_c45stump(data, model, parameters, posterior);
}

void* train_softstump(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	Decisionnodeptr treenode;
	Softregtree_parameterptr p;
	p = (Softregtree_parameterptr) parameters;
 treenode = createnewnode(NULL, 1);
 treenode->weight = 1.0;
 treenode->instances = *trdata;
 treenode->leaftype = p->leaftype;
 if (p->leaftype == CONSTANTLEAF)
   treenode->w0 = produce_random_value(-0.01, +0.01);
 else
   treenode->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
 find_best_soft_tree_split(treenode, treenode, *trdata, p);
 if (treenode->left != NULL)
  {
   make_leaf_node(treenode->left);
   make_leaf_node(treenode->right);
  }
 else
   make_leaf_node(treenode);
	*trdata = NULL;
	return treenode;
}

Prediction test_softstump(Instanceptr data, void* model, void* parameters, double* posterior)
{
	Decisionnodeptr treenode;
 Prediction predicted;
	treenode = (Decisionnodeptr) model;
 posterior[1] = sigmoid(soft_tree_output(treenode, data));
 posterior[0] = 1 - posterior[1];
 if (posterior[0] > posterior[1])
   predicted.classno = 0;
 else
   predicted.classno = 1;
 return predicted;
}

/**
 * Training algorithm for C4.5.
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the C4.5
 * @return Model of the C4.5.
 */
void* train_c45(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Decisionnodeptr rootnode;
 C45_parameterptr p;
 p = (C45_parameterptr) parameters;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 create_c45children(rootnode, p);
 if (in_list(p->prunetype, 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
  prune_tree(rootnode, rootnode, *cvdata);
 else
  if (p->prunetype == SRMPRUNING)
   {
   if (p->prunemodel == LOCAL)
    prune_tree_according_to_model_selection_method(rootnode, MODEL_SELECTION_SRM);
   else
    prune_tree_global(rootnode, rootnode, MODEL_SELECTION_SRM);
   }
 *trdata = NULL;
 return rootnode;
}

/**
 * Testing algorithm for C4.5.
 * @param[in] data Test instance.
 * @param[in] model Model of the C4.5.
 * @param[in] parameters Parameters of the C4.5.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_c45(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 11.07.2006 if there is no data at the node the posteriors will be same and 1 / classno*/
 /*!Last Changed 01.11.2005 added posteriors*/
 Decisionnodeptr currentnode, root = (Decisionnodeptr) model;
 Prediction predicted;
 currentnode = root;
 while (currentnode->featureno != LEAF_NODE)
   currentnode = next_node(currentnode, data);
 if (get_parameter_o("laplaceestimate") == ON)
   normalize_array_laplace(currentnode->counts, posterior, current_dataset->classno);
	else
   normalize_array(currentnode->counts, posterior, current_dataset->classno);
 predicted.classno = currentnode->classno;
 return predicted;
}

void* train_ktree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Decisionnodeptr rootnode;
 C45_parameterptr p;
 p = (C45_parameterptr) parameters;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 create_ktreechildren(rootnode, p);
 if (in_list(p->prunetype, 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
   prune_tree(rootnode, rootnode, *cvdata);
 *trdata = NULL;
 return rootnode;
}

Prediction test_ktree(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_c45(data, model, parameters, posterior);
}

/**
 * Training algorithm for Nearest Mean classifier. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Nearest Mean classifier
 * @return Model of the Nearest Mean classifier.
 */
void* train_nearestmean(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 22.03.2005*/
 int *counts;
 Instanceptr classmeans;
 counts = find_class_counts(*trdata);
 classmeans = find_class_means(*trdata, counts);
 safe_free(counts);
 return classmeans;
}

/**
 * Testing algorithm for Nearest Mean classifier.
 * @param[in] data Test instance.
 * @param[in] model Model of the Nearest Mean classifier.
 * @param[in] parameters Parameters of the Nearest Mean classifier.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_nearestmean(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 22.03.2005*/
 Instanceptr classmeans;
 int j;
 Prediction predicted;
 classmeans = (Instanceptr) model;
 find_nearest_mean(classmeans, data, &(predicted.classno));
 j = give_classno(data);
 posterior[j] = 1.0;
 return predicted;
}

/**
 * Training algorithm for simple Gaussian classifier. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the simple Gaussian classifier
 * @return Model of the simple Gaussian classifier.
 */
void* train_gaussian(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 19.12.2005*/
 Model_gaussianptr m;
 int* counts, datacount = data_size(*trdata), i;
 m = safemalloc(sizeof(Model_gaussian), "train_gaussian", 3);
 counts = find_class_counts(*trdata);
 m->mean = find_class_means(*trdata, counts);
 m->variance = find_class_variance(*trdata, m->mean, counts);
 m->priors = safemalloc(current_dataset->classno * sizeof(double), "train_gaussian", 7);
 for (i = 0; i < current_dataset->classno; i++)
   m->priors[i] = counts[i] / (datacount + 0.0);
 safe_free(counts);
 return m;
}

/**
 * Testing algorithm for simple Gaussian classifier.
 * @param[in] data Test instance.
 * @param[in] model Model of the simple Gaussian classifier.
 * @param[in] parameters Parameters of the simple Gaussian classifier.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_gaussian(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 19.12.2005*/
 Model_gaussianptr m;
 Instanceptr m_i;
 int i;
 double sum;
 Prediction predicted;
 m = (Model_gaussianptr)model;
 m_i = m->mean;
 sum = 0;
 for (i = 0; i < current_dataset->classno; i++)
  {
   posterior[i] = m->priors[i] * gaussian_vector(data, m_i, sqrt(m->variance[i]));
   sum += posterior[i];
   m_i = m_i->next;
  }
 for (i = 0; i < current_dataset->classno; i++)
   posterior[i] /= sum;
 predicted.classno = findMaxOutput(posterior, current_dataset->classno);
 return predicted;
}

/**
 * Training algorithm for Naive Bayes classifier. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Naive Bayes classifier
 * @return Model of the Naive Bayes classifier.
 */
void* train_naivebayes(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 22.01.2008 changed from variance to standard deviation*/
 /*!Last Changed 11.08.2007*/
 int i, j, k, *counts, size = data_size(*trdata);
 int ***feature_counts;
 Instanceptr tmp, classmeans;
 Model_naivebayesptr m;
 Instance s;
 m = safemalloc(sizeof(Model_naivebayes), "train_naivebayes", 5);
 counts = find_class_counts(*trdata); 
 classmeans = find_class_means(*trdata, counts);
 s = find_variance(*trdata);
 feature_counts = find_feature_counts(*trdata);
 m->s = (double *) safemalloc(current_dataset->featurecount * sizeof(double), "train_naivebayes", 9);
 m->m = (double **) safemalloc_2d(sizeof(double *), sizeof(double), current_dataset->classno, current_dataset->featurecount, "train_naivebayes", 10);
 m->priors = (double *)safemalloc(current_dataset->classno * sizeof(double), "train_naivebayes", 11);
 for (i = 0; i < current_dataset->featurecount; i++)
   m->s[i] = sqrt(s.values[i].floatvalue);
 tmp = classmeans;
 m->p = (double ***) safemalloc(current_dataset->classno * sizeof(double **), "train_naivebayes", 15);
 for (i = 0; i < current_dataset->classno; i++)
  {
   m->p[i] = (double **) safemalloc(current_dataset->featurecount * sizeof(double *), "train_naivebayes", 18);
   m->priors[i] = counts[i] / (size + 0.0);
   for (j = 0; j < current_dataset->featurecount; j++)
    {
     if (current_dataset->features[j].type == STRING)
      {
       m->p[i][j] = (double *) safemalloc(current_dataset->features[j].valuecount * sizeof(double), "train_naivebayes", 24);
       for (k = 0; k < current_dataset->features[j].valuecount; k++)
         m->p[i][j][k] = feature_counts[i][j][k] / (counts[i] + 0.0);
      }
     m->m[i][j] = tmp->values[j].floatvalue;
    }
   tmp = tmp->next;
  }
 free_feature_counts(feature_counts);
 safe_free(s.values);
 safe_free(counts);
 deallocate(classmeans);
 return m;
}

/**
 * Testing algorithm for Naive Bayes classifier.
 * @param[in] data Test instance.
 * @param[in] model Model of the Naive Bayes classifier.
 * @param[in] parameters Parameters of the Naive Bayes classifier.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_naivebayes(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 11.08.2007*/
 Model_naivebayesptr m;
 double max, value, sum = 0.0;
 int i, j;
 Prediction predicted;
 m = (Model_naivebayesptr) model;
 max = -LONG_MAX;
 predicted.classno = -1;
 for (i = 0; i < current_dataset->classno; i++)
  {
   value = log(m->priors[i]);
   for (j = 0; j < current_dataset->featurecount; j++)
     switch (current_dataset->features[j].type)
      {
       case INTEGER:if (m->s[j] != 0)
                      value += -0.5 * ((data->values[j].intvalue - m->m[i][j]) / m->s[j]) * ((data->values[j].intvalue - m->m[i][j]) / m->s[j]);
                    break;
       case    REAL:if (m->s[j] != 0)
                      value += -0.5 * ((data->values[j].floatvalue - m->m[i][j]) / m->s[j]) * ((data->values[j].floatvalue - m->m[i][j]) / m->s[j]);
                    break;
       case  STRING:if (m->p[i][j][data->values[j].stringindex] > 0)
                      value += log(m->p[i][j][data->values[j].stringindex]);
                    break;
      }
   posterior[i] = safeexp(value);
   sum += posterior[i];
   if (value > max)
    {
     max = value;
     predicted.classno = i;
    }
  }
 for (i = 0; i < current_dataset->classno; i++)
   posterior[i] /= sum; 
 return predicted;
}

/**
 * Training algorithm for Quadratic Discriminant Classifier. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Quadratic Discriminant Classifier
 * @return Model of the Quadratic Discriminant Classifier.
 */
void* train_qdaclass(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 05.08.2007*/
 Model_qdaclassptr m;
 matrix Si, mi, mitranspose, tmpmat;
 int i, *counts, size = data_size(*trdata);
 Instanceptr classmeans, tmp;
 double det;
 m = safemalloc(sizeof(Model_qdaclass), "train_qdaclass", 2);
 m->w0 = (double *)safemalloc(current_dataset->classno * sizeof(double), "qdaclass", 3);
 m->priors = (double *)safemalloc(current_dataset->classno * sizeof(double), "qdaclass", 4);
 m->w = (matrix *)safemalloc(current_dataset->classno * sizeof(matrix), "qdaclass", 5);
 m->W = (matrix *)safemalloc(current_dataset->classno * sizeof(matrix), "qdaclass", 6);
 counts = find_class_counts(*trdata);
 for (i = 0; i < current_dataset->classno; i++)
   m->priors[i] = counts[i] / (size + 0.0);
 classmeans = find_class_means(*trdata, counts);
 tmp = classmeans;
 for (i = 0; i < current_dataset->classno; i++)
  {
   mi = instance_to_matrix(tmp);
   mitranspose = matrix_transpose(mi);
   Si = class_covariance(*trdata, i, tmp);
   while (matrix_determinant(Si) < DBL_DELTA)
     matrix_perturbate(Si);
   det = matrix_determinant(Si);
   matrix_inverse(&Si);
   m->W[i] = matrix_copy(Si);
   matrix_multiply_constant(&(m->W[i]), -0.5);
   m->w[i] = matrix_multiply(mi, Si);
   tmpmat = matrix_multiply(m->w[i], mitranspose);
   m->w0[i] = -0.5 * (tmpmat.values[0][0] + log(det)) + log((counts[i] + 0.0) / size);
   tmp = tmp->next;
   matrix_free(mi);
   matrix_free(Si);
   matrix_free(mitranspose);
   matrix_free(tmpmat);
  }
 safe_free(counts);
 deallocate(classmeans);
 return m;
}

/**
 * Testing algorithm for Quadratic Discriminant Classifier.
 * @param[in] data Test instance.
 * @param[in] model Model of the Quadratic Discriminant Classifier.
 * @param[in] parameters Parameters of the Quadratic Discriminant Classifier.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_qdaclass(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 05.08.2007*/
 Model_qdaclassptr m;
 double max, gi, denom = 0.0;
 int i;
 matrix x, tmp1, tmp2, tmp3, xtranspose;
 Prediction predicted;
 m = (Model_qdaclassptr) model;
 max = -LONG_MAX;
 predicted.classno = -1;
 x = instance_to_matrix(data);
 xtranspose = matrix_transpose(x);
 for (i = 0; i < current_dataset->classno; i++)
  {
   tmp1 = matrix_multiply(x, m->W[i]);
   tmp2 = matrix_multiply(tmp1, xtranspose);
   tmp3 = matrix_multiply(m->w[i], xtranspose);
   gi = tmp2.values[0][0] + tmp3.values[0][0] + m->w0[i];
   if (gi > max)
    {
     max = gi;
     predicted.classno = i;
    }
   posterior[i] = safeexp(gi - log(m->priors[i])) * m->priors[i];
   denom += posterior[i];
   matrix_free(tmp1);
   matrix_free(tmp2);
   matrix_free(tmp3);
  }
 for (i = 0; i < current_dataset->classno; i++)
  	posterior[i] = posterior[i] / denom;
 matrix_free(x);
 matrix_free(xtranspose);
 return predicted;
}

/**
 * Training algorithm for Linear Discriminant Classifier. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Linear Discriminant Classifier
 * @return Model of the Linear Discriminant Classifier.
 */
void* train_ldaclass(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 int i, j, *counts, size = data_size(*trdata), howmany;
 Instanceptr classmeans, tmp;
 matrix ccov, temp, temp1, res, cov;
 Model_ldaclassptr m;
 Ldaclass_parameterptr p;
 p = (Ldaclass_parameterptr) parameters;
 m = safemalloc(sizeof(Model_ldaclass), "train_ldaclass", 7);
 m->w0 = (double *)safemalloc(current_dataset->classno * sizeof(double), "ldaclass", 8);
 counts = find_class_counts(*trdata); 
 m->w = (matrix *)safemalloc(current_dataset->classno * sizeof(matrix), "ldaclass", 10);
 classmeans = find_class_means(*trdata, counts);
 tmp = classmeans;
 cov = class_covariance(*trdata, 0, tmp);
 matrix_multiply_constant(&cov, counts[0] - 1);
 for (i = 1;i < current_dataset->classno;i++)
  {
   tmp = tmp->next;
   ccov = class_covariance(*trdata, i, tmp);
   matrix_multiply_constant(&ccov, counts[i] - 1);
   matrix_add(&cov, ccov);
   matrix_free(ccov);
  }
 matrix_divide_constant(&cov, size - current_dataset->classno);
 m->oldfeaturecount = current_dataset->multifeaturecount;
 m->eigenvalues = find_eigenvalues_eigenvectors(&(m->eigenvectors), cov, &howmany, p->variance_explained);
 m->newfeaturecount = howmany + 2;
 dimension_reduce_with_pca(*trdata, m->newfeaturecount - 1, m->eigenvectors);
 current_dataset->multifeaturecount = m->newfeaturecount;
 reduce_dimension_of_covariance_matrix(&cov, howmany, m->eigenvectors);
 if (cov.row == 1)
   cov.values[0][0] = 1.0 / cov.values[0][0];
 else
   matrix_inverse(&cov);
 deallocate(classmeans);
 classmeans = find_class_means(*trdata, counts);
 tmp = classmeans;
 i = 0;
 while (tmp)
  {
   temp = matrix_alloc(1, m->newfeaturecount - 1);
   for (j = 0;j < m->newfeaturecount - 1;j++)
     temp.values[0][j] = real_feature(tmp,j);
   m->w[i] = matrix_multiply(temp, cov);
   temp1 = matrix_transpose(temp);
   res = matrix_multiply(m->w[i], temp1);
   m->w0[i] = -0.5 * res.values[0][0];
   if (counts[i] != 0)
     m->w0[i] += log((counts[i] + 0.0) / size);
   matrix_free(temp1);
   matrix_free(temp);
   matrix_free(res);
   tmp = tmp->next;
   i++;
  }
 matrix_free(cov);
 safe_free(counts);
 deallocate(classmeans);
 current_dataset->multifeaturecount = m->oldfeaturecount;
 return m;
}

/**
 * Testing algorithm for Linear Discriminant Classifier.
 * @param[in] data Test instance.
 * @param[in] model Model of the Linear Discriminant Classifier.
 * @param[in] parameters Parameters of the Linear Discriminant Classifier.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_ldaclass(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 int i, j;
 matrix temp, res;
 double max, denom = 0.0;
 Model_ldaclassptr m;
 Prediction predicted;
 m = (Model_ldaclassptr) model;
 dimension_reduce_with_pca(data, m->newfeaturecount - 1, m->eigenvectors);
 current_dataset->multifeaturecount = m->newfeaturecount; 
 max = -LONG_MAX;
 predicted.classno = -1;
 temp = matrix_alloc(m->newfeaturecount - 1, 1);
 for (j = 0; j < m->newfeaturecount - 1;j++)
   temp.values[j][0] = real_feature(data, j);
 for (i = 0;i < current_dataset->classno;i++)
  {
   res = matrix_multiply(m->w[i], temp);
   posterior[i] = safeexp(res.values[0][0] + m->w0[i]);
   denom += posterior[i];
   if (res.values[0][0] + m->w0[i] > max)
    {
     max = res.values[0][0] + m->w0[i];
     predicted.classno = i;
    }
   matrix_free(res);
  }
 matrix_free(temp);
 for (i = 0; i < current_dataset->classno; i++)
   posterior[i] = posterior[i] / denom;
 current_dataset->multifeaturecount = m->oldfeaturecount;
 return predicted;
}

/**
 * Training algorithm for Linear Discriminant Trees (univariate splits). 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Linear Discriminant Trees
 * @return Model of the Linear Discriminant Trees.
 */
void* train_ldt(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 04.10.2004 added writerules*/
 /*!Last Changed 02.02.2004*/
 /*!Last Changed 09.02.2003*/
 Decisionnodeptr rootnode;
 C45_parameterptr p;
 p = (C45_parameterptr) parameters;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 create_ldtchildren(rootnode, p);
 if (in_list(p->prunetype, 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
   prune_tree(rootnode, rootnode, *cvdata);
 else
  {
   if (p->prunemodel == LOCAL)
     prune_tree_according_to_model_selection_method(rootnode, MODEL_SELECTION_SRM);
   else
     prune_tree_global(rootnode, rootnode, MODEL_SELECTION_SRM);
  }
 *trdata = NULL;
 return rootnode;
}

/**
 * Testing algorithm for Linear Discriminant Trees.
 * @param[in] data Test instance.
 * @param[in] model Model of the Linear Discriminant Trees.
 * @param[in] parameters Parameters of the Linear Discriminant Trees.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_ldt(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 04.10.2004 added writerules*/
 /*!Last Changed 02.02.2004*/
 /*!Last Changed 09.02.2003*/
 return test_c45(data, model, parameters, posterior);
}

/**
 * Training algorithm for Irep rule learning algorithm. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Irep rule learning algorithm
 * @return Model of the Irep rule learning algorithm.
 */
void* train_irep(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 21.05.2003 free_ruleset function is added*/
 /*!Last Changed 13.05.2003*/
 Ruleset* r;
 Ripper_parameterptr p;
 p = (Ripper_parameterptr) parameters;
 r = safemalloc(sizeof(Ruleset), "train_irep", 4);
 *r = learn_ruleset_ripper(trdata, 0, p);
 update_counts_ruleset(*r, *trdata);
 return r;
}

/**
 * Testing algorithm for Irep rule learning algorithm.
 * @param[in] data Test instance.
 * @param[in] model Model of the Irep rule learning algorithm.
 * @param[in] parameters Parameters of the Irep rule learning algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_irep(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 21.05.2003 free_ruleset function is added*/
 /*!Last Changed 13.05.2003*/
 int bestclass, rulesordered;
 BOOLEAN covered = BOOLEAN_FALSE;
 double besterror = 1.5, error; 
 Ruleset* r;
 Decisionruleptr tmp, bestrule = NULL;
 Prediction predicted;
 r = (Ruleset*) model;
 tmp = r->start;
 rulesordered = ((Ripper_parameterptr) parameters)->rulesordered;
 while (tmp)
  {
   if (rule_cover_instance(data, *tmp))
    {
     covered = BOOLEAN_TRUE;
     if (rulesordered)
      {
       if (get_parameter_o("laplaceestimate") == ON)
         normalize_array_laplace(tmp->counts, posterior, current_dataset->classno);
       else
         normalize_array(tmp->counts, posterior, current_dataset->classno);
       predicted.classno = tmp->classno;
       return predicted;
      }
     else
      {
       error = laplace_estimate_of_rule(*tmp);
       if (error < besterror)
        {
         besterror = error;
         bestclass = tmp->classno;
         bestrule = tmp;
        }
       tmp = get_next_class_rule(tmp);
       if (tmp)
         continue;
       else
         break;
      }
    }
   tmp = tmp->next;
  }
 if (!covered)
  {
   if (get_parameter_o("laplaceestimate") == ON)
     normalize_array_laplace(r->counts, posterior, current_dataset->classno);
			else
     normalize_array(r->counts, posterior, current_dataset->classno);
   predicted.classno = r->defaultclass;
   return predicted;
  }
 else
   if (!rulesordered)
    {
  			if (get_parameter_o("laplaceestimate") == ON)
       normalize_array_laplace(bestrule->counts, posterior, current_dataset->classno);
			  else
       normalize_array(bestrule->counts, posterior, current_dataset->classno);
     predicted.classno = bestclass;
     return predicted;
    }
   else
    {
  			if (get_parameter_o("laplaceestimate") == ON)
       normalize_array_laplace(r->counts, posterior, current_dataset->classno);
			  else
       normalize_array(r->counts, posterior, current_dataset->classno);
     predicted.classno = r->defaultclass;
     return predicted;
    }
}

/**
 * Training algorithm for Irep2 rule learning algorithm. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Irep2 rule learning algorithm
 * @return Model of the Irep2 rule learning algorithm.
 */
void* train_irep2(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 21.05.2003 free_ruleset function is added*/
 /*!Last Changed 13.05.2003*/
 Ruleset* r;
 Ripper_parameterptr p;
 p = (Ripper_parameterptr) parameters;
 r = safemalloc(sizeof(Ruleset), "train_irep2", 4);
 *r = learn_ruleset_ripper(trdata, 1, p);
 update_counts_ruleset(*r, *trdata);
 return r;
}

/**
 * Testing algorithm for Irep2 rule learning algorithm.
 * @param[in] data Test instance.
 * @param[in] model Model of the Irep2 rule learning algorithm.
 * @param[in] parameters Parameters of the Irep2 rule learning algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_irep2(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 21.05.2003 free_ruleset function is added*/
 /*!Last Changed 13.05.2003*/
 return test_irep(data, model, parameters, posterior);
}

/**
 * Training algorithm for Ripper. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Ripper
 * @return Model of the Ripper.
 */
void* train_ripper(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 21.05.2003 free_ruleset function is added*/
 /*!Last Changed 13.05.2003*/
 Ruleset* r;
 Ripper_parameterptr p;
 p = (Ripper_parameterptr) parameters;
 r = safemalloc(sizeof(Ruleset), "train_ripper", 4);
 *r = learn_ruleset_ripper(trdata, 2, p);
 update_counts_ruleset(*r, *trdata);
 return r;
}

/**
 * Testing algorithm for Ripper.
 * @param[in] data Test instance.
 * @param[in] model Model of the Ripper algorithm.
 * @param[in] parameters Parameters of the Ripper algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_ripper(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 21.05.2003 free_ruleset function is added*/
 /*!Last Changed 13.05.2003*/
 return test_irep(data, model, parameters, posterior);
}

/**
 * Training algorithm for RISE. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Rise
 * @return Model of the Rise.
 */
void* train_rise(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Model_riseptr m;
 m = learn_ruleset_rise(*trdata);
 return m;
}

/**
 * Testing algorithm for RISE.
 * @param[in] data Test instance.
 * @param[in] model Model of the Rise algorithm.
 * @param[in] parameters Parameters of the Rise algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_rise(Instanceptr data, void* model, void* parameters, double* posterior)
{
 Model_riseptr r;
 Decisionruleptr current;
 Prediction predicted;
 r = (Model_riseptr) model;
 current = nearest_rule(r, data, NO);
 if (current)
   predicted.classno = current->classno;
 else
   predicted.classno = r->rules.defaultclass;
 return predicted;
}

/**
 * Training algorithm for DIVS. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the DIVS
 * @return Model of the DIVS.
 */
void* train_divs(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Model_divsptr m;
 m = learn_ruleset_divs(*trdata);
 tune_consistency_and_generality(m, *cvdata);
 return m;
}

/**
 * Testing algorithm for DIVS.
 * @param[in] data Test instance.
 * @param[in] model Model of the DIVS algorithm.
 * @param[in] parameters Parameters of the DIVS algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_divs(Instanceptr data, void* model, void* parameters, double* posterior)
{
 Model_divsptr m;
 Prediction predicted;
 m = (Model_divsptr) model;
 predicted.classno = classify_using_divs_oracle(data, m, m->M, m->epsilon);
 return predicted;
}

/**
 * Training algorithm for CN2. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the CN2
 * @return Model of the CN2.
 */
void* train_cn2(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Ruleset* r;
 Cn2_parameterptr p;
 p = (Cn2_parameterptr)parameters;
 r = safemalloc(sizeof(Ruleset), "train_cn2", 2);
 *r = learn_ruleset_cn2(trdata, p);
 update_counts_ruleset(*r, *trdata);
 return r; 
}

/**
 * Testing algorithm for CN2.
 * @param[in] data Test instance.
 * @param[in] model Model of the CN2 algorithm.
 * @param[in] parameters Parameters of the CN2 algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_cn2(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_irep(data, model, parameters, posterior);
}

/**
 * Training algorithm for LERILS. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the LERILS
 * @return Model of the LERILS.
 */
void* train_lerils(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Ruleset* r;
 Lerils_parameterptr p;
 p = (Lerils_parameterptr)parameters;
 r = safemalloc(sizeof(Ruleset), "train_lerils", 2);
 *r = learn_ruleset_lerils(trdata, p);
 return r; 
}

/**
 * Testing algorithm for LERILS.
 * @param[in] data Test instance.
 * @param[in] model Model of the LERILS algorithm.
 * @param[in] parameters Parameters of the LERILS algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_lerils(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_irep(data, model, parameters, posterior);
}

/**
 * Training algorithm for PART. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the PART
 * @return Model of the PART.
 */
void* train_part(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Ruleset* r;
 r = safemalloc(sizeof(Ruleset), "train_part", 2);
 *r = learn_ruleset_part(trdata, cvdata);
 return r; 
}

/**
 * Testing algorithm for PART.
 * @param[in] data Test instance.
 * @param[in] model Model of the PART algorithm.
 * @param[in] parameters Parameters of the PART algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_part(Instanceptr data, void* model, void* parameters, double* posterior)
{
 Decisionruleptr tmp;
 Ruleset* r;
 Prediction predicted;
 r = (Ruleset*) model;
 tmp = r->start;
 while (tmp)
  {
   if (rule_cover_instance(data, *tmp))
    {
     predicted.classno = tmp->classno;
     return predicted;    
    }
   tmp = tmp->next;
  }
 predicted.classno = r->defaultclass;
 return predicted;
}

/**
 * Training algorithm for C4.5Rules learning algorithm. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the C4.5Rules learning algorithm
 * @return Model of the C4.5Rules learning algorithm.
 */
void* train_c45rules(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 10.11.2003 Rulecount, decisioncount and mdl added*/
 /*!Last Changed 03.11.2003*/
 Ruleset* r;
 C45_parameterptr p;
 Decisionnodeptr rootnode;
 r = safemalloc(sizeof(Ruleset), "train_c45rules", 3);
 p = safemalloc(sizeof(C45_parameter), "train_c45rules", 4);
 p->prunetype = PREPRUNING;
 p->pruningthreshold = 0.0;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 create_c45children(rootnode, p);
 safe_free(p);
 *trdata = NULL;
 accumulate_instances_tree(trdata, rootnode);
 *r = create_ruleset_from_decision_tree(rootnode);
 deallocate_tree(rootnode);
 safe_free(rootnode);
 r->possibleconditioncount = possible_condition_count(*trdata);
 generalize_c45rules(r, *trdata);
 sort_rules(r);
 find_class_rulesets(r, *trdata);
 if (r->rulecount > 0)
   rank_classes(r, *trdata);
 choose_default_class(r, *trdata);
 if (r->rulecount > 0)
   polish_ruleset(r, *trdata);
 update_counts_ruleset(*r, *trdata);
 return r;
}

/**
 * Testing algorithm for C4.5Rules learning algorithm.
 * @param[in] data Test instance.
 * @param[in] model Model of the C4.5Rules learning algorithm.
 * @param[in] parameters Parameters of the C4.5Rules learning algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_c45rules(Instanceptr data, void* model, void* parameters, double* posterior)
{
 int covered = 0, bestclass;
 double besterror = 1.5, error; 
 Ruleset* r;
 Prediction predicted;
 Decisionruleptr tmp, bestrule = NULL;
 r = (Ruleset*) model;
 tmp = r->start;
 while (tmp)
  {
   if (rule_cover_instance(data, *tmp))
    {
     covered = 1;
     error = laplace_estimate_of_rule(*tmp);
     if (error < besterror)
      {
       bestrule = tmp;
       besterror = error;
       bestclass = tmp->classno;
      }
     tmp = get_next_class_rule(tmp);
     if (tmp)
       continue;
     else
       break;
    }
   tmp = tmp->next;
  }
 if (!covered)
  {
			if (get_parameter_o("laplaceestimate") == ON)
     normalize_array_laplace(r->counts, posterior, current_dataset->classno);
			else
     normalize_array(r->counts, posterior, current_dataset->classno);
   predicted.classno = r->defaultclass;
   return predicted;
  }
 else
  {
			if (get_parameter_o("laplaceestimate") == ON)
     normalize_array_laplace(bestrule->counts, posterior, current_dataset->classno);
			else
     normalize_array(bestrule->counts, posterior, current_dataset->classno);
   predicted.classno = bestclass;
   return predicted;
  }
}

/**
 * Training algorithm for Multivariate Ripper rule learning algorithm. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Multivariate Ripper rule learning algorithm
 * @return Model of the Multivariate Ripper rule learning algorithm.
 */
void* train_multiripper(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 07.01.2004*/
 Ruleset* r;
 Ripper_parameterptr p;
 p = (Ripper_parameterptr) parameters;
 r = safemalloc(sizeof(Ruleset), "train_multiripper", 4);
 *r = multivariate_rule_learner(trdata, *cvdata, p);
 update_counts_ruleset(*r, *trdata);
 return r;
}

/**
 * Testing algorithm for Multivariate Ripper rule learning algorithm.
 * @param[in] data Test instance.
 * @param[in] model Model of the Multivariate Ripper rule learning algorithm.
 * @param[in] parameters Parameters of the Multivariate Ripper rule learning algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_multiripper(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_irep(data, model, parameters, posterior);
}

/**
 * Training algorithm for Omnivariate Ripper rule learning algorithm. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Omnivariate Ripper rule learning algorithm
 * @return Model of the Omnivariate Ripper rule learning algorithm.
 */
void* train_hybridripper(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 24.01.2004*/
 Ruleset* r;
 Ripper_parameterptr p;
 p = (Ripper_parameterptr) parameters;
 r = safemalloc(sizeof(Ruleset), "train_hybridripper", 4);
 *r = hybrid_rule_learner(trdata, *cvdata, p);
 update_counts_ruleset(*r, *trdata);
 return r;
}

/**
 * Testing algorithm for Omnivariate Ripper rule learning algorithm.
 * @param[in] data Test instance.
 * @param[in] model Model of the Omnivariate Ripper rule learning algorithm.
 * @param[in] parameters Parameters of the Omnivariate Ripper rule learning algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_hybridripper(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_irep(data, model, parameters, posterior);
}

/**
 * Training algorithm for Radial Basis Function Networks. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Radial Basis Function Networks
 * @return Model of the Radial Basis Function Networks.
 */
void* train_rbf(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 19.08.2007*/
 Rbf_parameterptr params;
 params = (Rbf_parameterptr) parameters;
 params->input = current_dataset->multifeaturecount - 1;
 return train_rbfnetwork_cls(*trdata, *cvdata, params);
}

/**
 * Testing algorithm for Radial Basis Function Networks.
 * @param[in] data Test instance.
 * @param[in] model Model of the Radial Basis Function Networks.
 * @param[in] parameters Parameters of the Radial Basis Function Networks.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_rbf(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 19.08.2007*/
 Rbfnetworkptr network;
 double* p;
 Prediction predicted;
 network = (Rbfnetworkptr) model;
 p = safemalloc((network->hidden + 1) * sizeof(double), "test_rbf", 4);
 rbf_forward_propagation(network, data, p, posterior);
 safe_free(p);
 predicted.classno = findMaxOutput(posterior, current_dataset->classno);
 return predicted;
}

/**
 * Training algorithm for Neural Networks (feedforward neural networks with any number of hidden layers). 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Neural Networks
 * @return Model of the Neural Networks.
 */
void* train_mlpgeneric(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 23.06.2005 added cv*/
 Mlpparameters mlpparams;
 Mlpnetworkptr neuralnetwork;
 matrix train, cv;
 neuralnetwork = safemalloc(sizeof(Mlpnetwork), "train_mlpgeneric", 4);
 mlpparams = *((Mlpparametersptr) parameters);
 mlpparams.inputnum = data_size(*trdata);
 mlpparams.trnum = mlpparams.inputnum;
 mlpparams.cvnum = data_size(*cvdata);
 train = instancelist_to_matrix(*trdata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 cv = instancelist_to_matrix(*cvdata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 *neuralnetwork = mlpn_general(train, cv, &mlpparams, CLASSIFICATION);
 matrix_free(train);
 matrix_free(cv);
 return neuralnetwork;
}

/**
 * Testing algorithm for Neural Networks.
 * @param[in] data Test instance.
 * @param[in] model Model of the Neural Networks.
 * @param[in] parameters Parameters of the Neural Networks.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_mlpgeneric(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 01.11.2005*/
 /*!Last Changed 23.06.2005 added cv*/
 Mlpnetwork neuralnetwork;
 matrix test, testY;
 int res, i;
 double prob;
 Prediction predicted;
 neuralnetwork = *((Mlpnetworkptr) model);
 test = instancelist_to_matrix(data, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 testMlpnInput_cls(test.values[0], &testY, neuralnetwork, &res, &prob);
 for (i = 0; i < current_dataset->classno; i++)
   posterior[i] = testY.values[0][i];
 predicted.classno = findMaxOutput(testY.values[0], current_dataset->classno);
 matrix_free(testY);
 matrix_free(test);
 return predicted;
}

/**
 * Training algorithm for Dynamic Node Creation. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Dynamic Node Creation
 * @return Model of the Dynamic Node Creation.
 */
void* train_dnc(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 16.04.2007*/
 Mlpparameters mlpparams;
 Mlpnetworkptr neuralnetwork;
 matrix train;
 neuralnetwork = safemalloc(sizeof(Mlpnetwork), "train_dnc", 4);
 mlpparams = *((Mlpparametersptr) parameters);
 mlpparams.inputnum = data_size(*trdata);
 mlpparams.trnum = mlpparams.inputnum;
 train = instancelist_to_matrix(*trdata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 *neuralnetwork = dnc_cls(train, &mlpparams);
 matrix_free(train);
 return neuralnetwork;
}

/**
 * Testing algorithm for Dynamic Node Creation.
 * @param[in] data Test instance.
 * @param[in] model Model of the Dynamic Node Creation.
 * @param[in] parameters Parameters of the Dynamic Node Creation.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_dnc(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 16.04.2007*/
 return test_mlpgeneric(data, model, parameters, posterior);
}

/**
 * Training algorithm for Ripper algorithm with exceptions handled using Rex. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Rexripper algorithm
 * @return Model of the Rexripper algorithm.
 */
void* train_rexripper(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 03.12.2004 added deallocate(exceptiondata)*/
 /*!Last Changed 08.05.2004*/
 Model_rexripperptr m;
 Ripper_parameterptr p;
 m = safemalloc(sizeof(Model_rexripper), "train_rexripper", 3);
 p = (Ripper_parameterptr) parameters;
 m->r = learn_ruleset_ripper(trdata, 2, p);
 update_counts_ruleset(m->r, *trdata);
 m->exceptiondata = NULL;
 create_list_from_exceptions(&(m->exceptiondata), trdata);
 return m;
}

/**
 * Testing algorithm for Rexripper.
 * @param[in] data Test instance.
 * @param[in] model Model of the Rexripper algorithm.
 * @param[in] parameters Parameters of the Rexripper algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_rexripper(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 03.12.2004 added deallocate(exceptiondata)*/
 /*!Last Changed 08.05.2004*/
 Model_rexripperptr m;
 Ripper_parameterptr p;
 int covered = 0, j, *classcounts;
 double besterror = 1.5, error;
 Instanceptr *neighbors;
 Decisionruleptr tmp;
 Prediction predicted;
 m = (Model_rexripperptr) model;
 p = (Ripper_parameterptr) parameters;
 tmp = m->r.start;
 predicted.classno = -1;
 while (tmp)
  {
   if (rule_cover_instance(data, *tmp))
    {
     covered = 1;
     error = laplace_estimate_of_rule(*tmp);
     if (error < besterror)
      {
       besterror = error;
       predicted.classno = tmp->classno;
      }
     tmp = get_next_class_rule(tmp);
     if (tmp)
       continue;
     else
       break;
    }
   tmp = tmp->next;
  }
 if (!covered)
  {
   data->isexception = 1;
   neighbors = find_nearest_neighbors_before_normalize(m->exceptiondata, data, p->nearcount, testmean, testvariance);
   classcounts = safecalloc(current_dataset->classno, sizeof(int), "test_rexripper", 40);
   for (j = 0; j < p->nearcount; j++)
     if (neighbors[j])
       classcounts[give_classno(neighbors[j])]++;
     else
       break;
   predicted.classno = find_max_in_list(classcounts, current_dataset->classno);
   safe_free(classcounts);
   safe_free(neighbors);
  }
 return predicted;
}

/**
 * Training algorithm for Multivariate Decision Tree (multivariate split is found using linear perceptron or linear discriminant analysis). 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Multivariate Decision Tree
 * @return Model of the Multivariate Decision Tree.
 */
void* train_multildt(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 04.10.2004 added writerules*/
 /*!Last Changed 21.09.2004*/
 Decisionnodeptr rootnode;
 Multildt_parameterptr p;
 p = (Multildt_parameterptr) parameters;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 create_multildtchildren(rootnode, p);
 prune_tree(rootnode, rootnode, *cvdata);
 *trdata = NULL;
 return rootnode;
}

/**
 * Testing algorithm for Multivariate Decision Tree.
 * @param[in] data Test instance.
 * @param[in] model Model of the Multivariate Decision Tree.
 * @param[in] parameters Parameters of the Multivariate Decision Tree.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_multildt(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 04.10.2004 added writerules*/
 /*!Last Changed 21.09.2004*/
 return test_c45(data, model, parameters, posterior);
}

/**
 * Training algorithm for CART. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of CART
 * @return Model of CART.
 */
void* train_cart(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 Decisionnodeptr rootnode;
 C45_parameterptr p;
 p = (C45_parameterptr) parameters;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 create_cartchildren(rootnode, p);
 prune_tree(rootnode, rootnode, *cvdata);
 *trdata = NULL;
 return rootnode;
}

/**
 * Testing algorithm for CART.
 * @param[in] data Test instance.
 * @param[in] model Model of CART.
 * @param[in] parameters Parameters of CART.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_cart(Instanceptr data, void* model, void* parameters, double* posterior)
{
 return test_c45(data, model, parameters, posterior);
}

/**
 * Training algorithm for Omnivariate Decision Tree (quadratic split is found using quadratic discriminant analysis). 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Omnivariate Decision Tree
 * @return Model of the Omnivariate Decision Tree.
 */
void* train_omnildt(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 24.11.2004 added univariate and multivariate condition counts*/
 /*!Last Changed 22.11.2004 added possible conditioncount*/
 /*!Last Changed 04.10.2004 added writerules*/
 /*!Last Changed 23.09.2004*/
 Decisionnodeptr rootnode;
 Multildt_parameterptr p;
 p = (Multildt_parameterptr) parameters;
 rootnode = createnewnode(NULL, 1);
 rootnode->instances = *trdata;
 rootnode->possibleconditioncount = possible_condition_count(*trdata);
 create_omnildtchildren(rootnode, p);
 if (in_list(p->c45param.prunetype, 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
  {
   if (p->c45param.prunetype == VALIDATIONPRUNING || (p->c45param.prunetype == MODELSELECTIONPRUNING && p->modelselectionmethod == MODEL_SELECTION_CV))
     prune_tree(rootnode, rootnode, *cvdata);
   else
     prune_tree_according_to_model_selection_method(rootnode, p->modelselectionmethod);
  }
 *trdata = NULL;
 return rootnode;
}

/**
 * Testing algorithm for Omnivariate Decision Tree.
 * @param[in] data Test instance.
 * @param[in] model Model of the Omnivariate Decision Tree.
 * @param[in] parameters Parameters of the Omnivariate Decision Tree.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_omnildt(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 24.11.2004 added univariate and multivariate condition counts*/
 /*!Last Changed 22.11.2004 added possible conditioncount*/
 /*!Last Changed 04.10.2004 added writerules*/
 /*!Last Changed 23.09.2004*/
 return test_c45(data, model, parameters, posterior);
}

/**
 * Training algorithm for Node Bounded Decision Tree. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Node Bounded Decision Tree
 * @return Model of the Node Bounded Decision Tree.
 */
void* train_nodeboundedtree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 30.09.2007 added postpruning via SRM*/
 /*!Last Changed 31.05.2006*/
 Decisionnodeptr rootnode, besttree = NULL;
 int i, j, k, performance, maxperformance = -1, size = data_size(*trdata);
 double bin;
 Instanceptr data = *trdata;
 Nodeboundedtree_parameterptr p;
 p = (Nodeboundedtree_parameterptr) parameters;
 bin = 4.0 / p->partitioncount;
 if (in_list(p->prunetype, 3, MODELSELECTIONPRUNING, PREPRUNING, VALIDATIONPRUNING))
  {
   k = 0;
   for (i = 0; i < p->partitioncount; i++)
     for (j = 0; j < p->partitioncount; j++)
      {
       rootnode = createnewnode(NULL, 1);
       rootnode->instances = data;
       p->a1 = (i + 1) * bin;
       p->a2 = (j + 1) * bin; 
       create_node_bounded_tree(rootnode, p);
       switch (p->prunetype)
        {
         case     VALIDATIONPRUNING:prune_node_bounded_tree(rootnode, p, size);
                                    break;
         case MODELSELECTIONPRUNING:set_float_parameter("srma1", p->a1);
                                    set_float_parameter("srma2", p->a2);
                                    prune_tree_according_to_model_selection_method(rootnode, MODEL_SELECTION_SRM);
                                    break;
        }
       performance = test_tree(rootnode, *cvdata);
       write_array_variables("aout", k + 1, 3, "d", performance);
       data = NULL;
       accumulate_instances_tree(&data, rootnode);
       if (performance > maxperformance)
        {
         if (maxperformance != -1)
           deallocate_tree(besttree);
         maxperformance = performance;
         besttree = rootnode;       
        }
       else
         deallocate_tree(rootnode);
       k++;
      }
   *trdata = data;
  }
 else
  {
   besttree = createnewnode(NULL, 1);
   besttree->instances = *trdata;  
   create_node_bounded_tree(besttree, p);
  } 
 return besttree;
}

/**
 * Testing algorithm for Node Bounded Decision Tree.
 * @param[in] data Test instance.
 * @param[in] model Model of the Node Bounded Decision Tree.
 * @param[in] parameters Parameters of the Node Bounded Decision Tree.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_nodeboundedtree(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 19.03.2006*/
 return test_c45(data, model, parameters, posterior);
}

/**
 * Training algorithm for SOFT TREE. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the SOFT TREE algorithm
 * @return Model of the SOFT TREE algorithm.
 */
void* train_softtree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
	Decisionnodeptr treenode;
	Softregtree_parameterptr p;
	p = (Softregtree_parameterptr) parameters;
 treenode = createnewnode(NULL, 1);
 treenode->weight = 1.0;
 treenode->instances = *trdata;
 if (current_dataset->classno == 2)
  {
   treenode->leaftype = p->leaftype;
   if (p->leaftype == CONSTANTLEAF)
     treenode->w0 = produce_random_value(-0.01, +0.01);
   else
     treenode->w = random_matrix(current_dataset->multifeaturecount, 1, -0.01, +0.01);
   create_softtree(treenode, treenode, *cvdata, p);
  }
 else
  {
   treenode->leaftype = CONSTANTLEAF;
   treenode->w0s = random_array_double(current_dataset->classno, -0.01, +0.01);
   treenode->outputs = safemalloc(current_dataset->classno * sizeof(double), "train_softtree", 14);
   create_softtree_K_class(treenode, treenode, *cvdata, p);
  }
	*trdata = NULL;
	return treenode;
}

/**
 * Testing algorithm for SOFT TREE.
 * @param[in] data Test instance.
 * @param[in] model Model of the SOFT TREE algorithm.
 * @param[in] parameters Parameters of the SOFT TREE algorithm.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_softtree(Instanceptr data, void* model, void* parameters, double* posterior)
{
 int i;
	Decisionnodeptr treenode;
 Prediction predicted;
 double* output;
	treenode = (Decisionnodeptr) model;
 if (current_dataset->classno == 2)
  {
   posterior[1] = sigmoid(soft_tree_output(treenode, data));
   posterior[0] = 1 - posterior[1];
   if (posterior[0] > posterior[1])
     predicted.classno = 0;
   else
     predicted.classno = 1;
  }
 else
  {
   output = soft_tree_output_K_class(treenode, data);
   softmax(output, current_dataset->classno);
   for (i = 0; i < current_dataset->classno; i++)
     posterior[i] = output[i];
   predicted.classno = find_max_in_list_double(posterior, current_dataset->classno);
  }
 return predicted;
}

/**
 * Training algorithm for Logistic Discrimination classifier. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Logistic Discrimination classifier
 * @return Model of the Logistic Discrimination classifier.
 */
void* train_logistic(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 17.04.2005*/
 double** w, **deltaw, *y, diff, eta;
 Instanceptr tmp;
 int i, j, k, cno;
 Logistic_parameterptr param;
 param = (Logistic_parameterptr) parameters;
 w = two_dimensional_initialize(current_dataset->classno, current_dataset->multifeaturecount, 0.01);
 deltaw = (double**) safemalloc_2d(sizeof(double *), sizeof(double), current_dataset->classno, current_dataset->multifeaturecount, "logistic", 7);
 y = safemalloc(current_dataset->classno * sizeof(double), "logistic", 6);
 eta = param->learning_rate;
 for (k = 0; k < param->perceptron_run; k++)
  {
   for (i = 0; i < current_dataset->classno; i++)
     for (j = 0; j < current_dataset->multifeaturecount; j++)
       deltaw[i][j] = 0.0;
   tmp = *trdata;
   while (tmp)
    {
     for (i = 0; i < current_dataset->classno; i++)
      {
       y[i] = w[i][0];
       for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
         y[i] += w[i][j + 1] * real_feature(tmp, j);
      }
     softmax(y, current_dataset->classno);
     for (i = 0; i < current_dataset->classno; i++)
      {
       cno = give_classno(tmp);
       if (cno == i)
         diff = 1.0 - y[i];
       else
         diff = 0.0 - y[i];
       deltaw[i][0] += diff;
       for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
         deltaw[i][j + 1] += diff * real_feature(tmp, j);
      }
     tmp = tmp->next;
    }
   for (i = 0; i < current_dataset->classno; i++)
     for (j = 0; j < current_dataset->multifeaturecount; j++)
       w[i][j] += eta * deltaw[i][j];
   eta *= param->etadecrease;
  }
 free_2d((void**)deltaw, current_dataset->classno);
 safe_free(y);
 return w;
}

/**
 * Testing algorithm for Logistic Discrimination classifier.
 * @param[in] data Test instance.
 * @param[in] model Model of the Logistic Discrimination classifier.
 * @param[in] parameters Parameters of the Logistic Discrimination classifier.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_logistic(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 17.04.2005*/
 double** w, *y;
 int i, j;
 Prediction predicted;
 w = (double**) model;
 y = safemalloc(current_dataset->classno * sizeof(double), "logistic", 6);
 for (i = 0; i < current_dataset->classno; i++)
  {
   y[i] = w[i][0];
   for (j = 0; j < current_dataset->multifeaturecount - 1; j++)
     y[i] += w[i][j + 1] * real_feature(data, j);
  }
 predicted.classno = findMaxOutput(y, current_dataset->classno);
 softmax(y, current_dataset->classno);
 for (i = 0; i < current_dataset->classno; i++)
   posterior[i] = y[i]; 
 safe_free(y);
 return predicted;
}

/**
 * Training algorithm for standard Support Vector Machine. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the standard Support Vector Machine algorithm
 * @return Model of the standard Support Vector Machine algorithm.
 */
void* train_svm(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 03.05.2005*/
 Partition dummyp;
 Svm_modelptr model;
 Svm_problem prob;
 Svm_parameterptr param;
 dummyp = get_two_class_partition();
 prepare_svm_problem(&prob, C_SVC, *trdata, dummyp);
 free_partition(dummyp);
 param = (Svm_parameter*) parameters;
 model = svm_train(prob, *param, 1);
 return model;
}

/**
 * Testing algorithm for standard Support Vector Machine.
 * @param[in] data Test instance.
 * @param[in] model Model of the standard Support Vector Machine.
 * @param[in] parameters Parameters of the standard Support Vector Machine.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_svm(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 03.05.2005*/
 Prediction predicted;
 double* dec_values;
 int classno, y;
 Svm_nodeptr node;
 Svm_modelptr m;
 m = (Svm_modelptr) model;
 node = instance_to_svmnode(data);
 svm_predict_probability(m, node, posterior);
 if (m->nr_class == 2)
  {
   dec_values = safemalloc(sizeof(double), "test_svm", 7);
   svm_predict_values(m, node, dec_values);
   classno = give_classno(data);
   y = 1 - 2 * classno;
   predicted.hingeloss = hinge_loss(y, dec_values[0]);
   safe_free(dec_values);
  }
 predicted.classno = (int) svm_predict(m, node);
 safe_free(node);
 return predicted;
}

/**
 * Training algorithm for simplex Support Vector Machine. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the simplex Support Vector Machine algorithm
 * @return Model of the simplex Support Vector Machine algorithm.
 */
void* train_simplexsvm(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 03.05.2005*/
 Svm_simplex_parameterptr param;
 param = (Svm_simplex_parameterptr) parameters;
 if (current_dataset->classno == 2)
   return solve_binary_svm(*trdata, param, 0);
 else
   if (param->problem_type == ONE_VS_ONE)
     return solve_one_vs_one_svm(trdata, param);
   else
     return solve_one_vs_rest_svm(*trdata, param);
}

/**
 * Testing algorithm for simplex Support Vector Machine.
 * @param[in] data Test instance.
 * @param[in] model Model of the simplex Support Vector Machine.
 * @param[in] parameters Parameters of the simplex Support Vector Machine.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_simplexsvm(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 03.05.2005*/
 Svm_simplex_parameterptr param;
 Prediction predicted;
 param = (Svm_simplex_parameterptr) parameters;
 if (current_dataset->classno == 2)
  {
   if (test_binary_svm((Svm_binary_modelptr) model, param, data) < 0)
     predicted.classno = 0;
   else
     predicted.classno = 1;
  }
 else
   if (param->problem_type == ONE_VS_ONE)
     predicted.classno = test_one_vs_one_svm((Svm_simplex_modelptr) model, param, data);
  else
     predicted.classno = test_one_vs_rest_svm((Svm_simplex_modelptr) model, param, data);
  return predicted;
}

/**
 * Training algorithm for Support Vector Machines Decision Tree. 
 * @param[in] trdata Training data
 * @param[in] cvdata Cross validation data
 * @param[in] parameters Parameters of the Support Vector Machines Decision Tree algorithm
 * @return Model of the Support Vector Machines Decision Tree algorithm.
 */
void* train_svmtree(Instanceptr* trdata, Instanceptr *cvdata, void* parameters)
{
 /*!Last Changed 09.07.2009*/
 int i, j, k = 0;
 Decisionnodeptr rootnode;
 Svmtree_parameterptr param;
 Decisionnodeptr* forest;
 Instanceptr positives, negatives;
 param = (Svmtree_parameterptr) parameters;
 if (param->multiclasstype == ONE_VS_ONE)
  {
   forest = safemalloc((current_dataset->classno * (current_dataset->classno - 1) / 2) * sizeof(Decisionnodeptr), "train_svmtree", 3);
   for (i = 0; i < current_dataset->classno; i++)
     for (j = i + 1; j < current_dataset->classno; j++)
      {
       rootnode = createnewnode(NULL, 1);
       positives = negatives = NULL;
       divide_instancelist_one_vs_one(trdata, &positives, i, j);
       rootnode->instances = positives;
       create_svmtreechildren(rootnode, param, i);
       positives = NULL;
       accumulate_instances_tree(&positives, rootnode);
       rootnode->instances = NULL;
       merge_instancelist(trdata, positives);
       if (in_list(param->c45param.prunetype, 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
        {
         positives = negatives = NULL;
         divide_instancelist_one_vs_one(cvdata, &positives, i, j);
         prune_tree(rootnode, rootnode, positives);
         merge_instancelist(cvdata, positives);
        }
       forest[k] = rootnode;
       k++;
      }
  }
 else
  {
   forest = safemalloc(current_dataset->classno * sizeof(Decisionnodeptr), "train_svmtree", 3);
   for (i = 0; i < current_dataset->classno; i++)
    {
     rootnode = createnewnode(NULL, 1);
     rootnode->instances = *trdata;
     create_svmtreechildren(rootnode, param, i);
     *trdata = NULL;
     accumulate_instances_tree(trdata, rootnode);
     rootnode->instances = NULL;
     if (in_list(param->c45param.prunetype, 2, MODELSELECTIONPRUNING, VALIDATIONPRUNING))
       prune_tree(rootnode, rootnode, *cvdata);
     forest[i] = rootnode;
    }
  }
 return forest;
}

/**
 * Testing algorithm for Support Vector Machines Decision Tree.
 * @param[in] data Test instance.
 * @param[in] model Model of the Support Vector Machines Decision Tree.
 * @param[in] parameters Parameters of the Support Vector Machines Decision Tree.
 * @param[out] posterior Posterior probability array to be filled in for current test instance.
 * @return Estimated class for the test instance.
 */
Prediction test_svmtree(Instanceptr data, void* model, void* parameters, double* posterior)
{
 /*!Last Changed 09.07.2009*/
 int i, j, k = 0;
 double* votes;
 Svmtree_parameterptr param;
 Decisionnodeptr* forest;
 Prediction predicted;
 param = (Svmtree_parameterptr) parameters;
 votes = safecalloc(current_dataset->classno, sizeof(double), "test_svmtree", 2);
 forest = (Decisionnodeptr*) model;
 if (param->multiclasstype == ONE_VS_ONE)
  {
   for (i = 0; i < current_dataset->classno; i++)
     for (j = i + 1; j < current_dataset->classno; j++)
      {
       test_c45(data, forest[k], parameters, posterior);
       votes[i] += posterior[i];
       votes[j] += posterior[j];
       k++;
      }
  }
 else
  {
   for (i = 0; i < current_dataset->classno; i++)
    {
     test_c45(data, forest[i], parameters, posterior);
     for (j = 0; j < current_dataset->classno; j++)
       votes[j] += posterior[j];
    }
  }
 normalize_array_of_doubles(votes, current_dataset->classno);
 memcpy(posterior, votes, current_dataset->classno * sizeof(double));
 safe_free(votes);
 predicted.classno = findMaxOutput(posterior, current_dataset->classno);
 return predicted;
}
