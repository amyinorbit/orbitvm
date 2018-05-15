//===--------------------------------------------------------------------------------------------===
// orbit/parser/parser.c
// This source is part of Orbit - Parser
//
// Created on 2017-05-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <orbit/ast/builders.h>
#include <orbit/csupport/console.h>
#include <orbit/csupport/diag.h>
#include <orbit/parser/parser.h>
#include <orbit/parser/lexer.h>

#include "recursive_descent.h"
#include "recognizers.h"

// MARK: - Recognizer implementations

static OrbitAST* recProgram(OCParser* parser) {
    ASTListBuilder program;
    orbit_astListStart(&program);
    
    for(;;) {
        if(have(parser, TOKEN_VAR))
            orbit_astListAdd(&program, recVarDecl(parser));
        else if(have(parser, TOKEN_FUN)) 
            orbit_astListAdd(&program, recFuncDecl(parser));
        else if(have(parser, TOKEN_TYPE))
            orbit_astListAdd(&program, recTypeDecl(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, TOKEN_EOF);
    return orbit_astMakeModuleDecl("TranslationUnit", orbit_astListClose(&program));
}

static OrbitAST* recBlock(OCParser* parser) {
    ASTListBuilder block;
    orbit_astListStart(&block);
    
    expect(parser, TOKEN_LBRACE);
    for(;;) {
        if(have(parser, TOKEN_VAR))
            orbit_astListAdd(&block, recVarDecl(parser));
        else if(haveTerm(parser)
            || haveConditional(parser)
            || have(parser, TOKEN_RETURN)
            || have(parser, TOKEN_BREAK)
            || have(parser, TOKEN_CONTINUE)
            || have(parser, TOKEN_LBRACE))
             orbit_astListAdd(&block, recStatement(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, TOKEN_RBRACE);
    return orbit_astMakeBlock(orbit_astListClose(&block));
}

static OrbitAST* recTypeDecl(OCParser* parser) {
    expect(parser, TOKEN_TYPE);
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    
    ASTListBuilder fields;
    orbit_astListStart(&fields);
    
    expect(parser, TOKEN_LBRACE);
    do {
        orbit_astListAdd(&fields, recVarDecl(parser));
        expectTerminator(parser);
    } while(have(parser, TOKEN_VAR));
    expect(parser, TOKEN_RBRACE);
    return orbit_astMakeStructDecl(&symbol, NULL, NULL, orbit_astListClose(&fields));
}

// 'var' identifier ((':', type) | ((':', type)? '=' expression))
static OrbitAST* recVarDecl(OCParser* parser) {
    expect(parser, TOKEN_VAR);
    
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    OrbitAST* typeAnnotation = NULL;
    
    if(match(parser, TOKEN_COLON)) {
        typeAnnotation = recType(parser);
    }
    
    OrbitAST* decl = orbit_astMakeVarDecl(&symbol, typeAnnotation);
    
    OCToken operator = parser->lexer.currentToken;
    if(match(parser, TOKEN_EQUALS)) {
        OrbitAST* rhs = recExpression(parser, 0);
        return orbit_astMakeBinaryExpr(&operator, decl, rhs);
    }
    return decl;
}

// func-proto '{' block '}'
// 'func' identifier parameters '->' type
static OrbitAST* recFuncDecl(OCParser* parser) {
    expect(parser, TOKEN_FUN);
    
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_LPAREN);
    
    OrbitAST* params = NULL;
    if(have(parser, TOKEN_IDENTIFIER)) {
        params = recParameters(parser);
    }
    expect(parser, TOKEN_RPAREN);
    
    OrbitAST* returnType = NULL;
    if(match(parser, TOKEN_ARROW)) {
        returnType = recType(parser);
    }
    OrbitAST* body = recBlock(parser);
    
    return orbit_astMakeFuncDecl(&symbol, returnType, params, body);
}


static OrbitAST* recParameters(OCParser* parser) {
    ASTListBuilder params;
    orbit_astListStart(&params);
    
    do {
        OCToken symbol = current(parser);
        expect(parser, TOKEN_IDENTIFIER);
        expect(parser, TOKEN_COLON);
        
        OrbitAST* type = recType(parser);
        orbit_astListAdd(&params, orbit_astMakeVarDecl(&symbol, type));
    } while(match(parser, TOKEN_COMMA));
    
    return orbit_astListClose(&params);
}

static OrbitAST* recStatement(OCParser* parser) {
    if(haveConditional(parser))
        return recConditional(parser);
    else if(haveTerm(parser))
        return recExpression(parser, 0);
    else if(have(parser, TOKEN_RETURN))
        return recReturnStatement(parser);
    else if(have(parser, TOKEN_BREAK) || have(parser, TOKEN_CONTINUE))
        return recFlowStatement(parser);
    else if(have(parser, TOKEN_LBRACE))
        return recBlock(parser);
    else
        orbit_diagEmitError(current(parser).sourceLoc, "expected a statement", 0);
    return NULL;
}

static OrbitAST* recConditional(OCParser* parser) {
    if(have(parser, TOKEN_IF))
        return recIfStatement(parser);
    else if(have(parser, TOKEN_WHILE))
        return recWhileLoop(parser);
    else if(have(parser, TOKEN_FOR))
        return recForLoop(parser);
    else
        orbit_diagEmitError(current(parser).sourceLoc, "expected an if statement or a loop", 0);
    return NULL;
}

static OrbitAST* recIfStatement(OCParser* parser) {
    expect(parser, TOKEN_IF);
    OrbitAST* condition = recExpression(parser, 0);
    OrbitAST* ifBody = recBlock(parser);
    OrbitAST* elseBody = NULL;
    
    if(match(parser, TOKEN_ELSE)) {
        if(have(parser, TOKEN_LBRACE))
            elseBody = recBlock(parser);
        else if(have(parser, TOKEN_IF))
            elseBody = recIfStatement(parser);
        else
            orbit_diagEmitError(current(parser).sourceLoc, "expected block or if statement", 0);
    }
    return orbit_astMakeConditional(condition, ifBody, elseBody);
}

static OrbitAST* recFlowStatement(OCParser* parser) {
    if(match(parser, TOKEN_BREAK))
        return orbit_astMakeBreak();
    else if(match(parser, TOKEN_CONTINUE))
        return orbit_astMakeContinue();
    else
        orbit_diagEmitError(current(parser).sourceLoc, "expected break or continue", 0);
    return NULL;
        
}

static OrbitAST* recReturnStatement(OCParser* parser) {
    OrbitAST* returnValue = NULL;
    expect(parser, TOKEN_RETURN);
    if(haveTerm(parser)) {
        returnValue = recExpression(parser, 0);
    }
    return orbit_astMakeReturn(returnValue);
}

static OrbitAST* recWhileLoop(OCParser* parser) {
    expect(parser, TOKEN_WHILE);
    OrbitAST* condition = recExpression(parser, 0);
    OrbitAST* body = recBlock(parser);
    return orbit_astMakeWhileLoop(condition, body);
}

static OrbitAST* recForLoop(OCParser* parser) {
    expect(parser, TOKEN_FOR);
    
    OCToken var = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_IN);
    OrbitAST* collection = recExpression(parser, 0);
    OrbitAST* body = recBlock(parser);
    return orbit_astMakeForInLoop(&var, collection, body);
}

// We don't embed precedence in the grammar - instead any expression
// is parsed using precedence-climbing
// http://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
static OrbitAST* recExpression(OCParser* parser, int minPrec) {
    OrbitAST* expr = recTerm(parser);
    for(;;) {
        
        if(!haveBinaryOp(parser) || precedence(parser) < minPrec) {
            break;
        }
        
        OCToken operator = current(parser);
        int prec = precedence(parser);
        bool right = rightAssoc(parser);
        
        int nextMinPrec = right ? prec : prec + 1;
        lexer_nextToken(&parser->lexer);
        
        OrbitAST* rhs = NULL;
        
        switch(operator.kind) {
        case TOKEN_LPAREN:
            rhs = recFuncCall(parser);
            expr = orbit_astMakeCallExpr(expr, rhs);
            break;
            
        case TOKEN_LBRACKET:
            rhs = recSubscript(parser);
            expr = orbit_astMakeSubscriptExpr(expr, rhs);
            break;
            
        case TOKEN_DOT:
            rhs = recFieldAccess(parser);
            expr = orbit_astMakeBinaryExpr(&operator, expr, rhs);
            break;
            
        default:
            rhs = recExpression(parser, nextMinPrec);
            expr = orbit_astMakeBinaryExpr(&operator, expr, rhs);
            break;
        }
    }
    return expr;
}

static OrbitAST* recTerm(OCParser* parser) {
    OrbitAST* term = NULL;
    OCToken operator;
    bool unary = false;
    
    if(haveUnaryOp(parser)) {
        operator = current(parser);
        unary = true;
        lexer_nextToken(&parser->lexer);
    }
    
    OCToken symbol = current(parser);
    
    if(match(parser, TOKEN_LPAREN)) {
        term = recExpression(parser, 0);
        expect(parser, TOKEN_RPAREN);
    }
    else if(have(parser, TOKEN_IDENTIFIER))
        term = recName(parser);
    else if(match(parser, TOKEN_STRING_LITERAL))
        term = orbit_astMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_STRING);
    else if(match(parser, TOKEN_INTEGER_LITERAL))
        term = orbit_astMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_INTEGER);
    else if(match(parser, TOKEN_FLOAT_LITERAL))
        term = orbit_astMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_FLOAT);
    else
        orbit_diagEmitError(current(parser).sourceLoc, "expected an expression term", 0);
    
    return unary ? orbit_astMakeUnaryExpr(&operator, term) : term;
}

