#include "llvm_operators.h"
#include "llvm_core.h"
#include "llvm_string.h"
#include "../type/type.h"
#include <stdio.h>
#include <stdlib.h>

LLVMValueRef handle_object_operation(LLVMValueRef left, LLVMValueRef right, int op) {
    // Obtener el tipo ID de los operandos
    LLVMValueRef left_type_id = LLVMBuildExtractValue(builder, left, 0, "left_type_id");
    LLVMValueRef right_type_id = LLVMBuildExtractValue(builder, right, 0, "right_type_id");
    
    // Crear bloques para el manejo dinámico de tipos
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
    LLVMValueRef current_function = LLVMGetBasicBlockParent(current_block);
    
    // Bloque para operación numérica
    LLVMBasicBlockRef number_block = LLVMAppendBasicBlock(current_function, "number_op");
    LLVMBasicBlockRef string_block = LLVMAppendBasicBlock(current_function, "string_op");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge");
    
    // Comparar tipos y bifurcar
    LLVMValueRef is_number = LLVMBuildICmp(builder, LLVMIntEQ, left_type_id, 
        LLVMConstInt(LLVMInt32Type(), 1, 0), "is_number"); // 1 = NUMBER_TYPE_ID
    LLVMBuildCondBr(builder, is_number, number_block, string_block);
    
    // Bloque de operación numérica
    LLVMPositionBuilderAtEnd(builder, number_block);
    LLVMValueRef num_result;
    switch(op) {
        case OP_ADD:
            num_result = LLVMBuildFAdd(builder, left, right, "add");
            break;
        case OP_SUB:
            num_result = LLVMBuildFSub(builder, left, right, "sub");
            break;
        // ... otros casos ...
    }
    LLVMBuildBr(builder, merge_block);
    
    // Bloque de operación de string
    LLVMPositionBuilderAtEnd(builder, string_block);
    LLVMValueRef str_result;
    // Manejar operaciones de string
    LLVMBuildBr(builder, merge_block);
    
    // Bloque de merge
    LLVMPositionBuilderAtEnd(builder, merge_block);
    LLVMValueRef phi = LLVMBuildPhi(builder, LLVMDoubleType(), "result");
    LLVMValueRef incoming_values[] = {num_result, str_result};
    LLVMBasicBlockRef incoming_blocks[] = {number_block, string_block};
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
    
    return phi;
}

