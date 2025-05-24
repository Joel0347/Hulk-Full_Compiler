#include "llvm_codegen.h"
#include "llvm_core.h"
#include "../scope/llvm_scope.h"
#include "llvm_operators.h"
#include "llvm_string.h"
#include "llvm_builtins.h"
#include "../type/type.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

LLVMTypeRef get_llvm_type(Type* type) {
    if (type_equals(type, &TYPE_NUMBER)) {
        return LLVMDoubleType();
    } else if (type_equals(type, &TYPE_STRING)) {
        return LLVMPointerType(LLVMInt8Type(), 0);
    } else if (type_equals(type, &TYPE_BOOLEAN)) {
        return LLVMInt1Type();
    } else if (type_equals(type, &TYPE_VOID)) {
        return LLVMVoidType();
    } else if (type_equals(type, &TYPE_OBJECT)) {
        // usamos un puntero void* que puede apuntar a cualquier tipo
        return LLVMPointerType(LLVMInt8Type(), 0);
    }

    fprintf(stderr, "Error: Tipo desconocido %s\n", type->name);
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
        .visit_function_dec = generate_function_body,
        .visit_let_in = generate_let_in,
        .visit_conditional = generate_conditional,
        .visit_loop = generate_loop
    };

    // Initialize LLVM
    init_llvm();
    
    // Declare external functions
    declare_external_functions();
    find_function_dec(&visitor, ast);
    make_body_function_dec(&visitor, ast);
    // Create scope
    push_scope();
    
    // Create main function
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_type);
    
    // Create entry block
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // Generate code for AST
    if (ast) {
        accept_gen(&visitor, ast);
    }

    
    // Make sure we're in the right block for the return
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
    if (!LLVMGetBasicBlockTerminator(current_block)) {
        // Return 0 from main if the block isn't already terminated
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    }
    
    // Write to file
    char* error = NULL;
    if (LLVMPrintModuleToFile(module, filename, &error)) {
        fprintf(stderr, "Error writing IR: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }
    
    // Free resources
    free_llvm_resources();
}

LLVMValueRef generate_program(LLVM_Visitor* v, ASTNode* node) {
    push_scope();
    LLVMValueRef last = NULL;

    for (int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* stmt = node->data.program_node.statements[i];
        if (stmt->type != NODE_FUNC_DEC) {
            last= accept_gen(v, stmt);
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
        if (stmt->type != NODE_FUNC_DEC) {
            last_val= accept_gen(v, stmt);
        }
    }
    if (last_val)
    {
        pop_scope();
    }
    return last_val ;
}

LLVMValueRef generate_assignment(LLVM_Visitor* v, ASTNode* node) {
    const char* var_name = node->data.op_node.left->data.variable_name;
    LLVMValueRef value = accept_gen(v, node->data.op_node.right);
    
    LLVMTypeRef new_type;
    if (type_equals(node->data.op_node.right->return_type, &TYPE_STRING)) {
        new_type = LLVMPointerType(LLVMInt8Type(), 0);
    } else if (type_equals(node->data.op_node.right->return_type, &TYPE_BOOLEAN)) {
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
        // Para asignación destructiva (:=), actualizar en todos los scopes
        if (node->type == NODE_D_ASSIGNMENT) {
            update_variable(var_name, existing_alloca);
        }
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
    LLVMBuildStore(builder, value, alloca);
    
    // Para asignación destructiva (:=), retornar el valor asignado
    if (node->type == NODE_D_ASSIGNMENT) {
        return value;
    }
    return NULL;
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
    if (!node) return;

    // Si es una declaración de función, procesarla y buscar dentro de su cuerpo
    if (node->type == NODE_FUNC_DEC) {
        make_function_dec(visitor, node);
        // Buscar funciones anidadas dentro del cuerpo de la función
        find_function_dec(visitor, node->data.func_node.body);
        return;
    }
    
    // Recursivamente buscar en los diferentes tipos de nodos
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (int i = 0; i < node->data.program_node.count; i++) {
                find_function_dec(visitor, node->data.program_node.statements[i]);
            }
            break;
        
        case NODE_LET_IN:
            // Buscar en las declaraciones
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT) {
                    find_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
                }
            }
            // Buscar en el cuerpo
            find_function_dec(visitor, node->data.func_node.body);
            break;
    }
}

