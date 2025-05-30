#ifndef VISITOR_H
#define VISITOR_H

#include "../ast/ast.h"
#include "../scope/scope.h"
#include "../type/type.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct Visitor Visitor;

typedef void (*VisitProgram)(Visitor*, ASTNode*);
typedef void (*VisitNumber)(Visitor*, ASTNode*);
typedef void (*VisitString)(Visitor*, ASTNode*);
typedef void (*VisitBoolean)(Visitor*, ASTNode*);
typedef void (*VisitVariable)(Visitor*, ASTNode*);
typedef void (*VisitBinaryOp)(Visitor*, ASTNode*);
typedef void (*VisitUnaryOp)(Visitor*, ASTNode*);
typedef void (*VisitAssignment)(Visitor*, ASTNode*);
typedef void (*VisitFuncCall)(Visitor*, ASTNode*);
typedef void (*VisitBlock)(Visitor*, ASTNode*);
typedef void (*VisitFuncDec)(Visitor*, ASTNode*);
typedef void (*VisitLetIn)(Visitor*, ASTNode*);
typedef void (*VisitConditional)(Visitor*, ASTNode*);
typedef void (*VisitLoop)(Visitor*, ASTNode*);
typedef void (*VisitTypeDec)(Visitor*, ASTNode*);
typedef void (*VisitTypeInst)(Visitor*, ASTNode*);
typedef void (*VisitCastingType)(Visitor*, ASTNode*);
typedef void (*VisitTestType)(Visitor*, ASTNode*);
typedef void (*VisitAttrGetter)(Visitor*, ASTNode*);
typedef void (*VisitAttrSetter)(Visitor*, ASTNode*);

struct Visitor {
    int error_count;
    
    VisitProgram visit_program;
    VisitNumber visit_number;
    VisitString visit_string;
    VisitBoolean visit_boolean;
    VisitVariable visit_variable;
    VisitBinaryOp visit_binary_op;
    VisitUnaryOp visit_unary_op;
    VisitAssignment visit_assignment;
    VisitFuncCall visit_function_call;
    VisitBlock visit_block;
    VisitFuncDec visit_function_dec;
    VisitLetIn visit_let_in;
    VisitConditional visit_conditional;
    VisitLoop visit_loop;
    VisitTypeDec visit_type_dec;
    VisitTypeInst visit_type_inst;
    VisitCastingType visit_casting_type;
    VisitTestType visit_test_type;
    VisitAttrGetter visit_attr_getter;
    VisitAttrSetter visit_attr_setter;

    char** errors;
};

void accept(Visitor* visitor, ASTNode* node);
void get_context(Visitor* visitor, ASTNode* node);
void report_error(Visitor* v, const char* fmt, ...);
void free_error(char** array, int count);

#endif