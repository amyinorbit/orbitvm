//===--------------------------------------------------------------------------------------------===
// orbit/parser/parser.c
// This source is part of Orbit - Parser
//
// Created on 2017-05-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
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
    ASTListBuilder block;
    orbitASTListStart(&block);

    for(;;) {
        if(have(parser, ORBIT_TOK_VAR))
            orbitASTListAdd(&block, recVarDecl(parser));
        // else if(have(parser, ORBIT_TOK_TYPE))
        //     orbitASTListAdd(&block, recTypeDecl(parser));
        else if(have(parser, ORBIT_TOK_FUN))
            orbitASTListAdd(&block, recFuncDecl(parser));
        else if(haveTerm(parser)
            || haveConditional(parser)
            || have(parser, ORBIT_TOK_RETURN)
            || have(parser, ORBIT_TOK_PRINT)
            || have(parser, ORBIT_TOK_BREAK)
            || have(parser, ORBIT_TOK_CONTINUE)
            || have(parser, ORBIT_TOK_LBRACE))
             orbitASTListAdd(&block, recStatement(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, ORBIT_TOK_EOF);
    return orbitASTMakeModuleDecl("Module", orbitASTListClose(&block));
}

static OrbitAST* recBlock(OCParser* parser) {
    ASTListBuilder block;
    orbitASTListStart(&block);
    
    expect(parser, ORBIT_TOK_LBRACE);
    for(;;) {
        if(have(parser, ORBIT_TOK_VAR))
            orbitASTListAdd(&block, recVarDecl(parser));
        else if(haveTerm(parser)
            || haveConditional(parser)
            || have(parser, ORBIT_TOK_RETURN)
            || have(parser, ORBIT_TOK_PRINT)
            || have(parser, ORBIT_TOK_BREAK)
            || have(parser, ORBIT_TOK_CONTINUE)
            || have(parser, ORBIT_TOK_LBRACE))
             orbitASTListAdd(&block, recStatement(parser));
        else
            break;
        expectTerminator(parser);
    }
    expect(parser, ORBIT_TOK_RBRACE);
    return orbitASTMakeBlock(orbitASTListClose(&block));
}

// static OrbitAST* recTypeDecl(OCParser* parser) {
//     expect(parser, ORBIT_TOK_TYPE);
//     OrbitToken symbol = current(parser);
//     expect(parser, ORBIT_TOK_IDENTIFIER);
//
//     OrbitAST* init = NULL;
//     ASTListBuilder fields;
//     orbitASTListStart(&fields);
//
//     expect(parser, ORBIT_TOK_LBRACE);
//     do {
//         if(have(parser, ORBIT_TOK_VAR)) {
//             orbitASTListAdd(&fields, recVarDecl(parser));
//         }
//         else if(have(parser, ORBIT_TOK_INIT)) {
//             // TODO: throw error if init is not null?
//             init = recTypeInitDecl(parser);
//         }
//         expectTerminator(parser);
//     } while(have(parser, ORBIT_TOK_VAR) || have(parser, ORBIT_TOK_INIT));
//     expect(parser, ORBIT_TOK_RBRACE);
//     return orbitASTMakeStructDecl(&symbol, init, NULL, orbitASTListClose(&fields));
// }

// type-init-decl  ::= 'init' '{' block '}'
// static OrbitAST* recTypeInitDecl(OCParser* parser) {
//     //OrbitToken symbol = current(parser);
//     expect(parser, ORBIT_TOK_INIT);
//     OrbitAST* block = recBlock(parser);
//     return block;
// }

// 'var' identifier ((':', type) | ((':', type)? '=' expression))
static OrbitAST* recVarDecl(OCParser* parser) {
    expect(parser, ORBIT_TOK_VAR);
    
    OrbitToken symbol = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    OrbitAST* typeAnnotation = NULL;
    
    if(match(parser, ORBIT_TOK_COLON)) {
        typeAnnotation = recType(parser);
    }
    
    OrbitAST* decl = orbitASTMakeVarDecl(&symbol, typeAnnotation);
    
    OrbitToken operator = parser->currentToken;
    if(match(parser, ORBIT_TOK_EQUALS)) {
        OrbitAST* rhs = recExpression(parser, 0);
        return orbitASTMakeAssign(&operator, decl, rhs);
    }
    return decl;
}

// func-proto '{' block '}'
// 'func' identifier parameters '->' type
static OrbitAST* recFuncDecl(OCParser* parser) {
    expect(parser, ORBIT_TOK_FUN);
    
    OrbitToken symbol = current(parser);

    expect(parser, ORBIT_TOK_IDENTIFIER);
    expect(parser, ORBIT_TOK_EQUALS);
    return recFuncLiteral(parser, &symbol);
}


static OrbitAST* recFuncLiteral(OCParser* parser, const OrbitToken* symbol) {
    OrbitAST* params = NULL;
    if(match(parser, ORBIT_TOK_LPAREN)) {
        if(have(parser, ORBIT_TOK_IDENTIFIER)) {
            params = recParameters(parser);
        }
        expect(parser, ORBIT_TOK_RPAREN);
    }
    
    OrbitAST* returnType = NULL;
    if(match(parser, ORBIT_TOK_ARROW)) {
        returnType = recType(parser);
    }
    OrbitAST* body = recBlock(parser);
    
    return orbitASTMakeFuncDecl(symbol, returnType, params, body);
}


static OrbitAST* recParameters(OCParser* parser) {
    ASTListBuilder params;
    orbitASTListStart(&params);
    
    do {
        OrbitToken symbol = current(parser);
        expect(parser, ORBIT_TOK_IDENTIFIER);
        expect(parser, ORBIT_TOK_COLON);
        
        OrbitAST* type = recType(parser);
        orbitASTListAdd(&params, orbitASTMakeVarDecl(&symbol, type));
    } while(match(parser, ORBIT_TOK_COMMA));
    
    return orbitASTListClose(&params);
}

static OrbitAST* recStatement(OCParser* parser) {
    if(haveConditional(parser))
        return recConditional(parser);
    else if(haveTerm(parser))
        return recExpression(parser, 0);
    else if(have(parser, ORBIT_TOK_RETURN))
        return recReturnStatement(parser);
    else if(have(parser, ORBIT_TOK_PRINT))
        return recPrintStatement(parser);
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
    return orbitASTMakeConditional(condition, ifBody, elseBody);
}

static OrbitAST* recFlowStatement(OCParser* parser) {
    if(match(parser, ORBIT_TOK_BREAK))
        return orbitASTMakeBreak();
    else if(match(parser, ORBIT_TOK_CONTINUE))
        return orbitASTMakeContinue();
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
    return orbitASTMakeReturn(returnValue);
}

static OrbitAST* recPrintStatement(OCParser* parser) {
    expect(parser, ORBIT_TOK_PRINT);
    return orbitASTMakePrint(recExpression(parser, 0));
}

static OrbitAST* recWhileLoop(OCParser* parser) {
    expect(parser, ORBIT_TOK_WHILE);
    OrbitAST* condition = recExpression(parser, 0);
    OrbitAST* body = recBlock(parser);
    return orbitASTMakeWhileLoop(condition, body);
}

static OrbitAST* recForLoop(OCParser* parser) {
    expect(parser, ORBIT_TOK_FOR);
    
    OrbitToken var = current(parser);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    expect(parser, ORBIT_TOK_IN);
    OrbitAST* collection = recExpression(parser, 0);
    OrbitAST* body = recBlock(parser);
    return orbitASTMakeForInLoop(&var, collection, body);
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
        orbitParserNextToken(parser);
        
        OrbitAST* rhs = NULL;
        
        switch(operator.kind) {
        case ORBIT_TOK_LPAREN:
            rhs = recFuncCall(parser);
            expr = orbitASTMakeCallExpr(expr, rhs);
            break;
            
        // case ORBIT_TOK_LBRACKET:
        //     rhs = recSubscript(parser);
        //     expr = orbitASTMakeSubscriptExpr(expr, rhs);
        //     break;
            
        // case ORBIT_TOK_DOT:
        //     rhs = recFieldAccess(parser);
        //     expr = orbitASTMakeBinaryExpr(&operator, expr, rhs);
        //     break;
            
        case ORBIT_TOK_THEN:
            expr = orbitASTMakeCallExpr(recExpression(parser, nextMinPrec), expr);
            //expr = recFuncApplication(parser, expr);
            break;
            
        default:
            rhs = recExpression(parser, nextMinPrec);
            // TODO: it would be good to catch use of assignments in bad places early instead of
            // punting to Sema. Makes the parser way more complicated however. Swift seems to punt
            // to sema and catches that by using () (no type, Void) as the result of an assignment
            // binary
            expr = orbitTokenIsAssign(operator.kind) ?
                orbitASTMakeAssign(&operator, expr, rhs) : 
                orbitASTMakeBinaryExpr(&operator, expr, rhs);
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
        orbitParserNextToken(parser);
    }
    
    OrbitToken symbol = current(parser);
    
    if(match(parser, ORBIT_TOK_LPAREN)) {
        term = recExpression(parser, 0);
        expect(parser, ORBIT_TOK_RPAREN);
    }
    else if(have(parser, ORBIT_TOK_RSLASH))
        term = recLambda(parser);
    // else if(have(parser, ORBIT_TOK_INIT))
    //     term = recInitExpr(parser);
    else if(have(parser, ORBIT_TOK_IDENTIFIER))
        term = recName(parser);
    else if(match(parser, ORBIT_TOK_STRING_LITERAL))
        term = orbitASTMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_STRING);
    else if(match(parser, ORBIT_TOK_INTEGER_LITERAL))
        term = orbitASTMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_INTEGER);
    else if(match(parser, ORBIT_TOK_FLOAT_LITERAL))
        term = orbitASTMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_FLOAT);
    else if(match(parser, ORBIT_TOK_TRUE) || match(parser, ORBIT_TOK_FALSE))
        term = orbitASTMakeConstantExpr(&symbol, ORBIT_AST_EXPR_CONSTANT_BOOL);
    else
        simpleParseError(parser, "expected an expression term");
    
    return unary ? orbitASTMakeUnaryExpr(&operator, term) : term;
}


