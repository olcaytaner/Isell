#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "command.h"
#include "datastructure.h"
#include "lang.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"

char *operandtype[8] = {"<","<=",">",">=","=","==","!=","<>"};
char *possibleparametertypes[17] = {"stringval","realval","integerval",
                                    "stringvaln","integervaln","realvaln",
																																				"stringvaln2","integervaln2","realvaln2",
																																				"stringvar","realvar","integervar","filevar",
																																				"compoperator","integerrealvar","anyvar","matrixvar"};
char *doublevalues[3] = {"onoff","+-","xy"}; 
char separators[5] = {'(','[',',',')',']'};
char operators[7] = {'+','-','*','/','^','$','@'};
char *default_variables[DEFAULT_VARIABLE_COUNT]={"out1","out2","out3","out4","out5","out6","out7","out8","out9","out10"};
char *default_string_variables[DEFAULT_STRING_VARIABLE_COUNT]={"sout1","sout2","sout3","sout4","sout5","sout6","sout7","sout8","sout9","sout10"};
char *default_array_variables[DEFAULT_ARRAY_VARIABLE_COUNT]={"aout1[512]","aout2[512]","aout3[512]","aout4[512]","aout5[512]","aout6[512]","aout7[512]","aout8[512]","aout9[512]","aout10[512]","aout11[512]","aout12[512]","aout13[512]","aout14[512]","aout15[512]","aout16[512]","aout17[512]","aout18[512]","aout19[512]","aout20[512]","aout21[512]","aout22[512]","aout23[512]","aout24[512]","aout25[512]","aout26[512]","aout27[512]","aout28[512]","aout29[512]","aout30[512]"};
char *default_performance_variables[DEFAULT_PERFORMANCE_VARIABLE_COUNT]={"pout1[512]","pout2[512]"};

extern variableptr vars;
extern int varcount;
extern int precision;

/**
 * Find the correct place of the ending line of the corresponding if, while, for, etc constructor. For example if the constructor is an if statement, this code finds the position of the endif statement corresponding to this if statement
 * @param[out] variable Variable included in the constructor
 * @param[in] constructorname Name of the constructor if, while, for etc.
 * @param[in] endconstructorname Name of the end of the constructor such as endif, endfor, endwhile etc.
 * @param[in] code Source code as a two dimensional character array. code[i] stores the line i of the code
 * @param[in] linecount Number of lines in the source code (Size of the code array)
 * @param[out] instruction_pointer Position of the corresponding end constructor
 */
void goto_endconstructor(int* variable, char* constructorname, char* endconstructorname, char** code, int linecount, int* instruction_pointer)
{
 /*!Last Changed 14.07.2006*/
 char templine[READLINELENGTH], *p;
 int howmany = 1, i;
 *variable = 0;
 for (i = *instruction_pointer + 1; i < linecount; i++)
  {
   strcpy(templine, code[i]);
   p = strtok(templine," \t\n");
   if (strcmp(p, constructorname) == 0)
     howmany++;
   else
     if (strcmp(p, endconstructorname) == 0)
      {
       howmany--;
       if (howmany == 0)
         break;
      }
  }
 *instruction_pointer = i;
}

/**
 * Sets an element's value of one or more aout array(s) in the program. For example this function is used to set aout2[3], aout3[3], aout4[3], aout5[3], etc.
 * @param[in] index Index of the element. 3 in the example above.
 * @param[in] start Starting index of the aout arrays. 2 in the example above.
 * @param[in] vartypes An array of types. Stored as a string. vartypes[i] is the type of the variable i. 'd' stands for integer type, 'f' stand for float type and 's' stands for string type. 
 */
void write_array_variables(char* arrayname, int index, int start, char* vartypes, ...)
{
 int i, length, value_integer;
	double value_double;
	char* value_string;
 va_list ap;
 char varname[MAXLENGTH];
 char varvalue[MAXLENGTH];
 va_start(ap, vartypes);
	length = strlen(vartypes);
	for (i = 0; i < length; i++)
	 {
   sprintf(varname, "%s%d[%d]", arrayname, i + start, index);
		 switch (vartypes[i])
    {
		   case 'd':value_integer = va_arg(ap, int);
	             sprintf(varvalue, "%d", value_integer);
						        break;
					case 'f':value_double = va_arg(ap, double);
	             sprintf(varvalue, "%f", value_double);
														break;
					case 's':value_string = va_arg(ap, char*);
						        strcpy(varvalue, value_string);
						        break;
     default :printf(m1243, vartypes[i]);
              break;
	   }
   set_variable(varname, varvalue);
	 }
}

/**
 * Checks if the type of the variable matches the given type
 * @param[in] varname String containing the variable name.
 * @param[in] vartype Type to be matched
 * @param[in] variables Array of variables (symbol table). variables[i] is the i'th variable. variables also contains the predefined variables such as out1, out2, sout1, aout1, etc.
 * @param[in] variablecount Number of variables defined in the program currently (Size of the variables array).
 * @return SUCCESS if the type of the variable matches the given type, FAILURE otherwise.
 */
int check_parameter_type(char* varname, VARIABLE_TYPE vartype, variableptr variables, int variablecount)
{
 /*!Last Changed 21.12.2006*/
 int k;
 Variable myvar;
 k = return_variable_index(varname, variables, variablecount);
 if (k == -1)
   return FAILURE;
 else
  {
   myvar = variables[k];
   while (myvar.type == ARRAY_TYPE) 
     myvar = myvar.value.array[0];
   if (myvar.type != vartype)
     return FAILURE;
  }
 return SUCCESS; 
}

/**
 * Check if the types of the parameters in a program line matches the parameters of the command in that line.
 * @param[in] params[] Parameters in the program line. params[i] is the i'th parameter stored as a string.
 * @param[in] paramcount Number of parameters in the program line (Size of the params array).
 * @param[in] grammar[] The grammar of the command in the program line. Each grammar is an array of strings containing values from possibleparametertypes array. For example, for constructor has three parameters (for i 1 10) integervar realval and realval.
 * @param[in] grammarcount Number of must be parameters of the command in the program line (Size of the grammar array).
 * @param[in] variables Array of variables (symbol table). variables[i] is the i'th variable. variables also contains the predefined variables such as out1, out2, sout1, aout1, etc.
 * @param[in] variablecount Number of variables defined in the program currently (Size of the variables array).
 * @return If one of the type of the parameters does not match the parameters of the command, the function returns the index of that parameter. If all parameter types match, function returns -1.
 */
