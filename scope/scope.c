#include "scope.h"
#include "ast/ast.h"
#include <stdlib.h>
#include <string.h>

// <----------DECLARATIONS---------->

// method to create a scope
Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->symbols = NULL;
    scope->functions = NULL;
    scope->defined_types = NULL;
    scope->parent = parent;
    scope->functions = (FuncTable*)malloc(sizeof(FuncTable));
    scope->functions->count = 0;
    scope->functions->first = NULL;
    scope->s_count = 0;
    scope->t_count = 0;
    return scope;
}

// method to save a symbol in a scope
void declare_symbol(
    Scope* scope, const char* name, Type* type, 
    int is_param, struct ASTNode* value
) {
    Symbol* s = find_symbol_in_scope(scope, name);

    if (s) {
        s->type = type;
        s->is_param = is_param;
        s->derivations = add_node_list(value, s->derivations);
        return;
    }

    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->is_param = is_param;
    symbol->is_type_param = 0;
    symbol->derivations = add_node_list(value, NULL);
    symbol->next = scope->symbols;
    scope->symbols = symbol;
    scope->s_count += 1;
}

// method to save a function in a scope
void declare_function(
    Scope* scope, int arg_count, Type** args_types, 
    Type* result_type, char* name
) {    
    Function* func = (Function*)malloc(sizeof(Function));
    func->name = name;
    func->arg_count = arg_count;
    func->next = NULL;
    func->result_type = result_type;

    if (arg_count > 0) {
        func->args_types = malloc(arg_count * sizeof(Type*));
        for (int i = 0; i < arg_count; i++) {
            func->args_types[i] = args_types[i];
        }
    } else {
        func->args_types = NULL;
    }

    func->next = scope->functions->first;
    scope->functions->first = func;
    scope->functions->count += 1;
}

// method to save a type in a scope
void declare_type(Scope* scope, Type* type) {
    Symbol* def_type = (Symbol*)malloc(sizeof(Symbol));
    def_type->name = type->name;
    def_type->type = type;
    def_type->next = scope->defined_types;
    scope->defined_types = def_type;
    scope->t_count += 1;
}

// method to initializate builtin functions and types
void init_builtins(Scope* scope) {
    // BUILTIN TYPES:

    declare_type(scope, &TYPE_NUMBER); //number
    declare_type(scope, &TYPE_STRING); // string
    declare_type(scope, &TYPE_BOOLEAN); // boolean
    declare_type(scope, &TYPE_OBJECT); // object
    declare_type(scope, &TYPE_VOID); // void


    // BUILTIN FUNCTIONS:

    //print
    declare_function(
        scope, 0, NULL, &TYPE_VOID, "print"
    );
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER }, &TYPE_VOID, "print"
    );
    declare_function(
        scope, 1, (Type*[]){ &TYPE_BOOLEAN }, &TYPE_VOID, "print"
    );
    declare_function(
        scope, 1, (Type*[]){ &TYPE_STRING }, &TYPE_VOID, "print"
    );
    // sqrt
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER }, &TYPE_NUMBER, "sqrt"
    );
    // sin
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER }, &TYPE_NUMBER, "sin"
    );
    // cos
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER }, &TYPE_NUMBER, "cos"
    );
    // exp
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER }, &TYPE_NUMBER, "exp"
    );
    // log
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER }, &TYPE_NUMBER, "log"
    );
    declare_function(
        scope, 2, (Type*[]){ &TYPE_NUMBER, &TYPE_NUMBER }, &TYPE_NUMBER, "log"
    );
    // rand
    declare_function(
        scope, 0, NULL, &TYPE_NUMBER, "rand"
    );
}


// <----------QUERIES---------->

// method to check whether or not two functions are equal
Tuple* func_equals(Function* f1, Function* f2) {
    if (strcmp(f1->name, f2->name)) {
        Tuple* tuple = init_tuple_for_count(0, -1, -1);
        tuple->same_name = 0;
        return tuple;
    }

    if (f1->arg_count != f2->arg_count)
        return init_tuple_for_count(0, f1->arg_count, f2->arg_count);

    return map_type_equals(f1->args_types, f2->args_types, f1->arg_count);
}

// method to check whether or not a type contains a method in its scope
int type_contains_method_in_scope(Type* type, char* name, int see_parent) {
    if (!type->dec)
        return 0;
        
    char* tmp_name = concat_str_with_underscore(
        type->name, name
    );

    if (find_function_by_name(type->dec->scope, tmp_name, 0))
        return 1;

    if (see_parent)
        return type_contains_method_in_scope(type->parent, name, see_parent);

    return 0;
}

