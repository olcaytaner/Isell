#include "algorithm.h"
#include "instance.h"
#include "instancelist.h"
#include "libmemory.h"
#include "libmisc.h"
#include "metaregression.h"
#include "parameter.h"

extern Algorithm regression_algorithms[REGRESSION_ALGORITHM_COUNT];

Meta_Regression_Algorithm meta_regression_algorithms[META_REGRESSION_ALGORITHM_COUNT] = {{train_bagging_regression, test_bagging_regression, "BAGGING"}};

void free_model_of_meta_regression_algorithm(int meta_algorithm, int algorithm, void** model, Datasetptr current_dataset)
{
 int i;
 switch (meta_algorithm)
  {
   case BAGGING_REGRESSION:for (i = 0; i < get_parameter_i("weaklearnercount"); i++)
                             free_model_of_supervised_algorithm(algorithm, model[i], current_dataset);
                           break;
  }
 safe_free(model);
}

void** train_bagging_regression(Instanceptr* trdata, Instanceptr* cvdata, int algorithm, void* parameters)
{
 int i, learner_count;
 Instanceptr traindata, validationdata;
 void** model;
 learner_count = get_parameter_i("weaklearnercount");
 model = safemalloc(sizeof(void*) * learner_count, "train_bagging_regression", 4);
 for (i = 0; i < learner_count; i++)
  {
   traindata = bootstrap_sample_without_stratification(*trdata);
   if (is_cvdata_needed(algorithm))
     validationdata = bootstrap_sample_without_stratification(*cvdata);
   else
     validationdata = NULL;
   model[i] = regression_algorithms[algorithm - ONEFEATURE].train_algorithm(&traindata, &validationdata, parameters);
  }
 return model;
}

Prediction test_bagging_regression(Instanceptr data, void** model, int algorithm, void* parameters, double* posterior)
{
 int i, learner_count;
 double* outputs;
 Prediction result, prediction;
 learner_count = get_parameter_i("weaklearnercount");
 outputs = safemalloc(sizeof(double) * learner_count, "test_bagging_regression", 4);
 for (i = 0; i < learner_count; i++)
  {
   prediction = regression_algorithms[algorithm - ONEFEATURE].test_algorithm(data, model[i], parameters, NULL);
   outputs[i] = prediction.regvalue;
  }
 qsort(outputs, learner_count, sizeof(double), sort_function_double_ascending);
 result.regvalue = outputs[learner_count / 2];
 return result;
}
