#include <stdio.h>
#include <stdlib.h>
#include "libmemory.h"
#include "libmisc.h"
#include "savenet.h"

/**
 * Save the Bayesian graph in the net file format.
 * @param[in] mygraph Bayesian graph
 * @param[in] file_name Output file name
 */
void save_graph_net(bayesiangraphptr mygraph, char *file_name)
{
 /*!Last Changed 06.02.2001*/
 FILE *myfile;
 int i, j, k, *list, *valuelist;
 myfile = fopen(file_name, "w");
	if (!myfile)
		 return;
 fprintf(myfile, "(network %s:probability)\n", mygraph->name);
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "(var %s (", mygraph->nodes[i].name);
   for (j = 0; j < mygraph->nodes[i].value_count; j++)
     fprintf(myfile, "%s ", mygraph->nodes[i].values[j]);
   fprintf(myfile, "))\n");
  }
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "parents %s ", mygraph->nodes[i].name);
   if (mygraph->nodes[i].parents == 0)/*No parents*/
    {
     fprintf(myfile, "(");
     for (j = 0; j < mygraph->nodes[i].value_count; j++)
       fprintf(myfile, "%.4f ", mygraph->nodes[i].table[0][j]);
     fprintf(myfile, ")\n");
    }
   else
    {
     list = find_parents(mygraph, i);/*Print parents*/
     fprintf(myfile, "(");
     for (j = 0; j < mygraph->nodes[i].parents; j++)
       fprintf(myfile, "%s ", mygraph->nodes[list[j]].name);
     fprintf(myfile, ")(\n");
     for (j = 0; j < mygraph->nodes[i].table_count; j++)
      {
       fprintf(myfile, "(");
       valuelist = table_list(mygraph->nodes[i], j);
       for (k = 0; k < mygraph->nodes[i].parents; k++)
         fprintf(myfile, "%s ", mygraph->nodes[list[k]].values[valuelist[k]]);
       fprintf(myfile, ")");
       for (k = 0; k < mygraph->nodes[i].value_count; k++)
         fprintf(myfile, "%.4f ", mygraph->nodes[i].table[j][k]);
       fprintf(myfile, ")\n");
       safe_free(valuelist);
      }
     fprintf(myfile, ")\n");
     safe_free(list);
    }
  }
 fclose(myfile);
}

/**
 * Save the Bayesian graph in the xml file format.
 * @param[in] mygraph Bayesian graph
 * @param[in] file_name Output file name
 */
void save_graph_xml(bayesiangraphptr mygraph, char *file_name)
{
/*!Last Changed 28.12.2000*/
 FILE *myfile;
 int i, j, k, *list, *valuelist;
 myfile = fopen(file_name, "w");
	if (!myfile)
		 return;
 fprintf(myfile, "<ANALYSISNOTEBOOK NAME=\"%s\"\n", mygraph->name);
 fprintf(myfile, "                  ROOT=\"%s\">\n", mygraph->name);
 fprintf(myfile, "   <BNMODEL NAME=\"%s\">\n", mygraph->name);
 fprintf(myfile, "      <STATICPROPERTIES>\n");
 fprintf(myfile, "         <FORMAT VALUE=\"MSR DTAS XML\"/>\n");
 fprintf(myfile, "         <VERSION VALUE=\"0.2\"/>\n");
 fprintf(myfile, "         <CREATOR VALUE=\"Bayesian Learner\"/>\n");
 fprintf(myfile, "      </STATICPROPERTIES>\n");
 fprintf(myfile, "      <VARIABLES>\n");
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "         <VAR NAME=\"%s\" TYPE=\"discrete\" XPOS=\"%d\" YPOS=\"%d\">\n", mygraph->nodes[i].name, mygraph->nodes[i].xpos, mygraph->nodes[i].ypos);
   fprintf(myfile, "            <DESCRIPTION></DESCRIPTION>\n");
   for (j = 0; j < mygraph->nodes[i].value_count; j++)
     fprintf(myfile, "            <STATENAME>%s</STATENAME>\n", mygraph->nodes[i].values[j]);
   fprintf(myfile, "         </VAR>\n");
  }
 fprintf(myfile, "      </VARIABLES>\n");
 fprintf(myfile, "      <STRUCTURE>\n");
 for (i = 0; i < mygraph->nodecount; i++)
   for (j = 0; j < mygraph->nodes[i].parents; j++)
    {
     k = parent_index(mygraph, i, j);
     fprintf(myfile, "         <ARC PARENT=\"%s\" CHILD=\"%s\"/>\n", mygraph->nodes[k].name, mygraph->nodes[i].name);
    }
 fprintf(myfile, "      </STRUCTURE>\n");
 fprintf(myfile, "      <DISTRIBUTIONS>\n");
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "         <DIST TYPE=\"discrete\">\n");
   if (mygraph->nodes[i].parents > 0)
    {
     fprintf(myfile, "            <CONDSET>\n");
     for (j = 0; j < mygraph->nodes[i].parents; j++)
      {
       k = parent_index(mygraph, i, j);
       fprintf(myfile, "               <CONDELEM NAME=\"%s\"/>\n", mygraph->nodes[k].name);
      }
     fprintf(myfile, "            </CONDSET>\n");
    }
   fprintf(myfile, "            <PRIVATE NAME=\"%s\"/>\n", mygraph->nodes[i].name);
   fprintf(myfile, "            <DPIS>\n");
   if (mygraph->nodes[i].parents == 0)
    {
     fprintf(myfile, "               <DPI>");
     for (j = 0; j < mygraph->nodes[i].value_count; j++)
       fprintf(myfile, " %.4f", mygraph->nodes[i].table[0][j]);
     fprintf(myfile, " <\\DPI>\n");
    }
   else
    {
     list = find_parents(mygraph, i);/*Print parents*/
     for (j = 0; j < mygraph->nodes[i].table_count; j++)
      {
       fprintf(myfile, "               <DPI INDEXES=\"");
       valuelist = table_list(mygraph->nodes[i], j);
       for (k = 0; k < mygraph->nodes[i].parents; k++)
         fprintf(myfile, "%s ", mygraph->nodes[list[k]].values[valuelist[k]]);
       fprintf(myfile, "\">");
       for (k = 0; k < mygraph->nodes[i].value_count; k++)
         fprintf(myfile, " %.4f ", mygraph->nodes[i].table[j][k]);
       fprintf(myfile, "</DPI>\n");  
       safe_free(valuelist);
      }
     safe_free(list);
    }
   fprintf(myfile, "            </DPIS>\n");
   fprintf(myfile, "         </DIST>\n");
  }
 fprintf(myfile, "      </DISTRIBUTIONS>\n");
 fprintf(myfile, "   </BNMODEL>\n");
 fprintf(myfile, "</ANALYSISNOTEBOOK>\n");
 fclose(myfile);
}

