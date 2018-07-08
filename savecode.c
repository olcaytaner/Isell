#include "instance.h"
#include "instancelist.h"
#include "lang.h"
#include "libmemory.h"
#include "libmisc.h"
#include "savecode.h"
#include "dataset.h"
#include "decisiontree.h"
#include "regressiontree.h"
#include "svmkernel.h"
#include <string.h>
#include <stdlib.h>

int indentation;
extern Instance mean;
extern Instance variance;

/**
 * Makes indentation in the generated test code. This function is called at the beginning of each line.
 * @param[in] testcodefile File handle of the test code
 */
void code_indentation(FILE* testcodefile)
{
 /*!Last Changed 14.06.2006*/
 int i;
 for (i = 0; i < indentation; i++)
   fprintf(testcodefile, " ");
}

/**
 * Prints the token (or reserved word) that starts a statement block in a programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_start_block(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 14.06.2006*/
 indentation++;
 if (in_list(language, 4, C_LANGUAGE, CPP_LANGUAGE, JAVA_LANGUAGE, PASCAL_LANGUAGE))
   code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "{\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "begin\n");
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:break;
 };
 indentation++;
}

/**
 * Prints the token (or reserved word) that ends a statement block in a programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_end_block(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 14.06.2006*/
 indentation--;
 if (in_list(language, 4, C_LANGUAGE, CPP_LANGUAGE, JAVA_LANGUAGE, PASCAL_LANGUAGE))
   code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "end\n");
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:break;
 };
 indentation--; 
}

/**
 * Generates the file name of the test code by adding corresponding extension according to the programming language.
 * @param[in] testcodefilename The output file name without extension
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @return Adds '.' and corresponding extension (c for C language, java for Java language etc.) to the test code file name.
 */
char* test_code_file_name_with_extension(char* testcodefilename, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 02.06.2006*/
 char* fname = safemalloc((strlen(testcodefilename) + 6) * sizeof(char), "test_code_file_name_with_extension", 1);
 switch (language)
  {
   case C_LANGUAGE      :sprintf(fname, "%s.c", testcodefilename);
                         break;
   case CPP_LANGUAGE    :sprintf(fname, "%s.cpp", testcodefilename);
                         break;
   case PASCAL_LANGUAGE :sprintf(fname, "%s.pas", testcodefilename);
                         break;
   case JAVA_LANGUAGE   :sprintf(fname, "%s.java", testcodefilename);
                         break;
   case MATLAB_LANGUAGE :sprintf(fname, "%s.m", testcodefilename);
                         break;
   case FORTRAN_LANGUAGE:sprintf(fname, "%s.f", testcodefilename);
                         break;
  } 
 return fname;
}

/**
 * Prints the test function header containing the output (double for class index and regression fit), the parameters (test instance features stored as a double array)
 * @param[in] testcodefile File handle of the test code
 * @param[in] functionname Name of the test function. It is usually the name of the algorithm for which the test code will be generated
 * @param[in] featurecount Number of features in the dataset prior to realization.
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_function_header(FILE* testcodefile, char* functionname, int featurecount, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 18.05.2006*/
 switch (language){
   case C_LANGUAGE      :fprintf(testcodefile, "double %s(double* testdata){\n", functionname);
                         break;
   case CPP_LANGUAGE    :fprintf(testcodefile, "double test::%s(double* testdata){\n", functionname);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "function %s(real testdata[1..%d]):real;\n", functionname, featurecount);
                         break;
   case JAVA_LANGUAGE   :fprintf(testcodefile, "public static double %s(double[] testdata){\n", functionname);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "function result = %s(testdata)\n", functionname);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "real function %s(testdata)\nreal testdata(%d)\n", functionname, featurecount);
                         break;
 };
}

/**
 * Prints the token (or reserved word) that starts a function block in a programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 05.06.2006*/
 switch (language){   
   case PASCAL_LANGUAGE :fprintf(testcodefile, "begin\n");
                         break;
   default              :break;
 };
 indentation++;
}

/**
 * Prints the starting character(s) of a one-dimensional array or one-dimensional part of a 2 or 3 dimensional array.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] index If the index is -1, the array is one-dimensional. Otherwise the index shows the index of the sub one-dimensional array. 0 for the first one, 1 for the second one etc.
 * @return The character that separates the elements of the array. ' ' for Matlab language ',' for other languages.
 */
char code_1d_array_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int index)
{
 /*!Last Changed 24.06.2006*/
 char ch = ',';
 if (index != 0 && index != -1)
  {
   switch (language){
     case C_LANGUAGE      :
     case CPP_LANGUAGE    :
     case JAVA_LANGUAGE   :
     case PASCAL_LANGUAGE :
     case FORTRAN_LANGUAGE:fprintf(testcodefile, ",");
                           break;
     case MATLAB_LANGUAGE :fprintf(testcodefile, "\n");
                           break;
   }
  }
 switch (language){
   case MATLAB_LANGUAGE:ch = ' ';
                        break;
   default             :break;
 }  
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "{");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "(");
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "(/");
                         break;
			case MATLAB_LANGUAGE :if (index == - 1)
																										 fprintf(testcodefile, "[");
																									break;
 }
	return ch;
}

/**
 * Prints the ending character(s) of a one-dimensional array or one-dimensional part of a 2 or 3 dimensional array.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] index If the index is -1, the array is one-dimensional. Otherwise the index shows the index of the sub one-dimensional array. 0 for the first one, 1 for the second one etc.
 */
void code_1d_array_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int index)
{
 /*!Last Changed 24.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}");  
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, ")");
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "/)");
                         break;
			case MATLAB_LANGUAGE :if (index == - 1)
																										 fprintf(testcodefile, "]");
																									break;
 }
	if (index == -1)
		 fprintf(testcodefile, ";\n");  
}

/**
 * Prints the elements of one dimensional array or one-dimensional part of a 2 or 3 dimensional array.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] size Size of the array
 * @param[in] values The array
 * @param[in] index If the index is -1, the array is one-dimensional. Otherwise the index shows the index of the sub one-dimensional array. 0 for the first one, 1 for the second one etc.
 */
void code_constant_subarray(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int size, double* values, int index)
{
 /*!Last Changed 24.06.2006 added code_1d_array_start and code_1d_array_end*/
 /*!Last Changed 12.06.2006*/
 int i;
 char ch;
 ch = code_1d_array_start(testcodefile, language, index);
 fprintf(testcodefile, "%.6f", values[0]);
 for (i = 1; i < size; i++)
   fprintf(testcodefile, "%c%.6f", ch, values[i]);
	code_1d_array_end(testcodefile, language, index);
}

/**
 * Prints the features of an instance as if they are the elements of a constant array.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] d The dataset
 * @param[in] tmp The instance that will be printed
 * @param[in] index If the index is -1, the array is one-dimensional. Otherwise the index shows the index of the sub one-dimensional array. 0 for the first one, 1 for the second one etc.
 */
void code_constant_instance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Instanceptr tmp, int index)
{
 /*!Last Changed 24.06.2006 added code_1d_array_start and code_1d_array_end*/
 /*!Last Changed 09.06.2006*/
 int i;
 char ch;
 ch = code_1d_array_start(testcodefile, language, index);
 switch (d->features[0].type){
   case REAL    :
			case REGDEF  :fprintf(testcodefile, "%.6f", tmp->values[0].floatvalue);
                 break;
   case INTEGER :fprintf(testcodefile, "%d", tmp->values[0].intvalue);
                 break;
   case CLASSDEF:fprintf(testcodefile, "%d", tmp->values[0].stringindex);
                 break;
 }   
 for (i = 1; i < d->featurecount; i++)
   switch (d->features[i].type){
     case REAL    :
					case REGDEF  :fprintf(testcodefile, "%c%.6f", ch, tmp->values[i].floatvalue);
                   break;
     case INTEGER :fprintf(testcodefile, "%c%d", ch, tmp->values[i].intvalue);
                   break;
     case CLASSDEF:fprintf(testcodefile, "%c%d", ch, tmp->values[i].stringindex);
                   break;
   }      
	code_1d_array_end(testcodefile, language, index);
}

/**
 * Prints the features of a support vector as if they are the elements of a constant array.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] tmp Support vector
 * @param[in] index If the index is -1, the array is one-dimensional. Otherwise the index shows the index of the sub one-dimensional array. 0 for the first one, 1 for the second one etc.
 */
void code_constant_sv(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Svm_nodeptr tmp, int index)
{
 /*!Last Changed 24.06.2006*/
 char ch;
 Svm_nodeptr x = tmp;
 ch = code_1d_array_start(testcodefile, language, index);
	fprintf(testcodefile, "%.6f", x->value);
	x++;
	while (x->index != -1)
	 {
		 fprintf(testcodefile, "%c%.6f", ch, x->value);
		 x++;
	 }
	code_1d_array_end(testcodefile, language, index);
}

/**
 * Prints the features of a realized (discrete features are converted to continuous) instance as if they are the elements of a constant array.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] d The dataset
 * @param[in] tmp The instance that will be printed
 * @param[in] index If the index is -1, the array is one-dimensional. Otherwise the index shows the index of the sub one-dimensional array. 0 for the first one, 1 for the second one etc.
 */
void code_constant_instance_realized(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Instanceptr tmp, int index)
{
 /*!Last Changed 24.06.2006 added code_1d_array_start and code_1d_array_end*/
 /*!Last Changed 05.06.2006*/
 int i;
 char ch;
 ch = code_1d_array_start(testcodefile, language, index);
 fprintf(testcodefile, "%.6f", real_feature(tmp, 0));
 for (i = 1; i < d->multifeaturecount - 1; i++)
   fprintf(testcodefile, "%c%.6f", ch, real_feature(tmp, i));
	code_1d_array_end(testcodefile, language, index);
}

/**
 * Prints a simple constant with real value
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] constname Name of the constant
 * @param[in] constvalue Constant value
 */
