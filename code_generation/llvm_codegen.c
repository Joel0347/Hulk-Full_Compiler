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
    if (type->sub_type) {
        return get_llvm_type(type->sub_type);
    }

    if (type_equals(type, &TYPE_NUMBER)) {
        return LLVMDoubleType();
    } else if (type_equals(type, &TYPE_STRING)) {
        return LLVMPointerType(LLVMInt8Type(), 0);
    } else if (type_equals(type, &TYPE_BOOLEAN)) {
        return LLVMInt1Type();
    } else if (type_equals(type, &TYPE_VOID)) {
        return LLVMVoidType();
    } else if (type_equals(type, &TYPE_OBJECT) || type_equals(type, &TYPE_NULL)) {
        return LLVMPointerType(object_type, 0);
    } else if (type->dec != NULL) {
        // Es un tipo personalizado
        LLVMTypeRef struct_type = LLVMGetTypeByName(module, type->name);
        if (!struct_type) {
            fprintf(stderr, "Error: Tipo %s no encontrado\n", type->name);
            exit(1);
        }
        // Retornamos un puntero al tipo estructurado
        return LLVMPointerType(struct_type, 0);
    }
    
    fprintf(stderr, "Error: Tipo desconocido %s\n", type->name);
    exit(1);
}

LLVMValueRef get_default(LLVM_Visitor* v, Type* type) {
    if (type->sub_type) {
        return get_llvm_type(type->sub_type);
    }

    if (type_equals(type, &TYPE_NUMBER)) {
        return generate_number(v, create_number_node(0));
    } else if (type_equals(type, &TYPE_STRING)) {
        return generate_string(v, create_string_node(""));
    } else if (type_equals(type, &TYPE_BOOLEAN)) {
        return generate_boolean(v, create_boolean_node("false"));
    } else if (type_equals(type, &TYPE_VOID)) {
        return generate_block(v, create_program_node(NULL, 0, NODE_BLOCK));
    } else if (type_equals(type, &TYPE_OBJECT) || type_equals(type, &TYPE_NULL)) {
        // Usamos LLVMConstNull para retornar un puntero nulo del tipo object
        return LLVMConstNull(LLVMPointerType(object_type, 0));
    } else if (type->dec != NULL) {
        // Es un tipo personalizado
        LLVMTypeRef struct_type = LLVMGetTypeByName(module, type->name);
        if (!struct_type) {
            fprintf(stderr, "Error: Tipo %s no encontrado\n", type->name);
            exit(1);
        }
        // Retornamos un puntero nulo al tipo estructurado para tipos definidos por el usuario
        return LLVMConstNull(LLVMPointerType(struct_type, 0));
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
        .visit_q_conditional = generate_q_conditional,
        .visit_loop = generate_loop,
        .visit_type_dec = generate_type_declaration,
        .visit_type_inst = generate_type_instance,
        .visit_type_get_attr = generate_field_access,
        .visit_type_set_attr = generate_set_attr,
        .visit_type_method = generate_method_call,
        .visit_type_test = generate_test_type,  // For 'is' operator
        .visit_type_cast = generate_cast_type
    };
    init_llvm();
    
    
    // Declare external functions
    declare_external_functions();
    
    // Process function and type declarations
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
    
    if (node->type == NODE_D_ASSIGNMENT) {
        return value;
    }
    return NULL;
}

LLVMValueRef generate_variable(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef alloca = lookup_variable(node->data.variable_name);
    if (!alloca) {
        fprintf(stderr, "Error: Variable '%s' no declarada\n", node->data.variable_name);
        exit(1);
    }

    if (node->return_type && node->return_type->dec != NULL) {
        return alloca; 
    }
    else {
        LLVMTypeRef var_type = LLVMGetElementType(LLVMTypeOf(alloca));
        return LLVMBuildLoad2(builder, var_type, alloca, "load");
    }
}

void find_function_dec(LLVM_Visitor* visitor, ASTNode* node) {
    if (!node) return;

    if (node->type == NODE_FUNC_DEC) {
        make_function_dec(visitor, node);
        find_function_dec(visitor, node->data.func_node.body);
        return;
    }
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (int i = 0; i < node->data.program_node.count; i++) {
                find_function_dec(visitor, node->data.program_node.statements[i]);
            }
            break;
        
        case NODE_LET_IN:
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT) {
                    find_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
                }
            }
            find_function_dec(visitor, node->data.func_node.body);
            break;
    }
}

