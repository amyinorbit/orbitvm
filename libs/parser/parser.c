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
#include <orbit/ast/diag.h>
#include <orbit/csupport/console.h>
#include <orbit/parser/parser.h>

//#include "lexer.h"
#include "recursive_descent.h"
#include "recognizers.h"

// MARK: - Recognizer implementations

static OrbitAST* recProgram(OCParser* parser) {
    ASTListBuilder program;
    orbit_astListStart(&program);
    
    for(;;) {
        if(have(parser, ORBIT_TOK_VAR))
            orbit_astListAdd(&program, recVarDecl(parser));
        else if(have(parser, ORBIT_TOK_FUN)) 
            orbit_astListAdd(&program, recFuncDecl(parser));
        else if(have(parser, ORBIT_TOK_TYPE))
            orbit_astListAdd(&program, recTypeDecl(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, ORBIT_TOK_EOF);
    return orbit_astMakeModuleDecl("TranslationUnit", orbit_astListClose(&program));
}

static OrbitAST* recBlock(OCParser* parser) {
    ASTListBuilder block;
    orbit_astListStart(&block);
    
    expect(parser, ORBIT_TOK_LBRACE);
    for(;;) {
        if(have(parser, ORBIT_TOK_VAR))
            orbit_astListAdd(&block, recVarDecl(parser));
        else if(haveTerm(parser)
            || haveConditional(parser)
            || have(parser, ORBIT_TOK_RETURN)
            || have(parser, ORBIT_TOK_BREAK)
            || have(parser, ORBIT_TOK_CONTINUE)
            || have(parser, ORBIT_TOK_LBRACE))
             orbit_astListAdd(&block, recStatement(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, ORBIT_TOK_RBRACE);
    return orbit_astMakeBlock(orbit_astListClose(&block));
}

static OrbitAST* recTypeDecl(OCParser* parser) {
    expect(parser, ORBIT_TOK_TYPE);
    OrbitToken symbol = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    
    ASTListBuilder fields;
    orbit_astListStart(&fields);
    
    expect(parser, ORBIT_TOK_LBRACE);
    do {
        orbit_astListAdd(&fields, recVarDecl(parser));
        expectTerminator(parser);
    } while(have(parser, ORBIT_TOK_VAR));
    expect(parser, ORBIT_TOK_RBRACE);
    return orbit_astMakeStructDecl(&symbol, NULL, NULL, orbit_astListClose(&fields));
}

// 'var' identifier ((':', type) | ((':', type)? '=' expression))
static OrbitAST* recVarDecl(OCParser* parser) {
    expect(parser, ORBIT_TOK_VAR);
    
    OrbitToken symbol = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    OrbitAST* typeAnnotation = NULL;
    
    if(match(parser, ORBIT_TOK_COLON)) {
        typeAnnotation = recType(parser);
    }
    
    OrbitAST* decl = orbit_astMakeVarDecl(&symbol, typeAnnotation);
    
    OrbitToken operator = parser->currentToken;
    if(match(parser, ORBIT_TOK_EQUALS)) {
        OrbitAST* rhs = recExpression(parser, 0);
        return orbit_astMakeBinaryExpr(&operator, decl, rhs);
    }
    return decl;
}

// func-proto '{' block '}'
// 'func' identifier parameters '->' type
static OrbitAST* recFuncDecl(OCParser* parser) {
    expect(parser, ORBIT_TOK_FUN);
    
    OrbitToken symbol = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    expect(parser, ORBIT_TOK_LPAREN);
    
    OrbitAST* params = NULL;
    if(have(parser, ORBIT_TOK_IDENTIFIER)) {
        params = recParameters(parser);
    }
    expect(parser, ORBIT_TOK_RPAREN);
    
    OrbitAST* returnType = NULL;
    if(match(parser, ORBIT_TOK_ARROW)) {
        returnType = recType(parser);
    }
    OrbitAST* body = recBlock(parser);
    
    return orbit_astMakeFuncDecl(&symbol, returnType, params, body);
}


static OrbitAST* recParameters(OCParser* parser) {
    ASTListBuilder params;
    orbit_astListStart(&params);
    
    do {
        OrbitToken symbol = current(parser);
        expect(parser, ORBIT_TOK_IDENTIFIER);
        expect(parser, ORBIT_TOK_COLON);
        
        OrbitAST* type = recType(parser);
        orbit_astListAdd(&params, orbit_astMakeVarDecl(&symbol, type));
    } while(match(parser, ORBIT_TOK_COMMA));
    
    return orbit_astListClose(&params);
}

static OrbitAST* recStatement(OCParser* parser) {
    if(haveConditional(parser))
        return recConditional(parser);
    else if(haveTerm(parser))
        return recExpression(parser, 0);
    else if(have(parser, ORBIT_TOK_RETURN))
        return recReturnStatement(parser);
    else if(have(parser, ORBIT_TOK_BREAK) || have(parser, ORBIT_TOK_CONTINUE))
        return recFlowStatement(parser);
    else if(have(parser, ORBIT_TOK_LBRACE))
        return recBlock(parser);
    else
        simpleParseError(parser, "expected a statement");
    return NULL;
}

static OrbitAST* recConditional(OCParser* parser) {
    if(have(parser, ORBIT_TOK_IF))
        return recIfStatement(parser);
    else if(have(parser, ORBIT_TOK_WHILE))
        return recWhileLoop(parser);
    else if(have(parser, ORBIT_TOK_FOR))
        return recForLoop(parser);
    else
        simpleParseError(parser, "expected an if statement or a loop");
    return NULL;
}

static OrbitAST* recIfStatement(OCParser* parser) {
    expect(parser, ORBIT_TOK_IF);
    OrbitAST* condition = recExpression(parser, 0);
    OrbitAST* ifBody = recBlock(parser);
    OrbitAST* elseBody = NULL;
    
    if(match(parser, ORBIT_TOK_ELSE)) {
        if(have(parser, ORBIT_TOK_LBRACE))
            elseBody = recBlock(parser);
        else if(have(parser, ORBIT_TOK_IF))
            elseBody = recIfStatement(parser);
        else
            simpleParseError(parser, "expected an else statement");
    }
    return orbit_astMakeConditional(condition, ifBody, elseBody);
}

static OrbitAST* recFlowStatement(OCParser* parser) {
    if(match(parser, ORBIT_TOK_BREAK))
        return orbit_astMakeBreak();
    else if(match(parser, ORBIT_TOK_CONTINUE))
        return orbit_astMakeContinue();
    else
        simpleParseError(parser, "expected 'break' or 'continue'");
        // TODO: replace with UNREACHEABLE
    return NULL;
        
}

static OrbitAST* recReturnStatement(OCParser* parser) {
    OrbitAST* returnValue = NULL;
    expect(parser, ORBIT_TOK_RETURN);
    if(haveTerm(parser)) {
        returnValue = recExpression(parser, 0);
    }
    return orbit_astMakeReturn(returnValue);
}

static OrbitAST* recWhileLoop(OCParser* parser) {
    expect(parser, ORBIT_TOK_WHILE);
    OrbitAST* condition = recExpression(parser, 0);
    OrbitAST* body = recBlock(parser);
    return orbit_astMakeWhileLoop(condition, body);
}

static OrbitAST* recForLoop(OCParser* parser) {
    expect(parser, ORBIT_TOK_FOR);
    
    OrbitToken var = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    expect(parser, ORBIT_TOK_IN);
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
        
        OrbitToken operator = current(parser);
        int prec = precedence(parser);
        bool right = rightAssoc(parser);
        
        int nextMinPrec = right ? prec : prec + 1;
        orbit_parserNextToken(parser);
        
        OrbitAST* rhs = NULL;
        
        switch(operator.kind) {
        case ORBIT_TOK_LPAREN:
            rhs = recFuncCall(parser);
            expr = orbit_astMakeCallExpr(expr, rhs);
            break;
            
        case ORBIT_TOK_LBRACKET:
            rhs = recSubscript(parser);
            expr = orbit_astMakeSubscriptExpr(expr, rhs);
            break;
            
        case ORBIT_TOK_DOT:
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
    OrbitToken operator;
    bool unary = false;
    
    if(haveUnaryOp(parser)) {
        operator = current(parser);
        unary = true;
        orbit_parserNextToken(parser);
    }
    
    OrbitToken symbol = current(parser);
    
    if(match(parser, ORBIT_TOK_LPAREN)) {
        term = recExpression(parser, 0);
        expect(parser, ORBIT_TOK_RPAREN);
    }
    else if(have(parser, ORBIT_TOK_IDENTIFIER))
        term = recName(parser);
    else if(match(parser, ORBIT_TOK_STRING_LITERAL))
        term = orbit_astMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_STRING);
    else if(match(parser, ORBIT_TOK_INTEGER_LITERAL))
        term = orbit_astMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_INTEGER);
    else if(match(parser, ORBIT_TOK_FLOAT_LITERAL))
        term = orbit_astMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_FLOAT);
    else
        simpleParseError(parser, "expected an expression term");
    
    return unary ? orbit_astMakeUnaryExpr(&operator, term) : term;
}

static OrbitAST* recName(OCParser* parser) {
    OrbitToken symbol = current(parser);
    OrbitAST* name = orbit_astMakeNameExpr(&symbol);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    return name;
}

static OrbitAST* recSubscript(OCParser* parser) {
    OrbitAST* subscript = recExpression(parser, 0);
    expect(parser, ORBIT_TOK_RBRACKET);
    return subscript;
}

static OrbitAST* recFieldAccess(OCParser* parser) {
    //expect(parser, ORBIT_TOK_DOT);
    OrbitToken symbol = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    return orbit_astMakeNameExpr(&symbol);
}

static OrbitAST* recFuncCall(OCParser* parser) {
    OrbitAST* paramList = NULL;
    if(haveTerm(parser)) {
        paramList = recExprList(parser);
    }
    expect(parser, ORBIT_TOK_RPAREN);
    return paramList;
}

static OrbitAST* recExprList(OCParser* parser) {
    
    ASTListBuilder list;
    orbit_astListStart(&list);
    orbit_astListAdd(&list, recExpression(parser, 0));
    
    while(match(parser, ORBIT_TOK_COMMA)) {
        orbit_astListAdd(&list, recExpression(parser, 0));
    }
    return orbit_astListClose(&list);
}

static OrbitAST* recType(OCParser* parser) {
    if(match(parser, ORBIT_TOK_MAYBE)) {
        //TODO: recognise optionals
    }
    return recTypename(parser);
}


static OrbitAST* recTypename(OCParser* parser) {
    if(havePrimitiveType(parser))
        return recPrimitive(parser);
    else if(have(parser, ORBIT_TOK_LPAREN))
        return recFuncType(parser);
    else if(have(parser, ORBIT_TOK_ARRAY))
        return recArrayType(parser);
    else if(have(parser, ORBIT_TOK_MAP))
        return recMapType(parser);
    else if(have(parser, ORBIT_TOK_IDENTIFIER)) {
        OrbitToken symbol = current(parser);
        expect(parser, ORBIT_TOK_IDENTIFIER);
        return orbit_astMakeUserType(&symbol);
    }
    else
        simpleParseError(parser, "missing type name");
    return NULL;
}

static OrbitAST* recFuncType(OCParser* parser) {
    expect(parser, ORBIT_TOK_LPAREN);
    
    ASTListBuilder params;
    orbit_astListStart(&params);
    
    if(haveType(parser)) {
        orbit_astListAdd(&params, recType(parser));
        while(match(parser, ORBIT_TOK_COMMA)) {
            orbit_astListAdd(&params, recType(parser));
        }
    }
    expect(parser, ORBIT_TOK_RPAREN);
    expect(parser, ORBIT_TOK_ARROW);
    
    OrbitAST* returnType = recType(parser);
    return orbit_astMakeFuncType(returnType, orbit_astListClose(&params));
}

static OrbitAST* recPrimitive(OCParser* parser) {
    //OrbitToken symbol = current(parser);
    if(have(parser, ORBIT_TOK_NUMBER)) {
        expect(parser, ORBIT_TOK_NUMBER);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_NUMBER);
    }
    else if(have(parser, ORBIT_TOK_BOOL)) {
        expect(parser, ORBIT_TOK_BOOL);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_BOOL);
    }
    else if(have(parser, ORBIT_TOK_STRING)) {
        expect(parser, ORBIT_TOK_STRING);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_STRING);
    }
    else if(have(parser, ORBIT_TOK_VOID)) {
        expect(parser, ORBIT_TOK_VOID);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_VOID);
    }
    else if(have(parser, ORBIT_TOK_ANY)) {
        expect(parser, ORBIT_TOK_ANY);
        return orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_ANY);
    }
    simpleParseError(parser, "expected a primitive type");
    return NULL;
}

