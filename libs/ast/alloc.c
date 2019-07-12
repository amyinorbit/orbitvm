//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_alloc.c
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdlib.h>
#include <string.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/ast.h>

void orbitASTDestroy(void* ref) {
    if(ref == NULL) { return; }
    OrbitAST* ast = (OrbitAST*)ref;
    
        ORCRELEASE(ast->type);
        // if(allocatedNodes == 4) abort();
    
        switch(ast->kind) {
            // STATEMENTS
            case ORBIT_AST_ASSIGN:
                ORCRELEASE(ast->assignStmt.lhs);
                ORCRELEASE(ast->assignStmt.rhs);
                break;
                
            case ORBIT_AST_CONDITIONAL:
                ORCRELEASE(ast->conditionalStmt.condition);
                ORCRELEASE(ast->conditionalStmt.ifBody);
                ORCRELEASE(ast->conditionalStmt.elseBody);
                break;
        
            case ORBIT_AST_FOR_IN:
                ORCRELEASE(ast->forInLoop.collection);
                ORCRELEASE(ast->forInLoop.body);
                break;
        
            case ORBIT_AST_WHILE:
                ORCRELEASE(ast->whileLoop.condition);
                ORCRELEASE(ast->whileLoop.body);
                break;
            
            case ORBIT_AST_BLOCK:
                ORCRELEASE(ast->block.body);
                break;
            
            case ORBIT_AST_BREAK:
                break;
            
            case ORBIT_AST_CONTINUE:
                break;
            
            case ORBIT_AST_RETURN:
                ORCRELEASE(ast->returnStmt.returnValue);
                break;
                
            case ORBIT_AST_PRINT:
                ORCRELEASE(ast->printStmt.expr);
                break;
        
            // DECLARATIONS
            case ORBIT_AST_DECL_MODULE:
                ORCRELEASE(ast->moduleDecl.body);
                break;
        
            case ORBIT_AST_DECL_FUNC:
                ORCRELEASE(ast->funcDecl.returnType);
                ORCRELEASE(ast->funcDecl.params);
                ORCRELEASE(ast->funcDecl.body);
                break;
        
            case ORBIT_AST_DECL_VAR:
                ORCRELEASE(ast->varDecl.typeAnnotation);
                break;
        
            case ORBIT_AST_DECL_STRUCT:
                ORCRELEASE(ast->structDecl.constructor);
                ORCRELEASE(ast->structDecl.destructor);
                ORCRELEASE(ast->structDecl.fields);
                break;
            
            // EXPRESSIONS
            case ORBIT_AST_EXPR_UNARY:
                ORCRELEASE(ast->unaryExpr.rhs);
                break;
        
            case ORBIT_AST_EXPR_BINARY:
                ORCRELEASE(ast->binaryExpr.lhs);
                ORCRELEASE(ast->binaryExpr.rhs);
                break;
        
            case ORBIT_AST_EXPR_CALL:
                ORCRELEASE(ast->callExpr.symbol);
                ORCRELEASE(ast->callExpr.params);
                break;
        
            case ORBIT_AST_EXPR_SUBSCRIPT:
                ORCRELEASE(ast->subscriptExpr.symbol);
                ORCRELEASE(ast->subscriptExpr.subscript);
                break;
        
            case ORBIT_AST_EXPR_CONSTANT:
            case ORBIT_AST_EXPR_CONSTANT_INTEGER:
            case ORBIT_AST_EXPR_CONSTANT_FLOAT:
            case ORBIT_AST_EXPR_CONSTANT_STRING:
            case ORBIT_AST_EXPR_CONSTANT_BOOL:
                break;
        
            case ORBIT_AST_EXPR_NAME:
                break;
            
            case ORBIT_AST_EXPR_INIT:
                ORCRELEASE(ast->initExpr.type);
                ORCRELEASE(ast->initExpr.params);
                break;
                
            case ORBIT_AST_EXPR_LAMBDA:
                ORCRELEASE(ast->lambdaExpr.params);
                ORCRELEASE(ast->lambdaExpr.body);
                break;
            
            case ORBIT_AST_EXPR_I2F:
            case ORBIT_AST_EXPR_F2I:
                ORCRELEASE(ast->conversionExpr.expr);
                break;
            
            case ORBIT_AST_TYPEEXPR_VOID:
            case ORBIT_AST_TYPEEXPR_BOOL:
            case ORBIT_AST_TYPEEXPR_INT:
            case ORBIT_AST_TYPEEXPR_FLOAT:
            case ORBIT_AST_TYPEEXPR_STRING:
            case ORBIT_AST_TYPEEXPR_ANY:
            case ORBIT_AST_TYPEEXPR_USER:
                break;
            
            case ORBIT_AST_TYPEEXPR_FUNC:
                ORCRELEASE(ast->typeExpr.funcType.returnType);
                ORCRELEASE(ast->typeExpr.funcType.params);
                break;
            
            case ORBIT_AST_TYPEEXPR_ARRAY:
                ORCRELEASE(ast->typeExpr.arrayType.elementType);
                break;
            
            case ORBIT_AST_TYPEEXPR_MAP:
                ORCRELEASE(ast->typeExpr.mapType.keyType);
                ORCRELEASE(ast->typeExpr.mapType.elementType);
                break;
        }
        ORCRELEASE(ast->next);
}

OrbitAST* orbitASTMake(ASTKind kind) {
    OrbitAST* ast = ORBIT_ALLOC(OrbitAST);
    memset(ast, 0, sizeof (OrbitAST));
    ORCINIT(ast, &orbitASTDestroy);
    
    ast->kind = kind;
    ast->next = NULL;
    ast->type = NULL;
    return ast;
}
