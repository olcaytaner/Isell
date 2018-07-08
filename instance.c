#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "messages.h"
#include "parallel.h"
#include "parameter.h"
#include "regressiontree.h"
#include "statistics.h"

extern int mustrealize;
extern Datasetptr current_dataset;

/**
 * Calculates the number of frames for a time-variant instance (number of observations in an instance).
 * @param[in] Time-variant instance
 * @return Number of frames in an instance
 */
int frame_count(Instanceptr inst)
{
	/*!Last Changed 07.01.2007*/
	int count = 1;
	Instanceptr tmp = inst->sublist;
	while (tmp)
	 {
		 tmp = tmp->next;
			count++;
	 }
	return count;
}

/**
 * Converts an instance structure to an SVM_node structure used in LIBSVM.
 * @param[in] inst Instance to be converted
 * @return LIBSVM node structure
 */
Svm_nodeptr instance_to_svmnode(Instanceptr inst)
{
	/*!Last Changed 02.05.2005*/
	int i, j = 0;
	Svm_nodeptr result;
 double value;
	result = safemalloc(sizeof(Svm_node) * current_dataset->multifeaturecount, "instance_to_svmnode", 3);
	for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
	 {
   value = real_feature(inst, i);
   if (value != 0.0 || current_dataset->storetype == STORE_KERNEL)
    {
  		 result[j].index = i;
		  	result[j].value = value;
     j++;
    }
	 }
	result[j].index = -1;
	result[j].value = -1;
 j++;
 result = alloc(result, sizeof(Svm_node) * j, j);
	return result;
}

/**
 * Generates a copy of an instance. Allocates memory for the features and copies the contents of the features including the class definition feature.
 * @param[in] inst Instance
 * @return New copy of the instance
 */
Instanceptr copy_of_instance(Instanceptr inst)
{
	/*!Last Changed 19.02.2005*/
	Instanceptr currentinstance;
	int i;
	if (!inst)
		 return NULL;
 currentinstance = (Instanceptr)safemalloc(sizeof(Instance), "copy_of_instance", 5);
 currentinstance->values = (union value*)safemalloc(current_dataset->multifeaturecount * sizeof(union value), "copy_of_instance", 6);
	for (i = 0; i < current_dataset->multifeaturecount; i++)
		 if (i != current_dataset->classdefno || current_dataset->classno == 0)
				 currentinstance->values[i].floatvalue = inst->values[i].floatvalue;
			else	 
				 currentinstance->values[i].stringindex = inst->values[i].stringindex;
 currentinstance->next = NULL;
 currentinstance->sublist = NULL;
 if (current_dataset->storetype == STORE_MULTILABEL)
  {
   currentinstance->class_labels = safemalloc(current_dataset->classno * sizeof(int), "copy_of_instance", 15);
   for (i = 0; i < current_dataset->classno; i++)
     currentinstance->class_labels[i] = inst->class_labels[i];  
  }
 else
   currentinstance->class_labels = NULL;
	return currentinstance;
}

/**
 * Generates a 2-dimensional copy of the instance. A 2-dimensional copy of an instance is obtained via the formula (x_1 + x_2 + \ldots + x_d + 1)^2. Allocates memory for each feature. 
 * @param[in] inst Instance
 * @warning Does not contain the class definition feature.
 * @return 2-dimensional copy of the instance. The new instance contains squares of the features x_i^2, two features multiplied x_ix_j and original features x_i.
 */
Instanceptr generate_2d_copy_of_instance(Instanceptr inst)
{
	/*!Last Changed 24.03.2005*/
 Instanceptr newinst;
	int i, j, x, dim = current_dataset->multifeaturecount - 1;
	newinst = empty_instance(multifeaturecount_2d(current_dataset) + 1);
	x = 0;
 for (i = 0; i < dim; i++)
	  for (j = i; j < dim; j++)
				{
				 if (x == current_dataset->classdefno)
						 x++;
		  	newinst->values[x].floatvalue = (double) (real_feature(inst, i) * real_feature(inst, j));
					x++;
				}
 for (i = 0; i < dim; i++)
		{
			if (x == current_dataset->classdefno)
					x++;
			newinst->values[x].floatvalue = (double) real_feature(inst, i);
			x++;
		}
	return newinst;
}