void make_body_function_dec(LLVM_Visitor* visitor, ASTNode* node) {
    if (!node) return;

    // Si es una declaración de función, generar su cuerpo y procesar funciones anidadas
    if (node->type == NODE_FUNC_DEC) {
        accept_gen(visitor, node);
        // Procesar funciones anidadas en el cuerpo de la función
        make_body_function_dec(visitor, node->data.func_node.body);
        return;
    }

    // Recursivamente procesar los diferentes tipos de nodos
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (int i = 0; i < node->data.program_node.count; i++) {
                make_body_function_dec(visitor, node->data.program_node.statements[i]);
            }
            break;
        
        case NODE_LET_IN:
            // Procesar declaraciones 
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT) {
                    make_body_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
                }
            }
            // Procesar el cuerpo
            make_body_function_dec(visitor, node->data.func_node.body);
            break;
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
    LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(func, "function_exit");

    // Configurar builder
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // 1. Stack depth handling
    // Increment counter
    LLVMTypeRef int32_type = LLVMInt32Type();
    LLVMValueRef depth_val = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, depth_val, LLVMConstInt(int32_type, 1, 0), "inc_depth");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);
    
    // Verificar overflow
    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT, new_depth, 
                                    LLVMConstInt(LLVMInt32Type(), MAX_STACK_DEPTH, 0), "cmp_overflow");
    
    // Crear bloques para manejo de error
    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(func, "stack_overflow");
    LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(func, "func_body");
    LLVMBuildCondBr(builder, cmp, error_block, continue_block);
    
    // Bloque de error
    LLVMPositionBuilderAtEnd(builder, error_block);
    handle_stack_overflow(builder, module, current_stack_depth_var, node);
    
    // Continuar con función normal
    LLVMPositionBuilderAtEnd(builder, continue_block);
            
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

    // Branch al bloque de salida
    LLVMBuildBr(builder, exit_block);

    // Bloque de salida
    LLVMPositionBuilderAtEnd(builder, exit_block);
    
    // Decrementar contador antes de retornar
    LLVMValueRef final_depth = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth_final");
    LLVMValueRef dec_depth = LLVMBuildSub(builder, final_depth, LLVMConstInt(int32_type, 1, 0), "dec_depth");
    LLVMBuildStore(builder, dec_depth, current_stack_depth_var);
    
    // Return value handling
    if (type_equals(return_type, &TYPE_VOID)) {
        LLVMBuildRetVoid(builder);
    } else if (body_val) {
        // If we have a return value, use it
        LLVMBuildRet(builder, body_val);
    } else {
        // Default return 0.0 as double
        LLVMBuildRet(builder, LLVMConstReal(LLVMDoubleType(), 0.0));
    }

    pop_scope();
    free(param_types);

    return func;
}

LLVMValueRef generate_let_in(LLVM_Visitor* v, ASTNode* node) {
    push_scope();
    // Procesar las declaraciones
    ASTNode** declarations = node->data.func_node.args;
    int dec_count = node->data.func_node.arg_count;
    
    // Evaluar y almacenar cada declaración
    for (int i = 0; i < dec_count; i++) {
        ASTNode* decl = declarations[i];
        const char* var_name = decl->data.op_node.left->data.variable_name;
        LLVMValueRef value = accept_gen(v, decl->data.op_node.right);
        if (!value)
        {
           return 1;
        }
        // Crear alloca para la variable
        LLVMTypeRef var_type = get_llvm_type(decl->data.op_node.right->return_type);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, var_type, var_name);
        LLVMBuildStore(builder, value, alloca);
        declare_variable(var_name, alloca);
    }
    
    // Obtener el bloque actual antes de evaluar el cuerpo
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);

    // Evaluar el cuerpo
    LLVMValueRef result = accept_gen(v, node->data.func_node.body);
    
    // Asegurar que estamos en el bloque correcto después de la evaluación
    LLVMPositionBuilderAtEnd(builder, LLVMGetInsertBlock(builder));
    
    pop_scope();
    return result;
}

