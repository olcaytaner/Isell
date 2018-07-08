#include <stdio.h>
#include "libfile.h"
#include "libmemory.h"
#include "libmisc.h"
#include "libstring.h"
#include "messages.h"
#include "xmlparser.h"

/**
 * Creates an empty xml document. 
 * @param[in] filename Name of the xml file
 * @return Empty xml document. Xml file is not parsed yet.
 */
Xmldocumentptr xml_document(char* filename)
{
 Xmldocumentptr xml;
 if (!fileexists(filename))
   return NULL;
 xml = safemalloc(sizeof(Xmldocument), "xml_document", 2);
 xml->filename = strcopy(xml->filename, filename);
 xml->root = NULL;
 return xml;
}

/**
 * Reads a token character by character from xml file.
 * @param[in] infile File handle
 * @param[in] prevchar Previous character read
 * @param[in] nextchar The character after the token
 * @param[in] space_allowed If BOOLEAN_TRUE, space is allowed in the token, otherwise it is not allowed
 * @return Token read
 */
char* xml_read_token(FILE* infile, char prevchar, char* nextchar, int space_allowed)
{
 char buffer[MAXTOKENLENGTH];
 char* token = NULL;
 int i;
 char ch = prevchar;
 i = 0;
 while ((ch != '\'') && (ch != '\"') && (ch != '=') && (ch != ' ' || space_allowed) && (ch != '/') && (ch != EOF) && (ch != '<') && (ch != '>'))
  {
   buffer[i] = ch;
   i++;
   ch = fgetc(infile);
  }
 buffer[i] = '\0';
 *nextchar = ch;
 token = strcopy(token, buffer);
 return token;
}

/**
 * Parses a tag like \<mytag> or \</mytag> 
 * @param[in] infile File handle
 * @param[out] tokentype Type of token read. If it is a tag like \<mytag> then it has a value of XML_OPENING_TAG, otherwise XML_CLOSING_TAG. IF not successful returns XML_END.
 * @return Token read
 */
char* xml_parse_tag(FILE* infile, XML_TOKEN_TYPE* tokentype)
{
 char* token;
 char ch;
 ch = fgetc(infile);
 if (ch == '/')
  {
   *tokentype = XML_CLOSING_TAG_WITHOUT_ATTRIBUTES;
   ch = fgetc(infile);
  }
 else
   *tokentype = XML_OPENING_TAG_WITH_ATTRIBUTES;
 token = xml_read_token(infile, ch, &ch, BOOLEAN_FALSE);
 if (ch == '>' && *tokentype == XML_OPENING_TAG_WITH_ATTRIBUTES)
   *tokentype = XML_OPENING_TAG_WITHOUT_ATTRIBUTES;
 if (*tokentype == XML_CLOSING_TAG_WITHOUT_ATTRIBUTES && ch != '>')
  {
   printf(x1001);
   *tokentype = XML_END;
   return NULL;
  }
 else
   return token;
}

/**
 * Parses an attribute value like "attribute value" or 'attribute value'
 * @param[in] infile Fiel handle
 * @param[out] tokentype XML_ATTRIBUTE_VALUE if successful, otherwise XML_END.
 * @return Attribute value read
 */
char* xml_parse_attribute_value(FILE* infile, XML_TOKEN_TYPE* tokentype)
{
 char* token;
 char ch;
 ch = fgetc(infile);
 if (ch == '\'' || ch == '\"')
  {
   *tokentype = XML_ATTRIBUTE_VALUE;
   return NULL;
  }
 token = xml_read_token(infile, ch, &ch, BOOLEAN_TRUE);
 if (ch != '\'' && ch != '\"')
  {
   printf(x1002);
   *tokentype = XML_END;
   return NULL;
  }
 *tokentype = XML_ATTRIBUTE_VALUE;
 return token;
}

/**
 * Parses a tag like />
 * @param[in] infile File handle
 * @param[out] tokentype XML_CLOSING_TAG if read is successful, otherwise XML_END.
 * @return NULL
 */
