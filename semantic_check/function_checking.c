#include "semantic.h"


void check_function_call(Visitor* v, ASTNode* node, Type* type) {
    ASTNode** args = node->data.func_node.args;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        args[i]->context->parent = node->context;
        accept(v, args[i]);
    }

    ContextItem* item = type?
        find_item_in_type( // para funciones de tipos especificos
            type->context,
            node->data.func_node.name,
            type, 1
        ) :
        find_context_item(
            node->context, node->data.func_node.name, 0, 0
        );

    if (item) {
        if (type) {
            check_function_dec(v, item->declaration, type);
        } else {
            accept(v, item->declaration);
        }
    }
    Scope* scope = type? type->scope : node->scope;
    IntList* unified = unify_func(
        v, args, scope, node->data.func_node.arg_count, 
        node->data.func_node.name, item
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

    Function* dec = NULL;

    if (item) {
        Type** dec_args_types = find_types(
            item->declaration->data.func_node.args, 
            item->declaration->data.func_node.arg_count
        );

        dec = (Function*)malloc(sizeof(Function));
        dec->name = item->declaration->data.func_node.name;
        dec->arg_count = item->declaration->data.func_node.arg_count;
        dec->args_types = dec_args_types;
        dec->result_type = item->return_type ? item->return_type : &TYPE_ANY;
    }

    FuncData* funcData = type? // verficar funciones de tipo especifico
        get_type_func(type, f, dec) : 
        find_function(node->scope, f, dec);

    if (funcData->func)
        node->return_type = funcData->func->result_type;

    if (!funcData->state->matched) {
        if (!funcData->state->same_name) {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "Undefined function '%s'. Line: %d.",
                node->data.func_node.name, node->line
            );

        } else if (!funcData->state->same_count) {
            report_error(
                v, "Function '%s' receives %d argument(s), but %d was(were) given. Line: %d.",
                node->data.func_node.name, funcData->state->arg1_count, 
                funcData->state->arg2_count, node->line
            );
        } else {
            if (!strcmp(funcData->state->type2_name, "Error"))
                return;

            report_error(
                v, "Function '%s' receives '%s', not '%s' as argument %d. Line: %d.",
                node->data.func_node.name, funcData->state->type1_name, 
                funcData->state->type2_name, funcData->state->pos, node->line
            );
        }
    }

    free_tuple(funcData->state);
    free(args_types);
    free(f);
}

void visit_function_call(Visitor* v, ASTNode* node) {
    check_function_call(v, node, NULL);
}

void visit_function_dec(Visitor* v, ASTNode* node) {
    check_function_dec(v, node, NULL);
}

