#include "semantic.h"


void visit_builtin_func_call(Visitor* v, ASTNode* node) {
    ASTNode** args = node->data.func_node.args;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        accept(v, args[i]);
    }

    Type** args_types = find_types(args, node->data.func_node.arg_count);
    
    FuncTypeRule rule = create_func_rule(
        node->data.func_node.arg_count, 
        args_types, node->return_type, 
        node->data.func_node.name
    );

    Tuple* compatibility = find_func_match(&rule);

    if (!compatibility->matched) {
        if (!compatibility->same_count) {
            char* str = NULL;
            asprintf(&str, "Function '%s' receives %d argument(s), but %d was(were) given. Line: %d.",
                node->data.func_node.name, compatibility->arg1_count, 
                compatibility->arg2_count, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        } else {
            if (!strcmp(compatibility->type2_name, "error"))
                return;

            char* str = NULL;
            asprintf(&str, "Function '%s' receives '%s', not '%s' as argument %d. Line: %d.",
                node->data.func_node.name, compatibility->type1_name, 
                compatibility->type2_name, compatibility->pos, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        }
    }

    free_tuple(compatibility);
    free(args_types);
}