#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "distributions.h"
#include "instance.h"
#include "instancelist.h"
#include "interval.h"
#include "lang.h"
#include "libarray.h"
#include "libfile.h"
#include "libiteration.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "libstring.h"
#include "messages.h"
#include "mlp.h"
#include "parameter.h"
#include "process.h"
#include "statistics.h"
#include "univariatetree.h"
#include "xmlparser.h"

#ifdef ODBC
#include <sqlext.h>
extern HENV henv;         /* Environment */
extern HDBC hdbc;         /* Connection handle */
extern HSTMT hstmt;       /* Statement handle */
extern unsigned char databuffer[SQL_MAX_MESSAGE_LENGTH + 1];
#endif

char *ftype[3] = {"Int","String","double"};
char *featuretype[4] = {"Integer", "String", "Real", "Output"};

extern Datasetptr Datasets;
extern char *datadir;
extern FILE* output;

/**
 * Exchange the class definition no with the previously stored class definition no. This function is mainly used before and after realization of the discrete features. If the discrete features are converted to continuous features, total number of features change and respectively the position of the class definition feature may change. classdefno and newclassdefno will store the position of the class definition feature before and after realization.
 * @param[in] d Dataset for which the classdefno will be exchanged
 */
void exchange_classdefno(Datasetptr d)
{
 /*!Last Changed 01.09.2005*/
 swap_int(&(d->classdefno), &(d->newclassdefno));
}

/**
 * Export a link list of instances to an output file such that, only a subset of features are exported. The subset is produced randomly. No matter what, the class definition feature will be exported. Uses print_instance_list to export the data.
 * @param[in] d Pointer to the dataset
 * @param[in] list Header of the link list of instances
 * @param[in] featurecount Number of features in the subset
 * @param[in] outfilename Name of the output file
 */
void export_subset_of_features(Datasetptr d, Instanceptr list, int featurecount, char* outfilename)
{
 /*!Last Changed 14.03.2007*/
 int bound, *selected, i;
 FILE* outfile;
 outfile = fopen(outfilename, "w");
 if (!outfile)
   return;
 selected = random_array(d->featurecount);
 /* If the class definition feature is in the selected feature list, increment bound so that exactly featurecount features will be selected*/
 if (selected[d->classdefno] < featurecount)
   bound = featurecount + 1;
 else
   bound = featurecount;
 for (i = 0; i < d->featurecount; i++)
   if (i != d->classdefno)
    {
     if (selected[i] < bound)
       d->features[i].selected = YES;
     else
       d->features[i].selected = NO;
    }
   else
     d->features[i].selected = YES;
 print_instance_list(outfile, d->separator, d, list);
 fclose(outfile);
 for (i = 0; i < d->featurecount; i++)
   d->features[i].selected = YES;
}

Datasetptr copy_of_dataset(Datasetptr d)
{
 int i, j;
 Datasetptr tmp;
 tmp = safemalloc(sizeof(Dataset), "copy_of_dataset", 2);
 tmp->traincount = d->traincount;
 tmp->storetype = d->storetype;
 tmp->sigma = d->sigma;
 tmp->separator = d->separator;
 tmp->newclassdefno = d->newclassdefno;
 tmp->name = strcopy(tmp->name, d->name);
 tmp->multifeaturecount = d->multifeaturecount;
 tmp->file = strcopy(tmp->file, d->file);
 tmp->featurecount = d->featurecount;
 tmp->datacount = d->datacount;
 tmp->classno = d->classno;
 tmp->classdefno = d->classdefno;
 tmp->baseerror = d->baseerror;
 tmp->allfeatures = d->allfeatures;
 tmp->active = d->active;
 for (i = 0; i < d->featurecount; i++)
  {
   tmp->selected[i] = d->selected[i];
   tmp->availability[i] = d->availability[i];
   tmp->features[i].max = d->features[i].max;
   tmp->features[i].min = d->features[i].min;
   tmp->features[i].oldtype = d->features[i].oldtype;
   tmp->features[i].type = d->features[i].type;
   tmp->features[i].valuecount = d->features[i].valuecount;
   for (j = 0; j < d->features[i].valuecount; j++)
     tmp->features[i].strings[j] = strcopy(tmp->features[i].strings[j], d->features[i].strings[j]);
  }
 for (i = 0; i < d->classno; i++)
  {
   tmp->classes[i] = strcopy(tmp->classes[i], d->classes[i]);
   tmp->classcounts[i] = d->classcounts[i];
  }
 return tmp;
}

void subsample_regression(Datasetptr d, char* outputfile, Instanceptr list, int datacount)
{
 FILE* outfile;
 int i, *array;
 Instanceptr tmp;
 outfile = fopen(outputfile, "w");
 if (!outfile)
   return;
	array = random_array(data_size(list));
 for (i = 0; i < datacount; i++)
  {
   tmp = data_x(list, array[i]);
   print_instance(outfile, d->separator, d, tmp);
  }
 fclose(outfile);
 safe_free(array);
}

/**
 * Generates a subsample of the dataset (with replacement) with the given weights of the classes. \n
 * @param[in] d Dataset
 * @param[in] outputfile Name of the output file
 * @param[in] list Header of the link list of instances in the dataset
 * @param[in] datacount Number of instances in the subsample
 * @param[in] weights Weights of the classes. weights[i] is the weight of class i.
 * @pre Weights array must have been filled before
 */
void subsample(Datasetptr d, char* outputfile, Instanceptr list, int datacount, double* weights)
{
 /*!Last Changed 21.05.2005*/
 FILE* outfile;
 int i, j, classno, count, **indexes, *currentindex, *listindex;
 double sum = 0;
 Instanceptr tmp;
 outfile = fopen(outputfile, "w");
 if (!outfile)
   return;
 indexes = safemalloc(d->classno * sizeof(int *), "subsample", 6);
 currentindex = safecalloc(d->classno, sizeof(int), "subsample", 7);
 listindex = safecalloc(d->classno, sizeof(int), "subsample", 8);
 for (i = 0; i < d->classno; i++)
  {
   weights[i] *= d->classcounts[i];
   sum += weights[i];
  }
 for (i = 0; i < d->classno; i++)
  {
   count = (int) ((weights[i] * datacount) / sum);
   indexes[i] = safemalloc(count * sizeof(int), "subsample", 19);
   for (j = 0; j < count; j++)
     indexes[i][j] = myrand() % d->classcounts[i];
   qsort(indexes[i], count, sizeof(int), sort_function_int_ascending);
  }
 tmp = list;
 while (tmp)
  {
   classno = give_classno(tmp);
   while (indexes[classno][currentindex[classno]] == listindex[classno])
    {
     print_instance(outfile, d->separator, d, tmp);
     (currentindex[classno])++;
    }
   listindex[classno]++;
   tmp = tmp->next;
  }
 fclose(outfile);
 safe_free(currentindex);
 safe_free(listindex);
 free_2d((void**)indexes, d->classno);
}

/**
 * Converts a dataset in the byte format to a regular dataset in the ISELL format. There are three format definition files for the byte format. The first one stores the types of the features one line each. The second one stores the lengths (in bytes) of each feature one line each. The third one stores the indexes of features those will be exported.
 * @param[in] inputfile Name of the data file in binary format
 * @param[in] separator Separator character for the output dataset
 * @param[in] featuretypes Name of the file that contains the types of features
 * @param[in] lengths Name of the file that contains the sizes of features (in bytes)
 * @param[in] useful Name of the file that contain the indexes of the exported features
 * @param[in] outputfile Name of the output data file in ISELL format
 */
void convert_byte_format_to_dataset_format(char* inputfile, char separator, char* featuretypes, char* lengths, char* useful, char* outputfile)
{
 /*!Last Changed 20.05.2005*/
 FILE *infile1, *infile2, *outfile;
 char myline[READLINELENGTH], feature[READLINELENGTH], pst[READLINELENGTH];
 int *ftypes, fcount, i, *fsizes, *available, *fstart, tmp;
 double tmpr;
 infile1 = fopen(featuretypes, "r");
 if (!infile1)
   return;
 infile2 = fopen(lengths, "r");
 if (!infile2)
   return;
 fcount = file_length(featuretypes);
 ftypes = safemalloc(fcount * sizeof(int), "convert_byte_format_to_dataset_format", 6);
 fsizes = safemalloc(fcount * sizeof(int), "convert_byte_format_to_dataset_format", 7);
 fstart = safemalloc(fcount * sizeof(int), "convert_byte_format_to_dataset_format", 8);
 available = safecalloc(fcount, sizeof(int), "convert_byte_format_to_dataset_format", 9);
 i = 0;
 fstart[0] = 0;
 while (fgets(myline, READLINELENGTH, infile1))
  {
   tmp = strlen(myline);
   myline[tmp - 1] = '\0';
   ftypes[i] = listindex(myline, ftype, 3);
   fgets(myline, READLINELENGTH, infile2);
   fsizes[i] = atoi(myline);
   if (i > 0)
     fstart[i] = fstart[i - 1] + fsizes[i - 1];
   i++;
  }
 fclose(infile1);
 fclose(infile2);
 infile1 = fopen(useful, "r");
 if (!infile1)
   return;
 while (fgets(myline, READLINELENGTH, infile1))
  {
   i = atoi(myline);
   available[i] = 1;
  }
 fclose(infile1);
 infile1 = fopen(inputfile, "r");
 if (!infile1)
   return;
 outfile = fopen(outputfile, "w");
 if (!outfile)
  {
   printf(m1248, outputfile);
   return;
  }
 while (fgets(myline, READLINELENGTH, infile1))
  {
   for (i = 0; i < fcount; i++)
     if (available[i])
      {
       memcpy(feature, &(myline[fstart[i]]), fsizes[i]);
       feature[fsizes[i]] = '\0';
       switch (ftypes[i])
        {
         case 0 :tmp = atoi(feature);
                 fprintf(outfile, "%d%c", tmp, separator);
                 break;
         case 1 :fprintf(outfile, "%s%c", feature, separator);
                 break;
         case 2 :tmpr = atof(feature);
                 set_precision(pst, NULL, "%c");
                 fprintf(outfile, pst, tmpr, separator);
                 break;
         default:printf(m1236, ftypes[i]);
                 break;
        }
      }
   fprintf(outfile, "\n");
  }
 fclose(infile1);
 fclose(outfile);
 safe_free(ftypes);
 safe_free(fsizes);
 safe_free(available);
 safe_free(fstart);
}

/**
 * Number of free parameters of quadratic multivariate regression problem for this dataset. The weights are \n
 * (i) Quadratic: x_ix_j (d (d + 1) / 2 parameters) \n
 * (ii) Linear: x_i (d parameters) \n
 * (iii) Bias: x_0 (1 parameter)
 * @param[in] d The dataset
 * @return Number of free parameters
 */
int multifeaturecount_2d(Datasetptr d)
{
 /*!Last Changed 01.02.2005*/
 int dim;
 dim = d->multifeaturecount - 1;
 return (dim * (dim + 3)) / 2;
}

/**
 * Frees memory allocated for all loaded datasets.
 */
void deallocate_datasets()
{
 /*!Last Changed 29.11.2004*/
 Datasetptr tmp, tmp1;
 tmp = Datasets;
 while (tmp)
  {   
   tmp1 = tmp->next;
   free_dataset(tmp);
   tmp = tmp1;
  }
}

/**
 * Produces a noisy dataset from a dataset sample where the classes of the original data is flipped with a given noise level
 * @param[in] d Pointer to the dataset 
 * @param[in] inputfile Name of the file that contains the sample
 * @param[in] outputfile Name of the file that will contain the output sample
 */
void generate_data_with_noisy_classes_from_a_data_sample(Datasetptr d, char* inputfile, char *outputfile)
{
 /*!Last Changed 20.03.2009*/
 Instanceptr whereto, inst;
 FILE* outfile;
 double noiselevel = get_parameter_f ("noiselevel"), probability;
 int classno;
 read_from_data_file(d, inputfile, &whereto);
 inst = whereto;
 while (inst)
  {
   probability = produce_random_value (0.0, 1.0);
   classno = give_classno(inst);
   if (probability < noiselevel)
     do{
       inst->values[d->classdefno].stringindex = myrand() % d->classno;
     }while (inst->values[d->classdefno].stringindex == classno);
   inst = inst->next;
  }
 outfile = fopen(outputfile, "w");
 print_instance_list(outfile, d->separator, d, whereto);
 deallocate(whereto);
 fclose(outfile);
}

/**
 * Generates random noisy data from a dataset
 * @param[in] d Dataset
 * @param[in] data Data list before noise is applied
 * @param[in] fname Output file name to store random noisy data
 * @param[in] stdev Standard deviation of the 0-mean gaussian noise
 * @param[in] percentage Noise percentage (Real number between 0 and 1)
 */
void generate_data_with_noisy_features_from_a_dataset(Datasetptr d, Instanceptr data, char* fname, double stdev, double percentage)
{
 /*!Last Changed 13.10.2004*/
 FILE* outfile;
 int i;
 Instanceptr tmp;
 double value;
 outfile = fopen(fname, "w");
 if (!outfile)
  {
   printf(m1248, fname);
   return;
  }
 tmp = data;
 while (tmp)
  {
   for (i = 0; i < d->featurecount; i++)
     switch (d->features[i].type)
      {
       case INTEGER:if (produce_random_value(0, 1) < percentage)
                     {
                      value = tmp->values[i].intvalue + produce_normal_value(0, stdev);
                      tmp->values[i].intvalue = (int) value;
                     }
                    break;
       case    REAL:if (produce_random_value(0, 1) < percentage)
                     {
                      value = tmp->values[i].floatvalue + produce_normal_value(0, stdev);
                      tmp->values[i].floatvalue = (double) value;
                     } 
                    break;
      }
   tmp = tmp->next;
  }
 print_instance_list(outfile, ' ', d, data);
 fclose(outfile);
}

