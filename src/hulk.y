%{
#include <stdio.h>
#include <stdlib.h>
#include "hulk.tab.h"
#include "ast.h"
%}
%define api.pure full
%locations

%union {
    double num;
    char *str;
    struct ASTNode *node;
}

%token <num> NUMBER
%token <str> IDENTIFIER STRING_LIT
%token FUNCTION LET IN IF ELIF ELSE TRUE FALSE PI E
%token PLUS MINUS TIMES DIVIDE POWER XOR CONCAT DCONCAT
%token EQ NEQ LT GT LEQ GEQ AND OR NOT
%token ASSIGN ARROW
%token SEMICOLON COMMA LPAREN RPAREN LBRACE RBRACE

%type <node> Program P Exp Block ExpList IfExp ElifList LetIn AssignList MoreAssign
%type <node> LogicOr OpOr LogicAnd OpAnd Compare OpCompare X Op1 F Op3 T TRest
%type <node> FunctionDef ArgList MoreArgs ArgListCall MoreArgsCall

%start Program

%%

Program: 
    FunctionDef Program { $$ = create_program_node($1, $2); }
    | P { $$ = $1; }
    ;

OpEnd: SEMICOLON | ;

P: 
    Exp OpEnd { $$ = $1; }
    | Block { $$ = $1; }
    ;

Exp: 
    IfExp { $$ = $1; }
    | LetIn { $$ = $1; }
    | LogicOr { $$ = $1; }
    ;

Block: 
    LBRACE ExpList RBRACE { $$ = create_block_node($2); }
    ;

ExpList: 
    P ExpList { $$ = create_exp_list_node($1, $2); }
    | /* epsilon */ { $$ = NULL; }
    ;

IfExp: 
    IF LPAREN Exp RPAREN P ElifList ELSE P { $$ = create_if_node($3, $5, $6, $8); }
    ;

ElifList: 
    ELIF LPAREN Exp RPAREN P ElifList { $$ = create_elif_node($3, $5, $6); }
    | /* epsilon */ { $$ = NULL; }
    ;

LetIn: 
    LET AssignList IN P { $$ = create_let_in_node($2, $4); }
    ;

AssignList: 
    IDENTIFIER ASSIGN Exp MoreAssign { $$ = create_assign_list_node($1, $3, $4); }
    | /* epsilon */ { $$ = NULL; }
    ;

MoreAssign: 
    COMMA IDENTIFIER ASSIGN Exp MoreAssign { $$ = create_more_assign_node($2, $4, $5); }
    | /* epsilon */ { $$ = NULL; }
    ;

LogicOr: 
    LogicAnd OpOr { $$ = create_logic_or_node($1, $2); }
    ;

OpOr: 
    OR LogicAnd OpOr { $$ = create_op_or_node($2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

LogicAnd: 
    Compare OpAnd { $$ = create_logic_and_node($1, $2); }
    ;

OpAnd: 
    AND Compare OpAnd { $$ = create_op_and_node($2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

Compare: 
    X OpCompare { $$ = create_compare_node($1, $2); }
    ;

OpCompare: 
    EQ X OpCompare { $$ = create_op_compare_node(EQ, $2, $3); }
    | NEQ X OpCompare { $$ = create_op_compare_node(NEQ, $2, $3); }
    | LT X OpCompare { $$ = create_op_compare_node(LT, $2, $3); }
    | GT X OpCompare { $$ = create_op_compare_node(GT, $2, $3); }
    | LEQ X OpCompare { $$ = create_op_compare_node(LEQ, $2, $3); }
    | GEQ X OpCompare { $$ = create_op_compare_node(GEQ, $2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

X: 
    F Op1 { $$ = create_x_node($1, $2); }
    ;

Op1: 
    PLUS F Op1 { $$ = create_op1_node(PLUS, $2, $3); }
    | MINUS F Op1 { $$ = create_op1_node(MINUS, $2, $3); }
    | CONCAT F Op1 { $$ = create_op1_node(CONCAT, $2, $3); }
    | DCONCAT F Op1 { $$ = create_op1_node(DCONCAT, $2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

F: 
    T Op3 { $$ = create_f_node($1, $2); }
    ;

Op3: 
    TIMES T Op3 { $$ = create_op3_node(TIMES, $2, $3); }
    | DIVIDE T Op3 { $$ = create_op3_node(DIVIDE, $2, $3); }
    | POWER T Op3 { $$ = create_op3_node(POWER, $2, $3); }
    | XOR T Op3 { $$ = create_op3_node(XOR, $2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

T: 
    NUMBER { $$ = create_number_node($1); }
    | IDENTIFIER TRest { $$ = create_identifier_node($1, $2); }
    | STRING_LIT { $$ = create_string_node($1); }
    | LPAREN Exp RPAREN { $$ = $2; }
    | MINUS T { $$ = create_unary_op_node(MINUS, $2); }
    | PI { $$ = create_pi_node(); }
    | E { $$ = create_e_node(); }
    | TRUE { $$ = create_boolean_node(1); }
    | FALSE { $$ = create_boolean_node(0); }
    | NOT T { $$ = create_unary_op_node(NOT, $2); }
    ;

TRest: 
    LPAREN ArgListCall RPAREN { $$ = create_function_call_node($2); }
    | ASSIGN Exp { $$ = create_assignment_node($2); }
    | /* epsilon */ { $$ = NULL; }
    ;

FunctionDef: 
    FUNCTION IDENTIFIER LPAREN ArgList RPAREN ARROW Exp SEMICOLON { $$ = create_function_def_node($2, $4, $7); }
    | FUNCTION IDENTIFIER LPAREN ArgList RPAREN Block { $$ = create_function_def_block_node($2, $4, $6); }
    ;

ArgList: 
    IDENTIFIER MoreArgs { $$ = create_arg_list_node($1, $2); }
    | /* epsilon */ { $$ = NULL; }
    ;

MoreArgs: 
    COMMA IDENTIFIER MoreArgs { $$ = create_more_args_node($2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

ArgListCall: 
    Exp MoreArgsCall { $$ = create_arg_list_call_node($1, $2); }
    | /* epsilon */ { $$ = NULL; }
    ;

MoreArgsCall: 
    COMMA Exp MoreArgsCall { $$ = create_more_args_call_node($2, $3); }
    | /* epsilon */ { $$ = NULL; }
    ;

%%

int main(int argc, char *argv[]) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir el archivo %s\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }
    
    yyparse();
    return 0;
}