LLVMValueRef generate_binary_operation(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef L = accept_gen(v, node->data.op_node.left);
    LLVMValueRef R = accept_gen(v, node->data.op_node.right);

    // Manejo de operaciones con strings (concatenación)
    if (node->data.op_node.op == OP_CONCAT || node->data.op_node.op == OP_DCONCAT) {
        // Convertir números a strings si es necesario
        if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER)) {
            LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMInt64Type(),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 3, 1);
            LLVMValueRef snprintf_func = LLVMGetNamedFunction(module, "snprintf");

            // Buffer para el número
            LLVMValueRef num_buffer = LLVMBuildAlloca(builder, 
                LLVMArrayType(LLVMInt8Type(), 32), "num_buffer");
            LLVMValueRef buffer_ptr = LLVMBuildBitCast(builder, num_buffer,
                LLVMPointerType(LLVMInt8Type(), 0), "buffer_cast");
            
            LLVMValueRef format = LLVMBuildGlobalStringPtr(builder, "%g", "num_format");
            LLVMBuildCall2(builder, snprintf_type, snprintf_func,
                (LLVMValueRef[]){
                    buffer_ptr,
                    LLVMConstInt(LLVMInt64Type(), 32, 0),
                    format,
                    L
                }, 4, "");
            
            L = buffer_ptr;
        }

        if (type_equals(node->data.op_node.right->return_type, &TYPE_NUMBER)) {
            LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMInt64Type(),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 3, 1);
            LLVMValueRef snprintf_func = LLVMGetNamedFunction(module, "snprintf");

            LLVMValueRef num_buffer = LLVMBuildAlloca(builder,
                LLVMArrayType(LLVMInt8Type(), 32), "num_buffer");
            LLVMValueRef buffer_ptr = LLVMBuildBitCast(builder, num_buffer,
                LLVMPointerType(LLVMInt8Type(), 0), "buffer_cast");
            
            LLVMValueRef format = LLVMBuildGlobalStringPtr(builder, "%g", "num_format");
            LLVMBuildCall2(builder, snprintf_type, snprintf_func,
                (LLVMValueRef[]){
                    buffer_ptr,
                    LLVMConstInt(LLVMInt64Type(), 32, 0),
                    format,
                    R
                }, 4, "");
            
            R = buffer_ptr;
        }

        return generate_string_concatenation(L, R, node->data.op_node.op == OP_DCONCAT);
    }

    // Si los operandos son números
    if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER)) {
        switch (node->data.op_node.op) {
            case OP_ADD: return LLVMBuildFAdd(builder, L, R, "add_tmp");
            case OP_SUB: return LLVMBuildFSub(builder, L, R, "sub_tmp");
            case OP_MUL: return LLVMBuildFMul(builder, L, R, "mul_tmp");
            case OP_DIV: return LLVMBuildFDiv(builder, L, R, "div_tmp");
            case OP_MOD: {
                LLVMTypeRef fmod_type = LLVMFunctionType(LLVMDoubleType(),
                    (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
                LLVMValueRef fmod_func = LLVMGetNamedFunction(module, "fmod");
                return LLVMBuildCall2(builder, fmod_type, fmod_func,
                    (LLVMValueRef[]){L, R}, 2, "mod_tmp");
            }
            case OP_POW: {
                LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleType(),
                    (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
                LLVMValueRef pow_func = LLVMGetNamedFunction(module, "pow");
                return LLVMBuildCall2(builder, pow_type, pow_func,
                    (LLVMValueRef[]){L, R}, 2, "pow_tmp");
            }
            case OP_EQ:
                return LLVMBuildFCmp(builder, LLVMRealOEQ, L, R, "eq_tmp");
            case OP_NEQ:
                return LLVMBuildFCmp(builder, LLVMRealONE, L, R, "neq_tmp");
            case OP_GR:
                return LLVMBuildFCmp(builder, LLVMRealOGT, L, R, "gt_tmp");
            case OP_GRE:
                return LLVMBuildFCmp(builder, LLVMRealOGE, L, R, "ge_tmp");
            case OP_LS:
                return LLVMBuildFCmp(builder, LLVMRealOLT, L, R, "lt_tmp");
            case OP_LSE:
                return LLVMBuildFCmp(builder, LLVMRealOLE, L, R, "le_tmp");
            default: break;
        }
    }

    // Operadores lógicos
    if (type_equals(node->data.op_node.left->return_type, &TYPE_BOOLEAN)) {
        switch (node->data.op_node.op) {
            case OP_AND:
                return LLVMBuildAnd(builder, L, R, "and_tmp");
            case OP_OR:
                return LLVMBuildOr(builder, L, R, "or_tmp");
            case OP_EQ:
                return LLVMBuildICmp(builder, LLVMIntEQ, L, R, "bool_eq_tmp");
            case OP_NEQ:
                return LLVMBuildICmp(builder, LLVMIntNE, L, R, "bool_neq_tmp");
            default:
                break;
        }
    }

    // Comparación de strings
    if (type_equals(node->data.op_node.left->return_type, &TYPE_STRING)) {
        LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(),
            (LLVMTypeRef[]){
                LLVMPointerType(LLVMInt8Type(), 0),
                LLVMPointerType(LLVMInt8Type(), 0)
            }, 2, 0);
        LLVMValueRef strcmp_func = LLVMGetNamedFunction(module, "strcmp");

        LLVMValueRef cmp = LLVMBuildCall2(builder, strcmp_type, strcmp_func,
            (LLVMValueRef[]){L, R}, 2, "strcmp_tmp");

        switch (node->data.op_node.op) {
            case OP_EQ:
                return LLVMBuildICmp(builder, LLVMIntEQ, cmp,
                    LLVMConstInt(LLVMInt32Type(), 0, 0), "str_eq_tmp");
            case OP_NEQ:
                return LLVMBuildICmp(builder, LLVMIntNE, cmp,
                    LLVMConstInt(LLVMInt32Type(), 0, 0), "str_neq_tmp");
            case OP_GR:
                return LLVMBuildICmp(builder, LLVMIntSGT, cmp,
                    LLVMConstInt(LLVMInt32Type(), 0, 0), "str_gt_tmp");
            case OP_GRE:
                return LLVMBuildICmp(builder, LLVMIntSGE, cmp,
                    LLVMConstInt(LLVMInt32Type(), 0, 0), "str_ge_tmp");
            case OP_LS:
                return LLVMBuildICmp(builder, LLVMIntSLT, cmp,
                    LLVMConstInt(LLVMInt32Type(), 0, 0), "str_lt_tmp");
            case OP_LSE:
                return LLVMBuildICmp(builder, LLVMIntSLE, cmp,
                    LLVMConstInt(LLVMInt32Type(), 0, 0), "str_le_tmp");
            default:
                break;
        }
    }

    // Manejo de operaciones con objetos
    if (type_equals(node->data.op_node.left->return_type, &TYPE_OBJECT) ||
        type_equals(node->data.op_node.right->return_type, &TYPE_OBJECT)) {
        return handle_object_operation(L, R, node->data.op_node.op);
    }

    exit(1);
}

LLVMValueRef generate_unary_operation(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef operand = accept_gen(v, node->data.op_node.left);
    
    switch (node->data.op_node.op) {
        case OP_NEGATE: 
            if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER)) {
                return LLVMBuildFNeg(builder, operand, "neg_tmp");
            }
            break;
        case OP_NOT:
            if (type_equals(node->data.op_node.left->return_type, &TYPE_BOOLEAN)) {
                return LLVMBuildNot(builder, operand, "not_tmp");
            }
            break;
    }
    exit(1);
}