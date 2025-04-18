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
    struct Scope* parent;
} Scope;

Scope* create_scope(Scope* parent);
void destroy_scope(Scope* scope);
void declare_symbol(Scope* scope, const char* name, Type* type);
Symbol* find_symbol(Scope* scope, const char* name);

#endif