static OrbitAST* recName(OCParser* parser) {
    OCToken symbol = current(parser);
    OrbitAST* name = orbit_astMakeNameExpr(&symbol);
    expect(parser, TOKEN_IDENTIFIER);
    return name;
}

static OrbitAST* recSubscript(OCParser* parser) {
    OrbitAST* subscript = recExpression(parser, 0);
    expect(parser, TOKEN_RBRACKET);
    return subscript;
}

static OrbitAST* recFieldAccess(OCParser* parser) {
    //expect(parser, TOKEN_DOT);
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    return orbit_astMakeNameExpr(&symbol);
}

static OrbitAST* recFuncCall(OCParser* parser) {
    OrbitAST* paramList = NULL;
    if(haveTerm(parser)) {
        paramList = recExprList(parser);
    }
    expect(parser, TOKEN_RPAREN);
    return paramList;
}

static OrbitAST* recExprList(OCParser* parser) {
    
    ASTListBuilder list;
    orbit_astListStart(&list);
    orbit_astListAdd(&list, recExpression(parser, 0));
    
    while(match(parser, TOKEN_COMMA)) {
        orbit_astListAdd(&list, recExpression(parser, 0));
    }
    return orbit_astListClose(&list);
}

static OrbitAST* recType(OCParser* parser) {
    if(match(parser, TOKEN_MAYBE)) {
        //TODO: recognise optionals
    }
    return recTypename(parser);
}


