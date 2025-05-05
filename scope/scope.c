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
        return;
    }

    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->is_param = is_param;
    symbol->value = value;
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

void declare_type(Scope* scope, Type* type) {    
    Symbol* def_type = (Symbol*)malloc(sizeof(Symbol));
    def_type->name = type->name;
    def_type->type = type;
    def_type->next = scope->defined_types;
    scope->defined_types = def_type;
}

void init_builtins(Scope* scope) {
    // BUILTIN TYPES:

    declare_type(scope, &TYPE_NUMBER_INST); //number
    declare_type(scope, &TYPE_STRING_INST); // string
    declare_type(scope, &TYPE_BOOLEAN_INST); // boolean
    declare_type(scope, &TYPE_OBJECT_INST); // object

    // BUILTIN FUNCTIONS:

    //print
    declare_function(
        scope, 0, NULL, &TYPE_VOID_INST, "print"
    );
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_VOID_INST, "print"
    );
    declare_function(
        scope, 1, (Type*[]){ &TYPE_BOOLEAN_INST }, &TYPE_VOID_INST, "print"
    );
    declare_function(
        scope, 1, (Type*[]){ &TYPE_STRING_INST }, &TYPE_VOID_INST, "print"
    );
    // sqrt
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "sqrt"
    );
    // sin
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "sin"
    );
    // cos
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "cos"
    );
    // exp
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "exp"
    );
    // log
    declare_function(
        scope, 1, (Type*[]){ &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "log"
    );
    declare_function(
        scope, 2, (Type*[]){ &TYPE_NUMBER_INST, &TYPE_NUMBER_INST }, &TYPE_NUMBER_INST, "log"
    );
    // rand
    declare_function(
        scope, 0, NULL, &TYPE_NUMBER_INST, "rand"
    );
}

Tuple* args_type_equals(Type** args1, Type** args2, int count) {
    for (int i = 0; i < count; i++)
    {
        if ((!type_equals(args2[i], &TYPE_ERROR_INST) &&
            !type_equals(args2[i], &TYPE_ANY_INST)
            ) &&
            (!type_equals(args1[i], &TYPE_ANY_INST) &&
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

        // if (not_found || data->state->matched) {
        //     return data;
        // } else {
        //     return result;
        // }
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