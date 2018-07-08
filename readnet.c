#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "messages.h"
#include "libfile.h"
#include "libmisc.h"
#include "libstring.h"
#include "readnet.h"
#include "typedef.h"

char *net_words[4] = {"network", "probability", "var", "parents"};
char *xml_words[28] = {"ANALYSISNOTEBOOK", "NAME", "ROOT", "BNMODEL",
                    "STATICPROPERTIES", "FORMAT", "VALUE", "VERSION",
                    "CREATOR", "VARIABLES", "VAR", "TYPE",
                    "XPOS", "YPOS", "DESCRIPTION", "STATENAME",
                    "STRUCTURE", "ARC", "PARENT", "CHILD",
                    "DISTRIBUTIONS", "DIST", "PRIVATE", "DPIS",
                    "DPI", "CONDSET", "CONDELEM", "INDEXES"};
char *hugin_words[7] = {"net", "node", "label", "position", "states",
                      "potential", "data"};
char *bif_words[9] = {"network", "property", "variable", "type",
                    "discrete", "function", "max", "probability",
                    "table"};

/**
 * Reads a Bayesian network in the bif file format
 * @param[in] f_name Name of the Bayesian network file
 * @return Bayesian network structure allocated and the properties (nodes, arcs, values of the nodes etc.) has been set.
 */
bayesiangraphptr read_network_bif(char *f_name)
{
/*!Last Changed 09.02.2001 Commentlere dikkat*/
 FILE *myfile;
 char myline[READLINELENGTH];
 char *st;
 char st1[15];
 int i,pos = -1, ind = -1, parents = 0, currentp, res, prob, noparents;
 int where, table_index, Index, parent_count;
 int parent[MAXPARENT];
 bayesiangraphptr result;
 nodeptr current;
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = '{';
 st1[3] = '}';
 st1[4] = '=';
 st1[5] = '(';
 st1[6] = ')';
 st1[7] = '"';
 st1[8] = ';';
 st1[9] = '|';
 st1[10] = '\t';
 st1[11] = '[';
 st1[12] = ']';
 st1[13] = ',';
 st1[14] = '\0';
 if ((myfile = fopen(f_name, "r")) == NULL)
  {
   printf(m1308, f_name);
   return NULL;
  }
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   while (st)
    {
     switch (listindex(st, bif_words, 9)) 
      {
       case -1:if ((st[0] == '/') && (st[1] == '/'))
                 goto devam;
               switch (pos) 
                {
                 case -1:
                 case  0:result = create_graph(st);
                         pos = 1;
                         break;
                 case  1:return NULL;
                 case  2:break;
                 case  3:current = add_one_node(result, st);
                         break;
                 case  4:return NULL;
                 case  5:if (ind == -1)
                           ind = 0;
                         else
                           add_one_value(current, st);
                         break;
                 case  6:if (parents == 0)
                          {
                           currentp = node_in(result, st);
                           noparents = 1;
                           if (currentp == -1)
                             return NULL;
                           current = &(result->nodes[currentp]);
                           parents = 1;
                          }
                         else
                           if ((parents == 1) && (isalpha(st[0]) || (noparents)))
                            {
                             res = node_in(result, st);
                             if (res != -1)
                              {
                               add_parent_counts(result, current, res);
                               add_one_arc(result);
                               add_one_pointer(result, current, res);
                              }
                             else
                              {
                               prob = 1;
                               parent_count = 0;
                               for (i = 0; i < MAXPARENT; i++)
                                 parent[i] = 0;
                               parents = 2;
                               Index = 0;
                               alloc_conditional_table(current);
                              }
                            }
                           else
                             if (parents == 1)
                              {/*No parents*/
                               if (current->parents != 0)
                                 return NULL;
                               parents = 2;
                               prob = 0;
                               Index = 0;
                               alloc_conditional_table(current);
                              }
                         if (parents == 2)
                          {
                           if (prob == 0)/*No parents*/
                            {
                             current->table[0][Index] = (double) atof(st);
                             Index++;
                            }
                           else
                             if (parent_count < result->nodes[currentp].parents)
                              {
                               prob = 1;
                               where = parent_index(result, currentp, parent_count);
                               parent[parent_count] = listindex(st, result->nodes[where].values, result->nodes[where].value_count);
                               parent_count++;
                              }
                             else
                              {
                               if (prob == 1)
                                {
                                 Index = 0;
                                 table_index = table_pos(result->nodes[currentp], parent);
                                 current->table[table_index][0] = (double) atof(st);
                                 prob = 2;
                                }
                               else
                                {
                                 Index++;
                                 current->table[table_index][Index] = (double) atof(st);
                                 if (Index + 1 == result->nodes[currentp].value_count)
                                   parent_count = 0;
                                }
                              }
                          }
                         break;
               }
               break;
       case  0:pos = 0; /*network*/
               break;
       case  1:pos = 2; /*property*/
               break;
       case  2:pos = 3; /*variable*/
               break;
       case  3:pos = 4;/*type*/
               break;
       case  4:if (pos == 4) /*discrete*/
                {
                 ind = -1;
                 pos = 5;
                }
               break;
       case  5:pos = 2; /*function*/
               break;
       case  6:pos = 6; /*max*/
               break;
       case  7:pos = 6; /*probability*/
               parents = 0;
               break;
       case  8:noparents = 0; /*table*/
               break;
     }
     st = strtok(NULL, st1);
    }
devam:;
  }
 fclose(myfile);
 return result;
}