void make_body_function_dec(LLVM_Visitor* visitor, ASTNode* node) {
    if (!node) return;

    if (node->type == NODE_FUNC_DEC) {
        accept_gen(visitor, node);
        make_body_function_dec(visitor, node->data.func_node.body);
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (int i = 0; i < node->data.program_node.count; i++) {
                make_body_function_dec(visitor, node->data.program_node.statements[i]);
            }
            break;
        
        case NODE_LET_IN:
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT) {
                    make_body_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
                }
            }

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

    LLVMTypeRef* param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++) {
        param_types[i] = get_llvm_type(params[i]->return_type);
    }

    LLVMValueRef func = LLVMGetNamedFunction(module, name);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(func, "function_exit");

    LLVMPositionBuilderAtEnd(builder, entry);
    
    LLVMTypeRef int32_type = LLVMInt32Type();
    LLVMValueRef depth_val = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, depth_val, LLVMConstInt(int32_type, 1, 0), "inc_depth");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);
    
    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT, new_depth, 
                                    LLVMConstInt(LLVMInt32Type(), MAX_STACK_DEPTH, 0), "cmp_overflow");
    
    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(func, "stack_overflow");
    LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(func, "func_body");
    LLVMBuildCondBr(builder, cmp, error_block, continue_block);
    
    LLVMPositionBuilderAtEnd(builder, error_block);
    handle_stack_overflow(
        builder, module, current_stack_depth_var,
        node->line, node->data.func_node.name 
    );
    
    LLVMPositionBuilderAtEnd(builder, continue_block);
            
    push_scope();

    for (int i = 0; i < param_count; i++) {
        LLVMValueRef param = LLVMGetParam(func, i);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, param_types[i], params[i]->data.variable_name);
        LLVMBuildStore(builder, param, alloca);
        declare_variable(params[i]->data.variable_name, alloca);
    }
    LLVMValueRef body_val = accept_gen(v, body);

    LLVMBuildBr(builder, exit_block);

    LLVMPositionBuilderAtEnd(builder, exit_block);
    
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
    ASTNode** declarations = node->data.func_node.args;
    int dec_count = node->data.func_node.arg_count;
    
    for (int i = 0; i < dec_count; i++) {
        ASTNode* decl = declarations[i];
        const char* var_name = decl->data.op_node.left->data.variable_name;
        LLVMValueRef value = accept_gen(v, decl->data.op_node.right);
        if (!value)
        {
           return 1;
        }

        LLVMTypeRef var_type = get_llvm_type(decl->data.op_node.right->return_type);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, var_type, var_name);
        LLVMBuildStore(builder, value, alloca);
        declare_variable(var_name, alloca);
    }

    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
    LLVMValueRef result = accept_gen(v, node->data.func_node.body);
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
    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "else");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge");

    // Generate condition code
    LLVMValueRef cond_val = accept_gen(v, condition);

    // Create conditional branch
    LLVMBuildCondBr(builder, cond_val, then_block, else_block ? else_block : merge_block);

    // Generate 'then' block
    LLVMPositionBuilderAtEnd(builder, then_block);
    LLVMValueRef then_val = accept_gen(v, true_body);
    
    // Cast then_val to the return type if necessary
    if (!type_equals(true_body->return_type, node->return_type)) {
        then_val = cast_value_to_type(then_val, true_body->return_type, node->return_type);
    }
    
    LLVMBuildBr(builder, merge_block);

    // Get updated then_block for PHI
    then_block = LLVMGetInsertBlock(builder);

    // Generate 'else' block if it exists
    LLVMValueRef else_val = NULL;
    LLVMBasicBlockRef last_else_block = NULL;
    LLVMPositionBuilderAtEnd(builder, else_block);
    else_val = false_body? accept_gen(v, false_body) : get_default(v, true_body->return_type);
    Type* false_type = false_body? false_body->return_type : NULL;

    if (!false_type) {
        if (is_builtin_type(true_body->return_type) &&
            !type_equals(true_body->return_type, &TYPE_OBJECT)
        ) {
            false_type = true_body->return_type;
        } else {
            false_type = &TYPE_NULL;
        }
    }

    // Cast else_val to the return type if necessary
    if (!type_equals(false_type, node->return_type)) {
        else_val = cast_value_to_type(else_val, false_type, node->return_type);
    }
    
    LLVMBuildBr(builder, merge_block);
    last_else_block = LLVMGetInsertBlock(builder);

    // Generate merge block with PHI node if needed
    LLVMPositionBuilderAtEnd(builder, merge_block);
    
    if (type_equals(node->return_type, &TYPE_VOID)) {
        return NULL;
    }
    
    // Create PHI node with the correct type
    LLVMTypeRef phi_type = get_llvm_type(node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(builder, phi_type, "if_result");
    
    // Add incoming values to PHI with proper null values for missing branches
    LLVMValueRef incoming_values[2] = { 
        then_val ? then_val : LLVMConstNull(phi_type),
        else_val ? else_val : LLVMConstNull(phi_type)
    };
    LLVMBasicBlockRef incoming_blocks[2] = { then_block, last_else_block };
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
    
    return phi;
}