void code_constant_real(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* constname, double constvalue)
{
 /*!Last Changed 12.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "const %s = %.6f;\n", constname, constvalue);
                         break;
   case PASCAL_LANGUAGE :
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = %.6f;\n", constname, constvalue);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "REAL :: %s = %.6f\n", constname, constvalue);
                         break;
 };
}

/**
 * Prints a simple constant with integer value
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] constname Name of the constant
 * @param[in] constvalue Constant value
 */
void code_constant_integer(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* constname, int constvalue)
{
 /*!Last Changed 13.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "const %s = %d;\n", constname, constvalue);
                         break;
   case PASCAL_LANGUAGE :
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = %d;\n", constname, constvalue);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "INTEGER :: %s = %d\n", constname, constvalue);
                         break;
 };
}

/**
 * Prints the reserved word for constants in a programming language (if it exists). This function is used before printing complex constants such as array constants.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_constant_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 05.06.2006*/
 switch (language){
   case PASCAL_LANGUAGE :fprintf(testcodefile, "const\n");
                         break;
   default              :break;
 };
}

/**
 * Prints an assignment statement to the test code file.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] varname Name of the variable in the left side of the assignment statement
 * @param[in] expression Right side of the assignment statement
 */
void code_set_variable(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* varname, char* expression)
{
 /*!Last Changed 05.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = %s;\n", varname, expression);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "%s = %s\n", varname, expression);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s := %s;\n", varname, expression);
                         break;
 };
}

/**
 * Prints the declaration of each array given the type, name and size. Prints one or more arrays. 
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] count Number of array variables
 * @warning For each array, add three parameters in calling this function: \n
 * First parameter is the type of the array, INTEGER_VARIABLE for integer arrays, REAL_VARIABLE for float or double arrays. \n
 * Second parameter is the name of the array. \n
 * Third parameter is the size of the array
 */
void code_add_array_variable(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int count, ...)
{
 /*!Last Changed 13.06.2006*/
 int i, arraytype, arraysize;
 char *arrayname, ch1, ch2;
 va_list ap;
 if (in_list(language, 1, MATLAB_LANGUAGE))
   return;
 va_start(ap, count);
 for (i = 0; i < count; i++)
  { 
   arraytype = va_arg(ap, int);
   arrayname = va_arg(ap, char *);
   arraysize = va_arg(ap, int);
   switch (language){
     case C_LANGUAGE      :
     case CPP_LANGUAGE    :
     case JAVA_LANGUAGE   :switch (arraytype){
                             case INTEGER_VARIABLE:fprintf(testcodefile, "int");
                                                   break;
                             case REAL_VARIABLE   :fprintf(testcodefile, "double");
                                                   break;
                           }
                           ch1 = '[';
                           ch2 = ']';
                           break;
     case FORTRAN_LANGUAGE:switch (arraytype){
                             case INTEGER_VARIABLE:fprintf(testcodefile, "integer");
                                                   break;
                             case REAL_VARIABLE   :fprintf(testcodefile, "real");
                                                   break;
                           }
                           ch1 = '(';
                           ch2 = ')';
                           break;
     default              :break;
   }   
   fprintf(testcodefile, " %s", arrayname);
   switch (language){
     case C_LANGUAGE      :
     case CPP_LANGUAGE    :
     case JAVA_LANGUAGE   :
     case FORTRAN_LANGUAGE:fprintf(testcodefile, "%c%d%c;\n", ch1, arraysize, ch2);
                           break;
     case PASCAL_LANGUAGE :fprintf(testcodefile, ": array[1..%d] of ", arraysize);
                           switch (arraytype){
                             case INTEGER_VARIABLE:fprintf(testcodefile, "integer;\n");
                                                   break;
                             case REAL_VARIABLE   :fprintf(testcodefile, "real;\n");
                                                   break;
                           }
                           break;
     default              :break;
   }
  }
}

/**
 * Prints the variable declarations of a given type. Prints one or more variables.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] vartype Type of the variable. INTEGER_VARIABLE for integer variables, REAL_VARIABLE for float or double variables.
 * @param[in] count Number of variables
 * @warning For each variable add its name in calling this function
 */
void code_add_variable(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int vartype, int count, ...)
{
 /*!Last Changed 05.06.2006*/
 int i;
 char* line;
 va_list ap;
 if (in_list(language, 1, MATLAB_LANGUAGE))
   return;
 switch (language)
  {
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :switch (vartype)
                          {
                           case INTEGER_VARIABLE:fprintf(testcodefile, "int");
                                                 break;
                           case REAL_VARIABLE   :fprintf(testcodefile, "double");
                                                 break;
                          }
                         break;
   case FORTRAN_LANGUAGE:switch (vartype)
                          {
                           case INTEGER_VARIABLE:fprintf(testcodefile, "integer");
                                                 break;
                           case REAL_VARIABLE   :fprintf(testcodefile, "real");
                                                 break;
                          }
                         break;
   default              :break;
  }
 va_start(ap, count);
 for (i = 0; i < count; i++)
  { 
   line = va_arg(ap, char *);
   if (i != 0)
     fprintf(testcodefile, ",");
   fprintf(testcodefile, " %s", line);
  }
 switch (language)
  {
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :
   case FORTRAN_LANGUAGE:fprintf(testcodefile, ";\n");
                         break;
   case PASCAL_LANGUAGE :switch (vartype)
                          {
                           case INTEGER_VARIABLE:fprintf(testcodefile, ": integer;\n");
                                                 break;
                           case REAL_VARIABLE   :fprintf(testcodefile, ": real;\n");
                                                 break;
                          }
                         break;
   default              :break;
  }
}

/**
 * Prints the reserved word that must be printed before variable declarations (if applicable for that programming language).
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_variable_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 05.06.2006*/
 switch (language){
   case PASCAL_LANGUAGE :fprintf(testcodefile, "var\n");
                         break;
   default              :break;
 };
}

/**
 * Prints the line containing the return value of the test function for classification. The function may exit from this line or may continue until the end of the function according to the programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] functionname Name of the test function. It is usually the name of the algorithm for which the test code will be generated
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] returnvalue The return value of the function
 */
void code_function_return_classification(FILE* testcodefile, char* functionname, PROGRAMMING_LANGUAGE_TYPE language, int returnvalue)
{
 /*!Last Changed 18.05.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "return %d;\n", returnvalue);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s := %d;\n", functionname, returnvalue);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "result = %d;\n", returnvalue);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "%s = %d\n", functionname, returnvalue);
                         break;
 };
if (language == FORTRAN_LANGUAGE)
 {
  code_indentation(testcodefile);
  fprintf(testcodefile, "return\n");
 }
}

/**
 * Prints the line containing the return value of the test function. The function may exit from this line or may continue until the end of the function according to the programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] functionname Name of the test function. It is usually the name of the algorithm for which the test code will be generated
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] returnvalue The return value of the function
 */
void code_function_return_string(FILE* testcodefile, char* functionname, PROGRAMMING_LANGUAGE_TYPE language, char* returnvalue)
{
 /*!Last Changed 07.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "return %s;\n", returnvalue);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s := %s;\n", functionname, returnvalue);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "result = %s;\n", returnvalue);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "%s = %s\n", functionname, returnvalue);
                         break;
 };
if (language == FORTRAN_LANGUAGE)
 {
  code_indentation(testcodefile);
  fprintf(testcodefile, "return\n");
 }
}

/**
 * Prints the line containing the return value of the test function for regression. The function may exit from this line or may continue until the end of the function according to the programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] functionname Name of the test function. It is usually the name of the algorithm for which the test code will be generated
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] returnvalue The return value of the function
 */
void code_function_return_regression(FILE* testcodefile, char* functionname, PROGRAMMING_LANGUAGE_TYPE language, double returnvalue)
{
 /*!Last Changed 29.05.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "return %.6f;\n", returnvalue);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s := %.6f;\n", functionname, returnvalue);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "result = %.6f;\n", returnvalue);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "%s = %.6f\n", functionname, returnvalue);
                         break;
 };
if (language == FORTRAN_LANGUAGE)
 {
  code_indentation(testcodefile);
  fprintf(testcodefile, "return\n");
 }
}

/**
 * Prints the token (or reserved word) that ends a function block in a programming language.
 * @param[in] testcodefile File handle of the test code
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 */
void code_function_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 18.05.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "end;\n");
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "end\n");
                         break;
 }
}

/**
 * Produces a string that refer to a one dimensional array element and concatenates it to an output string
 * @param[out] st The output string
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] arrayname Array name
 * @param[in] index Array index
 */
void code_array_index(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int index)
{
 /*!Last Changed 18.05.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :sprintf(st, "%s[%d]", arrayname, index);
                         break;
   case PASCAL_LANGUAGE :sprintf(st, "%s[%d]", arrayname, index + 1);
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:sprintf(st, "%s(%d)", arrayname, index + 1);
                         break;
 } 
}

/**
 * Produces a string that refer to a one dimensional array element and concatenates it to an output string
 * @param[out] st The output string
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] arrayname Array name
 * @param[in] index String containing the array index
 */
void code_array_index_variable(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index)
{
 /*!Last Changed 07.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :sprintf(st, "%s[%s]", arrayname, index);
                         break;
   case PASCAL_LANGUAGE :sprintf(st, "%s[%s + 1]", arrayname, index);
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:sprintf(st, "%s(%s + 1)", arrayname, index);
                         break;
 } 
}

/**
 * Produces a string that refer to a two dimensional array element and concatenates it to an output string
 * @param[out] st The output string
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] arrayname Array name
 * @param[in] index1 String containing the index of the first dimension
 * @param[in] index2 String containing the index of the second dimension
 */
void code_2d_array_index_variable(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index1, char* index2)
{
 /*!Last Changed 07.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :sprintf(st, "%s[%s][%s]", arrayname, index1, index2);
                         break;
   case PASCAL_LANGUAGE :sprintf(st, "%s[%s + 1, %s + 1]", arrayname, index1, index2);
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:sprintf(st, "%s(%s + 1, %s + 1)", arrayname, index1, index2);
                         break;
 } 
}

/**
 * Produces a string that refer to a three dimensional array element and concatenates it to an output string
 * @param[out] st The output string
 * @param[in] language The programming language of the test code. C_LANGUAGE for C language, CPP_LANGUAGE for C++ language, JAVA_LANGUAGE for Java language, MATLAB_LANGUAGE for Matlab and FORTRAN_LANGUAGE for Fortran.
 * @param[in] arrayname Array name
 * @param[in] index1 String containing the index of the first dimension
 * @param[in] index2 String containing the index of the second dimension
 * @param[in] index3 String containing the index of the third dimension
 */
void code_3d_array_index_variable(char* st, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index1, char* index2, char* index3)
{
 /*!Last Changed 08.08.2007*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :sprintf(st, "%s[%s][%s][%s]", arrayname, index1, index2, index3);
                         break;
   case PASCAL_LANGUAGE :sprintf(st, "%s[%s + 1, %s + 1, %s + 1]", arrayname, index1, index2, index3);
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:sprintf(st, "%s(%s + 1, %s + 1, %s + 1)", arrayname, index1, index2, index3);
                         break;
 } 
}

void code_case(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int* cases, int casecount)
{
 /*!Last Changed 12.06.2006*/
 int i;
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :for (i = 0; i < casecount - 1; i++)
																									 {
                           fprintf(testcodefile, "case %d:\n", cases[i]);
                           code_indentation(testcodefile);
																									 }
                         fprintf(testcodefile, "case %d:", cases[i]);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%d", cases[0]);
                         for (i = 1; i < casecount; i++)
                           fprintf(testcodefile, ", %d", cases[i]);
                         fprintf(testcodefile, ":");
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "case ");
                         if (casecount == 1)
                           fprintf(testcodefile, "%d,\n", cases[0]);
                         else
                          {
                           fprintf(testcodefile, "{%d", cases[0]);
                           for (i = 1; i < casecount; i++)
                             fprintf(testcodefile, ", %d", cases[i]);
                           fprintf(testcodefile, "}\n");
                          }
                         break;
   case FORTRAN_LANGUAGE:for (i = 0; i < casecount - 1; i++)
																									 {
                           fprintf(testcodefile, "(%d)\n", cases[i]);
																											code_indentation(testcodefile);
																									 }
				                     fprintf(testcodefile, "(%d)\n", cases[i]);
                         break;
 }  
}

void code_endcase(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 12.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "break;\n");
                         break;
   default              :break;
 } 
}

void code_switch(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* expression)
{
 /*!Last Changed 09.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "switch (%s){\n", expression);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "case %s of\n", expression);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "switch %s\n", expression);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "select case (%s)\n", expression);
                         break;
 }  
 indentation += 2;
}

void code_endswitch(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 09.06.2006*/
 indentation -= 2;
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "end;\n");
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "end\n");
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "end select\n");
                         break;
 }  
}

void code_simple_logical_expression(PROGRAMMING_LANGUAGE_TYPE language, char* operator, char* operand1, char* operand2, char* st2)
{
 /*!Last Changed 15.06.2006*/
 char st[5];
 switch (language){
   case PASCAL_LANGUAGE :if (strcmp(operator, "==") == 0)
                           strcpy(st, "=");
                         else
                           if (strcmp(operator, "!=") == 0)
                             strcpy(st, "<>");
                           else
                             strcpy(st, operator);
                         break;
   case FORTRAN_LANGUAGE:if (strcmp(operator, "==") == 0)
                           strcpy(st, ".EQ.");
                         else
                           if (strcmp(operator, "!=") == 0)
                             strcpy(st, ".NE.");
                           else 
                             if (strcmp(operator, "<=") == 0)
                               strcpy(st, ".LE.");
                             else
                               if (strcmp(operator, "<") == 0)
                                 strcpy(st, ".LT.");
                               else
                                 if (strcmp(operator, ">=") == 0)
                                   strcpy(st, ".GE.");
                                 else
                                   if (strcmp(operator, ">") == 0)
                                     strcpy(st, ".GT.");
                         break;
   default              :strcpy(st, operator);
                         break;
 };
 sprintf(st2, "%s %s %s", operand1, st, operand2);
}

void code_if_core(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* st)
{
 /*!Last Changed 15.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "if (%s)\n", st);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "if %s then\n", st);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "if %s,\n", st);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "if (%s) then\n", st);
                         break;
 };
 code_start_block(testcodefile, language);
}

void code_if(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* operator, char* operand1, char* operand2)
{
 /*!Last Changed 15.06.2006 divided into two functions, code_if_core and code_simple_logic_expression*/
 /*!Last Changed 18.05.2006*/
 char st[MAXLENGTH];
 code_simple_logical_expression(language, operator, operand1, operand2, st);
 code_if_core(testcodefile, language, st);
}

void code_while_core(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* st)
{
 /*!Last Changed 15.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :
   case MATLAB_LANGUAGE :fprintf(testcodefile, "while (%s)\n", st);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "while %s do\n", st);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "while (%s) do\n", st);
                         break;
 };
 code_start_block(testcodefile, language);
}

void code_while(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* operator, char* operand1, char* operand2)
{
 /*!Last Changed 15.06.2006 divided into two functions, code_if_core and code_simple_logic_expression*/
 /*!Last Changed 18.05.2006*/
 char st[MAXLENGTH];
 code_simple_logical_expression(language, operator, operand1, operand2, st);
 code_while_core(testcodefile, language, st);
}

void code_else(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 02.06.2006*/
 code_end_block(testcodefile, language);
 code_indentation(testcodefile);
 fprintf(testcodefile, "else\n");
 code_start_block(testcodefile, language);
}

void code_endif(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 02.06.2006*/
 indentation--;
 if (in_list(language, 2, MATLAB_LANGUAGE, FORTRAN_LANGUAGE))
   indentation--;
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "end;\n");
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "end\n");
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "endif\n");
                         break;
 }; 
 if (!in_list(language, 2, MATLAB_LANGUAGE, FORTRAN_LANGUAGE))
   indentation--;
}

