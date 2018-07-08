#ifndef __xmlparser_H
#define __xmlparser_H

#include "datastructure.h"

#define MAXTOKENLENGTH 50

enum xml_text_type{
      XML_TEXT_ATTRIBUTE = 0,
      XML_TEXT_VALUE,
      XML_TEXT_NOT_AVAILABLE
};

typedef enum xml_text_type XML_TEXT_TYPE;

enum xml_token_type{
      XML_OPENING_TAG_WITH_ATTRIBUTES = 0,
      XML_OPENING_TAG_WITHOUT_ATTRIBUTES,
      XML_OPENING_TAG_FINISH,
      XML_CLOSING_TAG_WITH_ATTRIBUTES,
      XML_CLOSING_TAG_WITHOUT_ATTRIBUTES,
      XML_ATTRIBUTE_VALUE,
      XML_EQUAL,
      XML_TEXT,
      XML_END
};

typedef enum xml_token_type XML_TOKEN_TYPE;

struct xmlattribute{
                    char* name;
                    char* value;
};

typedef struct xmlattribute Xmlattribute;
typedef Xmlattribute* Xmlattributeptr;

struct xmlelement{
                  char* name;
                  char* pcdata;
                  Linklistptr attributes;
                  struct xmlelement* parent;
                  struct xmlelement* child;
                  struct xmlelement* sibling;
};

typedef struct xmlelement Xmlelement;
typedef Xmlelement* Xmlelementptr;

struct xmldocument{
                   char* filename;
                   Xmlelementptr root;
};

typedef struct xmldocument Xmldocument;
typedef Xmldocument* Xmldocumentptr;

Xmldocumentptr  xml_document(char* filename);
void            xml_free_attribute(void* attribute);
void            xml_free_document(Xmldocumentptr document);
void            xml_free_element(Xmlelementptr element);
char*           xml_get_attribute_value(Xmlelementptr element, char* attribute_name);
Xmlelementptr   xml_get_child_with_name(Xmlelementptr element, char* name);
Xmlelementptr   xml_get_child_with_specific_attribute_value(Xmlelementptr element, char* attribute_name, char* attribute_value);
char*           xml_get_next_token(FILE* infile, XML_TOKEN_TYPE* tokentype);
int             xml_number_of_children(Xmlelementptr element);
int             xml_number_of_children_with_specific_attribute_value(Xmlelementptr element, char* attribute_name, char* attribute_value);
BOOLEAN         xml_parse(Xmldocumentptr document);
char*           xml_parse_attribute_value(FILE* infile, XML_TOKEN_TYPE* tokentype);
char*           xml_parse_empty_tag(FILE* infile, XML_TOKEN_TYPE* tokentype);
char*           xml_parse_tag(FILE* infile, XML_TOKEN_TYPE* tokentype);
void            xml_print_element(FILE* handle, Xmlelementptr element, int level);
void            xml_print_tree(Xmldocumentptr document, char* filename);
char*           xml_read_token(FILE* infile, char prevchar, char* nextchar, int space_allowed);
void            xml_set_attribute_value(Xmlelementptr element, char* attribute_name, char* attribute_value);
Xmlattributeptr xmlattribute(char* name);
Xmlelementptr   xmlelement(char* name, Xmlelementptr parent, int has_attributes);

#endif
