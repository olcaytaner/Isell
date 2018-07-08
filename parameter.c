#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"
#include "parameter.h"
#include "xmlparser.h"

Parameterptr parameters;
int parametercount;

char *parameter_types[MAX_PARAMETER_TYPE] = {"INTEGER", "FLOAT", "ONOFF", "LIST", "STRING", "ARRAY"};
char *onoff_type[2] = {"ON", "OFF"};

/**
 * Given its name, finds the index of the parameter in the parameters array
 * @param[in] name Name of the parameter. The name is case insensitive.
 * @return Index of the parameter in the parameters array. If the parameter does not exist returns -1.
 */
int parameter_index(char* name)
{
 /*!Last Changed 09.08.2007*/
 int i;
 for (i = 0; i < parametercount; i++)
   if (strIcmp(parameters[i].name, name) == 0)
     return i;
 return -1;
}

/**
 * Returns the value of an integer parameter
 * @param[in] name Name of the integer parameter
 * @return Value of the parameter with integer type. If the parameter does not exist returns 0.
 */
int get_parameter_i(char* name)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == INTEGER_PARAMETER)
   return parameters[index].value.intvalue;
 else
   return 0;
}

/**
 * Sets the value of an integer type parameter
 * @param[in] name Name of the integer parameter
 * @param[in] value New value of the integer parameter. Stored as a string.
 * @param[in] min Minimum threshold of the integer parameter
 * @param[in] max Maximum threshold of the integer parameter
 */
void set_parameter_i(char* name, char* value, int min, int max)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == INTEGER_PARAMETER)
   atoi_check(value, &(parameters[index].value.intvalue), min, max);
}

/**
 * Sets the value of an integer type parameter
 * @param[in] name Name of the integer parameter
 * @param[in] value New value of the integer parameter.
 */
void set_integer_parameter(char* name, int value)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == INTEGER_PARAMETER)
 		parameters[index].value.intvalue = value;
}

/**
 * Returns the value of a float parameter
 * @param[in] name Name of the float parameter
 * @return Value of the parameter with float type. If the parameter does not exist returns 0.0.
 */
double get_parameter_f(char* name)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == FLOAT_PARAMETER)
   return parameters[index].value.floatvalue;
 else
   return 0.0;
}

/**
 * Sets the value of a float type parameter
 * @param[in] name Name of the float parameter
 * @param[in] value New value of the float parameter. Stored as a string.
 * @param[in] min Minimum threshold of the integer parameter
 * @param[in] max Maximum threshold of the integer parameter
 */
void set_parameter_f(char* name, char* value, double min, double max)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == FLOAT_PARAMETER)
   atof_check(value, &(parameters[index].value.floatvalue), min, max);
}

/**
 * Sets the value of a float type parameter
 * @param[in] name Name of the float parameter
 * @param[in] value New value of the float parameter.
 */
void set_float_parameter(char* name, double value)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == FLOAT_PARAMETER)
		 parameters[index].value.floatvalue = value;
}

/**
 * Returns the value of a string parameter
 * @param[in] name Name of the string parameter
 * @return Value of the parameter with string type. If the parameter does not exist returns NULL.
 */
char* get_parameter_s(char* name)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == STRING_PARAMETER)
   return parameters[index].value.stringvalue;
 else
   return NULL;
}

/**
 * Sets the value of a string type parameter
 * @param[in] name Name of the string parameter
 * @param[in] value New value of the string parameter.
 */
void set_parameter_s(char* name, char* value)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == STRING_PARAMETER)
  {
   if (parameters[index].value.stringvalue != NULL)
     safe_free(parameters[index].value.stringvalue);
   parameters[index].value.stringvalue = strcopy(parameters[index].value.stringvalue, value);
  }
}

/**
 * Returns the value of a list parameter. 
 * @param[in] name Name of the list parameter
 * @return Value of the parameter with list type. If the parameter does not exist returns -1.
 */
int get_parameter_l(char* name)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == LIST_PARAMETER)
   return parameters[index].value.listvalue;
 else
   return -1;
}

/**
 * Sets the value of a list type parameter.
 * @param[in] name Name of the list type parameter.
 * @param[in] value New value of the list type parameter. Stored as a string.
 * @param[in] errormessage If the new value of the list type parameter is not one of the allowed values, prints this error message.
 */