// static OrbitAST* recInitExpr(OCParser* parser) {
//     expect(parser, ORBIT_TOK_INIT);
//     OrbitAST* type = recTypename(parser);
//     OrbitAST* paramList = NULL;
//     if(match(parser, ORBIT_TOK_LPAREN)) {
//         paramList = recExprList(parser);
//         expect(parser, ORBIT_TOK_RPAREN);
//     }
//     return orbitASTMakeInitExpr(type, paramList);
// }

static OrbitAST* recName(OCParser* parser) {
    OrbitToken symbol = current(parser);
    OrbitAST* name = orbitASTMakeNameExpr(&symbol);
    expect(parser, ORBIT_TOK_IDENTIFIER);
    return name;
}

// static OrbitAST* recSubscript(OCParser* parser) {
//     OrbitAST* subscript = recExpression(parser, 0);
//     expect(parser, ORBIT_TOK_RBRACKET);
//     return subscript;
// }

// static OrbitAST* recFieldAccess(OCParser* parser) {
//     //expect(parser, ORBIT_TOK_DOT);
//     OrbitToken symbol = current(parser);
//     expect(parser, ORBIT_TOK_IDENTIFIER);
//     return orbitASTMakeNameExpr(&symbol);
// }

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
    orbitASTListStart(&list);
    orbitASTListAdd(&list, recExpression(parser, 0));
    
    while(match(parser, ORBIT_TOK_COMMA)) {
        orbitASTListAdd(&list, recExpression(parser, 0));
    }
    return orbitASTListClose(&list);
}

