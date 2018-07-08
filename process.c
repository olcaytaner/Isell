#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <limits.h>
#include "messages.h"
#include "typedef.h"
#include "algorithm.h"
#include "convex.h"
#include "data.h"
#include "distributions.h"
#include "poly.h"
#include "statistics.h"
#include "screen.h"
#include "libarray.h"
#include "libfile.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "savenet.h"
#include "network.h"
#include "matrix.h"
#include "instance.h"
#include "instancelist.h"
#include "dataset.h"
#include "decisiontree.h"
#include "regressiontree.h"
#include "perceptron.h"
#include "graphdata.h"
#include "structure.h"
#include "rule.h"
#include "mlp.h"
#include "model.h"
#include "lang.h"
#include "eps.h"
#include "classification.h"
#include "regression.h"
#include "mixture.h"
#include "regressionrule.h"
#include "clustering.h"
#include "multiplemodel.h"
#include "svmprepare.h"
#include "parameter.h"
#include "loadmodel.h"
#include "savecode.h"
#include "savemodel.h"
#include "tests.h"
#include "plot.h"
#include "combine.h"
#include "srm.h"
#include "vc.h"
#include "interval.h"
#include "hmm.h"
#include "datastructure.h"
#include "command.h"
#include "rbf.h"
#include "dimreduction.h"
#include "binary.h"
#include "univariatetree.h"
#include "multivariatetree.h"
#include "omnivariaterule.h"
#include "omnivariatetree.h"
#include "nodeboundedtree.h"
#include "xmlparser.h"
#include "softtree.h"
#include "pstricks.h"
#include "process.h"
#include "svmsimplex.h"
#include "metaclassification.h"
#include "metaregression.h"

#ifdef MPI
#include "mpi.h"
#endif

/****TO ADD A SUPERVISED (CLASSIFICATION OR REGRESSION) LEARNING ALGORITHM****/
/*  WHAT TO DO                                                              FILE NAME                  
    ---------------------------------------------------------------------   ---------------------
1.  Add an order for the classifier or regressor                            command.txt
2.  Select a constant name, put it also increment ALGORITHMCOUNT            constants.h
3.  Add line run_supervised_algorithm(...) in the main                      process.c
4.  Add one more constant structure to the algorithms array                 algorithm.c
    Pay attention to the mustrealize, mustnormalize, cvdata_needed fields   
5.  Add train and test functions                                            classification.c, classification.h (regression.c, regression.h)
6.  If required construct model structure                                   classification.h (regression.h)
7.  Add function save_model_... to save the model                           savemodel.c, savemodel.h
8.  Add function load_model_... to be able to load the model                loadmodel.c, loadmodel.h
9.  Update save_model function with new algorithm                           process.c
10. Update load_model function with new algorithm                           process.c
11. Update free_model_of_supervised_algorithm with new algorithm            algorithm.c
12. If required construct parameter structure                               parameter.h
13. If required insert new parameter adding orders                          command.txt, parameters.xml, process.c
14. Update prepare_supervised_algorithm_parameters with new algorithm       algorithm.c
15. Update free_supervised_algorithm_parameters with new algorithm          algorithm.c
16. Update is_cvdata_needed if required                                     algorithm.c
17. Update write_to_output_variables if required                            process.c
18. Calculate complexity of the algorithm 
    Update function complexity_of_supervised_algorithm accordingly          algorithm.c
19. Write train and test functions                                          classification.c, classification.h (regression.c, regression.h)
20. Add function generate_test_code_... to generate test code               savecode.c, savecode.h
21. Update generate_test_code function with new algorithm                   algorithm.c
*/

/**********************************************************************************/

extern Meta_Classification_Algorithm meta_classification_algorithms[META_CLASSIFICATION_ALGORITHM_COUNT];
extern Meta_Regression_Algorithm meta_regression_algorithms[META_REGRESSION_ALGORITHM_COUNT];
extern Algorithm classification_algorithms[CLASSIFICATION_ALGORITHM_COUNT];
extern Algorithm regression_algorithms[REGRESSION_ALGORITHM_COUNT];
extern Dimension_reduction_algorithm dimension_reduction_algorithms[DIMENSION_REDUCTION_ALGORITHM_COUNT];

char *colors[COLORCOUNT] = {"black", "red", "green", "blue", "purple", "yellow", "cyan", "gray", "orange", "brown", "pink", "white"};
char *fonts[FONTCOUNT] = {"Helvetica","Courier","Times","Arial"};
char *imageparts[IMAGEPARTCOUNT] = {"data","legend","axis","label"};
char *legend_positions[POSITION_COUNT] = {"downright", "downleft", "upright", "upleft", "none"};
char *group_colorings[GROUP_COLORING_COUNT] = {"allsame", "groupsame", "individualsame"};
char *problem_type[MAX_PROBLEM_TYPE] = {"all", "classification", "regression", "twoclass", "kclass", "discrete", "continuous", "mixed"};
char *windows_commands[MAX_OS_COMMANDS] = {"cls", "copy", "cd", "del", "dir", "move", "mkdir", "rename", "rmdir"};
char *linux_commands[MAX_OS_COMMANDS] = {"reset", "cp", "cd", "rm", "ls", "mv", "mkdir", "mv", "rm -r"};
char *exportfiletypes[MAX_EXPORT_TYPE] = {"train", "test", "validation", "whole", "partition"};
char *mlp_simulation_algorithms[MAX_MLP_SIMULATION_ALGORITHMS]={"multitest","forw1","backw1","forw","backw"};

/* ODBC variables*/
#ifdef ODBC
#include <sqlext.h>
HENV henv;         /* Environment */
HDBC hdbc;         /* Connection handle */
HSTMT hstmt;       /* Statement handle */
RETCODE rc;        /* ODBC return code*/
unsigned char databuffer[SQL_MAX_MESSAGE_LENGTH + 1];
#endif

Image img;

/* Counters*/
int datasetcount, graphcount = 0, availableproccount = 1, varcount = 0;
int plotcount = 0, testlinecount = 0, trainlinecount = 0;

/* Language Variables*/
int goto_else_part = 0, goto_endif = 0, goto_while = 0, goto_endwhile = 0, goto_endfor = 0;
int instruction_pointer, correctcase, paramcount;

char *order = NULL, *params[MAXPARAMS] = {NULL}, *inswitch = NULL;
char orderst[MAXLENGTH], **testlines = NULL, **trainlines = NULL;

variableptr vars = NULL;

/* Variables about datasets in general*/
Datasetptr current_dataset = NULL, Datasets = NULL;

char *datadir = NULL, exportfiletemplates[MAX_EXPORT_TYPE][MAXLENGTH] = {"", "", "", "", ""};

/* Global variables used in running algorithms*/
Instanceptr traindata = NULL, testdata = NULL, cvdata = NULL, separateddata = NULL, class_means = NULL;

Instance mean, variance;
Instance testmean, testvariance;

double outputmean, outputvariance;

int *prior_class_information = NULL, rank = 0;
int mustrealize, mustnormalize, convert_missing_values = YES, totalrun;

Command_nodeptr commandtree;

/* Variable to display the performance of the algorithms*/
void* result;

FILE *output;
FILE *datafile;

Stackptr ipstack;

/* Graph Variables*/
bayesiangraphptr *Graphs = NULL;

extern int ordercount;
extern Orderptr orders;
/**********************************************************************************/

/**
 * Using the mean vector of the instances (meandata) fills the missing values of another (may be the same) set of instances (filleddata)
 * @param[in] meandata Link list of instances of which the mean vector will be found
 * @param[in] filleddata Link list of instances having missing values which are going to be filled
 */
void handle_missing_values(Instanceptr meandata, Instanceptr filleddata)
{
 /*!Last Changed 03.06.2006*/
 /*!Last Changed 16.10.2005*/
 Instance mean;
 mean = find_mean_with_missing(meandata);
 fill_missing_values(filleddata, mean);
 safe_free(mean.values);
}

/**
 * Fills the missing values of a set of instances using mean imputation
 * @param[in] data Link list of instances which may have one or more missing features
 * @param[in] mean Mean vector stored as an instance
 */
void fill_missing_values(Instanceptr data, Instance mean)
{
 /*!Last Changed 16.10.2005*/
 int i;
 Instanceptr tmp;
 tmp = data;
 while (tmp)
  {
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:if (tmp->values[i].intvalue == ISELL)
                      tmp->values[i].intvalue = mean.values[i].intvalue;
                    break;
       case    REAL:if (tmp->values[i].floatvalue == ISELL)
                      tmp->values[i].floatvalue = mean.values[i].floatvalue;
                    break;
       case  STRING:if (tmp->values[i].stringindex == -1)
                      tmp->values[i].stringindex = mean.values[i].stringindex;
                    break;
      }
   tmp = tmp->next;
  }
}

/**
 * Divides some data from a dataset into k parts and saves each part in another file
 * @param[in] datasetname Name of the dataset
 * @param[in] separator Separator character used in writing the instances to the output files
 * @param[in] infilename Name of the file containing the data to be partitioned
 * @param[in] partcount Number of parts
 */
void divide_data_file(char* datasetname, char* separator, char* infilename, int partcount)
{
 /*!Last Changed 23.02.2007 added minimum partcount and check for datasetname*/
 /*!Last Changed 03.08.2006*/
 FILE* outfile;
 char* exportfname;
 int i;
 Instanceptr* parts;
 if (partcount < 2)
  {
   printf(m1367);
   return;
  }
 current_dataset = search_dataset(Datasets, datasetname);
 if (!current_dataset)
  {
   printf(m1279, datasetname);
   return;
  }
 if (!read_from_data_file(current_dataset, infilename, &traindata)) 
   return;
 parts = divide_instancelist_into_parts(&traindata, partcount);
 for (i = 0; i < partcount; i++)
  { 
   exportfname = export_file_name(current_dataset, 4, -1, -1, i + 1);
   outfile = fopen(exportfname, "w");
   print_instance_list(outfile, *separator, current_dataset, parts[i]);
   fclose(outfile);
  }
 traindata = NULL;
 for (i = 0; i < partcount; i++)
   deallocate(parts[i]);
 safe_free(parts);
}

/**
 * Export some data from a dataset to another file in another format. The format will be given by general program parameters
 * @param[in] datasetname Name of the dataset
 * @param[in] separator Separator character used in writing the instances to the output files
 * @param[in] infilename Name of the file containing the data to be exported
 * @param[in] outfilename Name of the output file 
 */
void export_data_file(char* datasetname, char* separator, char* infilename, char* outfilename)
{
 /*!Last Changed 21.10.2005 added read_from_data_file*/
 /*!Last Changed 29.09.2004*/
 /*!Last Changed 28.10.2003*/
 FILE* outfile;
 mustnormalize = NO;
 outfile = fopen(outfilename, "w");
 if (!outfile)
   return;
 current_dataset = search_dataset(Datasets, datasetname);
 if (!current_dataset)
  {
   printf(m1279, datasetname);
   return;
  }
 if (!read_from_data_file(current_dataset, infilename, &traindata)) 
   return;
 pre_process();
 print_instance_list(outfile, *separator, current_dataset, traindata);
 post_process(YES);
 deallocate(traindata);
 traindata = NULL;
 fclose(outfile);
}

/**
 * Gets a command in the ISELL environment. Displays '>>' and waits for user input. If the user enters some command with parameters, if the string contains replacement strings (variables between %'s) make appropriate replacements, divides the string into command part, put all the parameters to the params array and changes special characters in the parameters such as _ for space.
 */
void getorder()
{
 /*!Last Changed 23.02.2007 added maximum parameter exceed message*/
 int i;
 char* p;
 printf(">>");
 fgets(orderst, MAXLENGTH, stdin);
 orderst[strlen(orderst) - 1] = '\0';
 change_according_to_variables(orderst);
 p = strtok(orderst," \t");
 if (p)
  {
   order = strcopy(order, p);
   paramcount = -1;
   do{
     paramcount++;
     p = strtok(NULL, " \t");
     if (p)
       params[paramcount] = strcopy(params[paramcount], p);
   }while(p && (paramcount < MAXPARAMS - 1));
  }
 if (paramcount >= MAXPARAMS - 1)
   printf(m1368);
 for (i = 0; i < paramcount; i++)
   change_special_characters(params[i]);
 params[paramcount] = NULL;
}

/**
 * Finds number of outlier features of a dataset. If the ratio of the variance of the feature to the average of feature values of the the nonoutlier instances (the instances between 5 and 95 percentile's) is less than 0.7, the feature is called an outlier feature.
 * @param[in] datasetname Name of the dataset
 * @return Number of outlier features
 * @warning Discrete features are not counted
 */
int outlier_features(char* datasetname)
{
 /*!Last Changed 23.02.2007 added free of variancefeatures*/
 /*!Last Changed 26.03.2004*/
 Datasetptr tmp;
 Instanceptr list;
 Instance variancefeatures;
 int i, count = 0, size, j, start, end;
 double sum;
 tmp = search_dataset(Datasets, datasetname);
 if (!tmp)
  {
   printf(m1279, datasetname);
   return -1;
  }
 current_dataset = tmp;
 if (traindata)
  {
   deallocate(traindata);
   traindata = NULL;
  }
 read_from_train_file(tmp, &traindata);
 variancefeatures = find_variance(traindata);
 for (i = 0; i < tmp->featurecount; i++)
   if (in_list(tmp->features[i].type, 2, REAL, INTEGER))
    {
     list = sort_instances(traindata, i, &size);
     sum = 0;
     start = (int)(size * 0.05);
     end = (int)(size * 0.95);
     for (j = start; j <= end; j++)
       sum += list[j].values[i].floatvalue;
     sum /= (end - start + 1);
     if (variancefeatures.values[i].floatvalue / sum < 0.7)
       count++;
     safe_free(list);
    }
 safe_free(variancefeatures.values);
 return count;
}

/**
 * Creates another copy of datasets in another metadataset directory and saves the information about the datasets in another ini file.
 * @param[in] separator Separator character used in writing the instances to the output files
 * @param[in] newdatadirectory New etadataset directory
 * @param[in] newinifile New ini file containing the information about the datasets
 * @warning The directories are automatically created using mkdir command.
 */
void convert_datasets(char separator, char* newdatadirectory, char* newinifile)
{
 /*!Last Changed 02.01.2006*/
 /*!Last Changed 02.11.2005*/
 Datasetptr d;
 Feature tmp;
 char tmpdir[READLINELENGTH], *fname, *dname, tmpcommand[READLINELENGTH];
 int i;
 FILE* outfile;
 Instanceptr data;
 convert_missing_values = NO;
 outfile = fopen(newinifile, "w");
 if (!outfile)
  {
   printf(m1248, newinifile);
   return;
  }
 fprintf(outfile, "<datasets>\n");
 strcpy(tmpdir, datadir);
 d = Datasets;
 while (d)
  {
   read_from_train_file(d, &data);
   safe_free(datadir);
   datadir = strcopy(datadir, newdatadirectory);
   fname = dataset_file_name(d);
   outfile = fopen(fname, "w");
   if (!outfile)
    {
     printf(m1248, newinifile);
     continue;
    }
   current_dataset = d;
   print_instance_list(outfile, separator, d, data);
   fclose(outfile);
   safe_free(fname);
   deallocate(data);
   if (d->classdefno != d->featurecount - 1)
    {
     tmp = d->features[d->classdefno];
     for (i = d->classdefno; i < d->featurecount - 1; i++)
       d->features[i] = d->features[i + 1];
     d->features[d->featurecount - 1] = tmp;
    }
   swap_char(&(d->separator), &separator);
   dname = dataset_directory_name(d);
   sprintf(tmpcommand, "mkdir %s", dname);
   safe_free(dname);
   system(tmpcommand);
   save_definitions_of_dataset_as_xml(outfile, d);
   swap_char(&(d->separator), &separator);
   if (d->classdefno != d->featurecount - 1)
    {
     tmp = d->features[d->featurecount - 1];
     for (i = d->featurecount - 1; i > d->classdefno; i--)
       d->features[i] = d->features[i - 1];
     d->features[d->classdefno] = tmp;
    }
   printf(m1225, d->name);
   d = d->next;
   safe_free(datadir);
   datadir = strcopy(datadir, tmpdir);
  }
 fprintf(outfile, "</datasets>\n");
 fclose(outfile);
 convert_missing_values = YES;
}

/**
 * Main function to read data from a dataset file.
 * @param[in] dataset Pointer to the dataset
 * @param[in] datafilename Dataset file name.
 * @param[out] whereto Pointer to the first instance of the new dataset
 * @return SUCCESS if the dataset is correctly read and stored as a link list of instances, FAILURE otherwise.
 */
int read_from_data_file(Datasetptr dataset, char* datafilename, Instanceptr* whereto)
{
 /*!Last Changed 31.10.2005*/
 FILE *myfile;
 char myline[READLINELENGTH];
 int j = 0;
 Instanceptr currentinstance, instancebefore = NULL;
 myfile = fopen(datafilename, "r");
 if (!myfile)
  {
   printf(m1121, datafilename);
   return FAILURE;
  }
 while (fgets(myline, READLINELENGTH, myfile))
  {
   currentinstance = create_instance(dataset, myline, get_parameter_o("runwithclassnoise"));
   if (!currentinstance)
     continue;
   if (instancebefore)
     instancebefore->next = currentinstance;
   else
     *whereto = currentinstance;
   currentinstance->index = j;
   j++;
   instancebefore = currentinstance;
  }
 fclose(myfile);
 if (convert_missing_values)
   handle_missing_values(*whereto, *whereto);
 return SUCCESS;
}

/**
 * Main function to read data from the original dataset's file. Calls read_from_data_file.
 * @param[in] dataset Pointer to the dataset
 * @param[out] whereto Pointer to the first instance of the new dataset
 * @return SUCCESS if the dataset is correctly read and stored as a link list of instances, FAILURE otherwise.
 */
int read_from_train_file(Datasetptr dataset, Instanceptr* whereto)
{
 /*!Last Changed 31.10.2005*/
 char *fname;
 int returnvalue;
 fname = dataset_file_name(dataset);
 returnvalue = read_from_data_file(dataset, fname, whereto);
 safe_free(fname);
 return returnvalue;
}

void read_and_normalize_train_file(Datasetptr dataset, Instanceptr* whereto)
{
 read_from_train_file(dataset, whereto);
 mustrealize = YES;
 mustnormalize = YES;
 realize(*whereto);
 exchange_classdefno(dataset);
 mean = find_mean(*whereto);
 variance = find_variance(*whereto);
 normalize(*whereto, mean, variance);
 safe_free(mean.values);
 safe_free(variance.values);
}

/**
 * Main function to read data from two files. One file for the training data, one file for the test data.
 * @param[in] dataset Pointer to the dataset
 * @param[in] param1 Parameter for the training data. If it is an integer, the dataset directory is searched for a data file as given in the exportfiletemplates[0], otherwise the parameter contains the name of the training file
 * @param[in] param2 Parameter for the test data. If it is an integer, the dataset directory is searched for a data file as given in the exportfiletemplates[1], otherwise the parameter contains the name of the training file
 * @return SUCCESS if the two datasets are correctly read and stored link list of instances, FAILURE otherwise.
 */
int prepare_from_files(Datasetptr dataset, char* param1, char* param2, char* param3)
{
 /*!Last Changed 02.11.2005*/
 /*!Last Changed 23.11.2004 added traincount*/
 char *fname1, *fname2;
 if (isinteger(param1))
  {
   fname1 = export_file_name(dataset, 0, atoi(param1), atoi(param2), -1);
   if (!fname1)
     return FAILURE;
   fname2 = export_file_name(dataset, 1, atoi(param1), atoi(param2), -1);
   if (!fname2)
     return FAILURE;
  }
 else
  {
   fname1 = param1;
   fname2 = param2;
  }
 if (!read_from_data_file(dataset, fname1, &traindata))
   return FAILURE;
 if (!read_from_data_file(dataset, fname2, &testdata))
   return FAILURE;
 if (param3 != NULL && !read_from_data_file(dataset, param3, &cvdata))
   return FAILURE;
 if (isinteger(param1))
  {
   safe_free(fname1);
   safe_free(fname2);
  }
 current_dataset->traincount = data_size(traindata);
 return SUCCESS;
}

#ifdef MPI
void get_fold_mpiclient(int fold, int* partition)
{
 /*!Last Changed 23.11.2004 added traincount*/
 long i, trainindex;
 Instanceptr currentinstance,trainbefore = NULL;
 if (traindata)
  {
   deallocate(traindata);
   traindata = NULL;
  }
 i = 0;
 trainindex = 0;
 while (i < trainlinecount)
  {
   if (partition[i] % get_parameter_i("foldcount") != fold)
    {
     if ((get_parameter_i("proccount") > 1) && (trainindex % (get_parameter_i("proccount") - 1) == (rank - 1)))
      {
       currentinstance = create_instance(current_dataset, trainlines[i]);
       if (!currentinstance)/*If the line does not contain correct instance*/
         continue;
       if (trainbefore)
        {
         trainbefore->next = currentinstance;
         trainbefore = currentinstance;
        }
       else
        {
         trainbefore = currentinstance;
         traindata = currentinstance;
        }
       currentinstance->index = trainindex;
      }
     trainindex++;
    }
   i++;
  }
 current_dataset->traincount = trainindex;
 if (convert_missing_values)
   handle_missing_values(traindata, traindata);
}
#endif

/**
 * Gets a single instance from the dataset. For the flat file datasets this means reading one line from the original dataset's file. For datasets stored in database tables this means getting one row from the table and concatenating the table fields to produce a string.
 * @param[in] dataset Pointer to the dataset
 * @param[out] myline[] Output string containing the features of instances
 * @return myline if successful, NULL otherwise.
 */
void* get_single_instance(Datasetptr dataset, char myline[])
{
 /*!Last Changed 07.04.2007 added STORE_TIME*/
 /*!Last Changed 29.12.2005*/
 #ifdef ODBC
 SQLINTEGER resultsize;
 double valuedouble;
 short int valueint;
 int i;
 #endif 
 if (in_list(current_dataset->storetype, 3, STORE_TEXT, STORE_SEQUENTIAL, STORE_KERNEL))
   return fgets(myline, READLINELENGTH, datafile);
 else
  {
   #ifdef ODBC
   rc = SQLFetch(hstmt);
   if (rc == SQL_SUCCESS)
    {
     sprintf(myline, "");
     for (i = 0; i < dataset->featurecount; i++)
       switch (dataset->features[i].type)
        {
         case  INTEGER:SQLGetData(hstmt, i + 1, SQL_C_SSHORT, &valueint, sizeof(valueint), &resultsize);
                       sprintf(myline, "%s%c%d", myline, dataset->separator, valueint);
                       break;
         case     REAL:
         case   REGDEF:SQLGetData(hstmt, i + 1, SQL_C_DOUBLE, &valuedouble, sizeof(valuedouble), &resultsize);
                       sprintf(myline, "%s%c%.8lf", myline, dataset->separator, valuedouble);
                       break;
         case   STRING:
         case CLASSDEF:SQLGetData(hstmt, i + 1, SQL_C_CHAR, databuffer, sizeof(databuffer), &resultsize);
                       sprintf(myline, "%s%c%s", myline, dataset->separator, databuffer);
                       break;
        }
     return myline;
    }
   else
     return NULL;
   #endif
  }
 return NULL;
}

/**
 * Allocates and fills the prior_class_information array. prior_class_information[i] stores the class index of the instance i.
 */
void get_prior_information()
{
 /*!Last Changed 20.06.2008 added deallocate*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 char myline[READLINELENGTH];
 long i;
 Instanceptr currentinstance;
 if (prior_class_information)
   safe_free(prior_class_information); 
 prior_class_information = (int *)safemalloc(current_dataset->datacount * sizeof(int), "get_prior_information", 9);
 if (!open_dataset(current_dataset))
   return;
 i = 0;
 while (get_single_instance(current_dataset, myline))
  {
   currentinstance = create_instance(current_dataset, myline, NO);
   if (!currentinstance)/*If the line does not contain correct instance*/
     continue;
   prior_class_information[i] = give_classno(currentinstance);
   deallocate(currentinstance);
   i++;
  }
 close_dataset(current_dataset);
}

/**
 * Creates a bootstrap sample for running.
 * @param[in] prior_class_information Class information of each instance. prior_class_information[i] is the class of the instance i.
 * @param[in] classcounts Number of instances in each class. classcounts[i] represents the number of instances of class i.
 */
void bootstrap_sample_with_stratification(int *prior_class_information, int* classcounts)
{
 long i, j;
 Instanceptr currentinstance, trainbefore = NULL;
 int* tmpcounts = safemalloc(sizeof(int) * current_dataset->classno, "bootstrap_sample_with_stratification", 2);
 for (i = 0; i < current_dataset->classno; i++)
   tmpcounts[i] = classcounts[i];
 i = 0;
 while (i < trainlinecount)
  {
   j = myrand() % trainlinecount;
   while (tmpcounts[prior_class_information[j]] == 0)
     j = myrand() % trainlinecount;
   (tmpcounts[prior_class_information[j]])--;
   currentinstance = create_instance(current_dataset, trainlines[j], NO);
   if (i != 0)
     trainbefore->next = currentinstance;
   else
     traindata = currentinstance;
   trainbefore = currentinstance;
   currentinstance->index = i;
   i++;
  }
 safe_free(tmpcounts);
 current_dataset->traincount = i;
 if (convert_missing_values)
   handle_missing_values(traindata, traindata);
}