void code_endwhile(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 15.06.2006*/
 indentation--;
 if (in_list(language, 2, MATLAB_LANGUAGE, FORTRAN_LANGUAGE))
   indentation--;
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "end;\n");
                         break;
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "end\n");
                         break;
 }; 
 if (!in_list(language, 2, MATLAB_LANGUAGE, FORTRAN_LANGUAGE))
   indentation--;
}

void code_for(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* variable, int start, int end, int difference)
{
 /*!Last Changed 07.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :if (difference == 1)
                           fprintf(testcodefile, "for (%s = %d; %s < %d; %s++)\n", variable, start, variable, end + 1, variable);
                         else
                           if (difference == -1)
                             fprintf(testcodefile, "for (%s = %d; %s > %d; %s--)\n", variable, start, variable, end - 1, variable);
                           else
                             if (difference > 0)
                               fprintf(testcodefile, "for (%s = %d; %s < %d; %s += %d)\n", variable, start, variable, end + 1, variable, difference);
                             else
                               fprintf(testcodefile, "for (%s = %d; %s > %d; %s -= %d)\n", variable, start, variable, end - 1, variable, -difference);                             
                         break;
   case PASCAL_LANGUAGE :if (difference > 0)
                           fprintf(testcodefile, "for %s := %d to %d do\n", variable, start, end);
                         else
                           fprintf(testcodefile, "for %s := %d downto %d do\n", variable, start, end);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "for %s = %d:%d:%d,\n", variable, start, difference, end);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "do %s = %d, %d, %d\n", variable, start, end, difference);
                         break;
 };  
 code_start_block(testcodefile, language);
}

void code_for_with_string(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* variable, char* start, char* end, char* difference)
{
 /*!Last Changed 25.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "for (%s = %s; %s <= %s; %s += %s)\n", variable, start, variable, end, variable, difference);                             
                         break;
   case PASCAL_LANGUAGE :if (atoi(difference) > 0)
                           fprintf(testcodefile, "for %s := %s to %s do\n", variable, start, end);
                         else
                           fprintf(testcodefile, "for %s := %s downto %s do\n", variable, start, end);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "for %s = %s:%s:%s,\n", variable, start, difference, end);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "do %s = %s, %s, %s\n", variable, start, end, difference);
                         break;
 };  
 code_start_block(testcodefile, language);
}

void code_break(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 13.06.2006*/
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :
   case PASCAL_LANGUAGE :
   case MATLAB_LANGUAGE :
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "break;\n");
                         break;
 };  
}

void code_endfor(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* forvariable)
{
 /*!Last Changed 07.06.2006*/
 indentation--;
 code_indentation(testcodefile);
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "}\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "end;\n");
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "end\n");
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "enddo\n");
                         break;
 };  
 indentation--;
}

void code_1d_array_constant_instance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* arrayname, int size, Instanceptr values)
{
 /*!Last Changed 19.10.2007 updated Java*/
	/*!Last Changed 27.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :fprintf(testcodefile, "const double %s[%d] = ", arrayname, size);
                         break;
   case JAVA_LANGUAGE   :fprintf(testcodefile, "final double[] %s = ", arrayname);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s: array[1..%d] of real = ", arrayname, size);
																									break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = ", arrayname);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "real %s(%d) = ", arrayname, size);
																									break;
 };  
 if (size == d->featurecount)   
   code_constant_instance(testcodefile, language, d, values, -1);
 else
   if (size == d->multifeaturecount - 1)
     code_constant_instance_realized(testcodefile, language, d, values, -1);
}

void code_1d_array_constant(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size, void* values, int elementtype)
{
 /*!Last Changed 19.10.2007 updated Java*/
 /*!Last Changed 25.06.2006 added elementtype to include integer arrays*/
 /*!Last Changed 05.06.2006*/
 char separator;
	double* dvalues;
	int* ivalues;
 int i;
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :switch (elementtype){
                           case REAL_TYPE   :fprintf(testcodefile, "const double %s[%d] = {", arrayname, size);
                                             break;
                           case INTEGER_TYPE:fprintf(testcodefile, "const int %s[%d] = {", arrayname, size);
                                             break;
                         };
                         separator = ',';
                         break;
   case JAVA_LANGUAGE   :switch (elementtype){
                           case REAL_TYPE   :fprintf(testcodefile, "final double[] %s = {", arrayname);
                                             break;
                           case INTEGER_TYPE:fprintf(testcodefile, "final int[] %s = {", arrayname);
                                             break;
                         };
                         separator = ',';
                         break;
   case PASCAL_LANGUAGE :switch (elementtype){
			                        case REAL_TYPE   :fprintf(testcodefile, "%s: array[1..%d] of real = (", arrayname, size);
																												                 break;
																											case INTEGER_TYPE:fprintf(testcodefile, "%s: array[1..%d] of integer = (", arrayname, size);
																												                 break;
				                     };
                         separator = ',';
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = [", arrayname);
                         separator = ' ';
                         break;
   case FORTRAN_LANGUAGE:switch (elementtype){
			                        case REAL_TYPE   :fprintf(testcodefile, "real %s(%d) = (/", arrayname, size);
																												                 break;
																											case INTEGER_TYPE:fprintf(testcodefile, "integer %s(%d) = (/", arrayname, size);
																												                 break;
																									};
				                     separator = ',';   
                         break;
 };  
	switch (elementtype){
	  case REAL_TYPE   :dvalues = (double*) values;
				                 fprintf(testcodefile, "%.6f ", dvalues[0]);
                     for (i = 1; i < size; i++)
                       fprintf(testcodefile, "%c%.6f ", separator, dvalues[i]);
																					break;
			case INTEGER_TYPE:ivalues = (int*) values;
				                 fprintf(testcodefile, "%d ", ivalues[0]);
                     for (i = 1; i < size; i++)
                       fprintf(testcodefile, "%c%d ", separator, ivalues[i]);
																					break;
	};
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "};\n");
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, ");\n");
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "];\n");
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "/);\n");
                         break;
 };  
}

void code_2d_array_constant(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, double** values)
{
 /*!Last Changed 12.06.2006*/
 int i;
 code_2d_array_start(testcodefile, language, arrayname, size1, size2);
 for (i = 0; i < size1; i++)
   code_constant_subarray(testcodefile, language, size2, values[i], i);
 code_2d_array_end(testcodefile, language); 
}

void code_2d_array_constant_matrix(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, matrixptr matrixarray)
{
 /*!Last Changed 08.08.2007*/
 int i;
 code_2d_array_start(testcodefile, language, arrayname, size1, size2);
 for (i = 0; i < size1; i++)
   code_constant_subarray(testcodefile, language, size2, matrixarray[i].values[0], i);
 code_2d_array_end(testcodefile, language); 
}

void code_2d_array_constant_instance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* arrayname, int size1, int size2, Instanceptr values)
{
 /*!Last Changed 09.06.2006*/
 int i;
 Instanceptr tmp = values;
 code_2d_array_start(testcodefile, language, arrayname, size1, size2);
 i = 0;
 while (tmp)
  {
   if (size2 == d->featurecount)   
     code_constant_instance(testcodefile, language, d, tmp, i);
   else
     if (size2 == d->multifeaturecount - 1)
       code_constant_instance_realized(testcodefile, language, d, tmp, i);
   i++;
   tmp = tmp->next;
  }
 code_2d_array_end(testcodefile, language); 
}

void code_2d_array_constant_sv(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, Svm_nodeptr* values)
{
 /*!Last Changed 24.06.2006*/
 int i;
 code_2d_array_start(testcodefile, language, arrayname, size1, size2);
	for (i = 0; i < size1; i++)
   code_constant_sv(testcodefile, language, values[i], i);
 code_2d_array_end(testcodefile, language); 
}

void code_2d_array_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2)
{
 /*!Last Changed 19.10.2007 updated Java, C, C++*/
 /*!Last Changed 05.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :fprintf(testcodefile, "const double %s[%d][%d] = {", arrayname, size1, size2);
                         break;
   case JAVA_LANGUAGE   :fprintf(testcodefile, "final double[][] %s = {", arrayname);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s: array[1..%d][1..%d] of real = (", arrayname, size1, size2);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = [", arrayname);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "real %s(%d, %d) = (", arrayname, size1, size2);
                         break;
 };  
}

void code_2d_array_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 05.06.2006*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :
   case JAVA_LANGUAGE   :fprintf(testcodefile, "};\n");
                         break;
   case PASCAL_LANGUAGE :
   case FORTRAN_LANGUAGE:fprintf(testcodefile, ");\n");
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "];\n");
                         break;
 };  
}

void code_3d_array_constant_matrix(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, int size3, matrixptr matrixarray)
{
 /*!Last Changed 08.08.2007*/
 int i, j;
 code_3d_array_start(testcodefile, language, arrayname, size1, size2, size3);
 for (i = 0; i < size1; i++)
  {
   code_1d_array_start(testcodefile, language, i);
   for (j = 0; j < size2; j++)
     code_constant_subarray(testcodefile, language, size2, matrixarray[i].values[j], j);
   code_1d_array_end(testcodefile, language, i);
  }
 code_3d_array_end(testcodefile, language); 
}

void code_3d_array_start(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size1, int size2, int size3)
{
 /*!Last Changed 19.10.2007 updated C, C++, Java*/
 /*!Last Changed 08.08.2007*/
 switch (language){
   case C_LANGUAGE      :
   case CPP_LANGUAGE    :fprintf(testcodefile, "const double %s[%d][%d][%d] = {", arrayname, size1, size2, size3);
                         break;
   case JAVA_LANGUAGE   :fprintf(testcodefile, "const double [][][] %s = {", arrayname);
                         break;
   case PASCAL_LANGUAGE :fprintf(testcodefile, "%s: array[1..%d][1..%d][1..%d] of real = (", arrayname, size1, size2, size3);
                         break;
   case MATLAB_LANGUAGE :fprintf(testcodefile, "%s = [", arrayname);
                         break;
   case FORTRAN_LANGUAGE:fprintf(testcodefile, "real %s(%d, %d, %d) = (", arrayname, size1, size2, size3);
                         break;
 };  
}

void code_3d_array_end(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language)
{
 /*!Last Changed 08.08.2007*/
 code_2d_array_end(testcodefile, language);
}

void code_array_initialize(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, int size, char* value)
{
 /*!Last Changed 13.06.2006*/
 char st[MAXLENGTH];
 code_for(testcodefile, language, "i", 0, size - 1, 1);   
 code_array_index_variable(st, language, arrayname, "i");  
 code_set_variable(testcodefile, language, st, value);
 code_endfor(testcodefile, language, "i");
}

void code_calculate_distance(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d)
{
 /*!Last Changed 13.06.2006*/
 char st[MAXLENGTH];
 int *symbolic, *numeric, scount, ncount;
 code_set_variable(testcodefile, language, "sum", "0.0"); 
 code_for(testcodefile, language, "j", 0, d->featurecount - 1, 1); 
 code_array_index_variable(st, language, "testdataconverted", "j");
 code_set_variable(testcodefile, language, "x_j", st);
 code_2d_array_index_variable(st, language, "m", "i", "j");
 code_set_variable(testcodefile, language, "m_ij", st);
 code_switch(testcodefile, language, "j");
 symbolic = symbolic_features(d, &scount);
 numeric = numeric_features(d, &ncount);
 if (scount > 0)
  {
   code_case(testcodefile, language, symbolic, scount);
   code_if(testcodefile, language, "==", "m_ij", "x_j");
   code_set_variable(testcodefile, language, "sum", "sum + 1");
   code_endif(testcodefile, language);
   code_endcase(testcodefile, language);
  }
 if (ncount > 0)
  {
   code_case(testcodefile, language, numeric, ncount);
   code_set_variable(testcodefile, language, "sum", "sum + (x_j - m_ij) * (x_j - m_ij)");
   code_endcase(testcodefile, language);
  }
 safe_free(symbolic);
 safe_free(numeric);
 code_endswitch(testcodefile, language);
 code_endfor(testcodefile, language, "j");
}

