#ifndef LLVM_VISITOR
#define LLVM_VISITOR

#include "../ast/ast.h"
#include <string.h>
#include <llvm-c/Core.h>

typedef struct LLVM_Visitor LLVM_Visitor;

typedef LLVMValueRef (*GenerateProgram)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateNumber)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateString)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateBoolean)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateVariable)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateBinaryOp)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateUnaryOp)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateAssignment)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateBultinFunc)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateBlock)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateFuncDec)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateLetIn)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateConditional)(LLVM_Visitor*, ASTNode*);
typedef LLVMValueRef (*GenerateLoop)(LLVM_Visitor*, ASTNode*);

struct LLVM_Visitor {
    GenerateProgram visit_program;
    GenerateNumber visit_number;
    GenerateString visit_string;
    GenerateBoolean visit_boolean;
    GenerateVariable visit_variable;
    GenerateBinaryOp visit_binary_op;
    GenerateUnaryOp visit_unary_op;
    GenerateAssignment visit_assignment;
    GenerateBultinFunc visit_function_call;
    GenerateBlock visit_block;
    GenerateFuncDec visit_function_dec;
    GenerateLetIn visit_let_in;
    GenerateConditional visit_conditional;
    GenerateLoop visit_loop;
};

LLVMValueRef accept_gen(LLVM_Visitor* visitor, ASTNode* node);

#endif
