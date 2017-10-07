//
//  orbit/ast/ast_traversal.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-10-07.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <orbit/ast/traversal.h>

void ast_traverse(AST* ast, ASTKind filter, ASTCallback callback) {
    
    if(ast == NULL) { return; }
    if(ast->kind & filter) {
        callback(ast, filter);
    }
    
    switch(ast->kind) {
    case AST_CONDITIONAL:
        ast_traverse(ast->conditionalStmt.condition, filter, callback);
        ast_traverse(ast->conditionalStmt.ifBody, filter, callback);
        ast_traverse(ast->conditionalStmt.elseBody, filter, callback);
        break;
    
    case AST_FOR_IN:
        ast_traverse(ast->forInLoop.collection, filter, callback);
        ast_traverse(ast->forInLoop.body, filter, callback);
        break;
    
    case AST_WHILE:
        ast_traverse(ast->whileLoop.condition, filter, callback);
        ast_traverse(ast->whileLoop.body, filter, callback);
        break;
    
    case AST_BREAK:
    case AST_CONTINUE:
        break;
        
    case AST_RETURN:
        ast_traverse(ast->returnStmt.returnValue, filter, callback);
        break;
    
    // DECLARATIONS
    case AST_DECL_MODULE:
        ast_traverse(ast->moduleDecl.body, filter, callback);
        break;
    
    case AST_DECL_FUNC:
        ast_traverse(ast->funcDecl.returnType, filter, callback);
        ast_traverse(ast->funcDecl.params, filter, callback);
        ast_traverse(ast->funcDecl.body, filter, callback);
        break;
    
    case AST_DECL_VAR:
        ast_traverse(ast->varDecl.typeAnnotation, filter, callback);
        break;
    
    case AST_DECL_STRUCT:
        ast_traverse(ast->structDecl.constructor, filter, callback);
        ast_traverse(ast->structDecl.destructor, filter, callback);
        ast_traverse(ast->structDecl.constructor, filter, callback);
        break;
        
    // EXPRESSIONS
    case AST_EXPR_UNARY:
        ast_traverse(ast->unaryExpr.rhs, filter, callback);
        break;
    
    case AST_EXPR_BINARY:
        ast_traverse(ast->binaryExpr.rhs, filter, callback);
        ast_traverse(ast->binaryExpr.rhs, filter, callback);
        break;
    
    case AST_EXPR_CALL:
        ast_traverse(ast->callExpr.symbol, filter, callback);
        ast_traverse(ast->callExpr.params, filter, callback);
        break;
        
    case AST_EXPR_SUBSCRIPT:
        ast_traverse(ast->subscriptExpr.symbol, filter, callback);
        ast_traverse(ast->subscriptExpr.subscript, filter, callback);
        break;
    
    case AST_EXPR_CONSTANT_INTEGER:
    case AST_EXPR_CONSTANT_FLOAT:
    case AST_EXPR_CONSTANT_STRING:
    case AST_EXPR_CONSTANT:
    case AST_EXPR_NAME:
    case AST_TYPEEXPR_SIMPLE:
        break;
        
    case AST_TYPEEXPR_FUNC:
        ast_traverse(ast->funcType.returnType, filter, callback);
        ast_traverse(ast->funcType.params, filter, callback);
        break;
        
    case AST_TYPEEXPR_ARRAY:
        ast_traverse(ast->arrayType.elementType, filter, callback);
        break;
        
    case AST_TYPEEXPR_MAP:
        ast_traverse(ast->mapType.keyType, filter, callback);
        ast_traverse(ast->mapType.elementType, filter, callback);
        break;
    }
    ast_traverse(ast->next, filter, callback);
}