void code_sort_according_to_distances(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int size, int nearcount)
{
 /*!Last Changed 13.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH];
 code_for(testcodefile, language, "i", 0, size - 1, 1);   
 code_calculate_distance(testcodefile, language, d); 
 code_for(testcodefile, language, "j", nearcount - 1, 0, -1);   
 code_array_index_variable(st, language, "distances", "j");  
 code_if(testcodefile, language, "<", "sum", st);
 sprintf(st, "%d", nearcount - 1);
 code_if(testcodefile, language, "!=", "j", st);
 code_array_index_variable(st, language, "distances", "j + 1");  
 code_array_index_variable(st2, language, "distances", "j");  
 code_set_variable(testcodefile, language, st, st2);  
 code_array_index_variable(st, language, "indexes", "j + 1");  
 code_array_index_variable(st2, language, "indexes", "j");  
 code_set_variable(testcodefile, language, st, st2);  
 code_endif(testcodefile, language);
 code_array_index_variable(st, language, "distances", "j");  
 code_set_variable(testcodefile, language, st, "sum");
 code_array_index_variable(st, language, "indexes", "j");  
 code_set_variable(testcodefile, language, st, "i");
 code_else(testcodefile, language);
 code_break(testcodefile, language);
 code_endif(testcodefile, language);
 code_endfor(testcodefile, language, "j");
 code_endfor(testcodefile, language, "i");
}

void code_regressionnode(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Regressionnodeptr m, int level)
{
 /*!Last Changed 14.06.2006*/
 int i;
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH], st4[MAXLENGTH];
 if (m->featureno != LEAF_NODE)
  {
   if (m->featureno >= 0)
    {
     code_array_index(st3, language, "testdataconverted", m->featureno);
     sprintf(st4, "%.6f", m->split);
    }
   else
     if (m->featureno == LINEAR)
      {
       code_array_index_variable(st2, language, "testdataconverted", "0");
       sprintf(st3, "%.6f * %s", real_feature(m->leftcenter, 0), st2);
       sprintf(st4, "%.6f * %s", real_feature(m->rightcenter, 0), st2);
       for (i = 1; i < d->multifeaturecount - 1; i++)
        {
         sprintf(st, "%d", i);
         code_array_index_variable(st2, language, "testdataconverted", st);
         sprintf(st3, "%s + %.6f * %s", st3, real_feature(m->leftcenter, i), st2);
         sprintf(st4, "%s + %.6f * %s", st4, real_feature(m->rightcenter, i), st2);
        }
      }
   code_if(testcodefile, language, "<=", st3, st4); 
   code_regressionnode(testcodefile, language, d, m->left, level + 1);
   code_else(testcodefile, language);
   code_regressionnode(testcodefile, language, d, m->right, level + 1);
   code_endif(testcodefile, language);      
  }
 else
  {
   if (m->leaftype == CONSTANTLEAF)
     code_function_return_regression(testcodefile, "regtree", language, m->regressionvalue);
   else
    {
     sprintf(st3, "%.6f", m->w.values[0][0]);
     for (i = 0; i < d->multifeaturecount - 1; i++)
      {
       sprintf(st, "%d", i);
       code_array_index_variable(st2, language, "testdataconverted", st);
       sprintf(st3, "%s + %.6f * %s", st3, m->w.values[i + 1][0], st2);
      }
     code_function_return_string(testcodefile, "regtree", language, st3);
    }
  }
}

void code_decisionnode(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* algorithm, char* arrayname, Datasetptr d, Decisionnodeptr m, int level)
{
 /*!Last Changed 24.06.2006 added linear model*/
 /*!Last Changed 14.06.2006*/
 int i, *cases;
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH], operand1[MAXLENGTH], operand2[MAXLENGTH];
 if (m->featureno != LEAF_NODE)
  {
		 if (m->featureno >= 0)
			 {
     if (d->features[m->featureno].type == STRING)
					 {
       cases = safemalloc(sizeof(int), "code_decisionnode", 3);
       code_array_index(st, language, arrayname, m->featureno);
       code_switch(testcodefile, language, st);
       for (i = 0; i < d->features[m->featureno].valuecount; i++)
							 {
         cases[0] = i;
         code_case(testcodefile, language, cases, 1);
         code_decisionnode(testcodefile, language, algorithm, arrayname, d, &(m->string[i]), level + 1);
         code_endcase(testcodefile, language);
							 }
       code_endswitch(testcodefile, language);
       safe_free(cases);
					 }
     else
					 {
       code_array_index(st, language, arrayname, m->featureno);
       sprintf(st2, "%.6f", m->split);
       code_if(testcodefile, language, "<=", st, st2); 
       code_decisionnode(testcodefile, language, algorithm, arrayname, d, m->left, level + 1);
       code_else(testcodefile, language);
       code_decisionnode(testcodefile, language, algorithm, arrayname, d, m->right, level + 1);
       code_endif(testcodefile, language);
					 }
			 }
			else
     if (m->featureno == LINEAR)
					 {
       sprintf(operand2, "%.6f", m->w0);
       sprintf(st2, "%d", 0);
       code_array_index_variable(st3, language, "testdataconverted", st2);
       sprintf(operand1, "%.6f * %s", m->w.values[0][0], st3);
       for (i = 1; i < d->multifeaturecount - 1; i++)
							 {
         sprintf(st2, "%d", i);
         code_array_index_variable(st3, language, "testdataconverted", st2);
         sprintf(operand1, "%s + %.6f * %s", operand1, m->w.values[0][i], st3);
							 }     
       code_if(testcodefile, language, "<=", operand1, operand2); 
       code_decisionnode(testcodefile, language, algorithm, arrayname, d, m->left, level + 1);
       code_else(testcodefile, language);
       code_decisionnode(testcodefile, language, algorithm, arrayname, d, m->right, level + 1);
       code_endif(testcodefile, language);
					 }
	 }
 else
   code_function_return_classification(testcodefile, algorithm, language, m->classno);
}

void code_condition(PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionconditionptr condition, char* st)
{
 /*!Last Changed 15.06.2006*/ 
 char stop[3], operand1[MAXLENGTH], operand2[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH];
 int i;
 switch (condition->comparison){
   case '<':strcpy(stop, "<=");
            break;
   case '>':strcpy(stop, ">");
            break;
   case '=':strcpy(stop, "==");
            break;
 }
 if (condition->featureindex >= 0)
  {
   code_array_index(operand1, language, "testdata", condition->featureindex);
   switch (d->features[condition->featureindex].type){
     case INTEGER:
     case STRING :sprintf(operand2, "%d", (int) condition->value);
                  break;
     case REAL   :sprintf(operand2, "%.6f", condition->value);
                  break;
   }
  }
 else
   if (condition->featureindex == LINEAR)
    {
     sprintf(operand2, "%.6f", condition->w0);
     sprintf(st2, "%d", 0);
     code_array_index_variable(st3, language, "testdataconverted", st2);
     sprintf(operand1, "%.6f * %s", condition->w.values[0][0], st3);
     for (i = 1; i < d->multifeaturecount - 1; i++)
      {
       sprintf(st2, "%d", i);
       code_array_index_variable(st3, language, "testdataconverted", st2);
       sprintf(operand1, "%s + %.6f * %s", operand1, condition->w.values[0][i], st3);
      }     
    }
 code_simple_logical_expression(language, stop, operand1, operand2, st);
}

void code_rule(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithm, Decisionruleptr rule)
{
 /*!Last Changed 15.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH];
 Decisionconditionptr condition;
 condition = rule->start;
 code_condition(language, d, condition, st);
 condition = condition->next;
 strcpy(st2, st);
 while (condition)
  {
   code_condition(language, d, condition, st);
   switch (language){
     case C_LANGUAGE      :
     case CPP_LANGUAGE    :
     case JAVA_LANGUAGE   :sprintf(st2, "%s && %s", st2, st);
                           break;
     case PASCAL_LANGUAGE :sprintf(st2, "%s and %s", st2, st);
                           break;
     case FORTRAN_LANGUAGE:sprintf(st2, "%s .AND. %s", st2, st);
                           break;
     case MATLAB_LANGUAGE :sprintf(st2, "%s & %s", st2, st);
                           break;
   }
   condition = condition->next;
  }
 code_if_core(testcodefile, language, st2);
}

void code_dimension_reduce(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int newdimension)
{
 /*!Last Changed 16.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH];
 code_array_initialize(testcodefile, language, "testdatareduced", newdimension, "0.0");
 code_for(testcodefile, language, "i", 0, d->multifeaturecount - 2, 1);   
 code_array_index_variable(st, language, "testdataconverted", "i"); 
 code_set_variable(testcodefile, language, "x_i", st);
 code_for(testcodefile, language, "j", 0, newdimension - 1, 1);
 code_array_index_variable(st2, language, "testdatareduced", "j"); 
 code_2d_array_index_variable(st, language, "eigenvectors", "i", "j");
 code_set_variable(testcodefile, language, "eigenvectors_ij", st);
 sprintf(st3, "%s + eigenvectors_ij * x_i", st2);
 code_set_variable(testcodefile, language, st2, st3);
 code_endfor(testcodefile, language, "j");
 code_endfor(testcodefile, language, "i");
}

void code_forward_propagation(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int row, int col, char* weightname, char* inputname, char* outputname, int sigmoid)
{
 /*!Last Changed 16.06.2006*/
 char st[MAXLENGTH];
 code_for(testcodefile, language, "i", 0, row - 1, 1);     
 code_2d_array_index_variable(st, language, weightname, "i", "0");
 code_set_variable(testcodefile, language, "sum", st);   
 code_for(testcodefile, language, "j", 1, col - 1, 1);
 code_2d_array_index_variable(st, language, weightname, "i", "j");
 code_set_variable(testcodefile, language, "weights_ij", st);        
 code_array_index_variable(st, language, inputname, "j - 1");            
 code_set_variable(testcodefile, language, "x_j", st);        
 code_set_variable(testcodefile, language, "sum", "sum + weights_ij * x_j");        
 code_endfor(testcodefile, language, "j");
 code_array_index_variable(st, language, outputname, "i");            
 if (sigmoid)
  {
   code_set_variable(testcodefile, language, "sigmoid", "1 / (1 + exp(-sum))");
   code_set_variable(testcodefile, language, st, "sigmoid");
  }
 else
   code_set_variable(testcodefile, language, st, "sum");
 code_endfor(testcodefile, language, "i");
}

void code_forward_propagation_regression_output(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, int col, char* weightname, char* inputname, char* outputname)
{
 /*!Last Changed 24.06.2006*/
 char st[MAXLENGTH];
 code_array_index_variable(st, language, weightname, "0");
 code_set_variable(testcodefile, language, "sum", st);   
 code_for(testcodefile, language, "j", 1, col - 1, 1);
 code_array_index_variable(st, language, weightname, "j");
 code_set_variable(testcodefile, language, "weights_j", st);        
 code_array_index_variable(st, language, inputname, "j - 1");            
 code_set_variable(testcodefile, language, "x_j", st);        
 code_set_variable(testcodefile, language, "sum", "sum + weights_j * x_j");        
 code_endfor(testcodefile, language, "j");
 code_set_variable(testcodefile, language, outputname, "sum");
}

void code_compare_and_set_bound(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* comparison, char* value, char* bestbound, char* index)
{
 /*!Last Changed 16.06.2006*/
 code_if(testcodefile, language, comparison, value, bestbound);
 code_set_variable(testcodefile, language, bestbound, value);
 code_set_variable(testcodefile, language, index, "i"); 
 code_endif(testcodefile, language);
}

void code_find_bound_of_list(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* listname, int list_length, char* initialbound, char* bestbound, char* index, char* comparison)
{
 /*!Last Changed 16.06.2006*/
 char st[MAXLENGTH];
 code_set_variable(testcodefile, language, bestbound, initialbound);
 code_for(testcodefile, language, "i", 0, list_length - 1, 1);   
 code_array_index_variable(st, language, listname, "i");  
 code_compare_and_set_bound(testcodefile, language, comparison, st, bestbound, index);
 code_endfor(testcodefile, language, "i");
}

