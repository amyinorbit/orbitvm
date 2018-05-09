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

void ast_listStart(ASTListBuilder* builder) {
    builder->head = NULL;
    builder->next = &builder->head;
}

void ast_listAdd(ASTListBuilder* builder, AST* item) {
    if(item == NULL) { return; }
    if(builder->head != NULL) { ORCRETAIN(item); }
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

AST* ast_makeConditional(AST* condition, AST* ifBody, AST* elseBody) {
    AST* ast = ast_makeNode(AST_CONDITIONAL);
    ast->conditionalStmt.condition = ORCRETAIN(condition);
    ast->conditionalStmt.ifBody = ORCRETAIN(ifBody);
    ast->conditionalStmt.elseBody = ORCRETAIN(elseBody);
    return ast;
}

AST* ast_makeForInLoop(const OCToken* var, AST* collection, AST* body) {
    AST* ast = ast_makeNode(AST_FOR_IN);
    ast->forInLoop.variable = ast_copyToken(var);
    ast->forInLoop.collection = ORCRETAIN(collection);
    ast->forInLoop.body = ORCRETAIN(body);
    return ast;
}

AST* ast_makeWhileLoop(AST* condition, AST* body) {
    AST* ast = ast_makeNode(AST_WHILE);
    ast->whileLoop.condition = ORCRETAIN(condition);
    ast->whileLoop.body = ORCRETAIN(body);
    return ast;
}

AST* ast_makeBlock(AST* body) {
    AST* ast = ast_makeNode(AST_BLOCK);
    ast->block.body = ORCRETAIN(body);
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
    ast->returnStmt.returnValue = ORCRETAIN(returned);
    return ast;
}

AST* ast_makeModuleDecl(const char* symbol, AST* body) {
    AST* ast = ast_makeNode(AST_DECL_MODULE);
    ast->moduleDecl.symbol = orbit_stringIntern(symbol, strlen(symbol));
    ast->moduleDecl.body = ORCRETAIN(body);
    return ast;
}

AST* ast_makeVarDecl(const OCToken* symbol, AST* typeAnnotation) {
    AST* ast = ast_makeNode(AST_DECL_VAR);
    ast->varDecl.symbol = ast_copyToken(symbol);
    ast->varDecl.name = orbit_stringIntern(symbol->source->bytes+symbol->sourceLoc.offset,
                                           symbol->length);
    ast->varDecl.typeAnnotation = ORCRETAIN(typeAnnotation);
    return ast;
}

AST* ast_makeFuncDecl(const OCToken* symbol, AST* returnType, AST* params, AST* body) {
    AST* ast = ast_makeNode(AST_DECL_FUNC);
    ast->funcDecl.symbol = ast_copyToken(symbol);
    ast->funcDecl.name = orbit_stringIntern(symbol->source->bytes+symbol->sourceLoc.offset,
                                            symbol->length);
    ast->funcDecl.mangledName = orbit_invalidStringID;
    ast->funcDecl.returnType = ORCRETAIN(returnType);
    ast->funcDecl.params = ORCRETAIN(params);
    ast->funcDecl.body = ORCRETAIN(body);
    return ast;
}

AST* ast_makeStructDecl(const OCToken* symbol, AST* constructor, AST* destructor, AST* fields) {
    AST* ast = ast_makeNode(AST_DECL_STRUCT);
    
    ast->structDecl.symbol = ast_copyToken(symbol);
    ast->structDecl.name = orbit_stringIntern(symbol->source->bytes+symbol->sourceLoc.offset,
                                              symbol->length);
    ast->structDecl.constructor = ORCRETAIN(constructor);
    ast->structDecl.destructor = ORCRETAIN(destructor);
    ast->structDecl.fields = ORCRETAIN(fields);
    
    return ast;
}

AST* ast_makeBinaryExpr(const OCToken* operator, AST* lhs, AST* rhs) {
    AST* ast = ast_makeNode(AST_EXPR_BINARY);
    
    ast->binaryExpr.operator = ast_copyToken(operator);
    ast->binaryExpr.lhs = ORCRETAIN(lhs);
    ast->binaryExpr.rhs = ORCRETAIN(rhs);
    ast->sourceRange = source_rangeUnion(lhs->sourceRange, rhs->sourceRange);
    return ast;
}

AST* ast_makeUnaryExpr(const OCToken* operator, AST* rhs) {
    AST* ast = ast_makeNode(AST_EXPR_UNARY);
    ast->unaryExpr.operator = ast_copyToken(operator);
    ast->unaryExpr.rhs = ORCRETAIN(rhs);
    // TODO: create source range here
    return ast;
}

AST* ast_makeCallExpr(AST* symbol, AST* params) {
    AST* ast = ast_makeNode(AST_EXPR_CALL);
    ast->callExpr.symbol = ORCRETAIN(symbol);
    ast->callExpr.params = ORCRETAIN(params);
    ast->sourceRange = source_rangeUnion(symbol->sourceRange, params->sourceRange);
    return ast;
}

AST* ast_makeSubscriptExpr(AST* symbol, AST* subscript) {
    AST* ast = ast_makeNode(AST_EXPR_SUBSCRIPT);
    ast->subscriptExpr.symbol = ORCRETAIN(symbol);
    ast->subscriptExpr.subscript = ORCRETAIN(subscript);
    ast->sourceRange = source_rangeUnion(symbol->sourceRange, subscript->sourceRange);
    return ast;
}

AST* ast_makeNameExpr(const OCToken* symbol) {
    AST* ast = ast_makeNode(AST_EXPR_NAME);
    ast->nameExpr.symbol = ast_copyToken(symbol);
    ast->nameExpr.name = orbit_stringIntern(
        symbol->source->bytes + symbol->sourceLoc.offset,
        symbol->length
    );
    ast->sourceRange = source_rangeFromLength(symbol->sourceLoc, symbol->displayLength);
    return ast;
}

AST* ast_makeConstantExpr(const OCToken* symbol, ASTKind kind) {
    AST* ast = ast_makeNode(kind);
    ast->constantExpr.symbol = ast_copyToken(symbol);
    ast->sourceRange = source_rangeFromLength(symbol->sourceLoc, symbol->displayLength);
    return ast;
}

AST* ast_makeUserTypePooled(OCStringID symbol) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_USER);
    ast->typeExpr.userType.symbol = symbol;
    return ast;
}

AST* ast_makeUserType(const OCToken* symbol) {
    OCStringID id = orbit_stringIntern(
        symbol->source->bytes+symbol->sourceLoc.offset,
        symbol->length
    );
    AST* ast = ast_makeUserTypePooled(id);
    ast->sourceRange = source_rangeFromLength(symbol->sourceLoc, symbol->length);
    return ast;
}

AST* ast_makePrimitiveType(ASTKind kind) {
    AST* ast = ast_makeNode(kind);
    return ast;
}

AST* ast_makeFuncType(AST* returnType, AST* params) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_FUNC);
    ast->typeExpr.funcType.returnType = ORCRETAIN(returnType);
    ast->typeExpr.funcType.params = ORCRETAIN(params);
    return ast;
}

AST* ast_makeArrayType(AST* elementType) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_ARRAY);
    ast->typeExpr.arrayType.elementType = ORCRETAIN(elementType);
    return ast;
}

AST* ast_makeMapType(AST* keyType, AST* elementType) {
    AST* ast = ast_makeNode(AST_TYPEEXPR_MAP);
    ast->typeExpr.mapType.keyType = ORCRETAIN(keyType);
    ast->typeExpr.mapType.elementType = ORCRETAIN(elementType);
    return ast;
}