void check_function_dec(Visitor* v, ASTNode* node, Type* type) {
    if (node->checked) {
        return;
    }

    node->checked = 1;

    ASTNode** params = node->data.func_node.args;
    ASTNode* body = node->data.func_node.body;
    body->scope->parent = node->scope;
    body->context->parent = node->context;

    for (int i = 0; i < node->data.func_node.arg_count - 1; i++) {
        for (int j = i + 1; j < node->data.func_node.arg_count; j++) {
            if (!strcmp(
                params[i]->data.variable_name,
                params[j]->data.variable_name
            )) {
                report_error(
                    v, "Symbol '%s' is used for both argument '%d' and argument '%d' in "
                    "function '%s' declaration. Line: %d.", params[i]->data.variable_name,
                    i + 1, j + 1, node->data.func_node.name, node->line
                );
                return;
            }
        }
    }
    

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        params[i]->scope->parent = node->scope;
        params[i]->context->parent = node->context;
        Symbol* param_type = find_defined_type(node->scope, params[i]->static_type);
        int free_type = 0;

        if (strcmp(params[i]->static_type, "") && !param_type) {
            ContextItem* item = find_context_item(
                node->context, params[i]->static_type, 1, 0
            );
    
            if (item) {
                accept(v, item->declaration);
                param_type = find_defined_type(node->scope, params[i]->static_type);
    
                if (!param_type) {
                    param_type = (Symbol*)malloc(sizeof(Symbol));
                    param_type->name = item->return_type->name;
                    param_type->type = item->return_type;
                    free_type = 1;
                }
            } else {
                report_error(
                    v, "Parameter '%s' was defined as '%s', which is not a valid type. Line: %d.", 
                    params[i]->data.variable_name, params[i]->static_type, node->line
                );
                params[i]->return_type = &TYPE_ERROR;
            }
        }

        if (!strcmp(params[i]->static_type, "")) {
            params[i]->return_type = &TYPE_ANY;
        } else if (param_type) {
            params[i]->return_type = param_type->type;
        }

        declare_symbol(
            node->scope, params[i]->data.variable_name,
            params[i]->return_type, 1, NULL
        );

        if (free_type)
            free(param_type);
    }

    accept(v, body);
    if (!strcmp(node->data.func_node.name, "_PolarPoint_g")) printf("body type: %s\n", body->return_type->name);
    ContextItem* item = type?
        find_item_in_type(
            type->context, 
            node->data.func_node.name, 
            type, 1) :
        find_context_item(node->context, node->data.func_node.name, 0, 0);
    Type* inferried_type = find_type(body);
    Symbol* defined_type = find_defined_type(node->scope, node->static_type);

    if (type_equals(inferried_type, &TYPE_ANY) && 
        defined_type && unify_member(v, body, defined_type->type)
    ) {
        accept(v, body);
        inferried_type = find_type(body);
    }

    if (!defined_type && item->return_type &&
        !is_ancestor_type(item->return_type, inferried_type) &&
        !type_equals(&TYPE_ERROR, item->return_type) &&
        !type_equals(&TYPE_ANY, item->return_type) &&
        !type_equals(&TYPE_ERROR, inferried_type) &&
        !type_equals(&TYPE_ANY, inferried_type)
    ) {
        report_error(
            v,  "Impossible to infer return type of function '%s'."
            " It behaves both as '%s' and '%s'. Line: %d."
            , node->data.func_node.name, inferried_type->name,
            item->return_type->name, node->line
        );
    }

    int free_type = 0;

    if (strcmp(node->static_type, "") && !defined_type) {
        ContextItem* item = find_context_item(
            node->context, node->static_type, 1, 0
        );

        if (item) {
            accept(v, item->declaration);
            defined_type = find_defined_type(node->scope, node->static_type);

            if (!defined_type) {
                defined_type = (Symbol*)malloc(sizeof(Symbol));
                defined_type->name = item->return_type->name;
                defined_type->type = item->return_type;
                free_type = 1;
            }
        } else {
            report_error(
                v,  "The return type of function '%s' was defined as '%s'"
                ", which is not a valid type. Line: %d.", node->data.func_node.name,
                node->static_type, node->line
            );
        }
    }

    if (defined_type && !is_ancestor_type(defined_type->type, inferried_type) &&
        !type_equals(inferried_type, &TYPE_ERROR)
    ) {
        report_error(
            v, "The return type of function '%s' was defined as '%s', but inferred "
            "as '%s'. Line: %d.", node->data.func_node.name, node->static_type,
            inferried_type->name, node->line
        );
    }
    
    if (defined_type)
        inferried_type = defined_type->type;

    if (type_equals(inferried_type, &TYPE_ANY)) {
        accept(v, body);
        if (type_equals(inferried_type, &TYPE_ANY)) {
            report_error(
                v, "Impossible to infer return type of function '%s'. It must be "
                "type annotated. Line: %d.", node->data.func_node.name, node->line
            );
        }
    }

    Function* func = find_function_by_name(node->scope, node->data.func_node.name);
    Type** param_types = find_types(params, node->data.func_node.arg_count);
    
    if (!func) {
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            Symbol* param = find_parameter(node->scope, params[i]->data.variable_name);

            if (type_equals(param->type, &TYPE_ANY)) {
                accept(v, body);
                if (type_equals(param->type, &TYPE_ANY)) {
                    report_error(
                        v, "Impossible to infer type of parameter '%s' in function '%s'."
                        " It must be type annotated. Line: %d.", param->name,
                        node->data.func_node.name, node->line
                    );
                }
            }

            Symbol* def_type = find_defined_type(node->scope, params[i]->static_type);
            param_types[i] = def_type?  def_type->type : param->type;
            node->data.func_node.args[i]->return_type = param_types[i];
        }

        Scope* scope = type? type->scope->parent : node->scope->parent;
        declare_function(
            scope, node->data.func_node.arg_count,
            param_types, inferried_type, node->data.func_node.name
        );

        if (free_type)
            free(defined_type);
    }
}