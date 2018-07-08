#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "messages.h"
#include "libfile.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "screen.h"

int ordercount;
Orderptr orders;

/**
 * Returns the grammar of a given command
 * @param[in] res Index of the command
 * @return Grammar of the command
 */
char* grammar_of_order(int res)
{
	return orders[res].grammar;
}

/**
 * Returns the index of a command
 * @param[in] order Command
 * @return If the command exists in the commands array returns the index of the command, otherwise returns -1.
 */
int get_order_index(char* order)
{
	/*!Last Changed 08.01.2006*/
	int i;
	if (!order)
		 return -2;
	for (i = 0; i < ordercount; i++)
		 if (strcmp(order, orders[i].command) == 0)
				 return i;
	return -1;
}

/**
 * Searches all commands (its name, its usage and its help messages) for a given string
 * @param[in] st Substring to search for
 */
void search_help(char* st)
{
	/*!Last Changed 19.01.2005 changed the instr*/
	/*!Last Changed 03.02.2004*/
	int i;
	for (i = 0; i < ordercount; i++)
		 if (instr(orders[i].command, st) != -1)
				 printf("%s ", orders[i].command);
			else
				 if (instr(orders[i].usage, st) != -1)
						 printf("%s ", orders[i].command);
					else
						 if (instr(orders[i].help_messages, st) != -1)
								 printf("%s ", orders[i].command);
	printf("\n");
}

/**
 * Destructor for the orders array. Deallocates memory for each command (all string fields) and memory for the array.
 */
void deallocate_orders()
{
 /*!Last Changed 18.01.2009 added memory deallocation functions for examples*/
	/*!Last Changed 05.08.2007*/
	int i, j;
	for (i = 0; i < ordercount; i++)
	 {
		 if (orders[i].command)
				 safe_free(orders[i].command);
		 if (orders[i].command_group)
				 safe_free(orders[i].command_group);
		 if (orders[i].command_subgroup)
				 safe_free(orders[i].command_subgroup);
		 if (orders[i].function_return)
				 safe_free(orders[i].function_return);
		 if (orders[i].grammar)
				 safe_free(orders[i].grammar);
		 if (orders[i].usage)
				 safe_free(orders[i].usage);
		 if (orders[i].help_messages)
				 safe_free(orders[i].help_messages);
   for (j = 0; j < orders[i].examplecount; j++)
     safe_free(orders[i].examples[j]);
	 }
	safe_free(orders);
}

/**
 * Prints the information about the command to the screen
 * @param[in] res Index of the command
 */
void print_order(int res)
{
 int i;
 char tmp[MAXLENGTH];
 printf("%s %s\n", m1284, orders[res].command);
 printf("%s %s\n", m1285, orders[res].command_group);
 printf("%s %s\n", m1286, orders[res].command_subgroup);
 printf("%s %s\n", m1287, orders[res].grammar);
	printf("%s %s\n", m1288, orders[res].function_return);
 printf("%s %s\n", m1289, orders[res].usage);
 printf("%s %s\n", m1290, orders[res].help_messages);
 for (i = 0; i < orders[res].examplecount; i++)
  {
   printf("%s %d\n", m1412, i + 1);
   sprintf(tmp, "examples/%s", orders[res].examples[i]);
   display_file(tmp);
  }
}

/**
 * Reads the command.txt for all commands. Constructor for orders array. Allocates memory for each command and its strings. Structure of the command.txt: \n
 * ...\n
 * n.Command \n
 * Command name \n
 * Command group \n
 * Command subgroup \n
 * Return of the command \n
 * Variables effected by this command \n
 * Grammar of the command \n
 * Usage of the command \n
 * Help message for the command \n
 * Example files of the command \n
 * ...
 * @warning If the line count of the order.txt is not a multiple of NUMBER_OF_INFORMATION_LINES, the commands will not be read.
 */
