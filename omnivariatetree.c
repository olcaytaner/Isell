#include "dataset.h"
#include "decisiontree.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"
#include "model.h"
#include "multivariatetree.h"
#include "omnivariatetree.h"
#include "rule.h"
#include "statistics.h"
#include "tests.h"
#include "univariatetree.h"
#include "vc.h"

extern Datasetptr current_dataset;

/**
 * Recursive function which generates a string containing level of the node, decision tree node type, number of instances in that node using the preorder traversal. The string is something like 1;U;5 which says that the node is in the first level, the node is a univariate node (L for linear, Q for quadratic nodes) and has 5 instances.
 * @param[in] node Decision tree node
 * @param[out] st[] Output string
 * @param[in] level Level of the node
 * @warning The string must not exceed READLINELENGTH characters.
 */
void decisiontree_node_types(Decisionnodeptr node, char st[READLINELENGTH], int level)
{
 int i;
 if (node->featureno != LEAF_NODE)
  {
   if (node->featureno >= 0)
     sprintf(st, "%s%d-U-%d;", st, level, node->covered);
   else
     if (node->featureno == LINEAR)
       sprintf(st, "%s%d-L-%d;", st, level, node->covered);
     else
       if (node->featureno == QUADRATIC) 
         sprintf(st, "%s%d-Q-%d;", st, level, node->covered);
       else
         if (node->featureno == DISCRETEMIX)
           sprintf(st, "%s%d-%d;", st, level, node->ksplit.featurecount);
   if (node->featureno >= 0 && current_dataset->features[node->featureno].type == STRING)
    {
     for (i = 0; i < current_dataset->features[node->featureno].valuecount; i++)
       decisiontree_node_types(&(node->string[i]), st, level + 1);
    }
   else
    {
     decisiontree_node_types(node->left, st, level + 1);
     decisiontree_node_types(node->right, st, level + 1);    
    }
  }
}

void prune_tree_global(Decisionnodeptr root, Decisionnodeptr currentnode, int modelselectionmethod)
{
 double performancebefore, performance, error;
 int i, h, tmp;
 if (currentnode->featureno == LEAF_NODE)
   return;
 switch (modelselectionmethod)
  {
   case MODEL_SELECTION_SRM:error = decisiontree_falsepositive_count(root) / (root->covered + 0.0);
                            h = decisiontree_vc_dimension_recursive(current_dataset, root);
                            performancebefore = srm_classification(error, h, root->covered, get_parameter_f("srma1"), get_parameter_f("srma2"));
                            break;
  }
 tmp = currentnode->featureno;
 currentnode->featureno = LEAF_NODE;
 switch (modelselectionmethod)
  {
   case MODEL_SELECTION_SRM:error = decisiontree_falsepositive_count(root) / (root->covered + 0.0);
                            h = decisiontree_vc_dimension_recursive(current_dataset, root);
                            performance = srm_classification(error, h, root->covered, get_parameter_f("srma1"),  get_parameter_f("srma2"));
                            break;
  }
 if (performance < performancebefore)
  {
   accumulate_instances_tree(&(currentnode->instances), currentnode);
   free_children(currentnode);
  }
 else
  {
   currentnode->featureno = tmp;
   if (currentnode->featureno >= 0 && (currentnode->conditiontype != HYBRID_CONDITION) && current_dataset->features[currentnode->featureno].type == STRING)
     for (i = 0; i < current_dataset->features[currentnode->featureno].valuecount; i++)
       prune_tree_global(root, &(currentnode->string[i]), modelselectionmethod);
   else
    {
     prune_tree_global(root, currentnode->left, modelselectionmethod);
     prune_tree_global(root, currentnode->right, modelselectionmethod);
    }
  }
}

/**
 * Recursive function that prunes the decision tree using the model selection method selected. Currently there are four model selection methods supported. MODEL_SELECTION_AIC for Akaike Information Criterion based pruning, MODEL_SELECTION_BIC for Bayesian Information Criterion based pruning, MODEL_SELECTION_SRM for Structural Risk Minimization based pruning and MODEL_SELECTION_MDL for Minimum Description Length pruning.
 * @param[in] root Root of the decision tree
 * @param[in] modelselectionmethod Type of model selection method. Possible model selection methods are explained above.
 */
