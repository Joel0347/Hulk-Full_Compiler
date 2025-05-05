#include "semantic.h"

int unify_member(Visitor* v, ASTNode* node, Type* type) {
    // 0 not unfied
    // 1 unified right
    // 2 unified left
    // 3 unified both
    int unified = 0;
    if (type_equals(node->return_type, &TYPE_ANY_INST) &&
        node->type == NODE_VARIABLE  && node->is_param == 1
    ) {
        Symbol* sym = find_symbol(node->scope, node->data.variable_name);
        
        if (type_equals(sym->type, &TYPE_ANY_INST)) {
            sym->type = type;
            node->return_type = type;
            unified = 1;
        } else {
            char* str = NULL;
            asprintf(&str, "Parameter '%s' behaves both as '%s' and '%s'. Line: %d.",
                node->data.variable_name, sym->type->name, type->name, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        }
    } else if (type_equals(node->value->return_type, &TYPE_ANY_INST)) {
        unified = unify_member(v, node->value, type);
    }

    return unified;
}

int unify(Visitor* v, ASTNode* left, ASTNode* right, Operator op, char* op_name) {
    int count = 0;
    int unified = 0;
    OperatorTypeRule* rule;

    if (right && type_equals(left->return_type, &TYPE_ANY_INST) && 
        type_equals(right->return_type, &TYPE_ANY_INST)) {
        
        for (int i = 0; i < op_rules_count; i++)
        {
            if (operator_rules[i].op == op) {
                count++;
                if (count > 1) {
                    return 0;
                }
                rule = &operator_rules[i];
            }
        }

        if (count) {
            int u_left = unify_member(v, left, rule->left_type);
            int u_right = unify_member(v, right, rule->right_type);

            unified = (u_left && u_right) ? 3 : 0;
        }
    } else if (type_equals(left->return_type, &TYPE_ANY_INST)) {
        for (int i = 0; i < op_rules_count; i++)
        {
            if (operator_rules[i].op == op && (!right ||
                type_equals(right->return_type, operator_rules[i].right_type))
            ) {
                count++;
                if (count > 1) {
                    return 0;
                }
                rule = &operator_rules[i];
            }
        }

        if (count) {
            unified = unify_member(v, left, rule->left_type) ? 2 : 0;
        }
    } else if (right && type_equals(right->return_type, &TYPE_ANY_INST)) {
        for (int i = 0; i < op_rules_count; i++)
        {
            if (operator_rules[i].op == op && 
                type_equals(left->return_type, operator_rules[i].left_type)
            ) {
                count++;
                if (count > 1) {
                    return 0;
                }
                rule = &operator_rules[i];
            }
        }

        if (count) {
            unified = unify_member(v, right, rule->right_type) ? 1 : 0;
        }
    }

    return unified;
}

void visit_number(Visitor* v, ASTNode* node) { }

void visit_string(Visitor* v, ASTNode* node) {
    char* string = node->data.string_value;
    int count = strlen(string);

    for (int i = 0; i < count; i++)
    {
        if (string[i] == '\\') {
            if (!is_scape_char(string[i + 1])) {
                char* str = NULL;
                asprintf(&str, "Invalid scape sequence '\\%c'. Line: %d.", 
                    string[i + 1], node->line
                );
                add_error(&(v->errors), &(v->error_count), str);
            }
            i++;
        }
    }
}

void visit_boolean(Visitor* v, ASTNode* node) { }

void visit_binary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;
    ASTNode* right = node->data.op_node.right;

    left->scope->parent = node->scope;
    left->context->parent = node->context;
    right->scope->parent = node->scope;
    right->context->parent = node->context;

    accept(v, left);
    accept(v, right);

    int unified = unify(
        v, left, right, node->data.op_node.op, 
        node->data.op_node.op_name
    );

    if (unified == 3) {
        accept(v, left);
        accept(v, right);
    } else if (unified == 2) {
        accept(v, left);
    } else if (unified == 1) {
        accept(v, right);
    }

    Type* left_type = find_type(v, left);
    Type* right_type = find_type(v, right);

    OperatorTypeRule rule = create_op_rule ( 
        left_type, right_type, 
        node->return_type, 
        node->data.op_node.op 
    );

    if (!find_op_match(&rule)) {
        char* str = NULL;
        asprintf(&str, "Operator '%s' can not be used between '%s' and '%s'. Line: %d.",
            node->data.op_node.op_name, left_type->name, right_type->name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

void visit_unary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;

    left->scope->parent = node->scope;
    left->context->parent = node->context;

    accept(v, left);

    if (unify(v, left, NULL, node->data.op_node.op, node->data.op_node.op_name)) {
        accept(v, left);
    }

    Type* left_type = find_type(v, left);

    OperatorTypeRule rule = create_op_rule( 
        left_type, NULL, 
        node->return_type, 
        node->data.op_node.op 
    );

    if (!find_op_match(&rule)) {
        char* str = NULL;
        asprintf(&str, "Operator '%s' can not be used with '%s'. Line: %d.",
            node->data.op_node.op_name, left_type->name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

void visit_block(Visitor* v, ASTNode* node) {
    get_context(v, node);
    ASTNode* current = NULL;
    
    for(int i = 0; i < node->data.program_node.count; i++) {
        current =  node->data.program_node.statements[i];
        current->scope->parent = node->scope;
        current->context->parent = node->context;
        accept(v, current);
    }

    if (current) {
        node->return_type = find_type(v, current);
        node->value = current;
    } else {
        node->return_type = &TYPE_VOID_INST;
    }
}