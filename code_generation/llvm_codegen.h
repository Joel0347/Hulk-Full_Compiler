#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"

// Función principal de generación de código para nodos AST
LLVMValueRef codegen(ASTNode* node);

// Función para generar la función main y el código del programa
void generate_main_function(ASTNode* ast, const char* filename);

LLVMValueRef generate_code(ASTNode* node);
LLVMValueRef generate_function(ASTNode* node);
LLVMValueRef generate_if_statement(ASTNode* node);
LLVMValueRef generate_while_loop(ASTNode* node);
LLVMValueRef generate_let_in(ASTNode* node);
LLVMValueRef generate_assignment(ASTNode* node);
LLVMValueRef generate_variable(ASTNode* node);
LLVMValueRef generate_constant(ASTNode* node);

#endif // LLVM_CODEGEN_H