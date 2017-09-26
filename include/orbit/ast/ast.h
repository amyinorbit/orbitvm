//
//  orbit/parser/ast.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-09-16.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_ast_h
#define orbit_ast_h

#include <stdarg.h>
#include <orbit/parser/tokens.h>
#include <orbit/utils/platforms.h>

typedef struct _AST AST;
typedef enum _ASTType ASTType;

// Tag enum for AST nodes
enum _ASTType {
    AST_CONDITIONAL,
    AST_FOR_IN,
    AST_WHILE,
    AST_DECL_MODULE,
    AST_DECL_FUNC,
    AST_DECL_VAR,
    AST_DECL_PARAM,
    AST_DECL_STRUCT,
    AST_EXPR_UNARY,
    AST_EXPR_BINARY,
    AST_EXPR_CALL, // Also does subscripts. Uh. maybe
    AST_EXPR_CONSTANT,
    AST_EXPR_VARIABLE,
    AST_EXPR_TYPE,
};

// The TUD (Tagged Union of Doom). Represents all possible nodes in an orbit
// AST. AST::next is used to represent "same level" collections (for example,
// a list of parameters, or a list of expressions.
struct _AST {
    ASTType         type;
    AST*            next;
    
    union {
        
        // --------------------------------------------------------------------
        // Statements
        // --------------------------------------------------------------------
        struct {
            AST*        condition;
            AST*        ifBody;
            AST*        elseBody;
        } conditionalStmt;
        
        struct {
            AST*        variable;
            AST*        collection;
            AST*        body;
        } forInLoop;
        
        struct {
            AST*        condition;
            AST*        body;
        } whileLoop;
        
        // --------------------------------------------------------------------
        // Declarations
        // --------------------------------------------------------------------
        struct {
            char*       symbol;
            AST*        body;
        } moduleDecl;
        
        struct {
            OCToken*    symbol;
            AST*        returnType;
            AST*        params;
            AST*        body;
        } funcDecl;
        
        struct {
            OCToken*    symbol;
            AST*        typeAnnotation;
        } varDecl;
        
        struct {
            OCToken*    symbol;
            AST*        typeAnnotation;
        } paramDecl;
        
        struct {
            OCToken*    symbol;
            AST*        constructor;
            AST*        destructor;
            AST*        fields;
        } structDecl;
        
        // --------------------------------------------------------------------
        // Expressions
        // --------------------------------------------------------------------
        struct  {
            OCToken*    operator;
            AST*        rhs;
        } unaryExpr;
        
        struct {
            OCToken*    operator;
            AST*        lhs;
            AST*        rhs;
        } binaryExpr;
        
        struct {
            OCToken*    symbol;
            AST*        params;
        } callExpr;
        
        struct {
            OCToken*    symbol;
        } constantExpr;
        
        struct {
            OCToken*    symbol;
        } variableExpr;
        
        struct {
            OCToken*    symbol; // TODO: Replace with smth better (multi-token types)
        } typeExpr;
    };
};

#define AST_IS_TYPE(node, type) (((AST*)node)->type == type)

void ast_print(AST* ast);

void ast_destroy(AST* ast);

AST* ast_makeBinaryExpr(const OCToken* operator, AST* lhs, AST* rhs);

AST* ast_makeUnaryExpr(const OCToken* operator, AST* rhs);

AST* ast_makeCallExpr(const OCToken* symbol, int argCount, ...);

AST* ast_makeVariableExpr(const OCToken* symbol);

AST* ast_makeConstantExpr(const OCToken* symbol);

AST* ast_makeTypExpr(const OCToken* symbol);

#endif /* orbit_ast_h_ */