int check_parameters(char *params[MAXPARAMS], int paramcount, char* grammar[MAXPARAMS], int grammarcount, variableptr variables, int variablecount)
{
 /*!Last Changed 21.12.2006 added check_parameter_type and matrix type*/
	/*!Last Changed 07.05.2005*/
	/*!Last Changed 12.10.2004*/
	int i = 0, res, j, k, res2;
	while (i < paramcount && i < grammarcount)
	 {
		 if (char_count(params[i], '%') == 0)
			 {
	  	 res = listindex(grammar[i], possibleparametertypes, 17);
		  	switch (res)
					{
					 case -1:res2 = listindex(grammar[i], doublevalues, 3);
							       switch (res2)
														 {
														  case 0:if (strcmp(params[i], "on") != 0 && strcmp(params[i], "off") != 0)
														 									 return i;
														 								break;
														 	case 1:if (strcmp(params[i], "+") != 0 && strcmp(params[i], "-") != 0)
														 									 return i;
														 		      break;
														 	case 2:if (strcmp(params[i], "x") != 0 && strcmp(params[i], "y") != 0)
														 									 return i;
														 		      break;
														 }
							       break;
				  case  1:if (!isfloat(params[i]))
															 return i;
							       break;
			   case  2:if (!isinteger(params[i]))
															 return i;
														break;
		 	  case  4:for (j = i; j < paramcount; j++)
															 if (!isinteger(params[j]) && char_count(params[j], '%') == 0)
																	 return j;
														return -1;
							       break;
		 			case  5:for (j = i; j < paramcount; j++)
															 if (!isfloat(params[j]) && char_count(params[j], '%') == 0)
																	 return j;
														return -1;
														break;
		 			case  7:for (j = i; j < paramcount; j++)
															 if (!isinteger(params[j]) && char_count(params[j], '%') == 0)
																	 return j;
														return -1;
														break;
		 	  case  8:for (j = i; j < paramcount; j++)
															 if (!isfloat(params[j]) && char_count(params[j], '%') == 0)
																	 return j;
														return -1;
														break;
		 			case  9:if (!check_parameter_type(params[i], STRING_TYPE, variables, variablecount)) 
                return i;
														break;
		 	  case 10:if (!check_parameter_type(params[i], REAL_TYPE, variables, variablecount)) 
                return i;
              break;
		 			case 11:if (!check_parameter_type(params[i], INTEGER_TYPE, variables, variablecount)) 
                return i;
              break;
		 	  case 12:if (!check_parameter_type(params[i], FILE_TYPE, variables, variablecount)) 
                return i;
              break;
		 			case 13:if (listindex(params[i], operandtype, 8) == -1)
															 return i;
														break;
						case 14:if (!check_parameter_type(params[i], INTEGER_TYPE, variables, variablecount) && !check_parameter_type(params[i], REAL_TYPE, variables, variablecount) && !check_parameter_type(params[i], MATRIX_TYPE, variables, variablecount)) 
                return i;
              break;
						case 15:k = return_variable_index(params[i], variables, variablecount);
							       if (k == -1)
															 return i;
														break;
      case 16:if (!check_parameter_type(params[i], MATRIX_TYPE, variables, variablecount)) 
                return i;
              break;
		 	 }
			 }
			i++;
	 }
	return -1;
}

/**
 * Add default variables to the variables array. These are real type default variables out1, out2, etc; string type default variables sout1, sout2, etc; and array type default variables aout1, aout2, etc.
 * @param[out] variables Array of variables (symbol table). variables[i] is the i'th variable. The default variables will be added to this array.
 * @param[out] variablecount Number of variables defined in the program currently (Size of the variables array).
 */
void add_default_variables(variableptr* variables, int* variablecount)
{
	/*!Last Changed 06.08.2007*/
 add_variable(REAL_TYPE, default_variables, DEFAULT_VARIABLE_COUNT, variables, variablecount);
 add_variable(STRING_TYPE, default_string_variables, DEFAULT_STRING_VARIABLE_COUNT, variables, variablecount);
 add_variable(STRING_TYPE, default_array_variables, DEFAULT_ARRAY_VARIABLE_COUNT, variables, variablecount);
 add_variable(STRING_TYPE, default_performance_variables, DEFAULT_PERFORMANCE_VARIABLE_COUNT, variables, variablecount);
}

/**
 * Compiles a given file. Checks if there are any syntax errors in the given script program. Syntax errors will be printed to the screen.
 * @param[in] filename Name of the file to be compiled
 * @return SUCCESS if the program is successfully compiled, FAILURE otherwise.
 */
int compile_file(char *filename)
{
 /*!Last Changed 26.01.2009 added matrix type*/
	/*!Last Changed 12.10.2004*/
 FILE *myfile;
 char myline[READLINELENGTH], *params[MAXPARAMS]={NULL}, *grammarparams[MAXPARAMS]={NULL}, *tempparams[MAXPARAMS], *temporder, *order = NULL;
 int i, paramcount = 0, linecount = 0, res, charcount, grammarcount = 0, result = SUCCESS;
	CONSTRUCT_TYPE counts[10] = {0};
 int variablecount = 0;
 variableptr variables = NULL;
 if ((myfile = fopen(filename, "r")) == NULL)
  {
   printf(m1274, filename);
   return FAILURE;
  }
	add_default_variables(&variables, &variablecount);
 while (fgets(myline, READLINELENGTH, myfile))
  {
		 linecount++;
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
   for (i = 0; i < grammarcount; i++)
    {
     safe_free(grammarparams[i]);
     grammarparams[i] = NULL;
    }
   temporder = strtok(myline," \t\n");
   if (temporder)
    {
     paramcount = -1;
     do{
       paramcount++;
       tempparams[paramcount] = strtok(NULL, " \t\n");
     }while((tempparams[paramcount]) && (paramcount < MAXPARAMS - 1));
    }
   else
     continue;
   order = strcopy(order, temporder);
			if (!check_of_variables(order, variables, variablecount))
			 {
				 printf(e1014, linecount, order);
					result = FAILURE;
			 }
   for (i = 0; i < paramcount; i++)
     params[i] = strcopy(params[i], tempparams[i]);
   if (order[0] != '\'' && char_count(order, '%') == 0)
    {
     res = get_order_index(order);
     if (res == -1)
					 {
						 printf(e1001, linecount, order);
							result = FAILURE;
						 continue;
					 }
					charcount = char_count(grammar_of_order(res), '<');
					if (charcount > paramcount)
					 {
						 printf(e1002, linecount, order);
							result = FAILURE;
							continue;
					 }
					else
						 if (res != 70 && res != 71 && res != 72 && res != 152 && res != 309)
  						 if (charcount < paramcount)
	  					   printf(w1001, linecount, order);
					switch (res)
					 {
					  case 97 :counts[IF]++;
								        break;
							case 98 :counts[ELSE]++;
								        if (counts[ELSE] > counts[IF])
																 {
																	 printf(e1003, linecount);
																		result = FAILURE;
																	 continue;
																 }
								        break;
							case 99 :counts[ENDIF]++;
								        if (counts[ENDIF] > counts[IF])
																 {
																	 printf(e1004, linecount);
																		result = FAILURE;
																	 continue;
																 }
								        break;
							case 76 :counts[SWITCH]++;
								        if (counts[SWITCH] - counts[ENDSWITCH] > 1)
																 {
																	 printf(e1005, linecount);
																		result = FAILURE;
																		continue;
																 }
								        break;
							case 77 :counts[ENDSWITCH]++;
								        if (counts[ENDSWITCH] > counts[SWITCH])
																 {
																	 printf(e1006, linecount);
																		result = FAILURE;
																		continue;
																 }
								        break;
							case 78 :counts[CASE]++;
								        if (counts[SWITCH] == counts[ENDSWITCH])
																 {
																	 printf(e1007, linecount);
																		result = FAILURE;
																		continue;
																 }
								        break;
							case 74 :counts[FOR]++;
								        break;
							case 75 :counts[ENDFOR]++;
								        if (counts[ENDFOR] > counts[FOR])
																 {
																	 printf(e1008, linecount);
																		result = FAILURE;
																		continue;
																 }
								        break;
							case 100:counts[WHILE]++;
								        break;
							case 101:counts[ENDWHILE]++;
								        if (counts[ENDWHILE] > counts[WHILE])
																 {
																	 printf(e1009, linecount);
																		result = FAILURE;
																		continue;
																 }
								        break;
						 case  70:for (i = 0; i < paramcount; i++)
																	 if (!add_variable(INTEGER_TYPE, &(params[i]), 1, &variables, &variablecount))
																			 printf(w1002, linecount, params[i]);
								        break;
							case  71:for (i = 0; i < paramcount; i++)
																	 if (!add_variable(REAL_TYPE, &(params[i]), 1, &variables, &variablecount))
																			 printf(w1002, linecount, params[i]);
																break;
							case  72:for (i = 0; i < paramcount; i++)
																	 if (!add_variable(STRING_TYPE, &(params[i]), 1, &variables, &variablecount))
																			 printf(w1002, linecount, params[i]);
								        break;
							case 152:for (i = 0; i < paramcount; i++)
																	 if (!add_variable(FILE_TYPE, &(params[i]), 1, &variables, &variablecount))
																			 printf(w1002, linecount, params[i]);
                break;
       case 309:for (i = 0; i < paramcount; i++)
                  if (!add_variable(MATRIX_TYPE, &(params[i]), 1, &variables, &variablecount))
                    printf(w1002, linecount, params[i]);
								        break;
					 }
					strcpy(myline, grammar_of_order(res));
     temporder = strtok(myline," <>\t\n");
     if (temporder)
					 {
       grammarcount = -1;
       do{
         grammarcount++;
         tempparams[grammarcount] = strtok(NULL, " <>\t\n");
							}while((tempparams[grammarcount]) && (grammarcount < MAXPARAMS - 1));
					 }
     for (i = 0; i < grammarcount; i++)
       grammarparams[i] = strcopy(grammarparams[i], tempparams[i]);
     res = check_parameters(params, paramcount, grammarparams, grammarcount, variables, variablecount);
					if (res != -1)
					 {
						 printf(e1016, linecount, params[res]);
							result = FAILURE;
					 }
    }
  }
 if (counts[IF] > counts[ENDIF])
	 {
		 printf(e1010, linecount);
			result = FAILURE;
	 }
 if (counts[SWITCH] > counts[ENDSWITCH])
	 {
		 printf(e1011, linecount);
			result = FAILURE;
	 }
 if (counts[FOR] > counts[ENDFOR])
	 {
		 printf(e1012, linecount);
			result = FAILURE;
	 }
 if (counts[WHILE] > counts[ENDWHILE])
	 {
		 printf(e1013, linecount);
			result = FAILURE;
	 }
	if (order)
		 safe_free(order);
 for (i = 0; i < paramcount; i++)
  {
   safe_free(params[i]);
   params[i] = NULL;
  }
 for (i = 0; i < grammarcount; i++)
  {
   safe_free(grammarparams[i]);
   grammarparams[i] = NULL;
  }
	free_variables(variables, variablecount);
	safe_free(variables);
	return result;
}