LLVMValueRef generate_q_conditional(LLVM_Visitor* v, ASTNode* node) {
    // Extraemos los nodos correspondientes
    ASTNode* condition = node->data.cond_node.cond;
    ASTNode* true_body = node->data.cond_node.body_true;
    ASTNode* false_body = node->data.cond_node.body_false;

    // Obtenemos la función actual a partir del bloque insertado
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    // Creamos tres bloques: then, else y merge
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "q_then");
    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "q_else");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "q_merge");

    // Generamos el código para la condición
    LLVMValueRef cond_expr = accept_gen(v, condition);

    // Para verificar "if (x is null)" de forma coherente:
    // - Si "cond_expr" es un puntero, usamos LLVMBuildIsNull.
    // - Si no es un puntero (por ejemplo, un número), asumimos que nunca es null.
    LLVMValueRef cond_bool = NULL;
    LLVMTypeRef cond_type = LLVMTypeOf(cond_expr);
    if (LLVMGetTypeKind(cond_type) == LLVMPointerTypeKind) {
        cond_bool = LLVMBuildIsNull(builder, cond_expr, "is_null");
    } else {
        // Para tipos que no son punteros, definimos la condición como siempre falsa.
        cond_bool = LLVMConstInt(LLVMInt1Type(), 0, 0);
    }

    // Construimos la bifurcación condicional basada en cond_bool.
    LLVMBuildCondBr(builder, cond_bool, then_block, else_block);

    // --- Bloque 'then' (cuerpo true) ---
    LLVMPositionBuilderAtEnd(builder, then_block);
    LLVMValueRef then_val = accept_gen(v, true_body);
    // Realizamos conversiones/casts si es necesario para que el tipo encaje.
    if (then_val && !type_equals(true_body->return_type, node->return_type)) {
        then_val = cast_value_to_type(then_val, true_body->return_type, node->return_type);
    }
    LLVMBuildBr(builder, merge_block);
    then_block = LLVMGetInsertBlock(builder);

    // --- Bloque 'else' (cuerpo false) ---
    LLVMPositionBuilderAtEnd(builder, else_block);
    LLVMValueRef else_val = accept_gen(v, false_body);
    // Realizamos el cast si es necesario.
    if (else_val && !type_equals(false_body->return_type, node->return_type)) {
        else_val = cast_value_to_type(else_val, false_body->return_type, node->return_type);
    }
    LLVMBuildBr(builder, merge_block);
    LLVMBasicBlockRef last_else_block = LLVMGetInsertBlock(builder);

    // --- Bloque de merge ---
    LLVMPositionBuilderAtEnd(builder, merge_block);
    
    // Si el tipo de retorno es void, no es necesario crear un PHI
    if (type_equals(node->return_type, &TYPE_VOID)) {
        return NULL;
    }
    
    // Creamos el nodo PHI para unificar los valores de ambos bloques,
    // asegurándonos de que se retorne el tipo esperado.
    LLVMTypeRef phi_type = get_llvm_type(node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(builder, phi_type, "q_if_result");
    
    // Agregamos las entradas del PHI: una de entonces y otra de else.
    LLVMValueRef incoming_values[2] = { 
        then_val ? then_val : LLVMConstNull(phi_type),
        else_val ? else_val : LLVMConstNull(phi_type)
    };
    LLVMBasicBlockRef incoming_blocks[2] = { then_block, last_else_block };
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

    return phi;
}

