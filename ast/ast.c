#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// method to create program node or block node
ASTNode* create_program_node(ASTNode** statements, int count, NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = type;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.program_node.statements = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        node->data.program_node.statements[i] = statements[i];
    }
    node->data.program_node.count = count;
    return node;
}

// method to create number node
ASTNode* create_number_node(double value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_NUMBER;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_NUMBER;
    node->data.number_value = value;
    return node;
}

// method to create string node
ASTNode* create_string_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_STRING;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_STRING;
    node->data.string_value = value;
    return node;
}

// method to create boolean node
ASTNode* create_boolean_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_BOOLEAN;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_BOOLEAN;
    node->data.string_value = value;
    return node;
}

// method to create variable node
ASTNode* create_variable_node(char* name, char* type, int is_param) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->is_param = is_param;
    node->type = NODE_VARIABLE;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_OBJECT;
    node->data.variable_name = name;

    if (type)
        node->static_type = type;

    return node;
}

// method to create binary operation node
ASTNode* create_binary_op_node(Operator op, char* op_name, ASTNode* left, ASTNode* right, Type* return_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_BINARY_OP;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = return_type;
    node->data.op_node.op_name = op_name;
    node->data.op_node.op = op;
    node->data.op_node.left = left;
    node->data.op_node.right = right;
    return node;
}

// method to create unary operation node
ASTNode* create_unary_op_node(Operator op, char* op_name, ASTNode* operand, Type* return_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_UNARY_OP;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = return_type;
    node->data.op_node.op_name = op_name;
    node->data.op_node.op = op;
    node->data.op_node.left = operand;
    node->data.op_node.right = NULL;
    return node;
}

// method to create assignment node or destructive assignment node
ASTNode* create_assignment_node(char* var, ASTNode* value, char* type_name, NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->checked = 0;
    node->type = type;
    node->return_type = &TYPE_VOID;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.op_node.left = create_variable_node(var, NULL, 0);
    node->data.op_node.left->static_type = type_name;
    node->data.op_node.right = value;
    node->derivations = add_node_list(value, NULL);
    return node;
}

// method to create function call node
ASTNode* create_func_call_node(char* name, ASTNode** args, int arg_count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_FUNC_CALL;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_OBJECT;
    node->data.func_node.name = name;
    node->checked = 0;
    node->data.func_node.args = malloc(sizeof(ASTNode*) * arg_count);
    for (int i = 0; i < arg_count; i++) {
        node->data.func_node.args[i] = args[i];
    }
    node->data.func_node.arg_count = arg_count;
    return node;
}

// method to create function declaration node
ASTNode* create_func_dec_node(char* name, ASTNode** args, int arg_count, ASTNode* body, char* ret_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_FUNC_DEC;
    node->return_type = &TYPE_VOID;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->static_type = ret_type;
    node->data.func_node.name = name;
    node->checked = 0;
    node->data.func_node.args = malloc(sizeof(ASTNode*) * arg_count);
    for (int i = 0; i < arg_count; i++) {
        node->data.func_node.args[i] = args[i];
    }
    node->data.func_node.arg_count = arg_count;
    node->data.func_node.body = body;
    return node;
}

// method to create let-in node
ASTNode* create_let_in_node(ASTNode** declarations, int dec_count, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_LET_IN;
    node->return_type = &TYPE_OBJECT;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.func_node.name = "";
    node->data.func_node.args = malloc(sizeof(ASTNode*) * dec_count);
    for (int i = 0; i < dec_count; i++) {
        node->data.func_node.args[i] = declarations[i];
    }
    node->data.func_node.arg_count = dec_count;
    node->data.func_node.body = body;
    node->derivations = add_node_list(body, NULL);
    return node;
}

// method to create conditional (if) node
ASTNode* create_conditional_node(ASTNode* condition, ASTNode* body_true, ASTNode* body_false) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_CONDITIONAL;
    node->return_type = &TYPE_OBJECT;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->derivations = add_node_list(body_true, NULL);
    node->derivations = add_node_list(body_false, node->derivations);
    node->data.cond_node.cond = condition;
    node->data.cond_node.body_true = body_true;
    node->data.cond_node.body_false = body_false;
    node->data.cond_node.stm = 1;
    return node;
}

