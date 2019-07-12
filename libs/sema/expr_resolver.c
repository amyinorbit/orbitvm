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

void resolverInit(ExprResolver* self) {
    self->operators = NULL;
    self->count = 0;
    self->capacity = 0;
}

void resolverDeinit(ExprResolver* self) {
    ORBIT_DEALLOC_ARRAY(self->operators, OperatorSemData, self->capacity);
    self->operators = NULL;
    self->count = 0;
    self->capacity = 0;
}

static void resolverEnsure(ExprResolver* self, size_t required) {
    if(required <= self->capacity) return;
    size_t oldCapacity = self->capacity;
    while(required > self->capacity)
        self->capacity = ORBIT_GROW_CAPACITY(self->capacity);
    self->operators = ORBIT_REALLOC_ARRAY(self->operators, OperatorSemData, oldCapacity, self->capacity);
}

static inline void declBinaryMath(Sema* self, ASTKind T, OrbitTokenKind op) {
    declareOperator(self, (OperatorSemData){OP_BINARY, T, T, op, T});
}

static inline void declComp(Sema* self, ASTKind T, OrbitTokenKind op) {
    declareOperator(self, (OperatorSemData){OP_BINARY, T, T, op, ORBIT_AST_TYPEEXPR_BOOL});
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
        declBinaryMath(self, ORBIT_AST_TYPEEXPR_INT, mathBinary[i]);
        declBinaryMath(self, ORBIT_AST_TYPEEXPR_FLOAT, mathBinary[i]);
    }
    
    for(int i = 0; i < compCount; ++i) {
        declComp(self, ORBIT_AST_TYPEEXPR_INT, comparison[i]);
        declComp(self, ORBIT_AST_TYPEEXPR_FLOAT, comparison[i]);
        // declComp(self, stringType, comparison[i], boolType);
    }
}

void declareOperator(Sema* self, OperatorSemData op) {
    resolverEnsure(&self->resolver, self->resolver.count + 1);
    self->resolver.operators[self->resolver.count] = op;
    self->resolver.count += 1;
}

static const Conversion conversions[] = {
    {ORBIT_AST_TYPEEXPR_INT, ORBIT_AST_TYPEEXPR_FLOAT, true, ORBIT_AST_EXPR_I2F},
    {ORBIT_AST_TYPEEXPR_FLOAT, ORBIT_AST_TYPEEXPR_INT, true, ORBIT_AST_EXPR_F2I},
};
static const size_t count = sizeof(conversions)/sizeof(Conversion);

const Conversion* findCast(const OrbitAST* from, const OrbitAST* to) {
    if(!from || !to) return NULL;
    
    for(const Conversion* cast = conversions; cast != conversions + count; ++cast) {
        if(orbitASTTypeEqualsPrimitive(from, cast->from)
           && orbitASTTypeEqualsPrimitive(to, cast->to)) {
            return cast;
        }
    }
    return NULL;
}

static const Conversion* findCastPrimitive(const OrbitAST* from, ASTKind to) {
    if(!from) return NULL;
    
    for(const Conversion* cast = conversions; cast != conversions + count; ++cast) {
        if(orbitASTTypeEqualsPrimitive(from, cast->from)
           && to == cast->to) {
            return cast;
        }
    }
    return NULL;
}

bool canConvert(OrbitAST* from, OrbitAST* to) {
    return findCast(from, to) != NULL;
}

bool canConvertPrimitive(const OrbitAST* from, ASTKind to) {
    return findCastPrimitive(from, to) != NULL;
}

OrbitAST* convertExprType(OrbitAST* node, OrbitAST* to) {
    if(orbitASTTypeEquals(node->type, to)) return node;
    const Conversion* cast = findCast(node->type, to);
    if(!cast) return NULL;
    
    OrbitAST* converted = ORCRETAIN(orbitASTMakeCastExpr(node, cast->nodeKind));
    converted->type = ORCRETAIN(orbitASTMakePrimitiveType(cast->to));
    ORCRELEASE(node);
    return converted;
}

OrbitAST* convertExprTypePrimitive(OrbitAST* node, ASTKind to) {
    if(orbitASTTypeEqualsPrimitive(node->type, to)) return node;
    const Conversion* cast = findCastPrimitive(node->type, to);
    if(!cast) return NULL; // TODO: we should probably signal, here
    
    OrbitAST* converted = ORCRETAIN(orbitASTMakeCastExpr(node, cast->nodeKind));
    converted->type = ORCRETAIN(orbitASTMakePrimitiveType(cast->to));
    ORCRELEASE(node);
    return converted;
}

static bool binaryOpMatches(bool strict, OperatorSemData* data, OrbitAST* expr) {
    if(expr->binaryExpr.operator.kind != data->operator) return false;
    if(data->kind != OP_BINARY) return false;
    const OrbitAST* lhs = expr->binaryExpr.lhs;
    const OrbitAST* rhs = expr->binaryExpr.rhs;
    if(strict) {
        return orbitASTTypeEqualsPrimitive(lhs->type, data->lhsType)
            && orbitASTTypeEqualsPrimitive(rhs->type, data->rhsType);
    }
    
    bool lhsMatches = orbitASTTypeEqualsPrimitive(lhs->type, data->lhsType)
                      || (canConvertPrimitive(lhs->type, data->lhsType));
    bool rhsMatches = orbitASTTypeEqualsPrimitive(rhs->type, data->rhsType)
                      || (canConvertPrimitive(rhs->type, data->rhsType));
    return lhsMatches && rhsMatches;
}

static const OperatorSemData* matchBinaryOp(Sema* self, OrbitAST* expr) {
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
    
    expr->binaryExpr.lhs = convertExprTypePrimitive(expr->binaryExpr.lhs, nonStrict->lhsType);
    expr->binaryExpr.rhs = convertExprTypePrimitive(expr->binaryExpr.rhs, nonStrict->rhsType);
    return nonStrict;
}

const OrbitAST* resolveBinaryExpr(Sema* self, OrbitAST* expr) {
    const OperatorSemData* op = matchBinaryOp(self, expr);
    if(!op) {
        errorBinary(self, expr); 
        return expr->binaryExpr.lhs;
    }
    expr->type = ORCRETAIN(orbitASTMakePrimitiveType(op->result));
    return expr->type;
}

const OrbitAST* unaryExprResult(Sema* self, OrbitTokenKind op, OrbitAST* rhs) {
    return NULL;
}
