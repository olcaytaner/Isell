#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include "data.h"
#include "dataset.h"
#include "decisiontree.h"
#include "instance.h"
#include "instancelist.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "messages.h"
#include "parallel.h"
#include "parameter.h"
#include "regressiontree.h"
#include "svmkernel.h"

extern int mustrealize;
extern Datasetptr current_dataset;

/**
 * Generates 2-d copy of an instance link list. The new instances of the new link list are obtained using generate_2d_copy_of_instance function. A 2-dimensional copy of an instance is obtained via the formula (x_1 + x_2 + \ldots + x_d + 1)^2.
 * @param[in] list Header of the link list containing the instances.
 * @return 2-d copy of an instance link list. The instances contain squares of the features x_i^2, two features multiplied x_ix_j and original features x_i.
 */
Instanceptr generate_2d_copy_of_instance_list(Instanceptr list)
{
 /*!Last Changed 23.03.2005*/
 Instanceptr result, tmp, newinst, newbefore;
 result = generate_2d_copy_of_instance(list);
 newbefore = result;
 tmp = list->next;
 while (tmp)
  {
   newinst = generate_2d_copy_of_instance(tmp);
   newbefore->next = newinst;
   newbefore = newinst;
   tmp = tmp->next;
  }
 return result;
}

/**
 * Finds the mean of the real output values (for regression). 
 * @param[in] instances The header of the link list storing the instances
 * @return The mean value of the regression output values. If there are no instances returns 0.
 */
double find_mean_of_regression_value(Instanceptr instances)
{
 /*!Last Changed 09.12.2004*/
 double sum = 0;
 int count = 0;
 Instanceptr tmp;
 tmp = instances;
 while (tmp)
  {
   count++;
   sum += give_regressionvalue(tmp);
   tmp = tmp->next;
  }
 if (count != 0)
   return sum / count;
 return 0;
}

/**
 * Finds the sum of square deviations of the real output values (for regression). 
 * @param[in] instances The header of the link list storing the instances
 * @warning This is not the standard deviation nor the variance. Variance will be obtained by dividing the function's output to the number of instances - 1.
 * @return Sum of square deviations from the mean value of the real output values.
 */
double find_variance_of_regression_value(Instanceptr instances, double mean)
{
 /*!Last Changed 09.12.2004*/
 double sum = 0, value;
 Instanceptr tmp;
 int count = 0;
 tmp = instances;
 while (tmp)
  {
   count++;
   value = give_regressionvalue(tmp);
   sum = sum + (value - mean) * (value - mean);
   tmp = tmp->next;
  }
 return sum;
}

/**
 * Converts a link list of instances to a matrix. Instances are stored as rows.
 * @param[in] list Header of the link list containing the instances
 * @param[in] problemtype Type of the machine learning problem. CLASSIFICATION for a classification problem, REGRESSION for a regression problem, 2 for a one-vs-all problem where the index of the class one is given in the parameter positive, 3 for one-class-problem where the class index is given in the parameter positive.
 * @param[in] positive If problemtype is 2, the index of the class one. If problem type is 3, the class index.
 * @param[in] multivariate_type Type of the multivariate problem. LINEAR where the instances are stored in the matrix as they are. QUADRATIC where the instances are stored as 2-d instances in the matrix. A 2-d instance is obtained via the formula (x_1 + x_2 + \ldots + x_d + 1)^2 and contains squares of the features x_i^2, two features multiplied x_ix_j and original features x_i.
 * @return Matrix storing a group of instances.
 */