void code_increment(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, char* arrayname, char* index)
{
 /*!Last Changed 25.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH];
 code_array_index_variable(st, language, arrayname, index);  
 strcpy(st2, st);
 sprintf(st2, "%s + 1", st2);
 code_set_variable(testcodefile, language, st, st2);
}

void code_realize(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int normalize)
{
 /*!Last Changed 01.20.2006 fixed for loop's ending value*/
 /*!Last Changed 24.06.2006*/
 int *symbolic, *numeric, scount, ncount, i, *cases;
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH], st4[MAXLENGTH], st5[MAXLENGTH];
 code_array_index_variable(st4, language, "mean", "a");  
 code_array_index_variable(st5, language, "variance", "a");  
	cases = safemalloc(sizeof(int), "code_realize", 5);
 symbolic = symbolic_features(d, &scount);
 numeric = numeric_features(d, &ncount);
 code_set_variable(testcodefile, language, "j", "0");
	if (normalize)
   code_set_variable(testcodefile, language, "a", "0");
 code_for(testcodefile, language, "i", 0, d->featurecount - 1, 1);   
 code_switch(testcodefile, language, "i");
	if (ncount > 0)
	 {
  	code_case(testcodefile, language, numeric, ncount);
   code_array_index_variable(st, language, "testdataconverted", "j");  
   code_array_index_variable(st2, language, "testdata", "i");  
	  if (normalize)
			 {
	  	 sprintf(st3, "(%s - %s) / sqrt(%s)", st2, st4, st5);
     code_set_variable(testcodefile, language, st, st3);
     code_set_variable(testcodefile, language, "a", "a + 1");
			 }
	  else
     code_set_variable(testcodefile, language, st, st2);
   code_set_variable(testcodefile, language, "j", "j + 1");
	  code_endcase(testcodefile, language);
	 }
 for (i = 0; i < scount; i++)
	 {
		 cases[0] = symbolic[i];
  	code_case(testcodefile, language, cases, 1);
			code_for(testcodefile, language, "k", 0, d->features[i].valuecount, 1);
			code_if(testcodefile, language, "==", st2, "k");
			if (normalize)
			 {
  		 sprintf(st3, "(1.0 - %s) / sqrt(%s)", st4, st5);
     code_set_variable(testcodefile, language, st, st3);
			 }
			else
     code_set_variable(testcodefile, language, st, "1.0");
			code_else(testcodefile, language);
			if (normalize)
			 {
  		 sprintf(st3, "-%s / sqrt(%s)", st4, st5);
     code_set_variable(testcodefile, language, st, st3);
			 }
			else
     code_set_variable(testcodefile, language, st, "0.0");
			code_endif(testcodefile, language);
   if (normalize)
     code_set_variable(testcodefile, language, "a", "a + 1");
			code_set_variable(testcodefile, language, "j", "j + 1");
			code_endfor(testcodefile, language, "k");
			code_endcase(testcodefile, language);
	 }
	cases[0] = d->classdefno;
	code_case(testcodefile, language, cases, 1);
	sprintf(st3, "%d", d->multifeaturecount - 1);
 code_array_index_variable(st, language, "testdataconverted", st3);  
 code_set_variable(testcodefile, language, st, st2);
	if (normalize)
   code_set_variable(testcodefile, language, "a", "a + 1");
 code_endcase(testcodefile, language);
	code_endswitch(testcodefile, language);
 code_endfor(testcodefile, language, "i");
	safe_free(symbolic);
	safe_free(numeric);
}

void code_normalize(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d)
{
 /*!Last Changed 26.06.2006*/
 int *symbolic, *numeric, scount, ncount;
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH], st4[MAXLENGTH], st5[MAXLENGTH];
 code_array_index_variable(st4, language, "mean", "i");  
 code_array_index_variable(st5, language, "variance", "i");  
 symbolic = symbolic_features(d, &scount);
 numeric = numeric_features(d, &ncount);
 code_for(testcodefile, language, "i", 0, d->featurecount, 1);   
 code_switch(testcodefile, language, "i");
	if (ncount > 0)
	 {
  	code_case(testcodefile, language, numeric, ncount);
   code_array_index_variable(st, language, "testdataconverted", "i");  
   code_array_index_variable(st2, language, "testdata", "i");  
  	sprintf(st3, "(%s - %s) / sqrt(%s)", st2, st4, st5);
   code_set_variable(testcodefile, language, st, st3);
  	code_endcase(testcodefile, language);
	 }
	if (scount > 0)
	 {
  	code_case(testcodefile, language, symbolic, scount);
   code_set_variable(testcodefile, language, st, st2);
	  code_endcase(testcodefile, language);
	 }
	code_endswitch(testcodefile, language);
 code_endfor(testcodefile, language, "i");
	safe_free(symbolic);
	safe_free(numeric);
}

void code_kernel_function(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Svm_parameter param)
{
 /*!Last Changed 25.06.2006*/
	char st[MAXLENGTH];
 code_set_variable(testcodefile, language, "sum", "0.0");
	code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_array_index_variable(st, language, "testdataconverted", "j");  				             
	code_set_variable(testcodefile, language, "x_j", st);
 code_2d_array_index_variable(st, language, "sv", "i", "j");  
	code_set_variable(testcodefile, language, "sv_ij", st);
	switch (param.kernel_type){
	  case KLINEAR :
			case KPOLY   :
			case KSIGMOID:code_set_variable(testcodefile, language, "sum", "sum + x_j * sv_ij");
				             break;
			case KRBF    :code_set_variable(testcodefile, language, "sum", "sum + (x_j - sv_ij) * (x_j - sv_ij)");
				             break;
   case KPREDEFINED:break;
	};	
	code_endfor(testcodefile, language, "j");
	switch (param.kernel_type){
			case KPOLY   :code_set_variable(testcodefile, language, "sum", "pow(gamma * sum + coef0, degree)");
				             break;
			case KSIGMOID:code_set_variable(testcodefile, language, "sum", "tanh(gamma * sum + coef0)");
				             break;
			case KRBF    :code_set_variable(testcodefile, language, "sum", "exp(-gamma * sum)");
				             break;
   default      :break;
	};	
}

void generate_test_code_selectmax(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_selectmaxptr m)
{
 /*!Last Changed 17.05.2006*/
 code_function_header(testcodefile, "selectmax", d->featurecount, language);
 code_start(testcodefile, language);
 code_function_return_classification(testcodefile, "selectmax", language, m->classno);
 code_function_end(testcodefile, language);
}

void generate_test_code_onefeature(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_onefeatureptr m)
{
 /*!Last Changed 02.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH];
 code_function_header(testcodefile, "onefeature", d->featurecount, language);
 code_start(testcodefile, language);
 code_array_index(st, language, "testdata", m->bestfeature);
 sprintf(st2, "%.6f", m->bestsplit);
 code_if(testcodefile, language, "<=", st, st2);
 code_function_return_regression(testcodefile, "onefeature", language, m->leftmean);
 code_else(testcodefile, language);
 code_function_return_regression(testcodefile, "onefeature", language, m->rightmean);
 code_endif(testcodefile, language);
 code_function_end(testcodefile, language);
}

void generate_test_code_selectaverage(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, double* m)
{
 /*!Last Changed 02.06.2006*/
 code_function_header(testcodefile, "selectaverage", d->featurecount, language);
 code_start(testcodefile, language);
 code_function_return_regression(testcodefile, "selectaverage", language, *m);
 code_function_end(testcodefile, language);
}

void generate_test_code_gaussian(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_gaussianptr m)
{
 /*!Last Changed 05.06.2006*/
 char st[MAXLENGTH];
 code_function_header(testcodefile, "gaussian", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_1d_array_constant(testcodefile, language, "priors", d->classno, m->priors, REAL_TYPE);
 code_1d_array_constant(testcodefile, language, "v", d->classno, m->variance, REAL_TYPE);
 code_2d_array_constant_instance(testcodefile, language, d, "m", d->classno, d->multifeaturecount - 1, m->mean);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 5, "i", "j", "maxindex", "k", "a");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 6, "sum", "maxposterior", "x_j", "m_ij", "v_i", "p_i", "posterior");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);  
	code_realize(testcodefile, language, d, YES);
 code_set_variable(testcodefile, language, "maxposterior", "0.0");
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);
 code_set_variable(testcodefile, language, "sum", "0.0");
 code_array_index_variable(st, language, "v", "i");
 code_set_variable(testcodefile, language, "v_i", st);
 code_array_index_variable(st, language, "priors", "i");
 code_set_variable(testcodefile, language, "p_i", st);
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_array_index_variable(st, language, "testdataconverted", "j");
 code_set_variable(testcodefile, language, "x_j", st);
 code_2d_array_index_variable(st, language, "m", "i", "j");
 code_set_variable(testcodefile, language, "m_ij", st);
 code_set_variable(testcodefile, language, "sum", "sum + (x_j - m_ij) * (x_j - m_ij)");
 code_endfor(testcodefile, language, "j");
 code_set_variable(testcodefile, language, "posterior", "(p_i / (v_i * sqrt(2 * 3.1415926))) * exp(-sum / (2 * v_i * v_i))");
 code_compare_and_set_bound(testcodefile, language, ">", "posterior", "maxposterior", "maxindex"); 
 code_endfor(testcodefile, language, "i");
 code_function_return_string(testcodefile, "gaussian", language, "maxindex");
 code_function_end(testcodefile, language);
}

void generate_test_code_nearestmean(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Instanceptr m)
{
 /*!Last Changed 09.06.2006*/
 code_function_header(testcodefile, "nearestmean", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->featurecount, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->featurecount, &variance);
 code_2d_array_constant_instance(testcodefile, language, d, "m", d->classno, d->featurecount, m);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 3, "i", "j", "minindex");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 4, "sum", "x_j", "m_ij", "mindistance");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->featurecount);
 code_start(testcodefile, language);  
 code_normalize(testcodefile, language, d);
 code_set_variable(testcodefile, language, "mindistance", "10000.0"); 
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);
 code_calculate_distance(testcodefile, language, d);
 code_compare_and_set_bound(testcodefile, language, "<", "sum", "mindistance", "minindex"); 
 code_endfor(testcodefile, language, "i");
 code_function_return_string(testcodefile, "nearestmean", language, "minindex");
 code_function_end(testcodefile, language);
}

void generate_test_code_linearreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_linearregressionptr m)
{
 /*!Last Changed 01.20.2006 fixed for loop's ending value*/
 /*!Last Changed 12.06.2006*/
 char st[MAXLENGTH];
 code_function_header(testcodefile, "linearreg", d->featurecount, language);
 code_constant_start(testcodefile, language); 
 code_1d_array_constant(testcodefile, language, "w", d->multifeaturecount - 1, m->w.values[0], REAL_TYPE);
 code_constant_real(testcodefile, language, "b", m->b);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 2, "i", "j");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 3, "w_i", "x_i", "sum");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);  
	code_realize(testcodefile, language, d, NO);
 code_set_variable(testcodefile, language, "sum", "0.0"); 
 code_for(testcodefile, language, "i", 0, d->multifeaturecount - 2, 1);
 code_array_index_variable(st, language, "testdataconverted", "i"); 
 code_set_variable(testcodefile, language, "x_i", st);
 code_array_index_variable(st, language, "w", "i"); 
 code_set_variable(testcodefile, language, "w_i", st);
 code_set_variable(testcodefile, language, "sum", "sum + w_i * x_i");
 code_endfor(testcodefile, language, "i");
 code_set_variable(testcodefile, language, "sum", "sum + b");
 code_function_return_string(testcodefile, "linearreg", language, "sum");
 code_function_end(testcodefile, language);
}

void generate_test_code_quantizereg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_quantizeregptr m)
{
 /*!Last Changed 12.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH];
 code_function_header(testcodefile, "quantizereg", d->featurecount, language);
 code_constant_start(testcodefile, language); 
 code_constant_real(testcodefile, language, "minx", m->minx);
 code_constant_real(testcodefile, language, "binx", m->binx);
 if (d->featurecount > 2)
  {
   code_2d_array_constant(testcodefile, language, "means", m->partitioncount, m->partitioncount, m->meanstwo);
   code_2d_array_constant(testcodefile, language, "eigenvectors", m->eigenvectors.row, m->eigenvectors.col, m->eigenvectors.values);
   code_constant_real(testcodefile, language, "miny", m->miny);
   code_constant_real(testcodefile, language, "biny", m->biny);
   code_variable_start(testcodefile, language);
   code_add_variable(testcodefile, language, INTEGER_VARIABLE, 3, "i", "x", "y");
   code_add_variable(testcodefile, language, REAL_VARIABLE, 2, "eigenvectors_ij", "x_i");
   code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdatareduced", 2);
   code_start(testcodefile, language);  
   code_dimension_reduce(testcodefile, language, d, 2);   
   code_array_index_variable(st, language, "testdatareduced", "0"); 
   sprintf(st2, "(%s - minx) / binx", st);
   code_set_variable(testcodefile, language, "x", st2);
   code_if(testcodefile, language, "<", "x", "0");
   code_set_variable(testcodefile, language, "x", "0");
   code_endif(testcodefile, language);
   sprintf(st, "%d", d->multifeaturecount - 1);
   code_if(testcodefile, language, ">", "x", st);
   code_set_variable(testcodefile, language, "x", st);   
   code_endif(testcodefile, language);
   code_array_index_variable(st, language, "testdatareduced", "1"); 
   sprintf(st2, "(%s - miny) / biny", st);
   code_set_variable(testcodefile, language, "y", st2);
   code_if(testcodefile, language, "<", "y", "0");
   code_set_variable(testcodefile, language, "y", "0");
   code_endif(testcodefile, language);
   sprintf(st, "%d", d->multifeaturecount - 1);
   code_if(testcodefile, language, ">", "y", st);
   code_set_variable(testcodefile, language, "y", st);   
   code_endif(testcodefile, language);
   code_2d_array_index_variable(st, language, "means", "x", "y");
  }
 else
  {
   code_1d_array_constant(testcodefile, language, "means", m->partitioncount, m->meansone, REAL_TYPE);
   code_variable_start(testcodefile, language);
   code_add_variable(testcodefile, language, INTEGER_VARIABLE, 1, "x");
   code_start(testcodefile, language);  
   code_array_index_variable(st, language, "testdataconverted", "0"); 
   sprintf(st2, "(%s - minx) / binx", st);
   code_set_variable(testcodefile, language, "x", st2);
   code_if(testcodefile, language, "<", "x", "0");
   code_set_variable(testcodefile, language, "x", "0");
   code_endif(testcodefile, language);
   sprintf(st, "%d", d->multifeaturecount - 1);
   code_if(testcodefile, language, ">", "x", st);
   code_set_variable(testcodefile, language, "x", st);   
   code_endif(testcodefile, language);   
   code_array_index_variable(st, language, "means", "x");
  }
 code_function_return_string(testcodefile, "quantizereg", language, st);   
 code_function_end(testcodefile, language);
}

void generate_test_code_knn(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int nearcount, Instanceptr m)
{
 /*!Last Changed 25.06.2006 added code_increment*/
 /*!Last Changed 13.06.2006*/
 char st[MAXLENGTH];
 int size = data_size(m);
 code_function_header(testcodefile, "knn", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->featurecount, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->featurecount, &variance);
 code_constant_integer(testcodefile, language, "classno", d->classdefno);
 code_2d_array_constant_instance(testcodefile, language, d, "m", size, d->featurecount, m);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 6, "i", "j", "k", "maxindex", "maxcount", "count");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 3, "sum", "m_ij", "x_j");
 code_add_array_variable(testcodefile, language, 4, REAL_VARIABLE, "distances", nearcount, INTEGER_VARIABLE, "indexes", nearcount, INTEGER_VARIABLE, "class_counts", d->classno, REAL_VARIABLE, "testdataconverted", d->featurecount);
 code_start(testcodefile, language); 
 code_normalize(testcodefile, language, d);
 code_array_initialize(testcodefile, language, "class_counts", d->classno, "0");
 code_array_initialize(testcodefile, language, "distances", nearcount, "10000.0");
 code_array_initialize(testcodefile, language, "indexes", nearcount, "-1");
 code_sort_according_to_distances(testcodefile, language, d, size, nearcount);
 code_for(testcodefile, language, "i", 0, nearcount - 1, 1);     
 code_array_index_variable(st, language, "indexes", "i");  
 code_set_variable(testcodefile, language, "j", st); 
 code_2d_array_index_variable(st, language, "m", "j", "classno");
 code_set_variable(testcodefile, language, "k", st);
	code_increment(testcodefile, language, "class_counts", "k");
 code_endfor(testcodefile, language, "i");
 code_find_bound_of_list(testcodefile, language, "class_counts", d->classno, "-1", "maxcount", "maxindex", ">");
 code_function_return_string(testcodefile, "knn", language, "maxindex");   
 code_function_end(testcodefile, language);
}

