#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"
#include "../visitor/llvm_visitor.h"

LLVMTypeRef get_llvm_type(Type* type);
LLVMValueRef get_default(LLVM_Visitor* v, Type* type);

// Función para generar la función main y el código del programa
void generate_main_function(ASTNode* ast, const char* filename);
void find_function_dec(LLVM_Visitor* visitor, ASTNode* node);
void make_body_function_dec(LLVM_Visitor* visitor, ASTNode* node);

LLVMValueRef generate_program(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_number(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_string(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_boolean(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_block(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_assignment(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_variable(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_let_in(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef make_function_dec(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_conditional(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_q_conditional(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_loop(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef cast_value_to_type(LLVMValueRef value, Type* from_type, Type* to_type);

// Type-related codegen functions
LLVMValueRef generate_type_declaration(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_type_instance(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_field_access(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_set_attr(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_method_call(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_test_type(LLVM_Visitor* v, ASTNode* node); // is operator
LLVMValueRef generate_cast_type(LLVM_Visitor* v, ASTNode* node); // as operator
static int find_field_index(Type* type, const char* field_name);

#endif // LLVM_CODEGEN_H