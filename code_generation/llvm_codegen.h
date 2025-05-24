#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"
#include "../visitor/llvm_visitor.h"

LLVMTypeRef get_llvm_type(Type* type);

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
LLVMValueRef generate_loop(LLVM_Visitor* v, ASTNode* node);

#endif // LLVM_CODEGEN_H