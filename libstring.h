#ifndef __libstring_H
#define __libstring_H

int      char_count(char* st, char ch);
int      instr(char* st, char* searchst);
int      listindex(char *element, char *array[], int elementcount);
int      listindexc(char element, char array[], int elementcount);
char*    strconcat(char *s,const char *ct);
char*    strcopy(char *s, char *ct);
void     stringlist(char* st, char* separator, char* list[], int* count);
int      strIcmp(const char *s1, const char *s2);
char     upper(char ch); 

#endif