static OrbitAST* recLambda(OCParser* parser) {
    expect(parser, ORBIT_TOK_RSLASH);
    
    ASTListBuilder params;
    orbitASTListStart(&params);
    // TODO: we need to somehow stay "neutral" about the types in here, and 'instantiate' the
    //          lambda as a concrete function later in the compilation process.
    orbitASTListAdd(&params, recName(parser));
    while(match(parser, ORBIT_TOK_COMMA)) {
        orbitASTListAdd(&params, recName(parser));
    }
    
    expect(parser, ORBIT_TOK_ARROW);
    
    return orbitASTMakeLambdaExpr(orbitASTListClose(&params), recExpression(parser, 0));
}

static OrbitAST* recType(OCParser* parser) {
    bool isOptional = match(parser, ORBIT_TOK_MAYBE);
    return orbitASTMakeOptional(recTypename(parser), isOptional);
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
        return orbitASTMakeUserType(&symbol);
    }
    else
        simpleParseError(parser, "missing type name");
    return NULL;
}

static OrbitAST* recFuncType(OCParser* parser) {
    expect(parser, ORBIT_TOK_LPAREN);
    
    ASTListBuilder params;
    orbitASTListStart(&params);
    
    if(haveType(parser)) {
        orbitASTListAdd(&params, recType(parser));
        while(match(parser, ORBIT_TOK_COMMA)) {
            orbitASTListAdd(&params, recType(parser));
        }
    }
    expect(parser, ORBIT_TOK_RPAREN);
    expect(parser, ORBIT_TOK_ARROW);
    
    OrbitAST* returnType = recType(parser);
    return orbitASTMakeFuncType(returnType, orbitASTListClose(&params));
}

