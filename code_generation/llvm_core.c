#include "llvm_core.h"
#include <stdio.h>
#include <stdlib.h>

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMContextRef context;
LLVMValueRef current_stack_depth_var;
LLVMTypeRef object_type;
const int MAX_STACK_DEPTH = 10000;

void init_llvm(void) {
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    context = LLVMGetGlobalContext();
    module = LLVMModuleCreateWithNameInContext("program", context);
    builder = LLVMCreateBuilderInContext(context);

    // Crear el tipo Object como un struct vacío
    // Esto crea un tipo nombrado "Object" en el contexto
    object_type = LLVMStructCreateNamed(context, "Object");
    LLVMTypeRef idType = LLVMInt32Type();
    LLVMTypeRef structFields[] = { idType };
    LLVMStructSetBody(object_type, structFields, 1, 0);

    // Inicializar la variable global de profundidad de stack
    current_stack_depth_var = LLVMAddGlobal(module, LLVMInt32Type(), "current_stack_depth");
    LLVMSetInitializer(current_stack_depth_var, LLVMConstInt(LLVMInt32Type(), 0, 0));
    LLVMSetLinkage(current_stack_depth_var, LLVMPrivateLinkage);
    
    // Declare external functions right after initialization
    declare_external_functions();
}

void free_llvm_resources(void) {
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    current_stack_depth_var = NULL;
}

void declare_external_functions(void) {
    // Declarar funciones estándar de C
    LLVMTypeRef strcpy_type = LLVMFunctionType(LLVMVoidType(),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8Type(), 0),
            LLVMPointerType(LLVMInt8Type(), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcpy", strcpy_type);

    LLVMTypeRef strcat_type = LLVMFunctionType(LLVMVoidType(),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8Type(), 0),
            LLVMPointerType(LLVMInt8Type(), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcat", strcat_type);

    // Funciones matemáticas
    LLVMTypeRef double_func_type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
    LLVMAddFunction(module, "sqrt", double_func_type);
    LLVMAddFunction(module, "sin", double_func_type);
    LLVMAddFunction(module, "cos", double_func_type);
    LLVMAddFunction(module, "exp", double_func_type);
    LLVMAddFunction(module, "log", double_func_type);
    
    // Otras funciones estándar
    LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMAddFunction(module, "rand", rand_type);

    LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64Type(),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
    LLVMAddFunction(module, "strlen", strlen_type);

    LLVMTypeRef malloc_type = LLVMFunctionType(
        LLVMPointerType(LLVMInt8Type(), 0),
        (LLVMTypeRef[]){LLVMInt64Type()}, 1, 0);
    LLVMAddFunction(module, "malloc", malloc_type);

    LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8Type(), 0),
            LLVMInt64Type(),
            LLVMPointerType(LLVMInt8Type(), 0)
        }, 3, 1);
    LLVMAddFunction(module, "snprintf", snprintf_type);

    LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8Type(), 0),
            LLVMPointerType(LLVMInt8Type(), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcmp", strcmp_type);

    LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
    LLVMAddFunction(module, "pow", pow_type);

    LLVMTypeRef fmod_type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
    LLVMAddFunction(module, "fmod", fmod_type);

    // Declarar printf
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
    LLVMValueRef printf_func = LLVMAddFunction(module, "printf", printf_type);
    LLVMSetLinkage(printf_func, LLVMExternalLinkage);

    // Asegurar que exit() está declarado
    if (!LLVMGetNamedFunction(module, "exit")) {
        LLVMTypeRef exit_type = LLVMFunctionType(LLVMVoidType(), 
            (LLVMTypeRef[]){LLVMInt32Type()}, 1, 0);
        LLVMAddFunction(module, "exit", exit_type);
    }

    // Asegurar que puts() está declarado
    if (!LLVMGetNamedFunction(module, "puts")) {
        LLVMTypeRef puts_type = LLVMFunctionType(LLVMInt32Type(), 
            (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
        LLVMAddFunction(module, "puts", puts_type);
    }
}