void prune_tree_according_to_model_selection_method(Decisionnodeptr root, int modelselectionmethod)
{
 /*!Last Changed 24.05.2006 added free_children*/
 /*!Last Changed 10.02.2005*/
 double performancebefore, performance, loglikelihoodbefore, loglikelihood, exceptionlengthbefore, exceptionlength, error;
 int d, *classcounts, N, i, C, fp, h;
 if (root->featureno == LEAF_NODE)
   return;
 switch (modelselectionmethod)
  {
   case MODEL_SELECTION_AIC:loglikelihoodbefore = decisiontree_loglikelihood(root);
                            d = decisiontree_complexity_count(root);
                            performancebefore = aic_loglikelihood(loglikelihoodbefore, d);
                            classcounts = safecalloc(current_dataset->classno, sizeof(int), "prune_tree_according_to_model_selection_method", 10);
                            find_subtree_class_counts(root, classcounts);
                            loglikelihood = log_likelihood_for_classification(classcounts);
                            safe_free(classcounts);
                            performance = aic_loglikelihood(loglikelihood, 0);
                            break;
   case MODEL_SELECTION_BIC:loglikelihoodbefore = decisiontree_loglikelihood(root);
                            d = decisiontree_complexity_count(root);
                            classcounts = safecalloc(current_dataset->classno, sizeof(int), "prune_tree_according_to_model_selection_method", 18);
                            find_subtree_class_counts(root, classcounts);
                            N = 0;
                            for (i = 0; i < current_dataset->classno; i++)
                              N += classcounts[i];
                            performancebefore = bic_loglikelihood(loglikelihoodbefore, d, N);
                            loglikelihood = log_likelihood_for_classification(classcounts);
                            safe_free(classcounts);
                            performance = bic_loglikelihood(loglikelihood, 0, N);
                            break;
   case MODEL_SELECTION_SRM:error = decisiontree_falsepositive_count(root) / (root->covered + 0.0);
                            switch (root->conditiontype)
                             {
                              case   UNIVARIATE_CONDITION:h = decisiontree_vc_dimension_recursive(current_dataset, root);
                                                          break;
                              case MULTIVARIATE_CONDITION:
                              case       HYBRID_CONDITION:h = omnivariatetree_vc_dimension(current_dataset, root);
                                                          break;
                             }
                            performancebefore = srm_classification(error, h, root->covered, get_parameter_f("srma1"), get_parameter_f("srma2"));
                            error = root->falsepositives / (root->covered + 0.0);
                            performance = srm_classification(error, 1, root->covered, get_parameter_f("srma1"), get_parameter_f("srma2"));
                            break;
   case MODEL_SELECTION_MDL:C = 0;
                            fp = 0;
                            subtree_covered_and_false_positives(root, &C, &fp);
                            exceptionlengthbefore = description_length_of_exceptions(C, 0, fp, 0, 1);
                            d = decisiontree_complexity_count(root);
                            performancebefore = mdl(exceptionlengthbefore, d, 0.01, BITS_FOR_REAL_NUMBER);
                            exceptionlength = description_length_of_exceptions(root->covered, 0, root->falsepositives, 0, 1);
                            performance = mdl(exceptionlength, 0, 0.01, BITS_FOR_REAL_NUMBER);
                            break;
   default                 :performance = 1.0;
                            performancebefore = 0.0;
                            break;
  }
 if (performance < performancebefore)
  {
   if (in_list(root->featureno, 2, LINEAR, QUADRATIC))
     matrix_free(root->w);
   accumulate_instances_tree(&(root->instances), root);
   free_children(root);
   root->featureno = LEAF_NODE;
  }
 else
  {
   if (root->featureno >= 0 && (root->conditiontype != HYBRID_CONDITION) && current_dataset->features[root->featureno].type == STRING)
     for (i = 0; i < current_dataset->features[root->featureno].valuecount; i++)
       prune_tree_according_to_model_selection_method(&(root->string[i]), modelselectionmethod);
   else
    {
     prune_tree_according_to_model_selection_method(root->left, modelselectionmethod);
     prune_tree_according_to_model_selection_method(root->right, modelselectionmethod);
    }
  }
}

