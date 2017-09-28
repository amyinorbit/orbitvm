//
//  orbit/parser/parser.c
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdarg.h>
#include <orbit/utils/assert.h>
#include <orbit/console/console.h>
#include <orbit/ast/ast_builders.h>
#include <orbit/parser/parser.h>
#include <orbit/parser/tokens.h>
#include <orbit/parser/lexer.h>
#include <orbit/parser/compiler_utils.h>

#include "recognizers.h"

// MARK: - Basic RD parser utilities

static inline int precedence(OCParser* parser) {
    return orbit_binaryPrecedence(parser->lexer.currentToken.type);
}

static inline bool rightAssoc(OCParser* parser) {
    return orbit_binaryRightAssoc(parser->lexer.currentToken.type);
}

static inline OCToken current(OCParser* parser) {
    return parser->lexer.currentToken;
}

static void compilerError(OCParser* parser, const char* fmt, ...) {
    OASSERT(parser != NULL, "Null instance error");
    if(parser->recovering) { return; }
    parser->recovering = true;
    
    fprintf(stderr, "%s:%llu:%llu: ",
                     parser->lexer.path,
                     parser->lexer.currentToken.line,
                     parser->lexer.currentToken.column);
    console_setColor(stderr, CLI_RED);
    fprintf(stderr, "error: ");
    console_setColor(stderr, CLI_RESET);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fputc('\n', stderr);
    lexer_printLine(stderr, &parser->lexer);
    console_printUnderlines(stderr, parser->lexer.currentToken.column,
                                    parser->lexer.currentToken.displayWidth);
}

static void syntaxError(OCParser* parser, OCTokenType type) {
    OASSERT(parser != NULL, "Null instance error");
    if(parser->recovering) { return; }
    parser->recovering = true;
    
    OCToken tok  = current(parser);
    fprintf(stderr, "%s:%llu:%llu: ", parser->lexer.path, tok.line, tok.column);
    console_setColor(stderr, CLI_RED);
    fprintf(stderr, "error: ");
    console_setColor(stderr, CLI_RESET);
    fprintf(stderr, "expected '%s'\n", orbit_tokenString(type));
    lexer_printLine(stderr, &parser->lexer);
    console_printUnderlines(stderr, tok.column, parser->lexer.currentToken.displayWidth);
}

// MARK: - RD Basics

static inline bool have(OCParser* parser, OCTokenType type) {
    return parser->lexer.currentToken.type == type;
}

static inline bool match(OCParser* parser, OCTokenType type) {
    if(have(parser, type)) {
        lexer_nextToken(&parser->lexer);
        return true;
    }
    return false;
}

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
static inline bool haveBinaryOp(OCParser* parser) {
    return orbit_isBinaryOp(parser->lexer.currentToken.type);
}

static inline bool haveUnaryOp(OCParser* parser) {
    return orbit_isUnaryOp(parser->lexer.currentToken.type);
}

static inline bool haveConditional(OCParser* parser) {
    return have(parser, TOKEN_IF)
        || have(parser, TOKEN_FOR)
        || have(parser, TOKEN_WHILE);
}

static inline bool haveTerm(OCParser* parser) {
    return haveUnaryOp(parser)
        || have(parser, TOKEN_LPAREN)
        || have(parser, TOKEN_IDENTIFIER)
        || have(parser, TOKEN_STRING_LITERAL)
        || have(parser, TOKEN_INTEGER_LITERAL)
        || have(parser, TOKEN_FLOAT_LITERAL);
}

static inline bool havePrimitiveType(OCParser* parser) {
    return have(parser, TOKEN_NUMBER)
        || have(parser, TOKEN_BOOL)
        || have(parser, TOKEN_BOOL)
        || have(parser, TOKEN_STRING)
        || have(parser, TOKEN_NIL)
        || have(parser, TOKEN_VOID)
        || have(parser, TOKEN_ANY);
}

static inline bool haveType(OCParser* parser) {
    return havePrimitiveType(parser)
        || have(parser, TOKEN_MAYBE)
        || have(parser, TOKEN_LPAREN)
        || have(parser, TOKEN_ARRAY)
        || have(parser, TOKEN_MAP)
        || have(parser, TOKEN_IDENTIFIER);
}

// Few functions to allow optional semicolons, swift-style.
// https://stackoverflow.com/questions/17646002
//

static inline bool implicitTerminator(OCParser* parser) {
    return parser->lexer.currentToken.startOfLine
        || have(parser, TOKEN_EOF)
        || have(parser, TOKEN_RBRACE);
}

