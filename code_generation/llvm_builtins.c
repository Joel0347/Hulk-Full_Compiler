#include "llvm_builtins.h"
#include "llvm_core.h"
#include "llvm_codegen.h"
#include "../type/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LLVMValueRef generate_builtin_function(ASTNode* node) {
    // Validar que es un nodo de función built-in
    if (strcmp(node->data.func_node.name, "print") == 0) {
        LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");

        // Si no hay argumentos, solo imprime una nueva línea
        if (node->data.func_node.arg_count == 0) {
            LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, "\n", "newline");
            LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
            return LLVMBuildCall2(builder, printf_type, printf_func,
                (LLVMValueRef[]){format_str}, 1, "printf_call");
        }

        // Generar código para el argumento
        ASTNode* arg_node = node->data.func_node.args[0];
        LLVMValueRef arg = codegen(arg_node);
        if (!arg) return NULL;

        const char* format = "";
        LLVMValueRef format_str;
        LLVMValueRef args[2];  // Pre-allocate array for args
        int num_args = 2;      // Default to 2 args (format string + value)

        // Seleccionar formato según el tipo del argumento
        if (type_equals(arg_node->return_type, &TYPE_NUMBER_INST)) {
            format = "%g\n";
            format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
            args[0] = format_str;
            args[1] = arg;
        } else if (type_equals(arg_node->return_type, &TYPE_BOOLEAN_INST)) {
            format_str = LLVMBuildGlobalStringPtr(builder, "%s\n", "fmt");
            LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
            LLVMValueRef false_str = LLVMBuildGlobalStringPtr(builder, "false", "false_str");
            LLVMValueRef cond_str = LLVMBuildSelect(builder, arg, true_str, false_str, "bool_str");
            args[0] = format_str;
            args[1] = cond_str;
        } else if (type_equals(arg_node->return_type, &TYPE_STRING_INST)) {
            format = "%s\n";
            format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
            args[0] = format_str;
            args[1] = arg;
        } else {
            // Handle unknown type
            format = "%s\n";
            format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
            LLVMValueRef unknown_str = LLVMBuildGlobalStringPtr(builder, "<unknown>", "unknown_str");
            args[0] = format_str;
            args[1] = unknown_str;
        }

        // Construir llamada a printf
        LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
            (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
        return LLVMBuildCall2(builder, printf_type, printf_func, args, num_args, "printf_call");
    }
    else if (strcmp(node->data.func_node.name, "sqrt") == 0) {
        LLVMValueRef arg = codegen(node->data.func_node.args[0]);
        LLVMTypeRef sqrt_type = LLVMFunctionType(LLVMDoubleType(),
            (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef sqrt_func = LLVMGetNamedFunction(module, "sqrt");
        return LLVMBuildCall2(builder, sqrt_type, sqrt_func, 
            (LLVMValueRef[]){arg}, 1, "sqrt_tmp");
    }
    else if (strcmp(node->data.func_node.name, "sin") == 0) {
        LLVMValueRef arg = codegen(node->data.func_node.args[0]);
        LLVMTypeRef sin_type = LLVMFunctionType(LLVMDoubleType(),
            (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef sin_func = LLVMGetNamedFunction(module, "sin");
        return LLVMBuildCall2(builder, sin_type, sin_func,
            (LLVMValueRef[]){arg}, 1, "sin_tmp");
    }
    else if (strcmp(node->data.func_node.name, "cos") == 0) {
        LLVMValueRef arg = codegen(node->data.func_node.args[0]);
        LLVMTypeRef cos_type = LLVMFunctionType(LLVMDoubleType(),
            (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef cos_func = LLVMGetNamedFunction(module, "cos");
        return LLVMBuildCall2(builder, cos_type, cos_func,
            (LLVMValueRef[]){arg}, 1, "cos_tmp");
    }
    else if (strcmp(node->data.func_node.name, "exp") == 0) {
        LLVMValueRef arg = codegen(node->data.func_node.args[0]);
        LLVMTypeRef exp_type = LLVMFunctionType(LLVMDoubleType(),
            (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef exp_func = LLVMGetNamedFunction(module, "exp");
        return LLVMBuildCall2(builder, exp_type, exp_func,
            (LLVMValueRef[]){arg}, 1, "exp_tmp");
    }
    else if (strcmp(node->data.func_node.name, "log") == 0) {
        // log con 1 o 2 argumentos
        if (node->data.func_node.arg_count == 1) {
            LLVMValueRef arg = codegen(node->data.func_node.args[0]);
            LLVMTypeRef log_type = LLVMFunctionType(LLVMDoubleType(),
                (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
            LLVMValueRef log_func = LLVMGetNamedFunction(module, "log");
            return LLVMBuildCall2(builder, log_type, log_func,
                (LLVMValueRef[]){arg}, 1, "log_tmp");
        } else {
            // log(base, x) = log(x) / log(base)
            LLVMValueRef base = codegen(node->data.func_node.args[0]);
            LLVMValueRef x = codegen(node->data.func_node.args[1]);
            
            LLVMTypeRef log_type = LLVMFunctionType(LLVMDoubleType(),
                (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
            LLVMValueRef log_func = LLVMGetNamedFunction(module, "log");

            LLVMValueRef log_x = LLVMBuildCall2(builder, log_type, log_func,
                (LLVMValueRef[]){x}, 1, "log_x");
            LLVMValueRef log_base = LLVMBuildCall2(builder, log_type, log_func,
                (LLVMValueRef[]){base}, 1, "log_base");
            
            return LLVMBuildFDiv(builder, log_x, log_base, "log_result");
        }
    }
    else if (strcmp(node->data.func_node.name, "rand") == 0) {
        LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
        LLVMValueRef rand_func = LLVMGetNamedFunction(module, "rand");
        
        // Convertir el resultado entero a double dividiendo por RAND_MAX
        LLVMValueRef rand_val = LLVMBuildCall2(builder, rand_type, rand_func, NULL, 0, "rand_tmp");
        LLVMValueRef rand_max = LLVMConstReal(LLVMDoubleType(), RAND_MAX);
        LLVMValueRef rand_double = LLVMBuildSIToFP(builder, rand_val, LLVMDoubleType(), "rand_double");
        
        return LLVMBuildFDiv(builder, rand_double, rand_max, "rand_result");
    }
    exit(1);
}