// method to check whether or not a type contains a method in its context
int type_contains_method_in_context(ASTNode* node, char* name) {
    for (int i = 0; i < node->data.type_node.def_count; i++) {
        ASTNode* def = node->data.type_node.definitions[i];
        if (def->type == NODE_FUNC_DEC &&
            (!strcmp(def->data.func_node.name, name) ||
            !strcmp(def->data.func_node.name, concat_str_with_underscore(
                node->data.type_node.name, name
            )))
        ) {
            return 1;
        } 
    }
    
    return 0;
}

// method to check whether or not a type method signature matches with the original
FuncData* match_signature(Type* type, char* name, Type** param_types, int count, Type* ret) {
    if (!type->dec || !type->parent || !type->parent->dec)
        return NULL;

    Function f = { count, param_types, ret, name, NULL };
    FuncData* data = find_type_func(type, &f, NULL);

    if (data && !data->state->matched && !data->state->same_name) {
        if (!find_item_in_type_hierarchy(type->parent->dec->context, name, type, 1)) {
            data = NULL;
        }
    }

    return data;
}


// <----------SEARCHES---------->

// method to find a symbol in a specific scope
Symbol* find_symbol_in_scope(Scope* scope, const char* name) {
    Symbol* current = scope->symbols;
    int i = 0;
    while (i < scope->s_count) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
        i++;
    }

    return NULL;
}

// method to find a symbol in a scope hierarchy
Symbol* find_symbol(Scope* scope, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* current = find_symbol_in_scope(scope, name);

    if (current)
        return current;
    
    if (scope->parent) {
        return find_symbol(scope->parent, name);
    }
    
    return NULL;
}

// method to find a parameter in a scope hierarchy
Symbol* find_parameter(Scope* scope, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* current = scope->symbols;
    int i = 0;
    while (i < scope->s_count) {
        if (current->is_param && !strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
        i++;
    }

    if (current)
        return current;
    
    if (scope->parent) {
        return find_parameter(scope->parent, name);
    }
    
    return NULL;
}

// method to find a function in a scope hierarchy by its signature
FuncData* find_function(Scope* scope, Function* f, Function* dec) {
    if (!scope) {
        return NULL;
    }

    FuncData* result = (FuncData*)malloc(sizeof(FuncData));
    int not_found = 1;
    Function* current = scope->functions->first;
    int i = 0;

    while (i < scope->functions->count) {
        // Pack the errors or the function found
        Tuple* tuple = func_equals(current, f);
        if (tuple->matched) {
            result->state = tuple;
            result->func = current;
            return result;
        }
        if (tuple->same_name) {
            not_found = 0;
            if ((!result->state && !tuple->same_count) || tuple->same_count) {
                // keep track of the signature found
                result->state = tuple;
                result->func = current;
            }
        }

        current = current->next;
        i++;
    }
        
    if (scope->parent) {
        FuncData* data = find_function(scope->parent, f, dec);

        if (not_found || data->state->matched)
            return data;

        return result;
    }
    
    if (not_found && dec) {
        // trying to match with the declaration
        result->state = func_equals(dec, f);
        result->func = dec;
    } else if (not_found && !dec) {
        result->state = init_tuple_for_count(0, -1, -1);
        result->state->same_name = 0;
    }
    
    return result;
}

// method to find a function in a scope hierarchy by its name
Function* find_function_by_name(Scope* scope, char* name, int see_parent) {
    if (!scope) {
        return NULL;
    }

    Function* current = scope->functions->first;
    int i = 0;
    while (i < scope->functions->count) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
        i++;
    }
    
    if (see_parent && scope->parent) {
        return find_function_by_name(scope->parent, name, see_parent);
    }
    
    return NULL;
}

// method to find a type in a scope hierarchy
Symbol* find_defined_type(Scope* scope, const char* name) {
    if (!scope || !name) {
        return NULL;
    }

    Symbol* current = scope->defined_types;
    int i = 0;
    while (i < scope->t_count) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
        i++;
    }
    
    if (scope->parent) {
        return find_defined_type(scope->parent, name);
    }
    
    return NULL;
}