void set_parameter_l(char* name, char* value, char* errormessage)
{
 /*!Last Changed 09.08.2007*/
 int i, index, val;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == LIST_PARAMETER)
  {
   val = listindex(value, parameters[index].listvalues, parameters[index].arraysize);
   if (val != -1)
     parameters[index].value.listvalue = val;
   else
    {
     printf(errormessage, value);
     printf(m1276);
     for (i = 0; i < parameters[index].arraysize; i++)
      {
       printf("%s", parameters[index].listvalues[i]);
       printf("\n");
      }     
    }
  }
}

/**
 * Returns the value of an on-off parameter. An on-off parameter has only on or off value.
 * @param[in] name Name of the on-off parameter
 * @return Value of the parameter with on-off type. If the parameter does not exist returns -1.
 */
int get_parameter_o(char* name)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == ONOFF_PARAMETER)
   return parameters[index].value.onoffvalue;
 else
   return -1;
}

/**
 * Sets the value of an on-off type parameter. An on-off parameter has only on or off value.
 * @param[in] name Name of the on-off parameter
 * @param[in] value New value of the on-off parameter.
 */
void set_parameter_o(char* name, char* value)
{
 /*!Last Changed 09.08.2007*/
 int index, val;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == ONOFF_PARAMETER)
  {
   val = listindex(value, onoff_type, 2);
   if (val != -1)
     parameters[index].value.onoffvalue = 1 - val;
   else
     printf(m1043);
  }
}

/**
 * Returns the address of an array parameter.
 * @param[in] name Name of the array parameter
 * @return the address of the parameter with array type. If the parameter does not exist returns NULL.
 */
void* get_parameter_a(char* name)
{
 /*!Last Changed 09.08.2007*/
 int index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == ARRAY_PARAMETER)
   return parameters[index].value.arrayvalue;
 else
   return NULL;
}

/**
 * Fills the values of the float array parameter with the given values.
 * @param[in] name Name of the array parameter
 * @param[in] values Values that will be assigned to the array. Values are stored as array of strings.
 * @param[in] arraysize Size of the array
 */
void set_parameter_af(char* name, char** values, int arraysize)
{
 /*!Last Changed 09.08.2007*/
 int i, index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == ARRAY_PARAMETER)
  {
   for (i = 0; i < parameters[index].arraysize && i < arraysize; i++)
     atof_check(values[i], &(((double *)(parameters[index].value.arrayvalue))[i]), -DBL_MAX, +DBL_MAX);
  }
}

/**
 * Fills the values of the integer array parameter with the given values.
 * @param[in] name Name of the array parameter
 * @param[in] values Values that will be assigned to the array. Values are stored as array of strings.
 * @param[in] arraysize Size of the array
 */
void set_parameter_ai(char* name, char** values, int arraysize)
{
 /*!Last Changed 09.08.2007*/
 int i, index;
 index = parameter_index(name);
 if (index != -1 && parameters[index].type == ARRAY_PARAMETER)
  {
   for (i = 0; i < parameters[index].arraysize && i < arraysize; i++)
     atoi_check(values[i], &(((int *)(parameters[index].value.arrayvalue))[i]), -INT_MAX, +INT_MAX);
  }
}

/**
 * Reads the parameters of the ISELL program from the parameters.xml file.
 */
