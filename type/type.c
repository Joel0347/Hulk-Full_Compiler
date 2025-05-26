#include "type.h"

// keywords
char* keywords[] = { 
    "Number", "String", "Boolean", "Object", "Void",
    "true", "false", "PI", "E", "function", "let", "in",
    "is", "as", "type", "inherits", "new"
};
char scape_chars[] = { 'n', 't', '\\', '\"' };

// Basic types instances
Type TYPE_OBJECT = { "Object", NULL, NULL, NULL, NULL, NULL, 0 };
Type TYPE_NUMBER = { "Number", NULL, &TYPE_OBJECT, NULL, NULL, NULL, 0 };
Type TYPE_STRING = { "String", NULL, &TYPE_OBJECT, NULL, NULL, NULL, 0 };
Type TYPE_BOOLEAN = { "Boolean", NULL, &TYPE_OBJECT, NULL, NULL, NULL, 0 };
Type TYPE_VOID = { "Void", NULL, &TYPE_OBJECT, NULL, NULL, NULL, 0 };
Type TYPE_ERROR = { "Error", NULL, NULL, NULL, NULL, NULL, 0 };
Type TYPE_ANY = { "Any", NULL, NULL, NULL, NULL, NULL, 0 };

OperatorTypeRule operator_rules[] = {

// ----------------------  OPERATIONS ALLOWED  --------------------------------

//      left_type    |      right_type     |   return_type   | operator  

    // math binary
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_NUMBER, OP_ADD },// +
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_NUMBER, OP_SUB },// -
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_NUMBER, OP_MUL },// *
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_NUMBER, OP_DIV },// /
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_NUMBER, OP_MOD },// %
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_NUMBER, OP_POW },// ^
    //string binary
    { &TYPE_NUMBER, &TYPE_STRING, &TYPE_STRING, OP_CONCAT},// @
    { &TYPE_STRING, &TYPE_STRING, &TYPE_STRING, OP_CONCAT},
    { &TYPE_STRING, &TYPE_NUMBER, &TYPE_STRING, OP_CONCAT},
    { &TYPE_NUMBER, &TYPE_STRING, &TYPE_STRING, OP_DCONCAT},// @@
    { &TYPE_STRING, &TYPE_STRING, &TYPE_STRING, OP_DCONCAT},
    { &TYPE_STRING, &TYPE_NUMBER, &TYPE_STRING, OP_DCONCAT},
    //boolean binary
    { &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN, OP_AND },// &
    { &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN, OP_OR },// |
    //comparison
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_BOOLEAN, OP_EQ },// ==
    { &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN, OP_EQ },
    { &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN, OP_EQ },
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_BOOLEAN, OP_NEQ },// !=
    { &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN, OP_NEQ },
    { &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN, OP_NEQ },
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_BOOLEAN, OP_GRE },// >=
    { &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN, OP_GRE },
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_BOOLEAN, OP_GR },// >
    { &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN, OP_GR },
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_BOOLEAN, OP_LSE },// <=
    { &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN, OP_LSE },
    { &TYPE_NUMBER, &TYPE_NUMBER, &TYPE_BOOLEAN, OP_LS },// <
    { &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN, OP_LS },
    //unary
    { &TYPE_BOOLEAN, NULL, &TYPE_BOOLEAN, OP_NOT },// !
    { &TYPE_NUMBER, NULL, &TYPE_NUMBER, OP_NEGATE },// (-)
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

Type* get_lca(Type* true_type, Type* false_type) {
    if (type_equals(true_type, &TYPE_ANY) ||
        type_equals(false_type, &TYPE_ANY)
    ) {
        return &TYPE_ANY;
    } else if (type_equals(true_type, &TYPE_ERROR) ||
        type_equals(false_type, &TYPE_ERROR)
    ) {
        return &TYPE_ERROR;
    }
    
    if (is_ancestor_type(true_type, false_type))
        return true_type;
    if (is_ancestor_type(false_type, true_type))
        return false_type;

    return get_lca(true_type->parent, false_type->parent);
}

int same_branch_in_type_hierarchy(Type* type1, Type* type2) {
    
    if (is_ancestor_type(type1, type2) ||
        is_ancestor_type(type2, type1)
    ) {
        return 1;
    }

    return 0;
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
            type_equals(op2->left_type, &TYPE_ERROR)||
            type_equals(op2->right_type, &TYPE_ANY) ||
            type_equals(op2->right_type, &TYPE_ERROR)||
            type_equals(op2->left_type, &TYPE_ANY)
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

Type* create_new_type(char* name, Type* parent, Type** param_types, int count) {
    Type* new_type = (Type*)malloc(sizeof(Type));
    new_type->name = name;
    new_type->parent = parent;
    new_type->param_types = param_types;
    new_type->arg_count = count;
    return new_type;
}

int is_builtin_type(Type* type) {
    return (
        type_equals(type, &TYPE_OBJECT)  ||
        type_equals(type, &TYPE_STRING)  ||
        type_equals(type, &TYPE_NUMBER)  ||
        type_equals(type, &TYPE_BOOLEAN) ||
        type_equals(type, &TYPE_VOID)
    );
}