#include "llvm_codegen.h"
#include "llvm_core.h"
#include "../scope/llvm_scope.h"
#include "llvm_operators.h"
#include "llvm_string.h"
#include "llvm_builtins.h"
#include "../type/type.h"
#include <stdio.h>
#include <string.h>

LLVMTypeRef get_llvm_type(Type* type) {
    if (type_equals(type, &TYPE_NUMBER_INST)) {
        return LLVMDoubleType();
    } else if (type_equals(type, &TYPE_STRING_INST)) {
        return LLVMPointerType(LLVMInt8Type(), 0);
    } else if (type_equals(type, &TYPE_BOOLEAN_INST)) {
        return LLVMInt1Type();
    } else if (type_equals(type, &TYPE_VOID_INST)) {
        return LLVMVoidType();
    }
    exit(1);
}

void generate_main_function(ASTNode* ast, const char* filename) {
    LLVM_Visitor visitor = {
        .visit_program = generate_program,
        .visit_assignment = generate_assignment,
        .visit_binary_op = generate_binary_operation,
        .visit_number = generate_number,
        .visit_function_call = generate_builtin_function,
        .visit_string = generate_string,
        .visit_boolean = generate_boolean,
        .visit_unary_op = generate_unary_operation,
        .visit_variable = generate_variable,
        .visit_block = generate_block,
        .visit_function_dec = generate_function_body
    };

    // Inicializar LLVM
    init_llvm();
    
    // Declarar funciones externas
    declare_external_functions();

    find_function_dec(&visitor, ast);

    make_body_function_dec(&visitor, ast);

    // Crear el scope
    push_scope();
    // Crear función main
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_type);
    
    // Crear bloque de entrada
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // Generar código para el AST
    if (ast) {
        accept_gen(&visitor, ast);
    }

    // Retornar 0 de main
    LLVMPositionBuilderAtEnd(builder, entry); // Resetear a bloque de main
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

LLVMValueRef generate_program(LLVM_Visitor* v, ASTNode* node) {
    push_scope();
    LLVMValueRef last = NULL;

    for (int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* stmt = node->data.program_node.statements[i];
        if (stmt->type != NODE_FUNC_DEC) {
            last = accept_gen(v, stmt);
        }
    }

    pop_scope();
    return last ? last : LLVMConstInt(LLVMInt32Type(), 0, 0);
}

LLVMValueRef generate_number(LLVM_Visitor* v,ASTNode* node) {
    return LLVMConstReal(LLVMDoubleType(), node->data.number_value);
}

LLVMValueRef generate_string(LLVM_Visitor* v,ASTNode* node) {
    char* processed = process_string_escapes(node->data.string_value);
    LLVMValueRef str = LLVMBuildGlobalStringPtr(builder, processed, "str");
    free(processed);
    return str;
}

LLVMValueRef generate_boolean(LLVM_Visitor* v,ASTNode* node) {
    int value = strcmp(node->data.string_value, "true") == 0 ? 1 : 0;
    return LLVMConstInt(LLVMInt1Type(), value, 0);  
}

LLVMValueRef generate_block(LLVM_Visitor* v,ASTNode* node) {
    push_scope();
    LLVMValueRef last_val = NULL;
    for (int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* stmt = node->data.program_node.statements[i];
        // No generar código para declaraciones de función aquí
        if (stmt->type != NODE_FUNC_DEC) {
            last_val = accept_gen(v, stmt);
        }
    }
    pop_scope();
    return last_val ? last_val : LLVMConstInt(LLVMInt32Type(), 0, 0);
}

LLVMValueRef generate_assignment(LLVM_Visitor* v,ASTNode* node) {
    const char* var_name = node->data.op_node.left->data.variable_name;
    LLVMValueRef value = accept_gen(v, node->data.op_node.right);
    
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

LLVMValueRef generate_variable(LLVM_Visitor* v,ASTNode* node) {
    LLVMValueRef alloca = lookup_variable(node->data.variable_name);
    if (!alloca) {
        exit(1);
    }
    LLVMTypeRef var_type = LLVMGetElementType(LLVMTypeOf(alloca));
    return LLVMBuildLoad2(builder, var_type, alloca, "load");
}

void find_function_dec(LLVM_Visitor* visitor, ASTNode* node) {
    if (node->type == NODE_PROGRAM || node->type == NODE_BLOCK) {
        for (int i = 0; i < node->data.program_node.count; i++) {
            ASTNode* stmt = node->data.program_node.statements[i];
            if (stmt->type == NODE_FUNC_DEC) {
                make_function_dec(visitor, stmt);
            }
            if (stmt->type == NODE_BLOCK) {
                find_function_dec(visitor, stmt);
            }
        }
    }
}

void make_body_function_dec(LLVM_Visitor* visitor, ASTNode* node) {
    if (node->type == NODE_PROGRAM || node->type == NODE_BLOCK) {
        for (int i = 0; i < node->data.program_node.count; i++) {
            ASTNode* stmt = node->data.program_node.statements[i];
            if (stmt->type == NODE_FUNC_DEC) {
                accept_gen(visitor, stmt);
            }
            if (stmt->type == NODE_BLOCK) {
                make_body_function_dec(visitor, stmt);
            }
        }
    }
}

LLVMValueRef make_function_dec(LLVM_Visitor* v, ASTNode* node) {
    const char* name = node->data.func_node.name;
    Type* return_type = node->data.func_node.body->return_type;
    ASTNode** params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;

    // Obtener tipos de parámetros
    LLVMTypeRef* param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++) {
        param_types[i] = get_llvm_type(params[i]->return_type);
    }

    // Crear y registrar la firma de la función
    LLVMTypeRef func_type = LLVMFunctionType(
        get_llvm_type(return_type),
        param_types,
        param_count,
        0
    );
    
    LLVMValueRef func = LLVMAddFunction(module, name, func_type);
    free(param_types);
    return func;
}

LLVMValueRef generate_function_body(LLVM_Visitor* v, ASTNode* node) {
    const char* name = node->data.func_node.name;
    Type* return_type = node->data.func_node.body->return_type;
    ASTNode** params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;
    ASTNode* body = node->data.func_node.body;

    // Obtener tipos de parámetros
    LLVMTypeRef* param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++) {
        param_types[i] = get_llvm_type(params[i]->return_type);
    }

    LLVMValueRef func = LLVMGetNamedFunction(module, name);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");

    // Guardar contexto previo del builder
    LLVMBasicBlockRef prev_block = LLVMGetInsertBlock(builder);
    LLVMValueRef prev_function = NULL;

    if (prev_block) {
        prev_function = LLVMGetBasicBlockParent(prev_block);
    }

    // Configurar builder para la función actual
    LLVMPositionBuilderAtEnd(builder, entry);
            
    push_scope();

    // Almacenar parámetros en variables locales
    for (int i = 0; i < param_count; i++) {
        LLVMValueRef param = LLVMGetParam(func, i);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, param_types[i], params[i]->data.variable_name);
        LLVMBuildStore(builder, param, alloca);
        declare_variable(params[i]->data.variable_name, alloca);
    }
    // Generar código para el cuerpo
    LLVMValueRef body_val = accept_gen(v, body);

    // Retornar
    if (type_equals(return_type, &TYPE_VOID_INST)) {
        LLVMBuildRetVoid(builder);
    } else {
        LLVMBuildRet(builder, body_val);
    }

    // Restaurar scope
    pop_scope();
    free(param_types);

    // Restaurar contexto anterior
    if (prev_function && LLVMGetBasicBlockParent(entry) != prev_function) {
        LLVMPositionBuilderAtEnd(builder, prev_block);
    }

    return func;
}