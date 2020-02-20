//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_builders.c
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
#include <orbit/ast/builders.h>

void orbitASTListStart(ASTListBuilder* builder) {
    builder->head = NULL;
    builder->next = &builder->head;
}

void orbitASTListAdd(ASTListBuilder* builder, OrbitAST* item) {
    if(item == NULL) { return; }
    if(builder->head != NULL) { ORCRETAIN(item); }
    *(builder->next) = item;
    builder->next = &item->next;
}

OrbitAST* orbitASTListClose(ASTListBuilder* builder) {
    *(builder->next) = NULL;
    return builder->head;
}


static OrbitToken ast_copyToken(const OrbitToken* token) {
    OrbitToken copy = *token;
    // TODO: deep copy of pointed-to string?
    return copy;
}

OrbitAST* orbitASTMakeAssign(const OrbitToken* operator, OrbitAST* lhs, OrbitAST* rhs) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_ASSIGN);
    
    ast->assignStmt.operator = ast_copyToken(operator);
    ast->assignStmt.lhs = ORCRETAIN(lhs);
    ast->assignStmt.rhs = ORCRETAIN(rhs);
    if(lhs && rhs)
        ast->sourceRange = orbitSrangeUnion(lhs->sourceRange, rhs->sourceRange);
    return ast;
}

OrbitAST* orbitASTMakeConditional(OrbitAST* condition, OrbitAST* ifBody, OrbitAST* elseBody) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_CONDITIONAL);
    ast->conditionalStmt.condition = ORCRETAIN(condition);
    ast->conditionalStmt.ifBody = ORCRETAIN(ifBody);
    ast->conditionalStmt.elseBody = ORCRETAIN(elseBody);
    return ast;
}

OrbitAST* orbitASTMakeForInLoop(const OrbitToken* var, OrbitAST* collection, OrbitAST* body) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_FOR_IN);
    ast->forInLoop.variable = ast_copyToken(var);
    ast->forInLoop.collection = ORCRETAIN(collection);
    ast->forInLoop.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbitASTMakeWhileLoop(OrbitAST* condition, OrbitAST* body) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_WHILE);
    ast->whileLoop.condition = ORCRETAIN(condition);
    ast->whileLoop.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbitASTMakeBlock(OrbitAST* body) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_BLOCK);
    ast->block.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbitASTMakeBreak() {
    return orbitASTMake(ORBIT_AST_BREAK);
}

OrbitAST* orbitASTMakeContinue() {
    return orbitASTMake(ORBIT_AST_CONTINUE);
}

OrbitAST* orbitASTMakeReturn(OrbitAST* returned) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_RETURN);
    ast->returnStmt.returnValue = ORCRETAIN(returned);
    return ast;
}

OrbitAST* orbitASTMakePrint(OrbitAST* expr) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_PRINT);
    ast->printStmt.expr = ORCRETAIN(expr);
    return ast;
}

OrbitAST* orbitASTMakeModuleDecl(const char* symbol, OrbitAST* body) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_DECL_MODULE);
    ast->moduleDecl.symbol = orbitStringIntern(symbol, strlen(symbol));
    ast->moduleDecl.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbitASTMakeVarDecl(const OrbitToken* symbol, OrbitAST* typeAnnotation) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_DECL_VAR);
    ast->sourceRange = orbitSrangeFromLength(symbol->sourceLoc, symbol->length);
    ast->varDecl.symbol = ast_copyToken(symbol);
    ast->varDecl.name = orbitStringIntern(
        symbol->source->bytes+ORBIT_SLOC_OFFSET(symbol->sourceLoc),
        symbol->length
    );
    ast->varDecl.typeAnnotation = ORCRETAIN(typeAnnotation);
    return ast;
}

OrbitAST* orbitASTMakeFuncDecl(const OrbitToken* symbol, OrbitAST* returnType, OrbitAST* params, OrbitAST* body) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_DECL_FUNC);
    // TODO: replace with union from name to return type?
    ast->sourceRange = orbitSrangeFromLength(symbol->sourceLoc, symbol->length);
    ast->funcDecl.symbol = ast_copyToken(symbol);
    ast->funcDecl.name = orbitStringIntern(
        symbol->source->bytes + ORBIT_SLOC_OFFSET(symbol->sourceLoc),
        symbol->length
    );
    ast->funcDecl.mangledName = orbitInvalidStringID;
    ast->funcDecl.returnType = ORCRETAIN(returnType);
    ast->funcDecl.params = ORCRETAIN(params);
    ast->funcDecl.body = ORCRETAIN(body);
    return ast;
}