matrix instancelist_to_matrix(Instanceptr list, int problemtype, int positive, MULTIVARIATE_TYPE multivariate_type)
{
 /*!Last Changed 28.05.2004*/
 /*!Last Changed 22.03.2004*/
 matrix result;
 int i, dim, matrixdim, x, j;
 Instanceptr tmp;
 long int dataindex;
 dim = current_dataset->multifeaturecount - 1;
 if (multivariate_type == MULTIVARIATE_LINEAR)
   matrixdim = dim;
 else
   matrixdim = multifeaturecount_2d(current_dataset);
 result = matrix_alloc(data_size(list), matrixdim + 1);
 tmp = list;
 dataindex = 0;
 while (tmp)
  {
   if (multivariate_type == MULTIVARIATE_LINEAR)
     for (i = 0; i < current_dataset->multifeaturecount - 1; i++)
       result.values[dataindex][i] = real_feature(tmp, i);
   else
     if (multivariate_type == MULTIVARIATE_QUADRATIC)
      {
       x = 0;
       for (i = 0; i < dim; i++)
         for (j = i; j < dim; j++)
          {
           result.values[dataindex][x] = real_feature(tmp, i) * real_feature(tmp, j);
           x++;
          }
       for (i = 0; i < dim; i++)
        {
         result.values[dataindex][x] = real_feature(tmp, i);
         x++;
        }
      }
   switch (problemtype)
    {
     case CLASSIFICATION:result.values[dataindex][matrixdim] = give_classno(tmp);
                         break;
     case REGRESSION    :result.values[dataindex][matrixdim] = give_regressionvalue(tmp);
                         break;
     case 2             :if (give_classno(tmp) == positive)
                           result.values[dataindex][matrixdim] = 0;
                         else
                           result.values[dataindex][matrixdim] = 1;
                         break;
     case 3             :result.values[dataindex][matrixdim] = positive;
                         break;
     default            :printf(m1241, problemtype);
                         break;
    }
   tmp = tmp->next;
   dataindex++;
  }
 return result;
}

void print_instance_list_with_extra_features(FILE* outfile, char s, Datasetptr d, Instanceptr listptr, int extra_features)
{
 Instanceptr tmp;
 tmp = listptr;
 while (tmp)
  {
   print_instance_with_extra_features(outfile, s, d, tmp, extra_features);
   tmp = tmp->next;
  }
}

void print_class_information(FILE* outfile, Instanceptr listptr)
{
 Instanceptr tmp;
 int index;
 tmp = listptr;
 index = 0;
 while (tmp)
  {
   fprintf(outfile, "%d %s\n", index, current_dataset->classes[give_classno(tmp)]);
   index++;
   tmp = tmp->next;
  }
}

void print_kernel_matrix(FILE* outfile, Instanceptr listptr)
{
 Svm_parameter param;
 Svm_nodeptr node1, node2;
 Instanceptr tmp1, tmp2;
	param.coef0 = get_parameter_f("svmcoef0");
	param.degree = get_parameter_i("svmdegree");
	param.gamma = get_parameter_f("svmgamma");
	param.kernel_type = get_parameter_l("kerneltype");
 tmp1 = listptr;
 while (tmp1)
  {
   node1 = instance_to_svmnode(tmp1);
   tmp2 = listptr;
   while (tmp2)
    {
     node2 = instance_to_svmnode(tmp2);
     fprintf(outfile, "%.6f ", k_function(node1, node2, param));    
     tmp2 = tmp2->next;
    }
   fprintf(outfile, "\n");
   tmp1 = tmp1->next;
  }
}

/**
 * Prints a link list of instances to an output stream. The format of the output file is determined by the program parameter namestype. One can select which classes to export with the program parameter is_class_exported.
 * @param[in] outfile Output file stream.
 * @param[in] s Character for separating the features in the output file
 * @param[in] d Dataset of the instances
 * @param[in] listptr Link list of instances
 */
void print_instance_list(FILE* outfile, char s, Datasetptr d, Instanceptr listptr)
{
 /*!Last Changed 15.03.2007 added dataset with selected features*/
 /*!Last Changed 19.04.2004*/
 int classno, colcount = 0, i;
 int* exported;
 Instanceptr tmp;
 if (mustrealize)
  {
   colcount++;
   for (i = 0; i < d->multifeaturecount - 1; i++)
      if (d->selected[i])
        colcount++;
  }
 else
   for (i = 0; i < d->featurecount; i++)
     if (d->features[i].selected)
       colcount++;
 if (get_parameter_l("namestype") == NAMES_AYSU)
   fprintf(outfile, "# %d %d\n", data_size(listptr), colcount);
 tmp = listptr;
 if (d->classno != 0)
   exported = (int *)get_parameter_a("is_class_exported");
 while (tmp)
  {
   if (d->classno != 0)
    {
     classno = give_classno(tmp);
     if (exported[classno])
       print_instance(outfile, s, d, tmp);
    }
   else
     print_instance(outfile, s, d, tmp);
   tmp = tmp->next;
  }
}

