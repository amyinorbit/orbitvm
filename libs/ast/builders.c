//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_builders.c
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
#include <orbit/ast/builders.h>

void orbit_astListStart(ASTListBuilder* builder) {
    builder->head = NULL;
    builder->next = &builder->head;
}

void orbit_astListAdd(ASTListBuilder* builder, OrbitAST* item) {
    if(item == NULL) { return; }
    if(builder->head != NULL) { ORCRETAIN(item); }
    *(builder->next) = item;
    builder->next = &item->next;
}

OrbitAST* orbit_astListClose(ASTListBuilder* builder) {
    *(builder->next) = NULL;
    return builder->head;
}


static OrbitToken ast_copyToken(const OrbitToken* token) {
    OrbitToken copy = *token;
    // TODO: deep copy of pointed-to string?
    return copy;
}

OrbitAST* orbit_astMakeConditional(OrbitAST* condition, OrbitAST* ifBody, OrbitAST* elseBody) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_CONDITIONAL);
    ast->conditionalStmt.condition = ORCRETAIN(condition);
    ast->conditionalStmt.ifBody = ORCRETAIN(ifBody);
    ast->conditionalStmt.elseBody = ORCRETAIN(elseBody);
    return ast;
}

OrbitAST* orbit_astMakeForInLoop(const OrbitToken* var, OrbitAST* collection, OrbitAST* body) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_FOR_IN);
    ast->forInLoop.variable = ast_copyToken(var);
    ast->forInLoop.collection = ORCRETAIN(collection);
    ast->forInLoop.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbit_astMakeWhileLoop(OrbitAST* condition, OrbitAST* body) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_WHILE);
    ast->whileLoop.condition = ORCRETAIN(condition);
    ast->whileLoop.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbit_astMakeBlock(OrbitAST* body) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_BLOCK);
    ast->block.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbit_astMakeBreak() {
    return orbit_astMake(ORBIT_AST_BREAK);
}

OrbitAST* orbit_astMakeContinue() {
    return orbit_astMake(ORBIT_AST_CONTINUE);
}

OrbitAST* orbit_astMakeReturn(OrbitAST* returned) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_RETURN);
    ast->returnStmt.returnValue = ORCRETAIN(returned);
    return ast;
}

OrbitAST* orbit_astMakeModuleDecl(const char* symbol, OrbitAST* body) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_DECL_MODULE);
    ast->moduleDecl.symbol = orbit_stringIntern(symbol, strlen(symbol));
    ast->moduleDecl.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbit_astMakeVarDecl(const OrbitToken* symbol, OrbitAST* typeAnnotation) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_DECL_VAR);
    ast->sourceRange = orbit_srangeFromLength(symbol->sourceLoc, symbol->length);
    ast->varDecl.symbol = ast_copyToken(symbol);
    ast->varDecl.name = orbit_stringIntern(symbol->source->bytes+symbol->sourceLoc.offset,
                                           symbol->length);
    ast->varDecl.typeAnnotation = ORCRETAIN(typeAnnotation);
    return ast;
}

OrbitAST* orbit_astMakeFuncDecl(const OrbitToken* symbol, OrbitAST* returnType, OrbitAST* params, OrbitAST* body) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_DECL_FUNC);
    // TODO: replace with union from name to return type?
    ast->sourceRange = orbit_srangeFromLength(symbol->sourceLoc, symbol->length);
    ast->funcDecl.symbol = ast_copyToken(symbol);
    ast->funcDecl.name = orbit_stringIntern(symbol->source->bytes+symbol->sourceLoc.offset,
                                            symbol->length);
    ast->funcDecl.mangledName = orbit_invalidStringID;
    ast->funcDecl.returnType = ORCRETAIN(returnType);
    ast->funcDecl.params = ORCRETAIN(params);
    ast->funcDecl.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbit_astMakeStructDecl(const OrbitToken* symbol, OrbitAST* constructor, OrbitAST* destructor, OrbitAST* fields) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_DECL_STRUCT);
    
    ast->sourceRange = orbit_srangeFromLength(symbol->sourceLoc, symbol->length);
    ast->structDecl.symbol = ast_copyToken(symbol);
    ast->structDecl.name = orbit_stringIntern(symbol->source->bytes+symbol->sourceLoc.offset,
                                              symbol->length);
    ast->structDecl.constructor = ORCRETAIN(constructor);
    ast->structDecl.destructor = ORCRETAIN(destructor);
    ast->structDecl.fields = ORCRETAIN(fields);
    
    return ast;
}