/**
 * Finds the character at a given index of a given string variable. Sets sout1 value to that character. If the variable is not of type string or the variable does not exist, the function does nothing.
 * @param[in] name Name of the string variable
 * @warning Strings in the ISELL script language starts from 1.
 * @param[in] index Index of the character
 */
void charatindex(char* name, int index)
{
	/*!Last Changed 27.01.2004*/
	char* st;
	char message[2];
	st = get_string_value(name);
	if (!st)
		 return;
	sprintf(message, "%c", st[index - 1]);
	set_default_string_variable("sout1", message);
	safe_free(st);
}

/**
 * Finds the length of the string type variable. Sets out1 value to the string length. If the variable is not of type string or the variable does not exist, the function does nothing.
 * @param[in] name Name of the string variable
 */
void stringlength(char* name)
{
	/*!Last Changed 27.01.2004*/
	char* st;
	st = get_string_value(name);
	if (!st)
		 return;
	set_default_variable("out1", strlen(st) + 0.0);
	safe_free(st);
}

/**
 * Finds how many times that character occured in a given string variable. Sets out1 value to that count. If the variable is not of type string or the variable does not exist, the function does nothing.
 * @param[in] name Name of the string variable
 * @param[in] ch ch[0] is the character that is being searched
 */
void countchar(char* name, char* ch)
{
 /*!Last Changed 27.01.2004*/
	int count = 0, i, len;
	char* st;
	st = get_string_value(name);
	if (!st)
		 return;
	len = strlen(st);
	for (i = 0; i < len; i++)
		 if (st[i] == ch[0])
				 count++;
	set_default_variable("out1", count + 0.0);
	safe_free(st);
}

/**
 * Separates the given string type variable to tokens. The 'tokens' are separated via a given separators. Sets out1 value to the number of tokens. Sets aout1[i] to the i'th token. If the variable is not of type string or the variable does not exist, the function does nothing.
 * @param[in] name Name of the string variable
 * @param[in] separators Arrays of separator characters. separators[i] is the i'th separator character.
 */
void tokenize(char* name, char* separators)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 27.01.2004*/
 int i, count;
	char message[READLINELENGTH];
 char* tokens[1000];
	char varname[15];
	variableptr myvar;
 myvar = get_and_check_variable_type(name, 2, "string");
	if (!myvar)
			return;
 if (myvar->value.stringvalue)
  	strcpy(message, myvar->value.stringvalue);
	else
		{
			printf(m1295, name);
			return;
		}
 stringlist(message, separators, tokens, &count);
 for (i = 0; i < count; i++)
  {
   sprintf(varname,"aout1[%d]", i + 1);
   set_variable(varname, tokens[i]);
   safe_free(tokens[i]);
  }
	set_default_variable("out1", count + 0.0);
}

/**
 * Reads one line from a file opened in the sript programs. If the file variable does not exists, the function does nothing.
 * @param[in] name Name of the file variable that was opened in the script
 * @param[in] varname Name of the string type variable in which the line read to be put
 */
void read_userfile(char* name, char* varname)
{
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 29.10.2005*/
 /*!Last Changed 21.10.2004*/
 /*!Last Changed 27.01.2004*/
	char message[READLINELENGTH];
	variableptr myvar;
 myvar = get_and_check_variable_type(name, 4, "file");
	if (!myvar)
			return;
	if (myvar->userfile)
		 fscanf(myvar->userfile, "%s", message);
	set_variable(varname, message);
}

/**
 * Reads an array from a file that was opened from a user script. If the file variable does not exists, the function does nothing.
 * @param[in] name Name of the file variable that was opened in the script.
 * @param[in] varname Name of the array type variable.
 * @param[in] size Number of values to be read (Size of the array).
 */
void read_array_from_userfile(char* name, char* varname, int size)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 15.12.2004*/
 VARIABLE_TYPE variabletype;
 int tmp, i;
	double tmpf;
	char message[READLINELENGTH];
	variableptr myvar, myvar2;
 myvar = get_and_check_variable_type(name, 4, "file");
	if (!myvar)
			return;
	myvar2 = get_variable(varname);
	if (myvar2->arraysize != 0)
			variabletype = myvar2->value.array[0].type;
	else
	  return;
	if (myvar->userfile)
			switch (variabletype)
			 {
				 case INTEGER_TYPE:for (i = 0; i < size; i++)
													           {
								                 fscanf(myvar->userfile, "%d", &tmp);
														           myvar2->value.array[i].value.intvalue = tmp;
													           }
								               break;
				 case REAL_TYPE   :for (i = 0; i < size; i++)
													           {
								                 fscanf(myvar->userfile, "%lf", &tmpf);
														           myvar2->value.array[i].value.floatvalue = tmpf;
													           }
								               break;
				 case STRING_TYPE :for (i = 0; i < size; i++)
													           {
								                 fscanf(myvar->userfile, "%s", message);
														           if (myvar2->value.array[i].value.stringvalue)
																           safe_free(myvar2->value.array[i].value.stringvalue);
                         myvar2->value.array[i].value.stringvalue = strcopy(myvar2->value.array[i].value.stringvalue, message);
													           }
								               break;
     default          :printf(m1245);
                       break;
			 }
}

/**
 * Writes one line to a file opened in the sript programs. If the file variable does not exists, the function does nothing.
 * @param[in] name Name of the file variable that was opened in the script.
 * @param[in] line Line to be written
 */
