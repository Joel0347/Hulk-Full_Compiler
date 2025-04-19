#include "llvm_gen.h"
#include "../scope/scope.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

LLVMValueRef codegen(ASTNode* node) {
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
            return LLVMBuildLoad2(builder, LLVMDoubleType(), alloca, "load");
        }

        case NODE_ASSIGNMENT: {
            const char* var_name = node->data.op_node.left->data.variable_name;
            LLVMValueRef value = codegen(node->data.op_node.right);
            
            LLVMValueRef alloca = lookup_variable(var_name);
            if (!alloca) {
                // Nueva variable - crear alloca
                LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
                LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(current_block));
                LLVMPositionBuilderAtEnd(builder, entry_block);
                
                alloca = LLVMBuildAlloca(builder, LLVMDoubleType(), var_name);
                declare_variable(var_name, alloca);
                
                LLVMPositionBuilderAtEnd(builder, current_block);
            }
            
            return LLVMBuildStore(builder, value, alloca);
        }

        case NODE_BINARY_OP: {
            LLVMValueRef L = codegen(node->data.op_node.left);
            LLVMValueRef R = codegen(node->data.op_node.right);
            
            switch (node->data.op_node.op) {
                case OP_ADD: return LLVMBuildFAdd(builder, L, R, "add_tmp");
                case OP_SUB: return LLVMBuildFSub(builder, L, R, "sub_tmp");
                case OP_MUL: return LLVMBuildFMul(builder, L, R, "mul_tmp");
                case OP_DIV: return LLVMBuildFDiv(builder, L, R, "div_tmp");
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
                default: 
                    fprintf(stderr, "Operador desconocido\n");
                    exit(1);
            }
        }

        case NODE_UNARY_OP: {
            LLVMValueRef operand = codegen(node->data.op_node.left);
            return LLVMBuildFNeg(builder, operand, "neg_tmp");
        }

        case NODE_STRING: {
            // Create a global string constant
            LLVMValueRef str = LLVMBuildGlobalStringPtr(builder, node->data.string_value, "str");
            return str;
        }

        case NODE_PRINT: {
            LLVMValueRef expr = codegen(node->data.op_node.left);
            if (!expr) return NULL;
            
            // Declarar printf como variádica
            LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
            if (!printf_func) {
                LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                    (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
                printf_func = LLVMAddFunction(module, "printf", printf_type);
            }
            
            // Verificar si estamos imprimiendo un string o un número
            if (node->data.op_node.left->type == NODE_STRING) {
                // Para strings, usar formato %s
                const char* format = "%s\n";
                LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
                
                // Construir la llamada
                LLVMValueRef args[] = {format_str, expr};
                return LLVMBuildCall2(builder,
                    LLVMFunctionType(LLVMInt32Type(), 
                        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1),
                    printf_func,
                    args,
                    2,
                    "printf_call");
            } else {
                // Para números, mantener el formato %.1f
                const char* format = "%.1f\n";
                LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
                
                LLVMValueRef args[] = {format_str, expr};
                return LLVMBuildCall2(builder,
                    LLVMFunctionType(LLVMInt32Type(), 
                        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1),
                    printf_func,
                    args,
                    2,
                    "printf_call");
            }
        }

        default:
            fprintf(stderr, "Nodo AST no reconocido\n");
            exit(1);
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