// Helper function to cast values between types
LLVMValueRef cast_value_to_type(LLVMValueRef value, Type* from_type, Type* to_type) {
    // If types are equal or value is NULL, no casting needed
    if (from_type->sub_type)
        from_type = from_type->sub_type;
    if (to_type->sub_type)
        to_type = to_type->sub_type;

    if (!value || type_equals(from_type, to_type)) {
        return value;
    }

    // Handle primitive type conversions first
    if (is_builtin_type(to_type)) {
        if (type_equals(to_type, &TYPE_NUMBER)) {
            if (type_equals(from_type, &TYPE_BOOLEAN)) {
                return LLVMBuildSIToFP(builder, value, LLVMDoubleType(), "bool_to_num");
            }
            return LLVMConstReal(LLVMDoubleType(), 0.0);
        }
        
        if (type_equals(to_type, &TYPE_STRING)) {
            if (type_equals(from_type, &TYPE_NUMBER)) {
                return LLVMBuildGlobalStringPtr(builder, "0", "num_to_str");
            }
            if (type_equals(from_type, &TYPE_BOOLEAN)) {
                return LLVMBuildICmp(builder, LLVMIntNE, value, LLVMConstInt(LLVMInt1Type(), 0, 0), "bool_val") ?
                    LLVMBuildGlobalStringPtr(builder, "true", "bool_to_str") :
                    LLVMBuildGlobalStringPtr(builder, "false", "bool_to_str");
            }
            return LLVMBuildGlobalStringPtr(builder, "", "to_str");
        }
        
        if (type_equals(to_type, &TYPE_BOOLEAN)) {
            if (type_equals(from_type, &TYPE_NUMBER)) {
                return LLVMBuildFCmp(builder, LLVMRealONE, value, 
                    LLVMConstReal(LLVMDoubleType(), 0.0), "num_to_bool");
            }
            return LLVMConstInt(LLVMInt1Type(), 0, 0);
        }
    }

    // Manejo de conversión de tipos de usuario a un supertipo built-in, por ejemplo, a OBJECT.
    // Esto es útil si TYPE_OBJECT es considerado built-in pero queremos admitir que
    // un tipo definido por el usuario (como A) se pueda convertir a Object.
    if (!is_builtin_type(from_type) && (type_equals(to_type, &TYPE_OBJECT))) {
        LLVMTypeRef from_struct = LLVMGetTypeByName(module, from_type->name);
        LLVMTypeRef object_struct = LLVMGetTypeByName(module, to_type->name);
        if (from_struct && object_struct) {
            LLVMTypeRef object_ptr = LLVMPointerType(object_struct, 0);
            return LLVMBuildBitCast(builder, value, object_ptr, "to_object");
        }
    }
    
    // Handle user-defined types
    if (!is_builtin_type(from_type) && !is_builtin_type(to_type)) {
        // Find closest common ancestor
        Type* common_ancestor = get_lca(from_type, to_type);
        if (common_ancestor) {
            common_ancestor = common_ancestor->sub_type? common_ancestor->sub_type : common_ancestor;
            // Get the struct types
            LLVMTypeRef from_struct = LLVMGetTypeByName(module, from_type->name);
            LLVMTypeRef to_struct = LLVMGetTypeByName(module, to_type->name);
            LLVMTypeRef ancestor_struct = LLVMGetTypeByName(module, common_ancestor->name);
            
            if (from_struct && to_struct && ancestor_struct) {
                // Create pointers to the struct types
                LLVMTypeRef from_ptr = LLVMPointerType(from_struct, 0);
                LLVMTypeRef to_ptr = LLVMPointerType(to_struct, 0);
                LLVMTypeRef ancestor_ptr = LLVMPointerType(ancestor_struct, 0);
                
                // Cast through the common ancestor
                LLVMValueRef as_ancestor = LLVMBuildBitCast(builder, value, ancestor_ptr, "as_ancestor");
                
                // If target type is the ancestor, we're done
                if (type_equals(to_type, common_ancestor)) {
                    return as_ancestor;
                }
                
                // Otherwise cast to target type
                return LLVMBuildBitCast(builder, as_ancestor, to_ptr, "as_target");
            }
        }
    }
    
    // If no valid cast found, return null value of target type
    return LLVMConstNull(get_llvm_type(to_type));
}

LLVMValueRef generate_loop(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMTypeRef body_type = get_llvm_type(node->data.op_node.right->return_type);

    LLVMValueRef result_addr = NULL;
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind) {
        result_addr = LLVMBuildAlloca(builder, body_type, "while.result.addr");
        LLVMBuildStore(builder, LLVMConstNull(body_type), result_addr);
    }

    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_function, "while.cond");
    LLVMBasicBlockRef loop_block = LLVMAppendBasicBlock(current_function, "while.body");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "while.end");

    LLVMBuildBr(builder, cond_block);

    LLVMPositionBuilderAtEnd(builder, cond_block);
    LLVMValueRef cond_val = accept_gen(v, node->data.op_node.left);
    LLVMBuildCondBr(builder, cond_val, loop_block, merge_block);

    LLVMPositionBuilderAtEnd(builder, loop_block);
    LLVMValueRef body_val = accept_gen(v, node->data.op_node.right);

    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind) {
        LLVMBuildStore(builder, body_val, result_addr);
    }
    LLVMBuildBr(builder, cond_block);

    LLVMPositionBuilderAtEnd(builder, merge_block);
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind) {
        LLVMValueRef final_val = LLVMBuildLoad2(builder, body_type, result_addr, "while.result");
        return final_val;
    } else {
        return NULL;
    }
}