// method to find a type in a scope hierarchy by its signature
FuncData* find_type_data(Scope* scope, Function* f, Function* dec) {
    if (!scope) {
        return NULL;
    }

    FuncData* result = (FuncData*)malloc(sizeof(FuncData));
    int not_found = 1;

    if (scope->defined_types) {
        Symbol* current_sym = scope->defined_types;
        int i = 0;
        while (i < scope->t_count) {
            // converting to Function to reuse existing methods
            Function* current = (Function*)malloc(sizeof(Function));
            current->name = current_sym->name;
            current->arg_count = current_sym->type->arg_count;
            current->args_types = current_sym->type->param_types;
            current->result_type = current_sym->type;
            Tuple* tuple = func_equals(current, f);
            if (tuple->matched) {
                result->state = tuple;
                result->func = current;
                return result;
            }
            if (tuple->same_name) {
                not_found = 0;
                if ((!result->state && !tuple->same_count) || tuple->same_count) {
                    result->state = tuple;
                    result->func = &current;
                }
            }

            current_sym = current_sym->next;
            i++;
        }
    }

    if (scope->parent) {
        FuncData* data = find_type_data(scope->parent, f, dec);

        if (not_found || data->state->matched)
            return data;

    
        return result;
    }

    if (not_found && dec) {
        result->state = func_equals(dec, f);
        result->func = dec;
    } else if (not_found && !dec) {
        result->state = init_tuple_for_count(0, -1, -1);
        result->state->same_name = 0;
    }
    
    return result;
}

// method to find a function in a type hierarchy by its signature
FuncData* find_type_func(Type* type, Function* f, Function* dec) {
    if (!type->dec) {
        FuncData* data = (FuncData*)malloc(sizeof(FuncData));
        Tuple* tuple = init_tuple_for_count(0, -1, -1);
        data->state = tuple;
        data->func = NULL;
        data->state->same_name = 0;
        return data;
    }
    
    FuncData* data = find_function(type->dec->scope, f, dec);

    if (data && (data->state->matched) || data->state->same_name) {
        return data;
    }

    if (type->parent) {
        f->name = delete_underscore_from_str(f->name, type->name);
        f->name = concat_str_with_underscore(type->parent->name, f->name);

        return find_type_func(type->parent, f, dec);
    }

    return data;
}

// method to find a attribute in a type hierarchy
Symbol* find_type_attr(Type* type, char* attr_name) {
    if (!type->dec)
        return NULL;

    Symbol* sym = find_symbol_in_scope(type->dec->scope, attr_name);

    if (!sym && type->parent) {
        char* name = delete_underscore_from_str(attr_name, type->name);
        name = concat_str_with_underscore(type->parent->name, name);
        return find_type_attr(type->parent, name);
    }
    
    return sym;
}

// method to find the closest function declaration in a type hierarchy
char* find_base_func_dec(Type* type, char* name) {
    if (!(type->parent) || !(type->parent->dec))
        return NULL;
    
    char* parent_name = type->parent->name;
    char* tmp_name = delete_underscore_from_str(name, type->name);
    tmp_name = concat_str_with_underscore(parent_name, tmp_name);

    Function* f = find_function_by_name(type->parent->dec->scope, tmp_name, 1);

    if (!f) {
        return find_base_func_dec(type->parent, tmp_name);
    }

    return f->name;
}

// method to find types which contains a given method
NodeList* find_types_by_method(Context* context, char* name) {
   NodeList* current = NULL;
    
    while (context) {
        int i = 0;
        ContextItem* item = context->first;
        while (i < context->count) {
            if (item->declaration->type == NODE_TYPE_DEC) {
                if (type_contains_method_in_context(item->declaration, name)) {
                    current = add_node_list(item->declaration, current);
                }
            }

            i++;
            item = item->next;
        }
        
        context = context->parent;
    }
    

    return current;
}


// <----------DESTRUCTION---------->

// method to free a symbol
void free_symbol(Symbol* current_symbol, int count) {
    int i = 0;
    while (i < count && current_symbol) {
        Symbol* next = current_symbol->next;
        free(current_symbol);
        current_symbol = next;
        i++;
    }
}

// method to free a function table
void free_func_table(FuncTable* table) {
    if (!table)
        return;

    Function* current = table->first;
    int i = 0;
    while (i < table->count) {
        Function* next = current->next;
        free(current);
        current = next;
        i++;
    }

    free(table);
}

// method to free a scope
void destroy_scope(Scope* scope) {
    if (scope == NULL) {
        return;
    }

    free_symbol(scope->symbols, scope->s_count);
    free_symbol(scope->defined_types, scope->t_count);
    free_func_table(scope->functions);

    free(scope);
}