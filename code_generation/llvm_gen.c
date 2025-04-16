#include "llvm_gen.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <stdio.h>

static LLVMModuleRef module;
static LLVMBuilderRef builder;
static LLVMContextRef context;

typedef struct SymbolEntry {
    char name;
    LLVMValueRef alloca;
    struct SymbolEntry* next;
} SymbolEntry;

static SymbolEntry* symbol_table = NULL;

// Funciones auxiliares
static LLVMValueRef codegen(ASTNode* node);
static LLVMValueRef lookup_symbol(char name);
static void add_symbol(char name, LLVMValueRef alloca);

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
    
    // Liberar tabla de símbolos
    SymbolEntry* entry = symbol_table;
    while (entry) {
        SymbolEntry* next = entry->next;
        free(entry);
        entry = next;
    }
    symbol_table = NULL;
}

LLVMValueRef codegen(ASTNode* node) {
   if (!node) return NULL;

    switch (node->type) {
        case NODE_NUMBER:
            return LLVMConstReal(LLVMDoubleType(), node->data.number_value);

        case NODE_VARIABLE: {
            LLVMValueRef alloca = lookup_symbol(node->data.variable_name);
            if (!alloca) {
                fprintf(stderr, "Variable no declarada: %c\n", node->data.variable_name);
                exit(1);
            }
            return LLVMBuildLoad2(builder, LLVMDoubleType(), alloca, "load");
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

        case NODE_ASSIGNMENT: {
            char var_name = node->data.op_node.left->data.variable_name;
            LLVMValueRef value = codegen(node->data.op_node.right);
            
            // Crear alloca si no existe
            LLVMValueRef alloca = lookup_symbol(var_name);
            if (!alloca) {
                LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
                LLVMPositionBuilderAtEnd(builder, LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(current_block)));
                char name_str[2] = {var_name, '\0'};
                alloca = LLVMBuildAlloca(builder, LLVMDoubleType(), name_str);
                LLVMPositionBuilderAtEnd(builder, current_block);
                add_symbol(var_name, alloca);
            }
            
            LLVMBuildStore(builder, value, alloca);
            return value;
        }

        default:
            fprintf(stderr, "Nodo AST no reconocido\n");
            exit(1);
    }
}

void generate_llvm_code(ASTNode* ast, const char* filename) {
    init_llvm();
    
    // Crear función main
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_type);
    
    // Crear bloque de entrada
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // Generar código para el AST
    LLVMValueRef result = codegen(ast);
    
    // Configurar printf
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), 
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
    LLVMValueRef printf_func = LLVMAddFunction(module, "printf", printf_type);
    
    // Imprimir resultado
    char* format_str = "Resultado: %f\n";
    LLVMValueRef format = LLVMBuildGlobalStringPtr(builder, format_str, "fmt");
    LLVMBuildCall2(builder, printf_type, printf_func, 
        (LLVMValueRef[]){format, result}, 2, "");
    
    // Retornar 0
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    
    // Escribir a archivo
    char* error = NULL;
    if (LLVMPrintModuleToFile(module, filename, &error)) {
        fprintf(stderr, "Error escribiendo IR: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }
    
    free_llvm_resources();
}
// Implementaciones de funciones auxiliares
LLVMValueRef lookup_symbol(char name) {
    SymbolEntry* entry = symbol_table;
    while (entry) {
        if (entry->name == name) {
            return entry->alloca;
        }
        entry = entry->next;
    }
    return NULL;
}

void add_symbol(char name, LLVMValueRef alloca) {
    SymbolEntry* entry = malloc(sizeof(SymbolEntry));
    entry->name = name;
    entry->alloca = alloca;
    entry->next = symbol_table;
    symbol_table = entry;
}