/**
 * Moves exceptions (instances whose exception field is TRUE) from list2 to list1.
 * @param[out] list The list that will contain the exceptions.
 * @param[inout] list2 The list from which exceptions will be removed
 */
void create_list_from_exceptions(Instanceptr* list, Instanceptr* list2)
{
 /*!Last Changed 03.12.2004 changed*/
 /*!Last Changed 08.05.2004*/
 Instanceptr tmp1, tmp2 = NULL, tmp1before;
 if (list2 == NULL)
   return;
 tmp1 = *list2;
 tmp1before = NULL;
 *list = NULL;
 while (tmp1)
  {
   if (tmp1->isexception)
    {
     if (*list == NULL)
       *list = tmp1;
     else
       tmp2->next = tmp1;
     tmp2 = tmp1;
    }
   else
    {
     if (tmp1before)
       tmp1before->next = tmp1;
     else
       *list2 = tmp1;
     tmp1before = tmp1;
    }
   tmp1 = tmp1->next;
  }
 if (tmp2)
   tmp2->next = NULL;
 if (tmp1before)
   tmp1before->next = NULL;
}

/**
 * Reverses a link list of instances. The first element will be the last element, the second element will be the element before the last element, etc.
 * @param[in] list Link list of instances.
 */
void reverse_instancelist(Instanceptr* list)
{
 /*!Last Changed 26.03.2007*/
 Instanceptr current, next, header = NULL;
 current = *list;
 while (current)
  {
   next = current->next;
   current->next = header;
   header = current;
   current = next;
  }
 *list = header;
}

/**
 * Appends second link list to the first one.
 * @param[inout] list1 First link list
 * @param[in] list2 Second link list
 */
void merge_instancelist(Instanceptr* list1, Instanceptr list2)
{
 /* Last Changed 09.05.2003*/
 Instanceptr tmp = *list1;
 if (list2 == NULL)
   return;
 if (tmp == NULL)
  {
   *list1 = list2;
   return;
  } 
 while (tmp->next)
   tmp = tmp->next;
 tmp->next = list2;
}

/**
 * Merges one or more link lists together and appends them to the first link list.
 * @param[inout] mergedlist First link list
 * @param[in] lists Array of link lists to be appended. lists[i] is i'th link list
 * @param[in] count Number of link lists to be merged and appended (Size of the lists array).
 */
void merge_instancelist_groups(Instanceptr* mergedlist, Instanceptr* lists, int count)
{
 /*!Last Changed 04.04.2007*/
 int i = 0, j;
 Instanceptr tmp;
 while (!lists[i] && i < count)
   i++;
 *mergedlist = lists[i];
 tmp = lists[i];
 for (j = i + 1; j < count; j++)
  {
   while (tmp->next) /*Go to the end of list[i - 1]*/
     tmp = tmp->next;
   tmp->next = lists[j];
  }
}

/**
 * Append instances from a specific class in the second link list to the first link list.
 * @param[inout] list1 First link list
 * @param[in] list2 Second link list
 * @param[in] classno Class index
 */
void merge_instancelist_without_outliers(Instanceptr* list1, Instanceptr* list2, int classno)
{
 /*!Last Changed 20.06.2005*/
 int instance_classno;
 Instanceptr tmp1 = *list1, tmp2 = *list2, tmp2before = NULL;
 if (tmp2 == NULL)
   return;
 if (tmp1)
   while (tmp1->next != NULL)
     tmp1 = tmp1->next;
 while (tmp2)
  {
   instance_classno = give_classno(tmp2);
   if (instance_classno == classno)
    {
     if (tmp1)
       tmp1->next = tmp2;
     else
       *list1 = tmp2;
     tmp1 = tmp2;
     if (tmp2before)
       tmp2before->next = tmp2->next;
     else
       *list2 = tmp2->next;
    }
   else
     tmp2before = tmp2;
   tmp2 = tmp2->next;
  }
 if (tmp1)
   tmp1->next = NULL;
}

/**
 * Create an array list of instances from a link list of instances.
 * @param[in] instances Link list of instances
 * @return An array of instances. result[i] is the instance in the position i (starts from 0) in the link list instances.
 */