void find_uni_and_multi_splits(Decisionnodeptr node, matrix* qw, double* qw0, double points[3], double variance_explained, int hybridspace)
{
 /*!Last Changed 22.11.2004*/
 node->featureno = find_best_ldt_feature_for_realized(node->instances, &(node->split));
 points[0] = node->featureno;
 node->lineard = find_best_multi_ldt_split(&(node->instances), &(node->w), &(node->w0), MULTIVARIATE_LINEAR, variance_explained);
 points[1] = node->lineard - 1;
 if (points[1] == -1)
   matrix_free(node->w);
 points[2] = -1;
 if (hybridspace == 3)
  {
   node->quadraticd = find_best_multi_ldt_split(&(node->instances), qw, qw0, MULTIVARIATE_QUADRATIC, variance_explained);
   points[2] = node->quadraticd - 1;
   if (points[2] == -1)
     matrix_free(*qw);
  }
}

void calculate_counts_for_ldt_splits(int univariate, Decisionnodeptr node, int *C, int *U, int *fp, int *fn)
{
 /*!Last Changed 22.11.2004*/
 int counts[2][MAXCLASSES], i, leftclass, rightclass;
 if (univariate)
   find_counts_for_split(node->instances, node->featureno, node->split, counts[0], counts[1], node->conditiontype);
 else
   find_counts_for_multivariate_split(node->instances, node->w, node->w0, counts[0], counts[1]);
 leftclass = find_max_in_list(counts[0], MAXCLASSES);
 rightclass = find_max_in_list(counts[1], MAXCLASSES);
 *C = 0;
 *U = 0;
 *fp = 0;
 *fn = 0;
 for (i = 0; i < current_dataset->classno; i++)
  {
   *C += counts[0][i];
   *U += counts[1][i];
   if (i != leftclass)
     *fp += counts[0][i];
   if (i != rightclass)
     *fn += counts[1][i];
  }
}

int set_best_hybrid_split(double results[3], Decisionnodeptr node, matrix qw, double qw0, int hybridspace)
{
 /*!Last Changed 01.02.2005*/
 int bestmodel;
 bestmodel = find_min_in_list_double(results, hybridspace);
 if (bestmodel == -1)
  {
   node->featureno = LEAF_NODE;
   return 0;
  }
 switch (bestmodel)
  {
   case MODEL_UNI:if (results[1] != -1)
                    matrix_free(node->w);
                  if (results[2] != -1 && hybridspace == 3)
                    matrix_free(qw);
                  break;
   case MODEL_LIN:node->featureno = LINEAR;
                  if (results[2] != -1 && hybridspace == 3)
                    matrix_free(qw);
                  break;
   case MODEL_QUA:node->featureno = QUADRATIC;
                  if (results[1] != -1)
                    matrix_free(node->w);
                  node->w = qw;
                  node->w0 = qw0;
                  break;
  }
 return 1;
}

int find_best_hybrid_ldt_split_single_model(Decisionnodeptr node, NODE_TYPE model, double variance_explained)
{
 /*!Last Changed 13.02.2005*/
 switch (model)
  {
   case UNIVARIATE:node->featureno = find_best_ldt_feature_for_realized(node->instances, &(node->split));
                   if (node->featureno == -1)
                     return 0;
                   break;
   case     LINEAR:node->lineard = find_best_multi_ldt_split(&(node->instances), &(node->w), &(node->w0), MULTIVARIATE_LINEAR, variance_explained);
                   if (node->lineard == 0)
                    {
                     matrix_free(node->w);
                     node->featureno = LEAF_NODE;
                     return 0;
                    }
                   else
                     node->featureno = LINEAR;
                   break;
   case  QUADRATIC:node->quadraticd = find_best_multi_ldt_split(&(node->instances), &(node->w), &(node->w0), MULTIVARIATE_QUADRATIC, variance_explained);
                   if (node->quadraticd == 0)
                    {
                     matrix_free(node->w);
                     node->featureno = LEAF_NODE;
                     return 0;
                    }
                   else
                     node->featureno = QUADRATIC;
                   break;
   default        :node->featureno = LEAF_NODE;
                   printf("This node type is not supported\n");
                   break;
  }
 return 1;
}