/**
 * Gets i'th fold of a dataset. By i'th fold, we mean the training data and the test data.
 * @param[in] fold Index of the fold
 * @param[in] partition To produce stratified partitioning in the folds, partition[i] stores the stratified partitioning number of instance i. We find the fold of an instance by taking the modulo to number of folds. If fold index is equal to this number the instance will go to the test set, otherwise it will go to the training set.
 */
void get_fold(int fold, int* partition)
{
 /*!Last Changed 25.05.2006*/
 /*!Last Changed 30.10.2005*/
 /*!Last Changed 23.11.2004 added traincount*/
 long i, trainindex;
 Instanceptr currentinstance, trainbefore = NULL, testbefore = NULL;
 i = 0;
 trainindex = 0;
 while (i < trainlinecount)
  {
   currentinstance = create_instance(current_dataset, trainlines[i], get_parameter_o("runwithclassnoise"));
   if (!currentinstance)/*If the line does not contain correct instance*/
     continue;
   if (partition[i] % get_parameter_i("foldcount") != fold)
    {
     if (trainbefore)
       trainbefore->next = currentinstance;
     else
       traindata = currentinstance;
     trainbefore = currentinstance;
     currentinstance->index = trainindex;
     trainindex++;
    }
   else
    {
     if (testbefore)
       testbefore->next = currentinstance;
     else
       testdata = currentinstance;
     testbefore = currentinstance;
     currentinstance->index = i;
    }
   i++;
  }
 current_dataset->traincount = trainindex;
 if (convert_missing_values)
  {
   handle_missing_values(traindata, traindata);
   handle_missing_values(traindata, testdata);
  }
}

/**
 * Generates all possible two class datasets from a given dataset.
 * @param d The dataset
 * @param inifile Name of the ini file to save the definitions of the dataset
 */
void export_all_two_class_problems(Datasetptr d, char* inifile)
{
/*!Last Changed 09.01.2009*/
 int i, j, k, p, classno = d->classno, tmpcount = d->datacount, tmpclasscounts[MAXCLASSES];
 char tmpname[READLINELENGTH];
 char tmpcommand[READLINELENGTH];
 char backupname[READLINELENGTH];
 char backupdirectory[READLINELENGTH];
 char backupclasses[MAXCLASSES][READLINELENGTH];
 char backupvalues[MAXCLASSES][READLINELENGTH];
 char* fname;
 char* dname;
 FILE* datafile;
 FILE* xmlfile;
 get_prior_information();
 read_train_lines();
 k = 0;
 strcpy(backupname, d->name);
 safe_free(d->name);
 strcpy(backupdirectory, d->directory);
 safe_free(d->directory);
 for (i = 0; i < classno; i++)
  {
   tmpclasscounts[i] = d->classcounts[i];
   strcpy(backupclasses[i], d->classes[i]);
   safe_free(d->classes[i]);
   strcpy(backupvalues[i], d->features[d->classdefno].strings[i]);
   safe_free(d->features[d->classdefno].strings[i]);
  }
 d->classno = 2;
 d->features[d->classdefno].valuecount = 2;
 xmlfile = fopen(inifile, "w");
 fprintf(xmlfile, "<datasets>\n");
 for (i = 0; i < classno; i++)
   for (j = i + 1; j < classno; j++)
    {
     k++;
     sprintf(tmpname, "%s%d", backupname, k);
     d->name = strcopy(d->name, tmpname);
     d->datacount = tmpclasscounts[i] + tmpclasscounts[j];
     d->classcounts[0] = tmpclasscounts[i];
     d->classcounts[1] = tmpclasscounts[j];
     sprintf(tmpname, "%s%d", backupdirectory, k);
     d->directory = strcopy(d->directory, tmpname);
     d->classes[0] = strcopy(d->classes[0], backupclasses[i]);
     d->classes[1] = strcopy(d->classes[1], backupclasses[j]);
     d->features[d->classdefno].strings[0] = strcopy(d->features[d->classdefno].strings[0], backupvalues[i]);
     d->features[d->classdefno].strings[1] = strcopy(d->features[d->classdefno].strings[1], backupvalues[j]);
     dname = dataset_directory_name(d);
     sprintf(tmpcommand, "mkdir %s", dname);
     safe_free(dname);
     system(tmpcommand);
     save_definitions_of_dataset_as_xml(xmlfile, d);
     fname = dataset_file_name(d);
     datafile = fopen(fname, "w");
     for (p = 0; p < trainlinecount; p++)
       if (prior_class_information[p] == i || prior_class_information[p] == j)
         fprintf(datafile, "%s", trainlines[p]);
     fclose(datafile);
     safe_free(fname);
     safe_free(d->name);
     safe_free(d->directory);
     safe_free(d->classes[0]);
     safe_free(d->classes[1]);
     safe_free(d->features[d->classdefno].strings[0]);
     safe_free(d->features[d->classdefno].strings[1]);
    }
 fprintf(xmlfile, "</datasets>\n");
 fclose(xmlfile);
 d->datacount = tmpcount;
 d->classno = classno;
 d->features[d->classdefno].valuecount = d->classno;
 d->name = strcopy(d->name, backupname);
 d->directory = strcopy(d->directory, backupdirectory);
 for (i = 0; i < classno; i++)
  {
   d->classcounts[i] = tmpclasscounts[i];
   d->classes[i] = strcopy(d->classes[i], backupclasses[i]);
   d->features[d->classdefno].strings[i] = strcopy(d->features[d->classdefno].strings[i], backupvalues[i]);
  }
 free_train_and_test_lines();
}

char* export_file_name(Datasetptr dataset, int exporttype, int runno, int foldno, int partno)
{
 /*!Last Changed 03.08.2006 added partno*/
 /*!Last Changed 31.10.2005*/
 char *fname, *st, converted[10];
 int i, len, len2, j = 0, k;
 fname = (char *)safemalloc(100, "definition_file_name", 2);
 st = exportfiletemplates[exporttype];
 len = strlen(st);
 if (len == 0)
  { 
   safe_free(fname);
   return NULL;
  }
 for (i = 0; i < len; i++)
   switch (st[i])
    {
     case 'D':len2 = strlen(dataset->name);
              for (k = 0; k < len2; k++, j++)
                fname[j] = dataset->name[k];
              break;
     case 'R':sprintf(converted, "%d", runno);
              len2 = strlen(converted);
              for (k = 0; k < len2; k++, j++)
                fname[j] = converted[k];
              break;
     case 'F':sprintf(converted, "%d", foldno);
              len2 = strlen(converted);
              for (k = 0; k < len2; k++, j++)
                fname[j] = converted[k];
              break;
     case 'P':sprintf(converted, "%d", partno);
              len2 = strlen(converted);
              for (k = 0; k < len2; k++, j++)
                fname[j] = converted[k];
              break;
     default :fname[j] = st[i];
              j++;
              break;
    }
 fname[j] = '\0';
 return fname;
}

void export_valdata(char* separator, int newseed)
{
 /*!Last Changed 13.02.2008 added regression for validation data export*/
 /*!Last Changed 21.04.2007 added export_single_file*/
 /*!Last Changed 24.11.2005*/
 /*!Last Changed 30.10.2005*/
 int *classes, *classcounts = NULL;
 Instanceptr exportdata;
 mustnormalize = NO;
 get_prior_information();
 mysrand(newseed);
 classes = read_train_and_test_lines(&classcounts);
 read_data_from_lines(testlines, testlinecount, &exportdata);
 pre_process_with_mean_and_variance(mean, variance, exportdata);
 export_single_file(separator, 2, -2, -2, exportdata, NULL);
 post_process(NO);
 deallocate(exportdata);
 read_data_from_lines(trainlines, trainlinecount, &exportdata);
 pre_process_with_mean_and_variance(mean, variance, exportdata);
 export_single_file(separator, 3, -2, -2, exportdata, NULL);
 post_process(NO);
 deallocate(exportdata);
 free_train_and_test_lines();
 if (classes)
  {
   safe_free(classes);
   safe_free(classcounts);
  }
}

void export_single_file(char* separator, int exporttype, int runno, int foldno, Instanceptr data1, Instanceptr data2)
{
 /*!Last Changed 21.04.2007*/
 FILE* outfile;
 char* fname;
 fname = export_file_name(current_dataset, exporttype, runno + 1, foldno + 1, -1);
 if (!fname)
   return;
 outfile = fopen(fname, "w");
 if (!outfile)
   return;
 if (get_parameter_l("namestype") == NAMES_CMU)
  {
   print_cmu_data_header(outfile, current_dataset);
   fprintf(outfile, "$TRAIN\n");
   print_instance_list(outfile, *separator, current_dataset, data1);
   fprintf(outfile, "$TEST\n");
   print_instance_list(outfile, *separator, current_dataset, data2);
  }
 else
   print_instance_list(outfile, *separator, current_dataset, data1);
 fclose(outfile);
 safe_free(fname);
}

void export_datafolds(char* separator, int newseed)
{
 /*!Last Changed 21.04.2007 added export_single_file*/
 /*!Last Changed 30.10.2005*/
 /*!Last Changed 16.10.2005*/
 /*!Last Changed 26.04.2005 added runcount and mysrand*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 int i, k, **partition, *classes, *classcounts;
 mustnormalize = NO;
 get_prior_information();
 mysrand(newseed);
 classes = read_train_and_test_lines(&classcounts);
 partition = safemalloc(get_parameter_i("runcount") * sizeof(int *), "export_datafolds", 10);
 for (k = 0; k < get_parameter_i("runcount"); k++)
   partition[k] = stratified_partition(classes, trainlinecount, current_dataset->classno, classcounts);
 for (k = 0; k < get_parameter_i("runcount"); k++)
   for (i = 0; i < get_parameter_i("foldcount"); i++)
    {
     get_fold(i, partition[k]);
     pre_process();
     if (get_parameter_l("namestype") != NAMES_CMU)
      {
       export_single_file(separator, 0, k, i, traindata, NULL);
       export_single_file(separator, 1, k, i, testdata, NULL);
      }
     else
       export_single_file(separator, 0, k, i, traindata, testdata);
     post_process(YES);
     if (traindata)
      {
       deallocate(traindata);
       traindata = NULL;
      }
     if (testdata)
      {
       deallocate(testdata);
       testdata = NULL;
      }
    }
 free_2d((void**)partition, get_parameter_i("runcount"));
 free_train_and_test_lines();
 safe_free(classes);
 if (current_dataset->classno > 0)
   safe_free(classcounts);
}

void create_cvdata()
{
 /*!Last Changed 26.11.2005 removed bug safe_free(myarray) called for regression*/
 /*!Last Changed 28.10.2005 added find_classes and find_class_counts*/
 /*!Last Changed 23.11.2004 added traincount*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 14.01.2004*/
 int j, dat = data_size(traindata), *myarray = NULL, *partition, *classcounts = NULL;
 Instanceptr cvbefore = NULL, trainbefore = NULL, train = traindata; 
 if (cvdata)
  {
   deallocate(cvdata);
   cvdata = NULL;
  } 
 if (current_dataset->classno > 0)
  { 
   myarray = find_classes(traindata);
   classcounts = find_class_counts(traindata);
  }  
 partition = stratified_partition(myarray, dat, current_dataset->classno, classcounts);
 j = 0; 
 while (train)
  {
   if (partition[j] % 5 == 0)
    {
     if (cvbefore)
      {
       cvbefore->next = train;
       cvbefore = cvbefore->next;
      }
     else
      {
       cvdata = train;
       cvbefore = cvdata;
      }
    }
   else
     if (trainbefore)
      {
       trainbefore->next = train;
       trainbefore = trainbefore->next;
      }
     else
      {
       traindata = train;
       trainbefore = traindata;
      }
   train = train->next;
   j++;
  }
	if (trainbefore)
   trainbefore->next = NULL;
	if (cvbefore)
   cvbefore->next = NULL;
 current_dataset->traincount = data_size(traindata);
 if (current_dataset->classno > 0)
  {
   safe_free(classcounts);
   safe_free(myarray);
  }
 safe_free(partition);
}

void pre_process_with_mean_and_variance(Instance mean, Instance variance, Instanceptr data)
{
 /*!Last Changed 13.03.2007 added selected feature for realization*/
 /*!Last Changed 05.03.2006*/
 int i;
 if (mustrealize)
  {
   for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
     current_dataset->selected[i] = YES;
   realize(data);
   exchange_classdefno(current_dataset);
  }
 if (mustnormalize)
  {
   normalize(data, mean, variance);
   if (!mustrealize)
     type_change(current_dataset);
  }
}

void pre_process()
{
 /*!Last Changed 13.03.2007 added selected feature for realization*/
 /*!Last Changed 21.12.2005 added normalizeoutput*/
 /*!Last Changed 31.10.2005 added type_change*/
 /*!Last Changed 11.08.2005*/
 int i, datacount;
 if (get_parameter_o("normalizeoutput") && current_dataset->classno == 0)
  {
   outputmean = find_mean_of_regression_value(traindata);
   datacount = data_size(traindata);
   outputvariance = find_variance_of_regression_value(traindata, outputmean) / (datacount - 1);
   normalize_regression_values(traindata, outputmean, outputvariance);
   normalize_regression_values(testdata, outputmean, outputvariance);
   normalize_regression_values(separateddata, outputmean, outputvariance);
  }
 if (mustrealize)
  {
   realize(traindata);
   realize(testdata);
   realize(separateddata);
   exchange_classdefno(current_dataset);
   for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
     current_dataset->selected[i] = YES;
  }
 if (mustnormalize)
  {
   mean = find_mean(traindata);
   variance = find_variance(traindata);
   normalize(traindata, mean, variance);
   normalize(testdata, mean, variance);
   normalize(separateddata, mean, variance);
   if (!mustrealize)
     type_change(current_dataset);
  }
}

void post_process(int free_values)
{
 /*!Last Changed 31.10.2005 added type_restore*/
 /*!Last Changed 11.08.2005*/
 if (mustrealize)
   exchange_classdefno(current_dataset);
 if (mustnormalize)
  {
   if (free_values)
    {
     safe_free(mean.values);
     safe_free(variance.values);
    }
   if (!mustrealize)
     type_restore(current_dataset);
  }
}

void write_to_output_variables(int algorithm, void* model, Instanceptr* trdata, Instanceptr testdata)
{
 /*!Last Changed 04.08.2005*/
 int i, j, h, nodecount, rulecount, conditioncount, treecount;
 Ruleset r;
 double error;
 Decisionnodeptr* forest;
 Decisionnodeptr rootnode;
 Regressionnodeptr regrootnode;
 Regressionruleset regr;
 Mlpnetworkptr mlpnet;
 Model_divsptr mdivs;
 char nodetypes[READLINELENGTH];
 char datasizes[READLINELENGTH];
 switch (algorithm)
  {
   case C45           :rootnode = (Decisionnodeptr) model;
                       strcpy(nodetypes, ""); 
                       decisiontree_node_types(rootnode, nodetypes, 1);
                       if (strlen(nodetypes) == 0)
                         strcpy(nodetypes, " ");
                       error = decisiontree_falsepositive_count(rootnode) / (rootnode->covered + 0.0);
                       h = decisiontree_vc_dimension_recursive(current_dataset, rootnode);
                       write_array_variables("aout", totalrun, 3, "dddfs", decisiontree_node_count(rootnode), decisiontree_rule_count(rootnode), decisiontree_condition_count(rootnode), 100 * srm_classification(error, h, rootnode->covered, get_parameter_f("srma1"), get_parameter_f("srma2")), nodetypes);
                       break;
   case LDT           :
   case NBTREE        :
   case MULTILDT      :
   case CART          :rootnode = (Decisionnodeptr) model;
                       write_array_variables("aout", totalrun, 3, "ddd", decisiontree_node_count(rootnode), decisiontree_rule_count(rootnode), decisiontree_condition_count(rootnode));
                       break;
   case KTREE         :rootnode = (Decisionnodeptr) model;
                       strcpy(nodetypes, "");
                       decisiontree_node_types(rootnode, nodetypes, 1);
                       if (strlen(nodetypes) == 0)
                         strcpy(nodetypes, " ");
                       write_array_variables("aout", totalrun, 3, "ddds", decisiontree_node_count(rootnode), decisiontree_rule_count(rootnode), decisiontree_condition_count(rootnode), nodetypes);
                       break;
   case RANDOMFOREST  :
   case KFOREST       :forest = (Decisionnodeptr*) model;
                       treecount = get_parameter_i("forestsize");
                       nodecount = rulecount = conditioncount = 0;
                       for (i = 0; i < treecount; i++)
                        {
                         rootnode = forest[i];
                         nodecount += decisiontree_node_count(rootnode);
                         rulecount += decisiontree_rule_count(rootnode);
                         conditioncount += decisiontree_condition_count(rootnode);
                        }
                       write_array_variables("aout", totalrun, 3, "fff", nodecount, rulecount, conditioncount);
                       break;
   case SVMTREE       :forest = (Decisionnodeptr*) model;
                       if (get_parameter_l("multiclasstype") == ONE_VS_ONE)
                         treecount = current_dataset->classno * (current_dataset->classno - 1) / 2;
                       else
                         treecount = current_dataset->classno;
                       nodecount = rulecount = conditioncount = 0;
                       for (i = 0; i < treecount; i++)
                        {
                         rootnode = forest[i];
                         nodecount += decisiontree_node_count(rootnode);
                         rulecount += decisiontree_rule_count(rootnode);
                         conditioncount += decisiontree_condition_count(rootnode);
                        }
                       write_array_variables("aout", totalrun, 3, "fff", nodecount / (treecount + 0.0), rulecount / (treecount + 0.0), conditioncount / (treecount + 0.0));
                       break;
   case OMNILDT       :rootnode = (Decisionnodeptr) model;
                       strcpy(nodetypes, ""); 
                       decisiontree_node_types(rootnode, nodetypes, 1);
                       if (strlen(nodetypes) == 0)
                         strcpy(nodetypes, " ");
                       write_array_variables("aout", totalrun, 3, "dddddds",  decisiontree_univariate_node_count(rootnode), decisiontree_multivariate_linear_node_count(rootnode), decisiontree_multivariate_quadratic_node_count(rootnode), decisiontree_node_count(rootnode), decisiontree_rule_count(rootnode), decisiontree_condition_count(rootnode), nodetypes);
                       break;
   case DNC           :
   case DNCREG        :mlpnet = (Mlpnetworkptr) model;
                       write_array_variables("aout", totalrun, 3, "df", mlpnet->P.hidden[0], mlpnet->P.sqrderror);
                       break;
   case IREP          :
   case IREP2         :
   case RIPPER        :
   case C45RULES      :
   case MULTIRIPPER   :
   case CN2           :
   case LERILS        :
   case PART          :r = *((Ruleset*) model);
                       error = error_of_ruleset(r, *trdata) / (data_size(*trdata) + 0.0);
                       h = ruleset_vc_dimension(current_dataset, &r);
                       write_nodetypes_rule(r, nodetypes);
                       error = 100 * srm_classification(error, h, data_size(*trdata), get_parameter_f("srma1"), get_parameter_f("srma2"));
                       /*write_array_variables("aout", totalrun, 3, "ddsf", r.rulecount, condition_count(r), nodetypes, error);*/
                       break;
   case RISE          :r = *((Ruleset*) model);
                       write_array_variables("aout", totalrun, 3, "dd", r.rulecount, condition_count(r));
                       break;
   case DIVS          :mdivs = (Model_divsptr) model;
                       rulecount = 0;
                       conditioncount = 0;
                       for (i = 0; i < mdivs->datacount; i++)
                        {
                         rulecount += mdivs->rulecounts[i];
                         for (j = 0; j < mdivs->rulecounts[i]; j++)
                           conditioncount += mdivs->conditioncounts[i][j];
                        }
                       write_array_variables("aout", totalrun, 3, "dd", rulecount, conditioncount);
                       break;
   case HYBRIDRIPPER  :r = *((Ruleset*) model);
                       write_nodetypes_rule(r, nodetypes);
                       write_datasizes_and_nodetypes_rule(r, datasizes);
                       write_array_variables("aout", totalrun, 3, "ddss", r.rulecount, condition_count(r), nodetypes, datasizes);
                       break; 
   case REXRIPPER     :r = ((Model_rexripperptr) model)->r;
                       write_array_variables("aout", totalrun, 3, "ddddf", r.rulecount, condition_count(r),  data_size(((Model_rexripperptr) model)->exceptiondata), count_exceptions(testdata), exception_performance(testdata));
                       break;
   case REGTREE       :regrootnode = (Regressionnodeptr) model;
                       strcpy(nodetypes, ""); 
                       regressiontree_node_types(regrootnode, nodetypes, 1);
                       write_array_variables("aout", totalrun, 3, "ds", regressiontree_node_count(regrootnode), nodetypes);
                       break;
   case SOFTREGTREE   :regrootnode = (Regressionnodeptr) model;
                       strcpy(nodetypes, ""); 
                       soft_regtree_dimension_reduction(regrootnode, nodetypes, 1);
                       write_array_variables("aout", totalrun, 3, "ds", regressiontree_node_count(regrootnode), nodetypes);
                       break;
   case SOFTTREE      :rootnode = (Decisionnodeptr) model;
                       strcpy(nodetypes, ""); 
                       soft_tree_dimension_reduction(rootnode, nodetypes, 1);
                       write_array_variables("aout", totalrun, 3, "ds", decisiontree_node_count(rootnode), nodetypes);
                       break;
   case REGRULE       :regr = *((Regressionruleset*) model);
                       strcpy(nodetypes, ""); 
                       write_nodetypes_regression_rule(regr, nodetypes);
                       write_array_variables("aout", totalrun, 3, "s", nodetypes);
                       break;
  }
}

void** load_model(char* infile, int* modelcount, int* algorithm, Instanceptr* means, Instanceptr* variances)
{
 /*!Last Changed 05.08.2007 added QDACLASS algorithm*/
 /*!Last Changed 02.04.2007 added HMM algorithm*/
 /*!Last Changed 11.03.2006 added get_supervised_algorithm_index*/
 /*!Last Changed 19.12.2005*/
 FILE* modelfile;
 char datasetname[READLINELENGTH], algname[READLINELENGTH];
 void** result;
 int i, treecount;
 modelfile = fopen(infile, "r");
 if (!modelfile) 
   return NULL;
 fscanf(modelfile, "%s%s%d", datasetname, algname, modelcount);
 current_dataset = search_dataset(Datasets, datasetname);
 if (!current_dataset)
   return NULL;
 *algorithm = get_supervised_algorithm_index(algname);
 if (*algorithm == -1)
   return NULL;
 if (must_realize(*algorithm))
   exchange_classdefno(current_dataset);
 result = safemalloc((*modelcount) * sizeof(void *), "load_model", 15);
 if (must_normalize(*algorithm))
  {
   *means = safemalloc((*modelcount) * sizeof(Instance), "load_model", 18);
   *variances = safemalloc((*modelcount) * sizeof(Instance), "load_model", 19);
  }
 else
  {
   *means = NULL;
   *variances = NULL;
  }
 for (i = 0; i < (*modelcount); i++)
  {
   if (get_algorithm_type(*algorithm) == REGRESSION && get_parameter_o("normalizeoutput"))
     load_output_mean_and_variance(modelfile, &outputmean, &outputvariance);
   if (must_normalize(*algorithm))
    {
     if (must_realize(*algorithm))
      {
       (*means)[i] = load_instance(modelfile, current_dataset, BOOLEAN_FALSE);
       (*variances)[i] = load_instance(modelfile, current_dataset, BOOLEAN_FALSE);
      }
     else
      {
       (*means)[i] = load_instance_discrete(modelfile, current_dataset);
       (*variances)[i] = load_instance_discrete(modelfile, current_dataset);
      }
    }
   switch (*algorithm)
    {
     case SELECTMAX    :result[i] = load_model_selectmax(modelfile, current_dataset);
                        break;
     case ONEFEATURE   :result[i] = load_model_onefeature(modelfile, current_dataset);
                        break;
     case SELECTAVERAGE:result[i] = load_model_selectaverage(modelfile, current_dataset);
                        break;
     case GAUSSIAN     :result[i] = load_model_gaussian(modelfile, current_dataset); 
                        break;
     case NEARESTMEAN  :result[i] = load_model_nearestmean(modelfile, current_dataset); 
                        break;
     case LINEARREG    :result[i] = load_model_linearreg(modelfile, current_dataset); 
                        break;
     case QUANTIZEREG  :result[i] = load_model_quantizereg(modelfile, current_dataset); 
                        break;
     case KNN          :
     case KNNREG       :result[i] = load_model_knn(modelfile, current_dataset); 
                        break;
     case GPROCESSREG  :result[i] = load_model_knn(modelfile, current_dataset); 
                        break;
     case C45          :
     case LDT          :result[i] = safemalloc(sizeof(Decisionnode), "load_model", 30);
                        load_model_c45(modelfile, current_dataset, result[i]);
                        break;
     case SOFTTREE     :result[i] = safemalloc(sizeof(Decisionnode), "load_model", 32);
                        load_model_softtree(modelfile, current_dataset, result[i]);
                        break;
     case SVMTREE      :if (get_parameter_l("multiclasstype") == ONE_VS_ONE)
                          treecount = (current_dataset->classno * (current_dataset->classno - 1) / 2);
                        else
                          treecount = current_dataset->classno;
                        result[i] = safemalloc(treecount * sizeof(Decisionnodeptr), "load_model", 32);
                        load_model_svmtree(modelfile, current_dataset, result[i], treecount);
                        break;
     case RANDOMFOREST :treecount = get_parameter_i("forestsize");
                        load_model_randomforest(modelfile, current_dataset, result[i], treecount);
                        break;
     case SVMREGTREE   :
     case REGTREE      :
     case SOFTREGTREE  :result[i] = safemalloc(sizeof(Regressionnode), "load_model", 38);
                        load_model_regtree(modelfile, current_dataset, result[i]);
                        break;
     case REGRULE      :result[i] = load_model_regrule(modelfile, current_dataset);
                        break;
     case RISE         :result[i] = load_model_rise(modelfile, current_dataset);
                        break;
     case IREP         :
     case IREP2        :
     case C45RULES     :
     case RIPPER       :
     case MULTIRIPPER  :
     case REXRIPPER    :
     case CN2          :
     case LERILS       :
     case PART         :result[i] = load_model_ripper(modelfile, current_dataset);
                        break;
     case DIVS         :result[i] = load_model_divs(modelfile, current_dataset);
                        break;
     case NAIVEBAYES   :result[i] = load_model_naivebayes(modelfile, current_dataset);
                        break;
     case QDACLASS     :result[i] = load_model_qdaclass(modelfile, current_dataset);
                        break;
     case LDACLASS     :result[i] = load_model_ldaclass(modelfile, current_dataset);
                        break;
     case LOGISTIC     :result[i] = load_model_logistic(modelfile, current_dataset);
                        break;
     case RBF          :
     case RBFREG       :result[i] = load_model_rbf(modelfile, current_dataset);
                        break;
     case MLPGENERIC   :
     case MLPGENERICREG:
     case DNC          :
     case DNCREG       :result[i] = load_model_mlpgeneric(modelfile, current_dataset);
                        break;
     case MULTILDT     :
     case CART         :result[i] = load_model_multildt(modelfile, current_dataset);
                        break;
     case NBTREE       :result[i] = load_model_nodeboundedtree(modelfile, current_dataset);
                        break;
     case SVM          :
     case SVMREG       :result[i] = load_model_svm(modelfile, current_dataset);
                        break;
     case HMM          :result[i] = load_model_hmm(modelfile, current_dataset);                         
                        break;
     default           :safe_free(result);
                        return NULL;
    }
  }
 fclose(modelfile);
 return result;
}