// method to create q-conditional (if?) node
ASTNode* create_q_conditional_node(ASTNode* exp, ASTNode* body_true, ASTNode* body_false) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_Q_CONDITIONAL;
    node->return_type = &TYPE_OBJECT;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->derivations = add_node_list(body_true, NULL);
    node->derivations = add_node_list(body_false, node->derivations);
    node->data.cond_node.cond = exp;
    node->data.cond_node.body_true = body_true;
    node->data.cond_node.body_false = body_false;
    node->data.cond_node.stm = 1;
    return node;
}

// method to create loop node (while loop)
ASTNode* create_loop_node(ASTNode* condition, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_LOOP;
    node->return_type = &TYPE_OBJECT;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.op_node.left = condition;
    node->data.op_node.right = body;
    node->derivations = add_node_list(body, NULL);
    return node;
}

// method to create for loop node
ASTNode* create_for_loop_node(char* var_name, ASTNode** params, ASTNode* body, int count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_FOR_LOOP;
    node->return_type = &TYPE_OBJECT;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.func_node.name = var_name;
    node->checked = 0;
    node->data.func_node.args = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        node->data.func_node.args[i] = params[i];
    }
    node->data.func_node.arg_count = count;
    node->data.func_node.body = body;
    return node;
}

// method to create type testing node or type downcasting node
ASTNode* create_test_casting_type_node(ASTNode* exp, char* type_name, int test) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = test? NODE_TEST_TYPE : NODE_CAST_TYPE;
    node->return_type = test? &TYPE_BOOLEAN : &TYPE_OBJECT;
    node->data.cast_test.type_name = type_name;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.cast_test.exp = exp;
    return node;
}

// method to create type declaration node
ASTNode* create_type_dec_node(
    char* name, ASTNode** params, int param_count,
    char* parent_name, ASTNode** p_params, int p_param_count, ASTNode* body_block
) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_TYPE_DEC;
    node->return_type = &TYPE_VOID;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.type_node.name = name;
    node->data.type_node.parent_name = parent_name;
    // node->data.type_node.parent = &TYPE_OBJECT;
    node->data.type_node.p_args = malloc(sizeof(ASTNode*) * p_param_count);
    for (int i = 0; i < p_param_count; i++) {
        node->data.type_node.p_args[i] = p_params[i];
    }
    node->data.type_node.p_arg_count = p_param_count;
    node->data.type_node.args = malloc(sizeof(ASTNode*) * param_count);
    for (int i = 0; i < param_count; i++) {
        node->data.type_node.args[i] = params[i];
    }
    node->data.type_node.arg_count = param_count;

    int count = body_block->data.program_node.count;
    node->data.type_node.definitions = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        node->data.type_node.definitions[i] = body_block->data.program_node.statements[i];
    }
    node->data.type_node.def_count = count;
    node->data.type_node.id = 0;
    return node;
}

// method to create type instance node
ASTNode* create_type_instance_node(char* name, ASTNode** args, int arg_count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_TYPE_INST;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.type_node.name = name;
    node->data.type_node.args = malloc(sizeof(ASTNode*) * arg_count);
    for (int i = 0; i < arg_count; i++) {
        node->data.type_node.args[i] = args[i];
    }
    node->data.type_node.arg_count = arg_count;

    return node;
}

// method to create type attribute or method getter node
ASTNode* create_attr_getter_node(ASTNode* instance, ASTNode* member) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_TYPE_GET_ATTR;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.op_node.left = instance;
    node->data.op_node.right = member;
    return node;
}

// method to create type attribute setter node
ASTNode* create_attr_setter_node(ASTNode* instance, ASTNode* member, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_TYPE_SET_ATTR;
    node->return_type = &TYPE_OBJECT;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.cond_node.cond = instance;
    node->data.cond_node.body_true = member;
    node->data.cond_node.body_false = value;
    return node;
}