void k_way_anova_dataset_cont(Datasetptr d, Instanceptr data, int* features, int fdependent)
{
 /*!Last Changed 07.05.2004 only for two way currently*/
 Instanceptr tmp;
 int rowsize, colsize, i, j, row, col, *rowcounts, *colcounts, **cellcounts, datasize;
 double grandmean = 0, value, criticalvalue;
 double** cellmeans;
 double* rowmeans;
 double* colmeans, sstotal = 0.0, sswithin = 0.0, sscolumns = 0.0, ssrows = 0.0;
 double mscolumn, msrow, msinteraction, mswithin;
 double columnconf, rowconf, interactionconf;
 datasize = data_size(data);
 rowsize = d->features[features[0]].valuecount;
 colsize = d->features[features[1]].valuecount;
 cellmeans = (double**) safecalloc_2d(sizeof(double *), sizeof(double), rowsize, colsize, "k_way_anova_dataset_cont", 9);
 cellcounts = (int**) safecalloc_2d(sizeof(int *), sizeof(int), rowsize, colsize, "k_way_anova_dataset_cont", 10);
 rowmeans = safecalloc(rowsize, sizeof(double), "k_way_anova_dataset_cont", 14);
 colmeans = safecalloc(colsize, sizeof(double), "k_way_anova_dataset_cont", 15);
 rowcounts = safecalloc(rowsize, sizeof(double), "k_way_anova_dataset_cont", 16);
 colcounts = safecalloc(colsize, sizeof(double), "k_way_anova_dataset_cont", 17);
 tmp = data;
 while (tmp)
  {
   switch (d->features[fdependent].type)
    {
     case INTEGER:value = tmp->values[fdependent].intvalue;
                  break;
     case    REAL:
     case  REGDEF:value = tmp->values[fdependent].floatvalue;
                  break;
     default     :value = 0;
                  printf(m1226);
                  break;
    }
   row = tmp->values[features[0]].stringindex;
   col = tmp->values[features[1]].stringindex;
   grandmean += value;
   rowmeans[row] += value;
   rowcounts[row]++;
   colmeans[col] += value;
   colcounts[col]++;
   cellmeans[row][col] += value;
   cellcounts[row][col]++;
   tmp = tmp->next;
  }
 check_and_set_mean(&grandmean, datasize);
 for (i = 0; i < rowsize; i++)
   check_and_set_mean(&(rowmeans[i]), rowcounts[i]);
 for (i = 0; i < colsize; i++)
   check_and_set_mean(&(colmeans[i]), colcounts[i]);
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     check_and_set_mean(&(cellmeans[i][j]), cellcounts[i][j]);
 tmp = data;
 while (tmp)
  {
   switch (d->features[fdependent].type)
    {
     case INTEGER:value = tmp->values[fdependent].intvalue;
                  break;
     case    REAL:
     case  REGDEF:value = tmp->values[fdependent].floatvalue;
                  break;
     default     :value = 0;
                  printf(m1226);
                  break;
    }   
   row = tmp->values[features[0]].stringindex;
   col = tmp->values[features[1]].stringindex;
   sstotal += (value - grandmean) * (value - grandmean);
   sswithin += (value - cellmeans[row][col]) * (value - cellmeans[row][col]);
   tmp = tmp->next;
  }
 for (i = 0; i < rowsize; i++)
   ssrows += rowcounts[i] * (rowmeans[i] - grandmean) * (rowmeans[i] - grandmean);
 for (i = 0; i < colsize; i++)
   sscolumns += colcounts[i] * (colmeans[i] - grandmean) * (colmeans[i] - grandmean);
 mscolumn = sscolumns / (colsize - 1);
 msrow = ssrows / (rowsize - 1);
 msinteraction = (sstotal - sscolumns - ssrows - sswithin) / ((colsize - 1) * (rowsize - 1));
 mswithin = sswithin / (datasize - rowsize * colsize);
 criticalvalue = msrow / mswithin;
 rowconf = f_distribution(criticalvalue, rowsize - 1, datasize - rowsize * colsize);
 criticalvalue = mscolumn / mswithin;
 columnconf = f_distribution(criticalvalue, colsize - 1, datasize - rowsize * colsize);
 criticalvalue = msinteraction / mswithin;
 interactionconf = f_distribution(criticalvalue, (colsize - 1) * (rowsize - 1), datasize - rowsize * colsize);
 if (get_parameter_o("displayresulton"))
  {
   if (rowconf < get_parameter_f("confidencelevel"))
     printf(m1249, features[0] + 1);
   else
     printf(m1250, features[0] + 1);
   if (columnconf < get_parameter_f("confidencelevel"))
     printf(m1249, features[1] + 1);
   else
     printf(m1250, features[1] + 1);
   if (interactionconf < get_parameter_f("confidencelevel"))
     printf(m1251, features[0] + 1, features[1] + 1);
   else
     printf(m1252, features[0] + 1, features[1] + 1);
  }
 set_default_variable("out1", rowconf);
 set_default_variable("out2", columnconf);
 set_default_variable("out3", interactionconf);
 safe_free(rowmeans);
 safe_free(colmeans);
 safe_free(rowcounts);
 safe_free(colcounts);
 free_2d((void**)cellmeans, rowsize);
 free_2d((void**)cellcounts, rowsize);
}

void k_way_anova_dataset_discrete(Datasetptr d, Instanceptr data, int* features, int fdependent)
{
 /*!Last Changed 07.05.2004 only for three way currently*/
 int ***counts, *rowsums, *colsums, *depsums, datasize, **rcsums, **rfsums, **cfsums;
 int rowsize, colsize, dependentsize, i, j, k, row, col, index;
 double expected, sum, completeconf, onefactorconf, conditionalconf;
 Instanceptr tmp;
 rowsize = d->features[features[0]].valuecount;
 colsize = d->features[features[1]].valuecount;
 dependentsize = d->features[fdependent].valuecount;
 counts = (int***) safecalloc_3d(sizeof(int **), sizeof(int *), sizeof(int), rowsize, colsize, colsize, "k_way_anova_dataset_discrete", 6);
 tmp = data;
 datasize = 0;
 while (tmp)
  {
   row = tmp->values[features[0]].stringindex;
   col = tmp->values[features[1]].stringindex;
   index = tmp->values[fdependent].stringindex;
   counts[row][col][index]++;
   tmp = tmp->next;
   datasize++;
  }
 rowsums = safecalloc(rowsize, sizeof(int), "k_way_anova_dataset_discrete", 20);
 colsums = safecalloc(colsize, sizeof(int), "k_way_anova_dataset_discrete", 21);
 depsums = safecalloc(dependentsize, sizeof(int), "k_way_anova_dataset_discrete", 22);
 rcsums = (int**) safecalloc_2d(sizeof(int *), sizeof(int), rowsize, colsize, "k_way_anova_dataset_discrete", 23);
 rfsums = (int**) safecalloc_2d(sizeof(int *), sizeof(int), rowsize, dependentsize, "k_way_anova_dataset_discrete", 24);
 cfsums = (int**) safecalloc_2d(sizeof(int *), sizeof(int), colsize, dependentsize, "k_way_anova_dataset_discrete", 25);
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
      {
       rowsums[i] += counts[i][j][k];
       colsums[j] += counts[i][j][k];
       depsums[k] += counts[i][j][k];
       rcsums[i][j] += counts[i][j][k];
       rfsums[i][k] += counts[i][j][k];
       cfsums[j][k] += counts[i][j][k];
      }
 /*Complete Independence*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = datasize * (rowsums[i] / (datasize + 0.0)) * (colsums[j] / (datasize + 0.0)) * (depsums[k] / (datasize + 0.0));
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 completeconf = chi_square(rowsize * colsize * dependentsize - rowsize - colsize - dependentsize + 2, sum);
 if (get_parameter_o("displayresulton"))
  {
   if (completeconf < get_parameter_f("confidencelevel"))
     printf(m1253, features[0] + 1, features[1] + 1, fdependent + 1);
   else
     printf(m1254, features[0] + 1, features[1] + 1, fdependent + 1);       
  }
 /*One Factor Independence*/
 /*One Factor Independence - Dependent Factor*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = datasize * (rcsums[i][j] / (datasize + 0.0)) * (depsums[k] / (datasize + 0.0));
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 onefactorconf = chi_square((rowsize - 1) * (colsize - 1), sum);
 if (get_parameter_o("displayresulton"))
  {
   if (onefactorconf < get_parameter_f("confidencelevel"))
     printf(m1255, fdependent + 1, features[0] + 1, features[1] + 1);
   else
     printf(m1256, fdependent + 1, features[0] + 1, features[1] + 1);
  }
 /*One Factor Independence - Factor 1*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = datasize * (rfsums[i][k] / (datasize + 0.0)) * (colsums[j] / (datasize + 0.0));
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 onefactorconf = chi_square((rowsize - 1) * (dependentsize - 1), sum);
 if (get_parameter_o("displayresulton"))
  {
   if (onefactorconf < get_parameter_f("confidencelevel"))
     printf(m1255, features[1] + 1, fdependent + 1, features[0] + 1);
   else
     printf(m1256, features[1] + 1, fdependent + 1, features[0] + 1);
  }
 /*One Factor Independence - Factor 2*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = datasize * (cfsums[j][k] / (datasize + 0.0)) * (rowsums[i] / (datasize + 0.0));
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 onefactorconf = chi_square((dependentsize - 1) * (colsize - 1), sum);
 if (get_parameter_o("displayresulton"))
  {  
   if (onefactorconf < get_parameter_f("confidencelevel"))
     printf(m1255, features[0] + 1, fdependent + 1, features[1] + 1);
   else
     printf(m1256, features[0] + 1, fdependent + 1, features[1] + 1);
  }
 /*Conditional Independence*/
 /*Conditional Independence - Dependent Factor*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = rfsums[i][k] * cfsums[j][k] / (depsums[k] + 0.0);
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 conditionalconf = chi_square((rowsize - 1) * (colsize - 1) * dependentsize, sum);
 if (get_parameter_o("displayresulton"))
  {
   if (conditionalconf < get_parameter_f("confidencelevel"))
     printf(m1257, features[0] + 1, features[1] + 1, fdependent + 1);
   else
     printf(m1258, features[0] + 1, features[1] + 1, fdependent + 1);
  }
 /*Conditional Independence - 1 Factor*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = rcsums[i][j] * rfsums[i][k] / (rowsums[i] + 0.0);
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 conditionalconf = chi_square((dependentsize - 1) * (colsize - 1) * rowsize, sum);
 if (get_parameter_o("displayresulton"))
  {
   if (conditionalconf < get_parameter_f("confidencelevel"))
     printf(m1257, features[1] + 1, fdependent + 1, features[0] + 1);
   else
     printf(m1258, features[1] + 1, fdependent + 1, features[0] + 1);
  }
 /*Conditional Independence - 2 Factor*/
 sum = 0;
 for (i = 0; i < rowsize; i++)
   for (j = 0; j < colsize; j++)
     for (k = 0; k < dependentsize; k++)
       if (counts[i][j][k] > 0)
        {
         expected = rcsums[i][j] * cfsums[j][k] / (colsums[j] + 0.0);
         if (expected > 0)
           sum += counts[i][j][k] * log(counts[i][j][k] / expected);
        }
 sum *= 2;
 conditionalconf = chi_square((rowsize - 1) * (dependentsize - 1) * colsize, sum);
 if (get_parameter_o("displayresulton"))
  {
   if (conditionalconf < get_parameter_f("confidencelevel"))
     printf(m1257, features[0] + 1, fdependent + 1, features[1] + 1);
   else
     printf(m1258, features[0] + 1, fdependent + 1, features[1] + 1);
  }
 safe_free(rowsums);
 safe_free(colsums);
 safe_free(depsums);
 free_2d((void**)rcsums, rowsize);
 free_2d((void**)rfsums, rowsize);
 free_2d((void**)cfsums, colsize);
 free_3d((void***)counts, rowsize, colsize);
}

int k_way_anova_dataset(Datasetptr d, Instanceptr data, char** featurenos, int fcount)
{
 /*!Last Changed 07.05.2004*/
 int* features, i, fdependent, fno;
 fdependent = atoi(featurenos[1]) - 1;
 features = (int *) safemalloc((fcount - 2) * sizeof(int), "k_way_anova_dataset", 3);
 for (i = 2; i < fcount; i++)
  {
   fno = atoi(featurenos[i]) - 1;
   features[i - 2] = fno;
   if (in_list(d->features[fno].type, 3, REAL, INTEGER, REGDEF))
     return 0;
  }
 switch (d->features[fdependent].type)
  {
   case   REGDEF:
   case     REAL:
   case  INTEGER:k_way_anova_dataset_cont(d, data, features, fdependent);
                 break;
   case   STRING:
   case CLASSDEF:k_way_anova_dataset_discrete(d, data, features, fdependent);
                 break;
  }
 safe_free(features);
 return 1;
}

double class_entropy(char* datasetname)
{
 /*!Last Changed 25.03.2004*/
 Datasetptr tmp;
 int i;
 double result = 0, p;
 tmp = search_dataset(Datasets, datasetname);
 if (!tmp)
   return -1;
 for (i = 0; i < tmp->classno; i++)
   if (tmp->classcounts[i] != 0)
    {
     p = tmp->classcounts[i] / ((double)(tmp->datacount));
     result += tmp->classcounts[i] * (-p * log2(p));
    }
 result /= tmp->datacount;
 return result;
}

double average_valuecount(Datasetptr d)
{
 /*!Last Changed 01.10.2006*/
 double sum = 0;
 int i, count = 0;
 for (i = 0; i < d->featurecount; i++)
   switch (d->features[i].type)
    {
     case REAL   :
     case INTEGER:sum += 1;
                  count++;
                  break;
     case STRING :sum += d->features[i].valuecount - 1;
                  count++;
                  break;
    }
 return sum/count;
}

int* symbolic_features(Datasetptr d, int* count)
{
 /*!Last Changed 12.06.2006*/
 int *result = NULL, i;
 *count = 0;
 for (i = 0; i < d->featurecount; i++)
   if (d->features[i].type == STRING)
    {
     (*count)++;
     result = alloc(result, (*count) * sizeof(int), *count);
     result[(*count) - 1] = i;
    }
 return result;
}

int number_of_symbolic_features(char* datasetname)
{
 /*!Last Changed 25.03.2004*/
 Datasetptr tmp;
 int i, count = 0;
 tmp = search_dataset(Datasets, datasetname);
 if (!tmp)
   return -1;
 for (i = 0; i < tmp->featurecount; i++)
   if (tmp->features[i].type == STRING)
     count++;
 return count;
}

