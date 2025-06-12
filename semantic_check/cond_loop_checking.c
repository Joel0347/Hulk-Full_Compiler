#include "semantic.h"

void visit_conditional(Visitor* v, ASTNode* node) {
    ASTNode* condition = node->data.cond_node.cond;
    ASTNode* true_body = node->data.cond_node.body_true;
    ASTNode* false_body = node->data.cond_node.body_false;

    condition->context->parent = node->context;
    true_body->context->parent = node->context;
    condition->scope->parent = node->scope;
    true_body->scope->parent = node->scope;

    if (false_body) {
        false_body->context->parent = node->context;
        false_body->scope->parent = node->scope;
    }

    accept(v, condition);

    if (unify_member(v, condition, &TYPE_BOOLEAN)) {
        accept(v, condition);
    }

    Type* condition_type = find_type(condition);

    if (!type_equals(condition_type, &TYPE_ERROR) &&
        !type_equals(condition_type, &TYPE_BOOLEAN)
    ) {
        report_error(
            v, "Condition in 'if' expression must return "
            "'Boolean', not '%s'. Line: %d", 
            condition_type->name, condition->line
        );
    }

    if (true_body->type == NODE_CONDITIONAL || true_body->type == NODE_Q_CONDITIONAL) {
        true_body->data.cond_node.stm = node->data.cond_node.stm;
    }

    if (false_body && (false_body->type == NODE_CONDITIONAL || true_body->type == NODE_Q_CONDITIONAL)) {
        false_body->data.cond_node.stm = node->data.cond_node.stm;
    }

    accept(v, true_body);
    Type* true_type = find_type(true_body);
    Type* false_type = NULL;

    if (false_body) {
        accept(v, false_body);
        accept(v, true_body);
        false_type = find_type(false_body);
        true_type = find_type(true_body);
    } else if (
        is_builtin_type(true_type) &&
        !type_equals(true_type, &TYPE_OBJECT)
    ) {
        false_type = find_type(true_body);
    } else {
        false_type = &TYPE_NULL;
    }

    if (!node->data.cond_node.stm && 
        (type_equals(true_type, &TYPE_VOID) || type_equals(false_type, &TYPE_VOID))
    ) {
        report_error(
            v, "Possible 'Void' value assigned to a variable. Line: %d", 
            node->line
        );
    }

    node->return_type = get_lca(true_type, false_type);
}

void visit_q_conditional(Visitor* v, ASTNode* node) {
    ASTNode* exp = node->data.cond_node.cond;
    ASTNode* null_body = node->data.cond_node.body_true;
    ASTNode* ok_body = node->data.cond_node.body_false;

    exp->context->parent = node->context;
    null_body->context->parent = node->context;
    exp->scope->parent = node->scope;
    null_body->scope->parent = node->scope;
    ok_body->context->parent = node->context;
    ok_body->scope->parent = node->scope;

    accept(v, exp);

    if (type_equals(exp->return_type, &TYPE_ERROR)) {
        node->return_type = &TYPE_ERROR;
        return;
    }

    Symbol* s = NULL;
    if (exp->type == NODE_TYPE_GET_ATTR) {
        Type* t = find_type(exp->data.op_node.left);
        int attr = exp->data.op_node.right->type == NODE_VARIABLE;

        if (attr) {
            s = get_type_attr(t, exp->data.op_node.right->data.variable_name);
        } else {
            report_error(
                v, "Clause 'if?' is still in trial period. It only accepts variables as arguments. Line: %d", 
                exp->line
            );
            node->return_type = &TYPE_ERROR;
            return;
        }
    } else {
        s = find_symbol(node->scope, exp->data.variable_name);
    }

    Type* exp_type = s->type;
    s->type = &TYPE_NULL;
    accept(v, null_body);
    s->type = exp_type->sub_type? exp_type->sub_type : exp_type;
    int any = type_equals(s->type, &TYPE_ANY);
    accept(v, ok_body);
    exp_type = any? s->type : exp_type;
    s->type = &TYPE_NULL;
    accept(v, null_body);
    s->type = exp_type;

    Type* ok_type = find_type(ok_body);
    Type* null_type = find_type(null_body);

    node->return_type = get_lca(ok_type, null_type);
}