OrbitAST* orbitASTMakeStructDecl(const OrbitToken* symbol, OrbitAST* constructor, OrbitAST* destructor, OrbitAST* fields) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_DECL_STRUCT);
    
    ast->sourceRange = orbitSrangeFromLength(symbol->sourceLoc, symbol->length);
    ast->structDecl.symbol = ast_copyToken(symbol);
    ast->structDecl.name = orbitStringIntern(
        symbol->source->bytes + ORBIT_SLOC_OFFSET(symbol->sourceLoc),
        symbol->length
    );
    ast->structDecl.constructor = ORCRETAIN(constructor);
    ast->structDecl.destructor = ORCRETAIN(destructor);
    ast->structDecl.fields = ORCRETAIN(fields);
    
    return ast;
}

OrbitAST* orbitASTMakeBinaryExpr(const OrbitToken* operator, OrbitAST* lhs, OrbitAST* rhs) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_BINARY);
    
    ast->binaryExpr.operator = ast_copyToken(operator);
    ast->binaryExpr.lhs = ORCRETAIN(lhs);
    ast->binaryExpr.rhs = ORCRETAIN(rhs);
    if(lhs && rhs)
        ast->sourceRange = orbitSrangeUnion(lhs->sourceRange, rhs->sourceRange);
    return ast;
}

OrbitAST* orbitASTMakeUnaryExpr(const OrbitToken* operator, OrbitAST* rhs) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_UNARY);
    ast->unaryExpr.operator = ast_copyToken(operator);
    ast->unaryExpr.rhs = ORCRETAIN(rhs);
    ast->sourceRange = orbitSrangeUnion(
        orbitSrangeFromLength(operator->sourceLoc, operator->length),
        rhs->sourceRange
    );
    return ast;
}

OrbitAST* orbitASTMakeCallExpr(OrbitAST* symbol, OrbitAST* params) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_CALL);
    ast->callExpr.symbol = ORCRETAIN(symbol);
    ast->callExpr.params = ORCRETAIN(params);
    ast->sourceRange = params ?
                        orbitSrangeUnion(symbol->sourceRange, params->sourceRange) :
                        symbol->sourceRange;
    return ast;
}

OrbitAST* orbitASTMakeSubscriptExpr(OrbitAST* symbol, OrbitAST* subscript) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_SUBSCRIPT);
    ast->subscriptExpr.symbol = ORCRETAIN(symbol);
    ast->subscriptExpr.subscript = ORCRETAIN(subscript);
    ast->sourceRange = orbitSrangeUnion(symbol->sourceRange, subscript->sourceRange);
    return ast;
}

OrbitAST* orbitASTMakeNameExpr(const OrbitToken* symbol) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_NAME);
    ast->nameExpr.symbol = ast_copyToken(symbol);
    ast->nameExpr.name = orbitStringIntern(
        symbol->source->bytes + ORBIT_SLOC_OFFSET(symbol->sourceLoc),
        symbol->length
    );
    ast->sourceRange = orbitSrangeFromLength(symbol->sourceLoc, symbol->length);
    return ast;
}

OrbitAST* orbitASTMakeConstantExpr(const OrbitToken* symbol, ASTKind kind) {
    OrbitAST* ast = orbitASTMake(kind);
    ast->constantExpr.symbol = ast_copyToken(symbol);
    ast->sourceRange = orbitSrangeFromLength(symbol->sourceLoc, symbol->length);
    return ast;
}

OrbitAST* orbitASTMakeInitExpr(OrbitAST* type, OrbitAST* params) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_INIT);
    ast->initExpr.type = ORCRETAIN(type);
    ast->initExpr.params = ORCRETAIN(params);
    return ast;
}