// method to create 'base' function node
ASTNode* create_base_func_node(ASTNode** args, int arg_count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_BASE_FUNC;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_OBJECT;
    node->checked = 0;
    node->data.func_node.args = malloc(sizeof(ASTNode*) * arg_count);
    for (int i = 0; i < arg_count; i++) {
        node->data.func_node.args[i] = args[i];
    }
    node->data.func_node.arg_count = arg_count;
    return node;
}

// method to free an AST node
void free_ast(ASTNode* node) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case NODE_BINARY_OP:
        case NODE_UNARY_OP:
        case NODE_ASSIGNMENT:
        case NODE_D_ASSIGNMENT:
        case NODE_LOOP:
        case NODE_TYPE_GET_ATTR:
            free_ast(node->data.op_node.left);
            free_ast(node->data.op_node.right);
            break;
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (int i = 0; i < node->data.program_node.count; i++) {
                free_ast(node->data.program_node.statements[i]);
            }
            free(node->data.program_node.statements);
            break;
        case NODE_FUNC_CALL:
        case NODE_BASE_FUNC:
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                free_ast(node->data.func_node.args[i]);
            }
            break;
        case NODE_FUNC_DEC:
        case NODE_LET_IN:
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                free_ast(node->data.func_node.args[i]);
            }
            free_ast(node->data.func_node.body);
            break;
        case NODE_CONDITIONAL:
        case NODE_TYPE_SET_ATTR:
            free_ast(node->data.cond_node.body_true);
            free_ast(node->data.cond_node.body_false);
            free_ast(node->data.cond_node.cond);
            break;
        // case NODE_TEST_TYPE:
        // case NODE_CAST_TYPE:
        //     free_ast(node->data.cast_test.exp);
        //     free(node->data.cast_test.type);
        //     break;
        case NODE_TYPE_DEC:
            for (int i = 0; i < node->data.type_node.arg_count; i++) {
                free_ast(node->data.type_node.args[i]);
            }
            for (int i = 0; i < node->data.type_node.def_count; i++) {
                free_ast(node->data.type_node.definitions[i]);
            }
            for (int i = 0; i < node->data.type_node.p_arg_count; i++) {
                free_ast(node->data.type_node.p_args[i]);
            }
            break;
        case NODE_TYPE_INST:
            for (int i = 0; i < node->data.type_node.arg_count; i++) {
                free_ast(node->data.type_node.args[i]);
            }
            break;
        default:
            break;
    }
    destroy_scope(node->scope);
    destroy_context(node->context);
    // free(node);
    node = NULL;
}

