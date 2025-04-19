#include "llvm_gen.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para strcat y strcmp

static LLVMModuleRef module;
static LLVMBuilderRef builder;
static LLVMContextRef context;

// Nueva estructura para manejar variables LLVM con scope
typedef struct ScopeVarEntry {
    char* name;
    LLVMValueRef alloca;
    struct ScopeVarEntry* next;
} ScopeVarEntry;

typedef struct LLVMScope {
    ScopeVarEntry* variables;
    struct LLVMScope* parent;
} LLVMScope;

static LLVMScope* current_scope = NULL;

// Funciones auxiliares actualizadas
static LLVMValueRef codegen(ASTNode* node);
static LLVMValueRef lookup_variable(const char* name);
static void declare_variable(const char* name, LLVMValueRef alloca);

// Nueva función para manejar scopes
static void push_scope() {
    LLVMScope* new_scope = malloc(sizeof(LLVMScope));
    new_scope->variables = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

static void pop_scope() {
    if (!current_scope) return;
    
    // Liberar variables del scope actual
    ScopeVarEntry* current = current_scope->variables;
    while (current) {
        ScopeVarEntry* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    
    LLVMScope* parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
}

void init_llvm() {
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    context = LLVMGetGlobalContext();
    module = LLVMModuleCreateWithNameInContext("calculadora", context);
    builder = LLVMCreateBuilderInContext(context);
}

void free_llvm_resources() {
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    
    // Liberar scopes
    while (current_scope) {
        pop_scope();
    }
}

static LLVMValueRef codegen(ASTNode* node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_PROGRAM: {
            push_scope(); // Crear scope global
            LLVMValueRef last = NULL;
            for (int i = 0; i < node->data.program_node.count; i++) {
                last = codegen(node->data.program_node.statements[i]);
            }
            pop_scope();
            return last;
        }

        case NODE_NUMBER: {
            return LLVMConstReal(LLVMDoubleType(), node->data.number_value);
        }

        case NODE_VARIABLE: {
            LLVMValueRef alloca = lookup_variable(node->data.variable_name);
            if (!alloca) {
                fprintf(stderr, "Variable no declarada: %s\n", node->data.variable_name);
                exit(1);
            }
            // Cargar usando el tipo correcto
            LLVMTypeRef var_type = LLVMGetElementType(LLVMTypeOf(alloca));
            return LLVMBuildLoad2(builder, var_type, alloca, "load");
        }

        case NODE_ASSIGNMENT: {
            const char* var_name = node->data.op_node.left->data.variable_name;
            LLVMValueRef value = codegen(node->data.op_node.right);
            
            LLVMValueRef alloca = lookup_variable(var_name);
            if (!alloca) {
                // Nueva variable - crear alloca con el tipo correcto
                LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
                LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(current_block));
                LLVMPositionBuilderAtEnd(builder, entry_block);
                
                // Usar tipo correcto según el valor a asignar
                LLVMTypeRef var_type;
                if (node->data.op_node.right->return_type->kind == TYPE_STRING) {
                    var_type = LLVMPointerType(LLVMInt8Type(), 0);
                } else {
                    var_type = LLVMDoubleType();
                }
                
                alloca = LLVMBuildAlloca(builder, var_type, var_name);
                declare_variable(var_name, alloca);
                
                LLVMPositionBuilderAtEnd(builder, current_block);
            }
            
            return LLVMBuildStore(builder, value, alloca);
        }

        case NODE_BINARY_OP: {
            LLVMValueRef L = codegen(node->data.op_node.left);
            LLVMValueRef R = codegen(node->data.op_node.right);

            // Para strings
            if (node->data.op_node.left->return_type->kind == TYPE_STRING) {
                switch (node->data.op_node.op) {
                    case OP_CONCAT:
                    case OP_DCONCAT: {
                        // Declarar funciones necesarias
                        LLVMValueRef strlen_func = LLVMGetNamedFunction(module, "strlen");
                        if (!strlen_func) {
                            LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64Type(),
                                (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
                            strlen_func = LLVMAddFunction(module, "strlen", strlen_type);
                        }

                        LLVMValueRef malloc_func = LLVMGetNamedFunction(module, "malloc");
                        if (!malloc_func) {
                            LLVMTypeRef malloc_type = LLVMFunctionType(
                                LLVMPointerType(LLVMInt8Type(), 0),
                                (LLVMTypeRef[]){LLVMInt64Type()}, 1, 0);
                            malloc_func = LLVMAddFunction(module, "malloc", malloc_type);
                        }

                        // Obtener longitudes de los strings
                        LLVMValueRef len1 = LLVMBuildCall2(builder, 
                            LLVMGetElementType(LLVMTypeOf(strlen_func)),
                            strlen_func, (LLVMValueRef[]){L}, 1, "len1");
                        LLVMValueRef len2 = LLVMBuildCall2(builder,
                            LLVMGetElementType(LLVMTypeOf(strlen_func)),
                            strlen_func, (LLVMValueRef[]){R}, 1, "len2");

                        // Calcular tamaño total necesario
                        LLVMValueRef total_len = LLVMBuildAdd(builder, len1, len2, "total_len");
                        if (node->data.op_node.op == OP_DCONCAT) {
                            // Agregar 1 para el espacio si es @@
                            total_len = LLVMBuildAdd(builder, total_len, 
                                LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_space");
                        }
                        // Agregar 1 para el null terminator
                        total_len = LLVMBuildAdd(builder, total_len,
                            LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_null");

                        // Asignar memoria
                        LLVMValueRef buffer = LLVMBuildCall2(builder,
                            LLVMGetElementType(LLVMTypeOf(malloc_func)),
                            malloc_func, (LLVMValueRef[]){total_len}, 1, "buffer");

                        // Copiar strings
                        LLVMBuildCall2(builder, 
                            LLVMFunctionType(LLVMVoidType(),
                                (LLVMTypeRef[]){
                                    LLVMPointerType(LLVMInt8Type(), 0),
                                    LLVMPointerType(LLVMInt8Type(), 0)
                                }, 2, 0),
                            LLVMGetNamedFunction(module, "strcpy"),
                            (LLVMValueRef[]){buffer, L}, 2, "");

                        if (node->data.op_node.op == OP_DCONCAT) {
                            // Agregar espacio si es @@
                            LLVMValueRef space_ptr = LLVMBuildGEP2(builder, 
                                LLVMInt8Type(), 
                                buffer,
                                &len1,  // Cambiado: solo pasamos el puntero a len1
                                1,
                                "space_ptr");
                            LLVMBuildStore(builder, 
                                LLVMConstInt(LLVMInt8Type(), ' ', 0),
                                space_ptr);
                            
                            // Actualizar posición para segundo string
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
                            // Concatenar directamente si es @
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

                    // Comparaciones de strings
                    case OP_EQ:
                    case OP_NEQ:
                    case OP_GR:
                    case OP_GRE:
                    case OP_LS:
                    case OP_LSE: {
                        // Declarar función strcmp
                        LLVMValueRef strcmp_func = LLVMGetNamedFunction(module, "strcmp");
                        if (!strcmp_func) {
                            LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(),
                                (LLVMTypeRef[]){
                                    LLVMPointerType(LLVMInt8Type(), 0),
                                    LLVMPointerType(LLVMInt8Type(), 0)
                                }, 2, 0);
                            strcmp_func = LLVMAddFunction(module, "strcmp", strcmp_type);
                        }

                        LLVMValueRef cmp = LLVMBuildCall2(builder,
                            LLVMGetElementType(LLVMTypeOf(strcmp_func)),
                            strcmp_func,
                            (LLVMValueRef[]){L, R}, 2, "strcmp_result");

                        switch (node->data.op_node.op) {
                            case OP_EQ:  return LLVMBuildICmp(builder, LLVMIntEQ, cmp, LLVMConstInt(LLVMInt32Type(), 0, 0), "str_eq");
                            case OP_NEQ: return LLVMBuildICmp(builder, LLVMIntNE, cmp, LLVMConstInt(LLVMInt32Type(), 0, 0), "str_ne");
                            case OP_GR:  return LLVMBuildICmp(builder, LLVMIntSGT, cmp, LLVMConstInt(LLVMInt32Type(), 0, 0), "str_gt");
                            case OP_GRE: return LLVMBuildICmp(builder, LLVMIntSGE, cmp, LLVMConstInt(LLVMInt32Type(), 0, 0), "str_ge");
                            case OP_LS:  return LLVMBuildICmp(builder, LLVMIntSLT, cmp, LLVMConstInt(LLVMInt32Type(), 0, 0), "str_lt");
                            case OP_LSE: return LLVMBuildICmp(builder, LLVMIntSLE, cmp, LLVMConstInt(LLVMInt32Type(), 0, 0), "str_le");
                            default: break;
                        }
                    }
                }
            }

            // Para booleanos
            if (node->data.op_node.left->return_type->kind == TYPE_BOOLEAN) {
                switch (node->data.op_node.op) {
                    case OP_EQ:  return LLVMBuildICmp(builder, LLVMIntEQ, L, R, "bool_eq");
                    case OP_NEQ: return LLVMBuildICmp(builder, LLVMIntNE, L, R, "bool_ne");
                    case OP_AND: return LLVMBuildAnd(builder, L, R, "bool_and");
                    case OP_OR:  return LLVMBuildOr(builder, L, R, "bool_or");
                    default: break;
                }
            }

            // Si los operandos son números, convertirlos para comparación
            if (node->data.op_node.left->return_type->kind == TYPE_NUMBER) {
                switch (node->data.op_node.op) {
                    case OP_ADD: return LLVMBuildFAdd(builder, L, R, "add_tmp");
                    case OP_SUB: return LLVMBuildFSub(builder, L, R, "sub_tmp");
                    case OP_MUL: return LLVMBuildFMul(builder, L, R, "mul_tmp");
                    case OP_DIV: return LLVMBuildFDiv(builder, L, R, "div_tmp");
                    case OP_MOD: {
                        // Añadir función fmod si no existe
                        LLVMTypeRef fmod_type = LLVMFunctionType(LLVMDoubleType(),
                            (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
                        LLVMValueRef fmod_func = LLVMGetNamedFunction(module, "fmod");
                        if (!fmod_func) {
                            fmod_func = LLVMAddFunction(module, "fmod", fmod_type);
                        }
                        // Llamar a fmod(L, R)
                        return LLVMBuildCall2(builder, fmod_type, fmod_func,
                            (LLVMValueRef[]){L, R}, 2, "mod_tmp");
                    }
                    case OP_POW: {
                        LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleType(),
                            (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
                        LLVMValueRef pow_func = LLVMGetNamedFunction(module, "pow");
                        if (!pow_func) {
                            pow_func = LLVMAddFunction(module, "pow", pow_type);
                        }
                        return LLVMBuildCall2(builder, pow_type, pow_func, 
                            (LLVMValueRef[]){L, R}, 2, "pow_tmp");
                    }
                    // Comparaciones numéricas
                    case OP_EQ:  return LLVMBuildFCmp(builder, LLVMRealOEQ, L, R, "eq_tmp");
                    case OP_NEQ: return LLVMBuildFCmp(builder, LLVMRealONE, L, R, "neq_tmp");
                    case OP_GR:  return LLVMBuildFCmp(builder, LLVMRealOGT, L, R, "gt_tmp");
                    case OP_GRE: return LLVMBuildFCmp(builder, LLVMRealOGE, L, R, "ge_tmp");
                    case OP_LS:  return LLVMBuildFCmp(builder, LLVMRealOLT, L, R, "lt_tmp");
                    case OP_LSE: return LLVMBuildFCmp(builder, LLVMRealOLE, L, R, "le_tmp");
                }
            }
            
            fprintf(stderr, "Operador desconocido o no soportado\n");
            exit(1);
        }

        case NODE_UNARY_OP: {
            LLVMValueRef operand = codegen(node->data.op_node.left);
            
            switch (node->data.op_node.op) {
                case OP_NEGATE: 
                    return LLVMBuildFNeg(builder, operand, "neg_tmp");
                case OP_NOT:
                    return LLVMBuildNot(builder, operand, "not_tmp");
            }
            
            fprintf(stderr, "Operador unario desconocido\n");
            exit(1);
        }

        case NODE_BOOLEAN: {
            // Convertir "true"/"false" string a valor booleano LLVM
            int value = strcmp(node->data.string_value, "true") == 0 ? 1 : 0;
            return LLVMConstInt(LLVMInt1Type(), value, 0);  // Usar 0 en lugar de false
        }

        case NODE_STRING: {
            // Crear string global y retornar puntero
            return LLVMBuildGlobalStringPtr(builder, node->data.string_value, "str");
        }

        case NODE_PRINT: {
            LLVMValueRef expr = codegen(node->data.op_node.left);
            if (!expr) return NULL;
            
            // Obtener o declarar función printf
            LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
            if (!printf_func) {
                LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                    (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
                printf_func = LLVMAddFunction(module, "printf", printf_type);
            }
            
            const char* format;
            LLVMValueRef format_str;
            LLVMValueRef* args;
            int num_args;
            
            // Seleccionar formato según el tipo
            switch (node->data.op_node.left->return_type->kind) {
                case TYPE_NUMBER:
                    format = "%.1f\n";
                    format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
                    args = (LLVMValueRef[]){format_str, expr};
                    num_args = 2;
                    break;
                    
                case TYPE_BOOLEAN:
                    format_str = LLVMBuildGlobalStringPtr(builder, "%s\n", "fmt");
                    LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
                    LLVMValueRef false_str = LLVMBuildGlobalStringPtr(builder, "false", "false_str");
                    LLVMValueRef cond_str = LLVMBuildSelect(builder, expr, true_str, false_str, "bool_str");
                    args = (LLVMValueRef[]){format_str, cond_str};
                    num_args = 2;
                    break;
                    
                case TYPE_STRING:
                    format = "%s\n";
                    format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
                    args = (LLVMValueRef[]){format_str, expr};
                    num_args = 2;
                    break;
                    
                default:
                    fprintf(stderr, "Tipo no soportado para print\n");
                    exit(1);
            }
            
            // Construir llamada a printf
            LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
            return LLVMBuildCall2(builder, printf_type, printf_func, args, num_args, "printf_call");
        }
    }
}

// Implementaciones actualizadas de funciones auxiliares
static LLVMValueRef lookup_variable(const char* name) {
    LLVMScope* scope = current_scope;
    while (scope) {
        ScopeVarEntry* entry = scope->variables;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return entry->alloca;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

static void declare_variable(const char* name, LLVMValueRef alloca) {
    ScopeVarEntry* entry = malloc(sizeof(ScopeVarEntry));
    entry->name = strdup(name);
    entry->alloca = alloca;
    entry->next = current_scope->variables;
    current_scope->variables = entry;
}

void generate_llvm_code(ASTNode* ast, const char* filename) {
    init_llvm();
    
    // Declarar funciones de C necesarias con tipos correctos
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

    // Inicializar scope global
    current_scope = NULL;
    push_scope();
    
    // Declarar printf como función externa
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
    LLVMValueRef printf_func = LLVMAddFunction(module, "printf", printf_type);
    LLVMSetLinkage(printf_func, LLVMExternalLinkage);
    
    // Crear función main
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_type);
    
    // Crear bloque de entrada
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // Generar código para el AST
    if (ast != NULL) {
        // Verificar si es un nodo programa
        if (ast->type == NODE_PROGRAM) {
            // Generar código para cada statement en el programa
            for (int i = 0; i < ast->data.program_node.count; i++) {
                LLVMValueRef stmt = codegen(ast->data.program_node.statements[i]);
                if (!stmt) {
                    fprintf(stderr, "Error generando código para statement %d\n", i);
                }
            }
        } else {
            // Si no es un nodo programa, generar código directamente
            codegen(ast);
        }
    }
    
    // Retornar 0 de main
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    
    // Escribir a archivo
    char* error = NULL;
    if (LLVMPrintModuleToFile(module, filename, &error)) {
        fprintf(stderr, "Error escribiendo IR: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }
    
    pop_scope(); // Limpiar scope global
    free_llvm_resources();
}