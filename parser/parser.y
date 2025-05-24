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
    if (stmt == NULL)
        return;
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
%token ERROR ARROW FUNCTION DEQUALS LET IN IF ELIF ELSE WHILE
%token LPAREN RPAREN EQUALS SEMICOLON COMMA LBRACKET RBRACKET COLON

%token CONCATEQUAL ANDEQUAL OREQUAL PLUSEQUAL MINUSEQUAL
%token TIMESEQUAL DIVEQUAL MODEQUAL POWEQUAL

%left IS AS
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

%type <node> expression block_expr statement destructive_var_decl function_call
%type <node> param function_declaration simple_var_decl let_in_exp conditional
%type <node> elif_branch while_loop compound_operator

%type <arg_list> list_args block_expr_list param_list let_definitions

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
    | input statement { if ($2 != NULL) add_statement($2); }
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
    SEMICOLON                        { $$ = NULL; }
    | function_declaration           { $$ = $1; }
    | conditional                    { $$ = $1; }
    | while_loop                     { $$ = $1; }
    | function_declaration SEMICOLON { $$ = $1; }
    | block_expr                     { $$ = $1; }
    | expression SEMICOLON           { $$ = $1; }
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
    statement {
        $$ = malloc(sizeof(*$$));

        if ($1 == NULL) {
            $$->args = NULL;
            $$->arg_count = 0;
        } else {
            $$->args = malloc(sizeof(ASTNode *) * 1);
            $$->args[0] = $1;
            $$->arg_count = 1;
        }
    }
    | statement block_expr_list {
        $$ = malloc(sizeof(*$$));

        if ($1 == NULL) {
            $$->args = malloc(sizeof(ASTNode *) * ($2->arg_count));
            memcpy($$->args, $2->args, sizeof(ASTNode *) * $2->arg_count);
            $$->arg_count = $2->arg_count;
            free($2->args);
        } else {
            $$->args = malloc(sizeof(ASTNode *) * ($2->arg_count + 1));
            $$->args[0] = $1;
            memcpy($$->args + 1, $2->args, sizeof(ASTNode *) * $2->arg_count);
            $$->arg_count = $2->arg_count + 1;
            free($2->args);
        }
    }

    | /* empty */ {
        $$ = malloc(sizeof(*$$));
        $$->args = NULL;
        $$->arg_count = 0;
    }
;

param:
    ID            { $$ = create_variable_node($1, "", 1); }
    | ID COLON ID { $$ = create_variable_node($1, $3, 1); }

param_list:
    param {
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * 1);
        $$->args[0] = $1;
        $$->arg_count = 1;
    }
    | param COMMA param_list {
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

function_declaration:
    FUNCTION ID LPAREN param_list RPAREN ARROW expression SEMICOLON  { 
        $$ = create_func_dec_node($2, $4->args, $4->arg_count, $7, ""); 
    }
    | FUNCTION ID LPAREN param_list RPAREN COLON ID ARROW expression SEMICOLON { 
        $$ = create_func_dec_node($2, $4->args, $4->arg_count, $9, $7); 
    }
    | FUNCTION ID LPAREN param_list RPAREN block_expr {
        $$ = create_func_dec_node($2, $4->args, $4->arg_count, $6, "");
    }
    | FUNCTION ID LPAREN param_list RPAREN COLON ID block_expr {
        $$ = create_func_dec_node($2, $4->args, $4->arg_count, $8, $7);
    }

;

let_in_exp:
    LET let_definitions IN expression { $$ = create_let_in_node($2->args, $2->arg_count, $4); }
;

let_definitions:
    simple_var_decl { 
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * 1);
        $$->args[0] = $1;
        $$->arg_count = 1;
    }
    | simple_var_decl COMMA let_definitions {
        $$ = malloc(sizeof(*$$));
        $$->args = malloc(sizeof(ASTNode *) * ($3->arg_count + 1));
        $$->args[0] = $1;
        memcpy($$->args + 1, $3->args, sizeof(ASTNode *) * $3->arg_count);
        $$->arg_count = $3->arg_count + 1;
        free($3->args);
    }
;

destructive_var_decl:
    ID COLON ID DEQUALS expression        { $$ = create_assignment_node($1, $5, $3, NODE_D_ASSIGNMENT); }
    | ID DEQUALS expression               { $$ = create_assignment_node($1, $3, "", NODE_D_ASSIGNMENT); }
