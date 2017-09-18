//
//  orbit/parser/ast.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-09-16.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_ast_h
#define orbit_ast_h

#include <orbit/parser/tokens.h>
#include <orbit/utils/platforms.h>

typedef struct _ASTNode ASTNode;
typedef enum _ASTNodeType ASTNodeType;

enum _ASTNodeType {
    
    // Statements + Blocks
    AST_MODULE,
    AST_BLOCK,
    AST_CONDITIONAL,
    AST_LOOP,
    
    // Declarations
    AST_FUNC_DECL,
    AST_VAR_DECL,
    AST_PARAM_DECL,
    AST_STRUCT_DECL,
    
    // Used for expressions
    AST_LITERAL,
    AST_UNARY,
    AST_BINARY,
    AST_CALL,
    AST_SUBSCRIPT,
};

/// Base type for all Syntax Tree nodes, used as parent "class"
struct _ASTNode {
    ASTNode*    parent;
    ASTNodeType type;
};

/// The Tree representation of a module (one orbit file)
typedef struct _ASTModule {
    const char* symbol;
    
    uint16_t    declarationCount;
    ASTNode*    declarations[ORBIT_FLEXIBLE_ARRAY_MEMB]
} ASTModule;

/// A list of other nodes, used for code blocks.
typedef struct _ASTBlock {
    uint16_t    statementCount;
    ASTNode*    statements[ORBIT_FLEXIBLE_ARRAY_MEMB];
} ASTBlock;

/// Used for if/else statements, links to the expression, the if and else blocks.
typedef struct _ASTConditional {
    ASTNode     base;
    
    ASTNode*    statement;
    ASTNode*    ifBlock;
    ASTNode*    elseBlock;
} ASTConditional;

/// TODO: split into a node for each loop type, + for-in loop support
typedef struct _ASTLoop {
    ASTNode     base;
    
    ASTNode*    statement;
    ASTNode*    block;
} ASTLoop;

/// Node representing a function declaration. Points to the signature (params, return)
/// and the function's code block.
typedef struct _ASTFuncDecl {
    ASTNode     base;
    
    OCToken     symbol;
    OCToken     returnType;
    ASTBlock*   body;
    
    uint8_t     paramCount;
    ASTNode*    params[ORBIT_FLEXIBLE_ARRAY_MEMB]
} ASTFuncDecl;

typedef struct _ASTVarDecl {
    ASTNode     base;
    
    OCToken     symbol;
    // TODO: move to the compiler-side type system
    OCToken     typeAnnotation;
} ASTVarDecl;

typedef struct _ASTParamDecl {
    ASTNode     base;
    
    OCToken     symbol;
    OCToken     typeAnnotation;
} ASTParamDecl;

typedef struct _ASTStructDecl {
    ASTNode     base;
    
    OCToken     symbol;
    ASTNode*    constructor;
    ASTNode*    destructor;
    
    uint16_t    fieldCount;
    ASTNode*    fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
} ASTStructDecl;

typedef struct _ASTUnaryOp {
    ASTNode     base;
    
    OCToken     operator;
    ASTNode*    expression;
} ASTUnaryOp;

typedef struct _ASTBinaryOp {
    ASTNode     base;
    
    OCToken     operator;
    ASTNode*    left;
    ASTNode*    right;
} ASTBinaryOp;

typedef struct _ASTFuncCall {
    ASTNode     base;
    
    OCToken     symbol;
    uint8_t     paramCount;
    ASTNode*    params[ORBIT_FLEXIBLE_ARRAY_MEMB];
} ASTFuncCall;

typedef struct _ASTSubscript {
    ASTNode     base;
    
    OCToken     symbol;
    ASTNode*    expression;
} ASTSubscript;

typedef struct _ASTLiteral {
    ASTNode     base;
    OCToken     symbol;
} ASTLiteral;

#endif /* orbit_ast_h_ */