LLVMTypeRef generate_struct_type(const char* type_name, ASTNode* type_node) {
    // Create struct type
    LLVMTypeRef struct_type = LLVMStructCreateNamed(context, type_name);
    
    // Count fields to allocate
    int field_count = 0;
    for (int i = 0; i < type_node->data.type_node.def_count; i++) {
        ASTNode* def = type_node->data.type_node.definitions[i];
        if (def->type == NODE_ASSIGNMENT) {
            field_count++;
        }
    }

    // Create array for field types 
    LLVMTypeRef* field_types = malloc(field_count * sizeof(LLVMTypeRef));
    int field_idx = 0;

    // Add fields from parent first if inheriting
    if (type_node->data.type_node.parent_name[0] != '\0') {
        LLVMTypeRef parent_type = LLVMGetTypeByName(module, type_node->data.type_node.parent_name);
        if (parent_type) {
            // Add parent fields
            unsigned parent_field_count = LLVMCountStructElementTypes(parent_type);
            for (unsigned i = 0; i < parent_field_count; i++) {
                LLVMTypeRef field_type = LLVMStructGetTypeAtIndex(parent_type, i);
                field_types[field_idx++] = field_type;
            }
        }
    }

    // Add this type's fields
    for (int i = 0; i < type_node->data.type_node.def_count; i++) {
        ASTNode* def = type_node->data.type_node.definitions[i];
        if (def->type == NODE_ASSIGNMENT) {
            field_types[field_idx++] = get_llvm_type(def->data.op_node.right->return_type);
        }
    }
    
    LLVMStructSetBody(struct_type, field_types, field_count, 0);
    free(field_types);
    
    return struct_type;
}

void generate_type_methods(LLVM_Visitor* visitor, const char* type_name, LLVMTypeRef struct_type, ASTNode* type_node) {
    // First generate vtable type
    // Guardar builder actual
    LLVMBuilderRef saved_builder = builder;
    builder = LLVMCreateBuilder();
    int method_count = 0;
    for (int i = 0; i < type_node->data.type_node.def_count; i++) {
        if (type_node->data.type_node.definitions[i]->type == NODE_FUNC_DEC) {
            method_count++;
        }
    }
    
    LLVMTypeRef* method_types = malloc(method_count * sizeof(LLVMTypeRef));
    char** method_names = malloc(method_count * sizeof(char*));
    int method_idx = 0;

    // Generate methods and collect their types
    for(int i = 0; i < type_node->data.type_node.def_count; i++) {
        ASTNode* def = type_node->data.type_node.definitions[i];
        if(def->type == NODE_FUNC_DEC) {
            char* method_name = strdup(def->data.func_node.name);
            // Generate method name
            // char* method_name = malloc(strlen(type_name) + strlen(def->data.func_node.name) + 3);
            // sprintf(method_name, "%s__%s", type_name, def->data.func_node.name);
            printf("Generating method %s for type %s\n", def->data.func_node.name, type_name);
            method_names[method_idx] = method_name;
            
            // Create function type that takes struct pointer as first arg
            LLVMTypeRef* param_types = malloc((def->data.func_node.arg_count + 1) * sizeof(LLVMTypeRef));
            param_types[0] = LLVMPointerType(struct_type, 0); // 'this' pointer
            
            for(int j = 0; j < def->data.func_node.arg_count; j++) {
                param_types[j+1] = get_llvm_type(def->data.func_node.args[j]->return_type);
            }
            
            // Use the correct return type from the method declaration
            LLVMTypeRef return_type = get_llvm_type(def->data.func_node.body->return_type);
            
            LLVMTypeRef func_type = LLVMFunctionType(
                return_type,
                param_types,
                def->data.func_node.arg_count + 1,
                0
            );
            method_types[method_idx] = func_type;

            // Add function declaration
            LLVMValueRef func = LLVMAddFunction(module, method_name, func_type);
            
            // Generate method body
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
            LLVMPositionBuilderAtEnd(builder, entry);
            
            // Store old scope and create new one
            push_scope();
            
            // Add 'this' pointer to scope
            LLVMValueRef this_ptr = LLVMGetParam(func, 0);
            declare_variable("self", this_ptr);
            
            // Add parameters to scope
            for(int j = 0; j < def->data.func_node.arg_count; j++) {
                LLVMValueRef param = LLVMGetParam(func, j + 1);
                declare_variable(def->data.func_node.args[j]->data.variable_name, param);
            }
            
            // Generate body and return value
            LLVMValueRef result = accept_gen(visitor, def->data.func_node.body);
            // Para funciones void, ignorar el valor de retorno de las llamadas
            if (type_equals(def->data.func_node.body->return_type, &TYPE_VOID)) {
                LLVMBuildRetVoid(builder);
                result = NULL;
            } else {
                LLVMBuildRet(builder, result);
            }
            
            pop_scope();
            method_idx++;
            free(param_types);
        }
    }

    // Create vtable with correctly typed function pointers
    char vtable_name[256];
    sprintf(vtable_name, "%s_vtable", type_name);
    
    // Create vtable type using the actual method types
    LLVMTypeRef* vtable_fn_types = malloc(method_count * sizeof(LLVMTypeRef));
    for(int i = 0; i < method_count; i++) {
        vtable_fn_types[i] = LLVMPointerType(method_types[i], 0);
    }
    
    LLVMTypeRef vtable_type = LLVMStructType(vtable_fn_types, method_count, 0);
    LLVMValueRef vtable = LLVMAddGlobal(module, vtable_type, vtable_name);
    LLVMSetLinkage(vtable, LLVMPrivateLinkage);
    
    // Initialize vtable with method pointers
    LLVMValueRef* method_ptrs = malloc(method_count * sizeof(LLVMValueRef));
    for(int i = 0; i < method_count; i++) {
        method_ptrs[i] = LLVMGetNamedFunction(module, method_names[i]);
    }
    
    LLVMValueRef vtable_init = LLVMConstStruct(method_ptrs, method_count, 0);
    LLVMSetInitializer(vtable, vtable_init);

    // Cleanup
    free(vtable_fn_types);
    for(int i = 0; i < method_count; i++) {
        free(method_names[i]);
    }
    free(method_names);
    free(method_types);
    free(method_ptrs);
    LLVMDisposeBuilder(builder);
    builder = saved_builder;
}

