#include "llvm_builtins.h"
#include "llvm_core.h"
#include "../type/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LLVMValueRef generate_builtin_function(LLVM_Visitor* v, ASTNode* node) {
    if (!strcmp(node->data.func_node.name, "print")) {
        return print_function(v, node);
    }
    else if (!strcmp(node->data.func_node.name, "sqrt")) {
        return basic_functions(v, node, "sqrt", "sqrt_tmp");
    }
    else if (!strcmp(node->data.func_node.name, "sin")) {
        return basic_functions(v, node, "sin", "sin_tmp");
    }
    else if (!strcmp(node->data.func_node.name, "cos")) {
        return basic_functions(v, node, "cos", "cos_tmp");
    }
    else if (!strcmp(node->data.func_node.name, "exp")) {
        return basic_functions(v, node, "exp", "exp_tmp");
    }
    else if (!strcmp(node->data.func_node.name, "log")) {
        return log_function(v, node);
    }
    else if (!strcmp(node->data.func_node.name, "rand")) {
        return rand_function(v, node);
    }

    return generate_user_function_call(v, node);
}

LLVMValueRef print_function(LLVM_Visitor* v, ASTNode* node) {
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
    LLVMValueRef arg = accept_gen(v, arg_node);
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

LLVMValueRef log_function(LLVM_Visitor* v, ASTNode* node) {
    // log con 1 o 2 argumentos
    if (node->data.func_node.arg_count == 1) {
        LLVMValueRef arg = accept_gen(v, node->data.func_node.args[0]);
        LLVMTypeRef log_type = LLVMFunctionType(LLVMDoubleType(),
            (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef log_func = LLVMGetNamedFunction(module, "log");
        return LLVMBuildCall2(builder, log_type, log_func,
            (LLVMValueRef[]){arg}, 1, "log_tmp");
    } else {
        // log(base, x) = log(x) / log(base)
        LLVMValueRef base = accept_gen(v, node->data.func_node.args[0]);
        LLVMValueRef x = accept_gen(v, node->data.func_node.args[1]);
        
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

LLVMValueRef rand_function(LLVM_Visitor* v, ASTNode* node) {
    LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef rand_func = LLVMGetNamedFunction(module, "rand");
    
    // Convertir el resultado entero a double dividiendo por RAND_MAX
    LLVMValueRef rand_val = LLVMBuildCall2(builder, rand_type, rand_func, NULL, 0, "rand_tmp");
    LLVMValueRef rand_max = LLVMConstReal(LLVMDoubleType(), RAND_MAX);
    LLVMValueRef rand_double = LLVMBuildSIToFP(builder, rand_val, LLVMDoubleType(), "rand_double");
    
    return LLVMBuildFDiv(builder, rand_double, rand_max, "rand_result");
}

LLVMValueRef basic_functions(
    LLVM_Visitor* v, ASTNode* node, 
    const char* name, const char* tmp_name
) {
    LLVMValueRef arg = accept_gen(v, node->data.func_node.args[0]);
    LLVMTypeRef type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
    LLVMValueRef func = LLVMGetNamedFunction(module, name);
    return LLVMBuildCall2(builder, type, func,
            (LLVMValueRef[]){arg}, 1, tmp_name);
}

LLVMValueRef generate_user_function_call(LLVM_Visitor* v, ASTNode* node) {
    const char* name = node->data.func_node.name;

    LLVMValueRef func = LLVMGetNamedFunction(module, name);    
    LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(func));
    unsigned param_count = LLVMCountParamTypes(func_type);

    int arg_count = node->data.func_node.arg_count;
    ASTNode** args = node->data.func_node.args;
    Type* return_type = node->return_type;

    // Obtener tipos de los argumentos
    LLVMTypeRef* arg_types = malloc(arg_count * sizeof(LLVMTypeRef));
    LLVMValueRef* arg_values = malloc(arg_count * sizeof(LLVMValueRef));
    
    for (int i = 0; i < arg_count; i++) {
        arg_types[i] = get_llvm_type(args[i]->return_type);
        arg_values[i] = accept_gen(v, args[i]);
    }

    // // Obtener/declarar función
    // LLVMTypeRef func_type = LLVMFunctionType(
    //     get_llvm_type(return_type),
    //     arg_types,
    //     arg_count,
    //     0
    // );
    
    if (!func) {
        func = LLVMAddFunction(module, name, func_type);
    }

    // Construir llamada
    LLVMValueRef call = LLVMBuildCall2(
        builder,
        LLVMGetElementType(LLVMTypeOf(func)),
        func,
        arg_values,
        arg_count,
        "calltmp"
    );

    free(arg_types);
    free(arg_values);
    return call;
}