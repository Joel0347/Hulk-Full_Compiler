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
    if (then_val && !type_equals(true_body->return_type, node->return_type)) {
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
    if (else_val && !type_equals(false_type, node->return_type)) {
        else_val = cast_value_to_type(else_val, false_type, node->return_type);
    }
    
    LLVMBuildBr(builder, merge_block);
    last_else_block = LLVMGetInsertBlock(builder);

    // Generate merge block with PHI node if needed
    LLVMPositionBuilderAtEnd(builder, merge_block);
    
    if (type_equals(node->return_type, &TYPE_VOID)) {
        return LLVMConstNull(get_llvm_type(&TYPE_OBJECT));
    }
    
    // Create PHI node with the correct type
    LLVMTypeRef phi_type = get_llvm_type(node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(builder, phi_type, "if_result");
    
    // Add incoming values to PHI with proper null values for missing branches
    if (then_val && else_val) {
        LLVMValueRef incoming_values[] = {then_val, else_val};
        LLVMBasicBlockRef incoming_blocks[] = {then_block, last_else_block};
        LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
    } 
    else {
        LLVMValueRef incoming_values[] = {
            then_val? then_val : LLVMBuildRetVoid(builder),
            else_val? else_val : LLVMBuildRetVoid(builder)
        };
        LLVMBasicBlockRef incoming_blocks[] = {then_block, merge_block};
        LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
    }
    
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

    printf("typt from: %s, type to: %s\n", from_type->name, to_type->name);
    
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
    printf("Generating type declaration for %s\n", type_name);
    // Get struct type
    LLVMTypeRef struct_type = LLVMGetTypeByName(module, type_name);
    if (!struct_type) {
        fprintf(stderr, "Type %s not found\n", type_name);
        exit(1);
    }

    // Allocate memory for instance
    LLVMValueRef instance = LLVMBuildMalloc(builder, struct_type, "instance");
    
    // Get the type definition to match parameters to fields
    Type* type = node->return_type;
    ASTNode* type_def = type->dec;
    
    // Keep track of which fields have been initialized
    int field_count = LLVMCountStructElementTypes(struct_type);
    char** initialized_fields = calloc(field_count, sizeof(char*));
    int initialized_count = 0;

    // First handle constructor parameters
    for(int i = 0; i < node->data.type_node.arg_count; i++) {
        LLVMValueRef arg_value = accept_gen(v, node->data.type_node.args[i]);
        // Find matching field
        const char* param_name = type_def->data.type_node.args[i]->data.variable_name;
        int field_index = find_field_index(type, param_name);
        
        if (field_index >= 0) {
            // Store constructor parameter value to field
            LLVMValueRef field_ptr = LLVMBuildStructGEP2(
                builder, struct_type, instance, field_index, "field_ptr");
            LLVMBuildStore(builder, arg_value, field_ptr);
            
            // Mark field as initialized
            initialized_fields[initialized_count++] = strdup(param_name);
        }
    }

    // Initialize remaining fields with default values
    for(int i = 0; i < field_count; i++) {
        // Check if field was already initialized
        int already_initialized = 0;
        for(int j = 0; j < initialized_count; j++) {
            if (initialized_fields[j] && !strcmp(
                type_def->data.type_node.definitions[i]->data.op_node.left->data.variable_name,
                initialized_fields[j])) {
                already_initialized = 1;
                break;
            }
        }
        if (!already_initialized) {
            LLVMTypeRef field_type = LLVMStructGetTypeAtIndex(struct_type, i);
            LLVMValueRef field_ptr = LLVMBuildStructGEP2(
                builder, struct_type, instance, i, "field_ptr");
            
            // Initialize with appropriate default value based on field type
            LLVMValueRef default_value;
            LLVMTypeKind type_kind = LLVMGetTypeKind(field_type);
            
            // Get the actual Type* from the AST node for this field
            Type* field_ast_type = type_def->data.type_node.definitions[i]->data.op_node.right->return_type;
            
            if (type_equals(field_ast_type, &TYPE_NUMBER)) {
                default_value = LLVMConstReal(field_type, 0.0);
            }
            else if (type_equals(field_ast_type, &TYPE_STRING)) {
                // Empty string
                default_value = LLVMBuildGlobalStringPtr(builder, "", "empty_str");
            }
            else if (type_equals(field_ast_type, &TYPE_BOOLEAN)) {
                default_value = LLVMConstInt(field_type, 0, 0); // false
            }
            else if (field_ast_type->dec != NULL) {
                // Es un tipo definido por el usuario
                default_value = LLVMConstPointerNull(field_type);
            }
            else {
                // Para cualquier otro tipo, usar null como valor por defecto
                default_value = LLVMConstPointerNull(field_type);
            }
            
            LLVMBuildStore(builder, default_value, field_ptr);
        }
    }

    // Cleanup
    for(int i = 0; i < initialized_count; i++) {
        free(initialized_fields[i]);
    }
    free(initialized_fields);

    // // Si el tipo de retorno esperado es un tipo base, hacer el casting apropiado
    // Type* target_type = node->return_type ;
    // printf("Target type: %s\n", target_type ? target_type->name : "NULL");
    // printf("Instance type: %s\n", node->data.type_node.name);
    // Type* instance_type = (Type*)&node->data.type_node;
    // if (target_type && target_type !=instance_type) {
    //     // Obtener el tipo LLVM del tipo base
    //     LLVMTypeRef target_struct_type = LLVMGetTypeByName(module, target_type->name);
    //     if (target_struct_type) {
    //         // Crear un puntero al tipo base
    //         LLVMTypeRef target_ptr_type = LLVMPointerType(target_struct_type, 0);
    //         // Realizar el bitcast de la instancia al tipo base
    //         instance = LLVMBuildBitCast(builder, instance, target_ptr_type, "base_cast");
    //     }
    // }

    return instance;
}

LLVMValueRef generate_field_access(LLVM_Visitor* v, ASTNode* node) {
    // Get instance pointer
    LLVMValueRef instance = accept_gen(v, node->data.op_node.left);
    const char* name = node->data.op_node.right->data.variable_name;
    
    // Get type info from AST node
    Type* instance_type = node->data.op_node.left->return_type;
    
    // Primero intentamos buscar como campo
    int field_index = find_field_index(instance_type, name);
    
    if (field_index >= 0) {
        // Es un campo, procesarlo como antes
        LLVMTypeRef struct_type = LLVMGetElementType(LLVMTypeOf(instance));
        LLVMValueRef field_ptr = LLVMBuildStructGEP2(
            builder, struct_type, instance, field_index, "field_ptr");
        
        return LLVMBuildLoad2(
            builder, 
            LLVMGetElementType(LLVMTypeOf(field_ptr)),
            field_ptr, 
            "field_value"
        );
    } else {
        // No es un campo, intentar buscar como método
        // Construir el nombre completo del método (Type__methodName)
        char* method_name = malloc(strlen(instance_type->name) + strlen(name) + 3);
        sprintf(method_name, "%s__%s", instance_type->name, name);
        // Buscar la función
        LLVMValueRef method = LLVMGetNamedFunction(module, method_name);
        
        if (!method) {
            // Si no se encuentra en este tipo, buscar en los padres
            Type* current_type = instance_type->parent;
            while (current_type && !is_builtin_type(current_type)) {
                free(method_name);
                method_name = malloc(strlen(current_type->name) + strlen(name) + 3);
                sprintf(method_name, "%s__%s", current_type->name, name);
                
                method = LLVMGetNamedFunction(module, method_name);
                if (method) break;
                
                current_type = current_type->parent;
            }
        }
        
        if (method) {
            // Es un método, retornar el puntero a la función
            free(method_name);
            LLVMTypeRef method_type = LLVMTypeOf(method);
            return method;
        }
        
        free(method_name);
        fprintf(stderr, "Neither field nor method %s found in type %s or its parents\n", 
                name, instance_type->name);
        exit(1);
    }
}

static char* find_method_in_hierarchy(const char* method_name, Type* type) {
    char* name = strdup(method_name);
    while (type && !is_builtin_type(type)) {
        // Build full method name for this type
        // char* full_name = malloc(strlen(type->name) + strlen(method_name) + 3);
        // sprintf(full_name, "%s__%s", type->name, method_name);
        printf("full_name: %s\n", name);
        // Check if method exists in this type
        if (LLVMGetNamedFunction(module, name)) {
            return name;
        }
        
        // free(full_name);
        name = delete_underscore_from_str(name,type->name);
        type = type->parent;
        printf("type: %s\n", type ? type->name : "NULL");
        name = concat_str_with_underscore(type->name,name);
    }
    return NULL;
}

LLVMValueRef generate_method_call(LLVM_Visitor* v, ASTNode* node) {
    // Get instance pointer and method info    
    LLVMValueRef instance = accept_gen(v, node->data.op_node.left);
    const char* method_name = node->data.op_node.right->data.func_node.name;
    ASTNode** args = node->data.op_node.right->data.func_node.args;
    int arg_count = node->data.op_node.right->data.func_node.arg_count;
    
    // Get instance type info
    Type* instance_type = node->data.op_node.left->return_type;
    
    // Find method in type hierarchy
    char* full_method_name = find_method_in_hierarchy(method_name, instance_type);
    if (!full_method_name) {
        fprintf(stderr, "Method %s not found in type %s or its parents\n", 
                method_name, instance_type->name);
        exit(1);
    }
    
    // Get method function
    LLVMValueRef method = LLVMGetNamedFunction(module, full_method_name);
    
    // Prepare arguments (instance pointer + regular args)
    LLVMValueRef* call_args = malloc((arg_count + 1) * sizeof(LLVMValueRef));
    
    // Cast instance to base type pointer where method was defined
    if (strcmp(instance_type->parent->name, "Object") == 0)
    {
        call_args[0] = instance;
    }
    else{
        LLVMTypeRef base_struct_type = LLVMGetTypeByName(module, instance_type->parent->name);
        LLVMTypeRef base_ptr_type = LLVMPointerType(base_struct_type, 0);
        call_args[0] = LLVMBuildBitCast(builder, instance, base_ptr_type, "base_cast"); // Cast to base type
    }
    
    // Generate code for method arguments
    for(int i = 0; i < arg_count; i++) {
        call_args[i + 1] = accept_gen(v, args[i]);
    }
    
    // Call method
    char* calltmp = type_equals(node->return_type, &TYPE_VOID) ? "" : "method_call";
    LLVMValueRef result = LLVMBuildCall2(
        builder,
        LLVMGetElementType(LLVMTypeOf(method)),
        method, 
        call_args,
        arg_count + 1,
        calltmp
    );
    
    free(full_method_name);
    free(call_args);
    return type_equals(node->return_type, &TYPE_VOID) ? NULL : result;
}

static int find_field_index(Type* type, const char* field_name) {
    int current_index = 0;
    
    // First count fields in parent types
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
    
    // Then look through this type's fields
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
    
    // Try parent types if not found here
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

LLVMValueRef generate_test_type(LLVM_Visitor* v, ASTNode* node) {
    // Evaluar la expresión del lado izquierdo
    LLVMValueRef exp = accept_gen(v, node->data.op_node.left);
    Type* dynamic_type = node->data.op_node.left->return_type;
    // Obtener el tipo estático que queremos comprobar
    const char* type_name = node->static_type;;
    Type* test_type = node->data.cast_test.type;
    
    // Si alguno es null o error, retornar false
    if (!dynamic_type || !test_type || 
        type_equals(dynamic_type, &TYPE_ERROR) || 
        type_equals(test_type, &TYPE_ERROR)) {
        return LLVMConstInt(LLVMInt1Type(), 0, 0); // false
    }

    // Verificar si el tipo dinámico es descendiente del tipo a comprobar
    int is_descendant = same_branch_in_type_hierarchy(dynamic_type, test_type);
    
    // Retornar el resultado booleano
    return LLVMConstInt(LLVMInt1Type(), is_descendant, 0);
}

LLVMValueRef generate_cast_type(LLVM_Visitor* v, ASTNode* node) {
    // Evaluar la expresión a castear
    LLVMValueRef exp = accept_gen(v, node->data.op_node.left);
    Type* from_type = node->data.op_node.left->return_type;

    // Obtener el tipo al que queremos castear
    const char* type_name = node->static_type;
    Type* to_type = node->return_type;
    // Si los tipos no están en la misma rama de herencia, retornar null
    if(!is_ancestor_type(from_type, to_type)|| !is_ancestor_type(to_type,from_type)) {
        printf("Casting from type '%s' to type '%s'\n", 
            from_type->name, to_type->name);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            RED"!!RUNTIME ERROR: Type '%s' cannot be cast to type '%s'. Line: %d."RESET,
            from_type->name, to_type->name, node->line);
        // Imprimir mensaje de error
        LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(builder, error_msg, "error_msg");
        LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
        LLVMBuildCall2(builder, LLVMGetElementType(LLVMTypeOf(puts_func)), puts_func, &error_msg_global, 1, "");
        return LLVMConstNull(get_llvm_type(to_type));
    }
    else if (!same_branch_in_type_hierarchy(from_type, to_type)) {
        // Crear mensaje de error
        printf("Error: Cannot cast type '%s' to '%s' at line %d\n", 
            from_type->name, type_name, node->line);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            RED"!!RUNTIME ERROR: Type '%s' cannot be cast to type '%s'. Line: %d."RESET,
            from_type->name, type_name, node->line);

        // Imprimir mensaje de error
        LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(builder, error_msg, "error_msg");
        LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
        LLVMBuildCall(builder, puts_func, &error_msg_global, 1, "");

        // Salir del programa
        LLVMValueRef exit_func = LLVMGetNamedFunction(module, "exit");
        LLVMValueRef exit_code = LLVMConstInt(LLVMInt32Type(), 1, 0);
        LLVMBuildCall(builder, exit_func, &exit_code, 1, "");

        // Marcar como inalcanzable
        LLVMBuildUnreachable(builder);

        return LLVMConstNull(get_llvm_type(to_type));
    }

    // Realizar el cast si es válido
    return cast_value_to_type(exp, from_type, to_type);
}
