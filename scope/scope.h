#ifndef SCOPE_H
#define SCOPE_H

#include "../type/type.h"

typedef struct Symbol {
    char* name;
    Type* type;
    struct Symbol* next;
} Symbol;

typedef struct Scope {
    Symbol* symbols;
    Symbol* functions;
    Symbol* defined_types;
    struct Scope* parent;
} Scope;

Scope* create_scope(Scope* parent);
void destroy_scope(Scope* scope);
void declare_symbol(Scope* scope, const char* name, Type* type);
void declare_function(Scope* scope, const char* name, Type* type);
void declare_type(Scope* scope, Type* type);
void init_basic_types(Scope* scope);
Symbol* find_symbol(Scope* scope, const char* name);
Symbol* find_function(Scope* scope, const char* name);
Symbol* find_defined_type(Scope* scope, const char* name);

#endif