int find_best_hybrid_ldt_split_aic(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 01.02.2005*/
 /*!Last Changed 22.11.2004*/
 int hybridspace = get_parameter_i("hybridspace");
 matrix qw;
 int d, i;
 double qw0, results[3], loglikelihood;
 find_uni_and_multi_splits(node, &qw, &qw0, results, param->variance_explained, hybridspace);
 for (i = 0; i < hybridspace; i++)
   if (results[i] != -1)
    {
     switch (i)
      {
       case 0:loglikelihood = log_likelihood_for_uni_ldt_splits(node->instances, node->featureno, node->split, param->conditiontype);
              d = 2;
              break;
       case 1:loglikelihood = log_likelihood_for_multi_ldt_splits(node->instances, node->w, node->w0);
              d = node->lineard + 1;
              break;
       case 2:loglikelihood = log_likelihood_for_multi_ldt_splits(node->instances, qw, qw0);
              d = node->quadraticd + 1;
              break;
       default:loglikelihood = 0.0;
               d = 0;
               break;
      }
     results[i] = aic_loglikelihood(loglikelihood, d);
    }
 return set_best_hybrid_split(results, node, qw, qw0, hybridspace);
}

int find_best_hybrid_ldt_split_bic(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 01.02.2005*/
 /*!Last Changed 22.11.2004*/
 int hybridspace = get_parameter_i("hybridspace");
 int i, d, N = data_size(node->instances);
 matrix qw;
 double loglikelihood, qw0, results[3];
 find_uni_and_multi_splits(node, &qw, &qw0, results, param->variance_explained, hybridspace);
 for (i = 0; i < hybridspace; i++)
   if (results[i] != -1)
    {
     switch (i)
      {
       case 0:loglikelihood = log_likelihood_for_uni_ldt_splits(node->instances, node->featureno, node->split, param->conditiontype);
              d = 2;
              break;
       case 1:loglikelihood = log_likelihood_for_multi_ldt_splits(node->instances, node->w, node->w0);
              d = node->lineard + 1;
              break;
       case 2:loglikelihood = log_likelihood_for_multi_ldt_splits(node->instances, qw, qw0);
              d = node->quadraticd + 1;
              break;
       default:loglikelihood = 0.0;
               d = 0;
               break;
      }
     results[i] = bic_loglikelihood(loglikelihood, d, N);
    }
 return set_best_hybrid_split(results, node, qw, qw0, hybridspace);
}

int find_best_hybrid_ldt_split_srm(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 06.11.2007 added SRM prepruning*/
 /*!Last Changed 25.08.2007*/
 int hybridspace = get_parameter_i("hybridspace");
 int i, N = data_size(node->instances), h, counts[2][MAXCLASSES], bestmodel;
 matrix qw;
 double qw0, results[3], error, base_generalization_error;
 base_generalization_error = srm_classification(node->falsepositives / (N + 0.0), 1, N, param->a1, param->a2);
 find_uni_and_multi_splits(node, &qw, &qw0, results, param->variance_explained, hybridspace);
 for (i = 0; i < hybridspace; i++)
   if (results[i] != -1)
    {
     switch (i)
      {
       case 0:h = (int) log2(current_dataset->multifeaturecount - 1) + 1;
              find_counts_for_split(node->instances, node->featureno, node->split, counts[0], counts[1], HYBRID_CONDITION);
              break;
       case 1:h = node->lineard + 1;
              find_counts_for_multivariate_split(node->instances, node->w, node->w0, counts[0], counts[1]);
              break;
       case 2:h = node->quadraticd + 1;
              find_counts_for_multivariate_split(node->instances, qw, qw0, counts[0], counts[1]);
              break;
       default:h = 2;
               break;
      }
     error = error_of_split(counts, N);
     results[i] = srm_classification(error, h, N, param->a1, param->a2);
    }
 /*To implement prepruning, compare generalization error of the leaf with the best model*/
 bestmodel = find_min_in_list_double(results, hybridspace);
 if (bestmodel == -1 || results[bestmodel] > base_generalization_error)
  {
   if (hybridspace > 1 && results[1] != -1)
     matrix_free(node->w);
   if (hybridspace > 2 && results[2] != -1)
     matrix_free(qw);  
   node->featureno = LEAF_NODE;
   return 0;
  }
 else
   return set_best_hybrid_split(results, node, qw, qw0, hybridspace);
}

