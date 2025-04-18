#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>  

typedef enum {
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_VOID,
    TYPE_UNKNOWN,
    TYPE_ERROR  // Para manejar errores de tipo
} TypeKind;

typedef struct Type {
    TypeKind kind;
    char* name;         // Para tipos personalizados
    struct Type* sub_type; // Para arreglos, genéricos, etc.
} Type;

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

typedef struct OperatorTypeRule {
    Type* left_type;    // NULL para operadores unarios
    Type* right_type;
    Type* result_type;
    Operator op;
} OperatorTypeRule;

// Variables globales para tipos básicos (añade estas declaraciones)
extern Type TYPE_NUMBER_INST;
extern Type TYPE_STRING_INST;
extern Type TYPE_BOOLEAN_INST;
extern Type TYPE_VOID_INST;
extern Type TYPE_UNKNOWN_INST;
extern Type TYPE_ERROR_INST;

// Usa & para referenciar las instancias de tipos
extern int rules_count;
extern OperatorTypeRule operator_rules[];

#endif