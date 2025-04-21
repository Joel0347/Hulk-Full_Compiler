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
    while (current_symbol != NULL) {
        Symbol* next = current_symbol->next;
        free(current_symbol->name);
        free(current_symbol);
        current_symbol = next;
    }
}

void destroy_scope(Scope* scope) {
    if (scope == NULL) {
        return;
    }

    Symbol* lists_to_free[] = { 
        scope->symbols, 
        scope->functions, 
        scope->defined_types 
    };

    for (int i = 0; i < 3; i++)
        free_symbol(lists_to_free[i]);

    free(scope);
}

void declare(Symbol* symbol, const char* name, Type* type) {
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->name = strdup(name);
    new_symbol->type = type;
    new_symbol->next = symbol;
    symbol = new_symbol;
}

void declare_symbol(Scope* scope, const char* name, Type* type) {    
    // Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    // symbol->name = strdup(name);
    // symbol->type = type;
    // symbol->next = scope->symbols;
    // scope->symbols = symbol;
    declare(scope->symbols, name, type);
}

void declare_function(Scope* scope, const char* name, Type* type) {    
    declare(scope->functions, name, type);
}

void declare_type(Scope* scope, Type* type) {    
    declare(scope->defined_types, type->name, type);
}

void init_basic_types(Scope* scope) {
    declare_type(scope, &TYPE_NUMBER_INST);
    declare_type(scope, &TYPE_STRING_INST);
    declare_type(scope, &TYPE_BOOLEAN_INST);
    declare_type(scope, &TYPE_VOID_INST);
    declare_type(scope, &TYPE_OBJECT_INST);
}

Symbol* find(Scope* scope, Symbol* symbols, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* current = symbols;
    while (current) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }
    
    if (scope->parent) {
        return find(scope->parent, symbols, name);
    }
    
    return NULL;
}

Symbol* find_symbol(Scope* scope, const char* name) {
    // if (!scope) {
    //     return NULL;
    // }

    // Symbol* current = scope->symbols;
    // while (current) {
    //     if (!strcmp(current->name, name)) {
    //         return current;
    //     }
    //     current = current->next;
    // }
    
    // if (scope->parent) {
    //     return find_symbol(scope->parent, name);
    // }
    
    // return NULL;
    return find(scope, scope->symbols, name);
}

Symbol* find_function(Scope* scope, const char* name) {
    return find(scope, scope->functions, name);
}

Symbol* find_defined_type(Scope* scope, const char* name) {
    return find(scope, scope->defined_types, name);
}