Instanceptr* create_array_of_instances(Instanceptr instances)
{
 /*!Last Changed 02.02.2004*/
 /*!Last Changed 26.01.2003*/
 Instanceptr* result, tmp = instances;
 int i, size;
 size = data_size(instances);
 result = safemalloc(size * sizeof(Instanceptr), "create_array_of_instances", 4);
 i = 0;
 while (tmp)
  {
   result[i] = tmp;
   i++;
   tmp = tmp->next;
  }
 return result;
}

Instanceptr copy_of_instancelist(Instanceptr list)
{
 Instanceptr tmp = list, newinstance, before = NULL, start = NULL;
 while (tmp)
  {
   newinstance = copy_of_instance(tmp);
   if (before)
     before->next = newinstance;
   else
     start = newinstance;
   before = newinstance;
   tmp = tmp->next;
  }
 return start;
}

Instanceptr bootstrap_sample_without_stratification(Instanceptr list)
{
 /*!Last Changed 19.02.2005*/
 int size, *array, i;
 Instanceptr* instancearray, result, tmp, newinstance, current;
 size = data_size(list);
 array = bootstrap_array(size);
 instancearray = create_array_of_instances(list);
 result = copy_of_instance(instancearray[array[0]]);
 current = result;
 for (i = 1; i < size; i++)
  {
   tmp = instancearray[array[i]];
   newinstance = copy_of_instance(tmp);
   current->next = newinstance;
   current = newinstance;
  }
 safe_free(instancearray);
 safe_free(array);
 return result;
}

void divide_instancelist_according_partition(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, Partition p)
{
 /* Last Changed 21.09.2004*/
 int i = 0, *classassignments, classno;
 Instanceptr ptmp, pnext, next1 = NULL, next2 = NULL;
 classassignments = find_class_assignments(p);
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   classno = give_classno(ptmp);
   if (classassignments[classno] == LEFT)
    {
     if (!next1)
       *list1 = ptmp;
     else
       next1->next = ptmp;
     next1 = ptmp;
    }
   else
    {
     if (!next2)
       *list2 = ptmp;
     else
       next2->next = ptmp;
     next2 = ptmp;
    }
   ptmp = pnext;
   i++;
  }
 if (next1)
   next1->next = NULL;
 if (next2)
   next2->next = NULL;
 (*parentlist) = NULL;
 safe_free(classassignments);
}

void divide_instancelist_according_to_soft_split(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, matrix w)
{
 Instanceptr ptmp, pnext, next1 = NULL, next2 = NULL;
 *list1 = NULL;
 *list2 = NULL;
 ptmp = (*parentlist);
 while (ptmp)
 {
  pnext = ptmp->next;
  if (linear_fit_value(w, ptmp) > 0)
   {
    if (!next1)
      *list1 = ptmp;
    else
      next1->next = ptmp;
    next1 = ptmp;
   }
  else
   {
    if (!next2)
      *list2 = ptmp;
    else
      next2->next = ptmp;
    next2 = ptmp;
   }
  ptmp = pnext;
 }
 if (next1)
   next1->next = NULL;
 if (next2)
   next2->next = NULL;
 (*parentlist) = NULL;
}

void divide_instancelist_according_centers(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, Instanceptr leftcenter, Instanceptr rightcenter, int quadratic)
{
 /*!Last Changed 03.04.2005*/
 /*!Last Changed 24.03.2005*/
 Instanceptr ptmp, pnext, next1 = NULL, next2 = NULL;
 CHILD_TYPE result;
 *list1 = NULL;
 *list2 = NULL;
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   if (quadratic)
     result = nearest_center_in_regression_tree(ptmp, QUADRATIC, leftcenter, rightcenter);
   else
     result = nearest_center_in_regression_tree(ptmp, LINEAR, leftcenter, rightcenter);
   if (result == LEFTCHILD)
    {
     if (!next1)
       *list1 = ptmp;
     else
       next1->next = ptmp;
     next1 = ptmp;
    }
   else
    {
     if (!next2)
       *list2 = ptmp;
     else
       next2->next = ptmp;
     next2 = ptmp;
    }
   ptmp = pnext;
  }
 if (next1)
   next1->next = NULL;
 if (next2)
   next2->next = NULL;
 (*parentlist) = NULL;
}