void save_model(FILE* modelfile, int algorithm, void* model, void* parameters)
{
 /*!Last Changed 05.08.2007 added QDACLASS algorithm*/
 /*!Last Changed 02.04.2007 added HMM algorithm*/
 /*!Last Changed 19.12.2005*/
 Svmtree_parameterptr psvmtree;
 Kforest_parameterptr pkf;
 if (get_algorithm_type(algorithm) == REGRESSION && get_parameter_o("normalizeoutput"))
   save_output_mean_and_variance(modelfile, outputmean, outputvariance);
 if (must_normalize(algorithm))
   save_mean_and_variance(modelfile, current_dataset, mean, variance, must_realize(algorithm));
 switch (algorithm)
  {
   case SELECTMAX    :save_model_selectmax(modelfile, current_dataset, (Model_selectmaxptr) model);
                      break;
   case ONEFEATURE   :save_model_onefeature(modelfile, current_dataset, (Model_onefeatureptr) model);
                      break;
   case SELECTAVERAGE:save_model_selectaverage(modelfile, current_dataset, (double*) model);
                      break;
   case GAUSSIAN     :save_model_gaussian(modelfile, current_dataset, (Model_gaussianptr) model);
                      break;
   case NEARESTMEAN  :save_model_nearestmean(modelfile, current_dataset, (Instanceptr) model);
                      break;
   case LINEARREG    :save_model_linearreg(modelfile, current_dataset, (Model_linearregressionptr) model);
                      break;
   case QUANTIZEREG  :save_model_quantizereg(modelfile, current_dataset, (Model_quantizeregptr) model);
                      break;
   case KNN          :
   case KNNREG       :save_model_knn(modelfile, current_dataset, (Instanceptr) model);
                      break;
   case GPROCESSREG  :save_model_gprocessreg(modelfile, current_dataset, (Model_gprocessregressionptr) model);
                      break;
   case C45          :
   case LDT          :save_model_c45(modelfile, current_dataset, (Decisionnodeptr) model, 0);
                      break;
   case SOFTTREE     :save_model_softtree(modelfile, current_dataset, (Decisionnodeptr) model, 0);
                      break;
   case SVMTREE      :psvmtree = (Svmtree_parameterptr) parameters;
                      if (psvmtree->multiclasstype == ONE_VS_ONE)
                        save_model_svmtree(modelfile, current_dataset, (Decisionnodeptr*) model, current_dataset->classno * (current_dataset->classno - 1) / 2);
                      else
                        save_model_svmtree(modelfile, current_dataset, (Decisionnodeptr*) model, current_dataset->classno);
                      break;
   case RANDOMFOREST :pkf = (Kforest_parameterptr) parameters;
                      save_model_randomforest(modelfile, current_dataset, (Decisionnodeptr*) model, pkf->forestsize);
                      break;
   case SVMREGTREE   :
   case REGTREE      :
   case SOFTREGTREE  :save_model_regtree(modelfile, current_dataset, (Regressionnodeptr) model, 0);
                      break;
   case REGRULE      :save_model_regrule(modelfile, current_dataset, (Regressionrulesetptr) model);
                      break;
   case RISE         :save_model_rise(modelfile, current_dataset, (Model_riseptr) model);
                      break;
   case IREP         :
   case IREP2        :
   case RIPPER       :
   case C45RULES     :
   case MULTIRIPPER  :
   case REXRIPPER    :
   case CN2          :
   case LERILS       :
   case PART         :save_model_ripper(modelfile, current_dataset, (Rulesetptr) model);
                      break;
   case DIVS         :save_model_divs(modelfile, current_dataset, (Model_divsptr) model);
                      break;
   case NAIVEBAYES   :save_model_naivebayes(modelfile, current_dataset, (Model_naivebayesptr) model);
                      break;
   case QDACLASS     :save_model_qdaclass(modelfile, current_dataset, (Model_qdaclassptr) model);
                      break;
   case LDACLASS     :save_model_ldaclass(modelfile, current_dataset, (Model_ldaclassptr) model);
                      break;
   case LOGISTIC     :save_model_logistic(modelfile, current_dataset, (double**) model);
                      break;
   case RBF          :
   case RBFREG       :save_model_rbf(modelfile, current_dataset, (Rbfnetworkptr) model);
                      break;
   case MLPGENERIC   :
   case MLPGENERICREG:
   case DNC          :
   case DNCREG       :save_model_mlpgeneric(modelfile, current_dataset, (Mlpnetworkptr) model);
                      break;
   case MULTILDT     :
   case CART         :save_model_multildt(modelfile, current_dataset, (Decisionnodeptr) model, 0);
                      break;
   case NBTREE       :save_model_nodeboundedtree(modelfile, current_dataset, (Decisionnodeptr) model, 0);
                      break;
   case SVM          :
   case SVMREG       :save_model_svm(modelfile, current_dataset, (Svm_modelptr) model);
                      break;
   case HMM          :save_model_hmm(modelfile, current_dataset, (Hmmptr*) model);
                      break;
  }
}

void test_meta_algorithm(int meta_algorithm, int algorithm, void** model, void* parameters, int testsize)
{
 Instanceptr test, next;
 int testindex = 0, classno;
 double real;
 Prediction predicted;
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
   result = performance_initialize(testsize);
 else
   result = performance_initialize_regression(testsize);
 testmean = find_mean(testdata);
 testvariance = find_variance(testdata);
 test = testdata;
 while (test)
  {
   next = test->next;
   test->next = NULL;
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
    {
     predicted = meta_classification_algorithms[meta_algorithm - ONE_VERSUS_ONE].test_algorithm(test, model, algorithm, parameters, ((Classificationresultptr) result)->posteriors[testindex]);
     if (get_parameter_o("changedecisionthreshold") == ON && current_dataset->classno == 2)
      {
       if (((Classificationresultptr) result)->posteriors[testindex][0] > get_parameter_f("decisionthreshold"))
         predicted.classno = 0;
       else
         predicted.classno = 1;
      }
    }
   else
     predicted = meta_regression_algorithms[meta_algorithm - BAGGING_REGRESSION].test_algorithm(test, model, algorithm, parameters, NULL);
   test->next = next;
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
    {
     classno = give_classno(test);
     ((Classificationresultptr) result)->class_counts[classno]++;
     if (classno == predicted.classno)
      {
       ((Classificationresultptr) result)->class_performance[classno]++;
       ((Classificationresultptr) result)->performance++;
       test->classified = YES;
      }
     else
      {
       test->classified = NO;
      }
     ((Classificationresultptr) result)->confusion_matrix[classno][predicted.classno]++;
    }
   else
    {
     real = give_regressionvalue(test);
     ((Regressionresultptr) result)->absoluteerrors[testindex] = predicted.regvalue;
     ((Regressionresultptr) result)->regressionperformance += (real - predicted.regvalue) * (real - predicted.regvalue);    
    }
   testindex++;
   test = test->next;
  }
 safe_free(testmean.values);
 safe_free(testvariance.values);
}

void test_algorithm(int algorithm, void* model, void* parameters, int testsize)
{
 /*!Last Changed 01.08.2007 added confusion_matrix*/
 /*!Last Changed 29.12.2005*/
 Instanceptr test, next;
 int i, testindex = 0, classno;
 double real;
 Prediction predicted;
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
   result = performance_initialize(testsize);
 else
   result = performance_initialize_regression(testsize);
 testmean = find_mean(testdata);
 testvariance = find_variance(testdata);
 test = testdata;
 while (test)
  {
   next = test->next;
   test->next = NULL;
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
    {
     predicted = classification_algorithms[algorithm - SELECTMAX].test_algorithm(test, model, parameters, ((Classificationresultptr) result)->posteriors[testindex]);
     if (current_dataset->classno == 2 && in_list(algorithm, 2, SVM, SIMPLEXSVM))
       ((Classificationresultptr) result)->hingeloss += predicted.hingeloss;
     if (algorithm == KNN)
       for (i = 0; i < 3; i++)
         ((Classificationresultptr) result)->knnloss[i] += predicted.knnloss[i];
     if (get_parameter_o("changedecisionthreshold") == ON && current_dataset->classno == 2)
      {
       if (((Classificationresultptr) result)->posteriors[testindex][0] > get_parameter_f("decisionthreshold"))
         predicted.classno = 0;
       else
         predicted.classno = 1;
      }
    }
   else
    {
     predicted = regression_algorithms[algorithm - ONEFEATURE].test_algorithm(test, model, parameters, NULL);
     if (in_list(algorithm, 2, SVMREG, SIMPLEXSVMREG))
       ((Regressionresultptr) result)->epsilonloss += predicted.epsilonloss;
    }
   test->next = next;
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
    {
     classno = give_classno(test);
     ((Classificationresultptr) result)->class_counts[classno]++;
     if (classno == predicted.classno)
      {
       ((Classificationresultptr) result)->class_performance[classno]++;
       ((Classificationresultptr) result)->performance++;
       test->classified = YES;
      }
     else
      {
       test->classified = NO;
      }
     ((Classificationresultptr) result)->confusion_matrix[classno][predicted.classno]++;
    }
   else
    {
     real = give_regressionvalue(test);
     ((Regressionresultptr) result)->absoluteerrors[testindex] = predicted.regvalue;
     ((Regressionresultptr) result)->regressionperformance += (real - predicted.regvalue) * (real - predicted.regvalue);
    }
   testindex++;
   test = test->next;
  }
 safe_free(testmean.values);
 safe_free(testvariance.values);
}

void single_run_supervised_algorithm(void* parameters, int meta_algorithm, int algorithm, RUN_TYPE runtype, FILE* posfile, FILE* modelfile, FILE* testcodefile, FILE* confusionfile)
{
 /*!Last Changed 16.12.2005 added write_aout_variables*/
 /*!Last Changed 28.10.2005*/
 double tstart, tend, rocarea, prarea;
 void* model = NULL;
 void** meta_model;
 int testsize;
 matrix posteriors;
 BOOLEAN testtocv = BOOLEAN_FALSE;
 totalrun++;
 tstart = clock();
 pre_process();
 if (is_cvdata_needed(algorithm))
  {
   if (in_list(runtype, 3, CROSSVALIDATION, BOOTSTRAP, TRAIN_TEST_SETS_GIVEN))
     create_cvdata();
   else
     if (get_parameter_o("separatedataused") == OFF)
       create_cvdata();
     else
       if (runtype != TRAIN_TEST_CV_SETS_GIVEN)
        {
         cvdata = testdata;
         testtocv = BOOLEAN_TRUE;
        }
  }
 else
   cvdata = NULL;
 if (runtype == CROSSVALIDATION_WITH_SEPARATE_TEST_SET && get_parameter_o("separatedataused") == ON)
  {
   if (!testtocv)
     deallocate(testdata);
   testdata = separateddata;  
  }
 testsize = data_size(testdata);
 if (meta_algorithm == NO_META_ALGORITHM)
  {
   if (get_parameter_o("hyperparametertune"))
     model = tune_hyperparameters_of_the_supervised_algorithm(algorithm, &traindata, &cvdata, &cvdata, parameters);
   else
    {
     if (get_algorithm_type(algorithm) == CLASSIFICATION)
       model = classification_algorithms[algorithm - SELECTMAX].train_algorithm(&traindata, &cvdata, parameters);
     else
       model = regression_algorithms[algorithm - ONEFEATURE].train_algorithm(&traindata, &cvdata, parameters);
    }
   test_algorithm(algorithm, model, parameters, testsize);
  }
 else
  {
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
     meta_model = meta_classification_algorithms[meta_algorithm - ONE_VERSUS_ONE].train_algorithm(&traindata, &cvdata, algorithm, parameters);
   else
     meta_model = meta_regression_algorithms[meta_algorithm - BAGGING_REGRESSION].train_algorithm(&traindata, &cvdata, algorithm, parameters);
   test_meta_algorithm(meta_algorithm, algorithm, meta_model, parameters, testsize);
  }
 if (get_parameter_o("printposterior"))
  {
   fprintf(posfile, "####### CURRENT RUN COUNT %d\n", totalrun);
   print_posterior(posfile, ((Classificationresultptr) result)->posteriors, testsize, testdata);
  }
 if (get_parameter_o("printconfusion"))
  {
   if (((Classificationresultptr) result)->classno == 2)
     print_confusion_2d(confusionfile, ((Classificationresultptr) result)->confusion_matrix);
   else
     print_confusion(confusionfile, ((Classificationresultptr) result)->confusion_matrix, ((Classificationresultptr) result)->classno);
  }
 display_output(algorithm, model);
 if (meta_algorithm == NO_META_ALGORITHM)
   write_to_output_variables(algorithm, model, &traindata, testdata);
 tend = clock();
 if (get_parameter_o("showrundetails"))
   printf("Run %d took %.3f seconds\n", totalrun, (tend - tstart) / CLOCKS_PER_SEC);
 /*if (meta_algorithm == NO_META_ALGORITHM)*/
   /*write_array_variables("aout", totalrun, 1, "fd", (tend - tstart) / CLOCKS_PER_SEC, complexity_of_supervised_algorithm(algorithm, model, parameters, current_dataset));*/
 if (meta_algorithm == NO_META_ALGORITHM && current_dataset->classno == 2 && in_list(algorithm, 2, SVM, SIMPLEXSVM))
   write_array_variables("aout", totalrun, 3, "f", ((Classificationresultptr) result)->hingeloss / ((Classificationresultptr) result)->datasize);
 if (meta_algorithm == NO_META_ALGORITHM && in_list(algorithm, 2, SVMREG, SIMPLEXSVMREG))
   write_array_variables("aout", totalrun, 3, "f", ((Regressionresultptr) result)->epsilonloss / ((Regressionresultptr) result)->datasize);
 if (meta_algorithm == NO_META_ALGORITHM && algorithm == KNN)
   write_array_variables("aout", totalrun, 3, "fff", ((Classificationresultptr) result)->knnloss[0] / ((Classificationresultptr) result)->datasize, ((Classificationresultptr) result)->knnloss[1] / ((Classificationresultptr) result)->datasize, ((Classificationresultptr) result)->knnloss[2] / ((Classificationresultptr) result)->datasize);
 if (meta_algorithm == NO_META_ALGORITHM &&get_parameter_o("savemodel"))
   save_model(modelfile, algorithm, model, parameters);
 if (meta_algorithm == NO_META_ALGORITHM &&get_parameter_o("savetestcode"))
   generate_test_code(testcodefile, algorithm, model, parameters, get_parameter_l("language"), current_dataset);
 if (meta_algorithm == NO_META_ALGORITHM)
   free_model_of_supervised_algorithm(algorithm, model, current_dataset);
 else
   free_model_of_meta_classification_algorithm(meta_algorithm, algorithm, meta_model, current_dataset);
 post_process(YES);
 #ifdef MPI
 if (rank == 0)
  {
   if (get_algorithm_type(algorithm) == CLASSIFICATION)
    {
     print_classification_results(testdata, (Classificationresultptr) result, (tend - tstart) / CLOCKS_PER_SEC, get_parameter_o("printbinary"), get_parameter_o("displayruntime"), get_parameter_o("accuracy"), get_parameter_o("displayclassresulton"));
     free_performance((Classificationresultptr) result);
    }
   else
    {
     print_regression_results(testdata, (Regressionresultptr) result, (tend - tstart) / CLOCKS_PER_SEC, get_parameter_o("printbinary"), get_parameter_o("displayruntime"));
     free_performance_regression((Regressionresultptr) result);
    }
  }
 #else
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
  {
   print_classification_results(testdata, (Classificationresultptr) result, (tend - tstart) / CLOCKS_PER_SEC, get_parameter_o("printbinary"), get_parameter_o("displayruntime"), get_parameter_o("accuracy"), get_parameter_o("displayclassresulton"));
   posteriors = construct_posterior_matrix(((Classificationresultptr) result)->posteriors, testdata);
   rocarea = find_area_under_the_curve_kclass(posteriors, tpr_fpr_performance);
   prarea = find_area_under_the_curve_kclass(posteriors, precision_recall_performance);
   matrix_free(posteriors);
   /*write_array_variables("pout", totalrun, 1, "ff", rocarea, prarea);*/
   free_performance((Classificationresultptr) result);
  }
 else
  {
   print_regression_results(testdata, (Regressionresultptr) result, (tend - tstart) / CLOCKS_PER_SEC, get_parameter_o("printbinary"), get_parameter_o("displayruntime"));
   free_performance_regression((Regressionresultptr) result);
  }
 #endif 
 if (traindata)
  {
   deallocate(traindata);
   traindata = NULL;
  }
 if (testdata)
  {
   deallocate(testdata);
   testdata = NULL;
  }
 if (cvdata && runtype == CROSSVALIDATION_WITH_SEPARATE_TEST_SET)
  {
   deallocate(cvdata);
   cvdata = NULL;
  }
}

void read_data_from_lines_with_probability(char** lines, int linecount, double* probability, Instanceptr* data)
{
 /*!Last Changed 28.02.2006*/
 int i, bin;
 double val;
 Instanceptr currentinstance, before;
 for (i = 0; i < linecount; i++)
  {
   val = produce_random_value(0.0, 1.0);
   bin = find_bin(probability, linecount, val);
   currentinstance = create_instance(current_dataset, lines[bin], NO);
   if (i == 0)
     *data = currentinstance;
   else
     before->next = currentinstance;
   before = currentinstance;
  }
}

/**
 * Opens the dataset. For the flat file datasets this means opening the original dataset's file. For datasets stored in database tables this means connecting to the database and opening the table for reading.
 * @param[in] dataset Pointer to the dataset
 * @return SUCCESS if the connection is successfull (For datasets stored in database tables) or the file exists and successfully opened for reading (For the flat file datasets), FAILURE otherwise.
 */
int open_dataset(Datasetptr dataset)
{
 /*!Last Changed 07.04.2007 added STORE_TIME*/
 /*!Last Changed 29.12.2005*/
 char* datafilename;
 if (in_list(dataset->storetype, 3, STORE_TEXT, STORE_SEQUENTIAL, STORE_KERNEL))
  {
   datafilename = dataset_file_name(dataset);
   if ((datafile = fopen(datafilename, "r")) == NULL)
    {
     safe_free(datafilename);
     printf(m1271, dataset->name);
     return FAILURE;
    }
   else
    {
     safe_free(datafilename);
     return SUCCESS;
    }
  }
 else
   return connect_to_database(dataset);
}

/**
 * Closes the dataset. For the flat file datasets this means closing the original dataset's file. For datasets stored in database tables this means closing the connection from the database and closing the table.
 * @param[in] dataset Pointer to the dataset
 */
void close_dataset(Datasetptr dataset)
{
 /*!Last Changed 07.04.2007 added STORE_TIME*/
 /*!Last Changed 29.12.2005*/
 if (in_list(dataset->storetype, 3, STORE_TEXT, STORE_SEQUENTIAL, STORE_KERNEL))
   fclose(datafile);
 else
   disconnect_from_database();
}

void read_data_from_lines(char** lines, int linecount, Instanceptr* data)
{
 /*!Last Changed 30.10.2005*/
 int i;
 Instanceptr currentinstance, before;
 *data = create_instance(current_dataset, lines[0], get_parameter_o("runwithclassnoise"));
 before = *data;
 for (i = 1; i < linecount; i++)
  {
   currentinstance = create_instance(current_dataset, lines[i], get_parameter_o("runwithclassnoise"));
   before->next = currentinstance;
   before = currentinstance;
  }
}

void add_single_data_line(char*** lines, int* count, char* myline)
{
 /*!Last Changed 26.11.2005*/
 *lines = alloc(*lines, (*count + 1) * sizeof(char *), *count + 1);
 (*lines)[*count] = strcopy((*lines)[*count], myline);
 (*count)++;
}

void free_train_and_test_lines()
{
 /*!Last Changed 05.03.2006*/
 free_2d((void**)trainlines, trainlinecount);
 trainlinecount = 0;
 trainlines = NULL;
 if (get_parameter_f("testpercentage") > 0.0)
  {
   free_2d((void**)testlines, testlinecount);
   testlinecount = 0;
   testlines = NULL;
  }
}

void read_train_lines()
{
 /*!Last Changed 08.01.2009*/
 char myline[READLINELENGTH];
 if (!open_dataset(current_dataset))
   return;  
 while (get_single_instance(current_dataset, myline))
  {
   if (is_line_comment(myline))
     continue;
   add_single_data_line(&trainlines, &trainlinecount, myline);
  }
 close_dataset(current_dataset);
}

int* read_train_and_test_lines(int** classcounts)
{
 /*!Last Changed 26.11.2005*/
 /*!Last Changed 30.10.2005*/
 char myline[READLINELENGTH];
 int* partition, i = 0, classno, *classes = NULL;
 if (current_dataset->classno > 0)
   *classcounts = safecalloc(current_dataset->classno, sizeof(int), "read_train_and_test_lines", 5);
 if (!open_dataset(current_dataset))
   return NULL;  
 if (get_parameter_f("testpercentage") > 0)
   partition = stratified_partition(prior_class_information, current_dataset->datacount, current_dataset->classno, current_dataset->classcounts);
 while (get_single_instance(current_dataset, myline))
  {
   if (is_line_comment(myline))
     continue;
   if (current_dataset->classno > 0)
    {
     classno = prior_class_information[i];
     if (get_parameter_f("testpercentage") > 0 && partition[i] < get_parameter_f("testpercentage") * current_dataset->classcounts[classno])
       add_single_data_line(&testlines, &testlinecount, myline);
     else
      {
       (*classcounts)[classno]++;
       classes = alloc(classes, (trainlinecount + 1) * sizeof(int), trainlinecount + 1);
       classes[trainlinecount] = classno;
       add_single_data_line(&trainlines, &trainlinecount, myline);
      }
    }
   else
    {
     if (get_parameter_f("testpercentage") > 0 && partition[i] < get_parameter_f("testpercentage") * current_dataset->datacount)
       add_single_data_line(&testlines, &testlinecount, myline);
     else
       add_single_data_line(&trainlines, &trainlinecount, myline);
    }
   i++;
  }
 if (get_parameter_f("testpercentage") > 0)
   safe_free(partition);
 close_dataset(current_dataset);
 return classes;
}

void test_single(void* model, void* parameters, int algorithm, FILE* posfile, Instanceptr means, Instanceptr variances)
{
 /*!Last Changed 20.12.2005*/
 int testsize = data_size(testdata);
 Instance dummy;
 totalrun++;
 if (get_parameter_o("normalizeoutput") && get_algorithm_type(algorithm) == REGRESSION)
   normalize_regression_values(testdata, outputmean, outputvariance);
 if (must_normalize(algorithm))
   pre_process_with_mean_and_variance(means[totalrun - 1], variances[totalrun - 1], testdata);
 else
   pre_process_with_mean_and_variance(dummy, dummy, testdata);
 test_algorithm(algorithm, model, parameters, testsize); 
 if (get_parameter_o("printposterior"))
  {
   fprintf(posfile, "####### CURRENT RUN COUNT %d\n", totalrun);
   testsize = data_size(testdata);
   print_posterior(posfile, ((Classificationresultptr) result)->posteriors, testsize, testdata);
  }
 if (get_algorithm_type(algorithm) == CLASSIFICATION)
  {
   print_classification_results(testdata, (Classificationresultptr) result, 0, get_parameter_o("printbinary"), OFF, get_parameter_o("accuracy"), get_parameter_o("displayclassresulton"));
   free_performance((Classificationresultptr) result);
  }
 else
  {
   print_regression_results(testdata, (Regressionresultptr) result, 0, get_parameter_o("printbinary"), OFF);
   free_performance_regression((Regressionresultptr) result);
  }
 post_process(NO);
 if (traindata)
  {
   deallocate(traindata);
   traindata = NULL;
  }
 if (testdata)
  {
   deallocate(testdata);
   testdata = NULL;
  }
}

