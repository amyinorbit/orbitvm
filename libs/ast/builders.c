//
//  orbit/ast/ast_builders.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-16.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <orbit/ast/builders.h>

void ast_listStart(ASTListBuilder* builder) {
    builder->head = NULL;
    builder->next = &builder->head;
}

void ast_listAdd(ASTListBuilder* builder, AST* item) {
    if(item == NULL) { return; }
    *(builder->next) = item;
    builder->next = &item->next;
}

AST* ast_listClose(ASTListBuilder* builder) {
    *(builder->next) = NULL;
    return builder->head;
}


static OCToken ast_copyToken(const OCToken* token) {
    OCToken copy = *token;
    // TODO: deep copy of pointed-to string?
    return copy;
}

// static char* ast_copyString(const char* string, size_t length) {
//     char* copy = calloc(sizeof(char), length + 1);
//     mcmcpy(copy, string)
// }

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

AST* ast_makeBreak() {
    return ast_makeNode(AST_BREAK);
}

AST* ast_makeContinue() {
    return ast_makeNode(AST_CONTINUE);
}

AST* ast_makeReturn(AST* returned) {
    AST* ast = ast_makeNode(AST_RETURN);
    ast->returnStmt.returnValue = returned;
    return ast;
}

AST* ast_makeModuleDecl(const char* symbol, AST* body) {
    AST* ast = ast_makeNode(AST_DECL_MODULE);
    ast->moduleDecl.symbol = symbol;
    ast->moduleDecl.body = body;
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

AST* ast_makeConstantExpr(const OCToken* symbol, ASTKind kind) {
    AST* ast = ast_makeNode(kind);
    ast->constantExpr.symbol = ast_copyToken(symbol);
    return ast;
}

AST* ast_makeTypeExpr(const OCToken* symbol) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_SIMPLE);
    ast->simpleType.symbol = ast_copyToken(symbol);
    return ast;
}

AST* ast_makeFuncType(AST* returnType, AST* params) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_FUNC);
    ast->funcType.returnType = returnType;
    ast->funcType.params = params;
    return ast;
}

AST* ast_makeArrayType(AST* elementType) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_ARRAY);
    ast->arrayType.elementType = elementType;
    return ast;
}

AST* ast_makeMapType(AST* keyType, AST* elementType) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_MAP);
    ast->mapType.keyType = keyType;
    ast->mapType.elementType = elementType;
    return ast;
}