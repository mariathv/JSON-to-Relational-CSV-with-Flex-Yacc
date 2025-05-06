%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "schema.h"
#include "common.h"

/* Declare yycolumn as extern */
extern int yycolumn;
extern int yylineno;
extern char* yytext;

void yyerror(const char* s);
int yylex(void);

Node* root = NULL;
%}

%locations
%defines

%union {
    double num;
    char* str;
    Node* node;
    Pair* pair;
    Element* element;
}

%token <num> NUMBER
%token <str> STRING
%token TRUE FALSE NULL_VAL
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA

%type <node> json value object array
%type <pair> pairs pair
%type <element> elements

%%

json: object { root = $1; }
    | array { root = $1; }
    ;

value: object
     | array
     | STRING { $$ = create_string_node($1); }
     | NUMBER { $$ = create_number_node($1); }
     | TRUE { $$ = create_boolean_node(1); }
     | FALSE { $$ = create_boolean_node(0); }
     | NULL_VAL { $$ = create_null_node(); }
     ;

object: LBRACE pairs RBRACE { $$ = create_object_node($2); }
      | LBRACE RBRACE { $$ = create_object_node(NULL); }
      ;

pairs:
      pair { $$ = $1; }
    | pairs COMMA pair { 
        /* THIS IS THE FIX: Link pairs together properly */
        $3->next = $1;
        $$ = $3;
      }
    ;

pair: STRING COLON value { 
    Pair* p = malloc(sizeof(Pair));
    if (!p) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    p->key = strdup($1);  // Use strdup to duplicate the string
    p->value = $3;
    p->next = NULL;
    $$ = p;  // Assign the created pair to $$ to return it
};

array: LBRACKET elements RBRACKET { $$ = create_array_node($2); }
     | LBRACKET RBRACKET { $$ = create_array_node(NULL); }
     ;

elements: value { 
            Element* elem = malloc(sizeof(Element));
            if (!elem) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            elem->value = $1;
            elem->next = NULL;
            $$ = elem;
         }
        | elements COMMA value { 
            Element* elem = malloc(sizeof(Element));
            if (!elem) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            elem->value = $3;
            elem->next = $1;
            $$ = elem;
          }
        ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineno);
    fprintf(stderr, "Current token: '%s'\n", yytext);
    fprintf(stderr, "Expected one of: {, }, [, ], :, ,, \", number, true, false, null\n");
    exit(1);
}