#include <stdlib.h>
#include "../ast/ast.h"
#include "../visitor/visitor.h"

extern char* keywords[]; // keywords of the language
extern char scape_chars[]; //scapes characters defined

int match_as_keyword(char* name);
int is_scape_char(char c);
int analyze_semantics(ASTNode* node);
int unify_op(Visitor* v, ASTNode* left, ASTNode* right, Operator op, char* op_name);
int unify_conditional(Visitor* v, ASTNode* node, Type* type);
int unify_type_by_attr(Visitor* v, ASTNode* node);
IntList* unify_func(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* f_name, ContextItem* item);
IntList* unify_type(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* t_name, ContextItem* item);
int unify(Visitor* v, ASTNode* node, Type* type);
void check_function_call(Visitor* v, ASTNode* node, Type* type);
void check_function_dec(Visitor* v, ASTNode* node, Type* type);
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
void visit_q_conditional(Visitor* v, ASTNode* node);
void visit_loop(Visitor* v, ASTNode* node);
void visit_for_loop(Visitor* v, ASTNode* node);
void visit_type_dec(Visitor* v, ASTNode* node);
void visit_type_instance(Visitor* v, ASTNode* node);
void visit_test_type(Visitor* v, ASTNode* node);
void visit_casting_type(Visitor* v, ASTNode* node);
void visit_attr_getter(Visitor* v, ASTNode* node);
void visit_attr_setter(Visitor* v, ASTNode* node);
void visit_base_func(Visitor* v, ASTNode* node);