/**
 * Multiplies an instance with a one row matrix (dot product). There are two cases: In the first case (linear case), each feature is multiplied with the corresponding matrix value (x_i * M[i]). In the second case (quadratic case), one thinks each instance as a 2-d instance and multiplies with the matrix values accordingly. A 2-d instance is obtained via the formula (x_1 + x_2 + \ldots + x_d + 1)^2 and contains squares of the features x_i^2, two features multiplied x_ix_j and original features x_i.
 * @param[in] inst Instance
 * @param[in] m Matrix with one row
 * @return Dot product of the instance with the matrix
 */
double multiply_with_matrix(Instanceptr inst, matrix m)
{
	/*!Last Changed 29.01.2005*/
	/*!Last Changed 09.01.2004*/
	double result = 0;
	int x, i, j = 0, dim = current_dataset->multifeaturecount - 1;
	if (m.col == dim) /*LINEAR MODEL*/
	 {
  	for (i = 0; i < dim; i++)
		  	result += m.values[0][i] * real_feature(inst, i);
	 }
	else /* QUADRATIC MODEL */
	 {
		 x = 0;
		 for (i = 0; i < dim; i++)
  			for (j = i; j < dim; j++)
					 {
						 result += m.values[0][x] * real_feature(inst, i) * real_feature(inst, j);
							x++;
					 }
   for (i = 0; i < dim; i++)
			 {
				 result += m.values[0][x] * real_feature(inst, i);
					x++;
			 }
	 }
	return result;
}

/**
 * Converts instance to a one row matrix.
 * @param[in] inst Instance
 * @return One row matrix
 */
matrix instance_to_matrix(Instanceptr inst)
{
	/*!Last Changed 09.01.2004*/
	int i, j = 0;
	matrix result;
	result = matrix_alloc(1, current_dataset->multifeaturecount - 1);
	for (i = 0; i < current_dataset->multifeaturecount; i++)
		 if (i != current_dataset->classdefno)
			 {
				 result.values[0][j] = inst->values[i].floatvalue;
					j++;
			 }
 return result;
}

void print_instance_with_extra_features(FILE* outfile, char s, Datasetptr d, Instanceptr currentinstance, int extra_features)
{
	int i, sindex;
	char pst[READLINELENGTH];
 for (i = 0; i < d->featurecount; i++)
   switch (d->features[i].type)
    {
     case  INTEGER:if (currentinstance->values[i].intvalue == ISELL)
                     fprintf(outfile,"?%c", s);
                   else
                     fprintf(outfile,"%d%c", currentinstance->values[i].intvalue, s);
                   break;
     case     REAL:if (currentinstance->values[i].floatvalue == ISELL)
                     fprintf(outfile,"?%c", s);
                   else
                     fprintf(outfile, "%5.6f%c", currentinstance->values[i].floatvalue, s);
                   break;
     case   STRING:sindex = currentinstance->values[i].stringindex;
                   if (sindex == -1)
                     fprintf(outfile, "?%c", s);
                   else
                     fprintf(outfile, "%s%c", d->features[i].strings[sindex], s);
                   break;
     case CLASSDEF:break;
     default      :printf(m1238, d->features[i].type);
                   break;
    }
 for (i = 0; i < extra_features; i++)
   fprintf(outfile, "%5.6f%c", produce_normal_value(0, 1), s);
 if (d->classno != 0)
   fprintf(outfile,"%s", d->features[d->classdefno].strings[give_classno(currentinstance)]);
 else
   if (d->classdefno != -1)
    {
     set_precision(pst, NULL, NULL);
     fprintf(outfile, pst, give_regressionvalue(currentinstance));                  
    }
 fprintf(outfile,"\n");
}

/**
 * Prints a single instance to an output stream. The format of the output is determined by the program parameter namestype.
 * @param[in] outfile Output file stream.
 * @param[in] s Character for separating the features in the output file
 * @param[in] d Dataset of the instance
 * @param[in] currentinstance Instance to be printed
 */
