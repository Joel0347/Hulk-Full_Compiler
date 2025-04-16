#ifndef LLVM_GEN_H
#define LLVM_GEN_H

#include "../ast/ast.h"

void generate_llvm_code(ASTNode* ast, const char* filename);
void init_llvm();
void free_llvm_resources();

#endif