/**
 * Reads a Bayesian network in the hugian file format
 * @param[in] f_name Name of the Bayesian network file
 * @return Bayesian network structure allocated and the properties (nodes, arcs, values of the nodes etc.) has been set.
 */
bayesiangraphptr read_network_hugin(char *f_name)
{
/*!Last Changed 06.02.2001 Commentlere dikkat*/
 FILE *myfile;
 bayesiangraphptr result = create_graph(file_only_name(f_name));
 char myline[READLINELENGTH];
 char *st;
 char st1[12];
 int pos = -1, ind = -1, child = -1, pindex, row = -1, col;
 nodeptr current;
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = '{';
 st1[3] = '}';
 st1[4] = '=';
 st1[5] = '(';
 st1[6] = ')';
 st1[7] = '"';
 st1[8] = ';';
 st1[9] = '|';
 st1[10] = '\t';
 st1[11] = '\0';
 if ((myfile = fopen(f_name, "r")) == NULL)
  {
   printf(m1308, f_name);
   return NULL;
  }
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   while (st)
    {
     switch (listindex(st, hugin_words, 7)) 
      {
       case -1:if (st[0] == '%')
                 goto devam;
               switch (pos) {
                 case -1:return NULL;
                 case  0:
                 case  2:break;
                 case  1:current = add_one_node(result, st);
                         break;
                 case  3:current->xpos = atoi(st);
                         pos++;
                         break;
                 case  4:current->ypos = atoi(st);
                         break;
                 case  5:add_one_value(current, st);
                         break;
                 case  6:if (ind == -1)
                          {
                           child = node_in(result, st);
                           if (child == -1)
                             return NULL;
                           ind++;
                          }
                         else
                          {
                           pindex = node_in(result, st);
                           if (pindex == -1)
                             return NULL;
                           add_parent_counts(result, &result->nodes[child],pindex);
                           add_one_arc(result);
                           add_one_pointer(result, &result->nodes[child],pindex);
                          }
                         break;
                 case  7:if (row == -1)
                          {
                           current = &result->nodes[child];
                           alloc_conditional_table(current);
                           row = 0;
                           col = 0;
                          }
                         else
                          {
                           current->table[row][col] = (double) atof(st);
                           col++;
                           if (col == current->value_count)
                            {
                             row++;
                             col = 0;
                            }
                          }
                         break;
               }
               break;
       case  0:pos = 0; /*net*/
               break;
       case  1:pos = 1; /*node*/
               break;
       case  2:pos = 2; /*label*/
               break;
       case  3:pos = 3; /*position*/
               break;
       case  4:pos = 5; /*states*/
               break;
       case  5:pos = 6; /*potential*/
               ind = -1;
               break;
       case  6:pos = 7; /*data*/
               row = -1;
               break;
     }
     st = strtok(NULL,st1);
    }
devam:;
  }
 fclose(myfile);
 return result;
}