void test_model(char* modelfilename, char* testfilename)
{
 /*!Last Changed 20.12.2005*/
 void** model;
 void* parameters;
 FILE* posfile;
 Instanceptr means = NULL, variances = NULL;
 int i, k, modelcount, algorithm, runstart, **partition, *classes, *classcounts;
 totalrun = 0;
 model = load_model(modelfilename, &modelcount, &algorithm, &means, &variances);
 mustrealize = must_realize(algorithm);
 mustnormalize = must_normalize(algorithm);
 if (must_realize(algorithm))
   exchange_classdefno(current_dataset);
 if (!model)
   return;
 if (get_parameter_o("printposterior"))
  {
   posfile = fopen(get_parameter_s("posteriorfilename"), "w");
   if (!posfile)
     return;
  }
 parameters = prepare_supervised_algorithm_parameters(algorithm);
 if (isinteger(testfilename) && get_parameter_i("foldcount") > 1)
  {
   runstart = atoi(testfilename);
   get_prior_information();
   mysrand(runstart);
   partition = safemalloc(get_parameter_i("runcount") * sizeof(int *), "test_model", 12);
   classes = read_train_and_test_lines(&classcounts);
   if (get_parameter_f("testpercentage") > 0)
     read_data_from_lines(testlines, testlinecount, &separateddata);
   for (i = 0; i < get_parameter_i("runcount"); i++)
     partition[i] = stratified_partition(classes, trainlinecount, current_dataset->classno, classcounts);
   for (i = 0; i < get_parameter_i("runcount"); i++)
     for (k = 0; k < get_parameter_i("runtofold"); k++)
      {
       get_fold(k, partition[i]);
       test_single(model[totalrun], parameters, algorithm, posfile, means, variances);
      }
   free_2d((void**)partition, get_parameter_i("runcount"));
   if (current_dataset->classno > 0)
    {
     safe_free(classes);
     safe_free(classcounts);
    }
   if (get_parameter_f("testpercentage") > 0)
    {
     deallocate(separateddata);
     separateddata = NULL;
    }
   free_train_and_test_lines();
  }
 else
  {
   for (i = 0; i < modelcount; i++)
    {
     read_from_data_file(current_dataset, testfilename, &testdata); 
     test_single(model[i], parameters, algorithm, posfile, means, variances);
    }
  }
 for (i = 0; i < modelcount; i++)
  {
   free_model_of_supervised_algorithm(algorithm, model[i], current_dataset);
   if (must_normalize(algorithm))
    {
     safe_free(means[i].values);
     safe_free(variances[i].values);
    }
  }
 if (must_normalize(algorithm))
  {
   safe_free(means);
   safe_free(variances);
  }
 safe_free(model);
 free_supervised_algorithm_parameters(algorithm, parameters);
 if (get_parameter_o("printposterior"))
   fclose(posfile);
}