OrbitAST* orbitASTMakeLambdaExpr(OrbitAST* params, OrbitAST* body) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_LAMBDA);
    ast->lambdaExpr.params = ORCRETAIN(params);
    ast->lambdaExpr.body = ORCRETAIN(body);
    if(params && body) {
        ast->sourceRange = orbitSrangeUnion(params->sourceRange, body->sourceRange);
        ast->sourceRange.start -= 1;
    }
    return ast;
}

OrbitAST* orbitASTMakeCastExpr(OrbitAST* expr, ASTKind kind) {
    OrbitAST* ast = orbitASTMake(kind);
    ast->conversionExpr.expr = ORCRETAIN(expr);
    return ast;
}

OrbitAST* orbitASTMakeI2F(OrbitAST* expr) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_I2F);
    ast->conversionExpr.expr = ORCRETAIN(expr);
    return ast;
}

OrbitAST* orbitASTMakeF2I(OrbitAST* expr) {
    OrbitAST* ast = orbitASTMake(ORBIT_AST_EXPR_F2I);
    ast->conversionExpr.expr = ORCRETAIN(expr);
    return ast;
}

static OrbitAST* _makeTypeNode(ASTKind kind) {
    OrbitAST* ast = orbitASTMake(kind);
    ast->typeExpr.flags = 0; // We make types R-values by default. L-values are rarer (basically, LHS variables)
    return ast;
}

OrbitAST* orbitASTMakeUserTypePooled(OCStringID symbol) {
    OrbitAST* ast = _makeTypeNode(ORBIT_AST_TYPEEXPR_USER);
    ast->typeExpr.userType.symbol = symbol;
    return ast;
}

OrbitAST* orbitASTMakeUserType(const OrbitToken* symbol) {
    OCStringID id = orbitStringIntern(
        symbol->source->bytes + ORBIT_SLOC_OFFSET(symbol->sourceLoc),
        symbol->length
    );
    OrbitAST* ast = orbitASTMakeUserTypePooled(id);
    ast->sourceRange = orbitSrangeFromLength(symbol->sourceLoc, symbol->length);
    return ast;
}

OrbitAST* orbitASTMakePrimitiveType(ASTKind kind) {
    OrbitAST* ast = _makeTypeNode(kind);
    return ast;
}

OrbitAST* orbitASTMakeFuncType(OrbitAST* returnType, OrbitAST* params) {
    OrbitAST* ast = _makeTypeNode(ORBIT_AST_TYPEEXPR_FUNC);
    ast->typeExpr.funcType.returnType = ORCRETAIN(returnType);
    ast->typeExpr.funcType.params = ORCRETAIN(params);
    return ast;
}

OrbitAST* orbitASTMakeArrayType(OrbitAST* elementType) {
    OrbitAST* ast = _makeTypeNode(ORBIT_AST_TYPEEXPR_ARRAY);
    ast->typeExpr.arrayType.elementType = ORCRETAIN(elementType);
    return ast;
}

OrbitAST* orbitASTMakeMapType(OrbitAST* keyType, OrbitAST* elementType) {
    OrbitAST* ast = _makeTypeNode(ORBIT_AST_TYPEEXPR_MAP);
    ast->typeExpr.mapType.keyType = ORCRETAIN(keyType);
    ast->typeExpr.mapType.elementType = ORCRETAIN(elementType);
    return ast;
}

OrbitAST* orbitASTMakeOptional(OrbitAST* type, bool isOptional) {
    if(isOptional)
        type->typeExpr.flags |= ORBIT_TYPE_OPTIONAL;
    else
        type->typeExpr.flags &= ~ORBIT_TYPE_OPTIONAL;
    return type;
}

OrbitAST* orbitASTMakeConst(OrbitAST* type, bool isConst) {
    if(isConst)
        type->typeExpr.flags |= ORBIT_TYPE_CONST;
    else
        type->typeExpr.flags &= ~ORBIT_TYPE_CONST;
    return type;
}

OrbitAST* orbitASTSetLValue(OrbitAST* type, bool isLValue) {
    if(isLValue)
        type->typeExpr.flags |= ORBIT_TYPE_LVALUE;
    else
        type->typeExpr.flags &= ~ORBIT_TYPE_LVALUE;
    return type;
}

