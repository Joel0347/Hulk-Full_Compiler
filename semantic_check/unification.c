#include "semantic.h"


int unify_member(Visitor* v, ASTNode* node, Type* type) {
    int unified = 0;
    
    if (node->type == NODE_VARIABLE && node->is_param == 1) {
        Symbol* sym = find_parameter(node->scope, node->data.variable_name);
    
        if (type_equals(sym->type, &TYPE_ANY) ||
            is_ancestor_type(sym->type, type)
        ) {
            sym->type = type;
            node->return_type = type;

            for (int i = 0; i < sym->derivations->count; i++)
            {
                ASTNode* value = at(i, sym->derivations);
                if (value && type_equals(value->return_type, &TYPE_ANY)) {
                    unify_member(v, value, type);
                }
            }
        } else if (!is_ancestor_type(type, sym->type)) {
            report_error(
                v, "Parameter '%s' behaves both as '%s' and '%s'. Line: %d.",
                node->data.variable_name, sym->type->name, type->name, node->line
            );
            return 0;
        }

        unified = 1;
    } else if (node->type == NODE_FUNC_CALL &&
        type_equals(node->return_type, &TYPE_ANY)
    ) {
        ContextItem* item = find_context_item(node->context, node->data.func_node.name);
        if (item) {
            item->return_type = type;
            node->return_type = type;
            unified = 1;
        }
    } 
    // else if (
    //     node->type == NODE_CONDITIONAL &&
    //     (type_equals(
    //         node->data.cond_node.body_true->return_type, &TYPE_ANY
    //     ) ||
    //     (node->data.cond_node.body_false &&
    //     type_equals(
    //         node->data.cond_node.body_false->return_type, &TYPE_ANY
    //     )))
    // ) {
    //     int unified_true = unify_member(v, node->data.cond_node.body_true, type);
    //     int unified_false = 0;

    //     if (node->data.cond_node.body_false)
    //         unified_false = unify_member(v, node->data.cond_node.body_false, type);

    //     unified = unified_true || unified_false;
        
    //     if (unified) {
    //         node->return_type = type;
    //     }
    // } 
    else if (node->derivations) {
        for (int i = 0; i < node->derivations->count; i++)
        {
            ASTNode* value = at(i, node->derivations);
            if (value && type_equals(value->return_type, &TYPE_ANY)) {
                unified |= unify_member(v, value, type);
            }
        }

        if (unified) {
            node->return_type = type;
        }
    }

    return unified;
}

int unify_op(Visitor* v, ASTNode* left, ASTNode* right, Operator op, char* op_name) {
    int count = 0;
    int unified = 0; // 0: not unfied, 1: unified right, 2: unified left, 3: unified both
    OperatorTypeRule* rule;

    if (right && type_equals(left->return_type, &TYPE_ANY) && 
        type_equals(right->return_type, &TYPE_ANY)) {
        
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
    } else if (type_equals(left->return_type, &TYPE_ANY)) {
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
        } else if (!type_equals(right->return_type, &TYPE_ERROR)) {
            report_error(
                v, "Operator '%s' can not be used with '%s' as right side. Line: %d.",
                op_name, right->return_type->name, right->line
            );
        }
    } else if (right && type_equals(right->return_type, &TYPE_ANY)) {
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
        } else if (!type_equals(left->return_type, &TYPE_ERROR)) {
            report_error(
                v, "Operator '%s' can not be used with '%s' as left side. Line: %d.",
                op_name, left->return_type->name, left->line
            );
        }
    }

    return unified;
}

IntList* unify_func(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* f_name, ContextItem* item) {
    int count = 0;
    IntList* unified = NULL;
    Function* f;

    while (scope)
    {
        if (scope->functions) {
            Function* current = scope->functions->first; 
            while (current)
            {
                if (!strcmp(f_name, current->name) &&
                    arg_count == current->arg_count
                ) {
                    count ++;
                    if (count > 1) {
                        return NULL;
                    }

                    f = current;
                }

                current = current->next;
            }
        }

        scope = scope->parent;
    }

    if (count) {
        for (int i = 0; i < arg_count; i++)
        {
            if (type_equals(&TYPE_ANY, args[i]->return_type)) {
                if (unify_member(v, args[i], f->args_types[i])) {
                    unified = add_int_list(unified, i);
                } else {
                    return NULL;
                }
            }
        }
    } else if (item && item->declaration->data.func_node.arg_count == arg_count) {
        for (int i = 0; i < item->declaration->data.func_node.arg_count; i++)
        {
            if (unify_member(
                v, item->declaration->data.func_node.args[i],
                args[i]->return_type
            )) {
                unified = add_int_list(unified, i);
            } else {
                return NULL;
            }
        }
    }
    
    return NULL;
}