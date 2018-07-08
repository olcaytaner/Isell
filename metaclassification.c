#include "algorithm.h"
#include "combine.h"
#include "dataset.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"
#include "libmisc.h"
#include "metaclassification.h"
#include "parameter.h"

extern Datasetptr current_dataset;
extern Algorithm classification_algorithms[CLASSIFICATION_ALGORITHM_COUNT];

int (*combination_algorithms[])(int*, matrix) = {&mean_combine, &max_combine, &min_combine, &median_combine, &prod_combine, &vote_combine};
Meta_Classification_Algorithm meta_classification_algorithms[META_CLASSIFICATION_ALGORITHM_COUNT] = {{train_one_vs_one, test_one_vs_one, "ONEVSONE"},
                                                        {train_one_vs_rest, test_one_vs_rest, "ONEVSREST"},
                                                        {train_bagging_classification, test_bagging_classification, "BAGGING"}};

void free_model_of_meta_classification_algorithm(int meta_algorithm, int algorithm, void** model, Datasetptr current_dataset)
{
 int i, d;
 d = current_dataset->classno;
 switch (meta_algorithm)
  {
   case         ONE_VERSUS_ONE:current_dataset->classno = 2;
                               for (i = 0; i < d * (d - 1) / 2; i++)
                                 free_model_of_supervised_algorithm(algorithm, model[i], current_dataset);
                               current_dataset->classno = d;
                               break;
   case        ONE_VERSUS_REST:current_dataset->classno = 2;
                               for (i = 0; i < d; i++)
                                 free_model_of_supervised_algorithm(algorithm, model[i], current_dataset);
                               current_dataset->classno = d;
                               break;
   case BAGGING_CLASSIFICATION:for (i = 0; i < get_parameter_i("weaklearnercount"); i++)
                                 free_model_of_supervised_algorithm(algorithm, model[i], current_dataset);
                               break;
  }
 safe_free(model);
}

void** train_one_vs_one(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters)
{
 Datasetptr tmp;
 int i, j, k = 0;
 void** model;
 Instanceptr train = NULL, cv = NULL;
 model = safemalloc(sizeof(void*) * (current_dataset->classno * (current_dataset->classno - 1) / 2), "train_one_vs_one", 2);
 for (i = 0; i < current_dataset->classno; i++)
   for (j = i + 1; j < current_dataset->classno; j++)
    {
     divide_instancelist_one_vs_one_with_copy(*trdata, &train, i, j);
     divide_instancelist_one_vs_one_with_copy(*cvdata, &cv, i, j);
     tmp = copy_of_dataset(current_dataset);
     current_dataset->classno = 2;
     current_dataset->classcounts[0] = tmp->classcounts[i];
     current_dataset->classcounts[1] = tmp->classcounts[j];
     model[k] = classification_algorithms[algorithm - SELECTMAX].train_algorithm(&train, &cv, parameters);
     current_dataset->classno = tmp->classno;
     current_dataset->classcounts[0] = tmp->classcounts[0];
     current_dataset->classcounts[1] = tmp->classcounts[1];
     free_dataset(tmp);
     if (!in_list(algorithm, 1, KNN))
      {
       deallocate(train);
       deallocate(cv);
      }
     k++;
    }
 return model;
}

Prediction test_one_vs_one(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior)
{
 int i, j, k = 0, *votes, tmp;
 Prediction result, prediction;
 double* tmp_posteriors;
 tmp_posteriors = safemalloc(current_dataset->classno * sizeof(double), "test_one_vs_one", 3);
 votes = safecalloc(current_dataset->classno, sizeof(int), "test_one_vs_one", 4);
 tmp = current_dataset->classno;
 for (i = 0; i < current_dataset->classno; i++)
   for (j = i + 1; j < current_dataset->classno; j++)
    {
     current_dataset->classno = 2;
     prediction = classification_algorithms[algorithm - SELECTMAX].test_algorithm(data, model[k], parameters, tmp_posteriors);
     current_dataset->classno = tmp;
     if (prediction.classno == 0)
       votes[i]++;
     else
       votes[j]++;
     k++;
    }
 safe_free(tmp_posteriors);
 result.classno = find_max_in_list(votes, current_dataset->classno);
 normalize_array(votes, posterior, current_dataset->classno);
 safe_free(votes);
 return result;
}

