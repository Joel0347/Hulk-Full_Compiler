#include "semantic.h"


int unify_member(Visitor* v, ASTNode* node, Type* type) {
    int unified = 0;
    if (node->type == NODE_VARIABLE  && node->is_param == 1) {
        Symbol* sym = find_parameter(node->scope, node->data.variable_name);
        
        if (type_equals(sym->type, &TYPE_ANY_INST)) {
            sym->type = type;
            node->return_type = type;
        } else if (!type_equals(sym->type, type)) {
            report_error(
                v, "Parameter '%s' behaves both as '%s' and '%s'. Line: %d.",
                node->data.variable_name, sym->type->name, type->name, node->line
            );
            return 0;
        }

        unified = 1;
    } else if (node->type == NODE_FUNC_CALL &&
        type_equals(node->return_type, &TYPE_ANY_INST)
    ) {
        ContextItem* item = find_context_item(node->context, node->data.func_node.name);
        if (item) {
            item->return_type = type;
            node->return_type = type;
            unified = 1;
        }
    }
    
    else if (node->value && type_equals(node->value->return_type, &TYPE_ANY_INST)) {
        unified = unify_member(v, node->value, type);
        if (unified) {
            node->return_type = type;
        }
    }

    return unified;
}

int unify(Visitor* v, ASTNode* left, ASTNode* right, Operator op, char* op_name) {
    int count = 0;
    int unified = 0; // 0: not unfied, 1: unified right, 2: unified left, 3: unified both
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

IntList* unify_func(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* f_name) {
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
            if (type_equals(&TYPE_ANY_INST, args[i]->return_type)) {
                if (unify_member(v, args[i], f->args_types[i])) {
                    unified = add_int_list(unified, i);
                } else {
                    return NULL;
                }
            }
        }
    }
    
    return NULL;
}