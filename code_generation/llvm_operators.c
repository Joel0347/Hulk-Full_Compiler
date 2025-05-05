#include "llvm_operators.h"
#include "llvm_core.h"
#include "llvm_string.h"
#include "../type/type.h"
#include <stdio.h>
#include <stdlib.h>

LLVMValueRef generate_binary_operation(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef L = accept_gen(v, node->data.op_node.left);
    LLVMValueRef R = accept_gen(v, node->data.op_node.right);

    // Manejo de operaciones con strings (concatenación)
    if (node->data.op_node.op == OP_CONCAT || node->data.op_node.op == OP_DCONCAT) {
        // Convertir números a strings si es necesario
        if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER_INST)) {
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

        if (type_equals(node->data.op_node.right->return_type, &TYPE_NUMBER_INST)) {
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
    if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER_INST)) {
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
    if (type_equals(node->data.op_node.left->return_type, &TYPE_BOOLEAN_INST)) {
        switch (node->data.op_node.op) {
            case OP_AND:
                return LLVMBuildAnd(builder, L, R, "and_tmp");
            case OP_OR:
                return LLVMBuildOr(builder, L, R, "or_tmp");
            default:
                break;
        }
    }

    // Comparación de strings
    if (type_equals(node->data.op_node.left->return_type, &TYPE_STRING_INST)) {
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
    exit(1);
}

LLVMValueRef generate_unary_operation(LLVM_Visitor* v, ASTNode* node) {
    LLVMValueRef operand = accept_gen(v, node->data.op_node.left);
    
    switch (node->data.op_node.op) {
        case OP_NEGATE: 
            if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER_INST)) {
                return LLVMBuildFNeg(builder, operand, "neg_tmp");
            }
            break;
        case OP_NOT:
            if (type_equals(node->data.op_node.left->return_type, &TYPE_BOOLEAN_INST)) {
                return LLVMBuildNot(builder, operand, "not_tmp");
            }
            break;
    }
    exit(1);
}