static OrbitAST* recPrimitive(OCParser* parser) {
    //OrbitToken symbol = current(parser);
    if(have(parser, ORBIT_TOK_INT)) {
        expect(parser, ORBIT_TOK_INT);
        // TODO: add support for int/float
        return orbitASTMakePrimitiveType(ORBIT_AST_TYPEEXPR_INT);
    }
    else if(have(parser, ORBIT_TOK_FLOAT)) {
        expect(parser, ORBIT_TOK_FLOAT);
        return orbitASTMakePrimitiveType(ORBIT_AST_TYPEEXPR_FLOAT);
    }
    else if(have(parser, ORBIT_TOK_BOOL)) {
        expect(parser, ORBIT_TOK_BOOL);
        return orbitASTMakePrimitiveType(ORBIT_AST_TYPEEXPR_BOOL);
    }
    else if(have(parser, ORBIT_TOK_STRING)) {
        expect(parser, ORBIT_TOK_STRING);
        return orbitASTMakePrimitiveType(ORBIT_AST_TYPEEXPR_STRING);
    }
    else if(have(parser, ORBIT_TOK_VOID)) {
        expect(parser, ORBIT_TOK_VOID);
        return orbitASTMakePrimitiveType(ORBIT_AST_TYPEEXPR_VOID);
    }
    else if(have(parser, ORBIT_TOK_ANY)) {
        expect(parser, ORBIT_TOK_ANY);
        return orbitASTMakePrimitiveType(ORBIT_AST_TYPEEXPR_ANY);
    }
    simpleParseError(parser, "expected a primitive type");
    return NULL;
}

static OrbitAST* recArrayType(OCParser* parser) {
    expect(parser, ORBIT_TOK_ARRAY);
    expect(parser, ORBIT_TOK_LBRACKET);
    OrbitAST* elementType = recType(parser);
    expect(parser, ORBIT_TOK_RBRACKET);
    
    return orbitASTMakeArrayType(elementType);
}

static OrbitAST* recMapType(OCParser* parser) {
    expect(parser, ORBIT_TOK_MAP);
    expect(parser, ORBIT_TOK_LBRACKET);
    OrbitAST* keyType = recPrimitive(parser);
    expect(parser, ORBIT_TOK_COLON);
    OrbitAST* elementType = recType(parser);
    expect(parser, ORBIT_TOK_RBRACKET);
    
    return orbitASTMakeMapType(keyType, elementType);
}

void orbitDumpTokens(OrbitASTContext* context) {
    OCParser parser;
    orbitParserInit(&parser, context);
    
    orbitParserNextToken(&parser);
    while(parser.currentToken.kind != ORBIT_TOK_EOF) {
        OrbitToken tok = parser.currentToken;
        const char* bytes = context->source.bytes + ORBIT_SLOC_OFFSET(tok.sourceLoc);
        printf("%20s\t'%.*s'", orbitTokenName(tok.kind),
                               (int)tok.length,
                               bytes);
        if(tok.isStartOfLine) {
            printf(" [line start]");
        }
        putchar('\n');
        orbitParserNextToken(&parser);
    }
}

bool orbitParse(OrbitASTContext* context) {
    
    OCParser parser;
    orbitParserInit(&parser, context);
    
    orbitParserNextToken(&parser);
    context->root = ORCRETAIN(recProgram(&parser));
    
    orbitParserDeinit(&parser);
    return context->root != NULL;
}
