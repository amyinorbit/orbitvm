//
//  orbit/ast/ast.h
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-16.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_ast_h
#define orbit_ast_h

#include <stdio.h>
#include <orbit/parser/tokens.h>
#include <orbit/utils/platforms.h>

typedef struct _AST AST;
typedef enum _ASTType ASTType;

// Tag enum for AST nodes
enum _ASTType {
    AST_CONDITIONAL,
    AST_FOR_IN,
    AST_WHILE,
    AST_BREAK,
    AST_CONTINUE,
    AST_RETURN,
    
    AST_DECL_MODULE,
    AST_DECL_FUNC,
    AST_DECL_VAR,
    AST_DECL_STRUCT,
    
    AST_EXPR_UNARY,
    AST_EXPR_BINARY,
    AST_EXPR_CALL,
    AST_EXPR_SUBSCRIPT,
    AST_EXPR_CONSTANT,
    AST_EXPR_CONSTANT_INTEGER,
    AST_EXPR_CONSTANT_FLOAT,
    AST_EXPR_CONSTANT_STRING,
    AST_EXPR_NAME,
    //AST_EXPR_TYPE,
    
    
    AST_TYPEEXPR_SIMPLE,
    AST_TYPEEXPR_ARRAY,
    AST_TYPEEXPR_MAP,
    AST_TYPEEXPR_FUNC,
};

// The TUD (Tagged Union of Doom). Represents all possible nodes in an orbit
// AST. AST::next is used to represent "same level" collections (for example,
// a list of parameters, or a list of expressions.
struct _AST {
    ASTType         type;
    AST*            next;
    
    union {
        
        struct {
            AST*        head;
        } list;
        
        // --------------------------------------------------------------------
        // Statements
        // --------------------------------------------------------------------
        struct {
            AST*        condition;
            AST*        ifBody;
            AST*        elseBody;
        } conditionalStmt;
        
        struct {
            OCToken     variable;
            AST*        collection;
            AST*        body;
        } forInLoop;
        
        struct {
            AST*        condition;
            AST*        body;
        } whileLoop;
        
        struct {
            AST*        returnValue;
        } returnStmt;
        
        // --------------------------------------------------------------------
        // Declarations
        // --------------------------------------------------------------------
        struct {
            const char* symbol;
            AST*        body;
        } moduleDecl;
        
        struct {
            OCToken     symbol;
            AST*        returnType;
            AST*        params;
            AST*        body;
        } funcDecl;
        
        struct {
            OCToken     symbol;
            AST*        typeAnnotation;
        } varDecl;
        
        struct {
            OCToken     symbol;
            AST*        constructor;
            AST*        destructor;
            AST*        fields;
        } structDecl;
        
        // --------------------------------------------------------------------
        // Expressions
        // --------------------------------------------------------------------
        struct  {
            OCToken     operator;
            AST*        rhs;
        } unaryExpr;
        
        struct {
            OCToken     operator;
            AST*        lhs;
            AST*        rhs;
        } binaryExpr;
        
        struct {
            AST*        symbol;
            AST*        params;
        } callExpr;
        
        struct {
            AST*        symbol;
            AST*        subscript;
        } subscriptExpr;
        
        struct {
            OCToken     symbol;
        } constantExpr;
        
        struct {
            OCToken     symbol;
        } nameExpr;
        
        // Type Expressions (necessary for a non-trivial type system)
        
        struct {
            OCToken     symbol;
        } simpleType;
        
        struct {
            AST*        elementType;
        } arrayType;
        
        struct {
            AST*        keyType;
            AST*        elementType;
        } mapType;
        
        struct {
            AST*        returnType;
            AST*        params;
        } funcType;
        
    };
};

void ast_print(FILE* out, AST* ast);
void ast_destroy(AST* ast);
AST* ast_makeNode(ASTType type);

#endif /* orbit_ast_h_ */