int* numeric_features(Datasetptr d, int* count)
{
 /*!Last Changed 12.06.2006*/
 int i, *result = NULL;
 *count = 0;
 for (i = 0; i < d->featurecount; i++)
   if (in_list(d->features[i].type, 2, REAL, INTEGER))
    {
     result = alloc(result, (*count + 1) * sizeof(int), *count + 1);
     result[*count] = i;
     (*count)++;
    }
 return result;
}

int number_of_numeric_features(char * datasetname)
{
 /*!Last Changed 25.03.2004*/
 Datasetptr tmp;
 int i, count = 0;
 tmp = search_dataset(Datasets, datasetname);
 if (!tmp)
   return -1;
 for (i = 0; i < tmp->featurecount; i++)
   if (in_list(tmp->features[i].type, 2, REAL, INTEGER))
     count++;
 return count;
}

void free_dataset(Datasetptr dataset)
{
 /*!Last Changed 06.01.2007*/
 /*!Last Changed 29.01.2004*/
 int i, j;
 for (i = 0; i < dataset->featurecount; i++)
   if (dataset->features[i].type == CLASSDEF || dataset->features[i].type == STRING)
     for (j = 0; j < dataset->features[i].valuecount; j++)
       safe_free(dataset->features[i].strings[j]);
 if (dataset->storetype != STORE_MULTILABEL)
   for (i = 0; i < dataset->classno; i++)
     safe_free(dataset->classes[i]);
 safe_free(dataset->name);
 switch (dataset->storetype)
  {
   case       STORE_TEXT:
   case STORE_SEQUENTIAL:
   case STORE_MULTILABEL:if (dataset->directory)
                           safe_free(dataset->directory);
                         if (dataset->file)
                           safe_free(dataset->file);
                         break;
   case       STORE_DB  :safe_free(dataset->dsnname);
                         safe_free(dataset->tablename);
                         break;
   case     STORE_KERNEL:if (dataset->directory)
                           safe_free(dataset->directory);
                         if (dataset->file)
                           safe_free(dataset->file);
                         if (dataset->kernel)
                          {
                           matrix_free(*(dataset->kernel));
                           safe_free(dataset->kernel);
                          }
                         break;
   default              :printf(m1237, dataset->storetype);
                         break;
  }
 safe_free(dataset);
}

Instanceptr generate_random_data(Datasetptr d, int datasize)
{
 /*!Last Changed 24.09.2006 added string type*/
 /*!Last Changed 09.03.2006*/
 Instanceptr tmp, head = NULL, tmpbefore = NULL;
 double value;
 int i, j;
 for (i = 0; i < datasize; i++)
  {
   tmp = empty_instance(d->featurecount);
   for (j = 0; j < d->featurecount - 1; j++)
     switch (d->features[j].type)
      {
       case REAL  :tmp->values[j].floatvalue = produce_random_value(-1, 1);
                   break;
       case STRING:tmp->values[j].stringindex = myrand() % d->features[j].valuecount;
                   break;
      }
   value = produce_random_value(-1, 1);
   if (value < 0)
     tmp->values[d->featurecount - 1].stringindex = 0;
   else
     tmp->values[d->featurecount - 1].stringindex = 1;
   if (tmpbefore)
     tmpbefore->next = tmp;
   else
     head = tmp;
   tmpbefore = tmp;
  }
 return head;
}

Datasetptr dimension_reduced_dataset(Datasetptr d, int dimension, Instanceptr reduceddata, char* datasetname)
{
 /*dimension includes the class definition feature*/
 Datasetptr newdataset;
 int i;
 newdataset = empty_dataset(datasetname, datasetname, "dataset.txt", ' ');
 if (d->classdefno == -1)
   dimension--;
 newdataset->allfeatures = dimension;
 newdataset->featurecount = dimension;
 newdataset->multifeaturecount = dimension;
 for (i = 0; i < dimension; i++)
  {
   newdataset->availability[i] = YES;
   newdataset->features[i].selected = YES;
  }
 if (d->classdefno != -1)
   newdataset->classdefno = newdataset->newclassdefno = dimension - 1;
 else
   newdataset->classdefno = newdataset->newclassdefno = -1;
 newdataset->classno = d->classno;
 for (i = 0; i < d->classno; i++)
   newdataset->classes[i] = strcopy(newdataset->classes[i], d->classes[i]);
 if (d->classdefno == -1)
   dimension++;
 for (i = 0; i < dimension - 1; i++)
  {
   newdataset->features[i].type = REAL;
   newdataset->features[i].oldtype = REAL;
   find_bounds_of_feature(newdataset, reduceddata, i, &(newdataset->features[i].min.floatvalue), &(newdataset->features[i].max.floatvalue));
   newdataset->features[i].valuecount = 0;
  }
 if (d->classdefno != -1)
  {
   newdataset->features[dimension - 1].type = CLASSDEF; 
   newdataset->features[dimension - 1].oldtype = CLASSDEF; 
   newdataset->features[dimension - 1].valuecount = d->classno;
   for (i = 0; i < d->classno; i++)
     if (d->classdefno == d->newclassdefno)
       newdataset->features[dimension - 1].strings[i] = strcopy(newdataset->features[dimension - 1].strings[i], d->features[d->classdefno].strings[i]);
     else
       newdataset->features[dimension - 1].strings[i] = strcopy(newdataset->features[dimension - 1].strings[i], d->features[d->newclassdefno].strings[i]);
  }
 newdataset->traincount = d->traincount;
 newdataset->datacount = d->datacount;
 for (i = 0; i < d->classno; i++)
   newdataset->classcounts[i] = d->classcounts[i];
 return newdataset;
}

Datasetptr generate_random_dataset(char* name, int featurecount, char* featuretypes, int* featurevaluecounts, int numberofclasses)
{
 /*!Last Changed 15.03.2007 added selected field*/
 /*!Last Changed 24.09.2006 added string type*/
 /*!Last Changed 09.03.2006*/
 /*featurecount does not include the class definition feature*/
 int i, count;
 char st[3];
 Datasetptr newdataset;
 newdataset = empty_dataset(name, "random", "random.txt", ' ');
 newdataset->allfeatures = featurecount + 1;
 newdataset->featurecount = featurecount + 1;
 count = 1; /*For class feature*/
 for (i = 0; i < featurecount; i++)
   count += featurevaluecounts[i];
 newdataset->multifeaturecount = count;
 for (i = 0; i < featurecount + 1; i++)
   newdataset->availability[i] = YES;
 newdataset->classdefno = featurecount;
 newdataset->newclassdefno = count - 1;
 newdataset->classno = numberofclasses;
 for (i = 0; i < numberofclasses; i++)
  {
   sprintf(st, "%d", i);
   newdataset->classes[i] = strcopy(newdataset->classes[i], st);
  }
 for (i = 0; i < featurecount; i++)
  {
   switch (featuretypes[i])
    {
     case 'R':newdataset->features[i].max.floatvalue = 1.0;
              newdataset->features[i].min.floatvalue = -1.0;
              newdataset->features[i].type = REAL;
              newdataset->features[i].oldtype = REAL;
              newdataset->features[i].valuecount = 0;
              break;
     case 'D':newdataset->features[i].type = STRING;
              newdataset->features[i].oldtype = STRING;
              newdataset->features[i].valuecount = featurevaluecounts[i];
              break;
    }
   newdataset->features[i].selected = YES;
  }
 newdataset->features[featurecount].type = CLASSDEF; 
 newdataset->features[featurecount].oldtype = CLASSDEF; 
 newdataset->features[featurecount].valuecount = numberofclasses;
 for (i = 0; i < numberofclasses; i++)
  {
   sprintf(st, "%d", i);
   newdataset->features[featurecount].strings[i] = strcopy(newdataset->features[featurecount].strings[i], st);
  }
 return newdataset;
}

void add_dataset(Datasetptr dataset)
{
 /*!Last Changed 29.01.2004*/
 Datasetptr tmp;
 tmp = Datasets;
 if (tmp == NULL)
  {
   Datasets = dataset;
   return;
  }
 while (tmp->next != NULL)
   tmp = tmp->next;
 tmp->next = dataset;
}

double* discretization_equalwidth(Datasetptr original, int fno, int valuecount)
{
 int i;
 double* splits, max, min;
 splits = safemalloc((valuecount - 1) * sizeof(double), "discretization_equalwidth", 1);
 switch (original->features[fno].type)
  {
   case INTEGER:min = original->features[fno].min.intvalue;
                max = original->features[fno].max.intvalue;
                break;
   case    REAL:min = original->features[fno].min.floatvalue;
                max = original->features[fno].max.floatvalue;
                break;
  }
 for (i = 0; i < valuecount - 1; i++)
   splits[i] = min + (i + 1) * ((max - min) / valuecount);
 return splits;
}

int** find_counts_for_all_splits(Datasetptr original, int fno, Instanceptr data, int* size, double** values)
{
 int i, j, k, count, **counts, classno;
 Instanceptr sorted;
 double previous = +INT_MAX, value;
 *size = 0;
 sorted = sort_instances(data, fno, &count);
 for (i = 0; i < count; i++)
  {
   switch (original->features[fno].type)
    {
     case INTEGER:value = sorted[i].values[fno].intvalue;
                  break;
     case    REAL:value = sorted[i].values[fno].floatvalue;
                  break; 
    }
   if (value != previous)
    {
     previous = value;
     (*size)++;
    }
  }
 counts = (int **) safecalloc_2d(sizeof(int*), sizeof(int), *size, MAXCLASSES, "find_counts_for_all_splits", 12);
 *values = safemalloc((*size) * sizeof(double), "find_counts_for_all_splits", 13); 
 j = -1;
 previous = +INT_MAX;
 for (i = 0; i < count; i++)
  {
   switch (original->features[fno].type)
    {
     case INTEGER:value = sorted[i].values[fno].intvalue;
                  break;
     case    REAL:value = sorted[i].values[fno].floatvalue;
                  break; 
    }
   classno = give_classno(&(sorted[i]));
   if (value != previous)
    {
     previous = value;
     j++;
     (*values)[j] = value;
     if (j != 0)
       for (k = 0; k < original->classno; k++)
         counts[j][k] = counts[j - 1][k];
    }
   counts[j][classno]++;
  }
 safe_free(sorted);
 return counts;
}

double* discretization_entropy(Datasetptr original, int fno, int valuecount, Instanceptr data)
{
 int ratio[MAXVALUES][MAXCLASSES];
 double *splits, *values = NULL, bestinfo = ISELL, info;
 int i, j, size, *combination, *best, **counts;
 splits = safemalloc((valuecount - 1) * sizeof(double), "discretization_entropy", 4);
 counts = find_counts_for_all_splits(original, fno, data, &size, &values);
 if (size > valuecount - 1)
  {
   best = safemalloc((valuecount - 1) * sizeof(int), "discretization_entropy", 5);
   combination = first_combination(valuecount - 1);
   for (i = 0; i < MAXVALUES; i++)
     for (j = 0; j < MAXCLASSES; j++)
       ratio[i][j] = 0;
   do{
     for (i = 0; i < valuecount; i++)
       for (j = 0; j < original->classno; j++)
         ratio[i][j] = 0;
     for (j = 0; j < original->classno; j++)
       ratio[0][j] = counts[combination[0]][j];
     for (i = 1; i < valuecount - 1; i++)
       for (j = 0; j < original->classno; j++)
         ratio[i][j] = counts[combination[i]][j] - counts[combination[i - 1]][j];
     for (j = 0; j < original->classno; j++)
       ratio[valuecount - 1][j] = counts[size - 1][j] - counts[combination[i - 1]][j];
     info = information_gain_from_ratios(ratio);
     if (info < bestinfo)
      {
       bestinfo = info;
       for (i = 0; i < valuecount - 1; i++)
         best[i] = combination[i];
      }
   }while (next_combination(combination, valuecount - 1, size - 1));
   for (i = 0; i < valuecount - 1; i++)
     splits[i] = values[best[i]];
   safe_free(best);
   safe_free(combination);   
  }
 else
  {
   for (i = 0; i < size; i++)
     splits[i] = values[i]; 
   for (i = size; i < valuecount - 1; i++)
     splits[i] = values[size - 1];  
  }
 safe_free(values);
 free_2d((void **) counts, size);
 return splits; 
}

Datasetptr discretize_dataset(Datasetptr original, int valuecount, DISCRETIZATION_TYPE discretization)
{
 Instanceptr data, inst;
 Datasetptr tmp;
 int i, j;
 char name[READLINELENGTH], tmpcommand[READLINELENGTH];
 char value[2];
 char* dname;
 char* fname;
 double **discretization_scheme, current;
 FILE* datafile;
 value[1] = '\0';
 sprintf(name, "%s%d", original->name, valuecount);
 tmp = empty_dataset(name, name, original->file, ' ');
 tmp->traincount = original->traincount;
 tmp->classdefno = original->classdefno;
 tmp->featurecount = original->featurecount;
 discretization_scheme = safemalloc(original->featurecount * sizeof(double *), "discretize_dataset", 12);
 tmp->datacount = original->datacount;
 tmp->classno = original->classno;
 for (i = 0; i < original->classno; i++)
  {
   tmp->classcounts[i] = original->classcounts[i];
   tmp->classes[i] = strcopy(tmp->classes[i], original->classes[i]);
  }
 tmp->allfeatures = original->allfeatures;
 for (i = 0; i < original->featurecount; i++)
  {
   tmp->features[i].selected = original->features[i].selected;
   tmp->availability[i] = original->availability[i];
   switch (original->features[i].type)
    {
     case  INTEGER:
     case     REAL:tmp->features[i].valuecount = valuecount;
                   tmp->features[i].type = STRING;
                   for (j = 0; j < valuecount; j++)
                    {
                     value[0] = '0' + j;
                     tmp->features[i].strings[j] = strcopy(tmp->features[i].strings[j], value);
                    }
                   break;
     case CLASSDEF:tmp->features[i].type = original->features[i].type;
                   tmp->features[i].valuecount = original->features[i].valuecount;
                   for (j = 0; j < tmp->features[i].valuecount; j++)
                     tmp->features[i].strings[j] = strcopy(tmp->features[i].strings[j], original->features[i].strings[j]);
                   break;
    }
  }
 read_from_train_file(original, &data);
 for (i = 0; i < original->featurecount; i++)
   if (original->features[i].type != CLASSDEF) 
     switch (discretization)
      {
       case EQUALWIDTH:discretization_scheme[i] = discretization_equalwidth(original, i, valuecount);
                       break;
       case    ENTROPY:discretization_scheme[i] = discretization_entropy(original, i, valuecount, data);
                       break; 
      }   
 inst = data;
 while (inst != NULL)
  {
   for (i = 0; i < original->featurecount; i++)
    {
     switch (original->features[i].type)
      {
       case INTEGER:current = inst->values[i].intvalue;
                    break;
       case    REAL:current = inst->values[i].floatvalue;
                    break;
      }
     if (in_list(original->features[i].type, 2, INTEGER, REAL))
      {
       j = 0;
       while (j < valuecount - 1 && current > discretization_scheme[i][j])
         j++;
       inst->values[i].stringindex = j;
      }
    }
   inst = inst->next;
  }
 dname = dataset_directory_name(tmp);
 sprintf(tmpcommand, "mkdir %s", dname);
 safe_free(dname);
 system(tmpcommand);
 fname = dataset_file_name(tmp);
 datafile = fopen(fname, "w");
 if (!datafile)
   return NULL;
 print_instance_list(datafile, ' ', tmp, data);
 fclose(datafile);
 for (i = 0; i < original->featurecount; i++)
   if (original->features[i].type != CLASSDEF) 
     safe_free(discretization_scheme[i]);
 safe_free(discretization_scheme);
 return tmp;
}

