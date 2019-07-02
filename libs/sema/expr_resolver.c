//===--------------------------------------------------------------------------------------------===
// expr_resolver.c - Implementation of expression type resolving functions
// This source is part of Orbit
//
// Created on 2019-07-02 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "expr_resolver.h"
#include "errors.h"
#include <orbit/ast/builders.h>
#include <orbit/ast/type.h>
#include <orbit/utils/memory.h>
#include <string.h>


// Create shared primitives
static OrbitAST* boolType = NULL;
static OrbitAST* intType = NULL;
static OrbitAST* floatType = NULL;
static OrbitAST* stringType = NULL;

void resolverInit(ExprResolver* self) {
    self->operators = NULL;
    self->count = 0;
    self->capacity = 0;
    
    boolType = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_BOOL));
    intType = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_INT));
    floatType = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_FLOAT));
    stringType = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_STRING));
}

void resolverDeinit(ExprResolver* self) {
    for(OperatorSemData* op = self->operators; op != self->operators + self->count; ++op) {
        ORCRELEASE(op->lhsType);
        ORCRELEASE(op->rhsType);
        ORCRELEASE(op->result);
        ORCRELEASE(op->impl);
    }
    ORBIT_DEALLOC_ARRAY(self->operators, OperatorSemData, self->capacity);
    resolverInit(self);
}

static void resolverEnsure(ExprResolver* self, size_t required) {
    size_t oldCapacity = self->capacity;
    while(required > self->capacity)
        self->capacity = ORBIT_GROW_CAPACITY(self->capacity);
    self->operators = ORBIT_REALLOC_ARRAY(self->operators, OperatorSemData, oldCapacity, self->capacity);
}

static inline void declBinaryMath(Sema* self, OrbitAST* T, OrbitTokenKind op) {
    declareOperator(self, (OperatorSemData){OP_BINARY, ORCRETAIN(T), ORCRETAIN(T), (op), ORCRETAIN(T), NULL});
}

static inline void declComp(Sema* self, OrbitAST* T, OrbitTokenKind op, OrbitAST* U) {
    declareOperator(self, (OperatorSemData){OP_BINARY, ORCRETAIN(T), ORCRETAIN(T), (op), ORCRETAIN(U), NULL});
}

void declareDefaultOperators(Sema* self) {
    
    static const OrbitTokenKind mathBinary[] = {
        ORBIT_TOK_PLUS, ORBIT_TOK_MINUS, ORBIT_TOK_STAR, ORBIT_TOK_SLASH, ORBIT_TOK_PERCENT
    };
    static const size_t mathBinCount = sizeof(mathBinary)/sizeof(OrbitTokenKind);
    
    static const OrbitTokenKind comparison[] = {
        ORBIT_TOK_LT, ORBIT_TOK_LTEQ, ORBIT_TOK_GT, ORBIT_TOK_GTEQ, ORBIT_TOK_EQEQ, ORBIT_TOK_BANGEQ
    };
    static const size_t compCount = sizeof(comparison)/sizeof(OrbitTokenKind);
    
    // This gets quite boring, we could probably automate even more, or use a code generator
    for(int i = 0; i < mathBinCount; ++i) {
        declBinaryMath(self, intType, mathBinary[i]);
        declBinaryMath(self, floatType, mathBinary[i]);
    }
    
    for(int i = 0; i < compCount; ++i) {
        declComp(self, intType, comparison[i], boolType);
        declComp(self, floatType, comparison[i], boolType);
        declComp(self, stringType, comparison[i], boolType);
    }
}

void declareOperator(Sema* self, OperatorSemData op) {
    resolverEnsure(&self->resolver, self->resolver.count + 1);
    self->resolver.operators[self->resolver.count] = op;
    self->resolver.count += 1;
}

bool canConvert(OrbitAST* from, OrbitAST* to) {
    if(!from || !to) return false;
    return orbit_astTypeEquals(from, intType) && orbit_astTypeEquals(to, floatType);
}

OrbitAST* convertExprType(OrbitAST* node, OrbitAST* to) {
    if(!canConvert(node->type, to)) return node;
    OrbitAST* converted = orbit_astMakeI2F(node);
    ORCRELEASE(node);
    return converted;
}

static bool binaryOpMatches(bool strict, OperatorSemData* data, OrbitAST* expr) {
    if(expr->binaryExpr.operator.kind != data->operator) return false;
    if(data->kind != OP_BINARY) return false;
    const OrbitAST* lhs = expr->binaryExpr.lhs;
    const OrbitAST* rhs = expr->binaryExpr.rhs;
    if(strict) {
        return orbit_astTypeEquals(data->lhsType, lhs->type)
            && orbit_astTypeEquals(data->rhsType, rhs->type);
    }
    
    bool lhsMatches = orbit_astTypeEquals(data->lhsType, lhs->type)
                    || (canConvert(lhs->type, data->lhsType));
    bool rhsMatches = orbit_astTypeEquals(data->rhsType, rhs->type)
                    || (canConvert(rhs->type, data->rhsType));
    return lhsMatches && rhsMatches;
}

static OperatorSemData* matchBinaryOp(Sema* self, OrbitAST* expr) {
    if(!expr) return NULL;
    
    OperatorSemData* nonStrict = NULL;
    OperatorSemData* end = self->resolver.operators + self->resolver.count;
    
    for(OperatorSemData* op = self->resolver.operators; op != end; ++op) {
        // op->lhs == NULL if op is unary
        // if(!expr->lhs) continue;
        if(binaryOpMatches(true, op, expr)) return op;
        if(binaryOpMatches(false, op, expr)) nonStrict = op;
    }
    if(!nonStrict) return NULL;
    
    expr->binaryExpr.lhs = ORCRETAIN(convertExprType(expr->binaryExpr.lhs, nonStrict->lhsType));
    expr->binaryExpr.rhs = ORCRETAIN(convertExprType(expr->binaryExpr.rhs, nonStrict->rhsType));
    return nonStrict;
}

OrbitAST* resolveBinaryExpr(Sema* self, OrbitAST* expr) {
    OperatorSemData* op = matchBinaryOp(self, expr);
    if(!op) {
        errorBinary(self, expr); 
        return expr->binaryExpr.lhs;
    }
    expr->type = ORCRETAIN(orbit_astTypeCopy(op->result));
    return expr->type;
}

OrbitAST* unaryExprResult(Sema* self, OrbitTokenKind op, OrbitAST* rhs) {
    return NULL;
}
