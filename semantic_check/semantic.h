#include <stdlib.h>
#include "../ast/ast.h"
#include "../visitor/visitor.h"

int analyze_semantics(ASTNode* node);
Type* find_type(Visitor* v, ASTNode* node);
Type** find_types(ASTNode** args, int args_count);
int unify_op(Visitor* v, ASTNode* left, ASTNode* right, Operator op, char* op_name);
IntList* unify_func(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* f_name, ContextItem* item);
int unify_member(Visitor* v, ASTNode* node, Type* type);
void visit_program(Visitor* v, ASTNode* node);
void visit_assignment(Visitor* v, ASTNode* node);
void visit_variable(Visitor* v, ASTNode* node);
void visit_number(Visitor* v, ASTNode* node);
void visit_string(Visitor* v, ASTNode* node);
void visit_boolean(Visitor* v, ASTNode* node);
void visit_binary_op(Visitor* v, ASTNode* node);
void visit_unary_op(Visitor* v, ASTNode* node);
void visit_function_call(Visitor* v, ASTNode* node);
void visit_block(Visitor* v, ASTNode* node);
void visit_function_dec(Visitor* v, ASTNode* node);
void visit_let_in(Visitor* v, ASTNode* node);
void visit_conditional(Visitor* v, ASTNode* node);
void visit_loop(Visitor* v, ASTNode* node);