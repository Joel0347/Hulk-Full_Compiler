#include "semantic.h"

// method to check function call node
void check_function_call(Visitor* v, ASTNode* node, Type* type) {
    ASTNode** args = node->data.func_node.args;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        args[i]->context->parent = node->context;
        accept(v, args[i]);
    }

    // deciding whether it is necessary to use the context of the type or not
    ContextItem* item = type?
        find_item_in_type_hierarchy(
            type->dec->context,
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
    
    Scope* scope = type? type->dec->scope : node->scope;
    IntList* unified = unify_func(
        v, args, scope, node->data.func_node.arg_count, 
        node->data.func_node.name, item
    );

    for (IntList* current = unified; current != NULL; current = current->next)
    {
        accept(v, args[current->value]); // visit again if unified
    }

    free_int_list(unified);
    

    Type** args_types = map_get_type(args, node->data.func_node.arg_count);

    Function* f = (Function*)malloc(sizeof(Function));
    f->name = node->data.func_node.name;
    f->arg_count = node->data.func_node.arg_count;
    f->args_types = args_types;

    Function* dec = NULL;

    if (item) {
        Type** dec_args_types = map_get_type(
            item->declaration->data.func_node.args, 
            item->declaration->data.func_node.arg_count
        );

        dec = (Function*)malloc(sizeof(Function));
        dec->name = item->declaration->data.func_node.name;
        dec->arg_count = item->declaration->data.func_node.arg_count;
        dec->args_types = dec_args_types;
        dec->result_type = item->return_type ? item->return_type : &TYPE_ANY;
    }

    // trying to find the function signature in the scope or context
    // deciding whether it is a type method or not
    FuncData* funcData = type?
        find_type_func(type, f, dec) : 
        find_function(node->scope, f, dec);

    if (funcData->func) {
        node->return_type = funcData->func->result_type;
    }

    if (!funcData->state->matched) {
        // Unpacking the error data
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

// method to visit function call node
void visit_function_call(Visitor* v, ASTNode* node) {
    check_function_call(v, node, NULL);
}

// method to visit function declaration node
void visit_function_dec(Visitor* v, ASTNode* node) {
    check_function_dec(v, node, NULL);
}

// method to check function declaration node
void check_function_dec(Visitor* v, ASTNode* node, Type* type) {
    // To prevent loops 
    if (node->checked) {
        return;
    }

    node->checked = 1;
    char* visitor_function = v->current_function;

    if (type) // save this type method as the current one in the visitor for 'base' node
        v->current_function = node->data.func_node.name;

    ASTNode** params = node->data.func_node.args;
    ASTNode* body = node->data.func_node.body;
    body->scope->parent = node->scope;
    body->context->parent = node->context;

    if (match_as_keyword(node->data.func_node.name)) {
        report_error(
            v, "Keyword '%s' can not be used as a function name. Line: %d.", 
            node->data.func_node.name, node->line
        );
    }

    // Checking multiple uses of the same symbol in the parameters
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
                v->current_function = visitor_function;
                return;
            }
        }
    }
    
    // Parameters checking
    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        params[i]->scope->parent = node->scope;
        params[i]->context->parent = node->context;
        Symbol* param_type = find_defined_type(node->scope, params[i]->static_type);
        int free_type = 0;

        // Type annotation
        if (strcmp(params[i]->static_type, "") && !param_type) {
            ContextItem* item = find_context_item(
                node->context, params[i]->static_type, 1, 0
            );
    
            if (item) {
                if (!item->declaration->checked) {
                    accept(v, item->declaration);
                    param_type = find_defined_type(node->scope, params[i]->static_type);
                } else if (item->return_type) {
                    param_type = (Symbol*)malloc(sizeof(Symbol));
                    param_type->name = item->return_type->name;
                    param_type->type = item->return_type;
                    free_type = 1;
                }
    
                if (!param_type) {
                    report_error(
                        v, "Parameter '%s' was defined as '%s', but that type produces a "
                        "cirular reference. Line: %d.", params[i]->data.variable_name, 
                        params[i]->static_type, node->line
                    );
                }
            } else {
                report_error(
                    v, "Parameter '%s' was defined as '%s', which is not a valid type. Line: %d.", 
                    params[i]->data.variable_name, params[i]->static_type, node->line
                );
                params[i]->return_type = &TYPE_ERROR;
            }
        } else if (strcmp(params[i]->static_type, "") && type_equals(param_type->type, &TYPE_VOID)) {
            report_error(
                v, "Parameter '%s' was defined as 'Void', which is not a valid type. Line: %d.", 
                params[i]->data.variable_name, node->line
            );
            params[i]->return_type = &TYPE_ERROR;
        }

        // Annotate parameters as Any if they weren't type annotated
        if (!strcmp(params[i]->static_type, "")) {
            params[i]->return_type = &TYPE_ANY;
        } else if (param_type) {
            params[i]->return_type = param_type->type;
        }

        // Declare parameters in the function scope
        declare_symbol(
            node->scope, params[i]->data.variable_name,
            params[i]->return_type, 1, NULL
        );

        if (free_type)
            free(param_type);
    }

    accept(v, body);
    // Deciding in which context to look for 
    ContextItem* item = type?
        find_item_in_type_hierarchy(
            type->dec->context, 
            node->data.func_node.name, 
            type, 1) :
        find_context_item(node->context, node->data.func_node.name, 0, 0);
    Type* inferried_type = get_type(body);
    Symbol* defined_type = find_defined_type(node->scope, node->static_type);

    if (type_equals(inferried_type, &TYPE_ANY) && 
        defined_type && unify(v, body, defined_type->type)
    ) {
        accept(v, body); // visit again if unified
        inferried_type = defined_type->type;
    }

    if (!defined_type && item->return_type &&
        !is_ancestor_type(item->return_type, inferried_type) &&
        !type_equals(&TYPE_ERROR, item->return_type) &&
        !type_equals(&TYPE_ANY, item->return_type) &&
        !type_equals(&TYPE_ERROR, inferried_type) &&
        !type_equals(&TYPE_ANY, inferried_type)
    ) { // comparing inferried type with the type got from use cases in its own body
        report_error(
            v,  "Impossible to infer return type of function '%s'."
            " It behaves both as '%s' and '%s'. Line: %d."
            , node->data.func_node.name, inferried_type->name,
            item->return_type->name, node->line
        );
    }

    int free_type = 0;

    // Checking return type
    if (strcmp(node->static_type, "") && !defined_type) {
        ContextItem* item = find_context_item(
            node->context, node->static_type, 1, 0
        );

        if (item) {
            if (!item->declaration->checked) {
                accept(v, item->declaration);
                defined_type = find_defined_type(node->scope, node->static_type);
            } else if (item->return_type) {
                defined_type = (Symbol*)malloc(sizeof(Symbol));
                defined_type->name = item->return_type->name;
                defined_type->type = item->return_type;
                free_type = 1;
            }

            if (!defined_type) {
                report_error(
                    v, "The return type of function '%s' was defined as '%s', but that type produces a "
                    "cirular reference. Line: %d.", node->data.variable_name, 
                    node->static_type, node->line
                );
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

    Function* func = find_function_by_name(node->scope, node->data.func_node.name, 1);
    Type** param_types = map_get_type(params, node->data.func_node.arg_count);

    if (!func) {
        // Checking that no parameter is still annotated as Any
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

        // Checking whether or not a type method changed the original signature
        if (type && type->parent && !is_builtin_type(type->parent)) {
            char* name = delete_underscore_from_str(
                node->data.func_node.name, type->name
            );
            FuncData* data = match_signature(
                type, node->data.func_node.name,
                param_types, node->data.func_node.arg_count,
                inferried_type
            );

            if (data && !data->state->matched) {
                if (!data->state->same_name) {
                    report_error(
                        v, "Method '%s' definition uses the "
                        "overridden by type '%s'. Line: %d.", 
                        name, type->name ,node->line
                    );
                } else if (!data->state->same_count) {
                    report_error(
                        v, "Method '%s' originally received %d argument(s), but %d was(were)"
                        " given when overridden by type '%s'. Line: %d.",
                        name, data->state->arg1_count, 
                        data->state->arg2_count, type->name, node->line
                    );
                } else {
                    if (!strcmp(data->state->type2_name, "Error"))
                        return;
        
                    report_error(
                        v, "Method '%s' originally received '%s', not '%s' as argument %d when"
                        " overridden by type '%s'. Line: %d.",
                        name, data->state->type1_name, 
                        data->state->type2_name, data->state->pos, type->name, node->line
                    );
                }
            } else if (data && data->func && !is_ancestor_type(data->func->result_type, inferried_type)) {
                if (!type_equals(inferried_type, &TYPE_ERROR) && 
                    !type_equals(data->func->result_type, &TYPE_ERROR)
                ) {
                    report_error(
                        v, "Method '%s' originally returned '%s', not '%s' when"
                        " overridden by type '%s'. Line: %d.",
                        name, data->func->result_type->name, 
                        inferried_type->name, type->name, node->line
                    );
                }
            }
        }

        declare_function(
            node->scope->parent, node->data.func_node.arg_count,
            param_types, inferried_type, node->data.func_node.name
        );

        body->return_type = inferried_type;

        if (free_type)
            free(defined_type);

        // save again the old type method before this one in the visitor
        v->current_function = visitor_function;
    }
}