void divide_instancelist_according_to_split(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, int fno, double split)
{
 /*!Last Changed 09.12.2004*/
 Instanceptr ptmp, pnext, next1 = NULL, next2 = NULL;
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   if (ptmp->values[fno].floatvalue <= split)
    {
     if (!next1)
       *list1 = ptmp;
     else
       next1->next = ptmp;
     next1 = ptmp;
    }
   else
    {
     if (!next2)
       *list2 = ptmp;
     else
       next2->next = ptmp;
     next2 = ptmp;
    }
   ptmp = pnext;
  }
 if (next1)
   next1->next = NULL;
 if (next2)
   next2->next = NULL;
 (*parentlist) = NULL;
}

void divide_instancelist_one_vs_one_with_copy(Instanceptr parentlist, Instanceptr* list, int positive, int negative)
{
 Instanceptr tmp, inst, next = NULL;
 tmp = parentlist;
 while (tmp)
  {
   if (give_classno(tmp) == positive)
    {
     inst = copy_of_instance(tmp);
     inst->values[current_dataset->classdefno].stringindex = 0;
     if (!next)
       *list = inst;
     else
       next->next = inst;
     next = inst;
    }
   else
     if (give_classno(tmp) == negative)
      {
       inst = copy_of_instance(tmp);
       inst->values[current_dataset->classdefno].stringindex = 1;
       if (!next)
         *list = inst;
       else
         next->next = inst;
       next = inst;
      }
   tmp = tmp->next;
  }
 if (next)
   next->next = NULL;
 else
   *list = NULL;
}

void divide_instancelist_one_vs_one(Instanceptr* parentlist, Instanceptr* list, int positive, int negative)
{
 Instanceptr ptmp, pnext, next1 = NULL, next = NULL;
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   if (give_classno(ptmp) == positive)
    {
     if (!next1)
       *list = ptmp;
     else
       next1->next = ptmp;
     next1 = ptmp;
    }
   else
     if (give_classno(ptmp) == negative)
      {
       if (!next1)
         *list = ptmp;
       else
         next1->next = ptmp;
       next1 = ptmp;
      }
     else
      {
       if (!next)
         *parentlist = ptmp;
       else
         next->next = ptmp;
       next = ptmp;
      }
   ptmp = pnext;
  }
 if (next1)
   next1->next = NULL;
 if (next)
   next->next = NULL;
 else
   *parentlist = NULL;
}

void divide_instancelist_one_vs_rest_with_copy(Instanceptr parentlist, Instanceptr* list, int positive)
{
 Instanceptr tmp, inst, next = NULL;
 tmp = parentlist;
 while (tmp)
  {
   if (give_classno(tmp) == positive)
    {
     inst = copy_of_instance(tmp);
     inst->values[current_dataset->classdefno].stringindex = 0;
     if (!next)
       *list = inst;
     else
       next->next = inst;
     next = inst;
    }
   else
    {
     inst = copy_of_instance(tmp);
     inst->values[current_dataset->classdefno].stringindex = 1;
     if (!next)
       *list = inst;
     else
       next->next = inst;
     next = inst;
    }
   tmp = tmp->next;
  }
 if (next)
   next->next = NULL;
 else
   *list = NULL;
}

void divide_instancelist_one_vs_rest(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, int positive)
{
 /*!Last Changed 07.01.2004*/
 Instanceptr ptmp, pnext, next1 = NULL, next2 = NULL;
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   if (give_classno(ptmp) == positive)
    {
     if (!next1)
       *list1 = ptmp;
     else
       next1->next = ptmp;
     next1 = ptmp;
    }
   else
    {
     if (!next2)
       *list2 = ptmp;
     else
       next2->next = ptmp;
     next2 = ptmp;
    }
   ptmp = pnext;
  }
 if (next1)
   next1->next = NULL;
 if (next2)
   next2->next = NULL;
 (*parentlist) = NULL;
}

