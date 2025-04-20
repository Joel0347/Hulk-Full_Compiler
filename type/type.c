#include "type.h"
#include <string.h>

// Definir las instancias de tipos básicos
Type TYPE_NUMBER_INST = { TYPE_NUMBER, "number", NULL };
Type TYPE_STRING_INST = { TYPE_STRING, "string", NULL };
Type TYPE_BOOLEAN_INST = { TYPE_BOOLEAN, "boolean", NULL };
Type TYPE_VOID_INST = { TYPE_VOID, "void", NULL };
Type TYPE_UNKNOWN_INST = { TYPE_UNKNOWN, "unknown", NULL };
Type TYPE_ERROR_INST = { TYPE_ERROR, "error", NULL };

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

FuncTypeRule func_rules[] = {
// ----------------------  FUNCTIONS ALLOWED  --------------------------------
 
//      args_count  |    [args_types]    |   return_type   | func_name

{ 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "sqrt" }, //sqrt
{ 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "sin" }, //sin
{ 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "cos" }, //cos
{ 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "exp" }, //exp
{ 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "log" }, //log
{ 2, (Type*[]){ &TYPE_NUMBER_INST, &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "log" }, //log
{ 0, NULL, &TYPE_NUMBER_INST, "rand" } //rand

};

int op_rules_count = sizeof(operator_rules) / sizeof(OperatorTypeRule);
int func_rules_count = sizeof(func_rules) / sizeof(FuncTypeRule);

OperatorTypeRule create_op_rule(Type* left_type, Type* right_type, Type* return_type, Operator op) {
    OperatorTypeRule rule = { 
        left_type, right_type, 
        return_type, op
    };

    return rule;
}

FuncTypeRule create_func_rule(int arg_count, Type** args_types, Type* result_type, char* name) {
    FuncTypeRule rule = {
        arg_count, args_types,
        result_type, name
    };

    return rule;
}


int type_equals(Type* type1, Type* type2) {
    if (type1 == NULL && type2 == NULL)
        return 1;

    if (type1 == NULL || type2 == NULL)
        return 0;
    
    if (type1->kind == type2->kind && !strcmp(type1->name, type2->name)) {
        return type_equals(type1->sub_type, type2->sub_type);
    }

    return 0;
}

int op_rule_equals(OperatorTypeRule* op1, OperatorTypeRule* op2) {
    return type_equals(op1->left_type, op2->left_type) &&
        type_equals(op1->right_type, op2->right_type) &&
        type_equals(op1->result_type, op2->result_type) &&
        op1->op == op2->op;
}

int find_op_match(OperatorTypeRule* possible_match) {
    for (int i = 0; i < op_rules_count; i++)
    {
        if (op_rule_equals(&operator_rules[i], possible_match))
            return 1;
    }

    return 0;
}

Tuple* args_type_equals(Type** args1, Type** args2, int count) {
    for (int i = 0; i < count; i++)
    {
        if (!type_equals(args1[i], args2[i])) {
            Tuple* tuple = init_tuple_for_types(0, args1[i]->name, args2[i]->name, i+1);
            return tuple;
        }
    }
    
    return init_tuple_for_types(1, "", "", -1);
}

Tuple* func_rule_equals(FuncTypeRule* f1, FuncTypeRule* f2) {
    if (strcmp(f1->name, f2->name)) {
        Tuple* tuple = init_tuple_for_count(0, -1, -1);
        tuple->same_name = 0;
        return tuple;
    }

    if (f1->arg_count != f2->arg_count)
        return init_tuple_for_count(0, f1->arg_count, f2->arg_count);

    return args_type_equals(f1->args_types, f2->args_types, f1->arg_count);
}

Tuple* find_func_match(FuncTypeRule* possible_match) {
    Tuple* _tuple = NULL;
    for (int i = 0; i < func_rules_count; i++)
    {
        Tuple* tuple = func_rule_equals(&func_rules[i], possible_match);
        if (tuple->matched)
            return tuple;
        if (tuple->same_name)
            _tuple = tuple;
    }
    
    return _tuple;
}