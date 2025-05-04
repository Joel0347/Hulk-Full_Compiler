#include "type.h"
#include <string.h>

// keywords
char* keywords[] = { 
    "number", "string", "boolean", "void", 
    "object", "true", "false", "PI", "E", "function"
};
char scape_chars[] = { 'n', 't', '\\', '\"' };

// Basic types instances
Type TYPE_OBJECT_INST = { "object", NULL, NULL };
Type TYPE_NUMBER_INST = { "number", NULL, &TYPE_OBJECT_INST };
Type TYPE_STRING_INST = { "string", NULL, &TYPE_OBJECT_INST };
Type TYPE_BOOLEAN_INST = { "boolean", NULL, &TYPE_OBJECT_INST };
Type TYPE_VOID_INST = { "void", NULL, &TYPE_OBJECT_INST };
Type TYPE_ERROR_INST = { "error", NULL, NULL };
Type TYPE_ANY_INST = { "any", NULL, NULL };

OperatorTypeRule operator_rules[] = {

// ----------------------  OPERATIONS ALLOWED  --------------------------------

//      left_type    |      right_type     |   return_type   | operator  

    // math binary
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_ADD },// +
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_SUB },// -
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_MUL },// *
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_DIV },// /
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_MOD },// %
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_POW },// ^
    //string binary
    { &TYPE_NUMBER_INST, &TYPE_STRING_INST, &TYPE_STRING_INST, OP_CONCAT},// @
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_STRING_INST, OP_CONCAT},
    { &TYPE_STRING_INST, &TYPE_NUMBER_INST, &TYPE_STRING_INST, OP_CONCAT},
    { &TYPE_NUMBER_INST, &TYPE_STRING_INST, &TYPE_STRING_INST, OP_DCONCAT},// @@
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_STRING_INST, OP_DCONCAT},
    { &TYPE_STRING_INST, &TYPE_NUMBER_INST, &TYPE_STRING_INST, OP_DCONCAT},
    //boolean binary
    { &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, OP_AND },// &
    { &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, OP_OR },// |
    //comparison
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_BOOLEAN_INST, OP_EQ },// ==
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_BOOLEAN_INST, OP_EQ },
    { &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, OP_EQ },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_BOOLEAN_INST, OP_NEQ },// !=
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_BOOLEAN_INST, OP_NEQ },
    { &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, &TYPE_BOOLEAN_INST, OP_NEQ },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_BOOLEAN_INST, OP_GRE },// >=
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_BOOLEAN_INST, OP_GRE },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_BOOLEAN_INST, OP_GR },// >
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_BOOLEAN_INST, OP_GR },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_BOOLEAN_INST, OP_LSE },// <=
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_BOOLEAN_INST, OP_LSE },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_BOOLEAN_INST, OP_LS },// <
    { &TYPE_STRING_INST, &TYPE_STRING_INST, &TYPE_BOOLEAN_INST, OP_LS },
    //unary
    { &TYPE_BOOLEAN_INST, NULL, &TYPE_BOOLEAN_INST, OP_NOT },// !
    { &TYPE_NUMBER_INST, NULL, &TYPE_NUMBER_INST, OP_NEGATE },// (-)
};


int op_rules_count = sizeof(operator_rules) / sizeof(OperatorTypeRule);
int keyword_count = sizeof(keywords) / sizeof(char*);
int scapes_count = sizeof(scape_chars) / sizeof(char);

int match_as_keyword(char* name) {
    for (int i = 0; i < keyword_count; i++)
    {
        if (!strcmp(keywords[i], name))
            return 1;
    }

    return 0;
}

int is_scape_char(char c) {
    for (int i = 0; i < scapes_count; i++)
    {
        if (scape_chars[i] == c)
            return 1;
    }

    return 0;
}

OperatorTypeRule create_op_rule(Type* left_type, Type* right_type, Type* return_type, Operator op) {
    OperatorTypeRule rule = { 
        left_type, right_type, 
        return_type, op
    };

    return rule;
}

int is_ancestor_type(Type* ancestor, Type* type) {
    if (!type)
        return 0;
 
    if (type_equals(ancestor, type))
        return 1;

    return is_ancestor_type(ancestor, type->parent);
}

int type_equals(Type* type1, Type* type2) {
    if (!type1 && !type2)
        return 1;

    if (!type1 || !type2)
        return 0;

    if (!strcmp(type1->name, type2->name)) {
        return type_equals(type1->sub_type, type2->sub_type);
    }

    return 0;
}

int op_rule_equals(OperatorTypeRule* op1, OperatorTypeRule* op2) {
    return (
            (type_equals(op1->left_type, op2->left_type) &&
             type_equals(op1->right_type, op2->right_type)
            ) ||
            type_equals(op2->left_type, &TYPE_ERROR_INST)||
            type_equals(op2->right_type, &TYPE_ANY_INST) ||
            type_equals(op2->right_type, &TYPE_ERROR_INST)||
            type_equals(op2->left_type, &TYPE_ANY_INST)
           ) &&
            type_equals(op1->result_type, op2->result_type) &&
            op1->op == op2->op;
}

int find_op_match(OperatorTypeRule* possible_match) {
    for (int i = 0; i < op_rules_count; i++)
    {
        if (op_rule_equals(&operator_rules[i], possible_match)) {
            return 1;
        }
    }

    return 0;
}