char* xml_parse_empty_tag(FILE* infile, XML_TOKEN_TYPE* tokentype)
{
 char ch;
 ch = fgetc(infile);
 if (ch != '>')
  {
   printf(x1001);
   *tokentype = XML_END;
  }
 else
   *tokentype = XML_CLOSING_TAG_WITH_ATTRIBUTES;
 return NULL;
}

/**
 * Gets next token from file.
 * @param[in] infile File handle
 * @param[out] tokentype Type of toke read
 * @return Token read. If not successful, returns NULL.
 */
char* xml_get_next_token(FILE* infile, XML_TOKEN_TYPE* tokentype)
{
 char ch;
 char* token;
 /*Skip spaces*/
 ch = fgetc(infile);
 while (ch == ' ' || ch == '\t' || ch == '\n')
   ch = fgetc(infile);
 switch (ch){
   case  '<':return xml_parse_tag(infile, tokentype);
             break;
   case '\"':
   case '\'':return xml_parse_attribute_value(infile, tokentype);
             break;
   case  '/':return xml_parse_empty_tag(infile, tokentype);
             break;
   case  '=':*tokentype = XML_EQUAL;             
             break;
   case  '>':*tokentype = XML_OPENING_TAG_FINISH;
             return NULL;
             break;
   case  EOF:*tokentype = XML_END;
             return NULL;
   default  :token = xml_read_token(infile, ch, &ch, BOOLEAN_TRUE);
             *tokentype = XML_TEXT;
             ungetc(ch, infile);
             return token;
             break;
 }
 return NULL;
}

/**
 * Constructor for xml element. Allocates memory and initializes an element.
 * @param[in] name Name of the element
 * @param[in] has_attributes If BOOLEAN_TRUE, the element will have attributes, otherwise it won't have.
 * @return Allocated and initialized element
 */
Xmlelementptr xmlelement(char* name, Xmlelementptr parent, int has_attributes)
{
 Xmlelementptr element;
 element = safemalloc(sizeof(Xmlelement), "xmlelement", 2);
 if (has_attributes)
   element->attributes = link_list();
 else
   element->attributes = NULL;
 element->parent = parent;
 element->sibling = NULL;
 element->child = NULL;
 element->pcdata = NULL;
 element->name = strcopy(element->name, name);
 return element;
}

/**
 * Sets the value of an attribute to a given value
 * @param[in] element Xml element
 * @param[in] attribute_name Name of the attribute
 * @param[in] attribute_value New attribute value
 */
void xml_set_attribute_value(Xmlelementptr element, char* attribute_name, char* attribute_value)
{
 Listitemptr listnode;
 Xmlattributeptr att;
 if (element->attributes)
  {
   listnode = element->attributes->head;
   while (listnode && strIcmp(attribute_name, ((Xmlattributeptr)(listnode->item))->name) != 0)
     listnode = listnode->next;
   if (listnode)
    {
     att = (Xmlattributeptr)(listnode->item);
     safe_free(att->value);
     att->value = strcopy(att->value, attribute_value);
    }
  }
}

/**
 * Finds the attribute with the given name of an Xml element
 * @param[in] element Xml element
 * @param[in] attribute_name Name of the attribute
 * @return If the Xml element has such an attribute returns its value, otherwise it returns NULL
 */
char* xml_get_attribute_value(Xmlelementptr element, char* attribute_name)
{
 Listitemptr listnode;
 if (element->attributes)
  {
   listnode = element->attributes->head;
   while (listnode && strIcmp(attribute_name, ((Xmlattributeptr)(listnode->item))->name) != 0)
     listnode = listnode->next;
   if (listnode)
     return ((Xmlattributeptr)(listnode->item))->value;
  }
 return NULL;
}

/**
 * Constructor for xml attribute. Allocates memory and initializes the attribute.
 * @param[in] name Name of the attribute
 * @return Allocated and initialized attribute
 */
Xmlattributeptr xmlattribute(char* name)
{
 Xmlattributeptr attribute;
 attribute = safemalloc(sizeof(Xmlattribute), "xmlattribute", 2);
 attribute->name = strcopy(attribute->name, name);
 return attribute;
}

/**
 * Prints single xml element to the output xml file
 * @param[in] handle File handle
 * @param[in] element Element to be written
 * @param[in] level Level of the element  
 */
