%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ast/ast.h"

int yylex(void);
void yyerror(const char *s);
int yyparse(void);

// Variables globales
extern int line_num;
ASTNode* root;
int error_count = 0;
int max_errors = 1;

#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"

// Prototipos
const char* token_to_str(int token);

ASTNode** statements = NULL;
int statement_count = 0;
int statement_capacity = 0;

void add_statement(ASTNode* stmt) {
    if (statement_count >= statement_capacity) {
        statement_capacity = statement_capacity ? statement_capacity * 2 : 16;
        statements = realloc(statements, sizeof(ASTNode*) * statement_capacity);
    }
    statements[statement_count++] = stmt;
}
%}

%union {
    double val;
    char* str;
    char* var;
    struct ASTNode* node;
}

%token <val> NUMBER
%token <var> VARIABLE
%token <str> STRING
%token PRINT
%token ERROR
%token LPAREN RPAREN EQUALS SEMICOLON COMMA

%left PLUS MINUS
%left TIMES DIVIDE
%left POWER
%left UMINUS

%type <node> expression statement expr_list

%%

input:
    /* empty */
    | input statement { add_statement($2); }
    | input error { 
        if (++error_count >= max_errors) {
            YYABORT;
        }
        // Buscar punto de sincronización (; o nueva línea)
        while (1) {
            int tok = yylex();
            if (tok == 0 || tok == SEMICOLON || tok == '\n') break;
        }
        yyerrok;
    }
    ;

statement:
    expr_list SEMICOLON          { $$ = $1; }
    | PRINT LPAREN expression RPAREN SEMICOLON { $$ = create_print_node($3); }
    | VARIABLE EQUALS expression SEMICOLON { $$ = create_assignment_node($1, $3); }
    | ERROR SEMICOLON { // Maneja errores léxicos seguidos de ;
        yyerrok;
        YYABORT;
      }
    ;

expr_list:
    expression                  { $$ = $1; }
    | expr_list COMMA expression { $$ = create_binary_op_node(OP_ADD, $1, $3); } // Simplificación
    | ERROR COMMA expression { // Maneja errores en listas
        yyerrok;
        YYABORT;
      }
    ;

expression:
    NUMBER                      { $$ = create_number_node($1); }
    | STRING                    { $$ = create_string_node($1); }
    | VARIABLE                  { $$ = create_variable_node($1); }
    | expression PLUS expression { $$ = create_binary_op_node(OP_ADD, $1, $3); }
    | expression MINUS expression { $$ = create_binary_op_node(OP_SUB, $1, $3); }
    | expression TIMES expression { $$ = create_binary_op_node(OP_MUL, $1, $3); }
    | expression DIVIDE expression { $$ = create_binary_op_node(OP_DIV, $1, $3); }
    | expression POWER expression { $$ = create_binary_op_node(OP_POW, $1, $3); }
    | MINUS expression %prec UMINUS { $$ = create_unary_op_node(OP_NEGATE, $2); }
    | LPAREN expression RPAREN  { $$ = $2; }
    | ERROR { // Maneja cualquier otro error en expresión
        yyerrok;
        YYABORT;
      }
    ;

%%

const char* token_to_str(int token) {
    switch(token) {
        case NUMBER: return "number";
        case VARIABLE: return "identifier";
        case STRING: return "string";
        case PRINT: return "'print'";
        case LPAREN: return "'('";
        case RPAREN: return "')'";
        case EQUALS: return "'='";
        case SEMICOLON: return "';'";
        case COMMA: return "','";
        case PLUS: return "'+'";
        case MINUS: return "'-'";
        case TIMES: return "'*'";
        case DIVIDE: return "'/'";
        case POWER: return "'^'";
        default: return "";
    }
}

void yyerror(const char *s) {
    extern int yychar;
    extern char *yytext;
    
    if (error_count >= max_errors) return;
        
    if (yychar == ERROR) {
        return;
    } else {
        fprintf(stderr, RED"!! SYNTAX ERROR: ");
        
        switch(yychar) {
            case ';' : fprintf(stderr, "Missing expression before ';'"); break;
            case ')' : fprintf(stderr, "Missing expression or parenthesis"); break;
            case '(' : fprintf(stderr, "Missing closing parenthesis"); break;
            default:
                if (!yychar) {
                    fprintf(stderr, "Missing ';' at the end of the statement");
                }
                else {
                    fprintf(stderr, "Unexpected token %s", token_to_str(yychar));
                }
        }

        fprintf(stderr, RED". Error in line: %d \n"RESET, line_num);
    }
    
    error_count++;
}