#ifndef __command_H
#define __command_H

#define NUMBER_OF_INFORMATION_LINES 10
#define MAX_EXAMPLES 5

/*! M-ary tree node for storing the commands of ISELL. Except the root node, each node may have sister(s), and every node except the leaf nodes may have children. The three has 3 levels. In the first level, command groups are stored. In the second level, command subgroups are stored. In the last level commands are stored*/
struct command_node{
                    /*! Name of the command*/
	                   char* command;
                    /*! Pointer to the next sister of the current node*/
																				struct command_node* next;
                    /*! Pointer to the first child of the current node*/
																				struct command_node* child;
};

typedef struct command_node Command_node;
typedef Command_node* Command_nodeptr;

/*! Struct storing the commands of the ISELL. These command properties are read from the command.txt file*/
struct order{
                 /*! Name of the command*/
	                char *command;
                 /*! Group of the command*/
																	char *command_group;
                 /*! Subgroup of the command*/
																	char *command_subgroup;
                 /*! What will the function return to the default variables? out1, out2, aout1 etc.*/
																	char *function_return;
                 /*! Grammar of the command. Used in the compilation phase. */
                 char *grammar;
                 /*! Usage of the command. Defined parameters in detail.*/
																	char *usage;
                 /*! Detailed help message about the command and its usage*/
																	char *help_messages;
                 /*! Example scripts that uses this command*/
                 char* examples[MAX_EXAMPLES];
                 /*!Number of example files. Size of the examples array*/
                 int examplecount;
                 /*! If can_be_parallelized is BOOLEAN_TRUE the command can be internally parallelized*/
                 int can_be_parallelized;
};

typedef struct order Order;
typedef Order* Orderptr;

Command_nodeptr add_command_child(Command_nodeptr root, char* st);
Command_nodeptr child_of_command(Command_nodeptr root, char* st);
void            deallocate_command_tree(Command_nodeptr root);
void            deallocate_orders();
Command_nodeptr generate_command_tree();
int             get_order_index(char* order);
char*           grammar_of_order(int res);
void            initialize_orders();
void            print_command_node(Command_nodeptr root, int level);
void            print_command_tree(Command_nodeptr root);
void            print_order(int res);
void            search_help(char* st);

#endif