LLVMValueRef generate_type_declaration(LLVM_Visitor* v, ASTNode* node) {
    const char* type_name = node->data.type_node.name;
    
    // Store type parameters in scope for use in field initializations
    push_scope();
    
    // Add type parameters to scope
    for(int i = 0; i < node->data.type_node.arg_count; i++) {
        ASTNode* param = node->data.type_node.args[i];
        // Create an alloca for each parameter
        LLVMTypeRef param_type = get_llvm_type(param->return_type);
        LLVMValueRef param_alloca = LLVMBuildAlloca(builder, param_type, param->data.variable_name);
        declare_variable(param->data.variable_name, param_alloca);
    }
    // Create the struct type for this class
    LLVMTypeRef struct_type = generate_struct_type(type_name, node);
    // Generate methods for this type
    generate_type_methods(v, type_name, struct_type, node);
    pop_scope();
    return NULL;
}

LLVMValueRef generate_type_instance(LLVM_Visitor* v, ASTNode* node) {
    const char* type_name = node->data.type_node.name;
    LLVMTypeRef struct_type = LLVMGetTypeByName(module, type_name);
    
    LLVMValueRef instance = LLVMBuildMalloc(builder, struct_type, "instance");
    
    Type* type = node->return_type;
    ASTNode* type_def = type->dec;
    
    for (int i = 0; i < node->data.type_node.arg_count; i++) {
        LLVMValueRef arg_value = accept_gen(v, node->data.type_node.args[i]);
        
        const char* param_name = type_def->data.type_node.args[i]->data.variable_name;
        int field_index = -1;
        
        for (int j = 0; j < type_def->data.type_node.def_count; j++) {
            ASTNode* def = type_def->data.type_node.definitions[j];
            if (def->type == NODE_ASSIGNMENT) {
                if (def->data.op_node.right->type == NODE_VARIABLE &&
                    strcmp(def->data.op_node.right->data.variable_name, param_name) == 0) {
                    field_index = find_field_index(type, def->data.op_node.left->data.variable_name);
                    break;
                }
            }
        }
        
        if (field_index >= 0) {
            LLVMValueRef field_ptr = LLVMBuildStructGEP2(
                builder, struct_type, instance, field_index, "field_ptr");
            LLVMBuildStore(builder, arg_value, field_ptr);
        }
    }
    
    if (node->data.type_node.arg_count == 0) {
        for (int j = 0; j < type_def->data.type_node.def_count; j++) {
            ASTNode* def = type_def->data.type_node.definitions[j];
            if (def->type == NODE_ASSIGNMENT) {
                if (def->data.op_node.right->type != NODE_VARIABLE) {
                    LLVMValueRef def_val = accept_gen(v, def->data.op_node.right);
                    const char* field_name = def->data.op_node.left->data.variable_name;
                    int field_index = find_field_index(type, field_name);
                    if (field_index >= 0) {
                        LLVMValueRef field_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, field_index, "field_ptr_default");
                        LLVMBuildStore(builder, def_val, field_ptr);
                    }
                }
            }
        }
    }
    
    return instance;
}

static char* find_method_in_hierarchy(const char* method_name, Type* type) {
    char* name = strdup(method_name);
    while (type && !is_builtin_type(type)) {
        printf("full_name: %s\n", name);

        if (LLVMGetNamedFunction(module, name)) {
            return name;
        }
        
        name = delete_underscore_from_str(name,type->name);
        type = type->parent;
        printf("type: %s\n", type ? type->name : "NULL");
        name = concat_str_with_underscore(type->name,name);
    }
    return NULL;
}

