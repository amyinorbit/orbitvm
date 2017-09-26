//
//  orbit/ast/ast_alloc.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-16.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <orbit/ast/ast.h>

void ast_destroy(AST* ast) {
    if(ast == NULL) { return; }
    
    switch(ast->type) {
        // STATEMENTS
        case AST_LIST:
            ast_destroy(ast->list.head);
            break;
        
        case AST_CONDITIONAL:
            ast_destroy(ast->conditionalStmt.condition);
            ast_destroy(ast->conditionalStmt.ifBody);
            ast_destroy(ast->conditionalStmt.elseBody);
            break;
        
        case AST_FOR_IN:
            ast_destroy(ast->forInLoop.collection);
            ast_destroy(ast->forInLoop.body);
            break;
        
        case AST_WHILE:
            ast_destroy(ast->whileLoop.condition);
            ast_destroy(ast->whileLoop.body);
            break;
            
        case AST_BREAK:
            break;
            
        case AST_CONTINUE:
            break;
            
        case AST_RETURN:
            ast_destroy(ast->returnStmt.returnValue);
            break;
        
        // DECLARATIONS
        case AST_DECL_MODULE:
            ast_destroy(ast->moduleDecl.body);
            break;
        
        case AST_DECL_FUNC:
            ast_destroy(ast->funcDecl.returnType);
            ast_destroy(ast->funcDecl.params);
            ast_destroy(ast->funcDecl.body);
            break;
        
        case AST_DECL_VAR:
            ast_destroy(ast->varDecl.typeAnnotation);
            break;
        
        case AST_DECL_STRUCT:
            ast_destroy(ast->structDecl.constructor);
            ast_destroy(ast->structDecl.destructor);
            ast_destroy(ast->structDecl.fields);
            break;
            
        // EXPRESSIONS
        case AST_EXPR_UNARY:
            ast_destroy(ast->unaryExpr.rhs);
            break;
        
        case AST_EXPR_BINARY:
            ast_destroy(ast->binaryExpr.lhs);
            ast_destroy(ast->binaryExpr.rhs);
            break;
        
        case AST_EXPR_CALL:
            ast_destroy(ast->callExpr.symbol);
            ast_destroy(ast->callExpr.params);
            break;
        
        case AST_EXPR_SUBSCRIPT:
            ast_destroy(ast->subscriptExpr.symbol);
            ast_destroy(ast->subscriptExpr.subscript);
            break;
        
        case AST_EXPR_CONSTANT:
            break;
        
        case AST_EXPR_NAME:
            break;
        
        case AST_TYPEEXPR_SIMPLE:
            break;
            
        case AST_TYPEEXPR_FUNC:
            ast_destroy(ast->funcType.returnType);
            ast_destroy(ast->funcType.params);
            break;
            
        case AST_TYPEEXPR_ARRAY:
            ast_destroy(ast->arrayType.elementType);
            break;
            
        case AST_TYPEEXPR_MAP:
            ast_destroy(ast->mapType.keyType);
            ast_destroy(ast->mapType.elementType);
            break;
    }
    
    ast_destroy(ast->next);
    free(ast);
}

AST* ast_makeNode(ASTType type) {
    AST* ast = malloc(sizeof (AST));
    memset(ast, 0, sizeof (AST));
    
    ast->type = type;
    ast->next = NULL;
    return ast;
}
