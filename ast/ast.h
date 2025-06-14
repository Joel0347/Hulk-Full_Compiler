#ifndef AST_H
#define AST_H

#include "../type/type.h"
#include "../scope/scope.h"

extern int line_num;
typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGNMENT,
    NODE_D_ASSIGNMENT,
    NODE_STRING,
    NODE_BOOLEAN,
    NODE_PROGRAM,
    NODE_FUNC_CALL,
    NODE_BLOCK,
    NODE_FUNC_DEC,
    NODE_LET_IN,
    NODE_CONDITIONAL,
    NODE_Q_CONDITIONAL,
    NODE_LOOP,
    NODE_FOR_LOOP,
    NODE_TEST_TYPE,
    NODE_CAST_TYPE,
    NODE_TYPE_DEC,
    NODE_TYPE_INST,
    NODE_TYPE_GET_ATTR,
    NODE_TYPE_SET_ATTR,
    NODE_BASE_FUNC
} NodeType;

typedef struct ASTNode {
    int line;
    int is_param;
    int checked;
    NodeType type;
    Type* return_type;
    char* static_type;
    Scope* scope;
    Context* context;
    NodeList* derivations;
    union {
        double number_value;
        char* string_value;
        char* variable_name;
        struct {
            Operator op;
            char* op_name;
            struct ASTNode *left;
            struct ASTNode *right;
        } op_node;
        struct {
            struct ASTNode** statements;
            int count;
        } program_node;
        struct {
            char* name;
            struct ASTNode** args;
            int arg_count;
            struct ASTNode *body;
        } func_node;
        struct {
            struct ASTNode *cond;
            struct ASTNode *body_true;
            struct ASTNode *body_false;
            int stm;
        } cond_node;
        struct {
            char* name;
            char* parent_name;
            struct ASTNode** p_args;
            int p_arg_count;
            struct ASTNode** args;
            int arg_count;
            struct ASTNode** definitions;
            int def_count;
            struct ASTNode* parent_instance;
            int id;
            int p_constructor;
        } type_node;
        struct {
            char* type_name;
            Type* type;
            struct ASTNode* exp;
        } cast_test;
    } data;
} ASTNode;

ASTNode* create_program_node(ASTNode** statements, int count, NodeType type);
ASTNode* create_number_node(double value);
ASTNode* create_string_node(char* value);
ASTNode* create_boolean_node(char* value);
ASTNode* create_variable_node(char* name, char* type, int is_param);
ASTNode* create_binary_op_node(Operator op, char* op_name, ASTNode* left, ASTNode* right, Type* return_type);
ASTNode* create_unary_op_node(Operator op, char* op_name, ASTNode* operand, Type* return_type);
ASTNode* create_assignment_node(char* var, ASTNode* value, char* type_name, NodeType type);
ASTNode* create_func_call_node(char* name, ASTNode** args, int arg_count);
ASTNode* create_func_dec_node(char* name, ASTNode** args, int arg_count, ASTNode* body, char* ret_type); 
ASTNode* create_let_in_node(ASTNode** declarations, int dec_count, ASTNode* body);
ASTNode* create_conditional_node(ASTNode* condition, ASTNode* body_true, ASTNode* body_false);
ASTNode* create_q_conditional_node(ASTNode* exp, ASTNode* body_true, ASTNode* body_false);
ASTNode* create_loop_node(ASTNode* condition, ASTNode* body);
ASTNode* create_for_loop_node(char* var_name, ASTNode** params, ASTNode* body, int count);
ASTNode* create_test_casting_type_node(ASTNode* exp, char* type_name, int test);
ASTNode* create_type_dec_node(
    char* name, ASTNode** params, int param_count, char* parent_name,
    ASTNode** p_params, int p_param_count, ASTNode* body_block, int p_constructor
);
ASTNode* create_type_instance_node(char* name, ASTNode** args, int arg_count);
ASTNode* create_attr_getter_node(ASTNode* instance, ASTNode* member);
ASTNode* create_attr_setter_node(ASTNode* instance, ASTNode* member, ASTNode* value);
ASTNode* create_base_func_node(ASTNode** args, int arg_count);
void print_ast(ASTNode* node, int indent);

#endif