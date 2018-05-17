//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast.h
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_ast_h
#define orbit_ast_ast_h

#include <stdio.h>
#include <stdint.h>
#include <orbit/csupport/tokens.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _OrbitAST OrbitAST;
typedef struct _OrbitASTType OrbitASTType;
typedef enum _OrbitASTTypeFlags OrbitASTTypeFlags;
typedef enum _ASTKind ASTKind;

// Tag enum for AST nodes
enum _ASTKind {
    ORBIT_AST_CONDITIONAL             = 1 << 0,
    ORBIT_AST_FOR_IN                  = 1 << 1,
    ORBIT_AST_WHILE                   = 1 << 2,
    ORBIT_AST_BREAK                   = 1 << 3,
    ORBIT_AST_CONTINUE                = 1 << 4,
    ORBIT_AST_RETURN                  = 1 << 5,
    ORBIT_AST_BLOCK                   = 1 << 6,
    
    ORBIT_AST_DECL_MODULE             = 1 << 7,
    ORBIT_AST_DECL_FUNC               = 1 << 8,
    ORBIT_AST_DECL_VAR                = 1 << 9,
    ORBIT_AST_DECL_STRUCT             = 1 << 10,
    
    ORBIT_AST_EXPR_UNARY              = 1 << 11,
    ORBIT_AST_EXPR_BINARY             = 1 << 12,
    ORBIT_AST_EXPR_CALL               = 1 << 13,
    ORBIT_AST_EXPR_SUBSCRIPT          = 1 << 14,
    ORBIT_AST_EXPR_CONSTANT           = 1 << 15,
    ORBIT_AST_EXPR_CONSTANT_INTEGER   = 1 << 16,
    ORBIT_AST_EXPR_CONSTANT_FLOAT     = 1 << 17,
    ORBIT_AST_EXPR_CONSTANT_STRING    = 1 << 18,
    ORBIT_AST_EXPR_NAME               = 1 << 19,
    
    ORBIT_AST_TYPEEXPR_VOID           = 1 << 20,
    ORBIT_AST_TYPEEXPR_BOOL           = 1 << 21,
    ORBIT_AST_TYPEEXPR_NUMBER         = 1 << 22,
    ORBIT_AST_TYPEEXPR_STRING         = 1 << 23,
    ORBIT_AST_TYPEEXPR_USER           = 1 << 24,
    ORBIT_AST_TYPEEXPR_ARRAY          = 1 << 25,
    ORBIT_AST_TYPEEXPR_MAP            = 1 << 26,
    ORBIT_AST_TYPEEXPR_FUNC           = 1 << 27,
    ORBIT_AST_TYPEEXPR_ANY            = 1 << 28,
};

extern const uint32_t ASTAllMask;
extern const uint32_t ASTStmtMask;
extern const uint32_t ASTDeclMask;
extern const uint32_t ASTExprMask;
extern const uint32_t ASTTypeExprMask;

#define ORBIT_AST_IS_STMT(ast) ((ast) != NULL && ((ast).type & ASTStmtMask) != 0)
#define ORBIT_AST_IS_DECL(ast) ((ast) != NULL && ((ast).type & ASTDeclMask) != 0)
#define ORBIT_AST_IS_EXPR(ast) ((ast) != NULL && ((ast).type & ASTExprMask) != 0)
#define ORBIT_AST_IS_TYPEEXPR(ast) ((ast) != NULL && ((ast).type & ASTTypeExprMask) != 0)

enum _OrbitASTTypeFlags {
    TYPE_CONST      = 1 << 0,
    TYPE_OPTIONAL   = 1 << 1
};

struct _OrbitASTType {
    //OrbitAST*       canonicalType;
    OrbitASTTypeFlags   flags;
    
    union {
        struct {
            OrbitAST*   elementType;
        } arrayType;
    
        struct {
            OrbitAST*   keyType;
            OrbitAST*   elementType;
        } mapType;
    
        struct {
            OrbitAST*   returnType;
            OrbitAST*   params;
        } funcType;
        
        struct {
            //OrbitToken     symbol;
            OCStringID  symbol;
        } userType;
    };
};

// The TUD (Tagged Union of Doom). Represents all possible nodes in an orbit
// AST. AST::next is used to represent "same level" collections (for example,
// a list of parameters, or a list of expressions.
struct _OrbitAST {
    ORCObject       super;
    ASTKind         kind;
    OrbitAST*       next;
    OrbitAST*       type;
    
    OCSourceRange   sourceRange;
    
    union {
        // --------------------------------------------------------------------
        // Statements
        // --------------------------------------------------------------------
        struct {
            OrbitAST*   condition;
            OrbitAST*   ifBody;
            OrbitAST*   elseBody;
        } conditionalStmt;
        
        struct {
            OrbitToken     variable;
            OrbitAST*   collection;
            OrbitAST*   body;
        } forInLoop;
        
        struct {
            OrbitAST*   condition;
            OrbitAST*   body;
        } whileLoop;
        
        struct {
            OrbitAST*   body;
        } block;
        
        struct {
            OrbitAST*   returnValue;
        } returnStmt;
        
        // --------------------------------------------------------------------
        // Declarations
        // --------------------------------------------------------------------
        struct {
            OCStringID  symbol;
            OrbitAST*   body;
        } moduleDecl;
        
        struct {
            OrbitToken     symbol;
            OCStringID  name;
            OCStringID  mangledName;
            OrbitAST*   returnType;
            OrbitAST*   params;
            OrbitAST*   body;
        } funcDecl;
        
        struct {
            OrbitToken     symbol;
            OCStringID  name;
            OrbitAST*   typeAnnotation;
        } varDecl;
        
        struct {
            OrbitToken     symbol;
            OCStringID  name;
            OrbitAST*   constructor;
            OrbitAST*   destructor;
            OrbitAST*   fields;
        } structDecl;
        
        // --------------------------------------------------------------------
        // Expressions
        // --------------------------------------------------------------------
        struct  {
            OrbitToken     operator;
            OrbitAST*   rhs;
        } unaryExpr;
        
        struct {
            OrbitToken     operator;
            OrbitAST*   lhs;
            OrbitAST*   rhs;
        } binaryExpr;
        
        struct {
            OrbitAST*   symbol;
            OrbitAST*   params;
        } callExpr;
        
        struct {
            OrbitAST*   symbol;
            OrbitAST*   subscript;
        } subscriptExpr;
        
        struct {
            OrbitToken     symbol;
        } constantExpr;
        
        struct {
            OrbitToken     symbol;
            OCStringID  name;
        } nameExpr;
        
        // Type Expressions (necessary for a non-trivial type system)
        
        OrbitASTType    typeExpr;
        
    };
};

void orbit_astPrint(FILE* out, OrbitAST* ast);
void orbit_astDestroy(void* ref);
OrbitAST* orbit_astMake(ASTKind kind);

#endif /* orbit_ast_ast_h */
