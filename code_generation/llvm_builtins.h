#ifndef LLVM_BUILTINS_H
#define LLVM_BUILTINS_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"

// Genera código LLVM para funciones built-in como print, sqrt, sin, etc.
LLVMValueRef generate_builtin_function(ASTNode* node);

// Declara las funciones built-in al inicio de la generación
void declare_builtin_functions(void);

#endif // LLVM_BUILTINS_H