%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ast/ast.h"

int yylex(void);
int yyparse(void);
void yyerror(const char *s);

ASTNode* root;
int error_count = 0;
int max_errors = 1;

#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"

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
    struct {
        struct ASTNode** args;
        int arg_count;
    } *arg_list;
}

%token <val> NUMBER
%token <val> PI
%token <val> E
%token <var> ID
%token <str> STRING
%token <str> BOOLEAN
%token ERROR
%token LPAREN RPAREN EQUALS SEMICOLON COMMA LBRACKET RBRACKET COLON

%left DCONCAT
%left CONCAT
%left AND
%left OR 
%left NOT
%left EQUALSEQUALS NEQUALS
%left EGREATER GREATER ELESS LESS
%left PLUS MINUS
%left TIMES DIVIDE MOD
%left POWER
%left UMINUS

%type <node> expression block_expr statement variable_declaration function_call
%type <arg_list> list_args block_expr_list

%%

program:
    input {
        // When parsing ends succesfully, it creates the program node
        if (error_count == 0) {
            root = create_program_node(statements, statement_count, NODE_PROGRAM);
        } else {
            root = NULL;
        }
    }
;

input:
    /* empty */
    | input statement { add_statement($2); }
    | input error { 
        if (++error_count >= max_errors) {
            YYABORT;
        }
        // Find syncronization point (; or new line)
        while (1) {
            int tok = yylex();
            if (tok == 0 || tok == SEMICOLON || tok == '\n') break;
        }
        yyerrok;
    }
;

statement:
      block_expr SEMICOLON { $$ = $1; }
    | block_expr           { $$ = $1; }
    | expression SEMICOLON { $$ = $1; }
;


list_args:
    expression {
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * 1);
        $$->args[0] = $1;
        $$->arg_count = 1;
    }
    | expression COMMA list_args {
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * ($3->arg_count + 1));
        $$->args[0] = $1;
        memcpy($$->args + 1, $3->args, sizeof(ASTNode *) * $3->arg_count);
        $$->arg_count = $3->arg_count + 1;
        free($3->args);
    }

    | /* empty */ {
        $$ = malloc(sizeof(*$$));
        $$->args = NULL;
        $$->arg_count = 0;
    }
;

block_expr:
    LBRACKET block_expr_list RBRACKET { $$ = create_program_node($2->args, $2->arg_count, NODE_BLOCK); }
;

block_expr_list:
    expression SEMICOLON {
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * 1);
        $$->args[0] = $1;
        $$->arg_count = 1;
    }
    | expression SEMICOLON block_expr_list {
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * ($3->arg_count + 1));
        $$->args[0] = $1;
        memcpy($$->args + 1, $3->args, sizeof(ASTNode *) * $3->arg_count);
        $$->arg_count = $3->arg_count + 1;
        free($3->args);
    }

    | /* empty */ {
        $$ = malloc(sizeof(*$$));
        $$->args = NULL;
        $$->arg_count = 0;
    }
;

variable_declaration:
    ID COLON ID EQUALS expression    { $$ = create_assignment_node($1, $5, $3); }
    | ID EQUALS expression           { $$ = create_assignment_node($1, $3, ""); }
;

function_call:
    ID LPAREN list_args RPAREN           { $$ = create_func_call_node($1, $3->args, $3->arg_count); }
;

expression:
    NUMBER                               { $$ = create_number_node($1); }
    | PI                                 { $$ = create_number_node(M_PI); }
    | E                                  { $$ = create_number_node(M_E); }
    | STRING                             { $$ = create_string_node($1); }
    | BOOLEAN                            { $$ = create_boolean_node($1); }
    | block_expr                         { $$ = $1; }
    | function_call                      { $$ = $1; }
    | ID                                 { $$ = create_variable_node($1); }
    | expression DCONCAT expression      { $$ = create_binary_op_node(OP_DCONCAT, "@@", $1, $3, &TYPE_STRING_INST) }
    | expression CONCAT expression       { $$ = create_binary_op_node(OP_CONCAT, "@", $1, $3, &TYPE_STRING_INST) }
    | expression AND expression          { $$ = create_binary_op_node(OP_AND, "&", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression OR expression           { $$ = create_binary_op_node(OP_OR, "|", $1, $3, &TYPE_BOOLEAN_INST)}
    | NOT expression                     { $$ = create_unary_op_node(OP_NOT, "!", $2, &TYPE_BOOLEAN_INST)}
    | expression EQUALSEQUALS expression { $$ = create_binary_op_node(OP_EQ, "==", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression NEQUALS expression      { $$ = create_binary_op_node(OP_NEQ, "!=", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression EGREATER expression     { $$ = create_binary_op_node(OP_GRE, ">=", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression GREATER expression      { $$ = create_binary_op_node(OP_GR, ">", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression ELESS expression        { $$ = create_binary_op_node(OP_LSE, "<=", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression LESS expression         { $$ = create_binary_op_node(OP_LS, "<", $1, $3, &TYPE_BOOLEAN_INST)}
    | expression PLUS expression         { $$ = create_binary_op_node(OP_ADD, "+", $1, $3, &TYPE_NUMBER_INST)}
    | expression MINUS expression        { $$ = create_binary_op_node(OP_SUB, "-", $1, $3, &TYPE_NUMBER_INST); }
    | expression TIMES expression        { $$ = create_binary_op_node(OP_MUL, "*", $1, $3, &TYPE_NUMBER_INST); }
    | expression DIVIDE expression       { $$ = create_binary_op_node(OP_DIV, "/", $1, $3, &TYPE_NUMBER_INST); }
    | expression MOD expression          { $$ = create_binary_op_node(OP_MOD, "%", $1, $3, &TYPE_NUMBER_INST); }
    | expression POWER expression        { $$ = create_binary_op_node(OP_POW, "^", $1, $3, &TYPE_NUMBER_INST); }
    | MINUS expression %prec UMINUS      { $$ = create_unary_op_node(OP_NEGATE, "-", $2, &TYPE_NUMBER_INST); }
    | LPAREN expression RPAREN           { $$ = $2; }
    | variable_declaration               { $$ = $1; }
    | ERROR { // Handle any other error
        yyerrok;
        YYABORT;
      }
;

%%

const char* token_to_str(int token) {
    switch(token) {
        case NUMBER:       return "number" ; case ID:       return "identifier"; case STRING:   return "string";
        case LPAREN:       return "'('"    ; case RPAREN:   return "')'"       ; case COLON:    return "':'"   ;
        case LBRACKET:     return "'{'"    ; case RBRACKET: return "'}'"       ; case EQUALS:   return "'='"   ;
        case SEMICOLON:    return "';'"    ; case PLUS:     return "'+'"       ; case MINUS:    return "'-'"   ;
        case TIMES:        return "'*'"    ; case DIVIDE:   return "'/'"       ; case MOD:      return "'%'"   ;
        case POWER:        return "'^'"    ; case CONCAT:   return "'@'"       ; case DCONCAT:  return "'@@'"  ;
        case AND:          return "'&'"    ; case OR:       return "'|'"       ; case NOT:      return "'!'"   ;
        case EQUALSEQUALS: return "'=='"   ; case NEQUALS:  return "'!='"      ; case EGREATER: return "'>='"  ;
        case GREATER:      return "'>'"    ; case ELESS:    return "'<='"      ; case LESS:     return "'<'"   ;
        case COMMA:        return "','"    ; case PI:       return "'PI'"      ; case E:        return "'E'"   ;

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