void extract_definitions_of_dataset(char* datasetname)
{
 /*!Last Changed 09.05.2008 added time variant datasets*/
 /*!Last Changed 02.04.2005*/
 /*!Last Changed 31.01.2004*/
 Datasetptr tmp;
 int i, tmpi, framecount;
 FILE* infile;
 char* fname, myline[READLINELENGTH], *st, separators[2];
 double tmpf;
 tmp = search_dataset(Datasets, datasetname);
 if (!tmp)
  {
   printf(m1259, datasetname);
   return;
  }
 sprintf(separators,"%c",tmp->separator);
 fname = dataset_file_name(tmp);
 infile = fopen(fname, "r");
 if (!infile)
   return;
 for (i = 0; i < tmp->featurecount; i++)
   switch (tmp->features[i].type)
    {
     case  INTEGER:tmp->features[i].max.intvalue = -INT_MAX;
                   tmp->features[i].min.intvalue = +INT_MAX;
                   break;
     case     REAL:
     case   REGDEF:tmp->features[i].max.floatvalue = (double) -INT_MAX;
                   tmp->features[i].min.floatvalue = (double) +INT_MAX;
                   break;
     case   STRING:tmp->features[i].valuecount = 0;
                   break;
    }
 tmp->datacount = 0;
 while (fgets(myline, READLINELENGTH, infile))
  {
   myline[strlen(myline) - 1] = '\0';
   st = strtok(myline, separators);
   i = 0;
   if (tmp->storetype == STORE_SEQUENTIAL)
    {
     st = strtok(NULL, separators);
     framecount = atoi(st);
     st = strtok(NULL, separators);
     i++;
    }
   while (st)
    {
     switch (tmp->features[i].type)
      {
       case INTEGER:if (isinteger(st))
                     {
                      tmpi = atoi(st);
                      if (tmpi < tmp->features[i].min.intvalue)
                        tmp->features[i].min.intvalue = tmpi;
                      else
                        if (tmpi > tmp->features[i].max.intvalue)
                          tmp->features[i].max.intvalue = tmpi;
                     }
                    else                      
                      return;
                    break;
       case    REAL:
       case  REGDEF:if (isfloat(st))
                     {
                      tmpf = (double) atof(st);
                      if (tmpf < tmp->features[i].min.floatvalue)
                        tmp->features[i].min.floatvalue = tmpf;
                      else
                        if (tmpf > tmp->features[i].max.floatvalue)
                          tmp->features[i].max.floatvalue = tmpf;
                     }
                    else
                      return;
                    break;
       case  STRING:if (listindex(st, tmp->features[i].strings, tmp->features[i].valuecount) == -1)
                     {
                      tmp->features[i].strings[tmp->features[i].valuecount] = strcopy(tmp->features[i].strings[tmp->features[i].valuecount], st);
                      (tmp->features[i].valuecount)++;
                      if (tmp->features[i].valuecount > MAXVALUES)
                       {
                        printf(m1420, i + 1);
                        exit(0);
                       }
                     }
                    break;  
      }
     st = strtok(NULL, separators);
     i++;
     if (tmp->storetype == STORE_SEQUENTIAL && i == tmp->featurecount)
       i = 1;
    }
   (tmp->datacount)++;
  }
 fclose(infile);
 safe_free(fname);
}

void save_definitions_of_dataset_as_xml(FILE* outfile, Datasetptr d)
{
 int i, j;
 char filetype[11];
 switch (d->storetype)
  {
   case         STORE_DB:strcpy(filetype, "database");
                         break;
   case       STORE_TEXT:strcpy(filetype, "text");
                         break;
   case STORE_SEQUENTIAL:strcpy(filetype, "sequential");
                         break;
   case     STORE_KERNEL:strcpy(filetype, "kernel");
                         break;
   case STORE_MULTILABEL:strcpy(filetype, "multilabel");
                         break;
  }
 if (d->classno != 0)
  {
   if (d->storetype == STORE_KERNEL)
    {
     fprintf(outfile, "\t<dataset name=\"%s\" task=\"classification\" size=\"%d\" directory=\"%s\" filetype=\"%s\" separator=\"%c\" filename=\"%s\" kernelcount=\"%d\" sigma=\"%.6f\">\n", d->name, d->datacount, d->directory, filetype, d->separator, d->file, d->kernelcount, d->sigma);
     fprintf(outfile, "\t\t<kernels>\n");
     for (i = 0; i < d->kernelcount; i++)
       fprintf(outfile, "\t\t\t<kernel value=\"%s\"/>\n", d->kernelfiles[i]);
     fprintf(outfile, "\t\t</kernels>\n");
    }
   else
     if (d->storetype == STORE_MULTILABEL)
       fprintf(outfile, "\t<dataset name=\"%s\" task=\"classification\" size=\"%d\" directory=\"%s\" filetype=\"%s\" separator=\"%c\" filename=\"%s\" classcount=\"%d\" classlabelstart=\"%d\" sigma=\"%.6f\">\n", d->name, d->datacount, d->directory, filetype, d->separator, d->file, d->classno, d->classdefno, d->sigma);
     else
       fprintf(outfile, "\t<dataset name=\"%s\" task=\"classification\" size=\"%d\" directory=\"%s\" filetype=\"%s\" separator=\"%c\" filename=\"%s\" sigma=\"%.6f\">\n", d->name, d->datacount, d->directory, filetype, d->separator, d->file, d->sigma);
   if (d->storetype != STORE_MULTILABEL)
    {
     fprintf(outfile, "\t\t<classes>\n");
     for (i = 0; i < d->classno; i++)
       fprintf(outfile, "\t\t\t<class value=\"%s\" size=\"%d\"/>\n", d->classes[i], d->classcounts[i]);
     fprintf(outfile, "\t\t</classes>\n");
    }
  }
 else
  {
   if (d->storetype == STORE_KERNEL)
    {
     fprintf(outfile, "\t<dataset name=\"%s\" task=\"regression\" size=\"%d\" directory=\"%s\" filetype=\"%s\" separator=\"%c\" filename=\"%s\" kernelcount=\"%d\" sigma=\"%.6f\">\n", d->name, d->datacount, d->directory, filetype, d->separator, d->file, d->kernelcount, d->sigma);
     fprintf(outfile, "\t\t<kernels>\n");
     for (i = 0; i < d->kernelcount; i++)
       fprintf(outfile, "\t\t\t<kernel value=\"%s\"/>\n", d->kernelfiles[i]);
     fprintf(outfile, "\t\t</kernels>\n");
    }
   else
     fprintf(outfile, "\t<dataset name=\"%s\" task=\"regression\" size=\"%d\" directory=\"%s\" filetype=\"%s\" separator=\"%c\" filename=\"%s\" sigma=\"%.6f\">\n", d->name, d->datacount, d->directory, filetype, d->separator, d->file, d->sigma);
  }
 fprintf(outfile, "\t\t<attributes>\n");
 for (i = 0; i < d->featurecount; i++)
   switch (d->features[i].type){
     case    INTEGER:fprintf(outfile, "\t\t\t<attribute type=\"Integer\" available=\"True\" min=\"%d\" max=\"%d\"/>\n", d->features[i].min.intvalue, d->features[i].max.intvalue);
                     break;
     case       REAL:fprintf(outfile, "\t\t\t<attribute type=\"Real\" available=\"True\" min=\"%.6f\" max=\"%.6f\"/>\n", d->features[i].min.floatvalue, d->features[i].max.floatvalue);
                     break;
     case     STRING:fprintf(outfile, "\t\t\t<attribute type=\"String\" available=\"True\">\n");
                     for (j = 0; j < d->features[i].valuecount; j++)
                       fprintf(outfile, "\t\t\t\t<value>%s</value>\n", d->features[i].strings[j]);
                     fprintf(outfile, "\t\t\t</attribute>\n");
                     break;
     case   CLASSDEF:fprintf(outfile, "\t\t\t<attribute type=\"Output\" available=\"True\">\n");
                     for (j = 0; j < d->features[i].valuecount; j++)
                       fprintf(outfile, "\t\t\t\t<value>%s</value>\n", d->features[i].strings[j]);
                     fprintf(outfile, "\t\t\t</attribute>\n");
                     break;
     case     REGDEF:fprintf(outfile, "\t\t\t<attribute type=\"Output\" available=\"True\" min=\"%.6f\" max=\"%.6f\"/>\n", d->features[i].min.floatvalue, d->features[i].max.floatvalue);
                     break;
    }
 fprintf(outfile, "\t\t</attributes>\n");
 fprintf(outfile, "\t</dataset>\n");
}

Datasetptr empty_dataset(char* datasetname, char* directoryname, char* filename, char separator)
{
 /*!Last Changed 24.12.2006*/
 Datasetptr tmp;
 tmp = safemalloc(sizeof(Dataset), "create_dataset", 12);
 tmp->name = strcopy(tmp->name, datasetname);
 tmp->directory = strcopy(tmp->directory, directoryname);
 tmp->file = strcopy(tmp->file, filename);
 tmp->separator = separator;
 tmp->active = 1;
 tmp->dsnname = NULL;
 tmp->tablename = NULL;
 tmp->storetype = STORE_TEXT;
 tmp->kernel = NULL;
 tmp->next = NULL;
 tmp->sigma = 0.0;
 return tmp;
}

int read_kernel_of_dataset(Datasetptr d)
{
 int i, j;
 char *st, myline[READLINELENGTH], *kernelfilename;
 FILE* infile;
 kernelfilename = kernel_file_name(d); 
 infile = fopen(kernelfilename, "r");
 if (!infile)
  {
   printf(m1476, kernelfilename);
   safe_free(kernelfilename);
   return 0;
  }
 if (d->kernel != NULL)
  {
   matrix_free(*(d->kernel));
   safe_free(d->kernel);
  }
 d->kernel = safemalloc(sizeof(matrix), "read_kernel_of_dataset", 10);
 *(d->kernel) = matrix_alloc(d->datacount, d->datacount);
 i = 0;
 while (fgets(myline, READLINELENGTH, infile))
  {
   if (strlen(myline) <= 1)
     continue;
   myline[strlen(myline) - 1] = '\0';
   st = strtok(myline, " ");
   j = 0;
   while (st)
    {
     d->kernel->values[i][j] = atof(st);
     j++;
     st = strtok(NULL, " ");
    }
   if (j != d->datacount)
    {
     printf(m1469, i + 1, d->datacount);
     matrix_free(*(d->kernel));
     safe_free(d->kernel);
     d->kernel = NULL;
     safe_free(kernelfilename);
     return 0;
    }
   i++;
  }
 if (i != d->datacount)
  {
   printf(m1470, d->datacount);
   matrix_free(*(d->kernel));
   safe_free(d->kernel);
   d->kernel = NULL;
   safe_free(kernelfilename);
   return 0;
  }
 fclose(infile);
 return 1;
}

int create_kernel_dataset(char* datasetname, char* classes, char* directoryname, char* datafilename, char* kernelfilename)
{
 Datasetptr tmp;
 int i, j, classcode;
 char* fname, *st, myline[READLINELENGTH];
 FILE* infile;
 tmp = search_dataset(Datasets, datasetname);
 if (tmp)
  {
   printf(m1120);
   delete_dataset(datasetname);
  }
 tmp = empty_dataset(datasetname, directoryname, datafilename, ' ');
 tmp->storetype = STORE_KERNEL;
 stringlist(kernelfilename, ";", tmp->kernelfiles, &(tmp->kernelcount));
 for (i = 0; i < MAXCLASSES; i++)
   tmp->classcounts[i] = 0;
 stringlist(classes, ";", tmp->classes, &(tmp->classno));
 if (tmp->classno > MAXCLASSES)
  {
   printf(m1419, tmp->classno);
   return 0;
  }
 fname = dataset_file_name(tmp);
 infile = fopen(fname, "r");
 if (!infile)
  {
   printf(m1121, fname);
   free_dataset(tmp);
   safe_free(fname);
   return 0;
  }
 j = 0;
 while (fgets(myline, READLINELENGTH, infile))
  {
   if (strlen(myline) <= 1)
     continue;
   myline[strlen(myline) - 1] = '\0';
   st = strtok(myline, " ");
   i = 0;
   while (st)
    {
     if (i == 0 && atoi(st) != j)
      {
       printf(m1475, j + 1);
       free_dataset(tmp);
       safe_free(fname);
       return 0;
      }
     if (i == 1 && tmp->classno)
      {
       classcode = listindex(st, tmp->classes, tmp->classno);
       if (classcode == -1)
        {
         printf(m1260, j, 1);
         free_dataset(tmp);
         safe_free(fname);
         return 0;
        }
       tmp->classcounts[classcode]++;
      }
     i++;
     st = strtok(NULL, " ");
    }
   j++;
  }
 fclose(infile);
 tmp->datacount = j;
 tmp->active = NO;
 tmp->classdefno = 1;
 tmp->featurecount = 2;
 tmp->features[0].type = REAL;
 tmp->features[0].min.floatvalue = 0.0;
 tmp->features[0].max.floatvalue = tmp->datacount - 1.0;
 if (tmp->classno)
  {
   tmp->features[1].type = CLASSDEF;
   for (i = 0; i < tmp->classno; i++)
     tmp->features[1].strings[i] = strcopy(tmp->features[1].strings[i], tmp->classes[i]);
   tmp->features[1].valuecount = tmp->classno;
  }
 else
   tmp->features[1].type = REGDEF;
 add_dataset(tmp);
 tmp->kernel = NULL;
 printf(m1126);
 safe_free(fname);
 return 1;
}