void print_instance(FILE* outfile, char s, Datasetptr d, Instanceptr currentinstance)
{
 /*!Last Changed 13.02.2008 added regression datasets for !mustrealize*/
 /*!Last Changed 16.10.2005 updated to include the new namestype classatend*/
	/*!Last Changed 13.10.2004 updated to include the new namestype classic*/
	/*!Last Changed 10.03.2004 extended to regression*/
	/*!Last Changed 28.10.2003*/
	int i, sindex;
	double value;
	char pst[READLINELENGTH];
	if (!mustrealize)
	 {
   for (i = 0; i < d->featurecount; i++)
				 if (d->features[i].selected)
					 {
       switch (d->features[i].type)
					   {
         case  INTEGER:if (currentinstance->values[i].intvalue == ISELL)
					    			         			 fprintf(outfile,"?%c", s);
             						    else
             						      fprintf(outfile,"%d%c", currentinstance->values[i].intvalue, s);
             						    break;
  		  			case   REGDEF:
         case     REAL:if (currentinstance->values[i].floatvalue == ISELL)
             											 fprintf(outfile,"?%c", s);
             										else
             												fprintf(outfile, "%5.6f%c", currentinstance->values[i].floatvalue, s);
             						    break;
         case   STRING:
	    				case CLASSDEF:if (in_list(get_parameter_l("namestype"), 3, NAMES_RIPPER, NAMES_CLASSATEND, NAMES_AYSU) && d->features[i].type == CLASSDEF)
		    																			continue;
			  	  											    sindex = currentinstance->values[i].stringindex;
             						    if (sindex == -1)
             												fprintf(outfile, "?%c", s);
           	  									else
           			  			      fprintf(outfile, "%s%c", d->features[i].strings[sindex], s);
           					  	    break;
         default      :printf(m1238, d->features[i].type);
                       break;
					   }
				  }
   if (d->classno != 0)
    {/*Classification*/
     sindex = give_classno(currentinstance);
     if (sindex != -1 && in_list(get_parameter_l("namestype"), 3, NAMES_RIPPER, NAMES_CLASSATEND, NAMES_AYSU))
      {
       if (get_parameter_l("namestype") == NAMES_RIPPER)
         fprintf(outfile, "c%s", d->features[d->classdefno].strings[sindex]);
       else
         if (get_parameter_l("namestype") == NAMES_CLASSATEND)
           fprintf(outfile,"%s", d->features[d->classdefno].strings[sindex]);
         else
           fprintf(outfile,"%d", sindex);     
      }
    }
	 }
	else
	 {
		 for (i = 0; i < d->multifeaturecount - 1; i++)
				 if (d->selected[i])
					 {
				   value = real_feature(currentinstance, i);
       if (value == ISELL)
         fprintf(outfile, "?%c", s);
       else
							 {
								 if (get_parameter_l("namestype") == NAMES_CMU && i == d->multifeaturecount - 2)
									 {
										 set_precision(pst, NULL, "=>");
           fprintf(outfile, pst, value);
									 }
									else
									 {
										 set_precision(pst, NULL, "%c");
           fprintf(outfile, pst, value, s);
									 }
							 }
					 }
			if (d->classno != 0)
			 {/*Classification*/
     sindex = give_classno(currentinstance);
	  		if (in_list(get_parameter_l("namestype"), 2, NAMES_RIPPER, NAMES_CLASSATEND))
		  		 exchange_classdefno(current_dataset);
	  		if (get_parameter_l("namestype") == NAMES_RIPPER)
    			fprintf(outfile, "c%s", d->features[d->classdefno].strings[sindex]);
		  	else
			  	 if (get_parameter_l("namestype") == NAMES_CLASSATEND)
  		  			fprintf(outfile, "%s", d->features[d->classdefno].strings[sindex]);
		  			else
			  			 if (in_list(get_parameter_l("namestype"), 3, NAMES_C50, NAMES_CLASSIC, NAMES_AYSU))
				  		   fprintf(outfile, "%d", sindex);
			  				else
									 {								 
  								 for (i = 0; i < d->classno - 1; i++)
	  									 if (i == sindex)
		  										 fprintf(outfile, " +,");
			  								else
				  								 fprintf(outfile, " -,");
					  				if (sindex == d->classno - 1)
						  				 fprintf(outfile, " +;");
						  			else
							  			 fprintf(outfile, " -;");
									 }
			  if (in_list(get_parameter_l("namestype"), 2, NAMES_RIPPER, NAMES_CLASSATEND))
				   exchange_classdefno(current_dataset);
			 }
			else
     if (d->classdefno != -1) 
      {/*Regression*/
					  value = give_regressionvalue(currentinstance);
				   if (get_parameter_l("namestype") == NAMES_CMU)
        {
         set_precision(pst, NULL, ";");
						   fprintf(outfile, pst, value);        
        }
       else 
         if (get_parameter_l("namestype") == NAMES_CLASSATEND)
          {
           set_precision(pst, NULL, NULL);
           fprintf(outfile, pst, value);                  
          }
      }
	 }
	if (in_list(get_parameter_l("namestype"), 2, NAMES_C50, NAMES_RIPPER))
  	fprintf(outfile,".\n");
	else
			fprintf(outfile,"\n");
}

