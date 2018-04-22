//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_alloc.c
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdlib.h>
#include <string.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/ast.h>

const uint32_t ASTStmtMask      = AST_CONDITIONAL
                                | AST_FOR_IN
                                | AST_WHILE
                                | AST_BREAK
                                | AST_CONTINUE
                                | AST_RETURN;

const uint32_t ASTDeclMask      = AST_DECL_MODULE
                                | AST_DECL_FUNC
                                | AST_DECL_VAR
                                | AST_DECL_STRUCT;

const uint32_t ASTExprMask      = AST_EXPR_UNARY
                                | AST_EXPR_BINARY
                                | AST_EXPR_CALL
                                | AST_EXPR_SUBSCRIPT
                                | AST_EXPR_CONSTANT
                                | AST_EXPR_CONSTANT_INTEGER
                                | AST_EXPR_CONSTANT_FLOAT
                                | AST_EXPR_CONSTANT_STRING
                                | AST_EXPR_NAME;

const uint32_t ASTTypeExprMask  = AST_TYPEEXPR_NIL
                                | AST_TYPEEXPR_VOID
                                | AST_TYPEEXPR_BOOL
                                | AST_TYPEEXPR_NUMBER
                                | AST_TYPEEXPR_STRING
                                | AST_TYPEEXPR_USER
                                | AST_TYPEEXPR_ANY
                                | AST_TYPEEXPR_ARRAY
                                | AST_TYPEEXPR_MAP
                                | AST_TYPEEXPR_FUNC;

const uint32_t ASTAllMask       = 0xffffffff;

void ast_destroy(void* ref) {
    if(ref == NULL) { return; }
    AST* ast = (AST*)ref;
    
    switch(ast->kind) {
        // STATEMENTS
        case AST_CONDITIONAL:
            ORCRELEASE(ast->conditionalStmt.condition);
            ORCRELEASE(ast->conditionalStmt.ifBody);
            ORCRELEASE(ast->conditionalStmt.elseBody);
            break;
        
        case AST_FOR_IN:
            ORCRELEASE(ast->forInLoop.collection);
            ORCRELEASE(ast->forInLoop.body);
            break;
        
        case AST_WHILE:
            ORCRELEASE(ast->whileLoop.condition);
            ORCRELEASE(ast->whileLoop.body);
            break;
            
        case AST_BREAK:
            break;
            
        case AST_CONTINUE:
            break;
            
        case AST_RETURN:
            ORCRELEASE(ast->returnStmt.returnValue);
            break;
        
        // DECLARATIONS
        case AST_DECL_MODULE:
            ORCRELEASE(ast->moduleDecl.body);
            break;
        
        case AST_DECL_FUNC:
            ORCRELEASE(ast->funcDecl.returnType);
            ORCRELEASE(ast->funcDecl.params);
            ORCRELEASE(ast->funcDecl.body);
            break;
        
        case AST_DECL_VAR:
            ORCRELEASE(ast->varDecl.typeAnnotation);
            break;
        
        case AST_DECL_STRUCT:
            ORCRELEASE(ast->structDecl.constructor);
            ORCRELEASE(ast->structDecl.destructor);
            ORCRELEASE(ast->structDecl.fields);
            break;
            
        // EXPRESSIONS
        case AST_EXPR_UNARY:
            ORCRELEASE(ast->unaryExpr.rhs);
            break;
        
        case AST_EXPR_BINARY:
            ORCRELEASE(ast->binaryExpr.lhs);
            ORCRELEASE(ast->binaryExpr.rhs);
            break;
        
        case AST_EXPR_CALL:
            ORCRELEASE(ast->callExpr.symbol);
            ORCRELEASE(ast->callExpr.params);
            break;
        
        case AST_EXPR_SUBSCRIPT:
            ORCRELEASE(ast->subscriptExpr.symbol);
            ORCRELEASE(ast->subscriptExpr.subscript);
            break;
        
        case AST_EXPR_CONSTANT:
        case AST_EXPR_CONSTANT_INTEGER:
        case AST_EXPR_CONSTANT_FLOAT:
            break;
        case AST_EXPR_CONSTANT_STRING:
            ORCRELEASE(ast->constantExpr.symbol.parsedStringLiteral);
            break;
        
        case AST_EXPR_NAME:
            break;
        
        case AST_TYPEEXPR_NIL:
        case AST_TYPEEXPR_VOID:
        case AST_TYPEEXPR_BOOL:
        case AST_TYPEEXPR_NUMBER:
        case AST_TYPEEXPR_STRING:
        case AST_TYPEEXPR_ANY:
            break;
        case AST_TYPEEXPR_USER:
            ORCRELEASE(ast->typeExpr.userType.symbol.parsedStringLiteral);
            break;
            
        case AST_TYPEEXPR_FUNC:
            ORCRELEASE(ast->typeExpr.funcType.returnType);
            ORCRELEASE(ast->typeExpr.funcType.params);
            break;
            
        case AST_TYPEEXPR_ARRAY:
            ORCRELEASE(ast->typeExpr.arrayType.elementType);
            break;
            
        case AST_TYPEEXPR_MAP:
            ORCRELEASE(ast->typeExpr.mapType.keyType);
            ORCRELEASE(ast->typeExpr.mapType.elementType);
            break;
    }

    ORCRELEASE(ast->type);
    ORCRELEASE(ast->next);
}

AST* ast_makeNode(ASTKind kind) {
    AST* ast = orbit_alloc(sizeof (AST));
    memset(ast, 0, sizeof (AST));
    ORCINIT(ast, &ast_destroy);
    
    ast->kind = kind;
    ast->next = NULL;
    ast->type = NULL;
    return ast;
}