int find_best_hybrid_ldt_split_crossvalidation(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 22.11.2004*/
 int hybridspace = get_parameter_i("hybridspace");
 FILE* errorfiles[3];
 int i, j, k, datasize, trainsize, testsize, error, bestmodel = 0, *models, dummy, dummy2, testtype;
 Instanceptr train = NULL, test = NULL, traindata, testdata;
 char **files, st[READLINELENGTH] = "-", pst[READLINELENGTH], buffer[READLINELENGTH];
 files = safemalloc(hybridspace * sizeof(char *), "find_best_hybrid_ldt_split_crossvalidation", 5);
 strcpy(buffer, get_parameter_s("tempdirectory"));
 strcat(buffer, "/univariate.txt");
 files[0] = strcopy(files[0], buffer);
 strcpy(buffer, get_parameter_s("tempdirectory"));
 strcat(buffer, "/linear.txt");
 files[1] = strcopy(files[1], buffer);
 if (hybridspace == 3)
  {
   strcpy(buffer, get_parameter_s("tempdirectory"));
   strcat(buffer, "/quadratic.txt");
   files[2] = strcopy(files[2], buffer);
  }
 for (i = 0; i < hybridspace; i++)
  {
   errorfiles[i] = fopen(files[i], "w");
   if (!errorfiles[i])
     return 0;
  }
 for (i = 0; i < 5; i++)
  {
   divide_instancelist(&(node->instances), &train, &test, 2);
   trainsize = data_size(train);
   testsize = data_size(test);
   if (trainsize <= 1 || testsize <= 1)
    {
     node->instances = restore_instancelist(node->instances, train);
     merge_instancelist(&(node->instances), test); 
     bestmodel = 1;
     break;
    }
   for (j = 0; j < 2; j++)
    {
     if (j == 0)
      {
       traindata = train;
       testdata = test;
       datasize = testsize;
      }
     else
      {
       traindata = test;
       testdata = train;
       datasize = trainsize;
      }
     for (k = 0; k < hybridspace; k++)
      {
       switch (k)
        {
         case MODEL_UNI:error = find_error_and_free_matrix(&traindata, testdata, UNIVARIATE, param->variance_explained);
                        break;
         case MODEL_LIN:error = find_error_and_free_matrix(&traindata, testdata, LINEAR, param->variance_explained);
                        break;
         case MODEL_QUA:error = find_error_and_free_matrix(&traindata, testdata, QUADRATIC, param->variance_explained);
                        break;
        }
       set_precision(pst, NULL, "\n");
       fprintf(errorfiles[k], pst, error / (datasize + 0.0));
      }
    }
   node->instances = restore_instancelist(node->instances, testdata);
   merge_instancelist(&(node->instances), traindata); 
   train = NULL;
   test = NULL;
  }
 for (i = 0; i < hybridspace; i++)
   fclose(errorfiles[i]);
 if (bestmodel)
  {
   free_2d((void**)files, hybridspace);
   node->featureno = find_best_ldt_feature_for_realized(node->instances, &(node->split));
   if (node->featureno == LEAF_NODE)
     return 0;
   return 1;
  }
 testtype = get_parameter_l("testtype");
 if (in_list(testtype, 9, FTEST, PAIREDT5X2, COMBINEDT5X2, PERMUTATIONTEST, PAIREDPERMUTATIONTEST, SIGNTEST, RANKSUMTEST, SIGNEDRANKTEST, NOTEST))
   models = multiple_comparison(files, hybridspace, testtype, st, &dummy, &dummy2, param->correctiontype);
 else
   models = multiple_comparison(files, hybridspace, FTEST, st, &dummy, &dummy2, param->correctiontype);
 switch (models[0])
  {
   case 1:node->featureno = find_best_ldt_feature_for_realized(node->instances, &(node->split));
          break;
   case 2:node->lineard = find_best_multi_ldt_split(&(node->instances), &(node->w), &(node->w0), MULTIVARIATE_LINEAR, param->variance_explained);
          if (!node->lineard)
           {
            matrix_free(node->w);
            node->featureno = LEAF_NODE;
           }
          else
            node->featureno = LINEAR;
          break;
   case 3:node->quadraticd = find_best_multi_ldt_split(&(node->instances), &(node->w), &(node->w0), MULTIVARIATE_QUADRATIC, param->variance_explained);
          if (!node->quadraticd)
           {
            matrix_free(node->w);
            node->featureno = LEAF_NODE;
           }
          else
            node->featureno = QUADRATIC;
          break;
  }
 safe_free(models);
 free_2d((void**)files, hybridspace);
 return 1;
}

