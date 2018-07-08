#ifndef __process_H
#define __process_H

#include <stddef.h>
#include <stdio.h>
#include "network.h"
#include "matrix.h"
#include "svm.h"
#include "mlp.h"

#define MAX_STATISTICAL_TESTS 15
#define POSITION_COUNT 5
#define FONTCOUNT 4
#define MAX_CORRECTIONS 3
#define MAX_MODEL_SELECTION_METHODS 10
#define MAX_PRUNE_TYPES 4
#define MAX_PROBLEM_TYPE 8
#define MAX_LEAF_TYPE 2
#define MAX_KERNEL_TYPE 4
#define MAX_COMBINATION_TYPE 6
#define MAX_EXPERIMENT_DESIGN_TYPE 2
#define MAX_PROGRAMMING_LANGUAGE 6
#define MAX_EXPORT_TYPE 5
#define MAX_VC_CALCULATION_TYPE 2
#define MAX_HMM_TYPE 3
#define MAX_HMM_STATE_TYPE 5
#define MAX_HMM_LEARNING_TYPE 2
#define MAX_OS_COMMANDS 9

enum operating_system_type{ 
      WINDOWS = 0,
      SOLARIS,
      LINUX};

typedef enum operating_system_type OPERATING_SYSTEM_TYPE;

enum run_type{
      CROSSVALIDATION = 0,
      CROSSVALIDATION_WITH_SEPARATE_TEST_SET,
      BOOTSTRAP,
      TRAIN_TEST_SETS_GIVEN,
      TRAIN_TEST_CV_SETS_GIVEN};

typedef enum run_type RUN_TYPE;
     
enum operating_system_command_type{ 
      CLS = 0,
      COPY,
      CD,
      DEL,
      DIR,
      MOVE,
      MKDIR,
      RENAME,
      RMDIR};

typedef enum operating_system_command_type OPERATING_SYSTEM_COMMAND_TYPE;

void            add_single_data_line(char*** lines, int* count, char* myline);
void            autoencoder();
void            bootstrap_sample_with_stratification(int *prior_class_information, int* classcounts);
void            check_and_set_list_variable(void* variable, char* errormessage1, char* errormessage2, char* valuelist[], int listlength);
void            close_dataset(Datasetptr dataset);
void            convert_datasets(char separator, char* newdatadirectory, char* newinifile);
void            create_cvdata();
void            deallocate_remaining_memory();
void            divide_data_file(char* datasetname, char* separator, char* infilename, int partcount);
void            export_all_two_class_problems(Datasetptr d, char* inifile);
void            export_data_file(char* datasetname, char* separator, char* infilename, char* outfilename);
void            export_datafolds(char* separator, int newseed);
char*           export_file_name(Datasetptr dataset, int exporttype, int runno, int foldno, int partno);
void            export_single_file(char* separator, int exporttype, int runno, int foldno, Instanceptr data1, Instanceptr data2);
void            fill_missing_values(Instanceptr data, Instance mean);
void            free_train_and_test_lines();
void            get_fold(int fold, int* partition);
void            get_prior_information();
void*           get_single_instance(Datasetptr dataset, char myline[]);
void            getorder();
void            handle_missing_values(Instanceptr meandata, Instanceptr filleddata);
void            load_file(char *filename);
void**          load_model(char* infile, int* modelcount, int* algorithm, Instanceptr* means, Instanceptr* variances);
int             open_dataset(Datasetptr dataset);
void            os_command(OPERATING_SYSTEM_COMMAND_TYPE cmd_index);
int             outlier_features(char* datasetname);
void            post_process(int free_values);
void            pre_process();
void            pre_process_with_mean_and_variance(Instance mean, Instance variance, Instanceptr data);
int             prepare_from_files(Datasetptr dataset, char* param1, char* param2, char* param3);
void            print_classification_results();
void            process_order(int res);
void            read_and_normalize_train_file(Datasetptr dataset, Instanceptr* whereto);
void            read_data_from_lines(char** lines, int linecount, Instanceptr* data);
void            read_data_from_lines_with_probability(char** lines, int linecount, double* probability, Instanceptr* data);
int             read_from_data_file(Datasetptr dataset, char* datafilename, Instanceptr* whereto);
int             read_from_train_file(Datasetptr dataset, Instanceptr* whereto);
int*            read_train_and_test_lines(int** classcounts);
void            read_train_lines();
void            run_dimension_reduction_algorithm(int algorithm);
void            run_supervised_algorithm(int meta_algorithm, int algorithm);
void            save_model(FILE* modelfile, int algorithm, void* model, void* parameters);
void            single_run_supervised_algorithm(void* parameters, int meta_algorithm, int algorithm, RUN_TYPE runtype, FILE* posfile, FILE* modelfile, FILE* testcodefile, FILE* confusionfile);
void            test_algorithm(int algorithm, void* model, void* parameters, int testsize);
void            test_code();
void            test_meta_algorithm(int meta_algorithm, int algorithm, void** model, void* parameters, int testsize);
void            test_model(char* modelfilename, char* testfilename);
void            test_single(void* model, void* parameters, int algorithm, FILE* posfile, Instanceptr means, Instanceptr variances);
void            write_to_output_variables(int algorithm, void* model, Instanceptr* trdata, Instanceptr testdata);

#ifdef MPI
void get_fold_mpiclient(int fold, int* partition);
#endif

#endif