void** train_one_vs_rest(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters)
{
 Datasetptr tmp;
 int i;
 void** model;
 Instanceptr train = NULL, cv = NULL;
 model = safemalloc(sizeof(void*) * current_dataset->classno, "test_one_vs_rest", 2);
 for (i = 0; i < current_dataset->classno; i++)
  {
   divide_instancelist_one_vs_rest_with_copy(*trdata, &train, i);
   divide_instancelist_one_vs_rest_with_copy(*cvdata, &cv, i);
   tmp = copy_of_dataset(current_dataset);
   current_dataset->classno = 2;
   current_dataset->classcounts[0] = tmp->classcounts[i];
   current_dataset->classcounts[1] = tmp->datacount - tmp->classcounts[i];
   model[i] = classification_algorithms[algorithm - SELECTMAX].train_algorithm(&train, &cv, parameters);
   current_dataset->classno = tmp->classno;
   current_dataset->classcounts[0] = tmp->classcounts[0];
   current_dataset->classcounts[1] = tmp->classcounts[1];
   free_dataset(tmp);
   if (!in_list(algorithm, 1, KNN))
    {
     deallocate(train);
     deallocate(cv);
    }
  }
 return model;
}

Prediction test_one_vs_rest(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior)
{
 int i, tmp;
 double* tmp_posteriors;
 Prediction result;
 tmp_posteriors = safemalloc(current_dataset->classno * sizeof(double), "test_one_vs_rest", 3);
 tmp = current_dataset->classno;
 for (i = 0; i < current_dataset->classno; i++)
  {
   current_dataset->classno = 2;
   classification_algorithms[algorithm - SELECTMAX].test_algorithm(data, model[i], parameters, tmp_posteriors);
   current_dataset->classno = tmp;
   posterior[i] = tmp_posteriors[0];
  }
 safe_free(tmp_posteriors);
 normalize_array_of_doubles(posterior, current_dataset->classno);
 result.classno = findMaxOutput(posterior, current_dataset->classno);
 return result;
}

void** train_bagging_classification(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters)
{
 int i, learner_count;
 Instanceptr traindata, validationdata;
 void** model;
 learner_count = get_parameter_i("weaklearnercount");
 model = safemalloc(sizeof(void*) * learner_count, "train_bagging_classification", 4);
 for (i = 0; i < learner_count; i++)
  {
   traindata = bootstrap_sample_without_stratification(*trdata);
   if (is_cvdata_needed(algorithm))
     validationdata = bootstrap_sample_without_stratification(*cvdata);
   else
     validationdata = NULL;
   model[i] = classification_algorithms[algorithm - SELECTMAX].train_algorithm(&traindata, &validationdata, parameters);
   deallocate(traindata);
   deallocate(validationdata);
  }
 return model;
}

Prediction test_bagging_classification(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior)
{
 int i, learner_count;
 int* predictions;
 matrix posteriors;
 Prediction result, prediction;
 learner_count = get_parameter_i("weaklearnercount");
 predictions = safecalloc(learner_count, sizeof(int), "test_bagging_classification", 3);
 posteriors = matrix_alloc(learner_count, current_dataset->classno);
 for (i = 0; i < learner_count; i++)
  {
   prediction = classification_algorithms[algorithm - SELECTMAX].test_algorithm(data, model[i], parameters, posteriors.values[i]);
   predictions[i] = prediction.classno;
  }
 result.classno = combination_algorithms[get_parameter_l("combinationtype")](predictions, posteriors);
 safe_free(predictions);
 matrix_free(posteriors);
 return result;
}

Classificationresultptr ensemble(matrixptr* posteriors, int learner_count, int fold_count)
{
 int i, j, k, t, class_count, predicted_class, actual_class;
 matrix ensemble_posteriors;
 int* predictions;
 Classificationresultptr results;
 results = safemalloc(fold_count * sizeof(Classificationresult), "ensemble", 5);
 class_count = posteriors[0][0].col - 1;
 predictions = safemalloc(learner_count * sizeof(int), "ensemble", 7);
 ensemble_posteriors = matrix_alloc(learner_count, class_count);
 for (i = 0; i < fold_count; i++)
  {
   results[i].performance = 0;
   results[i].datasize = posteriors[0][i].row;
   results[i].classno = class_count;
   results[i].class_performance = safecalloc(class_count, sizeof(int), "ensemble", 14);
   results[i].class_counts = safecalloc(class_count, sizeof(int), "ensemble", 15);
   results[i].confusion_matrix = (int**) safecalloc_2d(sizeof(int*), sizeof(int), class_count, class_count, "ensemble", 16);
   for (j = 0; j < results[i].datasize; j++)
    {
     for (k = 0; k < learner_count; k++)
      {
       for (t = 0; t < class_count; t++)
         ensemble_posteriors.values[k][t] = posteriors[k][i].values[j][t];
       predictions[k] = find_max_in_list_double(ensemble_posteriors.values[k], class_count);
      }
     predicted_class = combination_algorithms[get_parameter_l("combinationtype")](predictions, ensemble_posteriors);
     actual_class = posteriors[0][i].values[j][class_count];
     (results[i].class_counts[actual_class])++;
     (results[i].confusion_matrix[actual_class][predicted_class])++;
     if (predicted_class == actual_class)
      {
       (results[i].performance)++;
       (results[i].class_performance[actual_class])++;
      }
    }
  }
 matrix_free(ensemble_posteriors);
 safe_free(predictions);
 return results;
}
