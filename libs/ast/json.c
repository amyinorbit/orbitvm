//===--------------------------------------------------------------------------------------------===
// orbit/ast/json.c
// This source is part of Orbit - AST
//
// Created on 2017-09-26 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <orbit/csupport/console.h>
#include <orbit/ast/ast.h>
#include <term/colors.h>

static void orbitASTPrintJSON(FILE* out, const OrbitAST* ast, int depth);

static void _jsonEscaped(FILE* out, const char* string, int length) {
    while(length--) {
        switch(*string) {
            
            case '"':
            fputs("\"", out);
            break;
            
            case '\\':
            fputs("\\\\", out);
            break;
            
            case '/':
            fputs("\\/", out);
            break;
            
            case '\b':
            fputs("\\b", out);
            break;
            
            case '\f':
            fputs("\\f", out);
            break;
            
            case '\n':
            fputs("\\n", out);
            break;
            
            case '\r':
            fputs("\\r", out);
            break;
            
            case '\t':
            fputs("\\t", out);
            break;
            
            default:
            fwrite(string, 1, 1, out);
            break;
        }
        string ++;
    }
}

static void _jsonQuoted(FILE* out, const char* value, int length) {
    fputc('"', out);
    _jsonEscaped(out, value, length);
    fputc('"', out);
}

static void _jsonQuotedPool(FILE* out, OCStringID id) {
    const OCString* string = orbitStringPoolGet(id);
    _jsonQuoted(out, string->data, string->length);
}

static void _jsonKey(FILE* out, const char* key) {
    fputc('"', out);
    _jsonEscaped(out, key, strlen(key));
    fputs("\": ", out);
}

static void _jsonInt(FILE* out, const char* name, int64_t value) {
    _jsonKey(out, name);
    fprintf(out, "%lld", value);
}

static void _jsonDouble(FILE* out, const char* name, double value) {
    _jsonKey(out, name);
    fprintf(out, "%f", value);
}

static void _jsonString(FILE* out, const char* name, const char* value) {
    _jsonKey(out, name);
    _jsonQuoted(out, value, strlen(value));
}

static void _jsonStringPooled(FILE* out, const char* name, OCStringID id) {
    _jsonKey(out, name);
    _jsonQuotedPool(out, id);
}

static void _jsonStringToken(FILE* out, const char* name, OrbitToken token) {
    const char* bytes = token.source->bytes + ORBIT_SLOC_OFFSET(token.sourceLoc);
    _jsonKey(out, name);
    _jsonQuoted(out, bytes, token.length);
}

static void _jsonBool(FILE* out, const char* name, bool value) {
    _jsonKey(out, name);
    fprintf(out, "%s", (value ? "true" : "false"));
}

static void _jsonNext(FILE* out, int depth, bool comma) {
    if(comma)
        fputc(',', out);
    fputc('\n', out);
    fprintf(out, "%*.s", depth * 2, " ");
}

static void _jsonList(FILE* out, const OrbitAST* list, int depth) {
    if(list == NULL) {
        fputs("[]", out);
        return;
    }
    
    fputc('[', out);
    depth += 1;
    _jsonNext(out, depth, false);
    
    while(list != NULL) {
        orbitASTPrintJSON(out, list, depth);
        _jsonNext(out, list->next != NULL ? depth : depth - 1, list->next != NULL);
        list = list->next;
    }
    
    fputc(']', out);
}

