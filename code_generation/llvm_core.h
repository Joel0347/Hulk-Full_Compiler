#ifndef LLVM_CORE_H
#define LLVM_CORE_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

extern LLVMModuleRef module;
extern LLVMBuilderRef builder;
extern LLVMContextRef context;

void init_llvm(void);
void free_llvm_resources(void);
void declare_external_functions(void);

#endif