void autoencoder()
{
 int i, j, li;
 FILE* outfile;
 Mlpparameters parameters;
 Mlpnetwork neuralnetwork;
 matrix train, cv, test;
 matrixptr* hidden_matrices;
 double sum;
 Instanceptr tmpdata;
 mustrealize = YES;
 mustnormalize = YES;
 current_dataset = search_dataset(Datasets, params[0]);
 if (!current_dataset)
  {
   printf(m1320);
   return;
  }
 if (current_dataset->active == NO)
   return;
 mysrand(atoi(params[1]));
 parameters = prepare_Mlpparameters(NULL, NULL, get_parameter_o("weightdecay"), get_parameter_f("decayalpha"), get_parameter_i("hiddenlayers"), (int *) get_parameter_a("hiddennodes"), 0, 0.0, 0.0, get_parameter_i("maxhidden"), AUTOENCODER);
 read_from_train_file(current_dataset, &traindata);
 pre_process();
 create_cvdata();
 divide_instancelist(&traindata, &testdata, &tmpdata, 4);
 traindata = tmpdata;
 parameters.inputnum = data_size(traindata);
 parameters.trnum = parameters.inputnum;
 parameters.cvnum = data_size(cvdata);
 train = instancelist_to_matrix(traindata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 cv = instancelist_to_matrix(cvdata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 test = instancelist_to_matrix(testdata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 neuralnetwork = mlpn_general(train, cv, &parameters, AUTOENCODER);
 set_default_variable("out1", testMlpnNetwork_autoencoder(test, neuralnetwork) + 0.0);
 matrix_rowwise_merge(&train, cv);
 matrix_rowwise_merge(&train, test);
 matrix_free(cv);
 matrix_free(test);
 merge_instancelist(&traindata, cvdata);
 merge_instancelist(&traindata, testdata);
 outfile = fopen(params[2], "w");
 if (!outfile)
   return;
 print_class_information(outfile, traindata);
 fclose(outfile);
 outfile = fopen(params[3], "w");
 if (!outfile)
   return;
 hidden_matrices = safemalloc(train.row * sizeof(matrixptr), "autoencoder", 20);
 for (i = 0; i < train.row; i++)
  {
   hidden_matrices[i] = allocate_hiddens(neuralnetwork.P);
   for (li = 0; li < neuralnetwork.P.layernum; li++)
    {
     if (li == 0)
       calculate_hidden_values(train.values[i], neuralnetwork.weights[0], &(hidden_matrices[i][0]));
     else
       calculate_hidden_values(hidden_matrices[i][li - 1].values[0], neuralnetwork.weights[li], &(hidden_matrices[i][li]));
    }
  }
 for (i = 0; i < train.row; i++)
  {
   for (j = 0; j < train.row; j++)
    {
     sum = 0.0;
     for (li = 0; li < hidden_matrices[i][0].col; li++)
      {
       sum += hidden_matrices[i][0].values[0][li] * hidden_matrices[j][0].values[0][li];
      }
     if (j != train.row - 1)
       fprintf(outfile, "%.6f ", sum);
     else
       fprintf(outfile, "%.6f", sum);
    }
   fprintf(outfile, "\n");
  }
 fclose(outfile);
 for (i = 0; i < train.row; i++)
   free_hiddens(hidden_matrices[i], neuralnetwork.P.layernum);
 safe_free(hidden_matrices);
 deallocate(traindata);
 traindata = NULL;
 testdata = NULL;
 cvdata = NULL;
 matrix_free(train);
}

void run_dimension_reduction_algorithm(int algorithm)
{
 void* parameters;
 char *fname, *dname;
 char tmpcommand[MAXLENGTH];
 FILE* datafile;
 FILE* xmlfile;
 Datasetptr result;
 Instanceptr newdata;
 if (!checkparams(paramcount, 3, m1006, m1391, m1224))
   return; 
 mustrealize = dimension_reduction_algorithms[algorithm].mustrealize;
 mustnormalize = dimension_reduction_algorithms[algorithm].mustnormalize;
 current_dataset = search_dataset(Datasets, params[0]);
 if (!current_dataset)
  {
   printf(m1320);
   return;
  }
 if (current_dataset->active == NO)
   return;
 parameters = prepare_dimension_reduction_algorithm_parameters(algorithm);
 read_from_train_file(current_dataset, &traindata);
 pre_process(); 
 newdata = dimension_reduction_algorithms[algorithm].algorithm(current_dataset, traindata, parameters, &result, params[1]);
 if (!newdata)
   return;
 add_dataset(result);
 xmlfile = fopen(params[2], "w");
 fprintf(xmlfile, "<datasets>\n");
 save_definitions_of_dataset_as_xml(xmlfile, result);
 fprintf(xmlfile, "</datasets>\n");
 fclose(xmlfile);
 dname = dataset_directory_name(result);
 sprintf(tmpcommand, "mkdir %s", dname);
 safe_free(dname);
 system(tmpcommand);
 result->active = NO;
 post_process(NO);
 free_dimension_reduction_algorithm_parameters(algorithm, parameters);
 if (traindata)
  {
   deallocate(traindata);
   traindata = NULL;
  }
 fname = dataset_file_name(result);
 datafile = fopen(fname, "w");
 if (!datafile)
   return;
 current_dataset = result;
 mustrealize = NO;
 print_instance_list(datafile, ' ', result, newdata);
 fclose(datafile);
 safe_free(fname); 
 deallocate(newdata);
}

void run_supervised_algorithm(int meta_algorithm, int algorithm)
{
 /*!Last Changed 12.12.2005*/
 /*!Last Changed 31.10.2005*/
 /*!Last Changed 11.04.2005*/
 /*!Last Changed 10.04.2004 allow also regression algorithms to run*/
 /*!Last Changed 29.01.2004* if dataset is not active you can not run any algorithm on it*/
 int i, runstart, k, **partition, *classes, *classcounts;
 RUN_TYPE runtype;
 void* parameters;
 char* testfname;
 FILE* posfile = NULL;
 FILE* modelfile = NULL;
 FILE* testcodefile = NULL;
 FILE* confusionfile = NULL;
 if (!checkparams(paramcount, 3, m1006, m1048, m1049))
   return;
 totalrun = 0;
 current_dataset = search_dataset(Datasets, params[0]);
 if (!current_dataset)
  {
   printf(m1320);
   return;
  }
 mustrealize = must_realize(algorithm);
 if (current_dataset->storetype != STORE_KERNEL)
   mustnormalize = must_normalize(algorithm);
 else
  {
   read_kernel_of_dataset(current_dataset);
   mustnormalize = NO;
  }
 if (!check_supervised_algorithm_type(algorithm, current_dataset))  
   return;
 if (current_dataset->active == NO)
   return;
 if (strIcmp(params[1], "cv") == 0)
   if (get_parameter_f("testpercentage") > 0)
     runtype = CROSSVALIDATION_WITH_SEPARATE_TEST_SET;
   else
     runtype = CROSSVALIDATION;
 else
   if (strIcmp(params[1], "bootstrap") == 0)
     if (get_parameter_f("testpercentage") > 0)
       runtype = BOOTSTRAP;
     else
      {
       printf(m1414);
       return;
      }
   else
     if (paramcount == 4)
       runtype = TRAIN_TEST_SETS_GIVEN;
     else
       if (paramcount == 5)
         runtype = TRAIN_TEST_CV_SETS_GIVEN;
       else
        {
         printf(m1186);
         return;
        }
 parameters = prepare_supervised_algorithm_parameters(algorithm);
 if (get_parameter_o("printposterior"))
  {
   posfile = fopen(get_parameter_s("posteriorfilename"), "w");
   if (!posfile)
     return;
  }
 if (get_parameter_o("printconfusion"))
  {
   confusionfile = fopen(get_parameter_s("confusionfilename"), "w");
   if (!confusionfile)
     return;
  }
 if (get_parameter_o("savetestcode"))
  {
   testfname = test_code_file_name_with_extension(get_parameter_s("testcodefilename"), get_parameter_l("language"));
   testcodefile = fopen(testfname, "w");
   if (!testcodefile)
     return;
   safe_free(testfname);
  }
 if (get_parameter_o("savemodel"))
  {
   modelfile = fopen(get_parameter_s("modelfilename"), "w");
   if (!modelfile)
     return;
   switch (runtype){
     case                        CROSSVALIDATION:
     case CROSSVALIDATION_WITH_SEPARATE_TEST_SET:fprintf(modelfile, "%s %s %d\n", current_dataset->name, get_algorithm_name(algorithm), get_parameter_i("runcount") * get_parameter_i("runtofold"));
                                                 break;
     case                              BOOTSTRAP:fprintf(modelfile, "%s %s %d\n", current_dataset->name, get_algorithm_name(algorithm), get_parameter_i("runcount"));
                                                 break;
     case                  TRAIN_TEST_SETS_GIVEN:
     case               TRAIN_TEST_CV_SETS_GIVEN:fprintf(modelfile, "%s %s 1\n", current_dataset->name, get_algorithm_name(algorithm));
                                                 break;
   }
  }
 if (runtype == CROSSVALIDATION || runtype == CROSSVALIDATION_WITH_SEPARATE_TEST_SET)
  {
   runstart = atoi(params[2]);
   get_prior_information();
   mysrand(runstart);
   partition = safemalloc(get_parameter_i("runcount") * sizeof(int *), "run_supervised_algorithm", 34);
   classes = read_train_and_test_lines(&classcounts);
   for (i = 0; i < get_parameter_i("runcount"); i++)
     partition[i] = stratified_partition(classes, trainlinecount, current_dataset->classno, classcounts);
   for (i = 0; i < get_parameter_i("runcount"); i++)
     for (k = get_parameter_i("runfromfold"); k < get_parameter_i("runtofold"); k++)
      {
       #ifdef MPI
       if (get_parameter_o("parallel") && get_parameter_i("paralleltype") == 2 && get_parameter_i("proccount") > 1 && rank > 0)   
         get_fold_mpiclient(k, partition[i]);
       else
         get_fold(k, partition[i]);
       #else
       if (get_parameter_i("foldcount") != 1)
         get_fold(k, partition[i]);
       else
        {
         read_from_train_file(current_dataset, &traindata);
         read_from_train_file(current_dataset, &testdata);
        }
       #endif
       if (runtype == CROSSVALIDATION_WITH_SEPARATE_TEST_SET)
        {
         read_data_from_lines(testlines, testlinecount, &separateddata);
         handle_missing_values(traindata, separateddata);
        }
       single_run_supervised_algorithm(parameters, meta_algorithm, algorithm, runtype, posfile, modelfile, testcodefile, confusionfile);
       separateddata = NULL;
      }
   free_2d((void**)partition, get_parameter_i("runcount"));
   free_train_and_test_lines();
   if (current_dataset->classno > 0)
    {
     safe_free(classes);
     safe_free(classcounts);
    }
  }
 else
   if (runtype == BOOTSTRAP)
    {
     runstart = atoi(params[2]);
     get_prior_information();
     mysrand(runstart);
     classes = read_train_and_test_lines(&classcounts);
     for (i = 0; i < get_parameter_i("runcount"); i++)
      {
       bootstrap_sample_with_stratification(classes, classcounts);
       read_data_from_lines(testlines, testlinecount, &testdata);
       handle_missing_values(traindata, testdata);
       single_run_supervised_algorithm(parameters, meta_algorithm, algorithm, runtype, posfile, modelfile, testcodefile, confusionfile);
      }
     free_train_and_test_lines();
     if (current_dataset->classno > 0)
      {
       safe_free(classes);
       safe_free(classcounts);
      }
    }
   else
     if (runtype == TRAIN_TEST_SETS_GIVEN)
      {
       if (prepare_from_files(current_dataset, params[1], params[2], NULL))
        {
         mysrand(atoi(params[3]));
         single_run_supervised_algorithm(parameters, meta_algorithm, algorithm, runtype, posfile, modelfile, testcodefile, confusionfile);
        }
      }
     else
      {
       if (prepare_from_files(current_dataset, params[1], params[2], params[3]))
        {
         mysrand(atoi(params[4]));
         single_run_supervised_algorithm(parameters, meta_algorithm, algorithm, runtype, posfile, modelfile, testcodefile, confusionfile);
        }
      }
 free_supervised_algorithm_parameters(algorithm, parameters);
 if (get_parameter_o("printposterior"))
   fclose(posfile);
 if (get_parameter_o("savemodel"))
   fclose(modelfile);
 if (get_parameter_o("savetestcode"))
   fclose(testcodefile);
 if (get_parameter_o("printconfusion"))
   fclose(confusionfile);
}

void load_file(char *filename)
{
 /*!Last Changed 11.10.2006 added read_file_into_array*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 05.05.2002*/
 char myline[READLINELENGTH], templine[READLINELENGTH], *p, **lines = NULL;
 int res, i, howmany, linecount;
 push(ipstack, instruction_pointer);
 lines = read_file_into_array(filename, &linecount);
 instruction_pointer = 0;
 while (instruction_pointer < linecount)
  {
   strcpy(myline, lines[instruction_pointer]);
   if (get_parameter_o("displaycodeon"))
     printf(">>%s\n", myline);
   if (order)
    {
     safe_free(order);
     order = NULL;
    }
   for (i = 0; i < paramcount; i++)
    {
     safe_free(params[i]);
     params[i] = NULL;
    }
   change_according_to_variables(myline);
   p = strtok(myline," \t\n");
   if (p)
    {
     order = strcopy(order, p);
     paramcount = -1;
     do{
       paramcount++;
       p = strtok(NULL," \t\n");
       if (p)
         params[paramcount] = strcopy(params[paramcount], p);
     }while(p && (paramcount < MAXPARAMS - 1));
    }
   else
    {
     instruction_pointer++;
     continue;
    }
   params[paramcount] = NULL;
   for (i = 0; i < paramcount; i++)
     change_special_characters(params[i]);
   if (order[0]!='\'')
    {
     res = get_order_index(order);
     if (inswitch == NULL || res == 78 || res == 77 || correctcase == 1)
       process_order(res);
     if (goto_else_part)
      {
       goto_else_part = 0;
       howmany = 1;
       for (i = instruction_pointer + 1; i < linecount; i++)
        {
         strcpy(templine, lines[i]);
         p=strtok(templine, " \t\n");
         if (strcmp(p, "if") == 0)
           howmany++;
         else
           if (strcmp(p, "endif") == 0)
            {
             howmany--;
             if (howmany == 0)
               break;
            }
           else
             if (strcmp(p, "else") == 0 && howmany == 1)
               break;
        }
       instruction_pointer = i;
      }
     else
       if (goto_endif)
         goto_endconstructor(&goto_endif, "if", "endif", lines, linecount, &instruction_pointer);
       else 
         if (goto_endwhile)
           goto_endconstructor(&goto_endwhile, "while", "endwhile", lines, linecount, &instruction_pointer);
         else
           if (goto_endfor)
             goto_endconstructor(&goto_endfor, "for", "endfor", lines, linecount, &instruction_pointer);
           else
             if (goto_while)
              {
               goto_while = 0;
               howmany = 1;
               for (i = instruction_pointer - 1; i > -1; i--)
                {
                 strcpy(templine, lines[i]);
                 p = strtok(templine, " \t\n");
                 if (strcmp(p, "endwhile") == 0)
                   howmany++;
                 else
                   if (strcmp(p, "while") == 0)
                    {
                     howmany--;
                     if (howmany == 0)
                       break;
                    }
                }
               instruction_pointer = i - 1;
              }
    }
   instruction_pointer++;
  }
 if (order)
  {
   safe_free(order);
   order = NULL;
  }
 for (i = 0; i < paramcount; i++)
  { 
   safe_free(params[i]);
   params[i] = NULL;
  }
 if (lines != NULL)
   free_2d((void**)lines, linecount);
 instruction_pointer = pop(ipstack);
}

void check_and_set_list_variable(void* variable, char* errormessage1, char* errormessage2, char* valuelist[], int listlength)
{
 /*!Last Changed 22.11.2004*/
 int tmp, i;
 if (checkparams(paramcount, 1, errormessage1))
  {
   tmp = listindex(params[0], valuelist, listlength);
   if (tmp != -1)
     *((int*)variable) = tmp;
   else
    {
     printf(errormessage2, params[0]);
     printf(m1276);
     for (i = 0; i < listlength; i++)
      {
       printf("%s", valuelist[i]);
       printf("\n");
      }
     printf("Current value: %s\n", valuelist[*((int*)variable)]);
    }
  }
}

void deallocate_remaining_memory()
{
 /*!Last Changed 29.11.2004*/
 int res;
 for (res = 0; res < graphcount; res++)
   deallocate_graph(Graphs[res]);
 deallocate_datasets();
 free_variables(vars, varcount);
 safe_free(vars);
 if (datadir)
   safe_free(datadir);
 if (prior_class_information)
   safe_free(prior_class_information); 
 if (traindata)
  {
   deallocate(traindata);
   traindata = NULL;
  }
 if (testdata)
  {
   deallocate(testdata);
   testdata = NULL;
  } 
 if (cvdata)
  {
   deallocate(cvdata);
   cvdata = NULL;
  }
 deallocate_orders();
}

void test_code()
{
}

void os_command(OPERATING_SYSTEM_COMMAND_TYPE cmd_index)
{
 /*!Last Changed 19.04.2007*/
 int i;
 char st[MAXLENGTH];
 switch (get_parameter_l("currentos"))
  {
   case WINDOWS:strcpy(st, windows_commands[cmd_index]);
                break;
   case SOLARIS:
   case   LINUX:strcpy(st, linux_commands[cmd_index]);
                break;
  }
 for (i = 0; i < paramcount; i++)
   sprintf(st, "%s %s", st, params[i]);
 system(st);
}

void process_order(int res)
{
 Limits* mylimits;
 int tmp, tmp2 = 0, loop, *tmplist, i, tmp3 = 0, **tmpdi, j, tmp4 = 0, tmp5, modelcount, algorithm, learnercount, foldcount;
 char* tmpst, varname[MAXLENGTH], varvalue[MAXLENGTH], tmpst2[MAXLENGTH] = "-", tmpst3[MAXLENGTH], tmpst4[MAXLENGTH], pst[READLINELENGTH], *strp;
 double tmpd, tmpd2, tmpd3, tmpd4, **data, *w, *eigenvalues;
 double tmpf;
 matrix eigenvectors, mat, cov;
 matrixptr matptr;
 Intervalptr interval;
 double tstart, tend;
 bayesiangraphptr mygraph;
 Datasetptr d, tmpdataset;
 variableptr myvar, myvar2;
 Polynomptr p;
 Multiplemodelptr m;
 Decisionnodeptr tree;
 Regressionnodeptr regtree;
 Gaussianmixtureptr gm;
 Instanceptr means = NULL, variances = NULL;
 Binaryfunctionptr bf;
 Confusion_matrixptr cm;
 Linearprogramptr lpprogram;
 Quadraticprogramptr qpprogram;
 Hmm_model hmmmodel;
 Convexsolutionptr solution;
 Instance dummy;
 matrixptr* posteriors;
 Classificationresultptr results;
 void** models;
 void* parameters;
 FILE* xmlfile;
 FILE* outfile;
 Point p1 = {0, 0}, p2 = {0, 0}, p3 = {0, 0};
 if (get_parameter_o("timeon"))
   tstart = clock();
 switch (res){
  case  -2:break;
  case  -1:printf(m1277, order);
           break;
  case   0:printf(m1001);
           exit(1);
           break;
  case   1:if (!params[0])
             printf(m1002);
           else
             datadir = strcopy(datadir, params[0]);
           break;
  case   2:printf(m1278, datadir);
           break;
  case   3:if (checkparams(paramcount, 1, m1004))
            {
             if (datadir)
              {
               if (!fileexists(params[0]))
                 printf(m1264);
               else
                {
                 Datasets = load_definitions(params[0], &datasetcount);
                 if (Datasets)
                   set_parameter_s("inifile", params[0]);
                }
              }
             else
               printf(m1003);
            }
           break;
  case   4:if (checkparams(paramcount, 1, m1005))
             set_parameter_l("currentos", params[0], m1169);
           break;
  case   5:if (checkparams(paramcount, 1, m1006))
             display_dataset_properties(params[0]);
           break;
  case   6:if (checkparams(paramcount, 1, m1007))
             set_parameter_f("testpercentage", params[0], 0.0, 1.0);
           break;
  case   7:if (checkparams(paramcount, 1, m1007))
             set_parameter_f("cvratio", params[0], 0.0, 1.0);
           break;
  case   8:if (checkparams(paramcount, 4, m1006, m1108, m1049, m1043))
            {
             if (get_parameter_f("testpercentage") > 0)
              {
               current_dataset = search_dataset(Datasets, params[0]);
               if (current_dataset)
                {
                 if (strIcmp(params[3], "off") == 0)
                   mustrealize = NO;
                 else
                   mustrealize = YES;
                 mustnormalize = NO;
                 if (!atoi_check(params[2], &tmp, -INT_MAX, +INT_MAX))
                   break;                        
                 export_valdata(params[1], tmp);
                }
               else
                 printf(m1279, params[0]);
              }
             else
               printf(m1217);
            }
           break;
  case   9:run_supervised_algorithm(NO_META_ALGORITHM, SELECTMAX);
           break;
  case  10:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("separatedataused", params[0]);
           break;
  case  11:if (checkparams(paramcount, 1, m1082))
             performance_plot(params, paramcount, precision_recall_performance);
           break;
  case  12:if (checkparams(paramcount, 2, m1218, m1219))
            {
             tmp = listindex(params[0], exportfiletypes, MAX_EXPORT_TYPE);
             if (tmp < 0)
               printf(m1218);
             else
               if (in_list(tmp, 2, 0, 1) && (!strchr(params[1], 'R') || !strchr(params[1], 'F') || !strchr(params[1], 'D')))
                 printf(m1220);
               else
                 if (in_list(tmp, 2, 2, 3) && !strchr(params[1], 'D'))
                   printf(m1221);
                 else
                   if (in_list(tmp, 1, 4) && (!strchr(params[1], 'D') || !strchr(params[1], 'P')))
                     printf(m1342);
                   else
                     strcpy(exportfiletemplates[tmp], params[1]);
            }
           break;
  case  13:if (checkparams(paramcount, 1, m1011))
             if (compile_file(params[0]))
              {
               for (i = 1; i < paramcount; i++)
                 write_array_variables("aout", i, 1, "s", params[i]);
               load_file(params[0]);
              }
           break;
  case  14:if (checkparams(paramcount, 1, m1012))
             set_parameter_f("errorthreshold", params[0], 0.0, 1.0);
           break;
  case  15:if (checkparams(paramcount, 1, m1013))
             set_parameter_f("learning_rate", params[0], 0.0, +DBL_MAX);
           break;
  case  16:if (checkparams(paramcount, 1, m1014))
             set_parameter_i("perceptronrun", params[0], 1, INT_MAX);
           break;
  case  17:if (checkparams(paramcount, 1, m1015))
             set_parameter_f("variance_explained", params[0], 0.0, 1.0);
           break;
  case  18:if (checkparams(paramcount, 1, m1016))
             set_parameter_i("reducedfeaturecount", params[0], 1, INT_MAX);
           break;
  case  19:if (checkparams(paramcount, 1, m1154))
             set_parameter_l("dimensionreducetype", params[0], m1390);
           break;
  case  20:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("printposterior", params[0]);
           break;
  case  21:if (checkparams(paramcount, 1, m1017))
             load_network(params[0]);
           break;
  case  22:if (checkparams(paramcount, 1, m1020))
            {
             mygraph = find_network(params[0]);
             if (mygraph)
              {
               if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
                 break;                        
               if (checkparams(paramcount, 3, m1020, m1019, m1018))
                 create_data(mygraph, tmp, params[2]);
              }
             else
              printf(m1280, params[0]);
            }
           break;
  case  23:if (checkparams(paramcount, 1, m1020))
            { 
             mygraph = find_network(params[0]);
             if (!mygraph)
              {
               if (checkparams(paramcount, 4, m1020, m1023, m1022, m1021))
                 learn_network(params[0], params[1], params[2], params[3], 0);
              }
             else
               printf(m1281, params[0]);
            }
           break;
  case  24:if (checkparams(paramcount, 1, m1020))
            {
             mygraph = find_network(params[0]);
             if (mygraph)
               graph_info(mygraph);
             else
               printf(m1273, params[0]);
            }
           break;
  case  25:if (checkparams(paramcount, 1, m1020))
            {
             mygraph = find_network(params[0]);
             if (mygraph)
               if (params[1])
                 node_info(mygraph, params[1]);
               else
                 printf(m1024);
             else
               printf(m1273, params[0]);
            }
           break;
  case  26:if (checkparams(paramcount, 1, m1130))
             set_parameter_f("alpha", params[0], 0.0, +DBL_MAX);
           break;
  case  27:if (checkparams(paramcount, 1, m1025))
             set_parameter_f("infthreshold", params[0], 0.0, 1.0);
           break;
  case  28:if (checkparams(paramcount, 1, m1020))
            {
             mygraph = find_network(params[0]);
             if (mygraph)
               if (params[1])
                {
                 tmp = net_extension(params[1]);
                 switch (tmp)
                  {
                   case NET_FILE  :save_graph_net(mygraph, params[1]);
                                   break;
                   case XML_FILE  :save_graph_xml(mygraph, params[1]);
                                   break;
                   case HUGIN_FILE:save_graph_hugin(mygraph, params[1]);
                                   break;
                   case BIF_FILE  :save_graph_bif(mygraph, params[1]);
                                   break;
                   default        :printf(m1026);
                                   break;
                  }
                }
               else
                 printf(m1027);
             else
               printf(m1273, params[0]);
            }
           break;
  case  29:if (checkparams(paramcount, 1, m1029))
             set_parameter_l("ordering", params[0], m1028);
           break;
  case  30:if (checkparams(paramcount, 4, m1032, m1031, m1030, m1454))
            {
             if (!atof_check(params[1], &tmpd, 0.0, 1.0))
               break;                        
             if (!atoi_check(params[2], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             if (!atoi_check(params[3], &tmp2, 10, +INT_MAX))
               break;                        
             produce_distribution(tmpd, tmp, tmp2, params[0]);
            }
           break;
  case  31:multiple_tests(anova, params, paramcount);
           break;
  case  32:binary_comparison(bonferroni, params, paramcount);
           break;
  case  33:binary_comparison(scheffe, params, paramcount);
           break;
  case  34:binary_comparison(tukey, params, paramcount);
           break;
  case  35:print_independences(print_zero_independence, params[0]);
           break;
  case  36:print_independences(print_one_independence, params[0]);
           break;
  case  37:for (tmp=0;tmp<graphcount;tmp++)
             printf("GRAPH %d: %s\n", tmp + 1, Graphs[tmp]->name);
           break;
  case  38:if (checkparams(paramcount, 1, m1037))
            {
             if (output != stdout)
               fclose(output);
             if ((output = fopen(params[0], "w")) == NULL)
              {
               printf(m1248, params[0]);
               output = stdout;
              }
            }
           break;
  case  39:binary_comparison(ftest5x2, params, paramcount);
           break;
  case  40:if (checkparams(paramcount, 2, m1035, m1035))
            {
             if (!(in_list(get_parameter_l("testtype"), 2, NEWMANKEULS, DUNCAN)))
              {
               tmplist = multiple_comparison(params,paramcount, get_parameter_l("testtype"), tmpst2, &tmp2, &tmp3, get_parameter_l("correctiontype"));
               if (get_parameter_o("texoutput"))
                {
                 tmpst = (char*) safemalloc((39 + 4 * paramcount)*sizeof(char), "main", 210);
                 strcpy(tmpst,"& MultiTest    & \\multicolumn{5}{|c|}{$");
                 for (i = 0; i < paramcount-1; i++)
                  {
                   sprintf(tmpst3,"%c > ",(tmplist[i] - 0) + '0');
                   strcat(tmpst,tmpst3);
                  }
                 sprintf(tmpst3,"%c",(tmplist[i] - 0) + '0');
                 strcat(tmpst,tmpst3);
                 strcat(tmpst,"$}");
                }
               else
                {
                 tmpst = (char*) safemalloc((paramcount+1) * sizeof(char), "main", 223);
                 for (i = 0; i < paramcount; i++)
                   tmpst[i] = tmplist[i] + '0';
                 tmpst[paramcount] = '\0';
                }
               set_default_string_variable("sout1",tmpst);
               set_default_string_variable("sout2",tmpst2);
               safe_free(tmpst);
               tmp = tmplist[0];
               safe_free(tmplist);
              }
             else
              {
               tmpst = range_tests(params, paramcount, get_parameter_l("testtype"), &tmp, &tmp4, tmpst2);
               set_default_string_variable("sout1",tmpst);
               set_default_string_variable("sout2",tmpst2);
              }
             set_default_variable("out1",tmp + 0.0);
             set_default_variable("out2",tmp2 + 0.0);
             set_default_variable("out3",tmp3 + 0.0);
             set_default_variable("out4",tmp4 + 0.0);
             if (get_parameter_o("displayresulton"))
               printf(m1282, params[tmp-1]);
            }
           break;
  case  41:if (checkparams(paramcount, 1, m1039))
             set_parameter_l("testtype", params[0], m1038);
           break;
  case  42:if (checkparams(paramcount,1,m1042))
            {
             tmp=generate_tests(params,paramcount);
             if (tmp>0)
               printf(m1040);
             else
               if (tmp==0)
                 printf(m1041);
               else
                 printf(m1283, params[0]);
            }
           break;
  case  43:if (checkparams(paramcount,1,m1020))
            {
             mygraph=find_network(params[0]);
             if (!mygraph)
              {
               if (checkparams(paramcount,4,m1020,m1023,m1022,m1021))
                 learn_network(params[0],params[1],params[2],params[3],1);
              }
             else
               printf(m1281, params[0]);
            }
           break;
  case  44:run_supervised_algorithm(NO_META_ALGORITHM, KNN);
           break;
  case  45:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("timeon", params[0]);
           break;
  case  46:run_supervised_algorithm(NO_META_ALGORITHM, C45);
           break;
  case  47:run_supervised_algorithm(NO_META_ALGORITHM, ONEFEATURE);
           break;
  case  48:if (checkparams(paramcount,1,m1055))
            {
             res = get_order_index(params[0]);
             if (res == -1)
               printf(m1057);
             else
               print_order(res);
            }
           else
             print_command_tree(commandtree);
           break;
  case  49:if (checkparams(paramcount,1,m1171))
            {
             tmp = listindex(params[0], problem_type, MAX_PROBLEM_TYPE);
             if (tmp == -1)
               printf(m1171);
             else
              {
               print_datasets(Datasets, tmp);
               printf("\n");
              }
            }
           break;
  case  50:if (checkparams(paramcount,1,m1058))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;
             tmpd2 = normal(tmpd,0);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  51:if (checkparams(paramcount,1,m1059))
            {
             if (!atof_check(params[0], &tmpd, 0.0, 1.0))
               break;
             tmpd2 = z_inverse(tmpd);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  52:binary_comparison(rank_sum_test, params, paramcount);
           break;
  case  53:multiple_tests(kruskalwallis, params, paramcount);
           break;
  case  54:if (checkparams(paramcount,1,m1060))
             display_file(params[0]);
           break;
  case  55:if (checkparams(paramcount, 3, m1058, m1061, m1062))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;            
             if (!atof_check(params[1], &tmpd2, 0.0, +DBL_MAX))
               break;            
             if (!atof_check(params[2], &tmpd3, 0.0, +DBL_MAX))
               break;            
             tmpd4 = studentized_range(tmpd, tmpd2, tmpd3);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd4);
              }
             set_default_variable("out1",tmpd4);
            }
           break;
  case  56:if (checkparams(paramcount,3,m1059,m1061,m1062))
            {
             if (!atof_check(params[0], &tmpd, 0.0, 1.0))
               break;                        
             if (!atof_check(params[1], &tmpd2, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atof_check(params[2], &tmpd3, -DBL_MAX, +DBL_MAX))
               break;                        
             tmpd4 = studentized_range_inverse(tmpd, tmpd2, tmpd3);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd4);
              }
             set_default_variable("out1", tmpd4);
            }
           break;
  case  57:if (checkparams(paramcount, 2, m1058, m1063))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             tmpd2 = chi_square(tmp,tmpd);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  58:if (checkparams(paramcount, 2, m1059, m1063))
            {
             if (!atof_check(params[0], &tmpd, 0.0, 1.0))
               break;                        
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             tmpd2 = chi_square_inverse(tmpd,tmp);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  59:if (checkparams(paramcount,3,m1058,m1064,m1065))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp2, 1, +INT_MAX))
               break;                        
             tmpd2 = f_distribution(tmpd, tmp, tmp2);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  60:if (checkparams(paramcount,3,m1059,m1064,m1065))
            {
             if (!atof_check(params[0], &tmpd, 0.0, 1.0))
               break;                        
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp2, 1, +INT_MAX))
               break;                        
             tmpd2 = f_distribution_inverse(tmpd, tmp, tmp2);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  61:if (checkparams(paramcount,2,m1058,m1066))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             tmpd2 = t_distribution(tmpd, tmp);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  62:if (checkparams(paramcount,2,m1059,m1066))
            {
             if (!atof_check(params[0], &tmpd, 0.0, 1.0))
               break;                        
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             tmpd2 = t_distribution_inverse(tmpd, tmp);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd2);
              }
             set_default_variable("out1",tmpd2);
            }
           break;
  case  63:binary_comparison(ttest, params, paramcount);
           break;
  case  64:multiple_tests(vanderwaerdan, params, paramcount);
           break;
  case  65:if (params[0])
             editfile(params[0]);
           else
             printf(m1067);
           break;
  case  66:run_supervised_algorithm(NO_META_ALGORITHM, NEARESTMEAN);
           break;
  case  67:run_supervised_algorithm(NO_META_ALGORITHM, LDACLASS);
           break;
  case  68:check_and_set_list_variable(&(img.imagelegend.position), m1345, m1346, legend_positions, POSITION_COUNT);
           export_image();
           break;
  case  69:if (checkparams(paramcount, 1, m1070))
            {
             myvar = get_variable(params[1]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               matptr = safemalloc(sizeof(matrix), "main", 105);
               *matptr = matrix_copy(*(myvar->value.matrixvalue));
               set_variable(params[0], matptr);
              }
             else
               set_variable(params[0], params[1]);
            }
           break;
  case  70:if (checkparams(paramcount, 1, m1071))
             add_variable(INTEGER_TYPE, params, paramcount, &vars, &varcount);
           break;
  case  71:if (checkparams(paramcount, 1, m1071))
             add_variable(REAL_TYPE, params, paramcount, &vars, &varcount);
           break;
  case  72:if (checkparams(paramcount, 1, m1071))
             add_variable(STRING_TYPE, params, paramcount, &vars, &varcount);
           break;
  case  73:print_variables();
           break;
  case  74:if (checkparams(paramcount, 3, m1072, m1073, m1074))
            {
             set_variable(params[0], params[1]);
             if (params[3])
              {
               if (!atoi_check(params[3], &tmp, -INT_MAX, +INT_MAX))
                 tmp = 1;                        
              }
             else
               tmp = 1;
             set_for_variable(params[0], params[2], tmp, instruction_pointer);
             tmp = return_variable_index(params[0], vars, varcount);
             if (vars[tmp].value.intvalue > vars[tmp].end)
               goto_endfor = 1;
            }
           break;
  case  75:if (checkparams(paramcount,1,m1071))
            {
             tmp = return_variable_index(params[0], vars, varcount);
             if (tmp != -1)
              {
               vars[tmp].value.intvalue = vars[tmp].value.intvalue + vars[tmp].step;
               if (vars[tmp].value.intvalue <= vars[tmp].end)
                 instruction_pointer = vars[tmp].ip;
              }
             else
               printf(m1075);
            }
           break;
  case  76:if (checkparams(paramcount, 1, m1076))
            {
             inswitch = strcopy(inswitch, params[0]);
             correctcase = 1;
            }
           break;
  case  77:if (inswitch)
             safe_free(inswitch);
           inswitch = NULL;
           break;
  case  78:if (checkparams(paramcount, 1, m1077))
            {
             if (check_variable(inswitch, params[0]))
               correctcase = 1;
             else
               correctcase = 0;
            }
           break;
  case  79:
  case  80:if (checkparams(paramcount, 1, m1071))
            {
             if (make_operation('+', params[0], "1"))
               ;
             else
               printf(m1075);
            }
           break;
  case  81:
  case  82:if (checkparams(paramcount, 1, m1071))
            {
             if (make_operation('+', params[0], "-1"))
               ;
             else
               printf(m1075);
            }
           break;
  case  83:
  case  84:
  case  85:
  case  86:
  case  87:
  case  88:
  case  89:if (checkparams(paramcount, 1, m1071))
             make_operation(order[0], params[0], params[1]);
           break;
  case  90:if (checkparams(paramcount, 1, m1078))
             fprintf(output, "%s\n", params[0]);
           break;
  case  91:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("displaycodeon", params[0]);
           break;
  case  92:if (checkparams(paramcount, 1, m1032))
            {
             tmpd = file_mean(params, paramcount);
             set_default_variable("out1", tmpd);
            }
           break;
  case  93:if (checkparams(paramcount, 1, m1032))
            {
             tmpd = file_variance(params, paramcount);
             set_default_variable("out1", tmpd);
            }
           break;
  case  94:if (checkparams(paramcount, 1, m1032))
            {
             tmpd = file_mean(params, paramcount);
             set_default_variable("out1", tmpd);
             tmpd = file_variance(params, paramcount);
             set_default_variable("out2", tmpd);
            }
           break;
  case  95:if (checkparams(paramcount, 2, m1079, m1079))
             concat_file(params[0], params[1]);
           break;
  case  96:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("displayresulton", params[0]);
           break;
  case  97:if (!check_if_statement(params))
             goto_else_part = 1;
           break;
  case  98:goto_endif = 1;
           break;
  case  99:break;
  case 100:if (!check_if_statement(params))
             goto_endwhile = 1;
           break;
  case 101:goto_while = 1;
           break;
  case 102:if (checkparams(paramcount, 2, m1035, m1035))
            {
             tmpst=range_tests(params, paramcount, NEWMANKEULS, &tmp, &tmp2, tmpst2);
             set_default_variable("out1", tmp + 0.0);
             set_default_variable("out2", tmp2 + 0.0);
             set_default_string_variable("sout1", tmpst);
             set_default_string_variable("sout2", tmpst2);
             safe_free(tmpst);
            }
           break;
  case 103:if (checkparams(paramcount, 2, m1035, m1035))
            {
             tmpst=range_tests(params, paramcount, DUNCAN, &tmp, &tmp2, tmpst2);
             set_default_variable("out1", tmp + 0.0);
             set_default_variable("out2", tmp2 + 0.0);
             set_default_string_variable("sout1", tmpst);
             set_default_string_variable("sout2", tmpst2);
             safe_free(tmpst);
            }
           break;
  case 104:binary_comparison(ttestpaired, params, paramcount);
           break;
  case 105:set_integer_parameter("tailed", 2);
           break;
  case 106:set_integer_parameter("tailed", 1);
           break;
  case 107:binary_comparison(ttest5x2, params, paramcount);
           break;
  case 108:if (checkparams(paramcount, 2, m1006, m1081))
             binary_tests_compare(params, paramcount, 0, output);
           break;
  case 109:if (checkparams(paramcount, 1, m1082))
             plotx(params[0]);
           break;
  case 110:if (checkparams(paramcount, 1, m1082))
             plotxy(params[0]);
           break;
  case 111:if (checkparams(paramcount, 1, m1083))
            {
             if (img.xaxis.label)
               safe_free(img.xaxis.label);
             img.xaxis.label = strcopy(img.xaxis.label, params[0]);
             export_image();
            }
           break;
  case 112:if (checkparams(paramcount, 1, m1083))
            {
             if (img.yaxis.label)
               safe_free(img.yaxis.label);
             img.yaxis.label = strcopy(img.yaxis.label, params[0]);
             export_image();
            }
           break;
  case 113:if (checkparams(paramcount, 1, m1084))
             plotay(params[0]);
           break;
  case 114:if (checkparams(paramcount, 1, m1186))
            {
             if (!atoi_check(params[0], &tmp, -INT_MAX, +INT_MAX))
               break;                        
             mysrand(tmp);
            }
           break; 
  case 115:if (checkparams(paramcount, 1, m1032))
            {
             tmplist = findmincounts(params, paramcount);
             if (tmplist)
               for (i = 0; i < paramcount; i++)
                {
                 sprintf(varname, "aout1[%d]", i + 1);
                 sprintf(varvalue, "%d", tmplist[i]);
                 set_variable(varname, varvalue);
                }
            }
           break; 
  case 116:if (checkparams(paramcount, 3, m1082, m1184, m1185))
            {
             if (!atof_check(params[2], &tmpf, 0.0, +DBL_MAX))
               break;                        
             tmp = listindex(params[1], colors, COLORCOUNT);             
             if (tmp == -1)
               tmp = 0;
             plotxyline(params[0], tmp, tmpf);
            }
           break;
  case 117:if (checkparams(paramcount, 1, m1085))
            {
             if (img.imagelegend.legendcount != 0)
               free_2d((void**)img.imagelegend.names, img.imagelegend.legendcount);
             img.imagelegend.names = (char **)safemalloc(paramcount * sizeof(char *), "main", 682);
             for (loop = 0;loop < paramcount; loop++)
               img.imagelegend.names[loop] = strcopy(img.imagelegend.names[loop], params[loop]);
             img.imagelegend.legendcount = paramcount;
             export_image();
            }
           break;
  case 118:if (checkparams(paramcount, 2, m1006, m1081))
             binary_tests_compare(params, paramcount, 1, output);
           break;
  case 119:if (checkparams(paramcount, 3, m1086, m1087, m1087))
            {
             if (!atoi_check(params[0], &tmp, 0, +INT_MAX))
               break;                        
             p = polyfitfile(params[1], tmp, &tmpd);
             set_default_variable("out1", tmpd);
             tmpd2 = calculate_msefile(params[2], p);
             set_default_variable("out2", tmpd2);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, "Error: ", "\n");
               printf(pst, tmpd);
               print_polynom(p, tmpst3);
               printf("%s\n", tmpst3);
              }
            }
           break;
  case 120:if (checkparams(paramcount, 1, m1082))
             plotgauss(params[0]);
           break;
  case 121:
  case 122:if (checkparams(paramcount, 5, m1032, m1019, m1088, m1089, m1090))
            {
             if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             if (!atof_check(params[2], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atof_check(params[3], &tmpd2, -DBL_MAX, +DBL_MAX))
               break;                        
             data = polydatagenerate(tmp, tmpd, tmpd2, params[4], res - 121);
             savepolydata(params[0], data, tmp);
            }
           break;
  case 123:if (checkparams(paramcount, 4, m1088, m1089, m1090, m1187))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atof_check(params[1], &tmpd2, -DBL_MAX, +DBL_MAX))
               break;                        
             tmp = listindex(params[3], colors, COLORCOUNT);             
             if (tmp == -1)
               tmp = 0;
             polyplot(tmpd, tmpd2, params[2], tmp, 0);
            }
           break;
  case 124:if (checkparams(paramcount, 4, m1032, m1091, m1092, m1093))
            {
             if (!atof_check(params[1], &tmpd, 0.0, 1.0))
               break;                        
             if (!atoi_check(params[2], &tmp, 2, +INT_MAX))
               break;                        
             dividefile(params[0], tmpd, tmp, params[3]);
            }
           break;
  case 125:if (checkparams(paramcount, 3, m1032, m1175, m1032))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             p = polylearn(params[0], params[2], tmp, &tmpd, &tmpd2, REGRESSION);
             set_default_variable("out1", p->start->power);
             set_default_variable("out2", tmpd);
             set_default_variable("out3", tmpd2);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, "Train Error: ", "\n");
               printf(pst, tmpd);
               set_precision(pst, "Test Error: ", "\n");
               printf(pst, tmpd2);
               print_polynom(p, tmpst3);
               printf("%s\n", tmpst3);
              }
             free_polynom(p);
            }
           break;
  case 126:if (checkparams(paramcount,1,m1032))
            {
             tmp=file_length(params[0]);
             if (get_parameter_o("displayresulton"))
               printf(m1291, tmp, params[0]);
             set_default_variable("out1",tmp + 0.0);
            }
           break;
  case 127:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("usediscrete", params[0]);
           break; 
  case 128:if (checkparams(paramcount, 1, m1032))
            {
             tmp = modoffile(params[0], &tmp2);
             if (get_parameter_o("displayresulton"))
               printf(m1292, tmp, tmp2);
             set_default_variable("out1", tmp);
             set_default_variable("out2", ((double)(tmp2)) / file_length(params[0]));
            }
           break;
  case 129:if (checkparams(paramcount, 2, m1174, m1175))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             tmp2 = polylearnvalidation(params[0], tmp, REGRESSION);
             set_default_variable("out1", tmp2 + 0.0);
             break;
            }
           break;
  case 130:if (checkparams(paramcount, 4, m1032, m1096, m1097, m1098))
            {
             if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp2, 1, +INT_MAX))
               break;                        
             if (!atof_check(params[3], &tmpd, 0.0, 1.0))
               break;                        
             generate_statistical_data(params[0], tmp, tmp2, tmpd);
            }
           break;
  case 131:if (checkparams(paramcount, 1, m1099))
            {
             set_parameter_i("foldcount", params[0], 1, +INT_MAX);
             set_parameter_i("runtofold", params[0], 1, +INT_MAX);
            }
           break;
  case 132:if (checkparams(paramcount, 1, m1100))
             set_parameter_i("runcount", params[0], 1, +INT_MAX);
           break;
  case 133:run_supervised_algorithm(NO_META_ALGORITHM, LDT);
           break;
  case 134:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("parallel", params[0]);
           break;
  case 135:if (checkparams(paramcount,1,m1032))
            {
             tmpd = fileexists(params[0]);
             if (get_parameter_o("displayresulton") && tmpd == 0)
               printf(m1293, params[0]);
             set_default_variable("out1",tmpd);
            }
           break;
  case 136:if (checkparams(paramcount, 1, m1101))
             set_parameter_i("paralleltype", params[0], 0, +INT_MAX);
           break;
  case 137:if (checkparams(paramcount, 1, m1103))
             set_parameter_i("runtofold", params[0], 0, get_parameter_i("foldcount"));
           break;
  case 138:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("printbinary", params[0]);
           break;
  case 139:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("displayruntime", params[0]);
           break;
  case 140:if (checkparams(paramcount, 1, m1104))
             set_parameter_i("proccount", params[0], 1, +INT_MAX);
           break;
  case 141:run_supervised_algorithm(NO_META_ALGORITHM, IREP);
           break;
  case 142:run_supervised_algorithm(NO_META_ALGORITHM, IREP2);
           break;
  case 143:run_supervised_algorithm(NO_META_ALGORITHM, RIPPER);
           break;
  case 144:if (checkparams(paramcount, 1, m1105))
             set_parameter_i("optimizecount", params[0], 0, +INT_MAX);
           break;
  case 145:if (checkparams(paramcount, 1, m1106))
             set_parameter_i("precision", params[0], 0, +INT_MAX);
           break;
  case 146:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("texoutput", params[0]);
           break;
  case 147:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("accuracy", params[0]);
           break;
  case 148:run_supervised_algorithm(NO_META_ALGORITHM, C45RULES);
           break;
  case 149:if (checkparams(paramcount,2,m1006,m1107))
             save_names_file(params[0],params[1]);
           break;
  case 150:if (checkparams(paramcount, 5, m1006, m1108, m1109, m1110, m1043))
            {
             if (strIcmp(params[4], "off") == 0)
               mustrealize = NO;
             else
               mustrealize = YES;
             mustnormalize = NO;
             export_data_file(params[0], params[1], params[2], params[3]);
            }
           break;
  case 151:if (checkparams(paramcount, 4, m1006, m1108, m1110, m1043))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               if (strIcmp(params[3], "off") == 0)
                 mustrealize = NO;
               else
                 mustrealize = YES;
               mustnormalize = NO;
               current_dataset = d;
               tmpst = dataset_file_name(d);
               export_data_file(params[0], params[1], tmpst, params[2]);
               safe_free(tmpst);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 152:if (checkparams(paramcount, 1, m1071))
             add_variable(FILE_TYPE, params, paramcount, &vars, &varcount);
           break;
  case 153:if (checkparams(paramcount, 1, m1071))
             open_userfile(params[0], params[1]);
           break;
  case 154:if (checkparams(paramcount, 1, m1071))
             close_userfile(params[0]);
           break;
  case 155:if (checkparams(paramcount, 2, m1071, m1078))
             write_userfile(params[0], params[1]);
           break;
  case 156:if (checkparams(paramcount, 4, m1006, m1108, m1049, m1043))
            {
             current_dataset = search_dataset(Datasets, params[0]);
             if (current_dataset)
              {
               if (strIcmp(params[3], "off") == 0)
                 mustrealize = NO;
               else
                 mustrealize = YES;
               mustnormalize = NO;
               if (!atoi_check(params[2], &tmp, -INT_MAX, +INT_MAX))
                 break;                        
               export_datafolds(params[1], tmp);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 157:if (checkparams(paramcount, 1, m1222))
             set_parameter_s("posteriorfilename", params[0]);
           break;
  case 158:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("usefisher", params[0]);
           break;
  case 159:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("smallset", params[0]);
           break;
  case 160:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("withoutoutliers", params[0]);
           break;
  case 161:if (checkparams(paramcount, 1, m1112))
             set_parameter_i("smallsetsize", params[0], 0, +INT_MAX);
           break;
  case 162:if (checkparams(paramcount, 1, m1112))
             set_parameter_f("variancerange", params[0], 0.0, +DBL_MAX);
           break;
  case 163:run_supervised_algorithm(NO_META_ALGORITHM, MULTIRIPPER);
           break;
  case 164:run_supervised_algorithm(NO_META_ALGORITHM, HYBRIDRIPPER);
           break;
  case 165:if (checkparams(paramcount, 2, m1071, m1114))
             read_userfile(params[0], params[1]);
           break;
  case 166:if (checkparams(paramcount, 2, m1071, m1115))
             tokenize(params[0], params[1]);
           break;
  case 167:if (checkparams(paramcount, 2, m1071, m1115))
             countchar(params[0], params[1]);
           break;
  case 168:if (checkparams(paramcount, 1, m1071))
             stringlength(params[0]);
           break;
  case 169:if (checkparams(paramcount, 2, m1071, m1116))
            {
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;                        
             charatindex(params[0], tmp); 
            }
           break;
  case 170:if (checkparams(paramcount, 7, m1006, m1108, m1117, m1214, m1118, m1119, m1213))
            {
             if (!atoi_check(params[2], &tmp, 0, +INT_MAX))
               tmp = -1;
             create_dataset(params[0], params[1], tmp, params[3], params[4], params[5], params[6]);
            }
           break;
  case 171:if (checkparams(paramcount, 2, m1006, m1032))
            {
             d = search_dataset(Datasets, params[0]);
             if (d->storetype != STORE_KERNEL)
               extract_definitions_of_dataset(params[0]);
             xmlfile = fopen(params[1], "w");
             fprintf(xmlfile, "<datasets>\n");
             save_definitions_of_dataset_as_xml(xmlfile, d);
             fprintf(xmlfile, "</datasets>\n");
             fclose(xmlfile);
            }
           break;
  case 172:if (checkparams(paramcount, 1, m1128))
             search_help(params[0]);
           break;
  case 173:run_supervised_algorithm(NO_META_ALGORITHM, MLPGENERIC);
           break;
  case 174:if (paramcount == 0)
             printf(m1129);
           else
            {
             if (!atoi_check(params[0], &tmp, 0, +INT_MAX))
               break;                        
             if (paramcount == 1 && tmp == 0)
               set_integer_parameter("hiddenlayers", 0);
             else
               set_integer_parameter("hiddenlayers", paramcount);
             set_parameter_ai("hiddennodes", params, paramcount);
            }
           break;
  case 175:if (checkparams(paramcount, 1, m1005))
             set_parameter_l("namestype", params[0], m1170);
           break;
  case 176:run_supervised_algorithm(NO_META_ALGORITHM, KNNREG);
           break;
  case 177:if (checkparams(paramcount, 1, m1134))
             set_parameter_i("nearcount", params[0], 1, +INT_MAX);
           break;
  case 178:mustrealize = YES;
           mustnormalize = YES;
           current_dataset = search_dataset(Datasets, params[0]);
           if (current_dataset->sigma == 0)
            {
             read_from_train_file(current_dataset, &traindata);
             set_float_parameter("sigmaestimate", (double) estimate_sigma_square_with_knn(traindata, get_parameter_i("nearcount")));
             deallocate(traindata);
             traindata = NULL;
            }
           else
             set_float_parameter("sigmaestimate", (double) current_dataset->sigma);
           run_supervised_algorithm(NO_META_ALGORITHM, REGTREE);
           break;
  case 179:run_supervised_algorithm(NO_META_ALGORITHM, MLPGENERICREG);
           break;
  case 180:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("writerules", params[0]);
           break;
  case 181:if (checkparams(paramcount, 1, m1006))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               printf("%d\n", d->classno);
               set_default_variable("out1",d->classno + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 182:if (checkparams(paramcount,1,m1006))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               printf("%d\n", d->datacount);
               set_default_variable("out1",d->datacount + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 183:if (checkparams(paramcount,1,m1006))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               printf("%d %d\n", d->featurecount - 1, d->multifeaturecount);
               set_default_variable("out1", d->featurecount - 1.0);
               set_default_variable("out2", d->multifeaturecount - 1.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 184:if (checkparams(paramcount,1,m1006))
            {
             tmp = number_of_symbolic_features(params[0]);
             if (tmp != -1)
              {
               printf("%d\n", tmp);
               set_default_variable("out1",tmp + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 185:if (checkparams(paramcount,1,m1006))
            {
             tmp = number_of_numeric_features(params[0]);
             if (tmp != -1)
              {
               printf("%d\n", tmp);
               set_default_variable("out1",tmp + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 186:if (checkparams(paramcount,1,m1006))
            {
             tmpd = class_entropy(params[0]);
             if (tmpd != -1)
              {
               set_precision(pst, NULL, "\n");
               printf(pst, tmpd);
               set_default_variable("out1",tmpd + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 187:if (checkparams(paramcount, 4, m1018, m1096, m1135, m1136))
            {
             if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             if (!atof_check(params[2], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atof_check(params[3], &tmpd2, 0.0, +DBL_MAX))
               break;                        
             produce_normal_distribution(params[0], tmp, tmpd, tmpd2);
            }
           break;
  case 188:if (checkparams(paramcount, 2, m1137, m1138))
             bootstrap(params[0], params[1]);
           break;
  case 189:if (checkparams(paramcount, 3, m1108, m1223, m1224))
             convert_datasets(params[0][0], params[1], params[2]);
           break;
  case 190:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("savetestfiles", params[0]);
           break;
  case 191:if (checkparams(paramcount, 2, m1140, m1141))
            {
             if (!atoi_check(params[0], &tmp, 1, 100))
               break;                        
             tmpdi = generate_permutations(tmp, &tmp2);
             for (i = 0; i < tmp2; i++)
              {
               sprintf(tmpst3, "%c", '0' + tmpdi[i][0]);
               for (j = 1; j < tmp; j++)
                 sprintf(tmpst3, "%s%c", tmpst3, '0' + tmpdi[i][j]);
               sprintf(tmpst4, "%s[%d]", params[1], i + 1);
               set_variable(tmpst4, tmpst3);
               safe_free(tmpdi[i]);
              }
             safe_free(tmpdi);
            }
           break;
  case 192:if (checkparams(paramcount, 1, m1143))
             set_parameter_s("classpermutation", params[0]);
           break;
  case 193:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("rulesordered", params[0]);
           break;
  case 194:switch (paramcount)
            {
             case  0:printf(m1006);
                     break;
             case  1:printf(m1144);
                     break;
             case  2:printf(m1145);
                     break;
             default:d = search_dataset(Datasets, params[0]);
                     if (d)
                      {
                       read_from_train_file(d, &traindata);
                       if (!k_way_anova_dataset(d, traindata, params, paramcount))
                         printf(m1294);
                       deallocate(traindata);
                       traindata = NULL;
                      }
                     else
                       printf(m1279, params[0]);
                     break;
            }
           break;
  case 195:run_supervised_algorithm(NO_META_ALGORITHM, REXRIPPER);
           break;
  case 196:if (checkparams(paramcount, 1, m1394))
             set_parameter_l("multivariatealgorithm", params[0], m1395);
           break;
  case 197:if (checkparams(paramcount, 5, m1216 , m1006, m1146, m1147, m1336))
            {
             tmp5 = listindex(params[0], mlp_simulation_algorithms, MAX_MLP_SIMULATION_ALGORITHMS);
             if (!atoi_check(params[3], &tmp2, 0, +INT_MAX))
               break;                        
             if (!atoi_check(params[4], &tmp4, 0, +INT_MAX))
               break;                        
             tmp = mlp_model_simulation(tmp5, params[1], params[2], tmp2, tmp4, &tmp3);
             set_default_variable("out1", tmp + 0.0);
             set_default_variable("out2", tmp3 + 0.0);
            }
           break;
  case 198:if (checkparams(paramcount, 1, m1059))
             set_parameter_f("confidencelevel", params[0], 0.0, 1.0);
           break;
  case 199:if (checkparams(paramcount, 3, m1006, m1148, m1148))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               if (!atoi_check(params[1], &tmp2, 1, d->featurecount))
                 break;                        
               if (!atoi_check(params[2], &tmp3, 1, d->featurecount))
                 break;                        
               current_dataset = d;
               read_from_train_file(d, &traindata);
               plotdata2d(traindata, d, tmp2 - 1, tmp3 - 1);
               deallocate(traindata);
               traindata = NULL;
              }
             else
               printf(m1259, params[0]);
            }
           break;
  case 200:if (checkparams(paramcount, 1, m1183))
             save_image(params[0]);
           break;
  case 201:if (checkparams(paramcount, 1, m1183))
             load_image(params[0]);
           break;
  case 202:if (checkparams(paramcount, 2, m1004, m1004))
            {
             xmlfile = fopen(params[0], "w");
             fprintf(xmlfile, "<datasets>\n");
             for (i = 1; i < paramcount; i++)
              {
               d = load_definitions(params[i], &tmp);
               tmpdataset = d;
               while (tmpdataset)
                {
                 save_definitions_of_dataset_as_xml(xmlfile, tmpdataset);
                 tmpdataset = tmpdataset->next;
                }
               free_dataset(d);
              }
             fprintf(xmlfile, "</datasets>\n");
             fclose(xmlfile);
            }
           break;
  case 203:if (checkparams(paramcount, 2, m1151, m1152))
            {
             tmp = listindex(params[0], imageparts, IMAGEPARTCOUNT);             
             if (tmp == -1)
               printf(m1151);
             else
              {
               tmp2 = listindex(params[1], fonts, FONTCOUNT);
               if (tmp2 == -1)
                 printf(m1152);
               else
                 switch (tmp)
                  {
                   case 0:for (i = 0; i < img.objectcount; i++)
                            if (img.objects[i].type == 1)
                              img.objects[i].fnt.fontname = tmp2;
                          break;
                   case 1:img.imagelegend.fnt.fontname = tmp2;
                          break;
                   case 2:img.xaxis.axisfnt.fontname = tmp2;
                          img.yaxis.axisfnt.fontname = tmp2;
                          break;
                   case 3:img.xaxis.labelfnt.fontname = tmp2;
                          img.yaxis.labelfnt.fontname = tmp2;
                          break;
                  }
              }
             export_image();
            }
           break;
  case 204:if (checkparams(paramcount, 2, m1151, m1153))
            {
             tmp = listindex(params[0], imageparts, IMAGEPARTCOUNT);             
             if (tmp == -1)
               printf(m1151);
             else
              {
               if (!atof_check(params[1], &tmpf, 8.0, +DBL_MAX))
                 break;                        
               switch (tmp)
                {
                 case 0:for (i = 0; i < img.objectcount; i++)
                          if (in_list(img.objects[i].type, 2, OBJECT_STRING, OBJECT_NAMEDPOINT))
                            img.objects[i].fnt.fontsize = tmpf;
                        break;
                 case 1:img.imagelegend.fnt.fontsize = tmpf;
                        break;
                 case 2:img.xaxis.axisfnt.fontsize = tmpf;
                        img.yaxis.axisfnt.fontsize = tmpf;
                        break;
                 case 3:img.xaxis.labelfnt.fontsize = tmpf;
                        img.yaxis.labelfnt.fontsize = tmpf;
                        break;
                }
              }
             export_image();
            }
           break;
  case 205:run_supervised_algorithm(NO_META_ALGORITHM, MULTILDT);
           break;
  case 206:run_supervised_algorithm(NO_META_ALGORITHM, OMNILDT);
           break;
  case 207:run_dimension_reduction_algorithm(PCA);
           break;
  case 208:if (paramcount == 0)
             printf(m1155);
           else
             set_parameter_ai("is_class_plotted", params, paramcount);
           break;
  case 209:if (paramcount == 0)
             printf(m1156);
           else
             set_parameter_ai("is_class_exported", params, paramcount);
           break;
  case 210:binary_comparison(combined5x2t, params, paramcount);
           break;
  case 211:if (checkparams(paramcount, 5, m1157, m1158, m1159, m1182, m1106))
            {
             if (strcmp(params[0],"x") == 0)
              {
               img.xaxis.limitsset = 1;
               mylimits = &(img.xaxis.lim);
              }
             else
               if (strcmp(params[0],"y") == 0)
                {
                 img.yaxis.limitsset = 1;
                 mylimits = &(img.yaxis.lim);
                }
               else
                {
                 printf(m1157);
                 break;
                }
             if (!atof_check(params[1], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atof_check(params[2], &tmpd2, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atoi_check(params[3], &tmp, 1, PARTITIONS))
               break;                        
             if (!atoi_check(params[4], &tmp2, 0, 5))
               break;                        
             mylimits->min = tmpd;
             mylimits->max = tmpd2;
             mylimits->division = tmp;
             mylimits->precision = tmp2;
             export_image();
            }
           break;
  case 212:if (checkparams(paramcount, 1, m1161))
             compile_file(params[0]);
           break;
  case 213:if (checkparams(paramcount, 4, m1006, m1037, m1162, m1163))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               current_dataset = d;
               read_from_train_file(d, &traindata);
               if (!atof_check(params[2], &tmpd, 0.0, +DBL_MAX))
                 break;                        
               if (!atof_check(params[3], &tmpd2, 0.0, 1.0))
                 break;                        
               generate_data_with_noisy_features_from_a_dataset(d, traindata, params[1], tmpd, tmpd2);
               deallocate(traindata);
               traindata = NULL;
              }
             else
               printf(m1259, params[0]);
            }
           break;
  case 214:if (checkparams(paramcount,2,m1035,m1035))
             if (in_list(get_parameter_l("testtype"), 13, RANKSUMTEST, SIGNEDRANKTEST, TTEST, PAIREDT5X2, PAIREDTTEST, COMBINEDT5X2, SIGNTEST, PERMUTATIONTEST, PAIREDPERMUTATIONTEST, HOTELLINGT, SIGNTEST, NEMENYI, NOTEST))
              {
               tmp = find_best_classifier(params, paramcount, get_parameter_l("testtype"));
               set_default_variable("out1",tmp + 0.0);
               if (get_parameter_o("displayresulton"))
                 if (tmp != -1)
                   printf(m1282, params[tmp-1]);
              }
           break;
  case 215:if (checkparams(paramcount, 1, m1082))
             plotinvgauss(params[0]);
           break;
  case 216:if (checkparams(paramcount, 1, m1082))
             plotsigmoid(params[0]);
           break;
  case 217:if (checkparams(paramcount, 1, m1165))
             set_parameter_l("correctiontype", params[0], m1168);
           break;
  case 218:if (checkparams(paramcount, 1, m1166))
             set_parameter_l("modelselectionmethod", params[0], m1167);
           break;
  case 219:run_supervised_algorithm(NO_META_ALGORITHM, SELECTAVERAGE);
           break;
  case 220:run_supervised_algorithm(NO_META_ALGORITHM, LINEARREG);
           break;
  case 221:binary_comparison(regressionftest, params, paramcount);
           break;
  case 222:if (checkparams(paramcount, 3, m1071, m1114, m1172))
            {
             if (!atoi_check(params[2], &tmp, 1, +INT_MAX))
               break;                        
             read_array_from_userfile(params[0], params[1], tmp);
            }
           break;
  case 223:if (checkparams(paramcount, 3, m1011, m1037, m1037))
             find_mse_and_sse(params[0], params[1], params[2]);
           break;
  case 224:run_supervised_algorithm(NO_META_ALGORITHM, QUANTIZEREG);
           break;
  case 225:if (checkparams(paramcount, 1, m1173))
             set_parameter_i("partitioncount", params[0], 2, +INT_MAX);
           break;
  case 226:if (checkparams(paramcount, 4, m1174, m1175, m1176, m1177))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp2, 2, +INT_MAX))
               break;                        
             if (!atoi_check(params[3], &tmp3, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             bias_variance_polynom(params[0], tmp, tmp2, tmp3, REGRESSION);
            }
           break;
  case 227:
  case 228:if (checkparams(paramcount, 3, m1174, m1019, m1032))
            {
             if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             data = functiondatageneratefile(tmp, params[0], res - 227);
             savepolydata(params[2], data, tmp);
            }
           break;
  case 229:if (checkparams(paramcount, 1, m1178))
             set_parameter_f("noiselevel", params[0], 0.0, 1.0);
           break;
  case 230:if (checkparams(paramcount, 2, m1174, m1187))
            {
             tmp = listindex(params[1], colors, COLORCOUNT);             
             if (tmp == -1)
               tmp = 0;
             functionplot(params[0], tmp);
            }
           break;
  case 231:if (checkparams(paramcount, 1, m1183))
             rename("image.eps", params[0]);
           break;
  case 232:if (checkparams(paramcount, 1, m1043))
            {
             set_parameter_o("hold", params[0]);
             if (get_parameter_o("hold") == OFF)
               clear_image();
            }
           break;
  case 233:if (checkparams(paramcount, 3, m1188, m1019, m1032))
            {
             if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             data = generate_data_from_mixture_file(params[0], tmp);
             savepolydata(params[2], data, tmp);
            }
           break;
  case 234:if (checkparams(paramcount, 1, m1085))
            {
             if (img.xaxis.namesavailable != 0)
               free_2d((void**)img.xaxis.names, img.xaxis.namesavailable);
             img.xaxis.names = (char **)safemalloc(paramcount * sizeof(char *), "main", 682);
             for (loop = 0;loop < paramcount; loop++)
               img.xaxis.names[loop] = strcopy(img.xaxis.names[loop], params[loop]);
             img.xaxis.namesavailable = paramcount;
             export_image();
            }
           break;
  case 235:if (checkparams(paramcount, 3, m1032, m1175, m1032))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             p = polylearn(params[0], params[2], tmp, &tmpd, &tmpd2, CLASSIFICATION);
             set_default_variable("out1",p->start->power);
             set_default_variable("out2",tmpd);
             set_default_variable("out3",tmpd2);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, "Train Error: ", "\n");
               printf(pst, tmpd);
               set_precision(pst, "Test Error: ", "\n");
               printf(pst, tmpd2);
               print_polynom(p, tmpst3);
               printf("%s\n", tmpst3);
              }
             free_polynom(p);
             break;
            }
           break;
  case 236:if (checkparams(paramcount, 1, m1013))
             set_parameter_f("etadecrease", params[0], 0.0, +DBL_MAX);
           break;
  case 237:if (checkparams(paramcount, 4, m1174, m1175, m1176, m1177))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp2, 2, +INT_MAX))
               break;                        
             if (!atoi_check(params[3], &tmp3, MIN_SAMPLE_SIZE, +INT_MAX))
               break;                        
             bias_variance_polynom(params[0], tmp, tmp2, tmp3, CLASSIFICATION);
            }
           break;
  case 238:if (checkparams(paramcount, 2, m1188, m1175))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             tmp2 = polylearnvalidation(params[0], tmp, CLASSIFICATION);
             set_default_variable("out1", tmp2 + 0.0);
             break;
            }
           break;
  case 239:if (checkparams(paramcount, 2, m1086, m1190))
            {
             if (!atoi_check(params[0], &tmp, 0, +INT_MAX))
               break;                        
             p = polyfitclsfile(params[1], tmp, &tmpd);
             set_default_variable("out1", tmpd + 0.0);
             if (get_parameter_o("displayresulton"))
              {
               set_precision(pst, "Error: ", "\n");
               printf(pst, tmpd);
               print_polynom(p, tmpst3);
               printf("%s\n", tmpst3);
              }
            }
           break;
  case 240:if (checkparams(paramcount, 4, m1088, m1089, m1090, m1187))
            {
             if (!atof_check(params[0], &tmpd, -DBL_MAX, +DBL_MAX))
               break;                        
             if (!atof_check(params[1], &tmpd2, -DBL_MAX, +DBL_MAX))
               break;                        
             tmp = listindex(params[3], colors, COLORCOUNT);             
             if (tmp == -1)
               tmp = 0;
             polyplot(tmpd, tmpd2, params[2], tmp, 1);
            }
           break;
  case 241:binary_comparison(signtest, params, paramcount);
           break;
  case 242:test_code();
           break;
  case 243:if (checkparams(paramcount, 1, m1032))
            {
             tmpd = file_sum(params[0]);
             set_default_variable("out1", tmpd);
            }
           break;
  case 244:if (checkparams(paramcount, 1, m1178))
             set_parameter_f("sigma", params[0], 0.0, +DBL_MAX);
           break;
  case 245:if (checkparams(paramcount, 1, m1192))
             set_parameter_l("prunetype", params[0], m1193);
           break;
  case 246:if (checkparams(paramcount, 1, m1194))
             set_parameter_f("pruningthreshold", params[0], 0.0, 1.0);
           break;
  case 247:if (checkparams(paramcount, 1, m1195))
             set_parameter_l("leaftype", params[0], m1196);
           break;
  case 248:mustrealize = YES;
           mustnormalize = YES;
           current_dataset = search_dataset(Datasets, params[0]);
           if (current_dataset->sigma == 0)
            {
             read_from_train_file(current_dataset, &traindata);
             set_float_parameter("sigmaestimate", (double) estimate_sigma_square_with_knn(traindata, get_parameter_i("nearcount")));
             deallocate(traindata);
             traindata = NULL;
            }
           else
             set_float_parameter("sigmaestimate", (double) current_dataset->sigma);
           run_supervised_algorithm(NO_META_ALGORITHM, REGRULE);
           break;
  case 249:current_dataset = search_dataset(Datasets, params[0]);
           if (current_dataset)
            {
             read_from_train_file(current_dataset, &traindata);
             tmpd = (double) estimate_sigma_square_with_knn(traindata, get_parameter_i("nearcount"));
             deallocate(traindata);
             traindata = NULL;
             current_dataset->sigma = tmpd;
             set_default_variable("out1", tmpd + 0.0);
            }
           break;
  case 250:if (checkparams(paramcount, 2, m1197, m1037))
            {
             m = read_multiple_model_from_file(params[0]);
             mat = generate_data_from_multiple_model(m);
             write_matrix(params[1], mat, NO);
             matrix_free(mat);
             free_multiple_model(m);
            }
           break;
  case 251:d = search_dataset(Datasets, params[0]);
           if (d && d->classno == 0)
            {
             current_dataset = d;
             read_from_train_file(d, &traindata);
             mmelin1(&traindata);
             deallocate(traindata);
             traindata = NULL;
            }
           break;
  case 252:run_supervised_algorithm(NO_META_ALGORITHM, LOGISTIC);
           break;
  case 253:if (checkparams(paramcount, 1, m1198))
             set_parameter_l("kerneltype", params[0], m1199);
           break;
  case 254:if (checkparams(paramcount, 1, m1200))
             set_parameter_i("svmdegree", params[0], 0, +INT_MAX);
           break;
  case 255:if (checkparams(paramcount, 1, m1201))
             set_parameter_f("svmgamma", params[0], -DBL_MAX, +DBL_MAX);
           break;
  case 256:if (checkparams(paramcount, 1, m1202))
             set_parameter_f("svmcoef0", params[0], -DBL_MAX, +DBL_MAX);
           break;
  case 257:if (checkparams(paramcount, 1, m1203))
             set_parameter_f("svmC", params[0], -DBL_MAX, +DBL_MAX);
           break;
  case 258:if (checkparams(paramcount, 1, m1204))
             set_parameter_f("svmnu", params[0], 0.0, 1.0);
           break;
  case 259:if (checkparams(paramcount, 1, m1205))
             set_parameter_f("svmepsilon", params[0], 0.0, +DBL_MAX);
           break;
  case 260:run_supervised_algorithm(NO_META_ALGORITHM, SVM);
           break;
  case 261:run_supervised_algorithm(NO_META_ALGORITHM, SIMPLEXSVM);
           break;
  case 262:run_supervised_algorithm(NO_META_ALGORITHM, SVMREG);
           break;
  case 263:run_supervised_algorithm(NO_META_ALGORITHM, SIMPLEXSVMREG);
           break;
  case 264:if (paramcount == 0)
             printf(m1206);
           else
             set_parameter_af("svmcweights", params, paramcount);           
           break;
  case 265:if (checkparams(paramcount, 6, m1210, m1108, m1207, m1208, m1209, m1037))
             convert_byte_format_to_dataset_format(params[0], params[1][0], params[2], params[3], params[4], params[5]);
           break;
  case 266:if (paramcount < 5)
             checkparams(paramcount, 5, m1006, m1037, m1096, m1211, m1212);
           else
            {
             current_dataset = search_dataset(Datasets, params[0]);
             if (current_dataset)
              {
               read_from_train_file(current_dataset, &traindata);
               if (!atoi_check(params[2], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
                 break;                        
               w = safemalloc(current_dataset->classno * sizeof(double), "process_order", 1555); 
               for (i = 3; i < paramcount; i++)
                 if (!atof_check(params[i], &(w[i - 3]), 0.0, +DBL_MAX))
                   w[i - 3] = 0.0;
               subsample(current_dataset, params[1], traindata, tmp, w);
               deallocate(traindata);
               traindata = NULL;
               safe_free(w);
              }
            }
           break;
  case 267:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("displayclassresulton", params[0]);
           break;
  case 268:run_supervised_algorithm(NO_META_ALGORITHM, SVMTREE);
           break;
  case 269:if (checkparams(paramcount, 3, m1006, m1147, m1049))
            {
             current_dataset = search_dataset(Datasets, params[0]);
             if (current_dataset)
              {
               if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
                 break;                        
               if (!atoi_check(params[2], &tmp2, -INT_MAX, +INT_MAX))
                 break;                        
               mysrand(tmp2);
               read_from_train_file(current_dataset, &traindata);
               testdata = NULL;
               mustrealize = YES;
               pre_process();
               tmpd = mlp_model_selection(&traindata, get_parameter_l("modelselectionmethod"), &tmp2, tmp, &tmpd2);
               set_default_variable("out1", tmp2 + 0.0);
               set_default_variable("out2", tmpd);
               set_default_variable("out3", tmpd2);
               post_process(YES);
               deallocate(traindata);
               traindata = NULL;
              }
            }
           break;
  case 270:if (checkparams(paramcount, 1, m1078))
             printf("%s\n", params[0]);
           break;
  case 271:if (checkparams(paramcount, 1, m1082))
             plotmv(params, paramcount);
           break;
  case 272:if (checkparams(paramcount, 2, m1032, m1215))
            {
             if (!atof_check(params[1], &tmpd, 0.0, 100.0))
               break;                        
             tmpd2 = file_percentile(params[0], tmpd);
             set_default_variable("out1", tmpd2);
            }
           break;
  case 273:if (checkparams(paramcount, 1, m1082))
             box_plot(params, paramcount);
           break;
  case 274:if (checkparams(paramcount, 1, m1006))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = YES;
               current_dataset = d;
               read_from_train_file(d, &traindata);
               realize(traindata);
               exchange_classdefno(current_dataset);
               mean = find_mean(traindata);
               variance = find_variance(traindata);
               normalize(traindata, mean, variance);
               safe_free(mean.values);
               mean = find_mean(traindata);
               cov = covariance(traindata, mean);
               safe_free(mean.values);
               safe_free(variance.values);
               eigenvalues = find_eigenvalues_eigenvectors(&eigenvectors, cov, &tmp, get_parameter_f("variance_explained"));
               safe_free(eigenvalues);
               matrix_free(eigenvectors);
               matrix_free(cov);
               exchange_classdefno(current_dataset);
               deallocate(traindata);
               traindata = NULL;
               set_default_variable("out1", tmp + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 275:if (checkparams(paramcount, 4, m1006, m1146, m1147, m1336))
            {
             if (!atoi_check(params[2], &tmp, 0, +INT_MAX))
               break;                        
             if (!atoi_check(params[3], &tmp2, 0, +INT_MAX))
               break;                        
             tmp3 = compare_mlp_model_selection_techniques(params[0], params[1], tmp, tmp2);
             set_default_variable("out1", tmp3 + 0.0);
            }
           break;
  case 276:run_supervised_algorithm(NO_META_ALGORITHM, GAUSSIAN);
           break;           
  case 277:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("savemodel", params[0]);
           break;
  case 278:if (checkparams(paramcount, 1, m1227))
             set_parameter_s("modelfilename", params[0]);
           break;
  case 279:if (checkparams(paramcount, 2, m1227, m1228))
             test_model(params[0], params[1]);
           break;
  case 280:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("normalizeoutput", params[0]);
           break;
  case 281:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("weightdecay", params[0]);
           break;
  case 282:if (checkparams(paramcount, 1, m1229))
             set_parameter_f("decayalpha", params[0], 0.0, +DBL_MAX);
           break;
  case 283:deallocate_datasets();
           Datasets = NULL;
           break;
  case 284:if (checkparams(paramcount, 3, m1006, m1037, m1096))
            {
             current_dataset = search_dataset(Datasets, params[0]);
             if (current_dataset)
              {
															mustrealize = OFF;
               read_from_train_file(current_dataset, &traindata);
               if (!atoi_check(params[2], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
                 break;                        
               subsample_regression(current_dataset, params[1], traindata, tmp);
               deallocate(traindata);
               traindata = NULL;
              }
            }
           break;
  case 285:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("laplaceestimate", params[0]);
           break;
  case 286:if (checkparams(paramcount, 4, m1326, m1327, m1323, m1347))
            {
             if (!atoi_check(params[0], &tmp2, 1, +INT_MAX))
               break;                        
             if (!atoi_check(params[1], &tmp3, 1, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp4, 2, +INT_MAX))
               break;  
             tmpst = safemalloc((tmp2 + 1) * sizeof(char), "main", 345);
             tmplist = safemalloc(tmp2 * sizeof(int), "main", 346);
             strcpy(tmpst3, params[3]);
             strp = strtok(tmpst3, "-");
             for (i = 0; i < tmp2; i++)
              {
               if (strp)
                 if (!atoi_check(strp, &tmp5, 1, +INT_MAX))
                   break;  
               if (tmp5 == 1)
                 tmpst[i] = 'R';
               else
                 tmpst[i] = 'D';
               tmplist[i] = tmp5;
               if (strp)
                 strp = strtok(NULL, "-");
              }
             switch (get_parameter_l("experimentdesign"))
              {
               case UNIFORM_DESIGN  :vc_estimate_uniform_of_decision_tree(tmp2, tmp3, tmp4, tmpst, tmplist);
                                     break;
               case OPTIMIZED_DESIGN:vc_estimate_optimized_of_decision_tree(tmp2, tmp3, tmp4, tmpst, tmplist);
                                     break;
              }
             safe_free(tmpst);
             safe_free(tmplist);
            }
           break;
  case 287:run_supervised_algorithm(NO_META_ALGORITHM, NBTREE);
           break;           
  case 288:if (checkparams(paramcount, 1, m1324))
             set_parameter_i("nodecount", params[0], 1, +INT_MAX);
           break;
  case 289:if (checkparams(paramcount, 1, m1328))
             set_parameter_l("combinationtype", params[0], m1329);
           break;
  case 290:if (checkparams(paramcount, 1, m1463))
             set_parameter_i("svmcache", params[0], 10, 1000);
           break;
  case 291:if (checkparams(paramcount, 1, m1331))
             set_parameter_l("experimentdesign", params[0], m1332);
           break;
  case 292:binary_comparison(no_test, params, paramcount);
           break;
  case 293:if (checkparams(paramcount, 1, m1006))/*F1*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = YES;
               mustnormalize = NO;
               current_dataset = d;
               tmpd = fisher_discriminant_ratio(d);
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 294:if (checkparams(paramcount, 1, m1006))/*F2*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = YES;
               mustnormalize = NO;
               current_dataset = d;
               tmpd = volume_overlap_region(d);
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 295:if (checkparams(paramcount, 1, m1006)) /*F3&F2*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = YES;
               mustnormalize = NO;
               current_dataset = d;
               tmpd = feature_efficiency(d, &tmpd2);
               set_default_variable("out1", tmpd + 0.0);
               set_default_variable("out2", tmpd2 + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 296:if (checkparams(paramcount, 1, m1006)) /*N1*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = NO;
               mustnormalize = YES;
               current_dataset = d;
               tmpd = find_opposite_class_ratio_in_mst(d);
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 297:if (checkparams(paramcount, 1, m1006)) /*N2&N3*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = NO;
               mustnormalize = YES;
               current_dataset = d;
               tmpd = nn_ratio(d, &tmpd2);
               set_default_variable("out1", tmpd + 0.0);
               set_default_variable("out2", tmpd2 + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 298:if (checkparams(paramcount, 1, m1006)) /*N4*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = NO;
               mustnormalize = YES;
               current_dataset = d;
               testdata = create_linear_test_set(d, d->datacount / 2, NO);
               tmpd = nn_nonlinearity(d, testdata);
               deallocate(testdata);
               testdata = NULL;
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 299:if (checkparams(paramcount, 1, m1006)) /*T1*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustrealize = NO;
               mustnormalize = YES;
               current_dataset = d;
               tmpd = adherentsubsets(d);
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1279, params[0]);
            }
           break;              
  case 300:if (checkparams(paramcount, 1, m1006)) /*L1&L2&L3*/
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               mustnormalize = YES;
               mustrealize = YES;
               current_dataset = d;
               tmpd = lp_separability(d, &tmpd2, &tmpd3);
               set_default_variable("out1", tmpd + 0.0); /*L1*/
               set_default_variable("out2", tmpd2 + 0.0); /*L2*/
               set_default_variable("out3", tmpd3 + 0.0); /*L3*/
              }
             else
               printf(m1279, params[0]);
            }
           break;                         
  case 301:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("savetestcode", params[0]);
           break;
  case 302:if (checkparams(paramcount, 1, m1333))
             set_parameter_s("testcodefilename", params[0]);
           break;
  case 303:if (checkparams(paramcount, 1, m1334))
             set_parameter_l("language", params[0], m1335);
           break;
  case 304:if (checkparams(paramcount, 1, m1337))
             set_parameter_s("model_per_level", params[0]);
           break;           
  case 305:if (checkparams(paramcount, 1, m1337))
            {
             tmplist = safecalloc(255, sizeof(int), "main", 452);
             tree = create_binary_tree_according_model_string(params[0]);
             leaf_distribution(tree, tmplist);
             for (i = 1; i < 512; i++)
               write_array_variables("aout", i, 1, "d", tmplist[i - 1]);
             deallocate_tree(tree);
             safe_free(tmplist);
            }
  case 306:if (checkparams(paramcount, 4, m1006, m1108, m1110, m1343))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               if (!atoi_check(params[3], &tmp, 2, +INT_MAX))
                 break;
               divide_data_file(params[0], params[1], params[2], tmp);
              }
             else
               printf(m1279, params[0]);
            }
  case 307:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("showticks", params[0]);
           export_image();           
           break;
  case 308:if (checkparams(paramcount, 1, m1349))
             set_parameter_l("vccalculationtype", params[0], m1350);
           break;
  case 309:if (checkparams(paramcount, 1, m1071))
             add_variable(MATRIX_TYPE, params, paramcount, &vars, &varcount);
           break;
  case 310:if (checkparams(paramcount, 2, m1352, m1353))
            {
             matptr = read_posteriors(1, params[0]);
             set_variable(params[1], matptr);
            }
           break;
  case 311:if (checkparams(paramcount, 2, m1353, m1354))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               interval = create_intervals(params[1]);
               matrix_resize(myvar->value.matrixvalue, interval);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 312:if (checkparams(paramcount, 2, m1353, m1355))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               matptr = safemalloc(sizeof(matrix), "main", 543);
               *matptr = matrix_covariance(*(myvar->value.matrixvalue), &w);
                set_variable(params[1], matptr);
               safe_free(w);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 313:if (checkparams(paramcount, 2, m1353, m1356))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               matptr = safemalloc(sizeof(matrix), "main", 554);
               *matptr = matrix_correlation(*(myvar->value.matrixvalue));
               set_variable(params[1], matptr);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 314:if (checkparams(paramcount, 2, m1353, m1357))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               matptr = safemalloc(sizeof(matrix), "main", 554);
               w = find_eigenvalues_eigenvectors_symmetric(&eigenvectors, *(myvar->value.matrixvalue));
               *matptr = matrix_transpose(eigenvectors);
               matrix_free(eigenvectors);
               for (i = 1; i < myvar->value.matrixvalue->col + 1; i++)
                 write_array_variables("aout", i, 1, "f", w[i - 1]);
               safe_free(w);
               set_variable(params[1], matptr);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 315:if (checkparams(paramcount, 1, m1353))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               mat = *(myvar->value.matrixvalue);
               *(myvar->value.matrixvalue) = matrix_transpose(mat);
               matrix_free(mat);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 316:if (checkparams(paramcount, 2, m1352, m1353))
            {
             matptr = read_posteriors(1, params[0]);
             for (i = 0; i < matptr->row; i++)
              {
               j = (int) (matptr->values[i][matptr->col - 1]);
               matptr->values[i][0] = matptr->values[i][j];
              }
             mat = *matptr;
             *matptr = matrix_partial(mat, 0, mat.row - 1, 0, 0);
             matrix_free(mat);
             set_variable(params[1], matptr);
            }
           break;
  case 317:if (checkparams(paramcount, 1, m1353))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               tmpd = matrix_sum_of_elements(*(myvar->value.matrixvalue));
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 318:if (checkparams(paramcount, 1, m1353))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               tmpd = matrix_trace(*(myvar->value.matrixvalue));
               set_default_variable("out1", tmpd + 0.0);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 319:if (checkparams(paramcount, 2, m1353, m1361))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
               write_matrix(params[1], *(myvar->value.matrixvalue), NO);
             else
               printf(m1437, params[0]);
            }
           break;
  case 320:if (checkparams(paramcount, 4, m1006, m1369, m1370, m1037))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               current_dataset = d;
               if (!atoi_check(params[1], &tmp, 1, d->featurecount))
                 break;
               read_from_data_file(d, params[2], &traindata);
               export_subset_of_features(d, traindata, tmp, params[3]);
               deallocate(traindata);
               traindata = NULL;
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 321:if (checkparams(paramcount, 1, m1373))
             set_parameter_l("hmmtype", params[0], m1374);
           break;
  case 322:if (checkparams(paramcount, 1, m1375))
             set_parameter_l("hmmstatetype", params[0], m1376);
           break;
  case 323:run_supervised_algorithm(NO_META_ALGORITHM, HMM);
           break;
  case 324:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("weightfreezing", params[0]);
           break;
  case 325:if (checkparams(paramcount, 1, m1378))
             set_parameter_l("hmmlearningtype", params[0], m1379);
           break;
  case 326:run_supervised_algorithm(NO_META_ALGORITHM, DNC);
           break;
  case 327:run_supervised_algorithm(NO_META_ALGORITHM, DNCREG);
           break;
  case 328:if (checkparams(paramcount, 1, m1380))
             set_parameter_i("windowsize", params[0], 2, +INT_MAX);
           break;
  case 329:if (checkparams(paramcount, 1, m1381))
             set_parameter_f("errordropratio", params[0], 0.0, 1.0);
           break;
  case 330:if (checkparams(paramcount, 1, m1382))
             set_parameter_i("maxhidden", params[0], 0, +INT_MAX);
           break;
  case 331:if (checkparams(paramcount, 1, m1383))
            {
             strcpy(tmpst3, params[0]);
             for (i = 1; i < paramcount; i++)
               sprintf(tmpst3, "%s %s", tmpst3, params[i]);
             system(tmpst3);
            }
           break;
  case 332:
  case 333:os_command(CLS);
           break;
  case 334:
  case 335:os_command(COPY);
           break;
  case 336:os_command(CD);
           break;
  case 337:
  case 338:os_command(DEL);
           break;
  case 339:
  case 340:os_command(DIR);
           break;
  case 341:
  case 342:os_command(MOVE);
           break;
  case 343:os_command(MKDIR);
           break;
  case 344:os_command(RENAME);
           break;
  case 345:os_command(RMDIR);
           break;
  case 346:if (checkparams(paramcount, 3, m1384, m1019, m1032))
            {
             if (!atoi_check(params[1], &tmp, MIN_SAMPLE_SIZE, +INT_MAX))
               break;
             gm = read_gaussian_mixture(params[0], &tmplist);
             generate_gaussian_mixture_data(gm, tmplist, tmp, params[2]);
             free_state_model(gm, STATE_GAUSSIAN_MIXTURE);
             safe_free(tmplist);
            }
           break;
  case 347:if (checkparams(paramcount, 1, m1385))
             set_parameter_s("confusionfilename", params[0]);
           break;
  case 348:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("printconfusion", params[0]);
           break;
  case 349:run_supervised_algorithm(NO_META_ALGORITHM, QDACLASS);
           break;
  case 350:run_supervised_algorithm(NO_META_ALGORITHM, NAIVEBAYES);
           break;
  case 351:if (paramcount == 0)
             printf(m1386);
           else
             set_parameter_ai("statecounts", params, paramcount);
           break;
  case 352:if (paramcount == 0)
             printf(m1387);
           else
             set_parameter_ai("componentcounts", params, paramcount);
           break;
  case 353:run_supervised_algorithm(NO_META_ALGORITHM, RBF);
           break;
  case 354:run_supervised_algorithm(NO_META_ALGORITHM, RBFREG);
           break;
  case 355:if (checkparams(paramcount, 1, m1388))
             set_parameter_f("srma1", params[0], 0.0, +DBL_MAX);
           break;
  case 356:if (checkparams(paramcount, 1, m1389))
             set_parameter_f("srma2", params[0], 0.0, +DBL_MAX);
           break;
  case 357:if (checkparams(paramcount, 1, m1082))
             error_histogram_plot(params, paramcount);
           break;
  case 358:if (checkparams(paramcount, 1, m1085))
            {
             if (img.yaxis.namesavailable != 0)
               free_2d((void**)img.yaxis.names, img.yaxis.namesavailable);
             img.yaxis.names = (char **)safemalloc(paramcount * sizeof(char *), "main", 682);
             for (loop = 0;loop < paramcount; loop++)
               img.yaxis.names[loop] = strcopy(img.yaxis.names[loop], params[loop]);
             img.yaxis.namesavailable = paramcount;
             export_image();
            }
           break;
  case 359:run_dimension_reduction_algorithm(LDA);
           break;
  case 360:run_dimension_reduction_algorithm(MDS);
           break;
  case 361:run_dimension_reduction_algorithm(ISOMAP);
           break;
  case 362:if (checkparams(paramcount, 1, m1392))
             set_parameter_f("epsilon", params[0], 0.0, +DBL_MAX);
           break;
  case 363:run_dimension_reduction_algorithm(LLE);
           break;
  case 364:if (checkparams(paramcount, 1, m1082))
             plotxynames(params[0]);
           break;
  case 365:if (checkparams(paramcount, 2, m1396, m1397))
            {
             if (!atoi_check(params[0], &tmp, 1, +INT_MAX))
               break;
             if (paramcount == 2)                      
               tmp2 = vc_dimension_of_ruleset(tmp, params[1], BOOLEAN_FALSE, 0);
             else
              {
               atoi_check(params[2], &tmp3, 1, +INT_MAX);
               tmp2 = vc_dimension_of_ruleset(tmp, params[1], BOOLEAN_TRUE, tmp3);
              }
             set_default_variable("out1", tmp2 + 0.0);
            }
           break;
  case 366:if (checkparams(paramcount, 1, m1398))
             set_parameter_l("neighborhoodtype", params[0], m1399);
           break;
  case 367:current_dataset = search_dataset(Datasets, params[0]);
           if (current_dataset)
            {
             read_and_normalize_train_file(current_dataset, &traindata);
             tmp = minimum_k_neighborhood(traindata);
             deallocate(traindata);
             traindata = NULL;
             exchange_classdefno(current_dataset);
             set_default_variable("out1", tmp + 0.0);
            }
           break;
  case 368:current_dataset = search_dataset(Datasets, params[0]);
           if (current_dataset)
            {
             read_and_normalize_train_file(current_dataset, &traindata);
             tmpd = minimum_epsilon_neighborhood(traindata);
             deallocate(traindata);
             traindata = NULL;
             exchange_classdefno(current_dataset);
             set_default_variable("out1", tmpd);
            }
           break;
  case 369:if (checkparams(paramcount, 3, m1400, m1401, m1032))
            {
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;
             bf = read_binary_function(params[0]);
             generate_random_data_from_binary_function(bf, tmp, params[2]);
             free_binary_function(bf);
            }
           break;
  case 370:if (checkparams(paramcount, 1, m1464))
             set_parameter_l("prunemodel", params[0], m1465);
           break;
  case 371:if (checkparams(paramcount, 4, m1400, m1401, m1163, m1032))
            {
             if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
               break;
             if (!atof_check(params[2], &tmpd, 0.0, 1.0))
               break;                        
             bf = read_binary_function(params[0]);
             generate_random_noisy_data_from_binary_function(bf, tmp, tmpd, params[3]);
             free_binary_function(bf);
            }
           break;
  case 372:if (checkparams(paramcount, 7, m1006, m1108, m1117, m1214, m1118, m1119, m1213))
            {
             if (!atoi_check(params[2], &tmp, 2, +INT_MAX))
               tmp = -1;
             create_sequential_dataset(params[0], params[1], tmp, params[3], params[4], params[5], params[6]);
            }
           break;
  case 373:if (checkparams(paramcount, 2, m1082, m1430))
            {
             if (atoi_check(params[1], &tmp, 2, +INT_MAX))
               plotgauss2d(params[0], tmp);
            }
           break;
  case 374:if (checkparams(paramcount, 1, m1082))
             performance_plot(params, paramcount, tpr_fpr_performance);
           break;
  case 375:if (checkparams(paramcount, 2, m1006, m1224))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               current_dataset = d;
               export_all_two_class_problems(d, params[1]);
              }
             else
               printf(m1006);
            }
           break;
  case 376:binary_comparison(hotellingt, params, paramcount);
           break;
  case 377:if (checkparams(paramcount, 1, m1222))
            {
             tmp = get_parameter_i("runcount") * get_parameter_i("foldcount");
             matptr = read_posteriors(tmp, params[0]);
             for (i = 0; i < tmp; i++)
               write_array_variables("aout", i + 1, 1, "ff", find_area_under_the_curve_2class(matptr[i], 0, 1, tpr_fpr_performance), find_area_under_the_curve_2class(matptr[i], 0, 1, precision_recall_performance));
            }
           break;
  case 378:run_supervised_algorithm(NO_META_ALGORITHM, GPROCESSREG);
           break;
  case 379:if (checkparams(paramcount, 2, m1222, m1409))
            {
             if (!atof_check(params[1], &tmpd, 0.0, 1.0))
               break;                        
             cm = construct_confusion_matrix_from_file(params[0], tmpd);
             tmp = get_parameter_i("runcount") * get_parameter_i("foldcount");
             for (i = 0; i < tmp; i++)
               write_array_variables("aout", i + 1, 1, "dddd", cm[i].true_positives, cm[i].false_positives, cm[i].true_negatives, cm[i].false_negatives);
             safe_free(cm);
            }
           break;
  case 380:if (checkparams(paramcount, 2, m1413, m1353))
            {
             matptr = read_matrix_without_header(params[0]);
             set_variable(params[1], matptr);
            }
           break;
  case 381:if (checkparams(paramcount, 1, m1353))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               w = matrix_average_columns(*(myvar->value.matrixvalue));
               for (i = 1; i < myvar->value.matrixvalue->col + 1; i++)
                 write_array_variables("aout", i, 1, "f", w[i - 1]);
               set_default_variable("out1", myvar->value.matrixvalue->col + 0.0);
               safe_free(w);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 382:if (checkparams(paramcount, 1, m1353))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
              {
               w = matrix_stdev_columns(*(myvar->value.matrixvalue));
               for (i = 1; i < myvar->value.matrixvalue->col + 1; i++)
                 write_array_variables("aout", i, 1, "f", w[i - 1]);
               set_default_variable("out1", myvar->value.matrixvalue->col + 0.0);
               safe_free(w);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 383:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("hyperparametertune", params[0]);
           break;
  case 384:if (checkparams(paramcount, 4, m1415, m1416, m1415, m1416))
            {
             atof_check(params[0], &(p1.x), -DBL_MAX, +DBL_MAX);
             atof_check(params[1], &(p1.y), -DBL_MAX, +DBL_MAX);
             atof_check(params[2], &(p2.x), -DBL_MAX, +DBL_MAX);
             atof_check(params[3], &(p2.y), -DBL_MAX, +DBL_MAX);
             if (paramcount == 5)
              {
               tmp = listindex(params[4], colors, COLORCOUNT);             
               if (tmp != -1)
                 p3.x = tmp;               
              }
             add_object(OBJECT_LINE, p1, p2, p3, "0");
             export_image();
            }
           break;
  case 385:if (checkparams(paramcount, 3, m1415, m1416, m1417))
            {
             atof_check(params[0], &(p1.x), -DBL_MAX, +DBL_MAX);
             atof_check(params[1], &(p1.y), -DBL_MAX, +DBL_MAX);
             if (paramcount >= 4)
              {
               tmp = listindex(params[3], colors, COLORCOUNT);             
               if (tmp != -1)
                 p3.x = tmp;               
              }
             else
               p3.x = 0.0;
             if (paramcount >= 5)
               atof_check(params[4], &(p3.y), 10.0, 72.0);
             else
               p3.y = 12.0;             
             add_object(OBJECT_STRING, p1, p2, p3, params[2]);
             export_image();
            }
           break;
  case 386:if (checkparams(paramcount, 4, m1216, m1146, m1147, m1418))
            {
             tmp5 = listindex(params[0], mlp_simulation_algorithms, MAX_MLP_SIMULATION_ALGORITHMS);
             if (!atoi_check(params[2], &tmp2, 0, +INT_MAX))
               break;                        
             if (!atoi_check(params[3], &tmp4, 0, +INT_MAX))
               break;                        
             hmmmodel = hmm_model_simulation(tmp5, params[1], tmp2, tmp4, &tmp3);
             set_default_variable("out1", hmmmodel.states + 0.0);
             set_default_variable("out2", hmmmodel.mixture + 0.0);
             set_default_variable("out3", tmp3 + 0.0);
            }
           break;
  case 387:if (checkparams(paramcount, 3, m1146, m1147, m1418))
            {
             if (!atoi_check(params[1], &tmp, 0, +INT_MAX))
               break;                        
             if (!atoi_check(params[2], &tmp2, 0, +INT_MAX))
               break;                        
             hmmmodel = compare_hmm_model_selection_techniques(params[0], tmp, tmp2);
             set_default_variable("out1", hmmmodel.states + 0.0);
             set_default_variable("out2", hmmmodel.mixture + 0.0);
            }
           break;
  case 388:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("runwithclassnoise", params[0]);
           break;
  case 389:if (checkparams(paramcount, 1, m1227))
            {
             models = load_model(params[0], &modelcount, &algorithm, &means, &variances);
             mustrealize = must_realize(algorithm);
             mustnormalize = must_normalize(algorithm);
             if (must_realize(algorithm))
               exchange_classdefno(current_dataset);             
             parameters = prepare_supervised_algorithm_parameters(algorithm);
             for (i = 0; i < modelcount; i++)
              {
               if (must_normalize(algorithm))
                {
                 switch (get_algorithm_type(algorithm))
                  {
                   case CLASSIFICATION:plot_classification_model(models[i], parameters, algorithm, means[i], variances[i]);
                                       break;
                   case REGRESSION    :plot_regression_model(models[i], parameters, algorithm, means[i], variances[i], outputmean, outputvariance);
                                       break;
                   default:
                           break;
                  }
                 safe_free(means[i].values);
                 safe_free(variances[i].values);
                }
               else
                 switch (get_algorithm_type(algorithm))
                  {
                   case CLASSIFICATION:plot_classification_model(models[i], parameters, algorithm, dummy, dummy);
                                       break;
                   case REGRESSION    :plot_regression_model(models[i], parameters, algorithm, dummy, dummy, outputmean, outputvariance);
                                       break;
                   default:
                           break;
                  }
               free_model_of_supervised_algorithm(algorithm, models[i], current_dataset);
              }
             free_supervised_algorithm_parameters(algorithm, parameters);             
             if (must_normalize(algorithm))
              {
               safe_free(means);
               safe_free(variances);
              }
            }
           break;
  case 390:if (checkparams(paramcount, 1, m1422))
             set_parameter_s("tempdirectory", params[0]);
           break;
  case 391:if (checkparams(paramcount, 1, m1424))
             set_parameter_l("multiclasstype", params[0], m1423);
           break;
  case 392:run_supervised_algorithm(NO_META_ALGORITHM, SVMREGTREE);
           break;
  case 393:if (checkparams(paramcount, 3, m1006, m1425, m1004))
            {
             atoi_check(params[1], &tmp, 2, +INT_MAX);
             current_dataset = search_dataset(Datasets, params[0]);
             if (current_dataset)
              {
               mustrealize = NO;
               d = discretize_dataset(current_dataset, tmp, get_parameter_l("discretization"));
               xmlfile = fopen(params[2], "w");
               fprintf(xmlfile, "<datasets>\n");
               save_definitions_of_dataset_as_xml(xmlfile, d);
               fprintf(xmlfile, "</datasets>\n");
               fclose(xmlfile);
              }
             else
               printf(m1006);
            }
           break;
  case 394:if (checkparams(paramcount, 3, m1415, m1416, m1427))
            {
             atof_check(params[0], &(p1.x), -DBL_MAX, +DBL_MAX);
             atof_check(params[1], &(p1.y), -DBL_MAX, +DBL_MAX);
             atof_check(params[2], &(p2.x), -DBL_MAX, +DBL_MAX);
             if (paramcount == 4)
              {
               tmp = listindex(params[3], colors, COLORCOUNT);             
               if (tmp != -1)
                 p3.x = tmp;               
              }
             add_object(OBJECT_CIRCLE, p1, p2, p3, "0");
             export_image();
            }
           break;
  case 395:check_and_set_list_variable(&(img.groupcoloring), m1431, m1432, group_colorings, GROUP_COLORING_COUNT);
           export_image();
           break;
  case 396:if (checkparams(paramcount, 1, m1433))
             set_parameter_i("featuresize", params[0], 1, +INT_MAX);
           break;
  case 397:if (checkparams(paramcount, 1, m1434))
             set_parameter_i("forestsize", params[0], 1, +INT_MAX);
           break;
  case 398:run_supervised_algorithm(NO_META_ALGORITHM, RANDOMFOREST);
           break;
  case 399:run_supervised_algorithm(NO_META_ALGORITHM, KTREE);
           break;
  case 400:if (checkparams(paramcount, 1, m1409))
             set_parameter_f("decisionthreshold", params[0], 0.0, 1.0);
           break;
  case 401:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("changedecisionthreshold", params[0]);
           break;
  case 402:if (checkparams(paramcount, 3, m1435, m1436, m1430))
            {
             if (atoi_check(params[2], &tmp, 2, +INT_MAX))
              {
               myvar = get_variable(params[0]);
               if (myvar && myvar->type == MATRIX_TYPE)
                {
                 myvar2 = get_variable(params[1]);
                 if (myvar2 && myvar2->type == MATRIX_TYPE)
                  {
                   if (!get_parameter_o("hold"))
                     clear_image();
                   plot_covariance(*(myvar->value.matrixvalue), *(myvar2->value.matrixvalue), tmp, BLACK, 0.0);
                   export_image();
                  }
                 else
                   printf(m1437, params[1]);
                }
               else
                 printf(m1437, params[0]);
              }
            }
           break;
  case 403:if (checkparams(paramcount, 2, m1036, m1035))
            {
             tmpd = friedman(params);
             nemenyi(params, 1 - get_parameter_f("confidencelevel"));
             if (tmpd < 1 - get_parameter_f("confidencelevel"))
               set_default_variable("out1", 0.0);
            }
           break;
  case 404:if (checkparams(paramcount, 2, m1439, m1440))
            {
             atof_check(params[0], &(tmpd), -DBL_MAX, +DBL_MAX);
             atof_check(params[1], &(tmpd2), -DBL_MAX, +DBL_MAX);
             if (tmpd < tmpd2)
               tmpf = produce_random_value(tmpd, tmpd2);
             else
               tmpf = produce_random_value(tmpd2, tmpd);
             set_default_variable("out1", tmpf);
            }
           break;
  case 405:multiple_tests(manova, params, paramcount);
           break;
  case 406:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("displayequalloss", params[0]);
           break;   
  case 407:if (checkparams(paramcount, 2, m1441, m1442))
            {
             atoi_check(params[0], &tmp, 0, +INT_MAX);
             atoi_check(params[1], &tmp2, 0, +INT_MAX);
             tmpf = binomdouble(tmp, tmp2);
             set_default_variable("out1", tmpf);
            }
           break; 
  case 408:binary_comparison(waldwolfowitz, params, paramcount);
           break;
  case 409:binary_comparison(smirnov, params, paramcount);
           break;   
  case 410:if (checkparams(paramcount, 2, m1444, m1445))
            {
             atoi_check(params[1], &tmp, 0, +INT_MAX);
             histogram_plot(params[0], tmp);
            }
           break;
  case 411:run_supervised_algorithm(NO_META_ALGORITHM, RISE);
           break;
  case 412:run_supervised_algorithm(NO_META_ALGORITHM, DIVS);
           break;
  case 413:if (checkparams(paramcount, 1, m1446))
             set_parameter_i("beamsize", params[0], 1, +INT_MAX);
           break;   
  case 414:run_supervised_algorithm(NO_META_ALGORITHM, CN2);
           break;
  case 415:run_supervised_algorithm(NO_META_ALGORITHM, LERILS);
           break;
  case 416:if (checkparams(paramcount, 1, m1447))
             set_parameter_f("mutationprobability", params[0], 0.0, 1.0);
           break;   
  case 417:if (checkparams(paramcount, 1, m1448))
             set_parameter_i("populationsize", params[0], 1, +INT_MAX);
           break;   
  case 418:if (checkparams(paramcount, 1, m1449))
             set_parameter_i("restarts", params[0], 1, +INT_MAX);
           break;  
  case 419:multiple_tests(kruskalwallis_multivariate, params, paramcount);
           break;   
  case 420:if (checkparams(paramcount, 1, m1060))
             set_default_variable("out1", multivariate_normality_test(params[0], 1 - get_parameter_f("confidencelevel"), &tmpd));
           break;   
  case 421:run_supervised_algorithm(NO_META_ALGORITHM, PART);
           break;
  case 422:if (checkparams(paramcount, 1, m1452))
            {
             lpprogram = read_linear_program(params[0]);
             if (lpprogram)
              {
               solution = solve_lp(lpprogram);
               if (solution)
                {
                 set_default_variable("out1", solution->optimalvalue);
                 set_default_variable("out2", solution->solutiontype);
                 for (i = 0; i < solution->variablecount; i++)
                   write_array_variables("aout", i + 1, 1, "f", solution->variables[i]);               
                 free_convex_solution(solution);
                }
               else
                 printf("No solution\n");
               free_linear_program(lpprogram);
              }
            }
           break;
  case 423:if (checkparams(paramcount, 2, m1453, m1141))
            {
             for (i = 0; i < strlen(params[0]) - 1; i++)
              {
               swap_char(&(params[0][i]), &(params[0][i + 1]));
               sprintf(tmpst3, "%s[%d]", params[1], i + 1);
               set_variable(tmpst3, params[0]);
               swap_char(&(params[0][i]), &(params[0][i + 1]));
              }
            }
           break;
  case 424:if (checkparams(paramcount, 1, m1082))
             switch (get_parameter_l("imagetype"))
              {
                    case IMAGE_EPS:rank_plot(params, paramcount);
                                   break;
               case IMAGE_PSTRICKS:rank_plot_pstricks(params, paramcount);
                                   break;
              }
           break;   
  case 425:if (checkparams(paramcount, 2, m1035, m1035))
             find_cliques(params, paramcount);
           break;
  case 426:if (checkparams(paramcount, 1, m1455))
             set_parameter_l("normalization", params[0], m1456);
            break;   
  case 427:if (checkparams(paramcount, 1, m1457))
             set_parameter_l("hingelosstype", params[0], m1458);
           break;   
  case 428:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("normalizekernel", params[0]);
           break;   
  case 429:binary_comparison(signed_rank_test, params, paramcount);
           break;
  case 430:run_supervised_algorithm(NO_META_ALGORITHM, SOFTREGTREE);
           break;
  case 431:if (checkparams(paramcount, 1, m1227))
            {
             models = load_model(params[0], &modelcount, &algorithm, &means, &variances);
             for (i = 0; i < modelcount; i++)
              {
               switch (algorithm)
                {
                 case         C45:
                 case         LDT:
                 case    MULTILDT:
                 case     OMNILDT:
                 case     SVMTREE:
                 case      NBTREE:
                 case    SOFTTREE:tree = (Decisionnodeptr)models[i];
                                  plot_decision_tree(tree);
                                  break;
                 case     REGTREE:
                 case  SVMREGTREE:
                 case SOFTREGTREE:regtree = (Regressionnodeptr)models[i];
                                  mean = means[i];
                                  variance = variances[i];
                                  plot_regression_tree(regtree, algorithm, outputmean, outputvariance);
                                  break;
                }
               free_model_of_supervised_algorithm(algorithm, models[i], current_dataset);
              }
            }
           break;
  case 432:if (checkparams(paramcount, 1, m1227))
            {
             models = load_model(params[0], &modelcount, &algorithm, &means, &variances);
             for (i = 0; i < modelcount; i++)
              {
               if (algorithm == SOFTREGTREE)
                {
                 regtree = (Regressionnodeptr)models[i];
                 mean = means[i];
                 variance = variances[i];
                 plot_regression_model_tree(regtree, outputmean, outputvariance);
                 break;
                }
               else
                 if (algorithm == SOFTTREE)
                  {
                   tree = (Decisionnodeptr)models[i];
                   mean = means[i];
                   variance = variances[i];
                   plot_model_tree(tree);
                   break;
                  }
               free_model_of_supervised_algorithm(algorithm, models[i], current_dataset);
              }
            }
           break;
  case 433:run_supervised_algorithm(NO_META_ALGORITHM, SOFTTREE);
           break;
  case 434:if (checkparams(paramcount, 1, m1459))
             set_parameter_l("discretization", params[0], m1460);
           break;   
  case 435:run_supervised_algorithm(NO_META_ALGORITHM, CART);
           break;
  case 436:if (checkparams(paramcount, 1, m1324))
             set_parameter_i("hybridspace", params[0], 1, 4);
           break;   
  case 437:if (checkparams(paramcount, 2, m1461, m1462))
             difference_plot(params, 2);
           break; 
  case 438:if (checkparams(paramcount, 1, m1043))
             set_parameter_o("showrundetails", params[0]);
           break;      
  case 439:if (checkparams(paramcount, 4, m1006, m1369, m1370, m1037))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               current_dataset = d;
               if (!atoi_check(params[1], &tmp, 1, +INT_MAX))
                 break;
               read_from_data_file(d, params[2], &traindata);
               outfile = fopen(params[3], "w");
               if (!outfile)
                 return;
               print_instance_list_with_extra_features(outfile, d->separator, d, traindata, tmp);
               fclose(outfile);
               deallocate(traindata);
               traindata = NULL;
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 440:if (checkparams(paramcount, 1, m1466))
             set_parameter_l("regularization", params[0], m1467);
           break;   
  case 441:if (checkparams(paramcount, 3, m1006, m1473, m1468))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               current_dataset = d;
               read_and_normalize_train_file(d, &traindata);
               outfile = fopen(params[1], "w");
               if (!outfile)
                 return;
               print_class_information(outfile, traindata);
               fclose(outfile);
               outfile = fopen(params[2], "w");
               if (!outfile)
                 return;
               print_kernel_matrix(outfile, traindata);
               fclose(outfile);
               deallocate(traindata);
               traindata = NULL;
              }
             else
               printf(m1279, params[0]);
            }
           break;  
  case 442:if (checkparams(paramcount, 4, m1006, m1214, m1118, m1119, m1474))
             create_kernel_dataset(params[0], params[1], params[2], params[3], params[4]);
           break;
  case 443:binary_comparison(permutation_test, params, paramcount);
           break;
  case 444:binary_comparison(permutation_test_paired, params, paramcount);
           break;
  case 445:run_supervised_algorithm(NO_META_ALGORITHM, KFOREST);
           break;
  case 446:if (checkparams(paramcount, 1, m1477))
             set_parameter_l("imagetype", params[0], m1478);
           break;
  case 447:if (checkparams(paramcount, 1, m1006))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               for (i = 0; i < d->classno; i++)
                 write_array_variables("aout", i + 1, 1, "d", d->classcounts[i]);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 448:if (checkparams(paramcount, 1, m1479))
            {
             qpprogram = read_quadratic_program(params[0]);
             if (qpprogram)
              {
               solution = solve_qp_wolfe_method(qpprogram);
               set_default_variable("out1", solution->optimalvalue);
               set_default_variable("out2", solution->solutiontype);
               for (i = 0; i < solution->variablecount; i++)
                 write_array_variables("aout", i + 1, 1, "f", solution->variables[i]);
               free_quadratic_program(qpprogram);
               free_convex_solution(solution);
              }
            }
           break;
  case 449:if (checkparams(paramcount, 1, m1480))
             set_parameter_i("kernelindex", params[0], 0, MAXKERNELS - 1);
           break;
  case 450:if (checkparams(paramcount, 1, m1353))
            {
             myvar = get_variable(params[0]);
             if (myvar && myvar->type == MATRIX_TYPE)
               matrix_inverse(myvar->value.matrixvalue);
             else
               printf(m1437, params[0]);
            }
           break;
  case 451:if (checkparams(paramcount, 6, m1006, m1108, m1481, m1118, m1119, m1213))
            {
             if (!atoi_check(params[2], &tmp, 0, +INT_MAX))
               tmp = -1;
             create_multi_label_dataset(params[0], params[1], tmp, params[3], params[4], params[5]);
            }
           break;
  case 452:run_supervised_algorithm(ONE_VERSUS_ONE, SELECTMAX + get_parameter_l("baseclassificationalgorithm"));
           break;
  case 453:run_supervised_algorithm(ONE_VERSUS_REST, SELECTMAX + get_parameter_l("baseclassificationalgorithm"));
           break;
  case 454:if (checkparams(paramcount, 1, m1320))
             set_parameter_l("baseclassificationalgorithm", params[0], m1482);
           break;
  case 455:if (checkparams(paramcount, 1, m1483))
             set_parameter_i("weaklearnercount", params[0], 0, INT_MAX);
           break;
  case 456:run_supervised_algorithm(BAGGING_CLASSIFICATION, SELECTMAX + get_parameter_l("baseclassificationalgorithm"));
           break;
  case 457:run_supervised_algorithm(BAGGING_REGRESSION, ONEFEATURE + get_parameter_l("baseregressionalgorithm"));
           break;
  case 458:if (checkparams(paramcount, 1, m1320))
             set_parameter_l("baseregressionalgorithm", params[0], m1482);
           break;
  case 459:if (checkparams(paramcount, 4, m1006, m1049, m1473, m1468))
             autoencoder();
           break;
  case 460:if (checkparams(paramcount, 1, m1006))
            {
             d = search_dataset(Datasets, params[0]);
             if (d)
              {
               strcpy(tmpst3, d->classes[0]);
               for (i = 1; i < d->classno; i++)
                 sprintf(tmpst3, "%s;%s", tmpst3, d->classes[i]);
               set_default_string_variable("sout1", tmpst3);
              }
             else
               printf(m1279, params[0]);
            }
           break;
  case 461:if (checkparams(paramcount, 1, m1103))
             set_parameter_i("runfromfold", params[0], 0, get_parameter_i("foldcount") - 1);
           break;
  case 462:if (checkparams(paramcount, 2, m1353, m1353))
            {
             myvar = get_variable(params[0]);
             myvar2 = get_variable(params[1]);
             if (myvar && myvar->type == MATRIX_TYPE && myvar2 && myvar2->type == MATRIX_TYPE)
              {
               w = matrix_max_rows(*(myvar->value.matrixvalue));
               for (i = 0; i < myvar->value.matrixvalue->row; i++)
                 myvar2->value.matrixvalue->values[i][0] = w[i];
               safe_free(w);
              }
             else
               printf(m1437, params[0]);
            }
           break;
  case 463:if (checkparams(paramcount, 2, m1353, m1353))
            {
             myvar = get_variable(params[0]);
             myvar2 = get_variable(params[1]);
             if (myvar && myvar->type == MATRIX_TYPE && myvar2 && myvar2->type == MATRIX_TYPE)
              {
               tmp = 0;
               for (i = 0; i < myvar->value.matrixvalue->row; i++)
                 for (j = 0; j < myvar->value.matrixvalue->col; j++)
                   if (myvar->value.matrixvalue->values[i][j] == myvar2->value.matrixvalue->values[i][j])
                     tmp++;
               set_default_variable("out1", tmp);
              }
             else
               printf(m1437, params[0]);
            }
            break;
   case 464:if (checkparams(paramcount, 3, m1141, m1488, m1485))
             {
              myvar = get_variable(params[0]);
              if (myvar && myvar->type == ARRAY_TYPE && myvar->value.array[0].type == STRING_TYPE)
               {
                set_default_variable("out1", -1);
                for (i = 0; i < atoi(params[1]); i++)
                 {
                  if (myvar->value.array[i].value.stringvalue != NULL && strcmp(myvar->value.array[i].value.stringvalue, params[2]) == 0)
                   {
                    set_default_variable("out1", i + 1);
                    break;
                   }
                 }
               }
              else
                printf(m1486, params[0]);
             }
            break;
   case 465:if (checkparams(paramcount, 3, m1141, m1488, m1487))
             {
              myvar = get_variable(params[0]);
              if (myvar && myvar->type == ARRAY_TYPE && myvar->value.array[0].type == STRING_TYPE)
               {
                for (i = atoi(params[2]) - 1; i < atoi(params[1]) - 1; i++)
                 {
                  safe_free(myvar->value.array[i].value.stringvalue);
                  myvar->value.array[i].value.stringvalue = strcopy(myvar->value.array[i].value.stringvalue, myvar->value.array[i + 1].value.stringvalue);
                 }
               }
              else
                printf(m1486, params[0]);
             }
            break;
   case 466:if (checkparams(paramcount, 4, m1141, m1488, m1487, m1453))
             {
              myvar = get_variable(params[0]);
              if (myvar && myvar->type == ARRAY_TYPE && myvar->value.array[0].type == STRING_TYPE)
               {
                for (i = atoi(params[1]) - 1; i > atoi(params[2]) - 1; i--)
                 {
                  safe_free(myvar->value.array[i].value.stringvalue);
                  myvar->value.array[i].value.stringvalue = strcopy(myvar->value.array[i].value.stringvalue, myvar->value.array[i - 1].value.stringvalue);
                 }
                myvar->value.array[atoi(params[2]) - 1].value.stringvalue = strcopy(myvar->value.array[atoi(params[2]) - 1].value.stringvalue, params[3]);
               }
              else
                printf(m1486, params[0]);
             }
            break;
   case 467:if (checkparams(paramcount, 2, m1141, m1488))
             {
              myvar = get_variable(params[0]);
              if (myvar && myvar->type == ARRAY_TYPE && myvar->value.array[0].type == STRING_TYPE)
               {
                learnercount = atoi(params[1]);
                foldcount = get_parameter_i("runcount") * get_parameter_i("foldcount");
                posteriors = safemalloc(learnercount * sizeof(matrixptr), "process.c", 3560);
                for (i = 0; i < learnercount; i++)
                  posteriors[i] = read_posteriors(foldcount, myvar->value.array[i].value.stringvalue);
                results = ensemble(posteriors, learnercount, foldcount);
                tmpd = 0;
                for (i = 0; i < foldcount; i++)
                 {
                  tmpd += 100 * (1 - results[i].performance / (results[i].datasize + 0.0));
                  free_2d((void **) results[i].confusion_matrix, results[i].classno);
                  safe_free(results[i].class_performance);
                  safe_free(results[i].class_counts);
                 }
                safe_free(results);
                for (i = 0; i < learnercount; i++)
                 {
                  for (j = 0; j < foldcount; j++)
                    matrix_free(posteriors[i][j]);
                  safe_free(posteriors[i]);
                 }
                safe_free(posteriors);
                set_default_variable("out1", tmpd / foldcount);
               }
              else
                printf(m1486, params[0]);
             }
            break;
   case 468:if (checkparams(paramcount, 2, m1141, m1488))
             {
              myvar = get_variable(params[0]);
              if (myvar && myvar->type == ARRAY_TYPE && myvar->value.array[0].type == STRING_TYPE)
               {
                for (i = 0; i < atoi(params[1]); i++)
                  for (j = i + 1; j < atoi(params[1]); j++)
                    if (strcmp(myvar->value.array[i].value.stringvalue, myvar->value.array[j].value.stringvalue) > 0)
                     {
                      strcpy(tmpst2, myvar->value.array[i].value.stringvalue);
                      safe_free(myvar->value.array[i].value.stringvalue);
                      myvar->value.array[i].value.stringvalue = strcopy(myvar->value.array[i].value.stringvalue, myvar->value.array[j].value.stringvalue);
                      safe_free(myvar->value.array[j].value.stringvalue);
                      myvar->value.array[j].value.stringvalue = strcopy(myvar->value.array[j].value.stringvalue, tmpst2);
                     }
               }
              else
                printf(m1486, params[0]);
             }
            break;
   case 469:if (checkparams(paramcount, 2, m1141, m1488))
             {
              myvar = get_variable(params[0]);
              if (myvar && myvar->type == ARRAY_TYPE && myvar->value.array[0].type == STRING_TYPE)
               {
                strcpy(tmpst2, "");
                for (i = 0; i < atoi(params[1]); i++)
                  strcat(tmpst2, myvar->value.array[i].value.stringvalue);
                set_default_string_variable("sout1", tmpst2);
               }
              else
                printf(m1486, params[0]);
             }
            break;
  case  470:if (checkparams(paramcount, 2, m1490, m1490))
             {
              i = 0;
              while (i < strlen(params[0]) && i < strlen(params[1]) && params[0][i] == params[1][i])
                i++;
              set_default_variable("out1", i);
             }
            break;
 }
 if (get_parameter_o("timeon"))
  {
   tend = clock();
   print_time((tend- tstart) / CLOCKS_PER_SEC);
  }
}
