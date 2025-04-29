#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>  
#include "../utils/utils.h"

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
    char* name;
    struct Type* sub_type;
    struct Type* parent;
} Type;

typedef struct OperatorTypeRule {
    Type* left_type;
    Type* right_type; // NULL for unary operators
    Type* result_type;
    Operator op;
} OperatorTypeRule;

extern char* keywords[]; // keywords of the language
extern char scape_chars[]; //scapes characters defined

extern Type TYPE_NUMBER_INST;
extern Type TYPE_STRING_INST;
extern Type TYPE_BOOLEAN_INST;
extern Type TYPE_VOID_INST;
extern Type TYPE_OBJECT_INST;
extern Type TYPE_UNKNOWN_INST;

extern int op_rules_count;
extern OperatorTypeRule operator_rules[];

OperatorTypeRule create_op_rule(Type* left_type, Type* right_type, Type* return_type, Operator op);
int match_as_keyword(char* name);
int is_scape_char(char c);
int type_equals(Type* type1, Type* type2);
int is_ancestor_type(Type* ancestor, Type* type);
int find_op_match(OperatorTypeRule* possible_match);

#endif