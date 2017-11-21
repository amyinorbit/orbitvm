//
//  orbit/ast/ast_alloc.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-16.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/ast.h>
#include <orbit/type/type.h>

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

const uint32_t ASTKindExprMask  = AST_TYPEEXPR_SIMPLE
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
        case AST_EXPR_CONSTANT_STRING:
            break;
        
        case AST_EXPR_NAME:
            break;
        
        case AST_TYPEEXPR_SIMPLE:
            break;
            
        case AST_TYPEEXPR_FUNC:
            ORCRELEASE(ast->funcType.returnType);
            ORCRELEASE(ast->funcType.params);
            break;
            
        case AST_TYPEEXPR_ARRAY:
            ORCRELEASE(ast->arrayType.elementType);
            break;
            
        case AST_TYPEEXPR_MAP:
            ORCRELEASE(ast->mapType.keyType);
            ORCRELEASE(ast->mapType.elementType);
            break;
    }
    
    type_destroy(ast->type);
    ORCRELEASE(ast->next);
    //free(ast);
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
