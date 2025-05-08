#ifndef LLVM_BUILTINS_H
#define LLVM_BUILTINS_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"
#include "../visitor/llvm_visitor.h"

// Genera código LLVM para funciones built-in como print, sqrt, sin, etc.
LLVMValueRef generate_builtin_function(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef print_function(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef log_function(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef rand_function(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef basic_functions(LLVM_Visitor* v, ASTNode* node, const char* name, const char* tmp_name);
LLVMValueRef generate_user_function_call(LLVM_Visitor* v, ASTNode* node);
LLVMValueRef generate_function_body(LLVM_Visitor* v, ASTNode* node);

// Declara las funciones built-in al inicio de la generación
void declare_builtin_functions(void);

#endif // LLVM_BUILTINS_H