OrbitAST* orbit_astMakeBinaryExpr(const OrbitToken* operator, OrbitAST* lhs, OrbitAST* rhs) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_EXPR_BINARY);
    
    ast->binaryExpr.operator = ast_copyToken(operator);
    ast->binaryExpr.lhs = ORCRETAIN(lhs);
    ast->binaryExpr.rhs = ORCRETAIN(rhs);
    ast->sourceRange = orbit_srangeUnion(lhs->sourceRange, rhs->sourceRange);
    return ast;
}

OrbitAST* orbit_astMakeUnaryExpr(const OrbitToken* operator, OrbitAST* rhs) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_EXPR_UNARY);
    ast->unaryExpr.operator = ast_copyToken(operator);
    ast->unaryExpr.rhs = ORCRETAIN(rhs);
    ast->sourceRange = orbit_srangeUnion(
        orbit_srangeFromLength(operator->sourceLoc, operator->length),
        rhs->sourceRange
    );
    // TODO: create source range here
    return ast;
}

OrbitAST* orbit_astMakeCallExpr(OrbitAST* symbol, OrbitAST* params) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_EXPR_CALL);
    ast->callExpr.symbol = ORCRETAIN(symbol);
    ast->callExpr.params = ORCRETAIN(params);
    ast->sourceRange = params ?
                        orbit_srangeUnion(symbol->sourceRange, params->sourceRange) :
                        symbol->sourceRange;
    return ast;
}

OrbitAST* orbit_astMakeSubscriptExpr(OrbitAST* symbol, OrbitAST* subscript) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_EXPR_SUBSCRIPT);
    ast->subscriptExpr.symbol = ORCRETAIN(symbol);
    ast->subscriptExpr.subscript = ORCRETAIN(subscript);
    ast->sourceRange = orbit_srangeUnion(symbol->sourceRange, subscript->sourceRange);
    return ast;
}

OrbitAST* orbit_astMakeNameExpr(const OrbitToken* symbol) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_EXPR_NAME);
    ast->nameExpr.symbol = ast_copyToken(symbol);
    ast->nameExpr.name = orbit_stringIntern(
        symbol->source->bytes + symbol->sourceLoc.offset,
        symbol->length
    );
    ast->sourceRange = orbit_srangeFromLength(symbol->sourceLoc, symbol->displayLength);
    return ast;
}

OrbitAST* orbit_astMakeConstantExpr(const OrbitToken* symbol, ASTKind kind) {
    OrbitAST* ast = orbit_astMake(kind);
    ast->constantExpr.symbol = ast_copyToken(symbol);
    ast->sourceRange = orbit_srangeFromLength(symbol->sourceLoc, symbol->displayLength);
    return ast;
}

OrbitAST* orbit_astMakeUserTypePooled(OCStringID symbol) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_TYPEEXPR_USER);
    ast->typeExpr.userType.symbol = symbol;
    return ast;
}

OrbitAST* orbit_astMakeUserType(const OrbitToken* symbol) {
    OCStringID id = orbit_stringIntern(
        symbol->source->bytes+symbol->sourceLoc.offset,
        symbol->length
    );
    OrbitAST* ast = orbit_astMakeUserTypePooled(id);
    ast->sourceRange = orbit_srangeFromLength(symbol->sourceLoc, symbol->length);
    return ast;
}

OrbitAST* orbit_astMakePrimitiveType(ASTKind kind) {
    OrbitAST* ast = orbit_astMake(kind);
    return ast;
}

OrbitAST* orbit_astMakeFuncType(OrbitAST* returnType, OrbitAST* params) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_TYPEEXPR_FUNC);
    ast->typeExpr.funcType.returnType = ORCRETAIN(returnType);
    ast->typeExpr.funcType.params = ORCRETAIN(params);
    return ast;
}

OrbitAST* orbit_astMakeArrayType(OrbitAST* elementType) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_TYPEEXPR_ARRAY);
    ast->typeExpr.arrayType.elementType = ORCRETAIN(elementType);
    return ast;
}

OrbitAST* orbit_astMakeMapType(OrbitAST* keyType, OrbitAST* elementType) {
    OrbitAST* ast = orbit_astMake(ORBIT_AST_TYPEEXPR_MAP);
    ast->typeExpr.mapType.keyType = ORCRETAIN(keyType);
    ast->typeExpr.mapType.elementType = ORCRETAIN(elementType);
    return ast;
}