void initialize_orders()
{
	/*!Last Changed 07.01.2006*/
	FILE* infile;
	int length, i, j;
	char myline[READLINELENGTH], *p, tmpline[READLINELENGTH];
	length = file_length("command.txt");
	if (length == 0)
	 {
		 printf(m1318);
		 exit(0);
	 }
	if (length % NUMBER_OF_INFORMATION_LINES != 0)
	 {
		 printf(m1319);
			exit(0);
	 }
	ordercount = length / NUMBER_OF_INFORMATION_LINES;
	orders = safemalloc(ordercount * sizeof(Order), "initialize_orders", 16);
	infile = fopen("command.txt", "r");
	for (i = 0; i < ordercount; i++)
		 for (j = 0; j < NUMBER_OF_INFORMATION_LINES; j++)
			 {				 
     fgets(myline, READLINELENGTH, infile);
					p = strtok(myline, "\n");
				 switch (j)
			   {
					  case 1:orders[i].command = strcopy(orders[i].command, p);
								      break;
							case 2:if (p)
  								      orders[i].command_group = strcopy(orders[i].command_group, p);
								      else
															 orders[i].command_group = NULL;
								      break;
							case 3:if (p)
															 orders[i].command_subgroup = strcopy(orders[i].command_subgroup, p);
								      else
															 orders[i].command_subgroup = NULL;
								      break;
							case 4:if (p)
  								      orders[i].function_return = strcopy(orders[i].function_return, p);
							       else
															 orders[i].function_return = NULL; 
								      break;
       case 5:break;
							case 6:orders[i].grammar = strcopy(orders[i].grammar, p);
								      break;
							case 7:if (p)
															 orders[i].usage = strcopy(orders[i].usage, p);
													 else
															 orders[i].usage = NULL;
								      break;
							case 8:if (p)
  								      orders[i].help_messages = strcopy(orders[i].help_messages, p);
								      else
															 orders[i].help_messages = NULL;
								      break;
       case 9:if (p)
               {
                strcpy(tmpline, p);
                stringlist(tmpline, ";\n", orders[i].examples, &orders[i].examplecount);
               }
              else
                orders[i].examplecount = 0;
              break;
		    }
			 }
 fclose(infile);
}

/**
 * Creates a child node (allocates memory for it) for this command and inserts this child node into the children of the parent node. 
 * @param[in] root Parent node
 * @param[in] st Name of the command
 * @return Child node
 */
Command_nodeptr add_command_child(Command_nodeptr root, char* st)
{
	/*!Last Changed 23.10.2005*/
	Command_nodeptr tmp = root->child, before = NULL, newnode;
 newnode = safemalloc(sizeof(Command_node), "add_command_child", 2);
	newnode->command = strcopy(newnode->command, st);
	newnode->child = NULL;
	newnode->next = NULL;
 while (tmp)
		{
	  if (strIcmp(st, tmp->command) < 0)
		  	break;
			before = tmp;
			tmp = tmp->next;
		}
	if (before)
		{
  	newnode->next = before->next;
			before->next = newnode;
		}
	else
		{
			newnode->next = root->child;
			root->child = newnode;
		}
	return newnode;
}

/**
 * Checks if there exists a child node with a given name of the parent node.
 * @param[in] root Parent node
 * @param[in] st Name of the command
 * @return If there is a child with the given name, returns its address, otherwise returns NULL.
 */
Command_nodeptr child_of_command(Command_nodeptr root, char* st)
{
	/*!Last Changed 23.10.2005*/
	Command_nodeptr tmp;
	tmp = root->child;
	while (tmp)
	 {
		 if (strIcmp(st, tmp->command) == 0)
				 return tmp;
		 tmp = tmp->next;
	 }
	return NULL;
}

/**
 * Generates the whole command tree from the orders array.
 * @return The root node of the command tree
 * @warning If the command is !!!, it will not be added to the command tree.
 */
Command_nodeptr generate_command_tree()
{
	/*!Last Changed 23.10.2005*/
	int i;
	Command_nodeptr root, group, subgroup;
	root = safemalloc(sizeof(Command_node), "generate_command_tree", 3);
	root->child = NULL;
	root->next = NULL;
	root->command = strcopy(root->command, "COMMANDS");
	for (i = 0; i < ordercount; i++)
		 if (strcmp(orders[i].command, "!!!") != 0)
			 {
	  	 group = child_of_command(root, orders[i].command_group);
		  	if (!group)
			  	 group = add_command_child(root, orders[i].command_group);
		  	subgroup = child_of_command(group, orders[i].command_subgroup);
		  	if (!subgroup)
		  		 subgroup = add_command_child(group, orders[i].command_subgroup);
		  	add_command_child(subgroup, orders[i].command);
			 }
	return root;
}

/**
 * Prints current command name to the screen with appropriate number of tabs. Calls also itself to print its children and its children of children etc.
 * @param[in] root Current node
 * @param[in] level Level of the current node.
 */
void print_command_node(Command_nodeptr root, int level)
{
	/*!Last Changed 23.10.2005*/
	Command_nodeptr tmp;
	int i;
	for (i = 0; i < level; i++)
		 printf("\t");
	printf("%s\n", root->command);
 tmp = root->child;
	while (tmp)
	 {
		 print_command_node(tmp, level + 1);
		 tmp = tmp->next;
	 }
}

/**
 * Calls print_command_node with root node to print the whole command tree.
 * @param[in] root Root node of the command tree
 */
void print_command_tree(Command_nodeptr root)
{
	/*!Last Changed 23.10.2005*/
	print_command_node(root, 0);
}

/**
 * Frees memory allocated to the current node and its children and its children etc.
 * @param[in] root Current node
 */
void deallocate_command_tree(Command_nodeptr root)
{
	/*!Last Changed 23.10.2005*/
	if (root->child)
  	deallocate_command_tree(root->child);
	if (root->next)
		 deallocate_command_tree(root->next);
	safe_free(root->command);
	safe_free(root);
}
