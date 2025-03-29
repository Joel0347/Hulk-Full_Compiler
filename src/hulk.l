%{
#include "hulk.tab.h"
#include "ast.h"
#include <stdlib.h>
%}

%option noyywrap

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT}|_)*
NUM         {DIGIT}+(\.{DIGIT}+)?
STRING      \"([^"\\]|\\.)*\"
COMMENT     #.*\n
WHITESPACE  [ \t\n]+

%%

"function"      { return FUNCTION; }
"let"           { return LET; }
"in"            { return IN; }
"if"            { return IF; }
"elif"          { return ELIF; }
"else"          { return ELSE; }
"true"          { return TRUE; }
"false"         { return FALSE; }
"PI"            { return PI; }
"E"             { return E; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return DIVIDE; }
"**"            { return POWER; }
"^"             { return XOR; }
"@"             { return CONCAT; }
"@@"            { return DCONCAT; }
"=="            { return EQ; }
"!="            { return NEQ; }
"<"             { return LT; }
">"             { return GT; }
"<="            { return LEQ; }
">="            { return GEQ; }
"&"             { return AND; }
"|"             { return OR; }
"!"             { return NOT; }
":="            { return ASSIGN; }
"=>"            { return ARROW; }

";"             { return SEMICOLON; }
","             { return COMMA; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }

{ID}            { yylval.str = strdup(yytext); return IDENTIFIER; }
{NUM}           { yylval.num = atof(yytext); return NUMBER; }
{STRING}        { yylval.str = strdup(yytext); return STRING_LIT; }
{COMMENT}       { /* Ignorar comentarios */ }
{WHITESPACE}    { /* Ignorar espacios en blanco */ }

.               { printf("Error léxico: carácter inválido '%s'\n", yytext); }

%%