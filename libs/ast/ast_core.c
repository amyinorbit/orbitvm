//
//  orbit/ast/ast.c
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
            free(ast->forInLoop.variable);
            ast_destroy(ast->forInLoop.collection);
            ast_destroy(ast->forInLoop.body);
            break;
        
        case AST_WHILE:
            ast_destroy(ast->whileLoop.condition);
            ast_destroy(ast->whileLoop.body);
            break;
        
        // DECLARATIONS
        case AST_DECL_MODULE:
            free(ast->moduleDecl.symbol);
            ast_destroy(ast->moduleDecl.body);
            break;
        
        case AST_DECL_FUNC:
            free(ast->funcDecl.symbol);
            ast_destroy(ast->funcDecl.returnType);
            ast_destroy(ast->funcDecl.params);
            ast_destroy(ast->funcDecl.body);
            break;
        
        case AST_DECL_VAR:
            free(ast->varDecl.symbol);
            ast_destroy(ast->varDecl.typeAnnotation);
            break;
        
        case AST_DECL_PARAM:
            free(ast->paramDecl.symbol);
            ast_destroy(ast->paramDecl.typeAnnotation);
            break;
        
        case AST_DECL_STRUCT:
            free(ast->structDecl.symbol);
            ast_destroy(ast->structDecl.constructor);
            ast_destroy(ast->structDecl.destructor);
            ast_destroy(ast->structDecl.fields);
            break;
            
        // EXPRESSIONS
        case AST_EXPR_UNARY:
            free(ast->unaryExpr.operator);
            ast_destroy(ast->unaryExpr.rhs);
            break;
        
        case AST_EXPR_BINARY:
            free(ast->binaryExpr.operator);
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
            free(ast->constantExpr.symbol);
            break;
        
        case AST_EXPR_NAME:
            free(ast->nameExpr.symbol);
            break;
        
        case AST_EXPR_TYPE:
            free(ast->typeExpr.symbol);
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

static OCToken* ast_copyToken(const OCToken* token) {
    OCToken* copy = malloc(sizeof (OCToken));
    *copy = *token;
    return copy;
}

AST* ast_makeConditional(AST* condition, AST* ifBody, AST* elseBody) {
    AST* ast = ast_makeNode(AST_CONDITIONAL);
    ast->conditionalStmt.condition = condition;
    ast->conditionalStmt.ifBody = ifBody;
    ast->conditionalStmt.elseBody = elseBody;
    return ast;
}

AST* ast_makeForInLoop(const OCToken* var, AST* collection, AST* body) {
    AST* ast = ast_makeNode(AST_FOR_IN);
    ast->forInLoop.variable = ast_copyToken(var);
    ast->forInLoop.collection = collection;
    ast->forInLoop.body = body;
    return ast;
}

AST* ast_makeWhileLoop(AST* condition, AST* body) {
    AST* ast = ast_makeNode(AST_WHILE);
    ast->whileLoop.condition = condition;
    ast->whileLoop.body = body;
    return ast;
}

AST* ast_makeVarDecl(const OCToken* symbol, AST* typeAnnotation) {
    AST* ast = ast_makeNode(AST_DECL_VAR);
    ast->varDecl.symbol = ast_copyToken(symbol);
    ast->varDecl.typeAnnotation = typeAnnotation;
    return ast;
}

AST* ast_makeFuncDecl(const OCToken* symbol, AST* returnType, AST* params, AST* body) {
    AST* ast = ast_makeNode(AST_DECL_FUNC);
    ast->funcDecl.symbol = ast_copyToken(symbol);
    ast->funcDecl.returnType = returnType;
    ast->funcDecl.params = params;
    ast->funcDecl.body = body;
    return ast;
}

AST* ast_makeStructDecl(const OCToken* symbol, AST* constructor, AST* destructor, AST* fields) {
    AST* ast = ast_makeNode(AST_DECL_STRUCT);
    
    ast->structDecl.symbol = ast_copyToken(symbol);
    ast->structDecl.constructor = constructor;
    ast->structDecl.destructor = destructor;
    ast->structDecl.fields = fields;
    
    return ast;
}

AST* ast_makeBinaryExpr(const OCToken* operator, AST* lhs, AST* rhs) {
    AST* ast = ast_makeNode(AST_EXPR_BINARY);
    
    ast->binaryExpr.operator = ast_copyToken(operator);
    ast->binaryExpr.lhs = lhs;
    ast->binaryExpr.rhs = rhs;
    
    return ast;
}

AST* ast_makeUnaryExpr(const OCToken* operator, AST* rhs) {
    AST* ast = ast_makeNode(AST_EXPR_UNARY);
    ast->unaryExpr.operator = ast_copyToken(operator);
    ast->unaryExpr.rhs = rhs;
    return ast;
}

AST* ast_makeCallExpr(AST* symbol, AST* params) {
    AST* ast = ast_makeNode(AST_EXPR_CALL);
    ast->callExpr.symbol = symbol;
    ast->callExpr.params = params;
    return ast;
}

AST* ast_makeSubscriptExpr(AST* symbol, AST* subscript) {
    AST* ast = ast_makeNode(AST_EXPR_SUBSCRIPT);
    ast->subscriptExpr.symbol = symbol;
    ast->subscriptExpr.subscript = subscript;
    return ast;
}

AST* ast_makeNameExpr(const OCToken* symbol) {
    AST* ast = ast_makeNode(AST_EXPR_NAME);
    ast->nameExpr.symbol = ast_copyToken(symbol);
    return ast;
}

AST* ast_makeConstantExpr(const OCToken* symbol) {
    AST* ast = ast_makeNode(AST_EXPR_CONSTANT);
    ast->constantExpr.symbol = ast_copyToken(symbol);
    return ast;
}

AST* ast_makeTypeExpr(const OCToken* symbol) {
    AST* ast = ast_makeNode(AST_EXPR_TYPE);
    ast->typeExpr.symbol = ast_copyToken(symbol);
    return ast;
}