BOOLEAN read_program_parameters()
{
 /*!Last Changed 04.03.2009*/
 /*!Last Changed 08.08.2007*/
 Xmldocumentptr doc;
 Xmlelementptr xmlparameter, xmlvalue;
 int i, j, index;
 char *st;
 doc = xml_document("parameters.xml");
 if (!doc)
  {
   printf(m1429);
   return BOOLEAN_FALSE;
  }
 if (!xml_parse(doc))
   return BOOLEAN_FALSE;
 parametercount = xml_number_of_children(doc->root);
 parameters = safemalloc(parametercount * sizeof(Parameter), "read_program_parameters", 8);
 xmlparameter = doc->root->child;
 i = 0;
 while (xmlparameter)
  {
   st = xml_get_attribute_value(xmlparameter, "name");
   if (st)
     parameters[i].name = strcopy(parameters[i].name, st);
   else
     parameters[i].name = NULL;
   st = xml_get_attribute_value(xmlparameter, "type");
   if (st)
     parameters[i].type = listindex(st, parameter_types, MAX_PARAMETER_TYPE);
   else
     parameters[i].type = INTEGER_PARAMETER;
   st = xml_get_attribute_value(xmlparameter, "value");
   switch (parameters[i].type)
    {
     case INTEGER_PARAMETER:if (st)
                              atoi_check(st, &(parameters[i].value.intvalue), -INT_MAX, +INT_MAX);
                            else
                              parameters[i].value.intvalue = 0;
                            break;
     case   FLOAT_PARAMETER:if (st)
                              atof_check(st, &(parameters[i].value.floatvalue), -DBL_MAX, +DBL_MAX);
                            else
                              parameters[i].value.floatvalue = 0.0;
                            break;
     case   ONOFF_PARAMETER:index = listindex(st, onoff_type, 2);
                            if (index != -1)
                              parameters[i].value.onoffvalue = 1 - index;
                            break;
     case  STRING_PARAMETER:if (st != NULL)
                              parameters[i].value.stringvalue = strcopy(parameters[i].value.stringvalue, st);
                            else
                              parameters[i].value.stringvalue = NULL;
                            break;
     case   ARRAY_PARAMETER:st = xml_get_attribute_value(xmlparameter, "size");
                            if (st)
                              atoi_check(st, &(parameters[i].arraysize), 1, +INT_MAX);
                            else
                              parameters[i].arraysize = 0;
                            st = xml_get_attribute_value(xmlparameter, "subtype");
                            if (st)
                             {
                              switch (listindex(st, parameter_types, MAX_PARAMETER_TYPE)){
                                case INTEGER_PARAMETER:parameters[i].value.arrayvalue = (int*) safemalloc(parameters[i].arraysize * sizeof(int), "read_program_parameters", 73);
                                                       st = xml_get_attribute_value(xmlparameter, "value");
                                                       for (j = 0; j < parameters[i].arraysize; j++)
                                                           ((int *)(parameters[i].value.arrayvalue))[j] = atoi(st);
                                                       break;
                                case   FLOAT_PARAMETER:parameters[i].value.arrayvalue = (double*) safemalloc(parameters[i].arraysize * sizeof(double), "read_program_parameters", 78);
                                                       st = xml_get_attribute_value(xmlparameter, "value");
                                                       for (j = 0; j < parameters[i].arraysize; j++)
                                                           ((double *)(parameters[i].value.arrayvalue))[j] = atof(st);
                                                       break;
                              }
                             }
                            break;
     case    LIST_PARAMETER:parameters[i].arraysize = xml_number_of_children(xmlparameter);
                            parameters[i].listvalues = safemalloc(parameters[i].arraysize * sizeof(char*), "read_program_parameters", 54);
                            xmlvalue = xmlparameter->child;
                            j = 0;
                            while (xmlvalue)
                             {
                              parameters[i].listvalues[j] = strcopy(parameters[i].listvalues[j], xmlvalue->pcdata);
                              xmlvalue = xmlvalue->sibling;
                              j++;
                             }
                            parameters[i].value.listvalue = listindex(st, parameters[i].listvalues, parameters[i].arraysize); 
                            break;
    }     
   xmlparameter = xmlparameter->sibling;
   i++;
  }
 xml_free_document(doc);
 return BOOLEAN_TRUE;
}

/**
 * Frees memory allocated to the parameters array and its elements.
 */
void free_program_parameters()
{
 /*!Last Changed 08.08.2007*/
 int i, j;
 for (i = 0; i < parametercount; i++)
  {
   safe_free(parameters[i].name);
   switch (parameters[i].type)
    {
     case STRING_PARAMETER:safe_free(parameters[i].value.stringvalue);
                           break;
     case   LIST_PARAMETER:for (j = 0; j < parameters[i].arraysize; j++)
                             safe_free(parameters[i].listvalues[j]);
                           safe_free(parameters[i].listvalues);
                           break;
     case  ARRAY_PARAMETER:safe_free(parameters[i].value.arrayvalue);
                           break;
     default              :break;
    }
  }
 safe_free(parameters);
}