int create_multi_label_dataset(char* datasetname, char* separator, int class_start, char* directoryname, char* filename, char* discretefeatures)
{
 Datasetptr tmp;
 int i;
 char* fname, *st, myline[READLINELENGTH];
 FILE* infile;
 Intervalptr intervals;
 tmp = search_dataset(Datasets, datasetname);
 if (tmp)
  {
  printf(m1120);
  delete_dataset(datasetname);
  }
 tmp = empty_dataset(datasetname, directoryname, filename, separator[0]);
 tmp->datacount = 1;
 tmp->allfeatures = 0;
 tmp->storetype = STORE_MULTILABEL;
 tmp->classdefno = class_start;
 for (i = 0; i < MAXCLASSES; i++)
   tmp->classcounts[i] = 0;
 intervals = create_intervals(discretefeatures);
 fname = dataset_file_name(tmp);
 infile = fopen(fname, "r");
 if (!infile)
  {
   printf(m1121, fname);
   free_dataset(tmp);
   safe_free(fname);
   return 0;
  }
 if (fgets(myline, READLINELENGTH, infile))
  {
   myline[strlen(myline) - 1]='\0';
   st = strtok(myline, separator);
   while (st)
    {
     if (strcmp(st,"?") == 0)
      {
       free_dataset(tmp);
       safe_free(fname);
       printf(m1127);
       return 0;
      }
     if (tmp->allfeatures < class_start)
      {
       if (is_in_interval(intervals, tmp->allfeatures))
         tmp->features[tmp->allfeatures].type = STRING;
       else
         if (isinteger(st))
           tmp->features[tmp->allfeatures].type = INTEGER;
         else
           tmp->features[tmp->allfeatures].type = REAL;
       tmp->features[tmp->allfeatures].selected = YES;
      }
     tmp->allfeatures++;
     st = strtok(NULL, separator);
    }
   if (tmp->allfeatures < class_start + 2)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1125);
     return 0;
    }
   if (tmp->allfeatures <= 1)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1123);
     return 0;
    }
   if (tmp->allfeatures > MAXFEATURES)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1421, tmp->allfeatures);
     return 0;
    }
   tmp->classno = tmp->allfeatures - class_start;
  }
 else
  {
   free_dataset(tmp);
   safe_free(fname);
   printf(m1122);
   return 0;
  }
 while (fgets(myline, READLINELENGTH, infile))
  {
   if (strlen(myline) <= 1)
     continue;
   myline[strlen(myline) - 1] = '\0';
   tmp->datacount++;
   st = strtok(myline, separator);
   i = 0;
   while (st)
    {
     if (strcmp(st, "?") != 0 && i < class_start)
       switch (tmp->features[i].type)
        {
         case  INTEGER:if (!isinteger(st) && isfloat(st))
                         tmp->features[i].type = REAL;
                       break;
        }
     i++;
     st = strtok(NULL, separator);
    }
   if (i != tmp->allfeatures)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1261, tmp->datacount, tmp->allfeatures);
     return 0;
    }
  }
 tmp->allfeatures = class_start;
 tmp->featurecount = tmp->allfeatures;
 for (i = 0; i < tmp->featurecount; i++)
   tmp->availability[i] = 1;
 fclose(infile);
 tmp->active = NO;
 add_dataset(tmp);
 printf(m1126);
 safe_free(fname);
 free_intervals(intervals);
 return 1;
}

int create_dataset(char* datasetname, char* separator, int classdefno, char* classes, char* directoryname, char* filename, char* discretefeatures)
{
 /*!Last Changed 13.03.2007 added selected for features*/
 /*!Last Changed 19.08.2006 added regression definition*/
 /*!Last Changed 09.05.2005 changed including discrete feature definitions*/
 /*!Last Changed 02.02.2004 added safemalloc and safecalloc*/
 /*!Last Changed 31.01.2004*/
 Datasetptr tmp;
 int found = 0, i, classcode;
 char* fname, *st, myline[READLINELENGTH];
 FILE* infile;
 Intervalptr intervals;
 tmp = search_dataset(Datasets, datasetname);
 if (tmp)
  {
   printf(m1120);
   delete_dataset(datasetname);
  }
 tmp = empty_dataset(datasetname, directoryname, filename, separator[0]);
 tmp->datacount = 1;
 tmp->allfeatures = 0;
 tmp->classdefno = classdefno;
 for (i = 0; i < MAXCLASSES; i++)
   tmp->classcounts[i] = 0;
 stringlist(classes, ";", tmp->classes, &(tmp->classno));
 if (tmp->classno > MAXCLASSES)
  {
   printf(m1419, tmp->classno);
   return 0;
  }
 intervals = create_intervals(discretefeatures);
 fname = dataset_file_name(tmp);
 infile = fopen(fname, "r");
 if (!infile)
  {
   printf(m1121, fname);
   free_dataset(tmp);
   safe_free(fname);
   return 0;
  }
 if (fgets(myline,READLINELENGTH,infile))
  {
   myline[strlen(myline) - 1]='\0';
   st = strtok(myline, separator);
   while (st)
    {
     if (strcmp(st,"?") == 0)
      {
       free_dataset(tmp);
       safe_free(fname);
       printf(m1127);
       return 0;     
      }
     if (tmp->allfeatures == classdefno)
      {
       if (tmp->classno)
        {
         tmp->features[tmp->allfeatures].type = CLASSDEF;
         for (i = 0; i < tmp->classno; i++)
           tmp->features[tmp->classdefno].strings[i] = strcopy(tmp->features[tmp->classdefno].strings[i], tmp->classes[i]);
         tmp->features[tmp->classdefno].valuecount = tmp->classno;
         tmp->classcounts[listindex(st, tmp->classes, tmp->classno)] = 1;
        }
       else
         tmp->features[tmp->allfeatures].type = REGDEF;
       found = 1;
      }
     else
       if (is_in_interval(intervals, tmp->allfeatures))
         tmp->features[tmp->allfeatures].type = STRING;
       else
         if (isinteger(st))
           tmp->features[tmp->allfeatures].type = INTEGER;
         else
           tmp->features[tmp->allfeatures].type = REAL;
     tmp->features[tmp->allfeatures].selected = YES;
     tmp->allfeatures++;
     st = strtok(NULL, separator);
    }
   if (!found && tmp->classdefno != -1)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1125);
     return 0;     
    }
   if (tmp->allfeatures <= 1)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1123);
     return 0;
    }
   if (tmp->allfeatures > MAXFEATURES)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1421, tmp->allfeatures);
     return 0;
    }
  }
 else
  {
   free_dataset(tmp);
   safe_free(fname);
   printf(m1122);
   return 0;
  }
 while (fgets(myline, READLINELENGTH, infile))
  {
   if (strlen(myline) <= 1)
     continue;
   myline[strlen(myline) - 1] = '\0';
   tmp->datacount++;
   st = strtok(myline, separator);
   i = 0;
   while (st)
    {
     if (strcmp(st,"?") != 0)
       switch (tmp->features[i].type)
        {
         case  INTEGER:if (!isinteger(st) && isfloat(st))
                         tmp->features[i].type = REAL;
                       break;
         case CLASSDEF:classcode = listindex(st, tmp->classes, tmp->classno);
                       if (classcode == -1)
                        {
                         printf(m1260, tmp->datacount, i+1);
                         free_dataset(tmp);
                         safe_free(fname);
                         return 0;
                        }
                       else
                         tmp->classcounts[classcode]++;
                       break;
        }
     i++;
     st = strtok(NULL, separator);
    }
   if (i != tmp->allfeatures)
    {
     free_dataset(tmp);
     safe_free(fname);
     printf(m1261, tmp->datacount, tmp->allfeatures);
     return 0;          
    }
  }
 tmp->featurecount = tmp->allfeatures;
 for (i = 0; i < tmp->featurecount; i++)
   tmp->availability[i] = 1;
 fclose(infile);
 tmp->active = NO;
 add_dataset(tmp);
 printf(m1126);
 safe_free(fname);
 free_intervals(intervals);
 return 1;
}

int create_sequential_dataset(char* datasetname, char* separator, int numfeatures, char* classes, char* directoryname, char* filename, char* discretefeatures)
{
 /*!Last Changed 09.05.2008*/
 Datasetptr tmp;
 int i, classcode, framecount, frameindex;
 char* fname, *st, myline[READLINELENGTH];
 FILE* infile;
 Intervalptr intervals;
 tmp = search_dataset(Datasets, datasetname);
 if (tmp)
  {
   printf(m1120);
   delete_dataset(datasetname);
  }
 tmp = empty_dataset(datasetname, directoryname, filename, separator[0]);
 tmp->datacount = 0;
 tmp->allfeatures = numfeatures;
 tmp->featurecount = tmp->allfeatures;
 tmp->storetype = STORE_SEQUENTIAL;
 for (i = 0; i < tmp->featurecount; i++)
   tmp->availability[i] = YES;
 tmp->classdefno = 0;
 for (i = 0; i < MAXCLASSES; i++)
   tmp->classcounts[i] = 0;
 stringlist(classes, ";", tmp->classes, &(tmp->classno));
 if (tmp->classno)
  {
   tmp->features[0].type = CLASSDEF;
   for (i = 0; i < tmp->classno; i++)
     tmp->features[0].strings[i] = strcopy(tmp->features[0].strings[i], tmp->classes[i]);
   tmp->features[0].valuecount = tmp->classno;
  }
 else
   tmp->features[0].type = REGDEF;
 tmp->features[0].selected = YES;
 intervals = create_intervals(discretefeatures);
 for (i = 1; i < numfeatures; i++)
  {
   tmp->features[i].selected = YES;
   if (is_in_interval(intervals, i))
     tmp->features[i].type = STRING;
   else
     tmp->features[i].type = INTEGER;
  }
 fname = dataset_file_name(tmp);
 infile = fopen(fname, "r");
 if (!infile)
  {
   printf(m1121, fname);
   free_dataset(tmp);
   safe_free(fname);
   free_intervals(intervals);
   return 0;
  }
 while (fgets(myline, READLINELENGTH, infile))
  {
   if (strlen(myline) <= 1)
     continue;
   tmp->datacount++;
   myline[strlen(myline) - 1] = '\0';
   st = strtok(myline, separator);
   classcode = listindex(st, tmp->classes, tmp->classno);
   if (classcode == -1)
    {
     printf(m1260, tmp->datacount, 0);
     free_dataset(tmp);
     safe_free(fname);
     free_intervals(intervals);
     return 0;
    }
   else
     tmp->classcounts[classcode]++;
   st = strtok(NULL, separator);
   framecount = atoi(st);
   frameindex = 0;
   i = 1;
   st = strtok(NULL, separator);
   while (st)
    {
     if (!is_in_interval(intervals, i) && !isinteger(st) && tmp->features[i].type != REAL)
       tmp->features[i].type = REAL;
     i++;
     if (i == numfeatures)
      {
       i = 1;
       frameindex++;
      }
     st = strtok(NULL, separator);
    }
   if (i != 1)
    {
     printf(m1404, frameindex);
     free_dataset(tmp);
     safe_free(fname);
     free_intervals(intervals);
     return 0;     
    }
   if (frameindex != framecount)
    {
     printf(m1403, tmp->datacount, framecount);
     free_dataset(tmp);
     safe_free(fname);
     free_intervals(intervals);
     return 0;
    }
  }
 fclose(infile);
 tmp->active = NO;
 add_dataset(tmp);
 printf(m1126);
 safe_free(fname);
 free_intervals(intervals);
 return 1;
}

void save_names_file(char* datasetname, char* namesfile)
{
 /*!Last Changed 19.02.2004 added C5.0 names type*/
 /*!Last Changed 27.10.2003*/
 int i, j, k;
 Datasetptr d;
 FILE* outfile;
 outfile = fopen(namesfile, "w");
 if (!outfile)
   return;
 d = search_dataset(Datasets, datasetname);
 switch (get_parameter_l("namestype"))
  {
   case NAMES_C50   :fprintf(outfile,"Feature%d.\n",d->classdefno + 1);
                     break;
   case NAMES_RIPPER:for (i = 0; i < d->classno-1; i++)
                       fprintf(outfile,"c%s,",d->classes[i]);
                     fprintf(outfile,"c%s.\n",d->classes[i]);
                     break;
   default          :printf(m1239, get_parameter_l("namestype"));
                     break;
  }
 k = 0;
 for (i = 0; i < d->featurecount; i++)
   if (get_parameter_l("namestype") == NAMES_C50 || d->features[i].type != CLASSDEF)
    {
     k++;
     fprintf(outfile,"Feature%d: ",k);
     switch (d->features[i].type)
      {
       case INTEGER :
       case REAL    :fprintf(outfile,"continuous");
                     break;
       case STRING  :
       case CLASSDEF:for (j = 0; j < d->features[i].valuecount-1; j++)
                       fprintf(outfile,"%s, ",d->features[i].strings[j]);
                     fprintf(outfile,"%s",d->features[i].strings[j]);
                     break;
      }
     fprintf(outfile,".\n");
    }
 fclose(outfile);
}

