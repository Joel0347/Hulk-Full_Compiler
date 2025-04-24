#include "llvm_codegen.h"
#include "llvm_core.h"
#include "llvm_scope.h"
#include "llvm_operators.h"
#include "llvm_string.h"
#include "llvm_builtins.h"
#include "../type/type.h"
#include <stdio.h>
#include <string.h>

LLVMValueRef codegen(ASTNode* node) {
    return generate_code(node);
}

LLVMValueRef generate_code(ASTNode* node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_PROGRAM: {
            push_scope(); // Crear scope global
            LLVMValueRef last = NULL;
            for (int i = 0; i < node->data.program_node.count; i++) {
                last = generate_code(node->data.program_node.statements[i]);
            }
            pop_scope();
            return last;
        }

        case NODE_NUMBER:
            return LLVMConstReal(LLVMDoubleType(), node->data.number_value);

        case NODE_VARIABLE: {
            LLVMValueRef alloca = lookup_variable(node->data.variable_name);
            if (!alloca) {
                exit(1);
            }
            LLVMTypeRef var_type = LLVMGetElementType(LLVMTypeOf(alloca));
            return LLVMBuildLoad2(builder, var_type, alloca, "load");
        }

        case NODE_ASSIGNMENT: {
            const char* var_name = node->data.op_node.left->data.variable_name;
            LLVMValueRef value = generate_code(node->data.op_node.right);
            
            LLVMTypeRef new_type;
            if (type_equals(node->data.op_node.right->return_type, &TYPE_STRING_INST)) {
                new_type = LLVMPointerType(LLVMInt8Type(), 0);
            } else if (type_equals(node->data.op_node.right->return_type, &TYPE_BOOLEAN_INST)) {
                new_type = LLVMInt1Type();
            } else {
                new_type = LLVMDoubleType();
            }

            LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
            LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(current_block));
            LLVMPositionBuilderAtEnd(builder, entry_block);

            LLVMValueRef existing_alloca = lookup_variable(var_name);
            LLVMValueRef alloca;

            if (existing_alloca) {
                LLVMTypeRef existing_type = LLVMGetElementType(LLVMTypeOf(existing_alloca));
                if (existing_type != LLVMTypeOf(value)) {
                    alloca = LLVMBuildAlloca(builder, new_type, var_name);
                    update_variable(var_name, alloca);
                } else {
                    alloca = existing_alloca;
                }
            } else {
                alloca = LLVMBuildAlloca(builder, new_type, var_name);
                declare_variable(var_name, alloca);
            }

            LLVMPositionBuilderAtEnd(builder, current_block);
            return LLVMBuildStore(builder, value, alloca);
        }

        case NODE_BINARY_OP:
            return generate_binary_operation(node);

        case NODE_UNARY_OP:
            return generate_unary_operation(node);

        case NODE_BOOLEAN: {
            int value = strcmp(node->data.string_value, "true") == 0 ? 1 : 0;
            return LLVMConstInt(LLVMInt1Type(), value, 0);  
        }

        case NODE_STRING: {
            char* processed = process_string_escapes(node->data.string_value);
            LLVMValueRef str = LLVMBuildGlobalStringPtr(builder, processed, "str");
            free(processed);
            return str;
        }

        case NODE_BUILTIN_FUNC:
            return generate_builtin_function(node);

        case NODE_BLOCK: {
            push_scope();
            LLVMValueRef last_val = NULL;
            for (int i = 0; i < node->data.program_node.count; i++) {
                last_val = generate_code(node->data.program_node.statements[i]);
            }
            if (last_val)
            {
                pop_scope();
                return last_val;
            }
            return 1;

        }

        default:
            exit(1);
    }
}

void generate_main_function(ASTNode* ast, const char* filename) {
    // Inicializar LLVM
    init_llvm();
    
    // Declarar funciones externas
    declare_external_functions();

    // Crear el scope
    push_scope();
    // Crear función main
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_type);
    
    // Crear bloque de entrada
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // Generar código para el AST
    if (ast != NULL) {
        if (ast->type == NODE_PROGRAM) {
            for (int i = 0; i < ast->data.program_node.count; i++) {
                LLVMValueRef stmt = generate_code(ast->data.program_node.statements[i]);
                if (!stmt) {
                    fprintf(stderr, "Error generando código para statement %d\n", i);
                }
            }
        } else {
            generate_code(ast);
        }
    }
    
    // Retornar 0 de main
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    
    // Escribir a archivo
    char* error = NULL;
    if (LLVMPrintModuleToFile(module, filename, &error)) {
        fprintf(stderr, "Error escribiendo IR: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }
    
    // Liberar recursos
    free_llvm_resources();
}