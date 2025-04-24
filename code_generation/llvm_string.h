#ifndef LLVM_STRING_H
#define LLVM_STRING_H

#include <llvm-c/Core.h>

// Procesa los caracteres de escape en un string
char* process_string_escapes(const char* input);

// Genera código LLVM para concatenación de strings
LLVMValueRef generate_string_concatenation(LLVMValueRef L, LLVMValueRef R, int is_double_concat);

#endif // LLVM_STRING_H