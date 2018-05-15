//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_traversal.c
// This source is part of Orbit - AST
//
// Created on 2017-10-07 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/ast/traversal.h>

void orbit_astTraverse(OrbitAST* ast, ASTKind filter, void* userData, ASTCallback callback) {
    
    if(ast == NULL) { return; }
    if(ast->kind & filter) {
        callback(ast, userData);
    }
    
    switch(ast->kind) {
    case ORBIT_AST_CONDITIONAL:
        orbit_astTraverse(ast->conditionalStmt.condition, filter, userData, callback);
        orbit_astTraverse(ast->conditionalStmt.ifBody, filter, userData, callback);
        orbit_astTraverse(ast->conditionalStmt.elseBody, filter, userData, callback);
        break;
    
    case ORBIT_AST_FOR_IN:
        orbit_astTraverse(ast->forInLoop.collection, filter, userData, callback);
        orbit_astTraverse(ast->forInLoop.body, filter, userData, callback);
        break;
    
    case ORBIT_AST_WHILE:
        orbit_astTraverse(ast->whileLoop.condition, filter, userData, callback);
        orbit_astTraverse(ast->whileLoop.body, filter, userData, callback);
        break;
        
    case ORBIT_AST_BLOCK:
        orbit_astTraverse(ast->block.body, filter, userData, callback);
        break;
    
    case ORBIT_AST_BREAK:
    case ORBIT_AST_CONTINUE:
        break;
        
    case ORBIT_AST_RETURN:
        orbit_astTraverse(ast->returnStmt.returnValue, filter, userData, callback);
        break;
    
    // DECLARATIONS
    case ORBIT_AST_DECL_MODULE:
        orbit_astTraverse(ast->moduleDecl.body, filter, userData, callback);
        break;
    
    case ORBIT_AST_DECL_FUNC:
        orbit_astTraverse(ast->funcDecl.returnType, filter, userData, callback);
        orbit_astTraverse(ast->funcDecl.params, filter, userData, callback);
        orbit_astTraverse(ast->funcDecl.body, filter, userData, callback);
        break;
    
    case ORBIT_AST_DECL_VAR:
        orbit_astTraverse(ast->varDecl.typeAnnotation, filter, userData, callback);
        break;
    
    case ORBIT_AST_DECL_STRUCT:
        orbit_astTraverse(ast->structDecl.constructor, filter, userData, callback);
        orbit_astTraverse(ast->structDecl.destructor, filter, userData, callback);
        orbit_astTraverse(ast->structDecl.fields, filter, userData, callback);
        break;
        
    // EXPRESSIONS
    case ORBIT_AST_EXPR_UNARY:
        orbit_astTraverse(ast->unaryExpr.rhs, filter, userData, callback);
        break;
    
    case ORBIT_AST_EXPR_BINARY:
        orbit_astTraverse(ast->binaryExpr.lhs, filter, userData, callback);
        orbit_astTraverse(ast->binaryExpr.rhs, filter, userData, callback);
        break;
    
    case ORBIT_AST_EXPR_CALL:
        orbit_astTraverse(ast->callExpr.symbol, filter, userData, callback);
        orbit_astTraverse(ast->callExpr.params, filter, userData, callback);
        break;
        
    case ORBIT_AST_EXPR_SUBSCRIPT:
        orbit_astTraverse(ast->subscriptExpr.symbol, filter, userData, callback);
        orbit_astTraverse(ast->subscriptExpr.subscript, filter, userData, callback);
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
        orbit_astTraverse(ast->typeExpr.funcType.returnType, filter, userData, callback);
        orbit_astTraverse(ast->typeExpr.funcType.params, filter, userData, callback);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        orbit_astTraverse(ast->typeExpr.arrayType.elementType, filter, userData, callback);
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        orbit_astTraverse(ast->typeExpr.mapType.keyType, filter, userData, callback);
        orbit_astTraverse(ast->typeExpr.mapType.elementType, filter, userData, callback);
        break;
    }
    orbit_astTraverse(ast->next, filter, userData, callback);
}