static int find_field_index(Type* type, const char* field_name) {
    int current_index = 0;
    
    Type* parent = type->parent;
    while(parent && !is_builtin_type(parent)) {
        ASTNode* parent_node = parent->dec;
        if (parent_node) {
            for(int i = 0; i < parent_node->data.type_node.def_count; i++) {
                ASTNode* def = parent_node->data.type_node.definitions[i];
                if (def->type == NODE_ASSIGNMENT) {
                    current_index++;
                }
            }
        }
        parent = parent->parent;
    }
    
    ASTNode* type_node = type->dec;
    if (type_node) {
        for(int i = 0; i < type_node->data.type_node.def_count; i++) {
            ASTNode* def = type_node->data.type_node.definitions[i];
            if (def->type == NODE_ASSIGNMENT) {
                if (!strcmp(def->data.op_node.left->data.variable_name, field_name)) {
                    return current_index;
                }
                current_index++;
            }
        }
    }
    
    parent = type->parent;
    int parent_fields = current_index;
    while(parent && !is_builtin_type(parent)) {
        current_index = find_field_index(parent, field_name);
        if (current_index >= 0) {
            return current_index;
        }
        parent = parent->parent;
    }
    
    return -1;
}

LLVMValueRef generate_field_access(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef instance_ptr = accept_gen(v, node->data.op_node.left);
    LLVMTypeRef instance_type_ref = LLVMTypeOf(instance_ptr);
    const char* name = node->data.op_node.right->data.variable_name;
    Type* instance_type = node->data.op_node.left->return_type;
    int field_index = find_field_index(instance_type, name);
    LLVMTypeRef struct_type = LLVMGetElementType(instance_type_ref);

    LLVMValueRef field_ptr = LLVMBuildStructGEP2(
        builder,
        struct_type,
        instance_ptr,
        field_index,
        "field_ptr"
    );

    return LLVMBuildLoad2(
        builder,
        LLVMGetElementType(LLVMTypeOf(field_ptr)),
        field_ptr,
        name
    );
}

LLVMValueRef generate_method_call(LLVM_Visitor* v, ASTNode* node) {
    LLVMTypeRef int32_type = LLVMInt32Type();
    LLVMValueRef depth_val = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth_call");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, depth_val, LLVMConstInt(int32_type, 1, 0), "inc_depth_call");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);

    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT,
                                     new_depth, LLVMConstInt(int32_type, MAX_STACK_DEPTH, 0),
                                     "cmp_overflow_call");

    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    LLVMValueRef current_func = LLVMGetBasicBlockParent(current_bb);
    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(current_func, "stack_overflow_call");
    LLVMBasicBlockRef call_block = LLVMAppendBasicBlock(current_func, "method_call_body");

    LLVMBuildCondBr(builder, cmp, error_block, call_block);

    LLVMPositionBuilderAtEnd(builder, error_block);
    handle_stack_overflow(
        builder, module, current_stack_depth_var, node->line,
        node->data.op_node.right->data.func_node.name
    );
    LLVMPositionBuilderAtEnd(builder, call_block);
    
    LLVMValueRef instance = accept_gen(v, node->data.op_node.left);
    const char* method_name = node->data.op_node.right->data.func_node.name;
    ASTNode** args = node->data.op_node.right->data.func_node.args;
    int arg_count = node->data.op_node.right->data.func_node.arg_count;
    
    LLVMValueRef method;

    LLVMTypeRef lhs_type = LLVMTypeOf(instance);
    if (LLVMGetTypeKind(lhs_type) == LLVMStructTypeKind &&
        LLVMCountStructElementTypes(lhs_type) == 2) {
        LLVMValueRef method_ptr = LLVMBuildExtractValue(builder, instance, 0, "extracted_method");
        LLVMValueRef self_ptr = LLVMBuildExtractValue(builder, instance, 1, "extracted_self");
        method = method_ptr;
        instance = self_ptr;
    } else { 
        Type* instance_type = node->data.op_node.left->return_type;
        char* full_method_name = find_method_in_hierarchy(method_name, instance_type);
        method = LLVMGetNamedFunction(module, full_method_name);
        free(full_method_name);
    }
    
    LLVMTypeRef instanceType = LLVMTypeOf(instance);
    if (LLVMGetTypeKind(instanceType) == LLVMPointerTypeKind) {
        LLVMTypeRef elementType = LLVMGetElementType(instanceType);
        if (LLVMGetTypeKind(elementType) == LLVMPointerTypeKind) {
            instance = LLVMBuildLoad(builder, instance, "loaded_instance");
        }
    }
    
    LLVMValueRef* call_args = malloc((arg_count + 1) * sizeof(LLVMValueRef));
    call_args[0] = instance;
    for (int i = 0; i < arg_count; i++) {
        call_args[i + 1] = accept_gen(v, args[i]);
    }
    
    LLVMValueRef result = LLVMBuildCall2(
        builder,
        LLVMGetElementType(LLVMTypeOf(method)),
        method,
        call_args,
        arg_count + 1,
        type_equals(node->return_type, &TYPE_VOID) ? "" : "method_call"
    );
    
    free(call_args);
    LLVMValueRef final_depth = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth_final_call");
    LLVMValueRef dec_depth = LLVMBuildSub(builder, final_depth, LLVMConstInt(int32_type, 1, 0), "dec_depth_call");
    LLVMBuildStore(builder, dec_depth, current_stack_depth_var);
    
    return type_equals(node->return_type, &TYPE_VOID) ? NULL : result;
}

