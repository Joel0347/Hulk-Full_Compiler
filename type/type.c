#include "type.h"
#include "ast/ast.h"


//<----------RULES---------->

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

// method to create an operation rule
OperatorTypeRule create_op_rule(Type* left_type, Type* right_type, Type* return_type, Operator op) {
    OperatorTypeRule rule = { 
        left_type, right_type, 
        return_type, op
    };

    return rule;
}

// method to check whether or not two operation rules are equal
int op_rule_equals(OperatorTypeRule* op1, OperatorTypeRule* op2) {
    return (
        ((type_equals(op1->left_type, op2->left_type) &&
        type_equals(op1->right_type, op2->right_type)) ||
        type_equals(op2->left_type, &TYPE_ERROR)||
        type_equals(op2->right_type, &TYPE_ANY) ||
        type_equals(op2->right_type, &TYPE_ERROR)||
        type_equals(op2->left_type, &TYPE_ANY)
        ) &&
        type_equals(op1->result_type, op2->result_type) &&
        op1->op == op2->op
    );
}

// method to check whether or not there is a rule that matches with the given one
int find_op_match(OperatorTypeRule* possible_match) {
    for (int i = 0; i < op_rules_count; i++)
    {
        if (op_rule_equals(&operator_rules[i], possible_match)) {
            return 1;
        }
    }

    return 0;
}


//<----------TYPES---------->

// Basic types instances
Type TYPE_OBJECT = { "Object", NULL, NULL, NULL, NULL, 0 };
Type TYPE_NUMBER = { "Number", NULL, &TYPE_OBJECT, NULL, NULL, 0 };
Type TYPE_STRING = { "String", NULL, &TYPE_OBJECT, NULL, NULL, 0 };
Type TYPE_BOOLEAN = { "Boolean", NULL, &TYPE_OBJECT, NULL, NULL, 0 };
Type TYPE_VOID = { "Void", NULL, &TYPE_OBJECT, NULL, NULL, 0 };
Type TYPE_ERROR = { "Error", NULL, NULL, NULL, NULL, 0 };
Type TYPE_ANY = { "Any", NULL, NULL, NULL, NULL, 0 };
Type TYPE_NULL = { "Null", NULL, &TYPE_OBJECT, NULL, NULL, 0 };

// method to get the return type of a node
Type* get_type(ASTNode* node) {
    Type* instance_type = node->return_type;
    Symbol* t = find_defined_type(node->scope, instance_type->name);

    if (t)
        return t->type;

    return instance_type;
}

// method to map node array to type array using the type of each node
Type** map_get_type(ASTNode** nodes, int count) {
    Type** types = (Type**)malloc(count * sizeof(Type*));
    for (int i = 0; i < count; i++)
    {
        types[i] = get_type(nodes[i]);
    }
    
    return types;
}

// method to check whether or not 'ancestor' is ancestor of 'type' in type hierarchy
int is_ancestor_type(Type* ancestor, Type* type) {
    if (!type)
        return 0;
 
    if (type_equals(ancestor, type))
        return 1;

    return is_ancestor_type(ancestor, type->parent);
}

// method to get the nulleable type associated to the given type
Type* get_nulleable(Type* type) {
    if (is_builtin_type(type) && !type_equals(type, &TYPE_OBJECT))
        return type;

    Type* new_type = (Type*)malloc(sizeof(Type));
    new_type->name = append_question(type->name);
    new_type->parent = &TYPE_OBJECT;
    new_type->param_types = NULL;
    new_type->arg_count = 0;
    new_type->dec = NULL;
    new_type->sub_type = type;
    return new_type;
}

// method to get the lowest common ancestor of two types
Type* get_lca(Type* t1, Type* t2) {
    int t1_nulleable = 0;
    int f2_nulleable = 0;

    if (t1->sub_type) {
        t1_nulleable = 1;
        t1 = t1->sub_type;
    }

    if (t2->sub_type) {
        f2_nulleable = 1;
        t2 = t2->sub_type;
    }

    if (type_equals(t1, &TYPE_ANY) || type_equals(t2, &TYPE_ANY)) {
        return &TYPE_ANY;
    } else if (type_equals(t1, &TYPE_ERROR) || type_equals(t2, &TYPE_ERROR)) {
        return &TYPE_ERROR;
    }

    if (type_equals(t2, &TYPE_NULL)) {
        return get_nulleable(t1);
    }
    
    if (is_ancestor_type(t1, t2)) {
        if (t1_nulleable)
            t1 = get_nulleable(t1);
        return t1;
    } else if (is_ancestor_type(t2, t1)) {
        if (f2_nulleable)
            t2 = get_nulleable(t2);
        return t2;
    }

    Type* result = get_lca(t1->parent, t2->parent);

    return (t1_nulleable || f2_nulleable)? get_nulleable(result) : result;
}

// method to check whethter or not two types are in the same branch of the type hierarchy
int same_branch_in_type_hierarchy(Type* type1, Type* type2) {
    
    if (is_ancestor_type(type1, type2) ||
        is_ancestor_type(type2, type1)
    ) {
        return 1;
    }

    return 0;
}

// method to check whether or not two types are equal
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

// method to check whether or not each type of 'model' is ancestor of 
//the corresponding type in 'candidate'
Tuple* map_type_equals(Type** model, Type** candidate, int count) {
    for (int i = 0; i < count; i++)
    {
        if ((!type_equals(candidate[i], &TYPE_ERROR) &&
            !type_equals(candidate[i], &TYPE_ANY)
            ) &&
            (!type_equals(model[i], &TYPE_ANY) &&
            !type_equals(model[i], &TYPE_ERROR) &&
            !is_ancestor_type(model[i], candidate[i])
            )
        ) {
            Tuple* tuple = init_tuple_for_types(
                0, model[i]->name, candidate[i]->name, i+1
            );
            return tuple;
        }
    }
    
    return init_tuple_for_types(1, "", "", -1);
}

// method to create a new type
Type* create_new_type(char* name, Type* parent, Type** param_types, int count, struct ASTNode* dec) {
    Type* new_type = (Type*)malloc(sizeof(Type));
    new_type->name = name;
    new_type->parent = parent;
    new_type->param_types = param_types;
    new_type->arg_count = count;
    new_type->dec = dec;
    new_type->sub_type = NULL;
    return new_type;
}

// method to check whether or not a type is builtin
int is_builtin_type(Type* type) {
    return (
        type_equals(type, &TYPE_OBJECT)  ||
        type_equals(type, &TYPE_STRING)  ||
        type_equals(type, &TYPE_NUMBER)  ||
        type_equals(type, &TYPE_BOOLEAN) ||
        type_equals(type, &TYPE_VOID)    ||
        type_equals(type, &TYPE_ERROR)   ||
        type_equals(type, &TYPE_NULL)
    );
}