/**
 * Reads a Bayesian network in the XML file format
 * @param[in] f_name Name of the Bayesian network file
 * @return Bayesian network structure allocated and the properties (nodes, arcs, values of the nodes etc.) has been set.
 */
bayesiangraphptr read_network_xml(char *f_name)
{
/*!Last Changed 28.12.2000*/
 FILE *myfile;
 bayesiangraphptr result = NULL;
 char myline[READLINELENGTH];
 char *st;
 char st1[10];
 int anydata = 0, pos = -1, temp, childindex = -1, cond = -1, pcount = -1, pindex;
 int parents[MAXPARENT], parent_values[MAXPARENT];
 nodeptr current;
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = '<';
 st1[3] = '>';
 st1[4] = ':';
 st1[5] = '"';
 st1[6] = '/';
 st1[7] = '=';
 st1[8] = '\t';
 st1[9] = '\0';
 if ((myfile = fopen(f_name,"r")) == NULL)
  {
   printf(m1308, f_name);
   return NULL;
  }
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   while (st)
    {
     switch (listindex(st, xml_words, 28)) 
      {
       case -1:switch (pos){
                 case -1:case  0:case  3:case  5:case  6:
                 case  8:case 10:case 12:case 13:case 14:
                 case 20:case 22:case 23:case 24:case 27:
                 case 28:case 29:case 31:case 33:case 35:
                 case 36:case 37:case 38:case 39:case 42:
                 case 43:case 44:
                 case 45:return NULL;
                 case  1:case  2:case  7:case 11:
                 case 19:anydata = 1;
                         break;
                 case  4:if (!result)
                           result = create_graph(st);
                         break;
                 case  9:if (strcmp(st,"0.2") != 0)
                           return NULL;
                         break;
                 case 15:current = add_one_node(result,st);
                         break;
                 case 16:
                 case 30:if (strcmp(st,"discrete") != 0)
                           return NULL;
                         break;
                 case 17:current->xpos = atoi(st);
                         break;
                 case 18:current->ypos = atoi(st);
                         break;
                 case 21:add_one_value(current, st);
                         break;
                 case 25:pindex = node_in(result, st);
                         if (pindex == -1)
                           return NULL;
                         break;
                 case 26:temp = node_in(result, st);
                         if (temp != -1)
                          {
                           add_parent_counts(result, &result->nodes[temp], pindex);
                           add_one_arc(result);
                           add_one_pointer(result, &result->nodes[temp], pindex);
                          }
                         else
                           return NULL;
                         break;
                 case 32:temp = node_in(result, st);
                         if (temp != -1)
                          {
                           current = &result->nodes[temp];
                           alloc_conditional_table(current);
                          }
                         else
                           return NULL;
                         break;
                 case 34:current->table[0][childindex] = (double) atof(st);
                         childindex++;
                         break;
                 case 40:temp = node_in(result, st);
                         if (temp != -1)
                          {
                           parents[cond] = temp;
                           cond++;
                          }
                         else
                           return NULL;
                         break;
                 case 41:if (pcount < current->parents)
                          {
                           parent_values[pcount] = listindex(st, result->nodes[parents[pcount]].values, result->nodes[parents[pcount]].value_count);
                           pcount++;
                           childindex = 0;
                          }
                         else
                          {
                           if (childindex == 0)
                             pindex = table_pos(*current, parent_values);
                           current->table[pindex][childindex] = (double) atof(st);
                           childindex++;
                          }
                         break;
               };
               break;
       case  0:if (pos == -1)/*ANALYSISNOTEBOOK basliyor*/
                 pos++;
               else
                 if (pos == 44)/*ANALYSISNOTEBOOK bitti*/
                   pos++;
                 else
                   return NULL;
               break;  
       case  1:if (in_list(pos, 5, 0, 3, 14, 31, 39))/*NAME*/
                 pos++;
               else
                 return NULL;
               break;
       case  2:
       case  3:if (((pos == 1) || (pos == 2)) && (anydata))/*ROOT,BNMODEL*/
                {
                 anydata = 0;
                 pos++;
                }
               else
                 if (pos == 43)/*BNMODEL bitti*/
                   pos++;
                 else
                   return NULL;
               break;
       case  4:if (in_list(pos, 2, 4, 11))/*STATICPROPERTIES*/
                 pos++;
               else
                 return NULL;
               break;
       case  5:
       case  7:
       case  8:if (in_list(pos, 3, 5, 7, 9))/*FORMAT,VERSION,CREATOR*/
                 pos++;
               else
                 return NULL;
               break;
       case  6:if (in_list(pos, 3, 6, 8, 10))/*VALUE*/
                {
                 anydata = 0;
                 pos++;
                }
               else
                 return NULL;
               break;
       case  9:if (pos == 12)/*VARIABLES basliyor*/
                 pos++;
               else
                 if (pos == 13)/*VARIABLES bitti*/
                   pos = 22;
                 else
                   return NULL;
               break;
       case 10:if (pos == 13)/*VAR basliyor*/
                 pos++;
               else
                 if (pos == 21)/*VAR bitti*/
                   pos = 13;
                 else
                   return NULL;
               break;
       case 11:if (pos == 15)/*TYPE*/
                 pos++;
               else
                 if (pos == 29)
                   pos++;
                 else
                   return NULL;
               break;
       case 12:if (pos == 16)/*XPOS*/
                 pos++;
               else
                 return NULL;
               break;
       case 13:if (pos == 17)/*YPOS*/
                 pos++;
               else
                 return NULL;
               break;
       case 14:if (in_list(pos, 2, 18, 19))/*DESCRIPTION*/
                 pos++;
               else
                 return NULL;
               break;
       case 15:if (pos == 20)/*STATENAME*/
                 pos++;
               else
                 if (pos == 21)
                   ;
                 else
                   return NULL;
               break;
       case 16:if (pos == 22)/*STRUCTURE basliyor*/
                 pos++;
               else
                 if (pos == 26)/*STRUCTURE bitti*/
                   pos++;
                 else
                   return NULL;
               break;
       case 17:if (pos == 23)/*ARC*/
                 pos++;
               else
                 if (pos == 26)
                   pos = 24;
                 else
                   return NULL;
               break;
       case 18:if (pos == 24)/*PARENT*/
                 pos++;
               else
                 return NULL;
               break;
       case 19:if (pos == 25)/*CHILD*/
                 pos++;
               else
                 return NULL;
               break;
       case 20:if (pos == 27)/*DISTRIBUTIONS basliyor*/
                 pos++;
               else
                 if (pos == 37)/*DISTRIBUTIONS bitti*/
                   pos = 43;
                 else
                   return NULL;
               break;
       case 21:if (in_list(pos, 2, 28, 37))/*DIST basliyor*/
                 pos = 29;
               else
                 if (pos == 36)/*DIST bitti*/
                   pos++;
                 else
                   return NULL;
               break;
       case 22:if (pos == 30)/*PRIVATE*/
                 pos++;
               else
                 return NULL;
               break;
       case 23:if (pos == 32)/*DPIS basliyor*/
                 pos++;
               else
                 if (in_list(pos, 2, 35, 42))/*DPIS bitti*/
                   pos = 36;
                 else
                   return NULL;
               break;
       case 24:if (in_list(pos, 2, 33, 42))/*DPI basliyor*/
                {
                 pos = 34;
                 childindex = 0;
                }
               else
                 if (in_list(pos, 2, 34, 41)) /*DPI bitti*/
                   pos++;
                 else
                   return NULL;
               break;
       case 25:if (pos == 30)/*CONDSET*/
                {
                 pos = 38;
                 cond = 0;
                }
               else
                 if (pos == 40)
                   pos = 30;
                 else
                   return NULL;
               break;
       case 26:if (pos == 38)/*CONDELEM*/
                 pos++;
               else
                 if (pos == 40)
                   pos = 39;
                 else
                   return NULL;
               break;
       case 27:if ((pos == 34) && (cond > 0))/*INDEXES*/
                {
                 pcount = 0;
                 pos = 41;
                }
               else
                 return NULL;
               break;
     }
     st = strtok(NULL, st1);
    }
  }
 fclose(myfile);
 return result;
}