void write_userfile(char* name, char* line)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 05.11.2003*/
	variableptr myvar;
 myvar = get_and_check_variable_type(name, 4, "file");
	if (!myvar)
			return;
	if (myvar->userfile)
			fprintf(myvar->userfile, "%s\n", line);
}

/**
 * Opens a file in the script program.
 * @param[in] name Name of the file script variable whose associated file will be opened
 * @param[in] filemode The file opening mode. r for reading, w for writing, a for append mode.
 */
void open_userfile(char* name, char* filetype)
{
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 27.01.2004*/
 /*!Last Changed 10.11.2003*/
 variableptr myvar;
 myvar = get_and_check_variable_type(name, 4, "file");
	if (!myvar)
			return;
	if (!(myvar->userfile))
		{
			myvar->userfile = fopen(myvar->value.stringvalue, filetype);
			if (!(myvar->userfile))
					printf(m1296, myvar->value.stringvalue, filetype);
		}
}

/**
 * Opens a file that was opened in the script program.
 * @param[in] name Name of the file script variable whose associated file will be closed
 */
void close_userfile(char* name)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 05.11.2003*/
 variableptr myvar;
 myvar = get_and_check_variable_type(name, 4, "file");
	if (!myvar)
			return;
	if (myvar->userfile)
		{
			fclose(myvar->userfile);
			myvar->userfile = NULL;
		}
}

/**
 * Prints current script variables (its type and its value) to the screen (including predefined script variables and not including array script variables). For integer and real script variables the values are printed. For string and file script variables, if they are not associated with a string value, NULL is printed. For matrix script variables nothing is printed.
 */
void print_variables()
{
 /*!Last Changed 26.12.2006*/
 /*!Last Changed 05.11.2003*/
	char pst[READLINELENGTH];
	int loop;
 for (loop = 0; loop < varcount; loop++)
  {
   if (vars[loop].type == ARRAY_TYPE)
     continue;
   switch (vars[loop].type)
    {
     case INTEGER_TYPE:printf(m1297);
                       break;
     case REAL_TYPE   :printf(m1298);
                       break;
     case STRING_TYPE :printf(m1299);
                       break;
     case FILE_TYPE   :printf(m1300);
                       break;
     case MATRIX_TYPE :printf(m1362);
                       break;
     default          :break;
    }   
   printf(m1301, vars[loop].name);
   switch (vars[loop].type)
    {
     case INTEGER_TYPE:printf("%d\n",vars[loop].value.intvalue);
                       break;
     case REAL_TYPE   :set_precision(pst, NULL, "\n");
						                 printf(pst, vars[loop].value.floatvalue);
                       break;
     case STRING_TYPE :
					case FILE_TYPE   :if (vars[loop].value.stringvalue)
                         printf("%s\n", vars[loop].value.stringvalue);
                       else
                         printf("NULL\n");
                       break;
     case MATRIX_TYPE :if (vars[loop].value.matrixvalue)
                         ;
                       else
                         printf("NULL\n");
                       break;
     default          :break;
    }
  }
}

/**
 * Checks if the string contains an array script variable. An array script variable is something like array_name[array_indices].
 * @param[in] name String that may or may not contain the array script variable.
 * @return If the string contains an array script variable, it returns the index string (for the example above array_indices). If the string does not contain an array script variable, it returns NULL.
 */
char* array_variable(char* name)
{
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 02.05.2002*/
 char* st = NULL;
 int i, len;
	len = strlen(name);
 for (i = 0; i < len; i++)
   if (name[i] == '[')
    {
     st = safemalloc(sizeof(char) * (i + 1), "array_variable", 7);
     strncpy(st, name, i);
     st[i] = '\0';
     break;
    }
 return st;
}

/**
 * Checks if a variable is equal to a given value. 
 * @param[in] var Structure containing the script variable
 * @param[in] value The given value
 * @return If the variable contains an integer or a real value, if the value of the variable is equal to the given value, it returns BOOLEAN_TRUE, otherwise it returns BOOLEAN_FALSE. \n
 * If the variable contains a string or a file name, if the string value is equal to the given value, it returns BOOLEAN_TRUE, otherwise it returns BOOLEAN_FALSE.
 */
BOOLEAN check_var(struct variable var, char* value)
{
	/*!Last Changed 05.11.2003 Added file variable with type 4*/
 /*!Last Changed 02.05.2002*/
 int tmp;
 double tmpd;
 switch (var.type)
  {
   case INTEGER_TYPE:tmp = atoi(value);
                     if (var.value.intvalue == tmp)
                       return BOOLEAN_TRUE;
                     else
                       return BOOLEAN_FALSE;
   case REAL_TYPE   :tmpd = atof(value);
                     if (var.value.floatvalue == tmpd)
                       return BOOLEAN_TRUE;
                     else
                       return BOOLEAN_FALSE;
   case STRING_TYPE :
			case FILE_TYPE   :if (strcmp(var.value.stringvalue, value) == 0)
                       return BOOLEAN_TRUE;
                     else
                       return BOOLEAN_FALSE;
   default          :break;
  }
 return BOOLEAN_FALSE;
}

/**
 * Wrapper function for check_var. Calls check_var.
 * @param[in] name Name of the script variable.
 * @param[in] value The given value
 * @return If the variable contains an integer or a real value, if the value of the variable is equal to the given value, it returns BOOLEAN_TRUE, otherwise it returns BOOLEAN_FALSE. \n
 * If the variable contains a string or a file name, if the string value is equal to the given value, it returns BOOLEAN_TRUE, otherwise it returns BOOLEAN_FALSE.
 */
int check_variable(char* name, char* value)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 05.04.2002*/
	variableptr myvar;
	myvar = get_variable(name);
	return check_var(*myvar, value);
}

/**
 * Checks the type of a script variable with a given type.
 * @param[in] myvar Pointer to the script variable
 * @param[in] validtype Type to be compared
 * @return If the type of the script variable is equal to the valid type, it returns BOOLEAN_TRUE, otherwise it returns BOOLEAN_FALSE.
 */
BOOLEAN type_check(variableptr myvar, VARIABLE_TYPE validtype)
{
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 06.02.2004*/
	if (myvar->type != validtype)
			return BOOLEAN_FALSE;
	else
			return BOOLEAN_TRUE;
}

/**
 * Returns the value of a string variable with a given name.
 * @param[in] name Name of the script variable.
 * @return The value of the script variable, if the script variable is of string type. Otherwise, it returns NULL.
 */
char* get_string_value(char* name)
{
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 06.02.2004 put controls*/
 /*!Last Changed 27.01.2004*/
	char message[READLINELENGTH], *st = NULL;
	variableptr myvar;
 myvar = get_and_check_variable_type(name, 2, "string");
	if (!myvar)
			return NULL;
	if (myvar->value.stringvalue)
		 strcpy(message, myvar->value.stringvalue);
	else
		{
			printf(m1295, name);
			return NULL;
		}
	st = strcopy(st, message);
	return st;
}

/**
 * Returns the value of an integer or a real script variable.
 * @param[in] var Script variable structure
 * @return The value of the script variable, if the script variable is of integer or real type. Otherwise, it returns 0.
 * @warning For integer script variable, the function returns a double value.
 */
double get_value(struct variable var)
{
/*!Last Changed 02.05.2002*/
 switch (var.type)
  {
   case INTEGER_TYPE:return var.value.intvalue;
   case REAL_TYPE   :return var.value.floatvalue;
   default          :return 0;
  }
 return 0;
}

/**
 * Sets the value of an integer or a real script variable.
 * @param[in] var Script variable structure
 * @param[in] value1 Value to be set
 */
void set_value(variableptr var, double value1)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 02.05.2002*/
 switch (var->type)
  {
   case INTEGER_TYPE:var->value.intvalue = (int)value1;
                     break;
   case REAL_TYPE   :var->value.floatvalue = value1;
                     break;
   default          :break;
  }
}