void type_change(Datasetptr d)
{
/*!Last Changed 19.03.2002*/
 int i;
 for (i = 0; i < d->featurecount; i++)
  {
   d->features[i].oldtype = d->features[i].type;
   if (d->features[i].type == INTEGER)
     d->features[i].type = REAL;
  }
}

void type_restore(Datasetptr d)
{
/*!Last Changed 19.03.2002*/
 int i;
 for (i = 0; i < d->featurecount; i++)
   d->features[i].type = d->features[i].oldtype;
}

void print_datasets(Datasetptr header, DATASET_TYPE printwhat)
{
 int i=1;
 Datasetptr dataset=header;
 if (!dataset)
  {
   printf(m1056);
   return;
  }
 printf("|Dataset Name      |# of Classes|# of Features|# of Instances|Base Error|\n");
 while (dataset)
  {
   if (printwhat == ALL || (printwhat == CLASSIFICATION_DATASETS && dataset->classno > 0) || (printwhat == REGRESSION_DATASETS && dataset->classno == 0) || (printwhat == TWOCLASS_DATASETS && dataset->classno == 2) || (printwhat == KCLASS_DATASETS && dataset->classno > 2) || (printwhat == DISCRETE_DATASETS && number_of_symbolic_features(dataset->name) == dataset->featurecount - 1) || (printwhat == CONTINUOUS_DATASETS && number_of_numeric_features(dataset->name) == dataset->featurecount - 1) || (printwhat == MIXED_DATASETS && number_of_numeric_features(dataset->name) != dataset->featurecount - 1 && number_of_symbolic_features(dataset->name) != dataset->featurecount - 1))
    {
     printf("|%18s|%12d|%13d|%14d|%10.2f|\n", dataset->name, dataset->classno, dataset->featurecount, dataset->datacount, dataset->baseerror);
     i++;  
    }
   dataset=dataset->next;
  }
}

Datasetptr search_dataset(Datasetptr header, char *dataset_name)
{
 Datasetptr dataset = header;
 while (dataset)
  {
   if (!strcmp(dataset->name, dataset_name))
     return dataset;
   dataset = dataset->next;
  }
 return NULL;
}

void delete_dataset(char *dataset_name)
{
 Datasetptr previous = NULL, dataset = Datasets;
 while (dataset)
  {
   if (!strcmp(dataset->name, dataset_name))
    {
     if (previous)
       previous->next = dataset->next;
     else
       Datasets = dataset->next;
     free_dataset(dataset);
     break;
    }
   previous = dataset;
   dataset = dataset->next;
  }
}

Instanceptr empty_instance(int fcount)
{
 /*!Last Changed 07.01.2007 added sublist*/
 /*!Last Changed 26.11.2005 removed unused dataset parameter*/
 /*!Last Changed 24.03.2005*/
 Instanceptr currentinstance;
 currentinstance = (Instanceptr)safemalloc(sizeof(Instance), "empty_instance", 2);
 currentinstance->values = (union value*) safemalloc(fcount * sizeof(union value), "empty_instance", 3);
 currentinstance->isexception = NO;
 currentinstance->next = NULL;
 currentinstance->sublist = NULL;
 currentinstance->class_labels = NULL;
 return currentinstance;
}

Instanceptr empty_multi_label_instance(int fcount, int classcount)
{
 Instanceptr currentinstance;
 currentinstance = empty_instance(fcount);
 currentinstance->class_labels = (int*) safemalloc(classcount * sizeof(int), "empty_multi_label_instance", 3);
 return currentinstance;
}

Instanceptr create_instance(Datasetptr dataset, char *myline, int withnoise)
{
 /*!Last Changed 07.01.2007 added sublist and continuous data*/
 /*!Last Changed 08.05.2004 can now handle exceptions*/
 /*!Last Changed 10.03.2004 Extended to regression*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 Instanceptr currentinstance, subinstance, firstinstance;
 int j, k, unknown, tmp, cno, framecount, currentframe = 0;
 char st1[4];
 char *st, tmpline[READLINELENGTH];
 if (is_line_comment(myline))
   return NULL;
 strcpy(tmpline, myline);
 st1[0] = dataset->separator;
 st1[1] = '\n';
 st1[2] = '\0';
 if (dataset->storetype == STORE_MULTILABEL)
   currentinstance = empty_multi_label_instance(dataset->featurecount, dataset->classno);
 else
   currentinstance = empty_instance(dataset->featurecount);
 firstinstance = currentinstance;
 j = 0;
 k = 0;
 st = (char *) strtok(tmpline, st1);
 if (dataset->storetype == STORE_SEQUENTIAL)
  {
   currentinstance->values[0].stringindex = listindex(st, dataset->features[0].strings, dataset->features[0].valuecount);
   cno = currentinstance->values[0].stringindex;
   if (cno == -1)
     printf(m1263, myline, st);
   st = (char *) strtok(NULL, st1);
   framecount = atoi(st);
   currentframe = 0;
   st = (char *) strtok(NULL, st1);
   j = 1;
   k = 1;
  }
 while (st)
  {
   if (dataset->availability[j])
    {
     if (strcmp(st, "?") == 0)
       unknown = 1;
     else
       unknown = 0;
     if (k < dataset->featurecount)
      {
       switch (dataset->features[k].type){
         case INTEGER :if (!unknown)
                         currentinstance->values[k].intvalue = atoi(st);
                       else
                         currentinstance->values[k].intvalue = ISELL;
                       break;
         case REGDEF  :
         case REAL    :if (!unknown)
                         currentinstance->values[k].floatvalue = (double)atof(st);
                       else
                         currentinstance->values[k].floatvalue = ISELL;
                       break;
         case STRING  :if (!unknown)
                        {
                         tmp = listindex(st, dataset->features[k].strings, dataset->features[k].valuecount);
                         if (tmp == -1)
                           printf(m1262, st, k);
                         else
                           currentinstance->values[k].stringindex = tmp;
                        }
                       else
                         currentinstance->values[k].stringindex = -1;
                       break;
         case CLASSDEF:currentinstance->values[k].stringindex = listindex(st, dataset->features[k].strings, dataset->features[k].valuecount);
                       if (currentinstance->values[k].stringindex == -1)
                         printf(m1263, myline, st);
                       if (withnoise && get_parameter_f("noiselevel") > 0.0)
                         if (produce_random_value(0.0, 1.0) < get_parameter_f("noiselevel"))
                           currentinstance->values[k].stringindex = myrand() % dataset->features[k].valuecount;
                       break;
         default      :printf(m1238, dataset->features[k].type);
                       break;
       }
      }
     else
       if (dataset->storetype == STORE_MULTILABEL)
         currentinstance->class_labels[k - dataset->classdefno] = atoi(st);
     k++;
    }
   st = (char *) strtok(NULL, st1);
   j++;
   if (k == dataset->featurecount && dataset->storetype == STORE_SEQUENTIAL && st)
    {
     subinstance = empty_instance(dataset->featurecount);
     if (currentframe == 0)
       currentinstance->sublist = subinstance;
     else
       currentinstance->next = subinstance;
     subinstance->values[0].stringindex = cno;
     currentframe++;
     j = 1;
     k = 1;
     currentinstance = subinstance;
    }
  }
 if (k == dataset->featurecount || (k == dataset->featurecount + dataset->classno && dataset->storetype == STORE_MULTILABEL))
   return firstinstance;
 else
   return NULL;  
}

char* dataset_directory_name(Datasetptr dataset)
{
 char *fname;
 fname = (char *)safemalloc(200, "dataset_directory_name", 2);
 if (get_parameter_l("currentos"))
   sprintf(fname, "%s/%s", datadir, dataset->directory);
 else
   sprintf(fname, "%s\\%s", datadir, dataset->directory);
 return fname;
}

char* definition_file_name(Datasetptr dataset)
{
 /*!Last Changed 31.10.2005*/
 char *fname;
 fname = (char *)safemalloc(200, "definition_file_name", 2);
 if (get_parameter_l("currentos"))
   sprintf(fname, "%s/%s/data.def", datadir, dataset->directory);
 else
   sprintf(fname, "%s\\%s\\data.def", datadir, dataset->directory);
 return fname;
}

char* dataset_file_name(Datasetptr dataset)
{
 /*!Last Changed 31.10.2005*/
 char *fname;
 fname = (char *)safemalloc(200, "dataset_file_name", 2);
 if (get_parameter_l("currentos"))
   sprintf(fname, "%s/%s/%s", datadir, dataset->directory, dataset->file);
 else
   sprintf(fname, "%s\\%s\\%s", datadir, dataset->directory, dataset->file);
 return fname;
}

char* kernel_file_name(Datasetptr dataset)
{
 char *fname;
 fname = (char *)safemalloc(200, "kernel_file_name", 2);
 if (get_parameter_l("currentos"))
   sprintf(fname, "%s/%s/%s", datadir, dataset->directory, dataset->kernelfiles[get_parameter_i("kernelindex")]);
 else
   sprintf(fname, "%s\\%s\\%s", datadir, dataset->directory, dataset->kernelfiles[get_parameter_i("kernelindex")]);
 return fname;
}

int connect_to_database(Datasetptr dataset)
{
 /*!Last Changed 29.12.2005*/
 #ifdef ODBC
 RETCODE rc;        /* ODBC return code*/
 int i;
 unsigned char nerr[10];
 SQLINTEGER resultsize;
 SQLSMALLINT bufsize;
 int result = 0;
 SQLAllocEnv(&henv);
 SQLAllocConnect(henv, &hdbc);
 rc = SQLConnect(hdbc, dataset->dsnname, strlen(dataset->dsnname), NULL, 0, NULL, 0);
 if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
  {
   rc = SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, nerr, &resultsize, databuffer, sizeof(databuffer), &bufsize);
   printf(m1265, nerr);
   for (i = 0; i < bufsize; i++)
     printf("%c", databuffer[i]);
   printf("\n");
  }
 else
  {
   rc = SQLAllocStmt(hdbc, &hstmt);
   sprintf(databuffer, "SELECT * FROM %s", dataset->tablename);
   rc = SQLExecDirect(hstmt, databuffer, strlen(databuffer));
   if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
    {
     rc = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, nerr, &resultsize, databuffer, sizeof(databuffer), &bufsize);
     printf(m1266, dataset->tablename, nerr);
     for (i = 0; i < bufsize; i++)
       printf("%c", databuffer[i]);
     printf("\n");
     SQLFreeStmt(hstmt, SQL_DROP);
     SQLDisconnect(hdbc);
    }
   else
     return 1;
  }
 SQLFreeEnv(henv);
 SQLFreeConnect(hdbc);
 #endif
 return 0;
}

void disconnect_from_database()
{
 /*!Last Changed 29.12.2005*/
 #ifdef ODBC
 SQLFreeStmt(hstmt, SQL_DROP);
 /*SQLDisconnect(hdbc);*/
 SQLFreeEnv(henv);
 SQLFreeConnect(hdbc);
 #endif
}

void read_definition_from_database(Datasetptr dataset)
{
 /*!Last Changed 28.12.2005*/
 #ifdef ODBC
 RETCODE rc;        /* ODBC return code*/
 SQLINTEGER resultsize;
 int i, datacount, valueindex, count;
 double valuedouble;
 short int valueint;
 dataset->active = connect_to_database(dataset);
 if (dataset->active)
  {
   for (i = 0; i < dataset->classno; i++)
     dataset->classcounts[i] = 0;
   for (i = 0; i < dataset->featurecount; i++)
     switch (dataset->features[i].type)
      {
       case  INTEGER:dataset->features[i].min.intvalue = +INT_MAX;
                     dataset->features[i].max.intvalue = -INT_MAX;
                     break;
       case     REAL:
       case   REGDEF:dataset->features[i].min.floatvalue = +INT_MAX;
                     dataset->features[i].max.floatvalue = -INT_MAX;
                     break;
       case CLASSDEF:
       case   STRING:dataset->features[i].valuecount = 0;
                     break;
       default      :printf(m1238, dataset->features[i].type);
                     break;
      }
   rc = SQLFetch(hstmt);
   datacount = 0;
   while (rc == SQL_SUCCESS)
    {
     for (i = 0; i < dataset->featurecount; i++)
      {
       switch (dataset->features[i].type)
        {
         case  INTEGER:SQLGetData(hstmt, i + 1, SQL_C_SSHORT, &valueint, sizeof(valueint), &resultsize);
                       if (valueint < dataset->features[i].min.intvalue)
                         dataset->features[i].min.intvalue = valueint;
                       if (valueint > dataset->features[i].max.intvalue)
                         dataset->features[i].max.intvalue = valueint;
                       break;
         case     REAL:
         case   REGDEF:SQLGetData(hstmt, i + 1, SQL_C_DOUBLE, &valuedouble, sizeof(valuedouble), &resultsize);
                       if (valuedouble < dataset->features[i].min.floatvalue)
                         dataset->features[i].min.floatvalue = valuedouble;
                       if (valueint > dataset->features[i].max.floatvalue)
                         dataset->features[i].max.floatvalue = valuedouble;
                       break;
         case   STRING:SQLGetData(hstmt, i + 1, SQL_C_CHAR, databuffer, sizeof(databuffer), &resultsize);
                       count = dataset->features[i].valuecount;
                       valueindex = listindex(databuffer, dataset->features[i].strings, count);
                       if (valueindex == -1)
                        {
                         dataset->features[i].strings[count] = (char *)strcopy(dataset->features[i].strings[count], databuffer); 
                         dataset->features[i].valuecount++;
                        }
                       break;
         case CLASSDEF:SQLGetData(hstmt, i + 1, SQL_C_CHAR, databuffer, sizeof(databuffer), &resultsize);
                       count = dataset->features[i].valuecount;
                       valueindex = listindex(databuffer, dataset->features[i].strings, count);
                       if (valueindex == -1)
                        {
                         dataset->features[i].strings[count] = (char *)strcopy(dataset->features[i].strings[count], databuffer); 
                         dataset->features[i].valuecount++;
                        }
                       valueindex = listindex(databuffer, dataset->classes, dataset->classno);
                       if (valueindex != -1)
                         dataset->classcounts[valueindex]++;
                       break;
         default      :printf(m1238, dataset->features[i].type);
                       break;
        }
      }
     datacount++;
     rc = SQLFetch(hstmt);
    }
   dataset->datacount = datacount;
   dataset->multifeaturecount = 0;
   for (i = 0; i < dataset->featurecount; i++)
     switch (dataset->features[i].type)
      {
       case  INTEGER:
       case     REAL:  
       case   REGDEF:
       case CLASSDEF:dataset->multifeaturecount++;
                     break;
       case   STRING:dataset->multifeaturecount += dataset->features[i].valuecount;
                     break;
       default      :printf(m1238, dataset->features[i].type);
                     break;
      }
   dataset->newclassdefno = dataset->multifeaturecount;
   disconnect_from_database();
  }
#endif
}

