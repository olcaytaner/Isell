#ifndef __lang_H
#define __lang_H
#include "typedef.h"

enum construct_type{ 
      IF = 0,
      ELSE,
      ENDIF,
      SWITCH,
      ENDSWITCH,
      CASE,
      FOR,
      ENDFOR,
      WHILE,
      ENDWHILE};

typedef enum construct_type CONSTRUCT_TYPE;

#define DEFAULT_VARIABLE_COUNT 10
#define DEFAULT_STRING_VARIABLE_COUNT 10
#define DEFAULT_ARRAY_VARIABLE_COUNT 30
#define DEFAULT_PERFORMANCE_VARIABLE_COUNT 2

void        add_default_variables(variableptr* variables, int* variablecount);
void        add_expnode(char* st, Expnodeptr* tmpbefore, Expnodeptr* start);
int         add_variable(VARIABLE_TYPE type, char** names, int count, variableptr* vars, int* varcount);
variableptr array_pos(char *name, variableptr myvar);
int*        array_sizes(char* name, int* dimcount);
char*       array_variable(char* name);
void        change_according_to_variables(char* st);
void        change_special_characters(char* st);
void        charatindex(char* name, int index);
int         check_if_statement(char** params);
int         check_of_variables(char* st, variableptr variables, int variablecount);
int         check_parameter_type(char* varname, VARIABLE_TYPE vartype, variableptr variables, int variablecount);
int         check_parameters(char *params[MAXPARAMS], int paramcount, char* grammar[MAXPARAMS], int grammarcount, variableptr variables, int variablecount);
BOOLEAN     check_var(struct variable var,char* value);
int         check_variable(char* name, char* value);
void        close_userfile(char* name);
int         compile_file(char *filename);
void        countchar(char* name, char* ch);
Expnodeptr  create_expression_list(char* st);
Expnodeptr  evaluate_operation(Expnodeptr operand1, Expnodeptr operand2, int optype);
variableptr evaluate_expression_list(Expnodeptr list);
void        free_expnode(Expnodeptr node);
void        free_multidimensional_array(variableptr myvar);
void        free_variables(variableptr vars, int count);
variableptr get_and_check_variable_type(char* name, VARIABLE_TYPE validtype, char* message);
char*       get_string_value(char* name);
double      get_value(struct variable var);
variableptr get_variable(char* st);
void        goto_endconstructor(int* variable, char* constructorname, char* endconstructorname, char** code, int linecount, int* instruction_pointer);
int         has_higher_or_equal_precedence(int op1, int op2);
Expnodeptr  infix_to_postfix_conversion(Expnodeptr list);
Expnodeptr  insert_into_exp_list(Expnodeptr* list, Expnodeptr tmp, Expnodeptr inserted);
BOOLEAN     make_operation(char op, char* name, char* value);
void        open_userfile(char* name, char* filemode);
void        print_expression_list(Expnodeptr list);
void        print_variables();
void        read_array_from_userfile(char* name, char* varname, int size);
void        read_userfile(char* name, char* varname);
int         return_variable_index(char* name, variableptr vars, int varcount);
int         set_default_string_variable(char* name, char* st);
void        set_default_value(struct variable* var);
int         set_default_variable(char* name, double value);
int         set_for_variable(char* name, char* end, int step, int ip);
void        set_multidimensional_array(variableptr myvar, void* value);
void        set_value(struct variable* var, double value1);
int         set_variable(char* name, void* value);
void        stringlength(char* name);
void        tokenize(char* name, char* separators);
BOOLEAN     type_check(variableptr myvar, VARIABLE_TYPE validtype);
void        write_array_variables(char* arrayname, int index, int start, char* vartypes, ...);
void        write_userfile(char* name, char* st);

#endif
