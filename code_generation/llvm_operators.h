#ifndef LLVM_OPERATORS_H
#define LLVM_OPERATORS_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"
#include "../visitor/llvm_visitor.h"

// Genera código LLVM para operaciones binarias (suma, resta, comparaciones, etc.)
LLVMValueRef generate_binary_operation(LLVM_Visitor* v, ASTNode* node);

// Genera código LLVM para operaciones unarias (negación, not)
LLVMValueRef generate_unary_operation(LLVM_Visitor* v, ASTNode* node);

#endif // LLVM_OPERATORS_H