/**
 * Performs an arithmetic operation on a script variable. If the variable is a matrix variable, only the operations '+', '-', '*', '/' and '^' applied. Division operation is performed only as scalar division.
 * @param[in] op Character representing the operation. '+' for adding, '-' for subtracting, '*' for multiplying, '/' for dividing, '^' for taking power, '$' for taking logarithm, '@' for modulus operation.
 * @param[in] name Name of the script variable on which one performs the operation.
 * @param[in] value Value to be added or subtracted to or multiplied with or divided to or taken power or taking modulus.
 * @return BOOLEAN_TRUE if the operation is succesfully applied.
 */
BOOLEAN make_operation(char op, char* name, char* value)
{
	/*!Last Changed 23.12.2006 added matrix operations*/
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 02.05.2002*/
 double value1, value2;
	variableptr myvar, myvar2;
	myvar = get_variable(name);
	if (myvar->type == MATRIX_TYPE)
	 {
		 myvar2 = get_variable(value);
			switch (op)
			 {
			  case '+':if (myvar->value.matrixvalue)
                matrix_add(myvar->value.matrixvalue, *(myvar2->value.matrixvalue));
              else
               {
                myvar->value.matrixvalue = safemalloc(sizeof(matrix), "make_operation", 14);
                *(myvar->value.matrixvalue) = matrix_copy(*(myvar2->value.matrixvalue));
               }
						        break;
					case '-':if (myvar->value.matrixvalue)
                matrix_subtract(myvar->value.matrixvalue, *(myvar2->value.matrixvalue));
              else
               {
                myvar->value.matrixvalue = safemalloc(sizeof(matrix), "make_operation", 22);
                *(myvar->value.matrixvalue) = matrix_copy(*(myvar2->value.matrixvalue));
               }
						        break;
					case '*':if (myvar2 && myvar2->type == MATRIX_TYPE)
  						        matrix_product(myvar->value.matrixvalue, *(myvar2->value.matrixvalue));
						        else
															 matrix_multiply_constant(myvar->value.matrixvalue, atof(value));
						        break;
					case '/':if (myvar2 && myvar2->type == MATRIX_TYPE)
															 ;
						        else
															 matrix_divide_constant(myvar->value.matrixvalue, atof(value));
														break;
					case '^':if (myvar->value.matrixvalue)
                matrix_columnwise_merge(myvar->value.matrixvalue, *(myvar2->value.matrixvalue));
              else
               {
                myvar->value.matrixvalue = safemalloc(sizeof(matrix), "make_operation", 28);
                *(myvar->value.matrixvalue) = matrix_copy(*(myvar2->value.matrixvalue));
               }
						        break;
			 }
	 }
	else
	 {
   value1 = get_value(*myvar);
   value2 = atof(value);
   switch (op)
			 {
     case '+':value1 += value2;
              break;
     case '-':value1 -= value2;
              break;
     case '*':value1 *= value2;
              break;
     case '/':value1 /= value2;
              break;
     case '^':value1 = pow(value1, value2);
              break;
     case '$':value1 = log(value1)/log(value2);
              break;
     case '@':value1 = value1 - value2 * ((int)(value1 / value2));
              break;
			 }
	  set_value(myvar, value1);
	 }
 return BOOLEAN_TRUE;
}

/**
 * Finds the size of each dimension of a multi-dimensional array from its definition string. The elements of a multi-dimensional array are stored in ISELL as array or array of array of ... elements.
 * @param[in] name Name of the multi-dimensional array that also contains the dimensions.
 * @param[out] dimcount Number of dimensions
 * @return An integer array containing the dimensions of the multi-dimensional array. result[i] is the size of the i'th dimension of the array.
 */
int* array_sizes(char* name, int* dimcount)
{
 /*!Last Changed 06.01.2006*/
 int i, start, end = 0, len, *dimensions = NULL;
 char st[READLINELENGTH], *p;
 len = strlen(name);
 for (i = 0; i < len; i++)
   if (name[i] == '[')
     start = i + 1;
   else
     if (name[i] == ']')
      {
       end = i - 1;
       break;
      }
 for (i = start; i <= end; i++)
   st[i - start] = name[i];
 st[i - start] = '\0';
 p = strtok(st, ",");
 *dimcount = 0;
 while (p)
  {
   (*dimcount)++;
   dimensions = alloc(dimensions, (*dimcount) * sizeof(int), *dimcount);
   dimensions[(*dimcount) - 1] = atoi(p);
   p = strtok(NULL, ",");
  }
 return dimensions;
}

/**
 * Finds the position (in the memory) of an element of a multi-dimensional array. The elements of a multi-dimensional array are stored in ISELL as array or array of array of ... elements.
 * @param[in] name Name of the multi-dimensional array that also contains the indices of each dimension separated by commas.
 * @param[in] myvar Pointer to the multi-dimensional array variable.
 * @return Pointer to the element of the multi-dimensional array.
 */
variableptr array_pos(char *name, variableptr myvar)
{
 /*!Last Changed 06.01.2006*/
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 02.05.2002*/
 int i, start, end = 0, len;
 char st[READLINELENGTH], *p;
 variableptr myvar2, result = myvar;
 Expnodeptr list;
 len = strlen(name);
 for (i = 0; i < len; i++)
   if (name[i] == '[')
     start = i + 1;
   else
     if (name[i] == ']')
      {
       end = i - 1;
       break;
      }
 for (i = start; i <= end; i++)
   st[i - start] = name[i];
 st[i - start] = '\0';
 p = strtok(st, ",");
 while (p)
  {
   list = create_expression_list(p);
   myvar2 = evaluate_expression_list(list);
   result = &(result->value.array[myvar2->value.intvalue - 1]);
   safe_free(myvar2);
   p = strtok(NULL, ",");
  }
 return result;
}

/**
 * Finds the index of the script variable in the symbol table.
 * @param[in] name Name of the script variable.
 * @param[in] vars Symbol table
 * @param[in] varcount Number of script variables in the symbol table (size of the vars array).
 * @return If a variable exists with the given name in the symbol table, the function returns the index of that script variable. Otherwise, it returns -1.
 */
int return_variable_index(char* name, variableptr vars, int varcount)
{
 /*!Last Changed 02.05.2002*/
 int i;
 char* st = array_variable(name);
 if (st)
  {
   for (i = 0; i < varcount; i++)
     if (strcmp(st, vars[i].name) == 0)
      {
       safe_free(st);
       return i;
      }
  }
 else
  {
   for (i = 0; i < varcount; i++)
     if (strcmp(name, vars[i].name)==0)
       return i;
  }
	if (st)
		 safe_free(st);
 return -1;
}

void set_default_value(variableptr var)
{
 /*!Last Changed 22.12.2006 added matrix type*/
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 05.11.2003 Added File variable with type 4*/
 /*!Last Changed 02.05.2002*/
 switch (var->type)
  {
   case INTEGER_TYPE:var->value.intvalue = 0;
                     break;
   case REAL_TYPE   :var->value.floatvalue = 0;
                     break;
   case FILE_TYPE   :var->userfile = NULL;
			case STRING_TYPE :var->value.stringvalue = NULL;
                     break;
   case MATRIX_TYPE :var->value.matrixvalue = NULL;
                     break;
   default          :break;
  }
}