void xml_print_element(FILE* handle, Xmlelementptr element, int level)
{
 int i;
 Listitemptr listnode;
 Xmlattributeptr attribute;
 for (i = 0; i < level; i++)
   fprintf(handle, "\t");
 fprintf(handle, "<");
 fprintf(handle, "%s", element->name);
 if (element->attributes != NULL)
  {
   listnode = element->attributes->head;
   while (listnode)
    {
     attribute = (Xmlattributeptr) listnode->item;
     fprintf(handle, " %s=\"%s\"", attribute->name, attribute->value);
     listnode = listnode->next;
    }
  }
 if (element->child == NULL)
   if (element->pcdata == NULL)
     fprintf(handle, "/>\n");
   else
     fprintf(handle, ">%s</%s>\n", element->pcdata, element->name);
 else
  { 
   fprintf(handle, ">\n");
   xml_print_element(handle, element->child, level + 1);
   for (i = 0; i < level; i++)
     fprintf(handle, "\t");
   fprintf(handle, "</%s>\n", element->name);
  }
 if (element->sibling)
   xml_print_element(handle, element->sibling, level);
}

/**
 * Prints xml parse tree to an xml file
 * @param[in] document Xml parse tree
 * @param[in] filename Output file name
 */
void xml_print_tree(Xmldocumentptr document, char* filename)
{
 FILE* outfile;
 outfile = fopen(filename, "w");
 xml_print_element(outfile, document->root, 0);
 fclose(outfile);
}

/**
 * Finds the number of children of Xml element
 * @param[in] element Xml element
 * @return Number of child elements of the Xml element
 */
int xml_number_of_children(Xmlelementptr element)
{
 int result = 0;
 Xmlelementptr childnode = element->child;
 while (childnode)
  {
   result++;
   childnode = childnode->sibling;
  }
 return result;
}

/**
 * Finds the number of children of Xml element with a given attribute name and an attribute value
 * @param[in] element Xml element
 * @param[in] attribute_name Name of the attribute
 * @param[in] attribute_value Value of the attribute
 * @return Number of child elements
 */
int xml_number_of_children_with_specific_attribute_value(Xmlelementptr element, char* attribute_name, char* attribute_value)
{
 int result = 0;
 Xmlelementptr childnode = element->child;
 while (childnode)
  {
   if (strIcmp(xml_get_attribute_value(childnode, attribute_name), attribute_value) == 0)
     result++;
   childnode = childnode->sibling;
  }
 return result;
}

/**
 * Finds the child of Xml element with a given attribute name and an attribute value
 * @param[in] element Xml element
 * @param[in] attribute_name Name of the attribute
 * @param[in] attribute_value Value of the attribute
 * @return The pointer to the child
 */
Xmlelementptr xml_get_child_with_specific_attribute_value(Xmlelementptr element, char* attribute_name, char* attribute_value)
{
 Xmlelementptr childnode = element->child;
 while (childnode)
  {
   if (strIcmp(xml_get_attribute_value(childnode, attribute_name), attribute_value) == 0)
     return childnode;
   childnode = childnode->sibling;
  }
 return NULL;
}

/**
 * Finds the child of Xml element with a given name
 * @param[in] element Xml element
 * @param[in] name Name of the child
 * @return The pointer to the child
 */
Xmlelementptr xml_get_child_with_name(Xmlelementptr element, char* name)
{
 Xmlelementptr childnode = element->child;
 while (childnode)
  {
   if (strIcmp(childnode->name, name) == 0)
     return childnode;
   childnode = childnode->sibling;
  }
 return NULL;
}

/**
 * Destructor for a single Xml attribute
 * @param[in] attribute Attribute that will be freed
 */
void xml_free_attribute(void* attribute)
{
 Xmlattributeptr att = (Xmlattributeptr) attribute;
 safe_free(att->name);
 safe_free(att->value);
 safe_free(att);
}

/**
 * Destructor for a single Xml element
 * @param[in] element Element that will be freed
 */
void xml_free_element(Xmlelementptr element)
{
 if (element->attributes != NULL)
   free_link_list(element->attributes, xml_free_attribute);
 if (element->pcdata != NULL)
   safe_free(element->pcdata);
 if (element->child != NULL)
   xml_free_element(element->child);
 if (element->sibling != NULL)
   xml_free_element(element->sibling);
 safe_free(element->name);
 safe_free(element);
}

