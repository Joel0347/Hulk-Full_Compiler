#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>  
#include "../utils/utils.h"

typedef enum {
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_VOID,
    TYPE_UNKNOWN,
    TYPE_ERROR  // Para manejar errores de tipo
} TypeKind;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,
    OP_NEGATE,
    OP_CONCAT,
    OP_DCONCAT,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_EQ,
    OP_NEQ,
    OP_GRE,
    OP_GR,
    OP_LSE,
    OP_LS
} Operator;

typedef struct Type {
    TypeKind kind;
    char* name;         // Para tipos personalizados
    struct Type* sub_type; // Para arreglos, genéricos, etc.
} Type;

typedef struct OperatorTypeRule {
    Type* left_type;    // NULL para operadores unarios
    Type* right_type;
    Type* result_type;
    Operator op;
} OperatorTypeRule;

typedef struct FuncTypeRule {
    int arg_count;
    Type** args_types;
    Type* result_type;
    char* name;
} FuncTypeRule;

// Variables globales para tipos básicos (añade estas declaraciones)
extern Type TYPE_NUMBER_INST;
extern Type TYPE_STRING_INST;
extern Type TYPE_BOOLEAN_INST;
extern Type TYPE_VOID_INST;
extern Type TYPE_UNKNOWN_INST;
extern Type TYPE_ERROR_INST;

// Usa & para referenciar las instancias de tipos
extern int op_rules_count;
extern int func_rules_count;
extern OperatorTypeRule operator_rules[];
extern FuncTypeRule func_rules[];

OperatorTypeRule create_op_rule(Type* left_type, Type* right_type, Type* return_type, Operator op);
FuncTypeRule create_func_rule(int arg_count, Type** args_types, Type* result_type, char* name);
int type_equals(Type* type1, Type* type2);
int op_rule_equals(OperatorTypeRule* op1, OperatorTypeRule* op2);
int find_op_match(OperatorTypeRule* possible_match);
Tuple* args_type_equals(Type** args1, Type** args2, int count);
Tuple* func_rule_equals(FuncTypeRule* f1, FuncTypeRule* f2);
Tuple* find_func_match(FuncTypeRule* possible_match);

#endif