void set_multidimensional_array(variableptr myvar, void* value)
{
 /*!Last Changed 22.12.2006 added matrix type*/
 /*!Last Changed 05.01.2006*/
 int j;
 for (j = 0; j < myvar->arraysize; j++)
   switch (myvar->value.array[j].type)
    {
     case INTEGER_TYPE:myvar->value.array[j].value.intvalue = atoi((char *)value);
                       break;
     case REAL_TYPE   :myvar->value.array[j].value.floatvalue = atof((char *)value);
                       break;
     case ARRAY_TYPE  :set_multidimensional_array(&(myvar->value.array[j]), value);
                       break;
     case STRING_TYPE :
     case FILE_TYPE   :if (myvar->value.array[j].value.stringvalue)
                         safe_free(myvar->value.array[j].value.stringvalue);
                       myvar->value.array[j].value.stringvalue = strcopy(myvar->value.array[j].value.stringvalue, (char *)value);
                       break;
     case MATRIX_TYPE :myvar->value.array[j].value.matrixvalue = (matrixptr) value;
                       break;
     default          :break;
    }
}

void free_multidimensional_array(variableptr myvar)
{
 /*!Last Changed 22.12.2006 added array of matrices*/
 /*!Last Changed 04.01.2006*/
 int j;
 for (j = 0; j < myvar->arraysize; j++)
   if (myvar->value.array[j].type == ARRAY_TYPE)
     free_multidimensional_array(&(myvar->value.array[j]));
   else
    {
     if (in_list(myvar->value.array[j].type, 2, STRING_TYPE, FILE_TYPE))
      {
       if (myvar->value.array[j].value.stringvalue)
         safe_free(myvar->value.array[j].value.stringvalue);
      }
     else
       if (myvar->value.array[j].type == MATRIX_TYPE && myvar->value.array[j].value.matrixvalue)
        {
         matrix_free(*(myvar->value.array[j].value.matrixvalue));
         safe_free(myvar->value.array[j].value.matrixvalue);
        }
    }
 safe_free(myvar->value.array);
}

void free_variables(variableptr vars, int count)
{
 /*!Last Changed 22.12.2006 added matrix type*/
 /*!Last Changed 04.01.2006 added free_multidimensional_array*/
	/*!Last Changed 29.11.2004*/
	int i;
 for (i = 0; i < count; i++)
	 {
		 safe_free(vars[i].name);
			switch (vars[i].type)
			 {
			  case INTEGER_TYPE:
					case REAL_TYPE   :break;
					case STRING_TYPE :
					case FILE_TYPE   :if (vars[i].value.stringvalue)
													            safe_free(vars[i].value.stringvalue);
						                 break;
     case MATRIX_TYPE :if (vars[i].value.matrixvalue)
                        {
                         matrix_free(*(vars[i].value.matrixvalue));
                         safe_free(vars[i].value.matrixvalue);
                        }
                       break;
					case ARRAY_TYPE  :free_multidimensional_array(&(vars[i]));
												           break;
     default          :break;
			 }
	 }
}

void create_multidimensional_array(variableptr myvar, int* dimensions, int curdimension, int dimcount, int type)
{
 /*!Last Changed 04.01.2006*/
 int k, size;
 size = dimensions[curdimension];
 myvar->arraysize = size;
 myvar->value.array = (variableptr)safemalloc(size * sizeof(struct variable), "create_multidimensional_array", 4);
 for (k = 0; k < size; k++)
   if (curdimension + 1 < dimcount)
    {
     myvar->value.array[k].type = ARRAY_TYPE;
     myvar->value.array[k].name = NULL;
     create_multidimensional_array(&(myvar->value.array[k]), dimensions, curdimension + 1, dimcount, type);
    }
   else
    {
     myvar->value.array[k].name = NULL;
     myvar->value.array[k].type = type;
     myvar->value.array[k].arraysize = 0;
     set_default_value(&(myvar->value.array[k]));
    }
}

int add_variable(VARIABLE_TYPE type, char** names, int count, variableptr* vars, int* varcount)
{
 /*!Last Changed 04.01.2006 added create_multidimensional_array*/
	/*!Last Changed 29.11.2004 added arraysize*/
	/*!Last Changed 02.02.2004 added safemalloc*/
 /*!Last Changed 02.05.2002*/
 int i, j, result = 1, *dimensions, dimcount;
 char* st;
 for (j = 0; j < count; j++)
  {
   i = return_variable_index(names[j], *vars, *varcount);
   if (i != -1)
			 {
				 result = 0;
     continue;
			 }
   *vars = (variableptr)alloc(*vars, sizeof(Variable) * (*varcount + 1), *varcount + 1);
   st = array_variable(names[j]);
   if (st)
    {
     (*vars)[*varcount].type = ARRAY_TYPE;
     (*vars)[*varcount].name = strcopy((*vars)[*varcount].name, st);
     dimensions = array_sizes(names[j], &dimcount);
     create_multidimensional_array(&((*vars)[*varcount]), dimensions, 0, dimcount, type);
     safe_free(dimensions);
     safe_free(st);
    }
   else
    {
				 (*vars)[*varcount].arraysize = 0;
     (*vars)[*varcount].type = type;
     (*vars)[*varcount].name = strcopy((*vars)[*varcount].name, names[j]);
     set_default_value(&((*vars)[*varcount]));
    }
   (*varcount)++;
  }
 return result;
}

int set_default_variable(char* name, double value)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 16.04.2002*/
 variableptr myvar;
 myvar = get_variable(name);
 if (!myvar)
   return 0;
 myvar->value.floatvalue = value;
 return 1;
}

int set_default_string_variable(char* name, char* st)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 05.05.2002*/
 variableptr myvar;
 myvar = get_variable(name);
 if (!myvar)
   return 0;
 if (myvar->value.stringvalue)
   safe_free(myvar->value.stringvalue);
 myvar->value.stringvalue = strcopy(myvar->value.stringvalue, st);
 return 1;
}

int set_variable(char* name, void* value)
{
 /*!Last Changed 22.12.2006 added matrix type*/
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 05.11.2003 Added File variable with type 4*/
 /*!Last Changed 02.05.2002*/
 variableptr myvar;
 myvar = get_variable(name);
 if (!myvar)
  {
   printf(m1437, name);
   return 0;
  }
 switch (myvar->type)
  {
   case INTEGER_TYPE:myvar->value.intvalue = atoi((char *)value);
                     break;
   case REAL_TYPE   :myvar->value.floatvalue = atof((char *)value);
                     break;
   case ARRAY_TYPE  :set_multidimensional_array(myvar, value);
                     break;
   case STRING_TYPE :
			case FILE_TYPE   :if (myvar->value.stringvalue)
                       safe_free(myvar->value.stringvalue);
                     myvar->value.stringvalue = strcopy(myvar->value.stringvalue, (char *)value);
                     break;
   case MATRIX_TYPE :if (myvar->value.matrixvalue)
																					 {
																						 matrix_free(*(myvar->value.matrixvalue));
																						 safe_free(myvar->value.matrixvalue);
                       myvar->value.matrixvalue = NULL;
																					 }
                     if (strcmp((char *)value, "0") != 0)
  				                 myvar->value.matrixvalue = (matrixptr)value;
                     break;
   default          :break;
  }
 return 1;
}

int set_for_variable(char* name, char* end, int step, int ip)
{
 /*!Last Changed 03.01.2006*/
 /*!Last Changed 05.04.2002*/
 int i;
 variableptr myvar;
 myvar = get_variable(name);
 if (!myvar)
   return 0;
 if (myvar->type != INTEGER_TYPE)
   return 0;
 i = atoi(end);
 myvar->end = i;
 myvar->step = step;
 myvar->ip = ip;
 return 1;
}

