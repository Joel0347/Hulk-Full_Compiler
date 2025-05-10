#ifndef LLVM_CORE_H
#define LLVM_CORE_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include "ast/ast.h"

#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"

extern LLVMModuleRef module;
extern LLVMBuilderRef builder;
extern LLVMContextRef context;
extern LLVMValueRef current_stack_depth_var;
extern const int MAX_STACK_DEPTH;

void init_llvm(void);
void free_llvm_resources(void);
void declare_external_functions(void);

static inline void handle_stack_overflow(
    LLVMBuilderRef builder, LLVMModuleRef module, 
    LLVMValueRef current_stack_depth_var, ASTNode* node
) {
    // Construir mensaje de error con nombre de función y línea
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), 
    RED"!!RUNTIME ERROR: Stack overflow detected in function '%s'. Line: %d.\n" RESET,
    node->data.func_node.name, 
    node->line);

    LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(builder, error_msg, "error_msg");

    // Llamar a puts para imprimir el mensaje
    LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
    LLVMBuildCall(builder, puts_func, &error_msg_global, 1, "");

    // Llamar a exit(1)
    LLVMValueRef exit_func = LLVMGetNamedFunction(module, "exit");
    LLVMValueRef exit_code = LLVMConstInt(LLVMInt32Type(), 0, 0);
    LLVMBuildCall(builder, exit_func, &exit_code, 1, "");

    // Marcar como unreachable
    LLVMBuildUnreachable(builder);
}

#endif