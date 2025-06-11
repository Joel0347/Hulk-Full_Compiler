#ifndef SCOPE_H
#define SCOPE_H

#include "../type/type.h"

struct ASTNode;
typedef struct Symbol {
    char* name;
    Type* type;
    int is_param;
    int is_type_param;
    ValueList* derivations;
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
    int s_count;
    int t_count;
    Symbol* symbols;
    FuncTable* functions;
    Symbol* defined_types;
    struct Scope* parent;
} Scope;

// to get global context in the program or block.

typedef struct ContextItem {
    struct ASTNode* declaration;
    struct Type* return_type;
    struct ContextItem* next;
} ContextItem;
typedef struct Context {
    int count;
    ContextItem* first;
    struct Context* parent;
} Context;

Scope* create_scope(Scope* parent);
// Scope* copy_scope_symbols(Scope* from);
Context* create_context(Context* parent);
void destroy_scope(Scope* scope);
void destroy_context(Context* context);
void declare_symbol(Scope* scope, const char* name, Type* type, int is_param, struct ASTNode* value);
void declare_function(
    Scope* scope, int arg_count, Type** args_types, 
    Type* result_type, char* name
);
void declare_type(Scope* scope, Type* type);
int save_context_item(Context* context, struct ASTNode* item);
int save_context_for_type(Context* context, struct ASTNode* item, char* type_name);
struct ContextItem* find_item_in_type(Context* context, char* name, Type* type, int func_dec);
Function* find_function_by_name(Scope* scope, char* name, int see_parent);
void init_builtins(Scope* scope);
Symbol* find_symbol(Scope* scope, const char* name);
FuncData* find_function(Scope* scope, Function* f, Function* dec);
FuncData* find_type_data(Scope* scope, Function* f, Function* dec);
Symbol* find_defined_type(Scope* scope, const char* name);
struct ContextItem* find_context_item(Context* context, char* name, int type, int var);
Symbol* find_parameter(Scope* scope, const char* name);
FuncData* get_type_func(Type* type, Function* f, Function* dec);
FuncData* match_signature(Type* type, char* name, Type** param_types, int count, Type* ret);
Symbol* get_type_attr(Type* type, char* attr_name);
char* find_base_func_dec(Type* type, char* name);
int type_contains_attr(Type* type, char* name, int see_parent);
ValueList* get_types_by_attr(Context* context, char* name);
void free_ast(struct ASTNode* node);

#endif
