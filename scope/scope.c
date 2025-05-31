#include "scope.h"
#include <stdlib.h>
#include <string.h>

Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->symbols = NULL;
    scope->functions = NULL;
    scope->defined_types = NULL;
    scope->parent = parent;
    return scope;
}

Scope* copy_scope_symbols(Scope* from, Scope* to) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->symbols = to->symbols;
    scope->functions = to->functions;
    scope->defined_types = to->defined_types;
    
    Symbol* current = from->symbols;

    while (current) {
        Symbol* tmp = (Symbol*)malloc(sizeof(Symbol));
        tmp->name = current->name;
        tmp->type = current->type;
        tmp->is_param = current->is_param;
        tmp->derivations = current->derivations;
        tmp->next = scope->symbols;
        scope->symbols = tmp;

        current = current->next;
    }

    return scope;
}

void free_symbol(Symbol* current_symbol) {
    while (current_symbol) {
        Symbol* next = current_symbol->next;
        free(current_symbol);
        current_symbol = next;
    }
}

void free_func_table(FuncTable* table) {
    if (!table)
        return;

    Function* current = table->first;
    while (current != NULL) {
        Function* next = current->next;
        free(current);
        current = next;
    }

    free(table);
}

void destroy_scope(Scope* scope) {
    if (scope == NULL) {
        return;
    }

    free_symbol(scope->symbols);
    free_symbol(scope->defined_types);
    free_func_table(scope->functions);

    free(scope);
}

