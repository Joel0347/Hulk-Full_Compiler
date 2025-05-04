#ifndef SCOPE_H
#define SCOPE_H

#include "../type/type.h"

struct ASTNode;

typedef struct Symbol {
    char* name;
    Type* type;
    int is_param;
    struct ASTNode* value;
    struct Symbol* next;
} Symbol;

typedef struct Function {
    int arg_count;
    Type** args_types;
    Type* result_type;
    char* name;
    struct Function* next;
} Function;

typedef struct FuncTable {
    struct Function* first;
    int count;
} FuncTable;

typedef struct FuncData {
    struct Function* func;
    struct Tuple* state;
} FuncData;

typedef struct Scope {
    Symbol* symbols;
    FuncTable* functions;
    Symbol* defined_types;
    struct Scope* parent;
} Scope;

Scope* create_scope(Scope* parent);
void destroy_scope(Scope* scope);
void declare_symbol(Scope* scope, const char* name, Type* type, int is_param, struct ASTNode*);
void declare_function(
    Scope* scope, int arg_count, Type** args_types, 
    Type* result_type, char* name
);
void declare_type(Scope* scope, Type* type);
char* find_function_by_name(Scope* scope, char* name);
void init_builtins(Scope* scope);
Symbol* find_symbol(Scope* scope, const char* name);
FuncData* find_function(Scope* scope, Function* f);
Symbol* find_defined_type(Scope* scope, const char* name);

#endif