void generate_test_code_knnreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, int nearcount, Instanceptr m)
{
 /*!Last Changed 13.06.2006*/
 char st[MAXLENGTH];
 int size = data_size(m);
 code_function_header(testcodefile, "knnreg", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->featurecount, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->featurecount, &variance);
 code_constant_integer(testcodefile, language, "regno", d->classdefno);
 code_2d_array_constant_instance(testcodefile, language, d, "m", size, d->featurecount, m);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 2, "i", "j");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 4, "sum", "m_ij", "x_j", "regvalue");
 code_add_array_variable(testcodefile, language, 3, REAL_VARIABLE, "distances", nearcount, INTEGER_VARIABLE, "indexes", nearcount, REAL_VARIABLE, "testdataconverted", d->featurecount);
 code_start(testcodefile, language);
	code_normalize(testcodefile, language, d);
 code_array_initialize(testcodefile, language, "distances", nearcount, "10000.0");
 code_array_initialize(testcodefile, language, "indexes", nearcount, "-1");
 code_sort_according_to_distances(testcodefile, language, d, size, nearcount);
 code_set_variable(testcodefile, language, "sum", "0.0");
 code_for(testcodefile, language, "i", 0, nearcount - 1, 1);     
 code_array_index_variable(st, language, "indexes", "i");  
 code_set_variable(testcodefile, language, "j", st); 
 code_2d_array_index_variable(st, language, "m", "j", "regno");
 code_set_variable(testcodefile, language, "regvalue", st);
 code_set_variable(testcodefile, language, "sum", "sum + regvalue");
 code_endfor(testcodefile, language, "i");
 sprintf(st, "sum / %d", nearcount);
 code_function_return_string(testcodefile, "knnreg", language, st);   
 code_function_end(testcodefile, language);
}

void generate_test_code_c45(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m)
{
 /*!Last Changed 14.06.2006*/
 code_function_header(testcodefile, "c45", d->featurecount, language);
 code_start(testcodefile, language);  
 code_decisionnode(testcodefile, language, "c45", "testdata", d, m, 0);
 code_function_end(testcodefile, language);
}

void generate_test_code_ldt(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m)
{
 /*!Last Changed 14.06.2006*/
 code_function_header(testcodefile, "ldt", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->featurecount, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->featurecount, &variance);
	code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 1, "i");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->featurecount);
 code_start(testcodefile, language);  
 code_normalize(testcodefile, language, d);
 code_decisionnode(testcodefile, language, "ldt", "testdataconverted", d, m, 0);
 code_function_end(testcodefile, language);
}

void generate_test_code_svmtree(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m)
{
 /*!Last Changed 14.06.2006*/
 code_function_header(testcodefile, "svmtree", d->featurecount, language);
 code_constant_start(testcodefile, language);
 code_1d_array_constant_instance(testcodefile, language, d, "mean", d->featurecount, &mean);
 code_1d_array_constant_instance(testcodefile, language, d, "variance", d->featurecount, &variance);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 1, "i");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->featurecount);
 code_start(testcodefile, language);  
 code_normalize(testcodefile, language, d);
 code_decisionnode(testcodefile, language, "ldt", "testdataconverted", d, m, 0);
 code_function_end(testcodefile, language);
}

void generate_test_code_regtree(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Regressionnodeptr m)
{
 /*!Last Changed 14.06.2006*/
 code_function_header(testcodefile, "regtree", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
	code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 4, "i", "j", "k", "a");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
 code_regressionnode(testcodefile, language, d, m, 0);
 code_function_end(testcodefile, language);
}

void generate_test_code_ripper(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithm, Rulesetptr m)
{
 /*!Last Changed 15.06.2006*/
 int i;
 Decisionruleptr rule;
 code_function_header(testcodefile, algorithm, d->featurecount, language);
	code_start(testcodefile, language);
 rule = m->start;
 while (rule)
  {
   code_rule(testcodefile, language, d, algorithm, rule);
   code_function_return_classification(testcodefile, algorithm, language, rule->classno);   
   code_else(testcodefile, language);
   rule = rule->next;
  }
 code_function_return_classification(testcodefile, algorithm, language, m->defaultclass);
 for (i = 0; i < m->rulecount; i++)
   code_endif(testcodefile, language);
 code_function_end(testcodefile, language);
}

void generate_test_code_multiripper(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithm, Rulesetptr m)
{
 /*!Last Changed 26.06.2006*/
 int i;
 Decisionruleptr rule;
 code_function_header(testcodefile, algorithm, d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
	code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 4, "i", "j", "k", "a");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
	code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
 rule = m->start;
 while (rule)
  {
   code_rule(testcodefile, language, d, algorithm, rule);
   code_function_return_classification(testcodefile, algorithm, language, rule->classno);   
   code_else(testcodefile, language);
   rule = rule->next;
  }
 code_function_return_classification(testcodefile, algorithm, language, m->defaultclass);
 for (i = 0; i < m->rulecount; i++)
   code_endif(testcodefile, language);
 code_function_end(testcodefile, language);
}

void generate_test_code_c45rules(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rulesetptr m)
{
 /*!Last Changed 15.06.2006*/
 char st[MAXLENGTH];
 Decisionruleptr rule;
 int i, classno = -1, rulecount = 0;
 code_function_header(testcodefile, "c45rules", d->featurecount, language);
 code_variable_start(testcodefile, language); 
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 1, "bestclass");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 2, "bestestimate", "laplaceestimate");
 code_start(testcodefile, language);
 code_set_variable(testcodefile, language, "bestestimate", "1.5"); 
 rule = m->start;
 while (rule)
  {
   if (rule->classno != classno)
    {
     if (classno != -1)
      {
       for (i = 0; i < rulecount; i++)
         code_endif(testcodefile, language);
      }
     classno = rule->classno;     
     rulecount = 0;
    }
   code_rule(testcodefile, language, d, "c45rules", rule);
   sprintf(st, "(%d + 1.0) / (%d + %d + 2.0)", rule->falsepositives, rule->covered, rule->falsepositives);
   code_set_variable(testcodefile, language, "laplaceestimate", st);
   code_if(testcodefile, language, "<", "laplaceestimate", "bestestimate");
   code_set_variable(testcodefile, language, "bestestimate", "laplaceestimate");
   sprintf(st, "%d", rule->classno);
   code_set_variable(testcodefile, language, "bestclass", st);
   code_endif(testcodefile, language);
   if (rule->next && rule->next->classno == rule->classno)
     code_else(testcodefile, language);
   rulecount++;
   rule = rule->next;
  }
 for (i = 0; i < rulecount; i++)
   code_endif(testcodefile, language);
 code_if(testcodefile, language, "!=", "bestestimate", "1.5");
 code_function_return_string(testcodefile, "c45rules", language, "bestclass");   
 code_else(testcodefile, language);
 code_function_return_classification(testcodefile, "c45rules", language, m->defaultclass);
 code_endif(testcodefile, language);
 code_function_end(testcodefile, language);
}

void generate_test_code_rise(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rulesetptr m)
{
 char st[MAXLENGTH], st2[MAXLENGTH];
 Decisionruleptr rule;
 code_function_header(testcodefile, "rise", d->featurecount, language);
 code_variable_start(testcodefile, language); 
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 3, "i", "maxcount", "maxindex"); 
 code_add_array_variable(testcodefile, language, 1, INTEGER_VARIABLE, "vote", d->classno);
 code_start(testcodefile, language);
 code_array_initialize(testcodefile, language, "vote", d->classno, "0");
 rule = m->start;
 while (rule)
  {
   code_rule(testcodefile, language, d, "rise", rule);
   code_array_index(st, language, "vote", rule->classno);
   sprintf(st2, "%s + %d", st, rule->covered - rule->falsepositives);
   code_set_variable(testcodefile, language, st, st2); 
   code_endif(testcodefile, language);
   rule = rule->next;
  }
 code_find_bound_of_list(testcodefile, language, "vote", d->classno, "-10000", "maxcount", "maxindex", ">");
 code_function_return_string(testcodefile, "rise", language, "maxindex");   
 code_function_end(testcodefile, language); 
}

void generate_test_code_ldaclass(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_ldaclassptr m)
{
 /*!Last Changed 16.06.2006*/
 char st[MAXLENGTH];
 int i;
 code_function_header(testcodefile, "ldaclass", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_1d_array_constant(testcodefile, language, "w0", d->classno, m->w0, REAL_TYPE);
 code_2d_array_start(testcodefile, language, "w", d->classno, m->newfeaturecount - 1);
 for (i = 0; i < d->classno; i++)
   code_constant_subarray(testcodefile, language, m->newfeaturecount - 1, m->w[i].values[0], i);
 code_2d_array_end(testcodefile, language);  
 code_2d_array_constant(testcodefile, language, "eigenvectors", m->eigenvectors.row, m->eigenvectors.col, m->eigenvectors.values);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 5, "i", "j", "maxindex", "k", "a");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 6, "w_ij", "eigenvectors_ij", "x_i", "x_j", "sum", "maxvalue");
 code_add_array_variable(testcodefile, language, 2, REAL_VARIABLE, "testdatareduced", m->newfeaturecount - 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
 code_dimension_reduce(testcodefile, language, d, m->newfeaturecount - 1);    
 code_set_variable(testcodefile, language, "maxvalue", "-10000.0"); 
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);     
 code_array_index_variable(st, language, "w0", "i");  
 code_set_variable(testcodefile, language, "sum", st); 
 code_for(testcodefile, language, "j", 0, m->newfeaturecount - 2, 1);     
 code_2d_array_index_variable(st, language, "w", "i", "j");
 code_set_variable(testcodefile, language, "w_ij", st);
 code_array_index_variable(st, language, "testdatareduced", "j");  
 code_set_variable(testcodefile, language, "x_j", st); 
 code_set_variable(testcodefile, language, "sum", "sum + w_ij * x_j"); 
 code_endfor(testcodefile, language, "j");
 code_compare_and_set_bound(testcodefile, language, ">", "sum", "maxvalue", "maxindex"); 
 code_endfor(testcodefile, language, "i");
 code_function_return_string(testcodefile, "ldaclass", language, "maxindex");   
 code_function_end(testcodefile, language);
}