Instanceptr* divide_instancelist_into_parts(Instanceptr* parentlist, int partcount)
{
 /*!Last Changed 03.08.2006*/
 int i = 0, *classes = NULL, *classcounts = NULL, *partition, pos;
 Instanceptr *result, *next, ptmp, pnext;
 result = safemalloc(partcount * sizeof(Instanceptr), "divide_instancelist_into_parts", 3);
 next = safecalloc(partcount, sizeof(Instanceptr), "divide_instancelist_into_parts", 4);
 if (current_dataset->classno > 0)
  {
   classes = find_classes(*parentlist);
   classcounts = find_class_counts(*parentlist);
  }
 partition = stratified_partition(classes, data_size(*parentlist), current_dataset->classno, classcounts);
 i = 0;
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   pos = partition[i] % partcount;
   if (!next[pos])
     result[pos] = ptmp;
   else
     next[pos]->next = ptmp;
   next[pos] = ptmp;
   ptmp = pnext;
   i++;
  }
 for (i = 0; i < partcount; i++)
   if (next[i])
     next[i]->next = NULL;
 (*parentlist) = NULL;
 if (current_dataset->classno > 0)
  {
   safe_free(classcounts);
   safe_free(classes);
  }
 safe_free(partition);
 safe_free(next);
 return result;
}

Instanceptr* divide_instancelist_into_classes(Instanceptr* parentlist)
{
 /*!Last Changed 04.04.2007*/
 int pos, i;
 Instanceptr *result, *next, ptmp, pnext;
 result = safemalloc(current_dataset->classno * sizeof(Instanceptr), "divide_instancelist_into_classes", 3);
 next = safecalloc(current_dataset->classno, sizeof(Instanceptr), "divide_instancelist_into_classes", 4);
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   pos = give_classno(ptmp);
   if (!next[pos])
     result[pos] = ptmp;
   else
     next[pos]->next = ptmp;
   next[pos] = ptmp;
   ptmp = pnext;
  }
 for (i = 0; i < current_dataset->classno; i++)
   if (next[i])
     next[i]->next = NULL;
 (*parentlist) = NULL;
 safe_free(next);
 return result;
}

void divide_instancelist(Instanceptr* parentlist, Instanceptr* list1, Instanceptr* list2, int mod)
{
 /*!Last Changed 28.10.2005 called find_classes and find_class_counts*/
 /*!Last Changed 24.03.2005*/
 /*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 23.11.2003*/
 /*!Last Changed 10.05.2003*/
 int i = 0, *classes = NULL, *classcounts = NULL, *partition;
 Instanceptr ptmp, pnext, next1 = NULL, next2 = NULL;
 if (current_dataset->classno > 0)
  {
   classes = find_classes(*parentlist);
   classcounts = find_class_counts(*parentlist);
  }
 partition = stratified_partition(classes, data_size(*parentlist), current_dataset->classno, classcounts);
 i = 0;
 ptmp = (*parentlist);
 while (ptmp)
  {
   pnext = ptmp->next;
   if (partition[i] % mod == (mod - 1))
    {
     if (!next1)
       *list1 = ptmp;
     else
       next1->next = ptmp;
     next1 = ptmp;
    }
   else
    {
     if (!next2)
       *list2 = ptmp;
     else
       next2->next = ptmp;
     next2 = ptmp;
    }
   ptmp = pnext;
   i++;
  }
 if (next1)
   next1->next = NULL;
 if (next2)
   next2->next = NULL;
 (*parentlist) = NULL;
 if (current_dataset->classno > 0)
  {
   safe_free(classcounts);
   safe_free(classes);
  }
 safe_free(partition);
}

Instanceptr create_instancelist(int* indexes, int count, Instanceptr* list)
{
 /*!Last Changed 09.02.2003*/
 Instanceptr result = NULL, tmp = (*list), tmp1 = NULL, tmpbefore = NULL, last = NULL;
 int i, firstgone = 1, firstleft = 1;
 i = 0;
 while (tmp && i < count)
  {
   if (tmp->index < indexes[i])
    {
     if (firstleft)
      {
       (*list) = tmp;
       firstleft = 0;
      }
     else
       last -> next = tmp;
     last = tmp; 
     tmp = tmp -> next;
    }
   else
    {
     if (tmp->index == indexes[i])
      {
       tmp1 = tmp;
       tmp = tmp -> next;
       if (firstgone)
        {
         result = tmp1;
         firstgone = 0;
        }
       else
         if (tmpbefore)
           tmpbefore -> next = tmp1;
       tmpbefore = tmp1;
      }
     i++;
    }
  }
 if (tmp1)
   tmp1 -> next = NULL;
 if (firstleft)
   (*list) = NULL; 
 else
   if (last)
     last -> next = tmp;
 return result;
}

