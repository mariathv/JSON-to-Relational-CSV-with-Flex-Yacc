#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

// Yacc/Bison variables
extern int yylineno;
extern int yycolumn;
extern char *yytext;
extern Node *root;

// Function declarations
void yyerror(const char *s);
int yylex(void);
int yyparse(void);

#endif // PARSER_H