Symbol* find_symbol_in_scope(Scope* scope, const char* name) {
    Symbol* current = scope->symbols;
    while (current) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void declare_symbol(
    Scope* scope, const char* name, Type* type, 
    int is_param, struct ASTNode* value
) {
    Symbol* s = find_symbol_in_scope(scope, name);

    if (s) {
        s->type = type;
        s->is_param = is_param;
        s->derivations = add_value_list(value, s->derivations);
        return;
    }

    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->is_param = is_param;
    symbol->derivations = add_value_list(value, NULL);
    symbol->next = scope->symbols;
    scope->symbols = symbol;
}

void declare_function(
    Scope* scope, int arg_count, Type** args_types, 
    Type* result_type, char* name
) {    
    Function* func = (Function*)malloc(sizeof(Function));
    func->name = name;
    func->arg_count = arg_count;
    func->result_type = result_type;

    if(arg_count > 0) {
        func->args_types = malloc(arg_count * sizeof(Type*));
        for (int i = 0; i < arg_count; i++) {
            func->args_types[i] = args_types[i];
        }
    } else {
        func->args_types = NULL;
    }

    if (scope->functions) {
        func->next = scope->functions->first;
        scope->functions->first = func;
        scope->functions->count += 1;
    } else {
        FuncTable* table = (FuncTable*)malloc(sizeof(FuncTable));
        table->first = func;
        table->count = 1;
        scope->functions = table;
    }
}

void declare_type(Scope* scope, Type* type, Scope* parent_scope) {
    Symbol* def_type = (Symbol*)malloc(sizeof(Symbol));
    def_type->name = type->name;
    def_type->type = type;

    if (!parent_scope) {
        def_type->next = scope->defined_types;
        def_type->type->scope = create_scope(NULL);
        def_type->type->context = create_context(NULL);
        scope->defined_types = def_type;
    } else {
        def_type->type->scope = scope;
        def_type->next = parent_scope->defined_types;
        parent_scope->defined_types = def_type;
    }
}

void init_builtins(Scope* scope) {
    // BUILTIN TYPES:

    declare_type(scope, &TYPE_NUMBER, NULL); //number
    declare_type(scope, &TYPE_STRING, NULL); // string
    declare_type(scope, &TYPE_BOOLEAN, NULL); // boolean
    declare_type(scope, &TYPE_OBJECT, NULL); // object
    declare_type(scope, &TYPE_VOID, NULL); // void


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

Tuple* args_type_equals(Type** args1, Type** args2, int count) {
    for (int i = 0; i < count; i++)
    {
        if ((!type_equals(args2[i], &TYPE_ERROR) &&
            !type_equals(args2[i], &TYPE_ANY)
            ) &&
            (!type_equals(args1[i], &TYPE_ANY) &&
            !is_ancestor_type(args1[i], args2[i])
            )
        ) {
            Tuple* tuple = init_tuple_for_types(
                0, args1[i]->name, args2[i]->name, i+1
            );
            return tuple;
        }
    }
    
    return init_tuple_for_types(1, "", "", -1);
}

Tuple* func_equals(Function* f1, Function* f2) {
    if (strcmp(f1->name, f2->name)) {
        Tuple* tuple = init_tuple_for_count(0, -1, -1);
        tuple->same_name = 0;
        return tuple;
    }

    if (f1->arg_count != f2->arg_count)
        return init_tuple_for_count(0, f1->arg_count, f2->arg_count);

    return args_type_equals(f1->args_types, f2->args_types, f1->arg_count);
}

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

Symbol* find_parameter(Scope* scope, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* current = scope->symbols;
    while (current) {
        if (current->is_param && !strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }

    if (current)
        return current;
    
    if (scope->parent) {
        return find_parameter(scope->parent, name);
    }
    
    return NULL;
}

FuncData* find_function(Scope* scope, Function* f, Function* dec) {
    if (!scope) {
        return NULL;
    }

    FuncData* result = (FuncData*)malloc(sizeof(FuncData));
    int not_found = 1;

    if (scope->functions) {
        Function* current = scope->functions->first;
        while (current) {
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
                    result->func = current;
                }
            }

            current = current->next;
        }
    }
        
    if (scope->parent) {
        FuncData* data = find_function(scope->parent, f, dec);

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

Function* find_function_by_name(Scope* scope, char* name) {
    if (!scope) {
        return NULL;
    }

    if (scope->functions) {
        Function* current = scope->functions->first;
        while (current) {
            if (!strcmp(current->name, name)) {
                return current;
            }
            current = current->next;
        }
    }
    
    if (scope->parent) {
        return find_function_by_name(scope->parent, name);
    }
    
    return NULL;
}

Symbol* find_defined_type(Scope* scope, const char* name) {
    if (!scope || !name) {
        return NULL;
    }

    Symbol* current = scope->defined_types;
    while (current) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }
    
    if (scope->parent) {
        return find_defined_type(scope->parent, name);
    }
    
    return NULL;
}

FuncData* find_type_data(Scope* scope, Function* f, Function* dec) {
    if (!scope) {
        return NULL;
    }

    FuncData* result = (FuncData*)malloc(sizeof(FuncData));
    int not_found = 1;

    if (scope->defined_types) {
        Symbol* current_sym = scope->defined_types;
        while (current_sym) {
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
                    result->func = current;
                }
            }

            current_sym = current_sym->next;
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

FuncData* get_type_func(Type* type, Function* f, Function* dec) {
    if (!type->scope) {
        return NULL;
    }

    // f->name = concat_str_with_underscore(type->name, f->name);
    
    // if (dec)
    //     dec->name = f->name;

    FuncData* data = find_function(type->scope, f, dec);

    if (data && (data->state->matched) || data->state->same_name) {
        return data;
    }

    if (type->parent) {
        f->name = delete_underscore_from_str(f->name, type->name);
        f->name = concat_str_with_underscore(type->parent->name, f->name);

        return get_type_func(type->parent, f, dec);
    }

    return data;
}

Symbol* get_type_attr(Type* type, char* attr_name) {
    if (!type->scope)
        return NULL;
    Symbol* sym = find_symbol_in_scope(type->scope, attr_name);

    if (!sym && type->parent) {
        char* name = delete_underscore_from_str(attr_name, type->name);
        name = concat_str_with_underscore(type->parent->name, name);
        return get_type_attr(type->parent, name);
    }
    
    return sym;
}