/**
 * Generates a stratified partition info array from prior class information data array.
 * @param[in] prior_class_information Class index of each instance. prior_class_information[i] is the classno (class index) of the i'th instance
 * @param[in] datasize Number of examples (Size of the prior_class_information array)
 * @param[in] classnumber Number of classes in the problem (Size of the classcounts array)
 * @param[in] classcounts Number of examples in each class. classcounts[i] is the number of examples whose class index is i.
 * @return An array of partition. result[i] shows the index of i'th instance considering only its acquantices (the instances in the same class)
 */
int* stratified_partition(int *prior_class_information, int datasize, int classnumber, int* classcounts)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
	/*!Last Changed 23.11.2003*/
 long i,j,tmp;
	int* classes, classno, **indexes, *positions, *result;
	if (classnumber > 0)
	 {
  	positions = (int*)safecalloc(classnumber,sizeof(int), "stratified_partition", 5);
  	classes = (int*)safecalloc(classnumber,sizeof(int), "stratified_partition", 6);
	  indexes = safemalloc(classnumber * sizeof(int *), "stratified_partition", 7);
			for (i = 0; i < classnumber; i++)
				 indexes[i] = safemalloc(classcounts[i] * sizeof(int), "stratified_partition", 9);
   result = (int *)safemalloc(datasize * sizeof(int), "stratified_partition", 10);
   for (i = 0; i < datasize;i++)
			 {
  		 classno = prior_class_information[i];
	  		indexes[classno][positions[classno]] = i;
     result[i] = classes[classno];
			  (classes[classno])++;
			  (positions[classno])++;
			 }
   for (i = 0; i < datasize; i++)
			 {
  		 classno = prior_class_information[i];
     j = myrand() % classcounts[classno];
		  	j = indexes[classno][j];
     tmp = result[i];
     result[i] = result[j];
     result[j] = tmp;
			 }
	  free_2d((void**)indexes, classnumber);
  	safe_free(positions);
  	safe_free(classes);
	 }
	else
		 result = random_array(datasize);
	return result;
}

/**
 * If the dataset contains discrete features, the class definition feature is after one or more discrete features and if we apply discrete-to-continuous transform the position of the class definition feature will change. Using this function one can get the feature with an index after discrete-to-continuous transform.
 * @param[in] myptr Instance
 * @param[in] fno Feature index
 * @return Value of the feature
 */
double real_feature(Instanceptr myptr, int fno)
{
 if (fno < current_dataset->classdefno)
   return myptr->values[fno].floatvalue;
 else
   return myptr->values[fno + 1].floatvalue;
}

/**
 * Calculates the euclidian distance between two instances before normalization of the features.
 * @param[in] inst1 First instance
 * @param[in] inst2 Second instance
 * @param[in] mean Mean vector of the data
 * @param[in] variance Standard deviation vector of the data
 * @return Square of the euclidian distance between two instances
 */