void generate_test_code_logistic(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, double** m)
{
 /*!Last Changed 16.06.2006*/
 char st[MAXLENGTH];
 code_function_header(testcodefile, "logistic", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_2d_array_constant(testcodefile, language, "w", d->classno, d->multifeaturecount, m); 
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 5, "i", "j", "maxindex", "k", "a");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 4, "w_ij", "x_j", "sum", "maxvalue");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
 code_set_variable(testcodefile, language, "maxvalue", "-10000.0"); 
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);     
 code_2d_array_index_variable(st, language, "w", "i", "0");  
 code_set_variable(testcodefile, language, "sum", st); 
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);     
 code_2d_array_index_variable(st, language, "w", "i", "j + 1");
 code_set_variable(testcodefile, language, "w_ij", st);
 code_array_index_variable(st, language, "testdataconverted", "j");  
 code_set_variable(testcodefile, language, "x_j", st); 
 code_set_variable(testcodefile, language, "sum", "sum + w_ij * x_j"); 
 code_endfor(testcodefile, language, "j");
 code_compare_and_set_bound(testcodefile, language, ">", "sum", "maxvalue", "maxindex"); 
 code_endfor(testcodefile, language, "i");
 code_function_return_string(testcodefile, "logistic", language, "maxindex");   
 code_function_end(testcodefile, language);
}

void generate_test_code_naivebayes(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_naivebayesptr m)
{
 /*!Last Changed 12.08.2007*/
 char st[MAXLENGTH];
	int* numeric, ncount;
 numeric = numeric_features(d, &ncount);
 code_function_header(testcodefile, "naivebayes", d->featurecount, language);
 code_constant_start(testcodefile, language);
 code_1d_array_constant(testcodefile, language, "priors", d->classno, m->priors, REAL_TYPE);  
 code_1d_array_constant(testcodefile, language, "s", d->featurecount, m->s, REAL_TYPE);  
 code_2d_array_constant(testcodefile, language, "m", d->classno, d->featurecount, m->m);
 code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, REAL_VARIABLE, 7, "sum", "maxvalue", "gi", "p_i", "m_ij", "s_j", "x_j");
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 3, "i", "j", "maxindex");
 code_start(testcodefile, language);
 code_set_variable(testcodefile, language, "maxvalue", "-10000.0");  
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);     
 code_array_index_variable(st, language, "priors", "i");  
 code_set_variable(testcodefile, language, "p_i", st); 
 code_set_variable(testcodefile, language, "gi", "log(p_i)"); 
 code_for(testcodefile, language, "j", 0, d->featurecount - 1, 1); 
 code_array_index_variable(st, language, "s", "j");
 code_set_variable(testcodefile, language, "s_j", st);
 code_2d_array_index_variable(st, language, "m", "i", "j");
 code_set_variable(testcodefile, language, "m_ij", st);
 code_array_index_variable(st, language, "testdata", "j");
 code_set_variable(testcodefile, language, "x_j", st);
 code_switch(testcodefile, language, "j");
 code_case(testcodefile, language, numeric, ncount);
 code_if(testcodefile, language, "!=", "s_j", "0.0");
 code_set_variable(testcodefile, language, "gi", "gi - 0.5 * ((x_j - m_ij) / s_j) * ((x_j - m_ij) / s_j)");
	code_endif(testcodefile, language);
 code_endcase(testcodefile, language);
 code_endswitch(testcodefile, language);
 code_endfor(testcodefile, language, "j");  
 code_compare_and_set_bound(testcodefile, language, ">", "gi", "maxvalue", "maxindex"); 
 code_endfor(testcodefile, language, "i");
 code_function_return_string(testcodefile, "naivebayes", language, "maxindex");   
 code_function_end(testcodefile, language); 
	safe_free(numeric);
}

void generate_test_code_qdaclass(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Model_qdaclassptr m)
{
 /*!Last Changed 08.08.2007*/
 char st[MAXLENGTH];
 code_function_header(testcodefile, "qdaclass", d->featurecount, language);
 code_constant_start(testcodefile, language);
 code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
 code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_3d_array_constant_matrix(testcodefile, language, "wqua", d->classno, d->multifeaturecount - 1, d->multifeaturecount - 1, m->W);
 code_2d_array_constant_matrix(testcodefile, language, "wlin", d->classno, d->multifeaturecount - 1, m->w);
 code_1d_array_constant(testcodefile, language, "w0", d->multifeaturecount - 1, m->w0, REAL_TYPE);  
 code_variable_start(testcodefile, language);
 code_add_array_variable(testcodefile, language, 2, REAL_VARIABLE, "tmp1", d->multifeaturecount - 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_add_variable(testcodefile, language, REAL_VARIABLE, 8, "sum", "maxvalue", "w0_i", "w_j", "w_ij", "w_ijk", "x_k", "x_j", "gi");
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 5, "i", "j", "k", "maxindex", "a");
 code_start(testcodefile, language);
 code_realize(testcodefile, language, d, YES);
 code_set_variable(testcodefile, language, "maxvalue", "-10000.0");  
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);     
 code_set_variable(testcodefile, language, "gi", "0.0"); 
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_set_variable(testcodefile, language, "sum", "0.0"); 
 code_for(testcodefile, language, "k", 0, d->multifeaturecount - 2, 1);     
 code_3d_array_index_variable(st, language, "wqua", "i", "j", "k");
 code_set_variable(testcodefile, language, "w_ijk", st);
 code_array_index_variable(st, language, "testdataconverted", "k");  
 code_set_variable(testcodefile, language, "x_k", st); 
 code_set_variable(testcodefile, language, "sum", "sum + w_ijk * x_k"); 
 code_endfor(testcodefile, language, "k");
 code_array_index_variable(st, language, "tmp1", "j");  
 code_set_variable(testcodefile, language, st, "sum"); 
 code_endfor(testcodefile, language, "j");
 code_set_variable(testcodefile, language, "sum", "0.0"); 
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_array_index_variable(st, language, "testdataconverted", "j");  
 code_set_variable(testcodefile, language, "x_j", st); 
 code_array_index_variable(st, language, "tmp1", "i");  
 code_set_variable(testcodefile, language, "w_j", st); 
 code_set_variable(testcodefile, language, "sum", "sum + w_j * x_j");  
 code_endfor(testcodefile, language, "j");
 code_set_variable(testcodefile, language, "gi", "gi + sum"); 
 code_set_variable(testcodefile, language, "sum", "0.0"); 
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_array_index_variable(st, language, "testdataconverted", "j");  
 code_set_variable(testcodefile, language, "x_j", st); 
 code_2d_array_index_variable(st, language, "wlin", "i", "j");
 code_set_variable(testcodefile, language, "w_ij", st);
 code_set_variable(testcodefile, language, "sum", "sum + w_ij * x_j");  
 code_endfor(testcodefile, language, "j");
 code_set_variable(testcodefile, language, "gi", "gi + sum"); 
 code_array_index_variable(st, language, "w0", "i");  
 code_set_variable(testcodefile, language, "w0_i", st); 
 code_set_variable(testcodefile, language, "gi", "gi + w0_i");  
 code_compare_and_set_bound(testcodefile, language, ">", "gi", "maxvalue", "maxindex"); 
 code_endfor(testcodefile, language, "i");
 code_function_return_string(testcodefile, "qdaclass", language, "maxindex");   
 code_function_end(testcodefile, language); 
}

void generate_test_code_rbf(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rbfnetworkptr m)
{
 /*!Last Changed 21.08.2007*/
 char st[MAXLENGTH];
 code_function_header(testcodefile, "rbf", d->featurecount, language);
 code_constant_start(testcodefile, language);
 code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
 code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_2d_array_constant(testcodefile, language, "m", m->weights.m.row, m->weights.m.col, m->weights.m.values);
 code_2d_array_constant(testcodefile, language, "w", m->weights.w.row, m->weights.w.col, m->weights.w.values);
 code_1d_array_constant(testcodefile, language, "s", m->weights.s.col, m->weights.s.values[0], REAL_TYPE);
 code_variable_start(testcodefile, language);
 code_add_array_variable(testcodefile, language, 3, REAL_VARIABLE, "p", m->hidden + 1, REAL_VARIABLE, "y", m->output, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_add_variable(testcodefile, language, REAL_VARIABLE, 8, "p_h", "s_h", "dist", "sum", "w_ih", "m_hj", "x_j", "maxvalue");
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 6, "i", "j", "h", "maxindex", "k", "a");
 code_start(testcodefile, language);
 code_realize(testcodefile, language, d, YES);
 code_array_index_variable(st, language, "p", "0");
 code_set_variable(testcodefile, language, st, "1.0");
 code_for(testcodefile, language, "h", 1, m->hidden, 1);
 code_set_variable(testcodefile, language, "dist", "0.0");
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_2d_array_index_variable(st, language, "m", "h", "j");
 code_set_variable(testcodefile, language, "m_hj", st);
 code_array_index_variable(st, language, "testdataconverted", "j");  
 code_set_variable(testcodefile, language, "x_j", st); 
 code_set_variable(testcodefile, language, "dist", "dist + m_hj * x_j");    
 code_endfor(testcodefile, language, "j");
 code_array_index_variable(st, language, "s", "h");  
 code_set_variable(testcodefile, language, "s_h", st);
 code_array_index_variable(st, language, "p", "h");  
 code_set_variable(testcodefile, language, st, "exp(-dist / (s_h * s_h))");
 code_endfor(testcodefile, language, "h");
 code_for(testcodefile, language, "i", 0, d->classno - 1, 1);
 code_set_variable(testcodefile, language, "sum", "0.0");
 code_for(testcodefile, language, "h", 0, m->hidden, 1);
 code_2d_array_index_variable(st, language, "w", "i", "h");
 code_set_variable(testcodefile, language, "w_ih", st);
 code_array_index_variable(st, language, "p", "h");  
 code_set_variable(testcodefile, language, "p_h", st);
 code_set_variable(testcodefile, language, "sum", "sum + w_ih * p_h");    
 code_endfor(testcodefile, language, "h");
 code_array_index_variable(st, language, "y", "i");
 code_set_variable(testcodefile, language, st, "sum");
 code_endfor(testcodefile, language, "i");
 code_find_bound_of_list(testcodefile, language, "y", d->classno, "-10000.0", "maxvalue", "maxindex", ">");
 code_function_return_string(testcodefile, "rbf", language, "maxindex");   
 code_function_end(testcodefile, language);
}

void generate_test_code_rbfreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Rbfnetworkptr m)
{
 /*!Last Changed 21.08.2007*/
 char st[MAXLENGTH];
 code_function_header(testcodefile, "rbfreg", d->featurecount, language);
 code_constant_start(testcodefile, language);
 code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
 code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_2d_array_constant(testcodefile, language, "m", m->weights.m.row, m->weights.m.col, m->weights.m.values);
 code_1d_array_constant(testcodefile, language, "w", m->weights.w.col, m->weights.w.values[0], REAL_TYPE);
 code_1d_array_constant(testcodefile, language, "s", m->weights.s.col, m->weights.s.values[0], REAL_TYPE);
 code_variable_start(testcodefile, language);
 code_add_array_variable(testcodefile, language, 2, REAL_VARIABLE, "p", m->hidden + 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_add_variable(testcodefile, language, REAL_VARIABLE, 9, "y", "p_h", "s_h", "dist", "sum", "w_h", "m_hj", "x_j", "maxvalue");
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 6, "i", "j", "h", "maxindex", "k", "a");
 code_start(testcodefile, language);
 code_realize(testcodefile, language, d, YES);
 code_array_index_variable(st, language, "p", "0");
 code_set_variable(testcodefile, language, st, "1.0");
 code_for(testcodefile, language, "h", 1, m->hidden, 1);
 code_set_variable(testcodefile, language, "dist", "0.0");
 code_for(testcodefile, language, "j", 0, d->multifeaturecount - 2, 1);
 code_2d_array_index_variable(st, language, "m", "h", "j");
 code_set_variable(testcodefile, language, "m_hj", st);
 code_array_index_variable(st, language, "testdataconverted", "j");  
 code_set_variable(testcodefile, language, "x_j", st); 
 code_set_variable(testcodefile, language, "dist", "dist + m_hj * x_j");    
 code_endfor(testcodefile, language, "j");
 code_array_index_variable(st, language, "s", "h");  
 code_set_variable(testcodefile, language, "s_h", st);
 code_array_index_variable(st, language, "p", "h");  
 code_set_variable(testcodefile, language, st, "exp(-dist / (s_h * s_h))");
 code_endfor(testcodefile, language, "h");
 code_set_variable(testcodefile, language, "y", "0.0");
 code_for(testcodefile, language, "h", 0, m->hidden, 1);
 code_array_index_variable(st, language, "w", "h");
 code_set_variable(testcodefile, language, "w_h", st);
 code_array_index_variable(st, language, "p", "h");  
 code_set_variable(testcodefile, language, "p_h", st);
 code_set_variable(testcodefile, language, "y", "y + w_h * p_h");    
 code_endfor(testcodefile, language, "h");
 code_function_return_string(testcodefile, "rbfreg", language, "y");   
 code_function_end(testcodefile, language);
}