Instanceptr create_instancelist_from_rules(Decisionconditionptr rules, int count, Instanceptr* list)
{
 /*!Last Changed 09.04.2003*/
 int i, found, featureindex;
 double value, datavalue;
 char comparison;
 Instanceptr result = NULL, tmp = (*list), last = NULL, next, tmpbefore = NULL;
 while (tmp)
  {
   found = 0;
   for (i = 0; i < count; i++)
    {
     featureindex = rules[i].featureindex;
     value = rules[i].value;
     comparison = rules[i].comparison;
     switch (current_dataset->features[featureindex].type)
      {
       case INTEGER:datavalue = tmp->values[featureindex].intvalue;
                    if (comparison=='<' && datavalue>value)
                      found = 1;
                    if (comparison=='>' && datavalue<=value)
                      found = 1;
                    break;
       case    REAL:datavalue = tmp->values[featureindex].floatvalue;
                    if (comparison=='<' && datavalue>value)
                      found = 1;
                    if (comparison=='>' && datavalue<=value)
                      found = 1;
                    break;
       case  STRING:if (tmp->values[featureindex].stringindex != value)
                      found = 1;
                    break;
      }
     if (found)
       break;
    }
   next = tmp->next;
   if (!found)
    {
     if (last)
       last -> next = tmp;
     else
       result = tmp;
     last = tmp;
    }
   else
    {
     if (tmpbefore)
       tmpbefore -> next = tmp;
     else
       (*list) = tmp;
     tmpbefore = tmp;
    }
   tmp = next;
  }
 if (last)
   last -> next = NULL;
 if (!tmpbefore)
   (*list) = NULL;
 else
   tmpbefore -> next = NULL;   
 return result;
}

Instanceptr restore_instancelist(Instanceptr mainlist,Instanceptr sublist)
{
 Instanceptr result = mainlist, tmpmain = mainlist, tmpsub = sublist, mainbefore = NULL, tmp;
 if (!mainlist)
   return sublist;
 while (tmpmain && tmpsub)
  {
   if (tmpmain->index < tmpsub->index)
    {
     mainbefore = tmpmain; 
     tmpmain = tmpmain -> next;
    }
   else
    {
     if (mainbefore != NULL)
       mainbefore -> next = tmpsub;
     else
       result = sublist;
     tmp = tmpsub -> next;
     tmpsub -> next = tmpmain;
     mainbefore = tmpsub;
     tmpsub = tmp;
    }
  }
 if (tmpmain == NULL)
   mainbefore -> next = tmpsub;
 return result;
}

int* create_index_array_for_instances(Instanceptr instances, int* count)
{
 /*!Last Changed 02.02.2004 added safemalloc*/
 int* result, i; 
 Instanceptr tmp = instances;
 (*count) = data_size(instances);
 result = (int *) safemalloc((*count) * sizeof(int), "create_index_array_for_instances", 4);
 i = 0;
 while (tmp)
  {
   result[i] = tmp -> index;
   tmp = tmp -> next;
   i++;
  }
 return result;
}

Instanceptr* find_all_neighbors(Instanceptr list, Instanceptr item, int* count)
{
 Instanceptr* neighbors, data;
 int i;
 data = list;
 *count = 0;
 while (data)
  {
   if (current_dataset->kernel->values[(int)(real_feature(item, 0))][(int)(real_feature(data, 0))] > 0)
     (*count)++;
   data = data->next;
  }
 neighbors = (Instanceptr*) safemalloc ((*count) * sizeof(Instanceptr), "find_all_neighbors", 10);
 data = list;
 i = 0;
 while (data)
  {
   if (current_dataset->kernel->values[(int)(real_feature(item, 0))][(int)(real_feature(data, 0))] > 0)
    {
     neighbors[i] = data;
     i++;
    }
   data = data->next;
  }
 return neighbors;
}