Datasetptr load_definitions(char *fname, int* datasetcount)
{
 /*!Last Changed 13.03.2007 added selected for features*/
 /*!Last Changed 06.01.2007 added store_time*/
 /*!Last Changed 02.04.2005 added sigma estimate*/
 /*!Last Changed 10.03.2004 extended to regression*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 Xmldocumentptr doc;
 Xmlelementptr xmldataset, xmlclasses, xmlkernels, xmlattributes, xmlclass, xmlkernel, xmlattribute, xmlvalue;
 Datasetptr header = NULL, dataset = NULL;
 Datasetptr datasetbefore = NULL;
 int i, j, k, maxclass;
 char* st;
 doc = xml_document(fname);
 if (!doc)
   return NULL;
 *datasetcount = 0;
 if (xml_parse(doc))
  {
   xmldataset = doc->root->child;
   while (xmldataset)
    {
     (*datasetcount)++;
     dataset = (Datasetptr) safemalloc(sizeof(Dataset), "load_definitions", 20);
     dataset->active = YES;
     dataset->next = NULL;
     dataset->name = strcopy(dataset->name, xml_get_attribute_value(xmldataset, "name"));
     dataset->file = strcopy(dataset->file, xml_get_attribute_value(xmldataset, "filename"));
     dataset->directory = strcopy(dataset->directory, xml_get_attribute_value(xmldataset, "directory"));
     st = xml_get_attribute_value(xmldataset, "size");
     if (st)
       dataset->datacount = atoi(st);
     else
       dataset->datacount = 0;
     st = xml_get_attribute_value(xmldataset, "separator");
     if (st)
       dataset->separator = st[0];
     else
       dataset->separator = ' ';
     st = xml_get_attribute_value(xmldataset, "dsnname");
     if (st)
       dataset->dsnname = strcopy(dataset->dsnname, st);
     else
       dataset->dsnname = NULL;
     st = xml_get_attribute_value(xmldataset, "tablename");
     if (st)
       dataset->tablename = strcopy(dataset->tablename, st);
     else
       dataset->tablename = NULL;
     dataset->newclassdefno = -1;
     st = xml_get_attribute_value(xmldataset, "sigma");
     if (st)
       dataset->sigma = atof(st);
     else
       dataset->sigma = 0.0;
     st = xml_get_attribute_value(xmldataset, "filetype");
     if (strIcmp(st, "text") == 0)
       dataset->storetype = STORE_TEXT;
     else
       if (strIcmp(st, "database") == 0)
         dataset->storetype = STORE_DB;
       else
         if (strIcmp(st, "sequential") == 0)
           dataset->storetype = STORE_SEQUENTIAL;
         else
           if (strIcmp(st, "multilabel") == 0)
            {
             dataset->storetype = STORE_MULTILABEL;
             st = xml_get_attribute_value(xmldataset, "classcount");
             if (st)
               dataset->classno = atoi(st);
             else
               dataset->classno = 0;
             st = xml_get_attribute_value(xmldataset, "classlabelstart");
             if (st)
               dataset->classdefno = atoi(st);
             else
               dataset->classdefno = -1;
            }
           else
             if (strIcmp(st, "kernel") == 0)
              {
               dataset->storetype = STORE_KERNEL;
               xmlkernels = xml_get_child_with_name(xmldataset, "kernels");
               dataset->kernelcount = xml_number_of_children(xmlkernels);
               xmlkernel = xmlkernels->child;
               i = 0;
               while (xmlkernel)
                {
                 dataset->kernelfiles[i] = strcopy(dataset->kernelfiles[i], xml_get_attribute_value(xmlkernel, "value"));
                 xmlkernel = xmlkernel->sibling;
                 i++;
                }
              }
     printf(m1272, dataset->name);
     st = xml_get_attribute_value(xmldataset, "task");
     if (dataset->storetype != STORE_MULTILABEL)
      {
       if (strIcmp(st, "classification") == 0)
        {
         xmlclasses = xml_get_child_with_name(xmldataset, "classes");
         dataset->classno = xml_number_of_children(xmlclasses);
         if (dataset->classno > MAXCLASSES)
          {
           printf(m1419, dataset->classno);
           exit(0);
          }
         xmlclass = xmlclasses->child;
         i = 0;
         while (xmlclass)
          {
           dataset->classes[i] = strcopy(dataset->classes[i], xml_get_attribute_value(xmlclass, "value"));
           st = xml_get_attribute_value(xmlclass, "size");
           if (st)
             dataset->classcounts[i] = atoi(st);
           xmlclass = xmlclass->sibling;
           i++;
          }
         maxclass = find_max_in_list(dataset->classcounts, dataset->classno);
         dataset->baseerror = 100 * (1 - dataset->classcounts[maxclass] / (dataset->datacount + 0.0));
        }
       else
        {
         dataset->classno = 0;
         dataset->classdefno = -1;
         dataset->newclassdefno = -1;
        }
      }
     xmlattributes = xml_get_child_with_name(xmldataset, "attributes");
     dataset->allfeatures = xml_number_of_children(xmlattributes);
     if (dataset->allfeatures > MAXFEATURES)
      {
       printf(m1421, dataset->allfeatures);
       exit(0);
      }
     dataset->featurecount = xml_number_of_children_with_specific_attribute_value(xmlattributes, "available", "True");
     dataset->multifeaturecount = 0;
     xmlattribute = xmlattributes->child;
     i = 0;
     j = 0;
     while (xmlattribute)
      {
       st = xml_get_attribute_value(xmlattribute, "available");
       if (st && strIcmp(st, "True") == 0)
        {
         dataset->availability[i] = YES;
         dataset->features[j].selected = YES;
         st = xml_get_attribute_value(xmlattribute, "type");
         if (st)
          {
           dataset->features[j].type = listindex(st, featuretype, 4);
           if (dataset->features[j].type == CLASSDEF && dataset->classno == 0)
             dataset->features[j].type = REGDEF;
           switch (dataset->features[j].type)
            {
             case  INTEGER:st = xml_get_attribute_value(xmlattribute, "min");
                           if (st)
                             dataset->features[j].min.intvalue = atoi(st); 
                           else
                             dataset->features[j].min.intvalue = -INT_MAX;
                           st = xml_get_attribute_value(xmlattribute, "max");
                           if (st)
                             dataset->features[j].max.intvalue = atoi(st);
                           else
                             dataset->features[j].max.intvalue = +INT_MAX;
                           dataset->multifeaturecount++;
                           break;
             case   STRING:
             case CLASSDEF:dataset->features[j].valuecount = xml_number_of_children(xmlattribute);
                           if (dataset->features[j].valuecount > MAXVALUES)
                            {
                             printf(m1420, j + 1);
                             exit(0);
                            }
                           if (dataset->features[j].type == STRING)
                             dataset->multifeaturecount += dataset->features[j].valuecount;
                           else
                            {
                             dataset->multifeaturecount++;
                             dataset->classdefno = j;
                             dataset->newclassdefno = dataset->multifeaturecount - 1;
                            }
                           xmlvalue = xmlattribute->child;
                           k = 0;
                           while (xmlvalue)
                            {
                             dataset->features[j].strings[k] = strcopy(dataset->features[j].strings[k], xmlvalue->pcdata);
                             xmlvalue = xmlvalue->sibling;
                             k++;
                            }
                           break;
             case     REAL:
             case   REGDEF:st = xml_get_attribute_value(xmlattribute, "min");
                           if (st)
                             dataset->features[j].min.floatvalue = atof(st); 
                           else
                             dataset->features[j].min.floatvalue = -INT_MAX;
                           st = xml_get_attribute_value(xmlattribute, "max");
                           if (st)
                             dataset->features[j].max.floatvalue = atof(st);
                           else
                             dataset->features[j].max.floatvalue = +INT_MAX;
                           dataset->multifeaturecount++;
                           if (dataset->features[j].type == REGDEF)
                            {
                             dataset->classdefno = j;
                             dataset->newclassdefno = dataset->multifeaturecount - 1;
                            }
                           break;
            }
          }
         else
           dataset->features[j].type = INTEGER;
         j++;
        }
       else
         dataset->availability[i] = NO;
       xmlattribute = xmlattribute->sibling;
       i++;
      }
     if (dataset->storetype == STORE_KERNEL)
       dataset->kernel = NULL;
     if (*datasetcount == 1)
       header = dataset;
     if (datasetbefore)
       datasetbefore->next = dataset;
     datasetbefore = dataset;
     xmldataset = xmldataset->sibling;
    }
  }
 else
   printf(m1052);
 xml_free_document(doc);
 return header;
}

void print_cmu_data_header(FILE* outfile, Datasetptr d)
{
 /*!Last Changed 02.05.2007 added regression*/
 /*!Last Changed 21.04.2007*/
 int i;
 fprintf(outfile, "$SETUP\n\n");
 fprintf(outfile, "PROTOCOL:  IO;\n");
 fprintf(outfile, "OFFSET:     0;\n");
 fprintf(outfile, "INPUTS:     %d;\n", d->multifeaturecount - 1);
 if (d->classno != 0)
   fprintf(outfile, "OUTPUTS:    %d;\n\n", d->classno);
 else
   fprintf(outfile, "OUTPUTS:    1;\n\n");
 for (i = 0; i < d->multifeaturecount - 1; i++)
   fprintf(outfile, "IN [%d]: CONT;\n", i + 1);
 fprintf(outfile, "\n");
 if (d->classno != 0)
   for (i = 0; i < d->classno; i++)
     fprintf(outfile, "OUT [%d]: BINARY;\n", i + 1);
 else
   fprintf(outfile, "OUT [1]: CONT;\n");
 fprintf(outfile, "\n\n");
}

void display_dataset_properties(char *dataset_name)
{
 /*!Last Changed 06.01.2007*/
 /*!Last Changed 28.12.2005 added database properties*/
 /*!Last Changed 10.03.2004 Extended to regression*/
 int i,j;
 char pst[READLINELENGTH];
 Datasetptr dataset;
 dataset = search_dataset(Datasets, dataset_name);
 if (dataset)
  {
   fprintf(output,"----------DATASET PROPERTIES----------\n");
   fprintf(output,"NAME        : %s\n",dataset->name);
   fprintf(output,"FEATURES    : %d\n",dataset->featurecount);
   for (i=0;i<dataset->featurecount;i++)
     switch (dataset->features[i].type){
       case INTEGER :fprintf(output,"FEATURE #%d : INTEGER, MINVALUE:%d, MAXVALUE:%d\n",i+1,dataset->features[i].min.intvalue,dataset->features[i].max.intvalue);
                     break;
       case REAL    :fprintf(output,"FEATURE #%d : REAL, MINVALUE:%f, MAXVALUE:%f\n",i+1,dataset->features[i].min.floatvalue,dataset->features[i].max.floatvalue);
                     break;
       case STRING  :fprintf(output,"FEATURE #%d : STRING,",i+1);
                     for (j=0;j<dataset->features[i].valuecount;j++)
                       fprintf(output,"VALUE%d:%s ",j+1,dataset->features[i].strings[j]);
                     fprintf(output,"\n");
                     break;
       case REGDEF  :fprintf(output,"FEATURE #%d : REGRESSION DEFINITION\n",i+1);
                     break;
       case CLASSDEF:fprintf(output,"FEATURE #%d : CLASS DEFINITION\n",i+1);
                     break;
       default      :printf(m1238, dataset->features[i].type);
                     break;
     }
   fprintf(output, "CLASS NUMBER: %d\n", dataset->classno);
   for (i = 0; i < dataset->classno; i++)
     fprintf(output,"CLASS #%d   : %s\n",i + 1, dataset->classes[i]);
   if (in_list(dataset->storetype, 3, STORE_TEXT, STORE_SEQUENTIAL, STORE_KERNEL))
    {
     fprintf(output,"DIRECTORY   : %s\n", dataset->directory);
     fprintf(output,"FILE        : %s\n", dataset->file);
    }
   else
    {
     fprintf(output,"DSN         : %s\n", dataset->dsnname);
     fprintf(output,"TABLE NAME  : %s\n", dataset->tablename);
    }
   fprintf(output,"DATA COUNT  : %d\n", dataset->datacount);
   for (i = 0;i < dataset->classno; i++)
     fprintf(output,"CLASS #%d COUNT: %d\n", i + 1, dataset->classcounts[i]);
   fprintf(output, "CLASSDEFNO: %d\n", dataset->classdefno);
   set_precision(pst, "SIGMA :", "\n");
   fprintf(output, pst, dataset->sigma);
  }
 else
   fprintf(output,"Dataset %s in not in the definition file\n",dataset_name);
}

/*oya -- complexity measures of supervised classification problems, Ho & Basu
20.4.2006*/

