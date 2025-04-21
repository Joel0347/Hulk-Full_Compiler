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
static void update_variable(const char* name, LLVMValueRef new_alloca);

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
            
            // Determinar el tipo correcto para la nueva asignación
            LLVMTypeRef new_type;
            if (type_equals(node->data.op_node.right->return_type, &TYPE_STRING_INST)) {
                new_type = LLVMPointerType(LLVMInt8Type(), 0);
            } else if (type_equals(node->data.op_node.right->return_type, &TYPE_BOOLEAN_INST)) {
                new_type = LLVMInt1Type();
            } else {
                new_type = LLVMDoubleType();
            }

            // Obtener el bloque actual y de entrada para las allocas
            LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
            LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(current_block));
            LLVMPositionBuilderAtEnd(builder, entry_block);

            // Verificar si la variable ya existe
            LLVMValueRef existing_alloca = lookup_variable(var_name);
            LLVMValueRef alloca;

            if (existing_alloca) {
                // Si el tipo es diferente, crear nueva asignación
                LLVMTypeRef existing_type = LLVMGetElementType(LLVMTypeOf(existing_alloca));
                if (existing_type != LLVMTypeOf(value)) {
                    alloca = LLVMBuildAlloca(builder, new_type, var_name);
                    update_variable(var_name, alloca);
                } else {
                    alloca = existing_alloca;
                }
            } else {
                // Nueva variable
                alloca = LLVMBuildAlloca(builder, new_type, var_name);
                declare_variable(var_name, alloca);
            }

            // Volver al bloque actual y almacenar el valor
            LLVMPositionBuilderAtEnd(builder, current_block);
            return LLVMBuildStore(builder, value, alloca);
        }

        case NODE_BINARY_OP: {
            LLVMValueRef L = codegen(node->data.op_node.left);
            LLVMValueRef R = codegen(node->data.op_node.right);

            OperatorTypeRule possible_rule = create_op_rule(
                node->data.op_node.left->return_type,
                node->data.op_node.right->return_type,
                node->return_type,
                node->data.op_node.op
            );

            if (find_op_match(&possible_rule)) {
                // Generar código para la operación
                // ...
            }

            // Manejo de operaciones con strings (concatenación)
            if (node->data.op_node.op == OP_CONCAT || node->data.op_node.op == OP_DCONCAT) {
                // Convertir números a strings si es necesario
                if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER_INST)) {
                    // Declarar snprintf si no existe
                    LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
                        (LLVMTypeRef[]){
                            LLVMPointerType(LLVMInt8Type(), 0),
                            LLVMInt64Type(),
                            LLVMPointerType(LLVMInt8Type(), 0)
                        }, 3, 1);
                    LLVMValueRef snprintf_func = LLVMGetNamedFunction(module, "snprintf");
                    if (!snprintf_func) {
                        snprintf_func = LLVMAddFunction(module, "snprintf", snprintf_type);
                    }

                    // Asignar buffer para el número
                    LLVMValueRef num_buffer = LLVMBuildAlloca(builder, 
                        LLVMArrayType(LLVMInt8Type(), 32), "num_buffer");

                    // Convertir el array buffer a puntero
                    LLVMValueRef buffer_ptr = LLVMBuildBitCast(builder, num_buffer,
                        LLVMPointerType(LLVMInt8Type(), 0), "buffer_cast");
                    
                    // Convertir número a string
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
                    // Mismo proceso para el segundo operando
                    LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
                        (LLVMTypeRef[]){
                            LLVMPointerType(LLVMInt8Type(), 0),
                            LLVMInt64Type(),
                            LLVMPointerType(LLVMInt8Type(), 0)
                        }, 3, 1);
                    LLVMValueRef snprintf_func = LLVMGetNamedFunction(module, "snprintf");
                    if (!snprintf_func) {
                        snprintf_func = LLVMAddFunction(module, "snprintf", snprintf_type);
                    }

                    LLVMValueRef num_buffer = LLVMBuildAlloca(builder,
                        LLVMArrayType(LLVMInt8Type(), 32), "num_buffer");

                    // Convertir el array buffer a puntero
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

                // Declarar strlen si no existe
                LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64Type(),
                    (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
                LLVMValueRef strlen_func = LLVMGetNamedFunction(module, "strlen");
                if (!strlen_func) {
                    strlen_func = LLVMAddFunction(module, "strlen", strlen_type);
                }

                // Obtener longitudes de los strings
                LLVMValueRef len1 = LLVMBuildCall2(builder, strlen_type, strlen_func, 
                    (LLVMValueRef[]){L}, 1, "len1");
                LLVMValueRef len2 = LLVMBuildCall2(builder, strlen_type, strlen_func,
                    (LLVMValueRef[]){R}, 1, "len2");

                // Calcular tamaño total
                LLVMValueRef total_len = LLVMBuildAdd(builder, len1, len2, "total_len");
                if (node->data.op_node.op == OP_DCONCAT) {
                    // Agregar 1 para el espacio en @@
                    total_len = LLVMBuildAdd(builder, total_len,
                        LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_space");
                }
                // Agregar 1 para el null terminator
                total_len = LLVMBuildAdd(builder, total_len,
                    LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_null");

                // Asignar memoria para el resultado
                LLVMValueRef malloc_func = LLVMGetNamedFunction(module, "malloc");
                if (!malloc_func) {
                    LLVMTypeRef malloc_type = LLVMFunctionType(
                        LLVMPointerType(LLVMInt8Type(), 0),
                        (LLVMTypeRef[]){LLVMInt64Type()}, 1, 0);
                    malloc_func = LLVMAddFunction(module, "malloc", malloc_type);
                }

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

                if (node->data.op_node.op == OP_DCONCAT) {
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

            // Operaciones booleanas de comparación
            if (type_equals(node->data.op_node.left->return_type, &TYPE_BOOLEAN_INST) &&
                type_equals(node->data.op_node.right->return_type, &TYPE_BOOLEAN_INST)) {
                switch (node->data.op_node.op) {
                    case OP_EQ:
                        return LLVMBuildICmp(builder, LLVMIntEQ, L, R, "bool_eq_tmp");
                    case OP_NEQ:
                        return LLVMBuildICmp(builder, LLVMIntNE, L, R, "bool_neq_tmp");
                    default:
                        break;
                }
            }

            // Si los operandos son números
            if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER_INST)) {
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
                // Usar strcmp para comparar strings
                LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(),
                    (LLVMTypeRef[]){
                        LLVMPointerType(LLVMInt8Type(), 0),
                        LLVMPointerType(LLVMInt8Type(), 0)
                    }, 2, 0);
                LLVMValueRef strcmp_func = LLVMGetNamedFunction(module, "strcmp");
                if (!strcmp_func) {
                    strcmp_func = LLVMAddFunction(module, "strcmp", strcmp_type);
                }

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

            fprintf(stderr, "Operador desconocido o no soportado\n");
            exit(1);
        }

        case NODE_UNARY_OP: {
            LLVMValueRef operand = codegen(node->data.op_node.left);
            
            switch (node->data.op_node.op) {
                case OP_NEGATE: 
                    if (type_equals(node->data.op_node.left->return_type, &TYPE_NUMBER_INST)) {
                        return LLVMBuildFNeg(builder, operand, "neg_tmp");
                    }
                    break;
                case OP_NOT:
                    if (type_equals(node->data.op_node.left->return_type, &TYPE_BOOLEAN_INST)) {
                        // Negar un booleano es diferente que negar un número
                        return LLVMBuildNot(builder, operand, "not_tmp");
                    }
                    break;
            }

            fprintf(stderr, "Operador unario desconocido\n");
            exit(1);
        }

        case NODE_BOOLEAN: {
            // Convertir "true"/"false" string a valor booleano LLVM
            int value = strcmp(node->data.string_value, "true") == 0 ? 1 : 0;
            return LLVMConstInt(LLVMInt1Type(), value, 0);  
        }

        case NODE_STRING: {
            // Crear string global y retornar puntero
            return LLVMBuildGlobalStringPtr(builder, node->data.string_value, "str");
        }

        case NODE_BUILTIN_FUNC: {
            // Built-in functions
            if (strcmp(node->data.func_node.name, "print") == 0) {
                // Obtener o declarar función printf
                LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
                if (!printf_func) {
                    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
                    printf_func = LLVMAddFunction(module, "printf", printf_type);
                }

                // Si no hay argumentos, solo imprime una nueva línea
                if (node->data.func_node.arg_count == 0) {
                    LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, "\n", "newline");
                    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
                    return LLVMBuildCall2(builder, printf_type, printf_func,
                        (LLVMValueRef[]){format_str}, 1, "printf_call");
                }

                // Generar código para el argumento
                LLVMValueRef arg = codegen(node->data.func_node.args[0]);
                if (!arg) return NULL;

                const char* format;
                LLVMValueRef format_str;
                LLVMValueRef* args;
                int num_args;

                // Seleccionar formato según el tipo del argumento
                if (node->data.func_node.arg_count > 0) {
                    Type* arg_type = node->data.func_node.args[0]->return_type;
                    if (type_equals(arg_type, &TYPE_NUMBER_INST)) {
                        format = "%g\n";
                        format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
                        args = (LLVMValueRef[]){format_str, arg};
                        num_args = 2;
                    }
                    else if (type_equals(arg_type, &TYPE_BOOLEAN_INST)) {
                        format_str = LLVMBuildGlobalStringPtr(builder, "%s\n", "fmt");
                        LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
                        LLVMValueRef false_str = LLVMBuildGlobalStringPtr(builder, "false", "false_str");
                        LLVMValueRef cond_str = LLVMBuildSelect(builder, arg, true_str, false_str, "bool_str");
                        args = (LLVMValueRef[]){format_str, cond_str};
                        num_args = 2;
                    }
                    else if (type_equals(arg_type, &TYPE_STRING_INST)) {
                        format = "%s\n";
                        format_str = LLVMBuildGlobalStringPtr(builder, format, "fmt");
                        args = (LLVMValueRef[]){format_str, arg};
                        num_args = 2;
                    }
                    else {
                        fprintf(stderr, "Tipo no soportado para print\n");
                        exit(1);
                    }
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
                if (!sqrt_func) {
                    sqrt_func = LLVMAddFunction(module, "sqrt", sqrt_type);
                }
                return LLVMBuildCall2(builder, sqrt_type, sqrt_func, 
                    (LLVMValueRef[]){arg}, 1, "sqrt_tmp");
            }
            else if (strcmp(node->data.func_node.name, "sin") == 0) {
                LLVMValueRef arg = codegen(node->data.func_node.args[0]);
                LLVMTypeRef sin_type = LLVMFunctionType(LLVMDoubleType(),
                    (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
                LLVMValueRef sin_func = LLVMGetNamedFunction(module, "sin");
                if (!sin_func) {
                    sin_func = LLVMAddFunction(module, "sin", sin_type);
                }
                return LLVMBuildCall2(builder, sin_type, sin_func,
                    (LLVMValueRef[]){arg}, 1, "sin_tmp");
            }
            else if (strcmp(node->data.func_node.name, "cos") == 0) {
                LLVMValueRef arg = codegen(node->data.func_node.args[0]);
                LLVMTypeRef cos_type = LLVMFunctionType(LLVMDoubleType(),
                    (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
                LLVMValueRef cos_func = LLVMGetNamedFunction(module, "cos");
                if (!cos_func) {
                    cos_func = LLVMAddFunction(module, "cos", cos_type);
                }
                return LLVMBuildCall2(builder, cos_type, cos_func,
                    (LLVMValueRef[]){arg}, 1, "cos_tmp");
            }
            else if (strcmp(node->data.func_node.name, "exp") == 0) {
                LLVMValueRef arg = codegen(node->data.func_node.args[0]);
                LLVMTypeRef exp_type = LLVMFunctionType(LLVMDoubleType(),
                    (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
                LLVMValueRef exp_func = LLVMGetNamedFunction(module, "exp");
                if (!exp_func) {
                    exp_func = LLVMAddFunction(module, "exp", exp_type);
                }
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
                    if (!log_func) {
                        log_func = LLVMAddFunction(module, "log", log_type);
                    }
                    return LLVMBuildCall2(builder, log_type, log_func,
                        (LLVMValueRef[]){arg}, 1, "log_tmp");
                } else {
                    // log(base, x) = log(x) / log(base)
                    LLVMValueRef base = codegen(node->data.func_node.args[0]);
                    LLVMValueRef x = codegen(node->data.func_node.args[1]);
                    
                    LLVMTypeRef log_type = LLVMFunctionType(LLVMDoubleType(),
                        (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
                    LLVMValueRef log_func = LLVMGetNamedFunction(module, "log");
                    if (!log_func) {
                        log_func = LLVMAddFunction(module, "log", log_type);
                    }

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
                if (!rand_func) {
                    rand_func = LLVMAddFunction(module, "rand", rand_type);
                }
                
                // Convertir el resultado entero a double dividiendo por RAND_MAX
                LLVMValueRef rand_val = LLVMBuildCall2(builder, rand_type, rand_func, NULL, 0, "rand_tmp");
                LLVMValueRef rand_max = LLVMConstReal(LLVMDoubleType(), RAND_MAX);
                LLVMValueRef rand_double = LLVMBuildSIToFP(builder, rand_val, LLVMDoubleType(), "rand_double");
                
                return LLVMBuildFDiv(builder, rand_double, rand_max, "rand_result");
            }

            fprintf(stderr, "Función builtin no soportada: %s\n", node->data.func_node.name);
            exit(1);
        }

        case NODE_BLOCK: {
            // Crear nuevo scope para el bloque
            push_scope();
            
            LLVMValueRef last_val = NULL;
            // Generar código para cada statement en el bloque
            for (int i = 0; i < node->data.program_node.count; i++) {
                last_val = codegen(node->data.program_node.statements[i]);
            }
            
            // Eliminar el scope del bloque
            pop_scope();
            
            // Retornar el valor de la última expresión
            return last_val;
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

static void update_variable(const char* name, LLVMValueRef new_alloca) {
    LLVMScope* scope = current_scope;
    while (scope) {
        ScopeVarEntry* entry = scope->variables;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                entry->alloca = new_alloca;
                return;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
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

    // Declarar funciones de C necesarias
    LLVMTypeRef double_func_type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        
    // Declarar todas las funciones matemáticas
    LLVMAddFunction(module, "sqrt", double_func_type);
    LLVMAddFunction(module, "sin", double_func_type);
    LLVMAddFunction(module, "cos", double_func_type);
    LLVMAddFunction(module, "exp", double_func_type);
    LLVMAddFunction(module, "log", double_func_type);
    
    // Declarar rand
    LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMAddFunction(module, "rand", rand_type);

    // Agregar declaraciones para funciones de string
    LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64Type(),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
    LLVMAddFunction(module, "strlen", strlen_type);

    LLVMTypeRef malloc_type = LLVMFunctionType(
        LLVMPointerType(LLVMInt8Type(), 0),
        (LLVMTypeRef[]){LLVMInt64Type()}, 1, 0);
    LLVMAddFunction(module, "malloc", malloc_type);

    // Agregar declaración de snprintf
    LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8Type(), 0),
            LLVMInt64Type(),
            LLVMPointerType(LLVMInt8Type(), 0)
        }, 3, 1);
    LLVMAddFunction(module, "snprintf", snprintf_type);

    // Agregar declaración de strcmp
    LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8Type(), 0),
            LLVMPointerType(LLVMInt8Type(), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcmp", strcmp_type);

    // Agregar declaración de pow que faltaba
    LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
    LLVMAddFunction(module, "pow", pow_type);

    // Agregar declaración de fmod que faltaba
    LLVMTypeRef fmod_type = LLVMFunctionType(LLVMDoubleType(),
        (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
    LLVMAddFunction(module, "fmod", fmod_type);

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