static bool expectTerminator(OCParser* parser) {
    if(parser->recovering) {
        while(!have(parser, TOKEN_SEMICOLON) && !have(parser, TOKEN_EOF) && !implicitTerminator(parser)) {
            lexer_nextToken(&parser->lexer);
        }
        if(have(parser, TOKEN_EOF)) {
            return false;
        }
        parser->recovering = false;
        return have(parser, TOKEN_SEMICOLON) ? match(parser, TOKEN_SEMICOLON) : true;
    } else {
        if(match(parser, TOKEN_SEMICOLON) || implicitTerminator(parser)) {
            return true;
        }
        syntaxError(parser, TOKEN_SEMICOLON);
        return false;
    }
}

static bool expect(OCParser* parser, OCTokenType type) {
    if(parser->recovering) {
        while(!have(parser, type) && !have(parser, TOKEN_EOF)) {
            lexer_nextToken(&parser->lexer);
        }
        if(have(parser, TOKEN_EOF)) {
            return false;
        }
        parser->recovering = false;
        return match(parser, type);
    } else {
        if(match(parser, type)) { return true; }
        syntaxError(parser, type);
        return false;
    }
}

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
    return ast_listClose(&block);
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
        AST* rhs = recExpression(parser, nextMinPrec);
        expr = ast_makeBinaryExpr(&operator, expr, rhs);
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
        term = ast_makeConstantExpr(&symbol);
    else if(match(parser, TOKEN_INTEGER_LITERAL))
        term = ast_makeConstantExpr(&symbol);
    else if(match(parser, TOKEN_FLOAT_LITERAL))
        term = ast_makeConstantExpr(&symbol);
    else
        compilerError(parser, "expected an expression term");
    
    return unary ? ast_makeUnaryExpr(&operator, term) : term;
}

static AST* recName(OCParser* parser) {
    OCToken symbol = current(parser);
    AST* name = ast_makeNameExpr(&symbol);
    expect(parser, TOKEN_IDENTIFIER);
    for(;;) {
        OCToken operator = current(parser);
        if(have(parser, TOKEN_LBRACKET)) {
            AST* rhs = recSubscript(parser);
            name = ast_makeSubscriptExpr(name, rhs);
        }
        else if(have(parser, TOKEN_DOT)) {
            AST* rhs = recFieldAccess(parser);
            name = ast_makeBinaryExpr(&operator, name, rhs);
        }
        else if(have(parser, TOKEN_LPAREN)) {
            AST* params = recFuncCall(parser);
            name = ast_makeCallExpr(name, params);
        }
        else {
            break;
        }
    }
    return name;
}

static AST* recSubscript(OCParser* parser) {
    expect(parser, TOKEN_LBRACKET);
    AST* subscript = recExpression(parser, 0);
    expect(parser, TOKEN_RBRACKET);
    return subscript;
}

static AST* recFieldAccess(OCParser* parser) {
    expect(parser, TOKEN_DOT);
    OCToken symbol = current(parser);
    expect(parser, TOKEN_IDENTIFIER);
    return ast_makeNameExpr(&symbol);
}

static AST* recFuncCall(OCParser* parser) {
    AST* paramList = NULL;
    expect(parser, TOKEN_LPAREN);
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
        return ast_makeTypeExpr(&symbol);
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
    OCToken symbol = current(parser);
    if(have(parser, TOKEN_NUMBER))
        expect(parser, TOKEN_NUMBER);
    else if(have(parser, TOKEN_BOOL))
        expect(parser, TOKEN_BOOL);
    else if(have(parser, TOKEN_STRING))
        expect(parser, TOKEN_STRING);
    else if(have(parser, TOKEN_NIL))
        expect(parser, TOKEN_NIL);
    else if(have(parser, TOKEN_VOID))
        expect(parser, TOKEN_VOID);
    else if(have(parser, TOKEN_ANY))
        expect(parser, TOKEN_ANY);
    else
        compilerError(parser, "expected a primitive type");
    return ast_makeTypeExpr(&symbol);
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

void orbit_dumpTokens(const char* sourcePath, const char* source, uint64_t length) {
    OCLexer lex;
    lexer_init(&lex, sourcePath, source, length);
    
    lexer_nextToken(&lex);
    while(lex.currentToken.type != TOKEN_EOF) {
        OCToken tok = lex.currentToken;
        printf("%20s\t'%.*s'", orbit_tokenName(tok.type), (int)tok.length, tok.start);
        if(tok.startOfLine) {
            printf(" [line start]");
        }
        putchar('\n');
        lexer_nextToken(&lex);
    }
}

AST* orbit_parse(const char* sourcePath, const char* source, uint64_t length) {
    
    OCParser parser;
    parser.recovering = false;
    lexer_init(&parser.lexer, sourcePath, source, length);
    
    lexer_nextToken(&parser.lexer);
    AST* ast = recProgram(&parser);
    return ast;
}