;

simple_var_decl:
    ID COLON ID EQUALS expression        { $$ = create_assignment_node($1, $5, $3, NODE_ASSIGNMENT); }
    | ID EQUALS expression               { $$ = create_assignment_node($1, $3, "", NODE_ASSIGNMENT); }
;

function_call:
    ID LPAREN list_args RPAREN           { $$ = create_func_call_node($1, $3->args, $3->arg_count); }
;

conditional:
    IF LPAREN expression RPAREN expression                   { $$ = create_conditional_node($3, $5, NULL); }
    | IF LPAREN expression RPAREN expression elif_branch     { $$ = create_conditional_node($3, $5, $6); }
    | IF LPAREN expression RPAREN expression ELSE expression { $$ = create_conditional_node($3, $5, $7); }
;

elif_branch:
    ELIF LPAREN expression RPAREN expression                   { $$ = create_conditional_node($3, $5, NULL); }
    | ELIF LPAREN expression RPAREN expression ELSE expression { $$ = create_conditional_node($3, $5, $7); }
    | ELIF LPAREN expression RPAREN expression elif_branch     { $$ = create_conditional_node($3, $5, $6); }
;

while_loop:
    WHILE LPAREN expression RPAREN expression { $$ = create_loop_node($3, $5); }
;

compound_operator:
    ID PLUSEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_ADD, "+", create_variable_node($1, "", 0), $3, &TYPE_NUMBER
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID MINUSEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_SUB, "-", create_variable_node($1, "", 0), $3, &TYPE_NUMBER
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID TIMESEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_MUL, "*", create_variable_node($1, "", 0), $3, &TYPE_NUMBER
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID DIVEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_DIV, "/", create_variable_node($1, "", 0), $3, &TYPE_NUMBER
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID MODEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_MOD, "%", create_variable_node($1, "", 0), $3, &TYPE_NUMBER
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID POWEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_POW, "^", create_variable_node($1, "", 0), $3, &TYPE_NUMBER
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID CONCATEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_CONCAT, "@", create_variable_node($1, "", 0), $3, &TYPE_STRING
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID ANDEQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_AND, "&", create_variable_node($1, "", 0), $3, &TYPE_BOOLEAN
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
    | ID OREQUAL expression {
        $$ = create_assignment_node(
            $1, create_binary_op_node(
                OP_OR, "|", create_variable_node($1, "", 0), $3, &TYPE_BOOLEAN
            ),
            "", NODE_D_ASSIGNMENT
        );
    }
;