double distance_between_instances_before_normalize(Instanceptr inst1,Instanceptr inst2, Instance mean, Instance variance)
{
	/*!Last Changed 17.05.2004*/
 int i;
 double res = 0, value1, value2;
 if (!mustrealize)
  {
   for (i=0;i<current_dataset->featurecount;i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:value1 = inst1->values[i].intvalue;
								            value2 = inst2->values[i].intvalue;
                    if (variance.values[i].floatvalue != 0)
																				 {
								              value1 = (value1 - mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue);
								              value2 = (value2 - mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue);
								              res += (value1 - value2) * (value1 - value2);
																				 }
                    break;
       case    REAL:value1 = inst1->values[i].floatvalue;
								            value2 = inst2->values[i].floatvalue;
                    if (variance.values[i].floatvalue != 0)
																				 {
								              value1 = (value1 - mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue);
								              value2 = (value2 - mean.values[i].floatvalue) / sqrt(variance.values[i].floatvalue);
								              res += (value1 - value2) * (value1 - value2);
																				 }
                    break;
       case  STRING:if (inst1->values[i].stringindex != inst2->values[i].stringindex)
                      res+=1;
                    break;
      }
  }
 return res;
}

double dot_product(Instanceptr inst1, Instanceptr inst2)
{
 int i;
 double sum = 0;
 for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
   sum += real_feature(inst1, i) * real_feature(inst2, i);
 return sum;
}

/**
 * Calculates the euclidian distance between two instances. If the dataset contains discrete features, if they are equal the distance is 0, if they are not equal the distance is 1.
 * @param[in] inst1 First instance
 * @param[in] inst2 Second instance
 * @return Square of the euclidian distance between two instances
 */
double distance_between_instances(Instanceptr inst1, Instanceptr inst2)
{
 int i;
 double res = 0;
 if (mustrealize)
  {
   for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
     res = res + pow((real_feature(inst1, i) - real_feature(inst2, i)), 2);
  }
 else
  {
   for (i = 0; i < current_dataset->featurecount; i++)
     switch (current_dataset->features[i].type)
      {
       case INTEGER:res += pow((inst1->values[i].intvalue - inst2->values[i].intvalue), 2);
                    break;
       case    REAL:res += pow((inst1->values[i].floatvalue - inst2->values[i].floatvalue), 2);
                    break;
       case  STRING:if (inst1->values[i].stringindex != inst2->values[i].stringindex)
                      res += 1;
                    break;
      }
  }
 return res;
}

/**
 * Returns the real regression output of the instance
 * @param[in] myptr Instance
 * @return Real regression output of the instance
 */
double give_regressionvalue(Instanceptr myptr)
{
 return myptr->values[current_dataset->classdefno].floatvalue;
}

/**
 * Returns the real class index of the instance
 * @param[in] myptr Instance
 * @return The class index of the instance
 */
int give_classno(Instanceptr myptr)
{
 if (current_dataset->classdefno != -1)
   return myptr->values[current_dataset->classdefno].stringindex;
 else
   return -1;
}

/**
 * Returns -1 if the class is negative +1 otherwise.
 * @param[in] myptr Instance
 * @return -1 if the class is negative +1 otherwise.
 */
int svm_classno(Instanceptr myptr, int negative)
{
 if (current_dataset->classdefno != -1)
   if (myptr->values[current_dataset->classdefno].stringindex == negative)
     return -1;
   else
     return 1;
 else
   return 0;
}

/**
 * Frees memory allocated for the instance
 * @param[in] myptr The instance
 */
void deallocate(Instanceptr myptr)
{
	/*!Last Changed 07.01.2007 added sublist*/
 Instanceptr tmp;
 if (myptr)
  {
   while(myptr)
    {
     tmp = myptr->next;
     safe_free(myptr->values);
					if (myptr->sublist)
						 deallocate(myptr->sublist);
     if (myptr->class_labels)
       safe_free(myptr->class_labels);
     safe_free(myptr);
     myptr = tmp;
    }
  }
}
