//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast.h
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
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
typedef uint64_t ASTKind;

#define DECL_AST_KIND(name, num) static const ASTKind ORBIT_AST_##name = 1 << (num)

// We can't use enum because C restricts them to 32 bit. Could probably not rely on bitsets,
// but that allows much faster pattern matching in AST visitors.
DECL_AST_KIND(CONDITIONAL, 0);
DECL_AST_KIND(FOR_IN, 1);
DECL_AST_KIND(WHILE, 2);
DECL_AST_KIND(BREAK, 3);
DECL_AST_KIND(CONTINUE, 4);
DECL_AST_KIND(RETURN, 5);
DECL_AST_KIND(BLOCK, 6);
DECL_AST_KIND(DECL_MODULE, 7);
DECL_AST_KIND(DECL_FUNC, 8);
DECL_AST_KIND(DECL_VAR, 9);
DECL_AST_KIND(DECL_STRUCT, 10);
DECL_AST_KIND(EXPR_UNARY, 11);
DECL_AST_KIND(EXPR_BINARY, 12);
DECL_AST_KIND(EXPR_CALL, 13);
DECL_AST_KIND(EXPR_SUBSCRIPT, 14);
DECL_AST_KIND(EXPR_CONSTANT, 15);
DECL_AST_KIND(EXPR_CONSTANT_INTEGER, 16);
DECL_AST_KIND(EXPR_CONSTANT_FLOAT, 17);
DECL_AST_KIND(EXPR_CONSTANT_STRING, 18);
DECL_AST_KIND(EXPR_LAMBDA, 19);
DECL_AST_KIND(EXPR_NAME, 20);
DECL_AST_KIND(EXPR_INIT, 21);

    
// TODO: Add Maybe type node, template system?
DECL_AST_KIND(TYPEEXPR_VOID, 22);
DECL_AST_KIND(TYPEEXPR_BOOL, 23);
DECL_AST_KIND(TYPEEXPR_NUMBER, 24);
DECL_AST_KIND(TYPEEXPR_STRING, 25);
DECL_AST_KIND(TYPEEXPR_USER, 26);
DECL_AST_KIND(TYPEEXPR_ARRAY, 27);
DECL_AST_KIND(TYPEEXPR_MAP, 28);
DECL_AST_KIND(TYPEEXPR_FUNC, 29);
DECL_AST_KIND(TYPEEXPR_ANY, 30);

extern const ASTKind ASTAllMask;
extern const ASTKind ASTStmtMask;
extern const ASTKind ASTDeclMask;
extern const ASTKind ASTExprMask;
extern const ASTKind ASTTypeExprMask;

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
    
    OrbitSRange sourceRange;
    
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
            OrbitToken  symbol;
        } constantExpr;
        
        struct {
            OrbitAST*   params;
            OrbitAST*   body;
        } lambdaExpr;
        
        struct {
            OrbitToken  symbol;
            OCStringID  name;
        } nameExpr;
        
        struct {
            OrbitAST*   type;
            OrbitAST*   params;
        } initExpr;
        
        // Type Expressions (necessary for a non-trivial type system)
        
        OrbitASTType    typeExpr;
        
    };
};

void orbit_astPrint(FILE* out, OrbitAST* ast);
void orbit_astDestroy(void* ref);
OrbitAST* orbit_astMake(ASTKind kind);

#endif /* orbit_ast_ast_h */