// method to print an AST node
void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program:\n");
            for (int i = 0; i < node->data.program_node.count; i++) {
                print_ast(node->data.program_node.statements[i], indent + 1);
            }
            break;
        case NODE_BLOCK:
            printf("Block:\n");
            for (int i = 0; i < node->data.program_node.count; i++) {
                print_ast(node->data.program_node.statements[i], indent + 1);
            }
            break;
        case NODE_FUNC_CALL:
            printf("Function_call: %s, receives:\n", node->data.func_node.name);
            int arg_count = node->data.func_node.arg_count;
            for (int i = 0; i < arg_count; i++) {
                print_ast(node->data.func_node.args[i], indent + 1);
            }
            break;
        case NODE_FUNC_DEC:
            printf("Function_Declaration: %s, returns:\n", node->data.func_node.name);
            print_ast(node->data.func_node.body, indent + 1);
            break;
        case NODE_LET_IN:
            printf("Let-in: %s\n", node->data.func_node.name);
            print_ast(node->data.func_node.body, indent + 1);
            break;
        case NODE_NUMBER:
            printf("Number: %g\n", node->data.number_value);
            break;
        case NODE_STRING:
            printf("String: %s\n", node->data.string_value);
            break;
        case NODE_BOOLEAN:
            printf("Boolean: %s\n", node->data.string_value);
            break;
        case NODE_VARIABLE:
            printf("Variable: %s\n", node->data.variable_name);
            break;
        case NODE_BINARY_OP: {
            const char* op_str;
            switch (node->data.op_node.op) {
                case OP_ADD: op_str = "+"; break;
                case OP_SUB: op_str = "-"; break;
                case OP_MUL: op_str = "*"; break;
                case OP_DIV: op_str = "/"; break;
                case OP_MOD: op_str = "%"; break;
                case OP_POW: op_str = "^"; break;
                case OP_AND: op_str = "&"; break;
                case OP_OR: op_str = "|"; break;
                case OP_CONCAT: op_str = "@"; break;
                case OP_DCONCAT: op_str = "@@"; break;
                case OP_EQ: op_str = "=="; break;
                case OP_NEQ: op_str = "!="; break;
                case OP_GRE: op_str = ">="; break;
                case OP_GR: op_str = ">"; break;
                case OP_LSE: op_str = "<="; break;
                case OP_LS: op_str = "<"; break;
                default: op_str = "?"; break;
            }
            printf("Binary Op: %s\n", op_str);
            print_ast(node->data.op_node.left, indent + 1);
            print_ast(node->data.op_node.right, indent + 1);
            break;
        }
        case NODE_UNARY_OP:
            printf("Unary Op: %s\n", node->data.op_node.op_name);
            print_ast(node->data.op_node.left, indent + 1);
            break;
        case NODE_ASSIGNMENT:
        case NODE_D_ASSIGNMENT:
            printf("Assignment:\n");
            print_ast(node->data.op_node.left, indent + 1);
            print_ast(node->data.op_node.right, indent + 1);
            break;
        case NODE_CONDITIONAL:
            printf("Conditional:\n");
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("Condition:\n");
            print_ast(node->data.cond_node.cond, indent + 2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("If part:\n");
            print_ast(node->data.cond_node.body_true, indent + 2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("else part:\n");
            print_ast(node->data.cond_node.body_false, indent + 2);
            break;
        case NODE_LOOP:
            printf("Loop:\n");
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("Condition:\n");
            print_ast(node->data.op_node.left, indent + 2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("Body:\n");
            print_ast(node->data.op_node.right, indent + 2);
            break;
        case NODE_TEST_TYPE:
            printf("IS %s\n", node->data.cast_test.type_name);
            print_ast(node->data.cast_test.exp, indent + 1);
            break;
        case NODE_CAST_TYPE:
            printf("AS %s\n", node->data.cast_test.type_name);
            print_ast(node->data.cast_test.exp, indent + 1);
            break;
        case NODE_TYPE_DEC:
            printf("Type_Declaration: %s, inherits %s\n",
                node->data.type_node.name,
                node->data.type_node.parent_name
            );
            
            for (int i = 0; i < node->data.type_node.def_count; i++) {
                print_ast(node->data.type_node.definitions[i], indent + 1);
            }
            
            break;
        case NODE_TYPE_INST:
            printf("Type_instance: %s, receives:\n", node->data.type_node.name);
            arg_count = node->data.type_node.arg_count;
            for (int i = 0; i < arg_count; i++) {
                print_ast(node->data.type_node.args[i], indent + 1);
            }
            break;
        case NODE_TYPE_GET_ATTR:
            printf("Type member: \n");
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf(" Instance:\n");
            print_ast(node->data.op_node.left, indent + 2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf(" Member:\n");
            print_ast(node->data.op_node.right, indent + 2);
            break;
        case NODE_TYPE_SET_ATTR:
            printf("Assignment:\n");
            print_ast(node->data.cond_node.body_true, indent + 1);
            print_ast(node->data.cond_node.body_false, indent + 1);
            break;
        case NODE_BASE_FUNC:
            printf("Base call to %s:\n", node->data.func_node.name);
            arg_count = node->data.func_node.arg_count;
            for (int i = 0; i < arg_count; i++) {
                print_ast(node->data.func_node.args[i], indent + 1);
            }
            break;
    }
}