/**
 * Save the Bayesian graph in the hugin file format.
 * @param[in] mygraph Bayesian graph
 * @param[in] file_name Output file name
 */
void save_graph_hugin(bayesiangraphptr mygraph, char *file_name)
{
 /*!Last Changed 11.02.2001*/
 FILE *myfile;
 int i, j, k;
 myfile = fopen(file_name, "w");
	if (!myfile)
		 return;
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "node %s\n", mygraph->nodes[i].name);
   fprintf(myfile, "{ label = \"%s\";\n", mygraph->nodes[i].name);
   fprintf(myfile, "position = (%d %d);\n", mygraph->nodes[i].xpos, mygraph->nodes[i].ypos);
   fprintf(myfile, "states = (");
   for (j = 0; j < mygraph->nodes[i].value_count; j++)
     fprintf(myfile, "\"%s\" ", mygraph->nodes[i].values[j]);
   fprintf(myfile, ");\n}\n\n");
  }
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "potential (%s", mygraph->nodes[i].name);
   if (mygraph->nodes[i].parents > 0)
     fprintf(myfile, " |");
   for (j = 0; j < mygraph->nodes[i].parents; j++)
    {
     k = parent_index(mygraph, i, j);
     fprintf(myfile, "%s ", mygraph->nodes[k].name);
    }
   fprintf(myfile, ")\n{ data = \n");
   for (j = 0; j < mygraph->nodes[i].table_count; j++)
    {
     fprintf(myfile, "( ");
     for (k = 0; k < mygraph->nodes[i].value_count; k++)
       fprintf(myfile, "%4.f ", mygraph->nodes[i].table[j][k]);
     fprintf(myfile, ")\n");
    }
   fprintf(myfile, ";\n}\n\n");
  }
 fclose(myfile);
}

/**
 * Save the Bayesian graph in the bif file format.
 * @param[in] mygraph Bayesian graph
 * @param[in] file_name Output file name
 */
void save_graph_bif(bayesiangraphptr mygraph,char *file_name)
{
 /*!Last Changed 11.02.2001*/
 FILE *myfile;
 int i, j, k, *list, *valuelist;
 myfile = fopen(file_name, "w");
	if (!myfile)
		 return;
 fprintf(myfile, "network %s { }\n\n", mygraph->name);
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "variable %s {\n", mygraph->nodes[i].name);
   fprintf(myfile, "type discrete [%d] { ", mygraph->nodes[i].value_count);
   for (j = 0; j < mygraph->nodes[i].value_count - 1; j++)
     fprintf(myfile, "%s, ", mygraph->nodes[i].values[j]);
   fprintf(myfile, "%s };\n}\n\n", mygraph->nodes[i].values[j]);
  }
 for (i = 0; i < mygraph->nodecount; i++)
  {
   fprintf(myfile, "probability (%s", mygraph->nodes[i].name);
   if (mygraph->nodes[i].parents)
    {
     fprintf(myfile, " | ");
     for (j = 0; j < mygraph->nodes[i].parents; j++)
      {
       k = parent_index(mygraph, i, j);
       fprintf(myfile, "%s ", mygraph->nodes[k].name);
      }
    }
   fprintf(myfile, ") {\n");
   if (!mygraph->nodes[i].parents)
    {
     fprintf(myfile, "  table ");
     for (j = 0; j < mygraph->nodes[i].value_count - 1; j++)
       fprintf(myfile, " %4.f, ", mygraph->nodes[i].table[0][j]);
     fprintf(myfile, " %4.f;\n", mygraph->nodes[i].table[0][j]);
    }
   else
    {
     list = find_parents(mygraph, i);
     for (j = 0; j < mygraph->nodes[i].table_count; j++)
      {
       fprintf(myfile, "(");
       valuelist = table_list(mygraph->nodes[i], j);
       for (k = 0; k < mygraph->nodes[i].parents - 1; k++)
         fprintf(myfile, " %s,", mygraph->nodes[list[k]].values[valuelist[k]]);
       fprintf(myfile, "%s ) ", mygraph->nodes[list[k]].values[valuelist[k]]);
       for (k = 0; k < mygraph->nodes[i].value_count - 1; k++)
         fprintf(myfile, "%.4f, ", mygraph->nodes[i].table[j][k]);
       fprintf(myfile, "%.4f;\n", mygraph->nodes[i].table[j][k]);
       safe_free(valuelist);
      }
     safe_free(list);
    }
   fprintf(myfile, "}\n\n");
  }
 fclose(myfile); 
}
