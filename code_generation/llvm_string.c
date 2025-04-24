#include "llvm_string.h"
#include "llvm_core.h"
#include <stdlib.h>
#include <string.h>

char* process_string_escapes(const char* input) {
    size_t len = strlen(input);
    char* output = malloc(len + 1);  // El resultado podría ser más corto, nunca más largo
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\' && i + 1 < len) {
            switch (input[i + 1]) {
                case 'n':
                    output[j++] = '\n';
                    i++;
                    break;
                case 't':
                    output[j++] = '\t';
                    i++;
                    break;
                case '"':
                    output[j++] = '"';
                    i++;
                    break;
                case '\\':
                    output[j++] = '\\';
                    i++;
                    break;
                default:
                    output[j++] = input[i];
                    break;
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
    return output;
}

LLVMValueRef generate_string_concatenation(LLVMValueRef L, LLVMValueRef R, int is_double_concat) {
    // Declarar strlen si no existe
    LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64Type(),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
    LLVMValueRef strlen_func = LLVMGetNamedFunction(module, "strlen");

    // Obtener longitudes de los strings
    LLVMValueRef len1 = LLVMBuildCall2(builder, strlen_type, strlen_func, 
        (LLVMValueRef[]){L}, 1, "len1");
    LLVMValueRef len2 = LLVMBuildCall2(builder, strlen_type, strlen_func,
        (LLVMValueRef[]){R}, 1, "len2");

    // Calcular tamaño total
    LLVMValueRef total_len = LLVMBuildAdd(builder, len1, len2, "total_len");
    if (is_double_concat) {
        // Agregar 1 para el espacio en @@
        total_len = LLVMBuildAdd(builder, total_len,
            LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_space");
    }
    // Agregar 1 para el null terminator
    total_len = LLVMBuildAdd(builder, total_len,
        LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_null");

    // Asignar memoria para el resultado
    LLVMValueRef malloc_func = LLVMGetNamedFunction(module, "malloc");
    LLVMValueRef buffer = LLVMBuildCall2(builder,
        LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0),
            (LLVMTypeRef[]){LLVMInt64Type()}, 1, 0),
        malloc_func, (LLVMValueRef[]){total_len}, 1, "buffer");

    // Copiar primer string
    LLVMBuildCall2(builder,
        LLVMFunctionType(LLVMVoidType(),
            (LLVMTypeRef[]){
                LLVMPointerType(LLVMInt8Type(), 0),
                LLVMPointerType(LLVMInt8Type(), 0)
            }, 2, 0),
        LLVMGetNamedFunction(module, "strcpy"),
        (LLVMValueRef[]){buffer, L}, 2, "");

    if (is_double_concat) {
        // Agregar espacio para @@
        LLVMValueRef space_ptr = LLVMBuildGEP2(builder,
            LLVMInt8Type(), buffer,
            (LLVMValueRef[]){len1}, 1, "space_ptr");
        LLVMBuildStore(builder,
            LLVMConstInt(LLVMInt8Type(), ' ', 0),
            space_ptr);

        // Concatenar segundo string después del espacio
        LLVMValueRef after_space = LLVMBuildGEP2(builder,
            LLVMInt8Type(), buffer,
            (LLVMValueRef[]){
                LLVMBuildAdd(builder, len1,
                    LLVMConstInt(LLVMInt64Type(), 1, 0), "")
            }, 1, "after_space");
        LLVMBuildCall2(builder,
            LLVMFunctionType(LLVMVoidType(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 2, 0),
            LLVMGetNamedFunction(module, "strcpy"),
            (LLVMValueRef[]){after_space, R}, 2, "");
    } else {
        // Concatenar directamente para @
        LLVMBuildCall2(builder,
            LLVMFunctionType(LLVMVoidType(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 2, 0),
            LLVMGetNamedFunction(module, "strcat"),
            (LLVMValueRef[]){buffer, R}, 2, "");
    }

    return buffer;
}