Instanceptr* find_nearest_neighbors(Instanceptr list, Instanceptr item, int count)
{
 /*!Last Changed 10.03.2004*/
 Instanceptr* neighbors, data;
 double* distances, d;
 int i, j;
 neighbors = (Instanceptr*) safemalloc (count * sizeof(Instanceptr), "find_nearest_neighbors", 4);
 distances = (double *) safemalloc(count * sizeof(double), "find_nearest_neighbors", 5);
 for (i = 0; i < count; i++)
  {
   distances[i] = (double) +LONG_MAX;
   neighbors[i] = NULL;
  }
 data = list;
 while (data)
  {
   d = distance_between_instances(data, item);
   if (d < distances[count - 1])
    {
     i = 0;
     while (distances[i] < d)
       i++;
     for (j = count - 1; j > i; j--)
      {
       neighbors[j] = neighbors[j - 1];
       distances[j] = distances[j - 1];
      }
     neighbors[i] = data;
     distances[i] = d;
    }
   data = data->next;
  }
 safe_free(distances);
 return neighbors;
}

Instanceptr* find_nearest_neighbors_before_normalize(Instanceptr list, Instanceptr item, int count, Instance mean, Instance variance)
{
 /*!Last Changed 17.05.2004*/
 Instanceptr* neighbors, data;
 double* distances, d;
 int i, j;
 neighbors = (Instanceptr*) safemalloc (count * sizeof(Instanceptr), "find_nearest_neighbors", 4);
 distances = (double *) safemalloc(count * sizeof(double), "find_nearest_neighbors", 5);
 for (i = 0; i < count; i++)
  {
   distances[i] = (double) +LONG_MAX;
   neighbors[i] = NULL;
  }
 data = list;
 while (data)
  {
   d = distance_between_instances_before_normalize(data, item, mean, variance);
   if (d < distances[count - 1])
    {
     i = 0;
     while (distances[i] < d)
       i++;
     for (j = count - 1; j > i; j--)
      {
       neighbors[j] = neighbors[j - 1];
       distances[j] = distances[j - 1];
      }
     neighbors[i] = data;
     distances[i] = d;
    }
   data = data->next;
  }
 safe_free(distances);
 return neighbors;
}

int* get_classes(Instanceptr instances, int *count)
{
 int *result = NULL, i, classno;
 Instanceptr tmp = instances;
 (*count) = 0;
 while (tmp)
  {
   classno = give_classno(tmp);
   for (i = 0; i < (*count); i++)
     if (classno == result[i])
       break;
   if (i == (*count))
    {
     (*count)++;
     result = (int *)alloc(result,(*count)*sizeof(int),(*count));
     result[i] = classno;
    }
   tmp = tmp->next;
  }
 return result;
}

int data_size(Instanceptr myptr)
{
 Instanceptr tmp=myptr;
 int i=0;
 while (tmp)
  {
   i++;
   tmp=tmp->next;
  }
 return i;
}

Instanceptr data_x(Instanceptr myptr, int index)
{
 Instanceptr tmp = myptr;
 int i = 0;
 while (tmp)
   if (index == i)
     return tmp;
   else
    {
     i++;
     tmp = tmp->next;
    }
 return NULL;
}

Instanceptr last_element_of_list(Instanceptr list)
{
 /*!Last Changed 18.03.2004*/
 Instanceptr tmp;
 tmp = list;
 if (!tmp)
   return NULL;
 while (tmp->next)
   tmp = tmp->next;
 return tmp;
}

double find_k_nearest_neighbor_distance(Instanceptr list, Instanceptr item, int count)
{
 /*!Last Changed 31.03.2008*/
 Instanceptr data;
 double* distances, d;
 int i, j;
 distances = (double *) safemalloc(count * sizeof(double), "find_nearest_neighbors", 5);
 for (i = 0; i < count; i++)
   distances[i] = (double) +LONG_MAX;
 data = list;
 while (data)
  {
   d = distance_between_instances(data, item);
   if (d < distances[count - 1])
    {
     i = 0;
     while (distances[i] < d)
       i++;
     for (j = count - 1; j > i; j--)
       distances[j] = distances[j - 1];
     distances[i] = d;
    }
   data = data->next;
  }
 d = distances[count - 1];
 safe_free(distances);
 return d;
}