void visit_loop(Visitor* v, ASTNode* node) {
    ASTNode* condition = node->data.op_node.left;
    ASTNode* body = node->data.op_node.right;

    condition->context->parent = node->context;
    body->context->parent = node->context;
    condition->scope->parent = node->scope;
    body->scope->parent = node->scope;

    accept(v, condition);

    if (unify_member(v, condition, &TYPE_BOOLEAN)) {
        accept(v, condition);
    }

    Type* condition_type = find_type(condition);

    if (!type_equals(condition_type, &TYPE_ERROR) &&
        !type_equals(condition_type, &TYPE_BOOLEAN)
    ) {
        report_error(
            v, "Condition in 'while' expression must return "
            "'Boolean', not '%s'. Line: %d", 
            condition_type->name, condition->line
        );
    }

    accept(v, body);
    node->return_type = find_type(body);
}

void visit_for_loop(Visitor* v, ASTNode* node) {
    ASTNode** args = node->data.func_node.args;
    ASTNode* body = node->data.func_node.body;
    char* name = node->data.func_node.name;
    int count = node->data.func_node.arg_count;

    if (!count || count > 2) {
        report_error(
            v, "Function 'range' receives 1 or 2 arguments, not %d. Line: %d", 
            count, node->line
        );
    }

    count = (count <= 2)? count : 2;
    ASTNode* start = (count > 1)? args[0] : create_number_node(0);
    ASTNode* end = (count > 1)? args[1] : ((count > 0)? args[0] : create_number_node(0));

    for (int i = 0; i < count; i++) {
        args[i]->scope->parent = node->scope;
        args[i]->context->parent = node->context;
        accept(v, args[i]);
        Type* t = find_type(args[i]);

        // if (type_equals(t, &TYPE_ANY) && unify_member(v, args[i], &TYPE_NUMBER)) {
        //     accept(v, args[i]);
        //     t = find_type(args[i]);
        // }

        if (!type_equals(t, &TYPE_ANY) && !type_equals(t, &TYPE_NUMBER)) {
            args[i]->return_type = &TYPE_ERROR;
            report_error(
                v, "Function 'range' receives 'Number', not '%s' as argument %d. Line: %d", 
                t->name, i + 1, node->line
            );
        }
    }

    ASTNode** internal_decs = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
    internal_decs[0] = create_assignment_node(
        name, create_variable_node("_iter", "", 0), "", NODE_ASSIGNMENT
    );
    ASTNode* internal_let = create_let_in_node(internal_decs, 1, body);
    ASTNode* iter_next = create_assignment_node(
        "_iter", create_binary_op_node(
            OP_ADD, "+", create_variable_node("_iter", "", 0), create_number_node(1), &TYPE_NUMBER
        ),
        "", NODE_D_ASSIGNMENT
    );
    ASTNode* condition = create_binary_op_node(OP_LS, "<", iter_next, end, &TYPE_BOOLEAN);
    ASTNode* _while =  create_conditional_node(
        condition, create_loop_node(condition, internal_let), NULL
    );

    node->type = NODE_LET_IN;
    node->return_type = &TYPE_OBJECT;
    node->data.func_node.name = "";
    node->data.func_node.args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
    node->data.func_node.args[0] = create_assignment_node(
        "_iter", create_binary_op_node(
            OP_SUB, "-", start, create_number_node(1), &TYPE_NUMBER
        ), "", NODE_ASSIGNMENT
    );
    node->data.func_node.arg_count = 1;
    node->data.func_node.body = _while;
    node->derivations = add_value_list(_while, NULL);

    accept(v, node);
}