LLVMValueRef generate_set_attr(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef instance = accept_gen(v, node->data.cond_node.cond);
    const char* property_name = node->data.cond_node.body_true->data.variable_name;
    LLVMValueRef new_value = accept_gen(v, node->data.cond_node.body_false);

    LLVMTypeRef instanceType = LLVMTypeOf(instance);
    if (LLVMGetTypeKind(instanceType) == LLVMPointerTypeKind) {
        LLVMTypeRef elementType = LLVMGetElementType(instanceType);
        if (LLVMGetTypeKind(elementType) == LLVMPointerTypeKind) {
            instance = LLVMBuildLoad(builder, instance, "loaded_instance");
        }
    }

    LLVMTypeRef instanceTypeRef = LLVMTypeOf(instance);
    Type* instance_type = node->data.cond_node.cond->return_type;

    int field_index = find_field_index(instance_type, property_name);

    LLVMTypeRef struct_type = LLVMGetElementType(instanceTypeRef);

    LLVMValueRef field_ptr = LLVMBuildStructGEP2(
        builder,
        struct_type,
        instance,
        field_index,
        "field_ptr"
    );

    LLVMBuildStore(builder, new_value, field_ptr);

    return new_value;
}

LLVMValueRef generate_test_type(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef exp = accept_gen(v, node->data.op_node.left);
    Type* dynamic_type = node->data.op_node.left->return_type;
    const char* type_name = node->static_type;;
    Type* test_type = node->data.cast_test.type;
    
    if (!dynamic_type || !test_type || 
        type_equals(dynamic_type, &TYPE_ERROR) || 
        type_equals(test_type, &TYPE_ERROR)) {
        return LLVMConstInt(LLVMInt1Type(), 0, 0);
    }

    int is_descendant = same_branch_in_type_hierarchy(dynamic_type, test_type);
    
    return LLVMConstInt(LLVMInt1Type(), is_descendant, 0);
}

LLVMValueRef generate_cast_type(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef exp = accept_gen(v, node->data.op_node.left);
    Type* from_type = node->data.op_node.left->return_type;

    const char* type_name = node->static_type;
    Type* to_type = node->return_type;

    if(!is_ancestor_type(from_type, to_type)|| !is_ancestor_type(to_type,from_type)) {
        printf("Casting from type '%s' to type '%s'\n", 
            from_type->name, to_type->name);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            RED"!!RUNTIME ERROR: Type '%s' cannot be cast to type '%s'. Line: %d."RESET,
            from_type->name, to_type->name, node->line);

        LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(builder, error_msg, "error_msg");
        LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
        LLVMBuildCall2(builder, LLVMGetElementType(LLVMTypeOf(puts_func)), puts_func, &error_msg_global, 1, "");
        return LLVMConstNull(get_llvm_type(to_type));
    }
    else if (!same_branch_in_type_hierarchy(from_type, to_type)) {
        printf("Error: Cannot cast type '%s' to '%s' at line %d\n", 
            from_type->name, type_name, node->line);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            RED"!!RUNTIME ERROR: Type '%s' cannot be cast to type '%s'. Line: %d."RESET,
            from_type->name, type_name, node->line);

        LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(builder, error_msg, "error_msg");
        LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
        LLVMBuildCall(builder, puts_func, &error_msg_global, 1, "");

        LLVMValueRef exit_func = LLVMGetNamedFunction(module, "exit");
        LLVMValueRef exit_code = LLVMConstInt(LLVMInt32Type(), 1, 0);
        LLVMBuildCall(builder, exit_func, &exit_code, 1, "");

        LLVMBuildUnreachable(builder);

        return LLVMConstNull(get_llvm_type(to_type));
    }

    return cast_value_to_type(exp, from_type, to_type);
}
