//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_traversal.c
// This source is part of Orbit - AST
//
// Created on 2017-10-07 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <orbit/ast/traversal.h>

OrbitASTVisitor orbit_astSimpleVisitor(OrbitASTCallback callback, ASTKind filter, void* data) {
    return (OrbitASTVisitor){
        .callback = callback,
        .predicate = NULL,
        .filter = filter,
        .data = data
    };
}

OrbitASTVisitor orbit_astVisitor(OrbitASTCallback callback, OrbitASTPredicate pred, void* data) {
    return (OrbitASTVisitor){
        .callback = callback,
        .predicate = pred,
        .filter = 0,
        .data = data
    };
}

static bool _ast_matchesPredicate(const OrbitASTVisitor* visitor, const OrbitAST* ast) {
    assert(visitor && "null AST visitor instance given");
    assert(ast && "null AST node given");
    
    if(!visitor->predicate) {
        return (ast->kind & visitor->filter);
    }
    return visitor->predicate(visitor, ast);
}

static void _ast_doTraverse(OrbitASTContext* ctx, OrbitAST* ast, const OrbitASTVisitor* visitor) {
    
    if(ast == NULL) { return; }
    if(_ast_matchesPredicate(visitor, ast)) {
        visitor->callback(ctx, ast, visitor->data);
    }
    
    switch(ast->kind) {
    case ORBIT_AST_CONDITIONAL:
        _ast_doTraverse(ctx, ast->conditionalStmt.condition, visitor);
        _ast_doTraverse(ctx, ast->conditionalStmt.ifBody, visitor);
        _ast_doTraverse(ctx, ast->conditionalStmt.elseBody, visitor);
        break;
    
    case ORBIT_AST_FOR_IN:
        _ast_doTraverse(ctx, ast->forInLoop.collection, visitor);
        _ast_doTraverse(ctx, ast->forInLoop.body, visitor);
        break;
    
    case ORBIT_AST_WHILE:
        _ast_doTraverse(ctx, ast->whileLoop.condition, visitor);
        _ast_doTraverse(ctx, ast->whileLoop.body, visitor);
        break;
        
    case ORBIT_AST_BLOCK:
        _ast_doTraverse(ctx, ast->block.body, visitor);
        break;
    
    case ORBIT_AST_BREAK:
    case ORBIT_AST_CONTINUE:
        break;
        
    case ORBIT_AST_RETURN:
        _ast_doTraverse(ctx, ast->returnStmt.returnValue, visitor);
        break;
    
    // DECLARATIONS
    case ORBIT_AST_DECL_MODULE:
        _ast_doTraverse(ctx, ast->moduleDecl.body, visitor);
        break;
    
    case ORBIT_AST_DECL_FUNC:
        _ast_doTraverse(ctx, ast->funcDecl.returnType, visitor);
        _ast_doTraverse(ctx, ast->funcDecl.params, visitor);
        _ast_doTraverse(ctx, ast->funcDecl.body, visitor);
        break;
    
    case ORBIT_AST_DECL_VAR:
        _ast_doTraverse(ctx, ast->varDecl.typeAnnotation, visitor);
        break;
    
    case ORBIT_AST_DECL_STRUCT:
        _ast_doTraverse(ctx, ast->structDecl.constructor, visitor);
        _ast_doTraverse(ctx, ast->structDecl.destructor, visitor);
        _ast_doTraverse(ctx, ast->structDecl.fields, visitor);
        break;
        
    // EXPRESSIONS
    case ORBIT_AST_EXPR_UNARY:
        _ast_doTraverse(ctx, ast->unaryExpr.rhs, visitor);
        break;
    
    case ORBIT_AST_EXPR_BINARY:
        _ast_doTraverse(ctx, ast->binaryExpr.lhs, visitor);
        _ast_doTraverse(ctx, ast->binaryExpr.rhs, visitor);
        break;
    
    case ORBIT_AST_EXPR_CALL:
        _ast_doTraverse(ctx, ast->callExpr.symbol, visitor);
        _ast_doTraverse(ctx, ast->callExpr.params, visitor);
        break;
        
    case ORBIT_AST_EXPR_SUBSCRIPT:
        _ast_doTraverse(ctx, ast->subscriptExpr.symbol, visitor);
        _ast_doTraverse(ctx, ast->subscriptExpr.subscript, visitor);
        break;
        
    case ORBIT_AST_EXPR_INIT:
        _ast_doTraverse(ctx, ast->initExpr.type, visitor);
        _ast_doTraverse(ctx, ast->initExpr.params, visitor);
        break;
    
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
    case ORBIT_AST_EXPR_CONSTANT_STRING:
    case ORBIT_AST_EXPR_CONSTANT:
    case ORBIT_AST_EXPR_NAME:
    case ORBIT_AST_TYPEEXPR_VOID:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_NUMBER:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_USER:
    case ORBIT_AST_TYPEEXPR_ANY:
        break;
        
    case ORBIT_AST_TYPEEXPR_FUNC:
        _ast_doTraverse(ctx, ast->typeExpr.funcType.returnType, visitor);
        _ast_doTraverse(ctx, ast->typeExpr.funcType.params, visitor);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        _ast_doTraverse(ctx, ast->typeExpr.arrayType.elementType, visitor);
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        _ast_doTraverse(ctx, ast->typeExpr.mapType.keyType, visitor);
        _ast_doTraverse(ctx, ast->typeExpr.mapType.elementType, visitor);
        break;
    }
    _ast_doTraverse(ctx, ast->next, visitor);
}

void orbit_astTraverse(OrbitASTContext* ctx, OrbitASTVisitor visitor) {
    assert(ctx && "null AST Context given to traversal");
    _ast_doTraverse(ctx, ctx->root, &visitor);
}
