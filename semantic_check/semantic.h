#include "../ast/ast.h"
#include "../visitor/visitor.h"

int analyze_semantics(ASTNode* node);
Type* type_checker(Visitor* v, ASTNode* node);
static void visit_program(Visitor* v, ASTNode* node);
static void visit_assignment(Visitor* v, ASTNode* node);
static void visit_variable(Visitor* v, ASTNode* node);
static void visit_number(Visitor* v, ASTNode* node);
static void visit_string(Visitor* v, ASTNode* node);
static void visit_boolean(Visitor* v, ASTNode* node);
static void visit_binary_op(Visitor* v, ASTNode* node);
static void visit_unary_op(Visitor* v, ASTNode* node);
static void visit_print(Visitor* v, ASTNode* node);
static void visit_builtin_func_call(Visitor* v, ASTNode* node);