static void orbitASTPrintJSON(FILE* out, const OrbitAST* ast, int depth) {
    if(ast == NULL) {
        fputs("null", out);
        return;
    }

    fputc('{', out);
    depth += 1;
    _jsonNext(out, depth, false);
    _jsonString(out, "kind", orbitASTKindString(ast->kind));
    
    if(ORBIT_SRANGE_ISVALID(ast->sourceRange)) {
        _jsonNext(out, depth, true);
        _jsonInt(out, "start", ORBIT_SLOC_OFFSET(ast->sourceRange.start));
        _jsonNext(out, depth, true);
        _jsonInt(out, "end", ORBIT_SLOC_OFFSET(ast->sourceRange.end));
    }
    
    switch(ast->kind) {
    case ORBIT_AST_ASSIGN:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "left"); orbitASTPrintJSON(out, ast->assignStmt.lhs, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "right"); orbitASTPrintJSON(out, ast->assignStmt.rhs, depth);
        break;
            
    case ORBIT_AST_CONDITIONAL:
        _jsonNext(out, depth, true);
        _jsonKey(out, "condition"); orbitASTPrintJSON(out, ast->conditionalStmt.condition, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "if_block"); orbitASTPrintJSON(out, ast->conditionalStmt.ifBody, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "else_block"); orbitASTPrintJSON(out, ast->conditionalStmt.elseBody, depth);
        break;
    
    case ORBIT_AST_FOR_IN:
        _jsonNext(out, depth, true);
        _jsonStringToken(out, "variable", ast->forInLoop.variable);
        _jsonNext(out, depth, true);
        _jsonKey(out, "collection"); orbitASTPrintJSON(out, ast->forInLoop.collection, depth);
        _jsonNext(out, depth, true);
        _jsonKey(out, "body"); orbitASTPrintJSON(out, ast->forInLoop.body, depth);
        break;
    
    case ORBIT_AST_WHILE:
        _jsonNext(out, depth, true);
        _jsonKey(out, "condition"); orbitASTPrintJSON(out, ast->whileLoop.condition, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "body"); orbitASTPrintJSON(out, ast->whileLoop.body, depth);
        break;
        
    case ORBIT_AST_BLOCK:
        _jsonNext(out, depth, true);
        _jsonKey(out, "statements"); _jsonList(out, ast->block.body, depth);
        break;
    
    case ORBIT_AST_BREAK:
    case ORBIT_AST_CONTINUE:
        break;
        
    case ORBIT_AST_RETURN:
    case ORBIT_AST_PRINT:
        _jsonNext(out, depth, true);
        _jsonKey(out, "expression"); orbitASTPrintJSON(out, ast->returnStmt.returnValue, depth);
        break;
        
    // DECLARATIONS
    case ORBIT_AST_DECL_MODULE:
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "name", ast->moduleDecl.symbol);
        _jsonNext(out, depth, true);
        _jsonKey(out, "declarations"); _jsonList(out, ast->moduleDecl.body, depth);
        break;
    
    case ORBIT_AST_DECL_FUNC:
        _jsonNext(out, depth, true);
        // _jsonStringPooled(out, "name", ast->funcDecl.symbol);
        
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "mangled_name", ast->funcDecl.mangledName);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth );
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "parameters"); orbitASTPrintJSON(out, ast->funcDecl.body, depth);
        break;
    
    case ORBIT_AST_DECL_VAR:
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "name", ast->varDecl.name);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        break;
    
    case ORBIT_AST_DECL_STRUCT:
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "name", ast->structDecl.name);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "constructor"); orbitASTPrintJSON(out, ast->structDecl.constructor, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "destructor"); orbitASTPrintJSON(out, ast->structDecl.destructor, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "members"); _jsonList(out, ast->structDecl.fields, depth);
        break;
        
    // EXPRESSIONS
    case ORBIT_AST_EXPR_UNARY:
        _jsonNext(out, depth, true);
        _jsonStringToken(out, "operator", ast->unaryExpr.operator);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "right"); orbitASTPrintJSON(out, ast->unaryExpr.rhs, depth);
        break;
    
    case ORBIT_AST_EXPR_BINARY:
        _jsonNext(out, depth, true);
        _jsonStringToken(out, "operator", ast->binaryExpr.operator);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "left"); orbitASTPrintJSON(out, ast->binaryExpr.lhs, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "right"); orbitASTPrintJSON(out, ast->binaryExpr.rhs, depth);
        break;
    
    case ORBIT_AST_EXPR_CALL:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "callee"); orbitASTPrintJSON(out, ast->callExpr.symbol, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "arguments"); _jsonList(out, ast->callExpr.params, depth);
        break;
        
    case ORBIT_AST_EXPR_SUBSCRIPT:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "callee"); orbitASTPrintJSON(out, ast->subscriptExpr.symbol, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "subscript"); orbitASTPrintJSON(out, ast->callExpr.params, depth);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_BOOL:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        _jsonNext(out, depth, true);
        _jsonBool(out, "value", true); // TODO: PLEASE CHANGE THIS
        break;
    
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        _jsonNext(out, depth, true);
        _jsonInt(out, "value", orbitTokenIntValue(&ast->constantExpr.symbol));
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        _jsonNext(out, depth, true);
        _jsonDouble(out, "value", orbitTokenDoubleValue(&ast->constantExpr.symbol));
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_STRING:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "value", ast->constantExpr.symbol.parsedStringLiteral);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT:
        break;
        
    case ORBIT_AST_EXPR_LAMBDA:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "parameters"); _jsonList(out, ast->lambdaExpr.params, depth);

        _jsonNext(out, depth, true);
        _jsonKey(out, "body"); orbitASTPrintJSON(out, ast->lambdaExpr.body, depth);
        break;
    
    case ORBIT_AST_EXPR_NAME:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "name", ast->nameExpr.name);
        break;
    
    case ORBIT_AST_EXPR_INIT:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        
        _jsonNext(out, depth, true);
        _jsonKey(out, "arguments"); _jsonList(out, ast->initExpr.params, depth);
        break;
        
    case ORBIT_AST_EXPR_I2F:
    case ORBIT_AST_EXPR_F2I:
        _jsonNext(out, depth, true);
        _jsonKey(out, "type"); orbitASTPrintJSON(out, ast->type, depth);
        _jsonNext(out, depth, true);
        _jsonKey(out, "expr"); orbitASTPrintJSON(out, ast->conversionExpr.expr, depth);
        break;
    
    case ORBIT_AST_TYPEEXPR_VOID:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_INT:
    case ORBIT_AST_TYPEEXPR_FLOAT:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_ANY:
        break;
        
    case ORBIT_AST_TYPEEXPR_USER:
        _jsonNext(out, depth, true);
        _jsonStringPooled(out, "name", ast->typeExpr.userType.symbol);
        break;
    
    case ORBIT_AST_TYPEEXPR_FUNC:
        _jsonNext(out, depth, true);
        _jsonKey(out, "parameter_types"); _jsonList(out, ast->typeExpr.funcType.params, depth);
        _jsonNext(out, depth, true);
        _jsonKey(out, "return_type"); orbitASTPrintJSON(out, ast->typeExpr.funcType.returnType, depth);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        _jsonNext(out, depth, true);
        _jsonKey(out, "value_type"); orbitASTPrintJSON(out, ast->typeExpr.arrayType.elementType, depth);
        break;
    
    case ORBIT_AST_TYPEEXPR_MAP:
        _jsonNext(out, depth, true);
        _jsonKey(out, "key_type"); orbitASTPrintJSON(out, ast->typeExpr.mapType.keyType, depth);
        _jsonNext(out, depth, true);
        _jsonKey(out, "value_type"); orbitASTPrintJSON(out, ast->typeExpr.mapType.elementType, depth);
        break;
    }
    depth -= 1;
    _jsonNext(out, depth, false);
    fputc('}', out);
}

void orbitASTJSON(FILE* out, const OrbitAST* ast) {
    if(ast == NULL) { return; }
    if(ast->next) {
        _jsonList(out, ast, 0);
    } else {
        orbitASTPrintJSON(out, ast, 0);
    }
    fputs("\n\n", out);
}
