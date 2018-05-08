//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast.h
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_h
#define orbit_ast_h

#include <stdio.h>
#include <stdint.h>
#include <orbit/csupport/tokens.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _AST AST;
typedef struct _ASTType ASTType;
typedef enum _ASTTypeFlags ASTTypeFlags;
typedef enum _ASTKind ASTKind;

// Tag enum for AST nodes
enum _ASTKind {
    AST_CONDITIONAL             = 1 << 0,
    AST_FOR_IN                  = 1 << 1,
    AST_WHILE                   = 1 << 2,
    AST_BREAK                   = 1 << 3,
    AST_CONTINUE                = 1 << 4,
    AST_RETURN                  = 1 << 5,
    AST_BLOCK                   = 1 << 6,
    
    AST_DECL_MODULE             = 1 << 7,
    AST_DECL_FUNC               = 1 << 8,
    AST_DECL_VAR                = 1 << 9,
    AST_DECL_STRUCT             = 1 << 10,
    
    AST_EXPR_UNARY              = 1 << 11,
    AST_EXPR_BINARY             = 1 << 12,
    AST_EXPR_CALL               = 1 << 13,
    AST_EXPR_SUBSCRIPT          = 1 << 14,
    AST_EXPR_CONSTANT           = 1 << 15,
    AST_EXPR_CONSTANT_INTEGER   = 1 << 16,
    AST_EXPR_CONSTANT_FLOAT     = 1 << 17,
    AST_EXPR_CONSTANT_STRING    = 1 << 18,
    AST_EXPR_NAME               = 1 << 19,
    
    AST_TYPEEXPR_VOID           = 1 << 20,
    AST_TYPEEXPR_BOOL           = 1 << 21,
    AST_TYPEEXPR_NUMBER         = 1 << 22,
    AST_TYPEEXPR_STRING         = 1 << 23,
    AST_TYPEEXPR_USER           = 1 << 24,
    AST_TYPEEXPR_ARRAY          = 1 << 25,
    AST_TYPEEXPR_MAP            = 1 << 26,
    AST_TYPEEXPR_FUNC           = 1 << 27,
    AST_TYPEEXPR_ANY            = 1 << 28,
};

extern const uint32_t ASTAllMask;
extern const uint32_t ASTStmtMask;
extern const uint32_t ASTDeclMask;
extern const uint32_t ASTExprMask;
extern const uint32_t ASTTypeExprMask;

#define AST_IS_STMT(ast) ((ast) != NULL && ((ast).type & ASTStmtMask) != 0)
#define AST_IS_DECL(ast) ((ast) != NULL && ((ast).type & ASTDeclMask) != 0)
#define AST_IS_EXPR(ast) ((ast) != NULL && ((ast).type & ASTExprMask) != 0)
#define AST_IS_TYPEEXPR(ast) ((ast) != NULL && ((ast).type & ASTTypeExprMask) != 0)

enum _ASTTypeFlags {
    TYPE_CONST      = 1 << 0,
    TYPE_OPTIONAL   = 1 << 1
};

struct _ASTType {
    //AST*            canonicalType;
    ASTTypeFlags    flags;
    
    union {
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
        
        struct {
            //OCToken     symbol;
            OCStringID  symbol;
        } userType;
    };
};

// The TUD (Tagged Union of Doom). Represents all possible nodes in an orbit
// AST. AST::next is used to represent "same level" collections (for example,
// a list of parameters, or a list of expressions.
struct _AST {
    ORCObject       super;
    ASTKind         kind;
    AST*            next;
    AST*            type;
    
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
            OCToken     variable;
            AST*        collection;
            AST*        body;
        } forInLoop;
        
        struct {
            AST*        condition;
            AST*        body;
        } whileLoop;
        
        struct {
            AST*        body;
        } block;
        
        struct {
            AST*        returnValue;
        } returnStmt;
        
        // --------------------------------------------------------------------
        // Declarations
        // --------------------------------------------------------------------
        struct {
            OCStringID  symbol;
            AST*        body;
        } moduleDecl;
        
        struct {
            OCToken     symbol;
            OCStringID  name;
            OCStringID  mangledName;
            AST*        returnType;
            AST*        params;
            AST*        body;
        } funcDecl;
        
        struct {
            OCToken     symbol;
            OCStringID  name;
            AST*        typeAnnotation;
        } varDecl;
        
        struct {
            OCToken     symbol;
            OCStringID  name;
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
            OCStringID  name;
        } nameExpr;
        
        // Type Expressions (necessary for a non-trivial type system)
        
        ASTType         typeExpr;
        
    };
};

extern const AST ast_voidType;
extern const AST ast_boolType;
extern const AST ast_numberType;
extern const AST ast_stringType;

void ast_print(FILE* out, AST* ast);
void ast_destroy(void* ref);
AST* ast_makeNode(ASTKind kind);

#endif /* orbit_ast_h_ */