int check_if_statement(char** params)
{
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 05.11.2003 Added File variable with type 4*/
 /*!Last Changed 03.05.2002*/
 int operand;
 double value1, value2, compresult;
 char* st = NULL;
 variableptr myvar;
 operand = listindex(params[0], operandtype, 8);
 if (operand == -1)
   return 0;
 myvar = get_variable(params[1]);
 switch (myvar->type)
  {
   case INTEGER_TYPE:value1 = myvar->value.intvalue;
                     value2 = atoi(params[2]);
                     compresult = value1 - value2;
                     break;
   case REAL_TYPE   :value1 = myvar->value.floatvalue;
                     value2 = atof(params[2]);
                     compresult = value1 - value2;
                     break;
   case STRING_TYPE :
			case FILE_TYPE   :st = strcopy(st, myvar->value.stringvalue);
                     compresult = strcmp(st, params[2]);
                     safe_free(st);
                     break;
   default          :break;
  }
 switch (operand)
  {
   case 0:if (compresult < 0)
            return 1;
          else
            return 0;
   case 1:if (compresult <= 0)
            return 1;
          else
            return 0;
   case 2:if (compresult > 0)
            return 1;
          else
            return 0;
   case 3:if (compresult >= 0)
            return 1;
          else
            return 0;
   case 4:
   case 5:if (compresult == 0)
            return 1;
          else
            return 0;
   case 6:
   case 7:if (compresult != 0)
            return 1;
          else
            return 0;
  }
 return 0;
}

int check_of_variables(char* st, variableptr variables, int variablecount)
{
	/*!Last Changed 12.10.2004*/
	int i = 0, len = strlen(st), mode = 0, start, end, j;
	char varname[READLINELENGTH];
	while (i < len)
	 {
		 if (st[i] == '%')
			 {
				 mode = (mode + 1) % 2;
					if (mode == 1)
						 start = i + 1;
					else
					 {
						 end = i - 1;
							if (start < end)
							 {
								 for (j = start; j <= end; j++)
										 varname[j - start] = st[j];
									varname[j - start] = '\0';
         if (return_variable_index(varname, variables, variablecount) == -1)
										 return 0;
							 }
					 }
			 }
		 i++;
	 }
	return 1;
}

void change_special_characters(char* st)
{
	/*!Last Changed 13.07.2005*/
	int i, len;
	len = strlen(st);
	for (i = 0; i < len; i++)
		 switch (st[i])
	   {
		   case '_':st[i] = ' ';
						        break;
					case '#':st[i] = '_';
						        break;
	   }
}

variableptr get_variable(char* st)
{
	/*!Last Changed 02.01.2006*/
	int ind;
	char* st2;
 variableptr myvar;
 ind = return_variable_index(st, vars, varcount);
 if (ind != -1)
		{
   st2 = array_variable(st);
   if (st2)
				{
     myvar = array_pos(st, &(vars[ind]));
     safe_free(st2);
					return myvar;
			 }
   else
				 return &(vars[ind]);
	 }
	return NULL;
}

variableptr get_and_check_variable_type(char* name, VARIABLE_TYPE validtype, char* message)
{
 /*!Last Changed 03.01.2006*/
 variableptr myvar;
 myvar = get_variable(name);
 if (!myvar)
   printf(m1302, name);
 else
   if (myvar->type != validtype)
    {
     printf(m1303, message);
     return NULL;
    }
 return myvar;
}

void free_expnode(Expnodeptr node)
{
 if (node->type == NO_TYPE)
   safe_free(node->name);
 safe_free(node);
}

void add_expnode(char* st, Expnodeptr* tmpbefore, Expnodeptr* start)
{
 /*!Last Changed 03.01.2006*/
 Expnodeptr tmp;
 variableptr myvar;
 if (st[0] == '\0')
   return;
 tmp = safemalloc(sizeof(Expnode), "add_expnode", 5);
 if (!isfloat(st) && (myvar = get_variable(st)) != NULL && myvar->type != ARRAY_TYPE)
  {
   tmp->type = myvar->type;
   tmp->value = myvar->value;
  }
 else
   if (isinteger(st) && strcmp(st, "+") != 0 && strcmp(st, "-") != 0)
    {
     tmp->type = INTEGER_TYPE;
     tmp->value.intvalue = atoi(st);
    }
   else
     if (isfloat(st) && strcmp(st, "+") != 0 && strcmp(st, "-") != 0)
      {
       tmp->type = REAL_TYPE;
       tmp->value.floatvalue = atof(st);
      }
     else
      {
       tmp->type = NO_TYPE;
       tmp->name = strcopy(tmp->name, st);
      }
 tmp->next = NULL;
 if (*tmpbefore)
   (*tmpbefore)->next = tmp;
 else
   *start = tmp;
 *tmpbefore = tmp;
}

Expnodeptr create_expression_list(char* st)
{
 /*!Last Changed 03.01.2006*/
 char st1[READLINELENGTH];
 int i = 0, j = 0, indexs, indexo;
 Expnodeptr start = NULL, tmpbefore = NULL;
 while (st[i])
  {
   indexs = listindexc(st[i], separators, 5);
   indexo = listindexc(st[i], operators, 7);
   if (indexs != -1 || indexo != -1)
    {
     st1[j] = '\0';
     add_expnode(st1, &tmpbefore, &start);
     st1[0] = st[i];
     st1[1] = '\0';
     add_expnode(st1, &tmpbefore, &start);
     j = -1;
    }
   else
     st1[j] = st[i];
   i++;
   j++;
  }
 st1[j] = '\0';
 add_expnode(st1, &tmpbefore, &start); 
 st1[0] = st[i];
 st1[1] = '\0';
 add_expnode(st1, &tmpbefore, &start);
 return start;
}

void print_expression_list(Expnodeptr list)
{
 /*!Last Changed 03.01.2006*/
 Expnodeptr tmp = list;
 while (tmp)
  {
   switch (tmp->type)
    {
     case  NO_TYPE     :printf("%s ", tmp->name);
                        break;
     case  INTEGER_TYPE:printf("%d ", tmp->value.intvalue);
                        break;
     case  REAL_TYPE   :printf("%.4f ", tmp->value.floatvalue);
                        break;
     case  STRING_TYPE :
     case  FILE_TYPE   :printf("%s ", tmp->value.stringvalue);
                        break;
     default           :break;
    }
   tmp = tmp->next;
  }
 printf("\n");
}

Expnodeptr evaluate_operation(Expnodeptr operand1, Expnodeptr operand2, int optype)
{
 /*!Last Changed 12.08.2010*/
 double value1, value2, result;
 Expnodeptr tmp;
 tmp = safemalloc(sizeof(Expnode), "evaluate_operation", 3);
 switch (operand1->type)
  {
   case INTEGER_TYPE:value1 = operand1->value.intvalue;
                     break;
   case REAL_TYPE   :value1 = operand1->value.floatvalue;
                     break;
   default          :value1 = 1.0;
                     break;
  }
 switch (operand2->type)
  {
   case INTEGER_TYPE:value2 = operand2->value.intvalue;
                     break;
   case REAL_TYPE   :value2 = operand2->value.floatvalue;
                     break;
   default          :value2 = 1.0;
                     break;
  }
 if (operand1->type == INTEGER_TYPE && operand2->type == INTEGER_TYPE)
   tmp->type = INTEGER_TYPE;
 else 
   tmp->type = REAL_TYPE;
 switch (optype)
  {
   case 0:result = value1 + value2;
          break;
   case 1:result = value1 - value2;
          break;
   case 2:result = value1 * value2;
          break;
   case 3:result = value1 / value2;
          tmp->type = REAL_TYPE;
          break;
   case 4:result = pow(value1, value2);
          break;
   case 5:result = log(value2) / log(value1);
          tmp->type = REAL_TYPE;
          break;
   case 6:result = value1 - value2 * ((int)(value1 / value2));
          break;
  }
 if (tmp->type == INTEGER_TYPE)
   tmp->value.intvalue = (int)result;
 else 
  tmp->value.floatvalue = result;
 return tmp;
}

