#ifndef LLVM_OPERATORS_H
#define LLVM_OPERATORS_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"

// Genera código LLVM para operaciones binarias (suma, resta, comparaciones, etc.)
LLVMValueRef generate_binary_operation(ASTNode* node);

// Genera código LLVM para operaciones unarias (negación, not)
LLVMValueRef generate_unary_operation(ASTNode* node);

#endif // LLVM_OPERATORS_H