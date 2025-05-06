%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "common.h"
#include "parser.tab.h"


void yyerror(const char* s);
void print_next_chars(const char* text, int len);

void print_next_chars(const char* text, int len) {
    printf("Next characters after token: ");
    for(int i = 0; i < 5 && i < len; i++) {
        printf("'%c'(%d) ", text[i], (int)text[i]);
    }
    printf("\n");
}

/* Local column counter */
static int current_column = 1;
%}

%option yylineno
%option noyywrap
%option nounput
%option noinput

%%

^\xEF\xBB\xBF { /* Skip UTF-8 BOM */ }
[ \t]+        { current_column += yyleng; }  /* Skip spaces and tabs */
\r\n          { current_column = 1; }        /* Handle Windows line endings */
\n            { current_column = 1; }        /* Handle Unix line endings */
\r            { }                           /* Skip bare carriage returns */
"{" { current_column++; return LBRACE; }
"}" { current_column++; return RBRACE; }
"[" { current_column++; return LBRACKET; }
"]" { current_column++; return RBRACKET; }
":" { current_column++; return COLON; }
"," { current_column++; return COMMA; }


\"([^"\\]|\\.)*\" {
    /* String literal */
    char* str = malloc(yyleng - 1);
    strncpy(str, yytext + 1, yyleng - 2);
    str[yyleng - 2] = '\0';
    yylval.str = str;
    printf("Found string '%s' at line %d, column %d\n", str, yylineno, current_column);
    current_column += yyleng;
    return STRING;
}

-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)? {
    /* Number literal */
    yylval.num = atof(yytext);
    printf("Found number %f at line %d, column %d\n", yylval.num, yylineno, current_column);
    current_column += yyleng;
    return NUMBER;
}

"true"        { printf("Found 'true' at line %d, column %d\n", yylineno, current_column); current_column += yyleng; return TRUE; }
"false"       { printf("Found 'false' at line %d, column %d\n", yylineno, current_column); current_column += yyleng; return FALSE; }
"null"        { printf("Found 'null' at line %d, column %d\n", yylineno, current_column); current_column += yyleng; return NULL_VAL; }

. {
    unsigned char c = (unsigned char)yytext[0];
    if (isprint(c)) {
        fprintf(stderr, "Error: Unexpected character '%c' (ASCII %d) at line %d, column %d\n", 
                c, (int)c, yylineno, current_column);
    } else {
        fprintf(stderr, "Error: Unexpected non-printable character (ASCII %d) at line %d, column %d\n", 
                (int)c, yylineno, current_column);
    }
    fprintf(stderr, "Expected one of: {, }, [, ], :, ,, \", number, true, false, null\n");
    exit(1);
}

%% 