int has_higher_or_equal_precedence(int op1, int op2)
{
 if (op1 >= op2)
   return YES;
 if (op1 == 0 && op2 == 1)
   return YES;
 if (op1 == 2 && op2 == 3)
   return YES;
 return NO;
}

Expnodeptr insert_into_exp_list(Expnodeptr* list, Expnodeptr tmp, Expnodeptr inserted)
{
 if (tmp != NULL)
   tmp->next = inserted;
 else 
   *list = inserted;
 return inserted;
}

Expnodeptr infix_to_postfix_conversion(Expnodeptr list)
{
 Expnodeptr s[STACKSIZE];
 int top = -1, currentop, stackop;
 Expnodeptr outputqueue = NULL, tmpoutput = NULL;
 Expnodeptr tmp = list, next;
 while (tmp)
  {
   next = tmp->next;
   if (tmp->type != NO_TYPE)
     tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, tmp);
   else 
     if (tmp->next && tmp->next->type == NO_TYPE && tmp->next->name[0] == '[') /*function token like myarray in myarray[*/
      {
       top++;
       s[top] = tmp;
      }
     else
       switch (tmp->name[0]) {
         case ',':while (top != -1 && s[top]->name[0] != '[') /*token is an array argument separator*/
                   {
                    tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, s[top]);
                    top--;
                   }
                  free_expnode(tmp);
                  if (top == -1)
                    return NULL;
                  break;
         case '[':
         case '(':top++;
                  s[top] = tmp;
                  break;
         case ']':while (top != -1 && s[top]->name[0] != '[')
                   {
                    tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, s[top]);
                    top--;
                   }
                  free_expnode(tmp);
                  if (top == -1)
                    return NULL;
                  free_expnode(s[top]);
                  top--;
                  tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, s[top]);
                  top--;         
                  break;
         case ')':while (top != -1 && s[top]->name[0] != '(')
                   {
                    tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, s[top]);
                    top--;
                   }
                  free_expnode(tmp);
                  if (top == -1)
                    return NULL;
                  free_expnode(s[top]);
                  top--;
                  break;
         default :currentop = listindexc(tmp->name[0], operators, 7);
                  if (currentop != -1)
                   {
                    if (top != -1)
                      stackop = listindexc(s[top]->name[0], operators, 7);
                    else 
                      stackop = -1;
                    while (stackop != -1 && has_higher_or_equal_precedence(stackop, currentop))
                     {
                      tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, s[top]);
                      top--;
                      if (top != -1)
                        stackop = listindexc(s[top]->name[0], operators, 7);
                      else 
                        stackop = -1;
                     }
                    top++;
                    s[top] = tmp;
                   }
                  break;
       }
   tmp = next;
  }
 while (top != -1)
  {
   tmpoutput = insert_into_exp_list(&outputqueue, tmpoutput, s[top]);
   top--;
  }
 if (tmpoutput)
   tmpoutput->next = NULL;
 return outputqueue;
}

variableptr evaluate_expression_list(Expnodeptr list)
{
 /*!Last Changed 12.08.2010*/
 /*!Last Changed 03.01.2006*/
 variableptr result, tmparray;
 char st[MAXLENGTH];
 Expnodeptr s[STACKSIZE];
 Stackptr indexstack;
 Expnodeptr tmp, operand1, operand2, tmpresult, next;
 int top = -1, optype, index;
 tmp = infix_to_postfix_conversion(list);
 while (tmp)
  {
   next = tmp->next;
   if (tmp->type != NO_TYPE)
    {
     top++;
     s[top] = tmp;
    }
   else
    {
     optype = listindexc(tmp->name[0], operators, 7);
     if (optype == -1)
      {
       sprintf(st, "%s[]", tmp->name);
       tmparray = get_variable(st);
       if (tmparray && tmparray->type == ARRAY_TYPE)
        {
         indexstack = stack();
         while (tmparray->type == ARRAY_TYPE)
          {
           operand1 = s[top];
           if (operand1->type == INTEGER_TYPE)
             index = operand1->value.intvalue;
           else 
             index = (int) operand1->value.floatvalue;
           push(indexstack, index);
           tmparray = &(tmparray->value.array[0]);
           top--;
           free_expnode(operand1);
          }
         tmparray = get_variable(st);
         while (tmparray->type == ARRAY_TYPE)
          {
           index = pop(indexstack);
           tmparray = &(tmparray->value.array[index - 1]);
          }
         free_stack(indexstack);
         tmpresult = safemalloc(sizeof(Expnode), "evaluate_expression_list", 33);
         tmpresult->type = tmparray->type;
         if (tmparray->type == INTEGER_TYPE)
           tmpresult->value.intvalue = tmparray->value.intvalue;
         else
           tmpresult->value.floatvalue = tmparray->value.floatvalue;
         top++;
         s[top] = tmpresult;
        }
       else
         return NULL;
      }
     else 
      {
       operand2 = s[top];
       top--;
       operand1 = s[top];
       top--;
       tmpresult = evaluate_operation(operand1, operand2, optype);
       free_expnode(operand1);
       free_expnode(operand2);
       top++;
       s[top] = tmpresult;
      }
     free_expnode(tmp);
    }
   tmp = next;
  }
 result = safemalloc(sizeof(struct variable), "evaluate_expression_list", 69);
 result->value = s[top]->value;
 result->type = s[top]->type;
 free_expnode(s[top]);
 return result;
}

void change_according_to_variables(char* st)
{
 /*!Last Changed 03.01.2006*/
	/*!Last Changed 19.01.2005 added _ for space*/
	/*!Last Changed 27.01.2004*/
	/*!Last Changed 05.11.2003 Added File variable with type 4*/
 /*!Last Changed 02.05.2002*/
 int i, len, start, percindex, removed, added, j, found = 0;
	char st3[READLINELENGTH], temp[READLINELENGTH], temp2[READLINELENGTH];
	variableptr myvar;
 Expnodeptr list;
	len = strlen(st);
 for (i = 0; i < len; i++)
			if (st[i] == '%')
				 found = 1;
	if (!found)
		 return;
	i = 0;
	start = 0;
 while (st[i])
	 {
		 if (st[i] == '%')
				 if (!start)
					 {
						 start = 1;
							percindex = i;
							strcpy(st3, "");
					 }
					else
					 {
						 start = 0;
       list = create_expression_list(st3);
       myvar = evaluate_expression_list(list);
       if (myvar)
							 {
         switch (myvar->type)
										{
           case INTEGER_TYPE:sprintf(temp, "%d", myvar->value.intvalue);
                             break;
           case REAL_TYPE   :set_precision(temp2, NULL, NULL);
												                 sprintf(temp, temp2, myvar->value.floatvalue);
                             break;
           case STRING_TYPE :
											case FILE_TYPE   :if (myvar->value.stringvalue)
                               sprintf(temp,"%s", myvar->value.stringvalue);
                             else
                               sprintf(temp,"NULL");
                             break;
           default          :break;
										}
         safe_free(myvar);
         removed = i - percindex + 1;
									added = strlen(temp);
									len = strlen(st);
									if (added > removed)
										 for (j = len; j > i; j--)
												 st[added - removed + j] = st[j];
									else
									  if (added < removed)
												 for (j = i + 1; j <= len; j++)
														 st[j + added - removed] = st[j];
									for (j = 0; j < added; j++)
										 if (temp[j] != ' ')
  										 st[percindex + j] = temp[j];
											else
												 st[percindex + j] = '_';
									i = i + added - removed;
						  }
					 }
			else
			  if (start)
						 sprintf(st3, "%s%c", st3, st[i]);
			i++;
	 }
}