int find_best_hybrid_ldt_split_according_char(Decisionnodeptr node, char model, double variance_explained)
{
 /*!Last Changed 01.07.2006*/
 switch (model){
   case 'U':
   case 'u':return find_best_hybrid_ldt_split_single_model(node, UNIVARIATE, variance_explained);
            break;
   case 'L':
   case 'l':return find_best_hybrid_ldt_split_single_model(node, LINEAR, variance_explained);
            break;
   case 'Q':
   case 'q':return find_best_hybrid_ldt_split_single_model(node, QUADRATIC, variance_explained);
            break;
   default :printf(m1325);
            return NO;
            break;
 }
}

int find_best_hybrid_ldt_split(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 22.11.2004 can now find the best split using model selection methods other than cross-validation*/
 /*!Last Changed 23.09.2004*/
 switch (param->modelselectionmethod)
  {
   case MODEL_SELECTION_CV :return find_best_hybrid_ldt_split_crossvalidation(node, param);
                            break;
   case MODEL_SELECTION_AIC:return find_best_hybrid_ldt_split_aic(node, param);
                            break;
   case MODEL_SELECTION_BIC:return find_best_hybrid_ldt_split_bic(node, param);
                            break;
   case MODEL_SELECTION_SRM:return find_best_hybrid_ldt_split_srm(node, param);
                            break;
   case MODEL_SELECTION_MDL:break;
   case MODEL_SELECTION_UNI:return find_best_hybrid_ldt_split_single_model(node, UNIVARIATE, param->variance_explained);
                            break;
   case MODEL_SELECTION_LIN:return find_best_hybrid_ldt_split_single_model(node, LINEAR, param->variance_explained);
                            break;
   case MODEL_SELECTION_QUA:return find_best_hybrid_ldt_split_single_model(node, QUADRATIC, param->variance_explained);
                            break;
  }
 return 0;
}

int find_error_and_free_matrix(Instanceptr* train, Instanceptr test, NODE_TYPE node_type, double variance_explained)
{
 /*!Last Changed 03.12.2004*/
 int bestfeature, error, counts[2][MAXCLASSES], leftclass, rightclass;
 double split, w0;
 matrix w;
 if (node_type == UNIVARIATE)
  {
   bestfeature = find_best_ldt_feature_for_realized(*train, &split);
   if (bestfeature == -1)
     return data_size(test);
  }
 else
   if (!find_best_multi_ldt_split(train, &w, &w0, node_type, variance_explained))
    {
     matrix_free(w);
     return data_size(test);
    }
 if (node_type == UNIVARIATE)
   find_counts_for_split(*train, bestfeature, split, counts[0], counts[1], HYBRID_CONDITION);
 else
   find_counts_for_multivariate_split(*train, w, w0, counts[0], counts[1]);
 leftclass = find_max_in_list(counts[0], MAXCLASSES);
 rightclass = find_max_in_list(counts[1], MAXCLASSES);
 if (node_type == UNIVARIATE)
   return test_univariate(test, bestfeature, split, leftclass, rightclass);
 else
  {
   error = test_multivariate(test, w, w0, leftclass, rightclass);
   matrix_free(w);
   return error;
  }
}

int create_omnildtchildren(Decisionnodeptr node, Multildt_parameterptr param)
{
 /*!Last Changed 22.11.2004 added model selection method*/
 /*!Last Changed 23.09.2004*/
 node->conditiontype = HYBRID_CONDITION;
 if (!(can_be_divided(node, &(param->c45param))))
   return 1;
 if (!(find_best_hybrid_ldt_split(node, param)))
  {
   make_leaf_node(node);
   return 1;
  }
 if (!(make_multivariate_children(node)))
   return 1;
 create_omnildtchildren(node->left, param);
 create_omnildtchildren(node->right, param);
 return 1;
}
