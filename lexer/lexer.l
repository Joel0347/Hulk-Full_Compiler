%{
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"
#include "ast/ast.h"

int yywrap(void) { return 1; }
int line_num = 1;
#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"
%}

%%

\n              { line_num++; }
[ \t]           { ; }
";"             { return SEMICOLON; }
","             { return COMMA; }
[0-9]+          { yylval.val = atof(yytext); return NUMBER; }
[0-9]+\.[0-9]+  { yylval.val = atof(yytext); return NUMBER; }
"."             { return DOT; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return DIVIDE; }
"%"             { return MOD; }
"^"             { return POWER; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACKET; }
"}"             { return RBRACKET; }
"+="            { return PLUSEQUAL; }
"-="            { return MINUSEQUAL; }
"*="            { return TIMESEQUAL; }
"/="            { return DIVEQUAL; }
"%="            { return MODEQUAL; }
"^="            { return POWEQUAL; }
"&="            { return ANDEQUAL; }
"|="            { return OREQUAL; }
"@="            { return CONCATEQUAL; }
":="            { return DEQUALS; }
":"             { return COLON; }      
"=="            { return EQUALSEQUALS; }
"=>"            { return ARROW; }
"="             { return EQUALS; }
"@@"            { return DCONCAT; }
"@"             { return CONCAT; }
"!="            { return NEQUALS; }
"!"             { return NOT; }
"&"             { return AND; }
"|"             { return OR; }
">="            { return EGREATER; }
">"             { return GREATER; } 
"<="            { return ELESS; }
"<"             { return LESS; }          
"PI"            { return PI; }
"E"             { return E; }
"true" | 
"false"         { yylval.var = strdup(yytext); return BOOLEAN; }
"function"      { return FUNCTION; }
"let"           { return LET; }
"in"            { return IN; }
"if"            { return IF; }
"elif"          { return ELIF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"as"            { return AS; }
"is"            { return IS; }
"type"          { return TYPE; }
"inherits"      { return INHERITS; }
"new"           { return NEW; }
"base"          { return BASE; }

[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*  { yylval.var = strdup(yytext); return ID; }
"//"[^\n]*?   { /* COMMENT */ }
\"([^"\\]|\\.)*\" {
    int len = strlen(yytext);
    yylval.var = strdup(yytext + 1);  // Skip first quote
    yylval.var[len - 2] = '\0';       // Replace last quote for '\0'
    return STRING;
}

.           { 
                fprintf(stderr, RED"!!LEXICAL ERROR: Invalid character '%s'. Line: %d \n"RESET, yytext, line_num);
                return ERROR;
            }

%%