#include <stdlib.h>
#include "../ast/ast.h"
#include "../visitor/visitor.h"

int analyze_semantics(ASTNode* node);
Type* find_type(Visitor* v, ASTNode* node);
Type** find_types(ASTNode** args, int args_count);
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