/**
 * Destructor of the Xml document
 * @param[in] document Document for which memory is freed
 */
void xml_free_document(Xmldocumentptr document)
{
 xml_free_element(document->root);
 safe_free(document->filename);
 safe_free(document);
}

/**
 * Parses given xml document
 * @param[in] document Xml document to be parsed
 */
BOOLEAN xml_parse(Xmldocumentptr document)
{
 FILE* infile;
 XML_TEXT_TYPE texttype = XML_TEXT_ATTRIBUTE;
 XML_TOKEN_TYPE tokentype;
 BOOLEAN sibling_closed = BOOLEAN_FALSE;
 char* token;
 Xmlattributeptr att = NULL;
 Xmlelementptr parent = NULL, sibling = NULL, current = NULL;
 infile = fopen(document->filename, "r");
 if (!infile)
   return BOOLEAN_FALSE;
 token = xml_get_next_token(infile, &tokentype);
 while (tokentype != XML_END)
  {
   switch (tokentype){
     case    XML_OPENING_TAG_WITH_ATTRIBUTES:
     case XML_OPENING_TAG_WITHOUT_ATTRIBUTES:current = xmlelement(token, parent, tokentype == XML_OPENING_TAG_WITH_ATTRIBUTES);
                                             if (parent)
                                              {
                                               if (sibling && sibling_closed)
                                                {
                                                 sibling->sibling = current;
                                                 sibling = current;
                                                }
                                               else
                                                 parent->child = current;
                                              }
                                             else
                                               if (!(document->root))
                                                 document->root = current;
                                               else
                                                 return BOOLEAN_FALSE;
                                             parent = current;
                                             sibling_closed = BOOLEAN_FALSE;
                                             if (tokentype == XML_OPENING_TAG_WITH_ATTRIBUTES)
                                               texttype = XML_TEXT_ATTRIBUTE;
                                             else
                                               texttype = XML_TEXT_VALUE;
                                             break;
     case             XML_OPENING_TAG_FINISH:texttype = XML_TEXT_VALUE;
                                             sibling_closed = BOOLEAN_FALSE;
                                             break;
     case    XML_CLOSING_TAG_WITH_ATTRIBUTES:sibling = current;
                                             parent = current->parent;
                                             texttype = XML_TEXT_VALUE;
                                             sibling_closed = BOOLEAN_TRUE;
                                             break;
     case XML_CLOSING_TAG_WITHOUT_ATTRIBUTES:if (strcmp(token, current->name) == 0)
                                              {
                                               sibling = current;
                                               parent = current->parent;
                                              }
                                             else
                                               if (strcmp(token, current->parent->name) == 0)
                                                {
                                                 sibling = parent;
                                                 parent = current->parent->parent;
                                                 current = current->parent;
                                                }
                                               else
                                                 return BOOLEAN_FALSE;
                                             sibling_closed = BOOLEAN_TRUE;
                                             texttype = XML_TEXT_VALUE;
                                             break;
     case                XML_ATTRIBUTE_VALUE:if (att)
                                               if (token)
                                                 att->value = strcopy(att->value, token);
                                               else
                                                 att->value = strcopy(att->value, "");
                                             else
                                               return BOOLEAN_FALSE;
                                             texttype = XML_TEXT_ATTRIBUTE;
                                             break;
     case                          XML_EQUAL:texttype = XML_TEXT_NOT_AVAILABLE;
                                             break;
     case                           XML_TEXT:if (texttype == XML_TEXT_ATTRIBUTE)
                                              {
                                               att = xmlattribute(token);
                                               link_list_insert(current->attributes, att);
                                              }
                                             else
                                               if (texttype == XML_TEXT_VALUE)
                                                 current->pcdata = strcopy(current->pcdata, token);
                                               else 
                                                 return BOOLEAN_FALSE;
                                             break;
     default                                :printf("This token type not supported\n");
                                             break;
   }
   if (token)
     safe_free(token);
   token = xml_get_next_token(infile, &tokentype);
  }
 fclose(infile);
 return BOOLEAN_TRUE;
}
