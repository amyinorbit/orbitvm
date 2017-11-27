//
//  orbit/ast/ast_traversal.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-10-07.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <orbit/ast/traversal.h>

void ast_traverse(AST* ast, ASTKind filter, void* userData, ASTCallback callback) {
    
    if(ast == NULL) { return; }
    if(ast->kind & filter) {
        callback(ast, userData);
    }
    
    switch(ast->kind) {
    case AST_CONDITIONAL:
        ast_traverse(ast->conditionalStmt.condition, filter, userData, callback);
        ast_traverse(ast->conditionalStmt.ifBody, filter, userData, callback);
        ast_traverse(ast->conditionalStmt.elseBody, filter, userData, callback);
        break;
    
    case AST_FOR_IN:
        ast_traverse(ast->forInLoop.collection, filter, userData, callback);
        ast_traverse(ast->forInLoop.body, filter, userData, callback);
        break;
    
    case AST_WHILE:
        ast_traverse(ast->whileLoop.condition, filter, userData, callback);
        ast_traverse(ast->whileLoop.body, filter, userData, callback);
        break;
    
    case AST_BREAK:
    case AST_CONTINUE:
        break;
        
    case AST_RETURN:
        ast_traverse(ast->returnStmt.returnValue, filter, userData, callback);
        break;
    
    // DECLARATIONS
    case AST_DECL_MODULE:
        ast_traverse(ast->moduleDecl.body, filter, userData, callback);
        break;
    
    case AST_DECL_FUNC:
        ast_traverse(ast->funcDecl.returnType, filter, userData, callback);
        ast_traverse(ast->funcDecl.params, filter, userData, callback);
        ast_traverse(ast->funcDecl.body, filter, userData, callback);
        break;
    
    case AST_DECL_VAR:
        ast_traverse(ast->varDecl.typeAnnotation, filter, userData, callback);
        break;
    
    case AST_DECL_STRUCT:
        ast_traverse(ast->structDecl.constructor, filter, userData, callback);
        ast_traverse(ast->structDecl.destructor, filter, userData, callback);
        ast_traverse(ast->structDecl.fields, filter, userData, callback);
        break;
        
    // EXPRESSIONS
    case AST_EXPR_UNARY:
        ast_traverse(ast->unaryExpr.rhs, filter, userData, callback);
        break;
    
    case AST_EXPR_BINARY:
        ast_traverse(ast->binaryExpr.lhs, filter, userData, callback);
        ast_traverse(ast->binaryExpr.rhs, filter, userData, callback);
        break;
    
    case AST_EXPR_CALL:
        ast_traverse(ast->callExpr.symbol, filter, userData, callback);
        ast_traverse(ast->callExpr.params, filter, userData, callback);
        break;
        
    case AST_EXPR_SUBSCRIPT:
        ast_traverse(ast->subscriptExpr.symbol, filter, userData, callback);
        ast_traverse(ast->subscriptExpr.subscript, filter, userData, callback);
        break;
    
    case AST_EXPR_CONSTANT_INTEGER:
    case AST_EXPR_CONSTANT_FLOAT:
    case AST_EXPR_CONSTANT_STRING:
    case AST_EXPR_CONSTANT:
    case AST_EXPR_NAME:
    case AST_TYPEEXPR_NIL:
    case AST_TYPEEXPR_VOID:
    case AST_TYPEEXPR_BOOL:
    case AST_TYPEEXPR_NUMBER:
    case AST_TYPEEXPR_STRING:
    case AST_TYPEEXPR_USER:
    case AST_TYPEEXPR_ANY:
        break;
        
    case AST_TYPEEXPR_FUNC:
        ast_traverse(ast->typeExpr.funcType.returnType, filter, userData, callback);
        ast_traverse(ast->typeExpr.funcType.params, filter, userData, callback);
        break;
        
    case AST_TYPEEXPR_ARRAY:
        ast_traverse(ast->typeExpr.arrayType.elementType, filter, userData, callback);
        break;
        
    case AST_TYPEEXPR_MAP:
        ast_traverse(ast->typeExpr.mapType.keyType, filter, userData, callback);
        ast_traverse(ast->typeExpr.mapType.elementType, filter, userData, callback);
        break;
    }
    ast_traverse(ast->next, filter, userData, callback);
}