static OrbitAST* recTypename(OCParser* parser) {
    if(havePrimitiveType(parser))
        return recPrimitive(parser);
    else if(have(parser, TOKEN_LPAREN))
        return recFuncType(parser);
    else if(have(parser, TOKEN_ARRAY))
        return recArrayType(parser);
    else if(have(parser, TOKEN_MAP))
        return recMapType(parser);
    else if(have(parser, TOKEN_IDENTIFIER)) {
        OCToken symbol = current(parser);
        expect(parser, TOKEN_IDENTIFIER);
        return orbit_astMakeUserType(&symbol);
    }
    else
        orbit_diagEmitError(current(parser).sourceLoc, "missing type name", 0);
    return NULL;
}

static OrbitAST* recFuncType(OCParser* parser) {
    expect(parser, TOKEN_LPAREN);
    
    ASTListBuilder params;
    orbit_astListStart(&params);
    
    if(haveType(parser)) {
        orbit_astListAdd(&params, recType(parser));
        while(match(parser, TOKEN_COMMA)) {
            orbit_astListAdd(&params, recType(parser));
        }
    }
    expect(parser, TOKEN_RPAREN);
    expect(parser, TOKEN_ARROW);
    
    OrbitAST* returnType = recType(parser);
    return orbit_astMakeFuncType(returnType, orbit_astListClose(&params));
}

static OrbitAST* recPrimitive(OCParser* parser) {
    //OCToken symbol = current(parser);
    if(have(parser, TOKEN_NUMBER)) {
        expect(parser, TOKEN_NUMBER);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_NUMBER);
    }
    else if(have(parser, TOKEN_BOOL)) {
        expect(parser, TOKEN_BOOL);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_BOOL);
    }
    else if(have(parser, TOKEN_STRING)) {
        expect(parser, TOKEN_STRING);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_STRING);
    }
    else if(have(parser, TOKEN_VOID)) {
        expect(parser, TOKEN_VOID);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_VOID);
    }
    else if(have(parser, TOKEN_ANY)) {
        expect(parser, TOKEN_ANY);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_ANY);
    }
    
    orbit_diagEmitError(current(parser).sourceLoc, "expected a primitive type", 0);
    return NULL;
    //return orbit_astMakeTypeExpr(&symbol);
}

static OrbitAST* recArrayType(OCParser* parser) {
    expect(parser, TOKEN_ARRAY);
    expect(parser, TOKEN_LBRACKET);
    OrbitAST* elementType = recType(parser);
    expect(parser, TOKEN_RBRACKET);
    
    return orbit_astMakeArrayType(elementType);
}

static OrbitAST* recMapType(OCParser* parser) {
    expect(parser, TOKEN_MAP);
    expect(parser, TOKEN_LBRACKET);
    OrbitAST* keyType = recPrimitive(parser);
    expect(parser, TOKEN_COLON);
    OrbitAST* elementType = recType(parser);
    expect(parser, TOKEN_RBRACKET);
    
    return orbit_astMakeMapType(keyType, elementType);
}

void orbit_dumpTokens(OCSource* source) {
    OCLexer lex;
    lexer_init(&lex, source);
    
    lexer_nextToken(&lex);
    while(lex.currentToken.kind != TOKEN_EOF) {
        OCToken tok = lex.currentToken;
        const char* bytes = source->bytes + tok.sourceLoc.offset;
        printf("%20s\t'%.*s'", source_tokenName(tok.kind),
                               (int)tok.length,
                               bytes);
        if(tok.isStartOfLine) {
            printf(" [line start]");
        }
        putchar('\n');
        lexer_nextToken(&lex);
    }
}

OrbitAST* orbit_parse(OCSource* source) {
    
    OCParser parser;
    parser.recovering = false;
    lexer_init(&parser.lexer, source);
    
    lexer_nextToken(&parser.lexer);
    OrbitAST* ast = recProgram(&parser);
    lexer_deinit(&parser.lexer);
    return ast;
}
