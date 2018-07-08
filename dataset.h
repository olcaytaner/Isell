#ifndef __dataset_H
#define __dataset_H

enum dataset_save_type{
      NAMES_C50 = 0,
      NAMES_RIPPER,
      NAMES_CLASSIC,
      NAMES_CLASSATEND,
      NAMES_AYSU,
      NAMES_CMU};

typedef enum dataset_save_type DATASET_SAVE_TYPE;

enum dataset_type{ALL = 0,
      CLASSIFICATION_DATASETS,
      REGRESSION_DATASETS,
      TWOCLASS_DATASETS,
      KCLASS_DATASETS,
      DISCRETE_DATASETS,
      CONTINUOUS_DATASETS,
      MIXED_DATASETS};

typedef enum dataset_type DATASET_TYPE;

#define PROPERTYCOUNT 9

#include "typedef.h"

void         add_dataset(Datasetptr dataset);
double       adherentsubsets(Datasetptr d);
double       average_valuecount(Datasetptr d);
double       class_entropy(char* datasetname);
int          connect_to_database(Datasetptr dataset);
void         convert_byte_format_to_dataset_format(char* inputfile, char separator, char* featuretypes, char* lengths, char* useful, char* outputfile);
Datasetptr   copy_of_dataset(Datasetptr d);
int          create_dataset(char* datasetname, char* separator, int classdefno, char* classes, char* directoryname, char* filename, char* discretefeatures);
int          create_kernel_dataset(char* datasetname, char* classes, char* directoryname, char* datafilename, char* kernelfilename);
int          create_multi_label_dataset(char* datasetname, char* separator, int class_start, char* directoryname, char* filename, char* discretefeatures);
int          create_sequential_dataset(char* datasetname, char* separator, int numfeatures, char* classes, char* directoryname, char* filename, char* discretefeatures);
Instanceptr  create_instance(Datasetptr dataset,char *myline, int withnoise);
Instanceptr  create_linear_test_set(Datasetptr d, int numexp, int mustrealize);
char*        dataset_directory_name(Datasetptr dataset);
char*        dataset_file_name(Datasetptr dataset);
void         deallocate_datasets();
char*        definition_file_name(Datasetptr dataset);
void         delete_dataset(char *dataset_name);
Datasetptr   dimension_reduced_dataset(Datasetptr d, int dimension, Instanceptr reduceddata, char* datasetname);
void         disconnect_from_database();
double*      discretization_entropy(Datasetptr original, int fno, int valuecount, Instanceptr data);
double*      discretization_equalwidth(Datasetptr original, int fno, int valuecount);
Datasetptr   discretize_dataset(Datasetptr original, int valuecount, DISCRETIZATION_TYPE discretization);
void         display_dataset_properties(char *dataset_name);
Datasetptr   empty_dataset(char* datasetname, char* directoryname, char* filename, char separator);
Instanceptr  empty_instance(int fcount);
Instanceptr  empty_multi_label_instance(int fcount, int classcount);
void         exchange_classdefno(Datasetptr d);
void         export_subset_of_features(Datasetptr d, Instanceptr list, int featurecount, char* outfilename);
void         extract_definitions_of_dataset(char* datasetname);
double       feature_efficiency(Datasetptr d, double *overlap);
int**        find_counts_for_all_splits(Datasetptr original, int fno, Instanceptr data, int* size, double** values);
double       find_opposite_class_ratio_in_mst(Datasetptr d);
double       fisher_discriminant_ratio(Datasetptr d);
void         free_dataset(Datasetptr dataset);
void         generate_data_with_noisy_classes_from_a_data_sample(Datasetptr d, char* inputfile, char *outputfile);
void         generate_data_with_noisy_features_from_a_dataset(Datasetptr d, Instanceptr data, char* fname, double stdev, double percentage);
Instanceptr  generate_random_data(Datasetptr d, int datasize);
Datasetptr   generate_random_dataset(char* name, int featurecount, char* featuretypes, int* featurevaluecounts, int numberofclasses);
int          k_way_anova_dataset(Datasetptr d, Instanceptr data, char** featurenos, int fcount);
void         k_way_anova_dataset_cont(Datasetptr d, Instanceptr data, int* features, int fdependent);
void         k_way_anova_dataset_discrete(Datasetptr d, Instanceptr data, int* features, int fdependent);
char*        kernel_file_name(Datasetptr dataset);
Datasetptr   load_definitions(char *fname, int* datasetcount);
double       lp_separability(Datasetptr d, double* trainerror, double* testerror);
int          multifeaturecount_2d(Datasetptr d);
double       nn_nonlinearity(Datasetptr d, Instanceptr testdata);
double       nn_ratio(Datasetptr d, double *estimate);
int          number_of_numeric_features(char * datasetname);
int          number_of_symbolic_features(char * datasetname);
int*         numeric_features(Datasetptr d, int* count);
void         print_cmu_data_header(FILE* outfile, Datasetptr d);
void         print_datasets(Datasetptr header, DATASET_TYPE printwhat);
void         read_definition_from_database(Datasetptr dataset);
int          read_kernel_of_dataset(Datasetptr d);
void         save_definitions_of_dataset_as_xml(FILE* outfile, Datasetptr d);
void         save_names_file(char* datasetname, char* namesfile);
Datasetptr   search_dataset(Datasetptr header,char *dataset_name);
void         subsample(Datasetptr d, char* outputfile, Instanceptr list, int datacount, double* weights);
void         subsample_regression(Datasetptr d, char* outputfile, Instanceptr list, int datacount);
int*         symbolic_features(Datasetptr d, int* count);
void         type_change(Datasetptr d);
void         type_restore(Datasetptr d);
double       volume_overlap_region(Datasetptr d);

#endif
