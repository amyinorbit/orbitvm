//===--------------------------------------------------------------------------------------------===
// orbit/parser/parser.c
// This source is part of Orbit - Parser
//
// Created on 2017-05-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/utils/assert.h>
#include <orbit/csupport/console.h>
#include <orbit/ast/builders.h>
#include <orbit/parser/parser.h>
#include <orbit/parser/lexer.h>

#include "recursive_descent.h"
#include "recognizers.h"

// MARK: - Recognizer implementations

static AST* recProgram(OCParser* parser) {
    ASTListBuilder program;
    ast_listStart(&program);
    
    for(;;) {
        if(have(parser, TOKEN_VAR))
            ast_listAdd(&program, recVarDecl(parser));
        else if(have(parser, TOKEN_FUN)) 
            ast_listAdd(&program, recFuncDecl(parser));
        else if(have(parser, TOKEN_TYPE))
            ast_listAdd(&program, recTypeDecl(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, TOKEN_EOF);
    return ast_makeModuleDecl("TranslationUnit", ast_listClose(&program));
}

static AST* recBlock(OCParser* parser) {
    ASTListBuilder block;
    ast_listStart(&block);
    
    expect(parser, TOKEN_LBRACE);
    for(;;) {
        if(have(parser, TOKEN_VAR))
            ast_listAdd(&block, recVarDecl(parser));
        else if(haveTerm(parser)
            || haveConditional(parser)
            || have(parser, TOKEN_RETURN)
            || have(parser, TOKEN_BREAK)
            || have(parser, TOKEN_CONTINUE)
            || have(parser, TOKEN_LBRACE))
             ast_listAdd(&block, recStatement(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, TOKEN_RBRACE);
    return ast_makeBlock(ast_listClose(&block));
}

static AST* recTypeDecl(OCParser* parser) {
    expect(parser, TOKEN_TYPE);
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    
    ASTListBuilder fields;
    ast_listStart(&fields);
    
    expect(parser, TOKEN_LBRACE);
    do {
        ast_listAdd(&fields, recVarDecl(parser));
        expectTerminator(parser);
    } while(have(parser, TOKEN_VAR));
    expect(parser, TOKEN_RBRACE);
    return ast_makeStructDecl(&symbol, NULL, NULL, ast_listClose(&fields));
}

// 'var' identifier ((':', type) | ((':', type)? '=' expression))
static AST* recVarDecl(OCParser* parser) {
    expect(parser, TOKEN_VAR);
    
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    AST* typeAnnotation = NULL;
    
    if(match(parser, TOKEN_COLON)) {
        typeAnnotation = recType(parser);
    }
    
    AST* decl = ast_makeVarDecl(&symbol, typeAnnotation);
    
    OCToken operator = parser->lexer.currentToken;
    if(match(parser, TOKEN_EQUALS)) {
        AST* rhs = recExpression(parser, 0);
        return ast_makeBinaryExpr(&operator, decl, rhs);
    }
    return decl;
}

// func-proto '{' block '}'
// 'func' identifier parameters '->' type
static AST* recFuncDecl(OCParser* parser) {
    expect(parser, TOKEN_FUN);
    
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_LPAREN);
    
    AST* params = NULL;
    if(have(parser, TOKEN_IDENTIFIER)) {
        params = recParameters(parser);
    }
    expect(parser, TOKEN_RPAREN);
    
    AST* returnType = NULL;
    if(match(parser, TOKEN_ARROW)) {
        returnType = recType(parser);
    }
    AST* body = recBlock(parser);
    
    return ast_makeFuncDecl(&symbol, returnType, params, body);
}


static AST* recParameters(OCParser* parser) {
    ASTListBuilder params;
    ast_listStart(&params);
    
    do {
        OCToken symbol = current(parser);
        expect(parser, TOKEN_IDENTIFIER);
        expect(parser, TOKEN_COLON);
        
        AST* type = recType(parser);
        ast_listAdd(&params, ast_makeVarDecl(&symbol, type));
    } while(match(parser, TOKEN_COMMA));
    
    return ast_listClose(&params);
}

static AST* recStatement(OCParser* parser) {
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
        compilerError(parser, "expected a statement");
    return NULL;
}

static AST* recConditional(OCParser* parser) {
    if(have(parser, TOKEN_IF))
        return recIfStatement(parser);
    else if(have(parser, TOKEN_WHILE))
        return recWhileLoop(parser);
    else if(have(parser, TOKEN_FOR))
        return recForLoop(parser);
    else
        compilerError(parser, "expected an if statement or a loop");
    return NULL;
}

static AST* recIfStatement(OCParser* parser) {
    expect(parser, TOKEN_IF);
    AST* condition = recExpression(parser, 0);
    AST* ifBody = recBlock(parser);
    AST* elseBody = NULL;
    
    if(match(parser, TOKEN_ELSE)) {
        if(have(parser, TOKEN_LBRACE))
            elseBody = recBlock(parser);
        else if(have(parser, TOKEN_IF))
            elseBody = recIfStatement(parser);
        else
            compilerError(parser, "expected block or if statement");
    }
    return ast_makeConditional(condition, ifBody, elseBody);
}

static AST* recFlowStatement(OCParser* parser) {
    if(match(parser, TOKEN_BREAK))
        return ast_makeBreak();
    else if(match(parser, TOKEN_CONTINUE))
        return ast_makeContinue();
    else
        compilerError(parser, "expected break or continue");
    return NULL;
        
}

static AST* recReturnStatement(OCParser* parser) {
    AST* returnValue = NULL;
    expect(parser, TOKEN_RETURN);
    if(haveTerm(parser)) {
        returnValue = recExpression(parser, 0);
    }
    return ast_makeReturn(returnValue);
}

static AST* recWhileLoop(OCParser* parser) {
    expect(parser, TOKEN_WHILE);
    AST* condition = recExpression(parser, 0);
    AST* body = recBlock(parser);
    return ast_makeWhileLoop(condition, body);
}

static AST* recForLoop(OCParser* parser) {
    expect(parser, TOKEN_FOR);
    
    OCToken var = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_IN);
    AST* collection = recExpression(parser, 0);
    AST* body = recBlock(parser);
    return ast_makeForInLoop(&var, collection, body);
}

// We don't embed precedence in the grammar - instead any expression
// is parsed using precedence-climbing
// http://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
static AST* recExpression(OCParser* parser, int minPrec) {
    AST* expr = recTerm(parser);
    for(;;) {
        
        if(!haveBinaryOp(parser) || precedence(parser) < minPrec) {
            break;
        }
        
        OCToken operator = current(parser);
        int prec = precedence(parser);
        bool right = rightAssoc(parser);
        
        int nextMinPrec = right ? prec : prec + 1;
        lexer_nextToken(&parser->lexer);
        
        AST* rhs = NULL;
        
        switch(operator.kind) {
        case TOKEN_LPAREN:
            rhs = recFuncCall(parser);
            expr = ast_makeCallExpr(expr, rhs);
            break;
            
        case TOKEN_LBRACKET:
            rhs = recSubscript(parser);
            expr = ast_makeSubscriptExpr(expr, rhs);
            break;
            
        case TOKEN_DOT:
            rhs = recFieldAccess(parser);
            expr = ast_makeBinaryExpr(&operator, expr, rhs);
            break;
            
        default:
            rhs = recExpression(parser, nextMinPrec);
            expr = ast_makeBinaryExpr(&operator, expr, rhs);
            break;
        }
    }
    return expr;
}

static AST* recTerm(OCParser* parser) {
    AST* term = NULL;
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
        term = ast_makeConstantExpr(&symbol, AST_EXPR_CONSTANT_STRING);
    else if(match(parser, TOKEN_INTEGER_LITERAL))
        term = ast_makeConstantExpr(&symbol, AST_EXPR_CONSTANT_INTEGER);
    else if(match(parser, TOKEN_FLOAT_LITERAL))
        term = ast_makeConstantExpr(&symbol, AST_EXPR_CONSTANT_FLOAT);
    else
        compilerError(parser, "expected an expression term");
    
    return unary ? ast_makeUnaryExpr(&operator, term) : term;
}

static AST* recName(OCParser* parser) {
    OCToken symbol = current(parser);
    AST* name = ast_makeNameExpr(&symbol);
    expect(parser, TOKEN_IDENTIFIER);
    return name;
}

static AST* recSubscript(OCParser* parser) {
    AST* subscript = recExpression(parser, 0);
    expect(parser, TOKEN_RBRACKET);
    return subscript;
}

static AST* recFieldAccess(OCParser* parser) {
    //expect(parser, TOKEN_DOT);
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    return ast_makeNameExpr(&symbol);
}

static AST* recFuncCall(OCParser* parser) {
    AST* paramList = NULL;
    if(haveTerm(parser)) {
        paramList = recExprList(parser);
    }
    expect(parser, TOKEN_RPAREN);
    return paramList;
}

static AST* recExprList(OCParser* parser) {
    
    ASTListBuilder list;
    ast_listStart(&list);
    ast_listAdd(&list, recExpression(parser, 0));
    
    while(match(parser, TOKEN_COMMA)) {
        ast_listAdd(&list, recExpression(parser, 0));
    }
    return ast_listClose(&list);
}

static AST* recType(OCParser* parser) {
    if(match(parser, TOKEN_MAYBE)) {
        //TODO: recognise optionals
    }
    return recTypename(parser);
}


static AST* recTypename(OCParser* parser) {
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
        return ast_makeUserType(&symbol);
    }
    else
        compilerError(parser, "expected a type name");
    return NULL;
}

static AST* recFuncType(OCParser* parser) {
    expect(parser, TOKEN_LPAREN);
    
    ASTListBuilder params;
    ast_listStart(&params);
    
    if(haveType(parser)) {
        ast_listAdd(&params, recType(parser));
        while(match(parser, TOKEN_COMMA)) {
            ast_listAdd(&params, recType(parser));
        }
    }
    expect(parser, TOKEN_RPAREN);
    expect(parser, TOKEN_ARROW);
    
    AST* returnType = recType(parser);
    return ast_makeFuncType(returnType, ast_listClose(&params));
}

static AST* recPrimitive(OCParser* parser) {
    //OCToken symbol = current(parser);
    if(have(parser, TOKEN_NUMBER)) {
        expect(parser, TOKEN_NUMBER);
        return ast_makePrimitiveType(AST_TYPEEXPR_NUMBER);
    }
    else if(have(parser, TOKEN_BOOL)) {
        expect(parser, TOKEN_BOOL);
        return ast_makePrimitiveType(AST_TYPEEXPR_BOOL);
    }
    else if(have(parser, TOKEN_STRING)) {
        expect(parser, TOKEN_STRING);
        return ast_makePrimitiveType(AST_TYPEEXPR_STRING);
    }
    else if(have(parser, TOKEN_VOID)) {
        expect(parser, TOKEN_VOID);
        return ast_makePrimitiveType(AST_TYPEEXPR_VOID);
    }
    else if(have(parser, TOKEN_ANY)) {
        expect(parser, TOKEN_ANY);
        return ast_makePrimitiveType(AST_TYPEEXPR_ANY);
    }
    
    compilerError(parser, "expected a primitive type");
    return NULL;
    //return ast_makeTypeExpr(&symbol);
}

static AST* recArrayType(OCParser* parser) {
    expect(parser, TOKEN_ARRAY);
    expect(parser, TOKEN_LBRACKET);
    AST* elementType = recType(parser);
    expect(parser, TOKEN_RBRACKET);
    
    return ast_makeArrayType(elementType);
}

static AST* recMapType(OCParser* parser) {
    expect(parser, TOKEN_MAP);
    expect(parser, TOKEN_LBRACKET);
    AST* keyType = recPrimitive(parser);
    expect(parser, TOKEN_COLON);
    AST* elementType = recType(parser);
    expect(parser, TOKEN_RBRACKET);
    
    return ast_makeMapType(keyType, elementType);
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

AST* orbit_parse(OCSource* source) {
    
    OCParser parser;
    parser.recovering = false;
    lexer_init(&parser.lexer, source);
    
    lexer_nextToken(&parser.lexer);
    AST* ast = recProgram(&parser);
    lexer_deinit(&parser.lexer);
    return ast;
}