void generate_test_code_mlpgeneric(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Mlpnetworkptr m)
{
 /*!Last Changed 16.06.2006*/
 int i;
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH];
 code_function_header(testcodefile, "mlpgeneric", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 for (i = 0; i <= m->P.layernum; i++)
  {
   sprintf(st, "weights%d", i);
   code_2d_array_constant(testcodefile, language, st, m->weights[i].row, m->weights[i].col, m->weights[i].values); 
  } 
 code_variable_start(testcodefile, language);
 for (i = 0; i < m->P.layernum; i++)
  {
   sprintf(st, "hidden%d", i);
   code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, st, m->weights[i].row);
  }
 code_add_array_variable(testcodefile, language, 2, REAL_VARIABLE, "output", d->classno, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_add_variable(testcodefile, language, REAL_VARIABLE, 5, "sum", "weights_ij", "x_j", "sigmoid", "maxvalue");
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 5, "i", "j", "maxindex", "k", "a");
 code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
 for (i = 0; i < m->P.layernum; i++)
  {
   sprintf(st, "weights%d", i);
   sprintf(st2, "hidden%d", i);
   sprintf(st3, "hidden%d", i - 1);
   if (i == 0)
     code_forward_propagation(testcodefile, language, m->weights[i].row, m->weights[i].col, st, "testdataconverted", st2, YES);
   else
     code_forward_propagation(testcodefile, language, m->weights[i].row, m->weights[i].col, st, st3, st2, YES);
  }
 if (m->P.layernum == 0)
   code_forward_propagation(testcodefile, language, m->weights[m->P.layernum].row, m->weights[m->P.layernum].col, "weights0", "testdataconverted", "output", NO);
 else
  {
   sprintf(st, "weights%d", m->P.layernum);
   sprintf(st2, "hidden%d", m->P.layernum - 1);
   code_forward_propagation(testcodefile, language, m->weights[m->P.layernum].row, m->weights[m->P.layernum].col, st, st2, "output", NO);
  }
 code_find_bound_of_list(testcodefile, language, "output", d->classno, "-10000.0", "maxvalue", "maxindex", ">");
 code_function_return_string(testcodefile, "logistic", language, "maxindex");   
 code_function_end(testcodefile, language);
}

void generate_test_code_mlpgenericreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Mlpnetworkptr m)
{
 /*!Last Changed 24.06.2006*/
 int i;
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH];
 code_function_header(testcodefile, "mlpgenericreg", d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 for (i = 0; i < m->P.layernum; i++)
  {
   sprintf(st, "weights%d", i);
   code_2d_array_constant(testcodefile, language, st, m->weights[i].row, m->weights[i].col, m->weights[i].values); 
  } 
 sprintf(st, "weights%d", m->P.layernum);
 code_1d_array_constant(testcodefile, language, st, m->weights[i].col, m->weights[i].values[0], REAL_TYPE); 
 code_variable_start(testcodefile, language);
 for (i = 0; i < m->P.layernum; i++)
  {
   sprintf(st, "hidden%d", i);
   code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, st, m->weights[i].row);
  }
 code_add_variable(testcodefile, language, REAL_VARIABLE, 6, "sum", "weights_ij", "weights_j", "x_j", "sigmoid", "output");
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 4, "i", "j", "k", "a");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
 for (i = 0; i < m->P.layernum; i++)
  {
   sprintf(st, "weights%d", i);
   sprintf(st2, "hidden%d", i);
   sprintf(st3, "hidden%d", i - 1);
   if (i == 0)
     code_forward_propagation(testcodefile, language, m->weights[i].row, m->weights[i].col, st, "testdataconverted", st2, YES);
   else
     code_forward_propagation(testcodefile, language, m->weights[i].row, m->weights[i].col, st, st3, st2, YES);
  }
 if (m->P.layernum == 0)
   code_forward_propagation_regression_output(testcodefile, language, m->weights[m->P.layernum].col, "weights0", "testdataconverted", "output");
 else
  {
   sprintf(st, "weights%d", m->P.layernum);
   sprintf(st2, "hidden%d", m->P.layernum - 1);
   code_forward_propagation_regression_output(testcodefile, language, m->weights[m->P.layernum].col, st, st2, "output");
  }
 code_function_return_string(testcodefile, "mlpgenericreg", language, "output");   
 code_function_end(testcodefile, language);
}

void generate_test_code_multildt(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, Decisionnodeptr m, char* algorithmname)
{
 /*!Last Changed 24.06.2006*/
 code_function_header(testcodefile, algorithmname, d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
	code_variable_start(testcodefile, language);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 4, "i", "j", "k", "a");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);  
	code_realize(testcodefile, language, d, YES);
 code_decisionnode(testcodefile, language, algorithmname, "testdataconverted", d, m, 0);
 code_function_end(testcodefile, language);
}

void generate_test_code_svm(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithmname, Svm_modelptr m)
{
 /*!Last Changed 25.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH], st3[MAXLENGTH];
 code_function_header(testcodefile, algorithmname, d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_constant_real(testcodefile, language, "gamma", m->param.gamma);
 code_constant_real(testcodefile, language, "coef0", m->param.coef0);
 code_constant_integer(testcodefile, language, "degree", m->param.degree);
 code_2d_array_constant_sv(testcodefile, language, "sv", m->l, d->multifeaturecount - 1, m->SV);
	code_2d_array_constant(testcodefile, language, "sv_coef", d->classno, m->l, m->sv_coef);
 code_1d_array_constant(testcodefile, language, "nsv", d->classno, m->nSV, INTEGER_TYPE); 
 code_1d_array_constant(testcodefile, language, "rho", (int)(d->classno * (d->classno - 1) / 2), m->rho, REAL_TYPE); 
 code_variable_start(testcodefile, language); 
 code_add_array_variable(testcodefile, language, 5, REAL_VARIABLE, "kvalue", m->l, INTEGER_VARIABLE, "start", d->classno, REAL_VARIABLE, "dec_values", (int)(d->classno * (d->classno - 1) / 2), INTEGER_VARIABLE, "vote", d->classno, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 11, "i", "j", "k", "p", "si", "sj", "ci", "cj", "maxcount", "maxindex", "a");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 7, "x_j", "sv_ij", "sum", "coef1", "kvalue_k", "coef2", "rho_p");
 code_start(testcodefile, language);
	code_realize(testcodefile, language, d, YES);
	code_for(testcodefile, language, "i", 0, m->l - 1, 1);
 code_array_index_variable(st, language, "kvalue", "i");  
	code_kernel_function(testcodefile, language, d, m->param);
	code_set_variable(testcodefile, language, st, "sum");
	code_endfor(testcodefile, language, "i");
 code_array_index_variable(st, language, "start", "0");  
	code_set_variable(testcodefile, language, st, "0");
	code_for(testcodefile, language, "i", 1, d->classno - 1, 1);
 code_array_index_variable(st, language, "start", "i");  
 code_array_index_variable(st2, language, "nsv", "i - 1");  
 code_array_index_variable(st3, language, "start", "i - 1");
	sprintf(st2, "%s + %s", st2, st3);
	code_set_variable(testcodefile, language, st, st2);
	code_endfor(testcodefile, language, "i");
	code_set_variable(testcodefile, language, "p", "0");
	code_for(testcodefile, language, "i", 0, d->classno - 1, 1);
	code_for(testcodefile, language, "j", 0, d->classno - 1, 1);
	code_if(testcodefile, language, ">", "j", "i");
	code_set_variable(testcodefile, language, "sum", "0.0");
 code_array_index_variable(st, language, "start", "i");  
	code_set_variable(testcodefile, language, "si", st);
 code_array_index_variable(st, language, "start", "j");  
	code_set_variable(testcodefile, language, "sj", st);
 code_array_index_variable(st, language, "nsv", "i");  
	code_set_variable(testcodefile, language, "ci", st);
 code_array_index_variable(st, language, "nsv", "j");  
	code_set_variable(testcodefile, language, "cj", st);
 code_for_with_string(testcodefile, language, "k", "0", "ci - 1", "1");
 code_2d_array_index_variable(st, language, "sv_coef", "j - 1", "si + k");
	code_set_variable(testcodefile, language, "coef1", st);
 code_array_index_variable(st, language, "kvalue", "si + k");  
	code_set_variable(testcodefile, language, "kvalue_k", st);
	code_set_variable(testcodefile, language, "sum", "sum + coef1 * kvalue_k");
	code_endfor(testcodefile, language, "k");
 code_for_with_string(testcodefile, language, "k", "0", "cj - 1", "1");
 code_2d_array_index_variable(st, language, "sv_coef", "i", "sj + k");
	code_set_variable(testcodefile, language, "coef2", st);
 code_array_index_variable(st, language, "kvalue", "sj + k");  
	code_set_variable(testcodefile, language, "kvalue_k", st);
	code_set_variable(testcodefile, language, "sum", "sum + coef2 * kvalue_k");
	code_endfor(testcodefile, language, "k");
 code_array_index_variable(st, language, "rho", "p");  
	code_set_variable(testcodefile, language, "rho_p", st);
	code_set_variable(testcodefile, language, "sum", "sum - rho_p");
 code_array_index_variable(st, language, "dec_values", "p");  
	code_set_variable(testcodefile, language, st, "sum");
	code_set_variable(testcodefile, language, "p", "p + 1");
	code_endif(testcodefile, language);
	code_endfor(testcodefile, language, "j");
	code_endfor(testcodefile, language, "i");
	code_array_initialize(testcodefile, language, "vote", (int)(d->classno * (d->classno - 1) / 2), "0");
	code_set_variable(testcodefile, language, "p", "0");
	code_for(testcodefile, language, "i", 0, d->classno - 1, 1);
	code_for(testcodefile, language, "j", 0, d->classno - 1, 1);
	code_if(testcodefile, language, ">", "j", "i");
 code_array_index_variable(st, language, "dec_values", "p");  
	code_if(testcodefile, language, ">", st, "0.0");
	code_increment(testcodefile, language, "vote", "i");
	code_else(testcodefile, language);
	code_increment(testcodefile, language, "vote", "j");
	code_endif(testcodefile, language);
	code_set_variable(testcodefile, language, "p", "p + 1");
	code_endif(testcodefile, language);
	code_endfor(testcodefile, language, "j");
	code_endfor(testcodefile, language, "i");
 code_find_bound_of_list(testcodefile, language, "vote", d->classno, "-1", "maxcount", "maxindex", ">");
 code_function_return_string(testcodefile, algorithmname, language, "maxindex");   
 code_function_end(testcodefile, language);
}

void generate_test_code_svmreg(FILE* testcodefile, PROGRAMMING_LANGUAGE_TYPE language, Datasetptr d, char* algorithmname, Svm_modelptr m)
{
 /*!Last Changed 25.06.2006*/
 char st[MAXLENGTH], st2[MAXLENGTH];
 code_function_header(testcodefile, algorithmname, d->featurecount, language);
 code_constant_start(testcodefile, language);
	code_1d_array_constant_instance(testcodefile, language, d, "mean", d->multifeaturecount - 1, &mean);
	code_1d_array_constant_instance(testcodefile, language, d, "variance", d->multifeaturecount - 1, &variance);
 code_constant_real(testcodefile, language, "gamma", m->param.gamma);
 code_constant_real(testcodefile, language, "coef0", m->param.coef0);
 code_constant_real(testcodefile, language, "rho", m->rho[0]);
 code_constant_integer(testcodefile, language, "degree", m->param.degree);
 code_2d_array_constant_sv(testcodefile, language, "sv", m->l, d->multifeaturecount - 1, m->SV);
	code_1d_array_constant(testcodefile, language, "sv_coef", m->l, m->sv_coef, REAL_TYPE);
 code_variable_start(testcodefile, language); 
 code_add_variable(testcodefile, language, INTEGER_VARIABLE, 4, "i", "j", "k", "a");
 code_add_variable(testcodefile, language, REAL_VARIABLE, 4, "x_j", "sv_ij", "sum", "regvalue");
 code_add_array_variable(testcodefile, language, 1, REAL_VARIABLE, "testdataconverted", d->multifeaturecount);
 code_start(testcodefile, language);  
	code_realize(testcodefile, language, d, YES);
	code_set_variable(testcodefile, language, "regvalue", "0.0");
	code_for(testcodefile, language, "i", 0, m->l - 1, 1);
	code_kernel_function(testcodefile, language, d, m->param);
 code_array_index_variable(st, language, "sv_coef", "i");  
	sprintf(st2, "regvalue + sum * %s", st);
	code_set_variable(testcodefile, language, "regvalue", st2);
	code_endfor(testcodefile, language, "i");
	code_set_variable(testcodefile, language, "regvalue", "regvalue - rho");
 code_function_return_string(testcodefile, algorithmname, language, "regvalue");   
 code_function_end(testcodefile, language);
}