static OrbitAST* recArrayType(OCParser* parser) {
    expect(parser, ORBIT_TOK_ARRAY);
    expect(parser, ORBIT_TOK_LBRACKET);
    OrbitAST* elementType = recType(parser);
    expect(parser, ORBIT_TOK_RBRACKET);
    
    return orbit_astMakeArrayType(elementType);
}

static OrbitAST* recMapType(OCParser* parser) {
    expect(parser, ORBIT_TOK_MAP);
    expect(parser, ORBIT_TOK_LBRACKET);
    OrbitAST* keyType = recPrimitive(parser);
    expect(parser, ORBIT_TOK_COLON);
    OrbitAST* elementType = recType(parser);
    expect(parser, ORBIT_TOK_RBRACKET);
    
    return orbit_astMakeMapType(keyType, elementType);
}

void orbit_dumpTokens(OrbitASTContext* context) {
    OCParser parser;
    orbit_parserInit(&parser, context);
    
    orbit_parserNextToken(&parser);
    while(parser.currentToken.kind != ORBIT_TOK_EOF) {
        OrbitToken tok = parser.currentToken;
        const char* bytes = context->source.bytes + ORBIT_SLOC_OFFSET(tok.sourceLoc);
        printf("%20s\t'%.*s'", orbit_tokenName(tok.kind),
                               (int)tok.length,
                               bytes);
        if(tok.isStartOfLine) {
            printf(" [line start]");
        }
        putchar('\n');
        orbit_parserNextToken(&parser);
    }
}

bool orbit_parse(OrbitASTContext* context) {
    
    OCParser parser;
    orbit_parserInit(&parser, context);
    
    orbit_parserNextToken(&parser);
    context->root = ORCRETAIN(recProgram(&parser));
    
    orbit_parserDeinit(&parser);
    return context->root != NULL;
}