/**
 * Reads a Bayesian network in the net file format
 * @param[in] f_name Name of the Bayesian network file
 * @return Bayesian network structure allocated and the properties (nodes, arcs, values of the nodes etc.) has been set.
 */
bayesiangraphptr read_network_net(char *f_name)
{
/*!Last Changed 24.12.2000*/
 FILE *myfile;
 bayesiangraphptr result;
 nodeptr current;
 int pos, var, parents, prob, res, parent_count, i, Index, where, table_index, currentp;
 int parent[MAXPARENT];
 char myline[READLINELENGTH];
 char *st;
 char st1[7];
 st1[0] = ' ';
 st1[1] = '\n';
 st1[2] = ':';
 st1[3] = '(';
 st1[4] = ')';
 st1[5] = '\t';
 st1[6] = '\0';
 if ((myfile = fopen(f_name, "r")) == NULL)
  {
   printf(m1308, f_name);
   return NULL;
  }
 pos = -1;
 var = 0;
 parents = 0;
 while (fgets(myline, READLINELENGTH, myfile))
  {
   st = strtok(myline, st1);
   while (st)
    {
     switch (listindex(st, net_words, 4)) 
      {
       case -1:switch (pos)
                {
                 case -1:return NULL;
                 case  0:result = create_graph(st);
                         break;
                 case  1:return NULL;
                 case  2:if (var == 0)
                          {
                           current = add_one_node(result, st);
                           var = 1;
                          }
                         else
                           add_one_value(current, st);
                         break;
                 case  3:if (parents == 0)
                          {
                           currentp = node_in(result, st);
                           if (currentp == -1)
                             return NULL;
                           current = &(result->nodes[currentp]);
                           parents = 1;
                          }
                         else
                           if ((parents == 1) && (isalpha(st[0])))
                            {
                             res = node_in(result, st);
                             if (res != -1)
                              {
                               add_parent_counts(result, current, res);
                               add_one_arc(result);
                               add_one_pointer(result, current, res);
                              }
                             else
                              {
                               prob = 1;
                               parent_count = 0;
                               for (i = 0; i < MAXPARENT; i++)
                                 parent[i] = 0;
                               parents = 2;
                               Index = 0;
                               alloc_conditional_table(current);
                              }
                            }
                           else
                             if (parents == 1)
                              {/*No parents*/
                               if (current->parents != 0)
                                 return NULL;
                               parents = 2;
                               prob = 0;
                               Index = 0;
                               alloc_conditional_table(current);
                              }
                         if (parents == 2)
                          {
                           if (prob == 0)/*No parents*/
                            {
                             current->table[0][Index] = (double) atof(st);
                             Index++;
                            }
                           else
                             if (isalpha(st[0]))
                              {
                               prob = 1;
                               where = parent_index(result, currentp, parent_count);
                               parent[parent_count] = listindex(st, result->nodes[where].values, result->nodes[where].value_count);
                               parent_count++;
                              }
                             else
                              {
                               if (prob == 1)
                                {
                                 parent_count = 0;
                                 Index = 0;
                                 table_index = table_pos(result->nodes[currentp], parent);
                                 current->table[table_index][0] = (double) atof(st);
                                 prob = 2;
                                }
                               else
                                {
                                 Index++;
                                 current->table[table_index][Index] = (double) atof(st);
                                }
                              }
                          }
                         break;
                 default:return NULL;
               }
               break;
       case  0:if (pos == -1)
                 pos = 0;
               else
                 return NULL;
               break;
       case  1:if (pos == 0)
                 pos = 1;
               else
                 return NULL;
               break;
       case  2:if (pos == 1)
                 pos = 2;
               else
                 if (pos == 2)
                   var = 0;
                 else
                   return NULL;
               break;
       case  3:if (pos == 2)
                 pos = 3;
               else
                 if (pos == 3)
                   parents = 0;
               break;
       default:return NULL;
     }
     st = strtok(NULL, st1);
    }
  }
 fclose(myfile);
 return result;
}