expression:
    NUMBER                               { $$ = create_number_node($1); }
    | PI                                 { $$ = create_number_node(M_PI); }
    | E                                  { $$ = create_number_node(M_E); }
    | STRING                             { $$ = create_string_node($1); }
    | BOOLEAN                            { $$ = create_boolean_node($1); }
    | block_expr                         { $$ = $1; }
    | function_call                      { $$ = $1; }
    | let_in_exp                         { $$ = $1; }
    | ID                                 { $$ = create_variable_node($1, "", 0); }
    | expression IS ID                   { $$ = create_test_casting_type_node($1, $3, 1); }
    | expression AS ID                   { $$ = create_test_casting_type_node($1, $3, 0); }
    | expression DCONCAT expression      { $$ = create_binary_op_node(OP_DCONCAT, "@@", $1, $3, &TYPE_STRING); }
    | expression CONCAT expression       { $$ = create_binary_op_node(OP_CONCAT, "@", $1, $3, &TYPE_STRING); }
    | expression AND expression          { $$ = create_binary_op_node(OP_AND, "&", $1, $3, &TYPE_BOOLEAN); }
    | expression OR expression           { $$ = create_binary_op_node(OP_OR, "|", $1, $3, &TYPE_BOOLEAN); }
    | NOT expression                     { $$ = create_unary_op_node(OP_NOT, "!", $2, &TYPE_BOOLEAN); }
    | expression EQUALSEQUALS expression { $$ = create_binary_op_node(OP_EQ, "==", $1, $3, &TYPE_BOOLEAN); }
    | expression NEQUALS expression      { $$ = create_binary_op_node(OP_NEQ, "!=", $1, $3, &TYPE_BOOLEAN); }
    | expression EGREATER expression     { $$ = create_binary_op_node(OP_GRE, ">=", $1, $3, &TYPE_BOOLEAN); }
    | expression GREATER expression      { $$ = create_binary_op_node(OP_GR, ">", $1, $3, &TYPE_BOOLEAN); }
    | expression ELESS expression        { $$ = create_binary_op_node(OP_LSE, "<=", $1, $3, &TYPE_BOOLEAN); }
    | expression LESS expression         { $$ = create_binary_op_node(OP_LS, "<", $1, $3, &TYPE_BOOLEAN); }
    | expression PLUS expression         { $$ = create_binary_op_node(OP_ADD, "+", $1, $3, &TYPE_NUMBER); }
    | expression MINUS expression        { $$ = create_binary_op_node(OP_SUB, "-", $1, $3, &TYPE_NUMBER); }
    | expression TIMES expression        { $$ = create_binary_op_node(OP_MUL, "*", $1, $3, &TYPE_NUMBER); }
    | expression DIVIDE expression       { $$ = create_binary_op_node(OP_DIV, "/", $1, $3, &TYPE_NUMBER); }
    | expression MOD expression          { $$ = create_binary_op_node(OP_MOD, "%", $1, $3, &TYPE_NUMBER); }
    | expression POWER expression        { $$ = create_binary_op_node(OP_POW, "^", $1, $3, &TYPE_NUMBER); }
    | MINUS expression %prec UMINUS      { $$ = create_unary_op_node(OP_NEGATE, "-", $2, &TYPE_NUMBER); }
    | LPAREN expression RPAREN           { $$ = $2; }
    | destructive_var_decl               { $$ = $1; }
    | simple_var_decl                    { $$ = $1; }
    | conditional                        { $$ = $1; }
    | while_loop                         { $$ = $1; }
    | compound_operator                  { $$ = $1; }
    | ERROR { // Handle any other error
        yyerrok;
        YYABORT;
      }
;

%%

const char* token_to_str(int token) {
    switch(token) {
        case NUMBER:       return "'number'" ; case ID:       return "'identifier'"; case STRING:     return "'string'";
        case LPAREN:       return "'('"      ; case RPAREN:   return "')'"         ; case COLON:      return "':'"     ;
        case LBRACKET:     return "'{'"      ; case RBRACKET: return "'}'"         ; case EQUALS:     return "'='"     ;
        case SEMICOLON:    return "';'"      ; case PLUS:     return "'+'"         ; case MINUS:      return "'-'"     ;
        case TIMES:        return "'*'"      ; case DIVIDE:   return "'/'"         ; case MOD:        return "'%'"     ;
        case POWER:        return "'^'"      ; case CONCAT:   return "'@'"         ; case DCONCAT:    return "'@@'"    ;
        case AND:          return "'&'"      ; case OR:       return "'|'"         ; case NOT:        return "'!'"     ;
        case EQUALSEQUALS: return "'=='"     ; case NEQUALS:  return "'!='"        ; case EGREATER:   return "'>='"    ;
        case GREATER:      return "'>'"      ; case ELESS:    return "'<='"        ; case LESS:       return "'<'"     ;
        case COMMA:        return "','"      ; case PI:       return "'PI'"        ; case E:          return "'E'"     ;
        case ARROW:        return "'=>'"     ; case FUNCTION: return "'function'"  ; case DEQUALS:    return "':='"    ;
        case BOOLEAN:      return "'boolean'"; case LET:      return "'let'"       ; case IN:         return "'in'"    ;
        case IF:           return "'if'"     ; case ELIF:     return "'elif'"      ; case ELSE:       return "'else'"  ;
        case PLUSEQUAL:    return "'+='"     ; case DIVEQUAL: return "'/='"        ; case POWEQUAL:   return "'^='"    ;
        case MINUSEQUAL:   return "'-='"     ; case ANDEQUAL: return "'&='"        ; case OREQUAL:    return "'|='"    ;
        case CONCATEQUAL:  return "'@='"     ; case MODEQUAL: return "'%='"        ; case TIMESEQUAL: return "'*='"    ;
        case WHILE:        return "'while'"  ; case IS:       return "'is'"        ; case AS:         return "'as'"    ;

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

        fprintf(stderr, RED". Line: %d. \n"RESET, line_num);
    }
    
    error_count++;
}