double fisher_discriminant_ratio(Datasetptr d)
{
 /* multiclass is handled by one vs one */
 Instanceptr tempinst, traindata, c_means;
 int *c_counts;
 matrix class_mean, class_cov, class_var, fisher_class_feature;
 int i, j, k, f;
 double result = 0, classsum, maxratio; 
 read_from_train_file(d, &traindata);
 realize(traindata);
 exchange_classdefno(d);
 c_counts = find_class_counts(traindata);
 c_means = find_class_means(traindata, c_counts);
 fisher_class_feature = matrix_alloc(d->multifeaturecount - 1, d->classno * d->classno);
 class_mean = matrix_alloc(d->multifeaturecount - 1, d->classno);
 class_var = matrix_alloc(d->multifeaturecount - 1, d->classno);
 tempinst = c_means;
 for (j = 0; j < d->classno; j++)
  {
   class_cov = class_covariance(traindata, j, tempinst);  /*OLCAY DEGISTIRDIM*/
   for (i = 0, f = 0; i < d->multifeaturecount; i++)
    {
     if (i != d->classdefno)
      {
       class_var.values[f][j] = class_cov.values[f][f];
       class_mean.values[f][j] = tempinst->values[f].floatvalue;
       f++;
      }
    }
   tempinst = tempinst->next;
   matrix_free(class_cov);
  }
 maxratio = 0;
 for (i = 0, f = 0;i < d->multifeaturecount; i++)
  {
   if (i != d->classdefno)
    {
     classsum = 0;
     for (j = 0; j < d->classno; j++)
      {
       for (k = j + 1; k < d->classno; k++)
        {
         classsum += (class_mean.values[f][j] - class_mean.values[f][k]) * (class_mean.values[f][j] - class_mean.values[f][k]) / ((class_var.values[f][j] * class_var.values[f][j]) + (class_var.values[f][k] * class_var.values[f][k]));
        }
      }
     if (classsum > maxratio)
      {
       maxratio = classsum;
      }
     f++;
    }
  }
 exchange_classdefno(d);
 matrix_free(fisher_class_feature);
 matrix_free(class_mean);
 matrix_free(class_var);
 safe_free(c_means);
 safe_free(c_counts);
 deallocate(traindata);
 traindata = NULL;
 result = maxratio / (d->classno * (d->classno - 1) / 2);
 return result;
}

double volume_overlap_region(Datasetptr d)
{
 /* multiclass is handled by one vs one */
 Instanceptr traindata;
 int i, j, k;
 double result = 0, classsum, min1, max1, min2, max2;
 read_from_train_file(d, &traindata);
 realize(traindata);
 exchange_classdefno(d);
 result = 1;
 for (i = 0; i < d->multifeaturecount; i++)
  {
   if (i != d->classdefno)
    {   
     classsum = 0;
     for (j = 0; j < d->classno; j++)
      {
       for (k = j + 1; k < d->classno; k++)
        {
         find_bounds_of_feature_class(traindata, i, j, &min1, &max1);
         find_bounds_of_feature_class(traindata, i, k, &min2, &max2);
         classsum += (fmin(max1, max2) - fmax(min1, min2)) / (fmax(max1, max2) + fmin(min1, min2));
        }
      }
     result *= (classsum / (d->classno * (d->classno - 1) / 2));
    }
  }
 exchange_classdefno(d); 
 deallocate(traindata);
 traindata = NULL;
 return result;
}

double feature_efficiency(Datasetptr d, double *overlap)
{
 /* multiclass is handled by one vs one */
 Instanceptr traindata;
 int i, j, k, ccount=0;
 double result = 0, maxefficiency, min1, max1, min2, max2;
 double efficiency, binoverlap;
 read_from_train_file(d, &traindata);
 realize(traindata);
 exchange_classdefno(d);
 maxefficiency = 0;
 *overlap = 1;
 for (i = 0; i < d->multifeaturecount; i++)
  {
   if (i != d->classdefno)
    {
     binoverlap = 0;
     efficiency = 0;
     for (j = 0; j < d->classno; j++)
      {
       for (k = j + 1; k < d->classno; k++)
        {
         find_bounds_of_feature_class(traindata, i, j, &min1, &max1);
         find_bounds_of_feature_class(traindata, i, k, &min2, &max2);
         ccount = 0;
         if (max1 > max2)
           ccount += find_thresholded_class_count(traindata, j, i, max2, UPPER);
         else
           ccount += find_thresholded_class_count(traindata, k, i, max1, UPPER);     
         if (min1 > min2)
           ccount += find_thresholded_class_count(traindata, k, i, min1, LOWER);
         else
           ccount += find_thresholded_class_count(traindata, j, i, min2, LOWER);     
         binoverlap += (fmin(max1, max2) - fmax(min1, min2)) / (fmax(max1, max2) + fmin(min1, min2));
         efficiency += ((double) ccount / (d->classcounts[j] + d->classcounts[k]));     
        }
      }
     *overlap *= (binoverlap / (d->classno * (d->classno - 1) / 2));
     if (efficiency > maxefficiency)
       maxefficiency = efficiency;
    }
  }
 result = (maxefficiency / (d->classno * (d->classno - 1) / 2));
 exchange_classdefno(d);
 deallocate(traindata);
 traindata = NULL;
 return result;
}

double nn_ratio(Datasetptr d, double *estimate)
{
 /* multiclass is handled by one vs one */
 Instance mean, variance;
 Instanceptr temp, traindata, *nearestneighbors;
 int count, cno1, cno2;
 double result = 0;
 double intraclass = 0, interclass = 0, dist;
 int intranum = 0, internum = 0;
 read_from_train_file(d, &traindata);
 mean = find_mean(traindata);
 variance = find_variance(traindata);
 normalize(traindata, mean, variance);
 type_change(d);
 temp = traindata;
 count = 2;
 while(temp)
  {
   nearestneighbors = find_nearest_neighbors(traindata, temp, count);
   dist = distance_between_instances(nearestneighbors[1], temp);
   cno1 = give_classno(nearestneighbors[1]);
   cno2 = give_classno(temp);
   if (cno1 == cno2)
    {
     intraclass += dist;
     intranum++;
    }
   else
    {
     interclass += dist;
     internum++;
    }
   temp = temp->next;
   safe_free(nearestneighbors);
  }
 result = intraclass / interclass;
 *estimate = (double) internum / (internum + intranum); 
 safe_free(mean.values);
 safe_free(variance.values);
 type_restore(d);
 deallocate(traindata);
 traindata = NULL;
 return result;
}

double find_opposite_class_ratio_in_mst(Datasetptr d)
{
 /* usedp=>how many points already used 
    p->array of structures, consisting x,y,& used/not used
    this problem is to get the MST of graph with n vertices
    where weight of an edge is the distance between 2 points */
 int usedp, smallp = -1, opposite_class = 0, i, j, ci, cj;
 matrix lengths;
 int *classes, *used;
 Instanceptr traindata;
 double small, length, minLength=0, result;
 Instance mean, variance;
 read_from_train_file(d, &traindata);
 mean = find_mean(traindata);
 variance = find_variance(traindata);
 normalize(traindata, mean, variance);
 type_change(d);
 classes = find_classes(traindata);
 lengths = distance_matrix(traindata);
 used = safecalloc(d->datacount, sizeof(int), "find_opposite_class_ratio_in_mst", 21);
 used[0] = 1;  /* select arbitrary point as starting point */
 usedp = 1;
 while (usedp < d->datacount) 
  {
   small = -1.0;   
   for (i = 0; i < d->datacount; i++)
    {
     if (used[i])
      {
       for (j = 0; j < d->datacount; j++)
        {
         if (!used[j]) 
          {
           length = lengths.values[i][j];
           if (small == -1.0 || length < small) 
            {
             small = length;
             smallp = j;
             ci = classes[i];
             cj = classes[j];
            }
          }    
        }
      }
    }
   minLength += small;
   used[smallp] = 1;
   usedp++;
   if (ci != cj)
     opposite_class++;
  }
 matrix_free(lengths);
 safe_free(used);
 safe_free(classes);
 safe_free(mean.values);
 safe_free(variance.values);
 result = (double) opposite_class / d->datacount;
 type_restore(d);
 deallocate(traindata);
 traindata = NULL;
 return result;
}

Instanceptr create_linear_test_set(Datasetptr d, int numexp, int mustrealize)
{
 /*!Last Changed 07.05.2006 Oya*/
 Instanceptr temptr, traindata = NULL, testdata = NULL, tempinst1, tempinst2, currentinstance;
 Instanceptr classdata = NULL, rest = NULL;
 int i, j, k, stratifiedcount, d1, d2;
 double prm;
 read_from_train_file(d, &traindata);
 if (mustrealize)
  {
   realize(traindata);
   exchange_classdefno(d);
  }
 rest = traindata;
 k = 0;
 for (i = 0; i < d->classno; i++)
  {
   temptr = rest;
   rest = NULL;
   divide_instancelist_one_vs_rest(&temptr, &classdata, &rest, i);
   stratifiedcount = (int) (((d->classcounts[i] + 0.0) / d->datacount) * numexp);
   /*generate stratifiedcount many examples from class i*/
   for (j = 0; j < stratifiedcount; j++)
    {
     /*chose two random points from class i*/
     d1 = myrand() % d->classcounts[i];
     d2 = myrand() % d->classcounts[i];
     prm = produce_random_value(0.0, 1.0);
     tempinst1 = data_x(classdata, d1);
     tempinst2 = data_x(classdata, d2);
     /*find new point by linear interpolation*/
     currentinstance = find_linear_interpolation_data(tempinst1, tempinst2, prm);
     /*add it to the test set*/
     currentinstance->next = testdata;
     currentinstance->index = k;
     testdata = currentinstance;
     k++;
    }
   deallocate(classdata);
   classdata = NULL;
  }
 if (mustrealize)
   exchange_classdefno(d);
 return testdata;
}

double nn_nonlinearity(Datasetptr d, Instanceptr testdata)
{
 /*!Last Changed 07.05.2006 Oya*/
 Instance mean, variance;
 Instanceptr temp, traindata, *nearestneighbors;
 int count, cno1, cno2;
 double result = 0;
 int intranum = 0, internum = 0;
 read_from_train_file(d, &traindata);
 mean = find_mean(traindata);
 variance = find_variance(traindata);
 normalize(traindata, mean, variance);
 normalize(testdata, mean, variance);
 type_change(d);
 temp = testdata;
 count = 1;
 while (temp)
  {
   nearestneighbors = find_nearest_neighbors(traindata, temp, count);
   cno1 = give_classno(nearestneighbors[0]);
   cno2 = give_classno(temp);
   if (cno1 == cno2)
     intranum++;
   else
     internum++;
   temp = temp->next;
   safe_free(nearestneighbors);
  }
 result = (double) internum / (internum + intranum);
 safe_free(mean.values);
 safe_free(variance.values);
 deallocate(traindata);
 type_restore(d);
 return result;
}

double adherentsubsets(Datasetptr d)
{
 /*!Last Changed 09.05.2006 Oya*/
 Instance mean, variance;
 Instanceptr temp, traindata, *nearestneighbors;
 int count = 0, cno1, cno2;
 double result = 0, *order;
 int j, i, sumdifferent, different, *selected, *dataindex;
 matrix allsubsets;
 allsubsets = matrix_alloc(d->datacount, d->datacount);
 initialize_matrix(&allsubsets, 0);
 dataindex = (int *) safecalloc(d->datacount, sizeof(int), "adherentsubsets", 9);
 order = (double *) safecalloc(d->datacount, sizeof(double), "adherentsubsets", 10);
 selected = (int *) safecalloc(d->datacount, sizeof(int), "adherentsubsets", 11);
 read_from_train_file(d, &traindata);
 mean = find_mean(traindata);
 variance = find_variance(traindata);
 normalize(traindata, mean, variance);
 type_change(d);
 temp = traindata;
 j = 0;
 while (temp)
  {
   dataindex[j] = j; 
   cno2 = give_classno(temp);
   allsubsets.values[j][j] = 1;
   nearestneighbors = find_nearest_neighbors(traindata, temp, d->classcounts[cno2] + 1);
   for (i = 1; i < d->classcounts[cno2] + 1; i++)
    {   
     cno1 = give_classno(nearestneighbors[i]);
     if (cno1 == cno2)
      {
       allsubsets.values[j][nearestneighbors[i]->index] = 1;
       order[j] = order[j] + 1; 
      }
     else
      {
       break;
      }
    }
   temp = temp->next;
   j++;
   safe_free(nearestneighbors);
  }
 sort_two_arrays(order, dataindex, d->datacount);
 sumdifferent = 0;
 count = 0;
 for (i = d->datacount - 1; i >= 0; i--)
  {
   different = 0;
   for (j = 0; j < d->datacount; j++)
    {
     if (allsubsets.values[i][j] == 1)
       if (selected[j] == 0)
        {
         selected[j] = 1;
         different++;
        }
     }
   if (different != 0)
     count++;
   sumdifferent += different;
   if (sumdifferent == d->datacount)
     break;
  }
 safe_free(selected);
 safe_free(dataindex);
 safe_free(order);
 safe_free(mean.values);
 safe_free(variance.values);
 matrix_free(allsubsets);
 result = (double) count / d->datacount;
 deallocate(traindata);
 type_restore(d);
 return result;
}

double lp_separability(Datasetptr d, double* trainerror, double* testerror)
{
 /*!Last Changed 10.05.2006 Oya*/
 Instanceptr traindata, testdata;
 Instance mean, variance;
 double result, loglikelihood, losstrain = 0, losstest = 0;
 int performance;
 Mlpparameters mlpparams;
 Mlpnetwork neuralnetwork;
 matrix train, test; 
 testdata = create_linear_test_set(d, d->datacount / 2, YES);
 read_from_train_file(d, &traindata);
 realize(traindata);
 exchange_classdefno(d);
 mean = find_mean(traindata);
 variance = find_variance(traindata);
 normalize(traindata, mean, variance);
 normalize(testdata, mean, variance);               
 mlpparams = prepare_Mlpparameters(traindata, traindata, 0, 0.0, 0, NULL, 0, 0.0, 0.0, 0, CLASSIFICATION);
 train = instancelist_to_matrix(traindata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 test = instancelist_to_matrix(testdata, CLASSIFICATION, -1, MULTIVARIATE_LINEAR);
 neuralnetwork = mlpn_general(train, train, &mlpparams, AUTOENCODER);
 neuralnetwork.P = mlpparams;
 performance = testMlpnNetwork2_cls(train, neuralnetwork, &loglikelihood, &losstrain);
 *trainerror = 1 - (performance / (train.row + 0.0));
 neuralnetwork.P = mlpparams;
 performance = testMlpnNetwork2_cls(test, neuralnetwork, &loglikelihood, &losstest); 
 *testerror = 1 - (performance / (test.row + 0.0));
 free_mlpnnetwork(&neuralnetwork);               
 safe_free(mean.values);
 safe_free(variance.values);
 matrix_free(train);
 matrix_free(test);
 exchange_classdefno(d);
 deallocate(traindata); 
 deallocate(testdata); 
 result = losstrain;
 return result;
}
