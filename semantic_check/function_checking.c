#include "semantic.h"

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
    
    return NULL;
}

void visit_function_call(Visitor* v, ASTNode* node) {
    ASTNode** args = node->data.func_node.args;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        accept(v, args[i]);
    }

    IntList* unified = unify_func(
        v, args, node->scope, node->data.func_node.arg_count, 
        node->data.func_node.name
    );

    for (IntList* current = unified; current != NULL; current = current->next)
    {
        accept(v, args[current->value]);
    }

    free_int_list(unified);
    

    Type** args_types = find_types(args, node->data.func_node.arg_count);

    Function* f = (Function*)malloc(sizeof(Function));
    f->name = node->data.func_node.name;
    f->arg_count = node->data.func_node.arg_count;
    f->args_types = args_types;

    FuncData* funcData = find_function(node->scope, f);

    if (funcData->func)
        node->return_type = funcData->func->result_type;

    if (!funcData->state->matched) {
        if (!funcData->state->same_name) {
            node->return_type = &TYPE_ERROR_INST;
            char* str = NULL;
            asprintf(&str, "Undefined function '%s'. Line: %d.",
                node->data.func_node.name, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        } else if (!funcData->state->same_count) {
            char* str = NULL;
            asprintf(&str, "Function '%s' receives %d argument(s), but %d was(were) given. Line: %d.",
                node->data.func_node.name, funcData->state->arg1_count, 
                funcData->state->arg2_count, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        } else {
            if (!strcmp(funcData->state->type2_name, "error"))
                return;

            char* str = NULL;
            asprintf(&str, "Function '%s' receives '%s', not '%s' as argument %d. Line: %d.",
                node->data.func_node.name, funcData->state->type1_name, 
                funcData->state->type2_name, funcData->state->pos, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        }
    }

    free_tuple(funcData->state);
    free(args_types);
    free(f);
}

void visit_function_dec(Visitor* v, ASTNode* node) {
    ASTNode** params = node->data.func_node.args;
    ASTNode* body = node->data.func_node.body;
    body->scope->parent = node->scope;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        params[i]->scope->parent = node->scope;
        Symbol* param_type = find_defined_type(node->scope, params[i]->static_type);

        if (strcmp(params[i]->static_type, "") && !param_type) {
            char* str = NULL;
            asprintf(&str, "Parameter '%s' was defined as '%s', which is not a valid type. Line: %d.", 
                params[i]->data.variable_name, params[i]->static_type, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        }

        if (!strcmp(params[i]->static_type, "")) {
            params[i]->return_type = &TYPE_ANY_INST;
        } else if (param_type) {
            params[i]->return_type = param_type->type;
        }

        declare_symbol(
            node->scope, params[i]->data.variable_name,
            params[i]->return_type, 1, NULL
        );
    }

    accept(v, body);
    Type* inferried_type = find_type(v, body);
    Symbol* defined_type = find_defined_type(node->scope, node->static_type);

    if (strcmp(node->static_type, "") && !defined_type) {
        char* str = NULL;
        asprintf(&str, "The return type of function '%s' was defined as '%s', which is not a valid type. Line: %d.", 
            node->data.func_node.name, node->static_type, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }

    if (defined_type && !is_ancestor_type(defined_type->type, inferried_type)) {
        char* str = NULL;
        asprintf(&str, "The return type of function '%s' was defined as '%s', but inferred as '%s'. Line: %d.", 
            node->data.func_node.name, node->static_type, 
            inferried_type->name, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }
    
    if (defined_type)
        inferried_type = defined_type->type;

    if (type_equals(inferried_type, &TYPE_ANY_INST)) {
        char* str = NULL;
        asprintf(&str, "Impossible to infer return type of function '%s'. It must be type annotated. Line: %d.", 
            node->data.func_node.name, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }

    Function* func = find_function_by_name(node->scope, node->data.func_node.name);
    Type** param_types = find_types(params, node->data.func_node.arg_count);
    
    if (!func) {

        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            Symbol* param = find_symbol(node->scope, params[i]->data.variable_name);

            if (type_equals(param->type, &TYPE_ANY_INST)) {
                char* str = NULL;
                asprintf(&str, "Impossible to infer type of parameter '%s' in function '%s'. It must be type annotated. Line: %d.", 
                    param->name, node->data.func_node.name, node->line
                );
                add_error(&(v->errors), &(v->error_count), str);
            }

            param_types[i] = param->type;
        }

        declare_function(
            node->scope->parent, node->data.func_node.arg_count,
            param_types, inferried_type, node->data.func_node.name
        );
    } else {
        node->return_type = &TYPE_ERROR_INST;
        char* str = NULL;
        asprintf(&str, "Function '%s' already exists. Line: %d.", 
            node->data.func_node.name, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }
}