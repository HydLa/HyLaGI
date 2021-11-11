%{
#include <stdio.h>
#include "y.tab.hpp"
%}

NONZERO             [1-9]
DIGIT               [[:digit:]]

%%

"/*"[^"*/"]*"*/" {
  // char comment[2048];
  sscanf(yytext, "%*[^\r\n]");
  // sscanf(yytext, "%[^\r\n]", comment);
  // printf("Comment:\n %s\n", comment);
}

"never" return NEVER;
"skip" return SKIP;
"{"   return L_CRULY;
"}"   return R_CRULY;
"("   return L_PAREN;
")"   return R_PAREN;
":="   return DEFINITION;
"::"   return D_COLON;
":"   return COLON;
";"   return SEMICOLON;
"->"  return R_ARROW;
"&&"  return AND;
"||"  return OR;
"IF"|"if"  return IF;
"FI"|"fi"  return FI;
"GOTO"|"goto" return GOTO;
"TRUE"|"true" return TRUE;
"1" return ONE;
"!" return NOT;
[:blank:] return BLANK;
"[]"  return ALWAYS;
"<>"  return EVENTUALLY;
"\n"    return CR;
[A-Za-z_][A-Za-z_0-9]* {
  // printf("Identifier \"%s\" was recognized.\n", yytext);
  yylval.string_value = (char *)malloc(strlen(yytext) + 1);
  strcpy(yylval.string_value, yytext);
  return IDENTIFIER;
}
":="" "*[^"/*""*/"\n]* {
  // printf("Constraint \"%s\" was recognized.\n", yytext);
  yylval.string_value = (char *)malloc(strlen(yytext) + 1);
  strcpy(yylval.string_value, yytext);
  return CONSTRAINT_STRING;
}

%%
