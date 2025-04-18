%{
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"  // Asegúrate que este es el header correcto
#include "ast/ast.h"    // Necesario para el tipo ASTNode

int yywrap(void) { return 1; }
int line_num = 1;
#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"
%}

%%

\n         { line_num++; }
[ \t]        ;
";"            return SEMICOLON;
[0-9]+         { yylval.val = atoi(yytext); return NUMBER; }
[0-9]+\.[0-9]+ { yylval.val = atof(yytext); return NUMBER; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return DIVIDE; }
"^"             { return POWER; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"="             { return EQUALS; }
"print"         { return PRINT; }

[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*  { yylval.var = strdup(yytext); return VARIABLE; }
"//"[^\n]*?   { /* COMMENT */ }
\"([^"\\]|\\.)*\" { yylval.var = strdup(yytext); return STRING; }

.           { 
                fprintf(stderr, RED"!!LEXICAL ERROR: Invalid character '%s' in line: %d \n"RESET, yytext, line_num);
                return ERROR;
            }

%%