LLVMValueRef generate_conditional(LLVM_Visitor* v, ASTNode* node) {
    ASTNode* condition = node->data.cond_node.cond;
    ASTNode* true_body = node->data.cond_node.body_true;
    ASTNode* false_body = node->data.cond_node.body_false;

    // Get current function
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    // Create basic blocks
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "then");
    LLVMBasicBlockRef else_block = false_body ? LLVMAppendBasicBlock(current_function, "else") : NULL;
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge");

    // Generate condition code
    LLVMValueRef cond_val = accept_gen(v, condition);

    // Create conditional branch
    LLVMBuildCondBr(builder, cond_val, then_block, else_block ? else_block : merge_block);

    // Generate 'then' block
    LLVMPositionBuilderAtEnd(builder, then_block);
    LLVMValueRef then_val = accept_gen(v, true_body);
    LLVMBuildBr(builder, merge_block);

    // Get updated then_block for PHI
    then_block = LLVMGetInsertBlock(builder);

    // Generate 'else' block if it exists
    LLVMValueRef else_val = NULL;
    if (false_body) {
        LLVMPositionBuilderAtEnd(builder, else_block);
        else_val = accept_gen(v, false_body);
        LLVMBuildBr(builder, merge_block);
        else_block = LLVMGetInsertBlock(builder);
    }

    // Generate merge block with PHI node if needed
    LLVMPositionBuilderAtEnd(builder, merge_block);
    
    if (type_equals(node->return_type, &TYPE_VOID)) {
        return NULL;
    }
    
    // Create PHI node to merge values
    LLVMTypeRef phi_type = get_llvm_type(node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(builder, phi_type, "if_result");
    
    // Add incoming values to PHI
    LLVMValueRef incoming_values[] = {then_val, else_val ? else_val : LLVMConstNull(phi_type)};
    LLVMBasicBlockRef incoming_blocks[] = {then_block, else_block ? else_block : merge_block};
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, else_block ? 2 : 1);
    
    return phi;
}

LLVMValueRef generate_loop(LLVM_Visitor* v, ASTNode* node) {
    // Obtener la función actual
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    // Determinar el tipo del cuerpo del ciclo
    LLVMTypeRef body_type = get_llvm_type(node->data.op_node.right->return_type);

    // Solo si el cuerpo no es void, reservar una variable temporal para almacenar el valor
    LLVMValueRef result_addr = NULL;
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind) {
        result_addr = LLVMBuildAlloca(builder, body_type, "while.result.addr");
        // Inicializamos con un valor nulo del tipo correspondiente
        LLVMBuildStore(builder, LLVMConstNull(body_type), result_addr);
    }

    // Crear bloques básicos: condición, cuerpo y merge
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_function, "while.cond");
    LLVMBasicBlockRef loop_block = LLVMAppendBasicBlock(current_function, "while.body");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "while.end");

    // Saltar al bloque de condición
    LLVMBuildBr(builder, cond_block);

    // Bloque de condición: evalúa la condición del ciclo
    LLVMPositionBuilderAtEnd(builder, cond_block);
    LLVMValueRef cond_val = accept_gen(v, node->data.op_node.left);
    LLVMBuildCondBr(builder, cond_val, loop_block, merge_block);

    // Bloque del cuerpo del ciclo: se evalúa la expresión del cuerpo
    LLVMPositionBuilderAtEnd(builder, loop_block);
    LLVMValueRef body_val = accept_gen(v, node->data.op_node.right);
    // Si el cuerpo no es void, se almacena el valor resultante
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind) {
        LLVMBuildStore(builder, body_val, result_addr);
    }
    // Volver a la condición para la siguiente iteración
    LLVMBuildBr(builder, cond_block);

    // Bloque de merge: una vez que la condición es falsa, se continúa
    LLVMPositionBuilderAtEnd(builder, merge_block);
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind) {
        LLVMValueRef final_val = LLVMBuildLoad2(builder, body_type, result_addr, "while.result");
        return final_val;
    } else {
        // Si el cuerpo es void, simplemente retornamos NULL
        return NULL;
    }
}
