//===--------------------------------------------------------------------------------------------===
// orbit/csupport/tokens.c
// This source is part of Orbit - Compiler Support
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <string.h>
#include <orbit/csupport/tokens.h>
#include <orbit/utils/utf8.h>

void source_printTokenLine(FILE* out, const OCToken token) {
    const char* line = token.source->bytes + token.sourceLoc.offset;
    
    // Backtrack until the beginning of the line...
    while(*line != '\n'&& line != token.source->bytes) {
        line -= 1;
    }
    
    char utf[6];
    
    // ...then print the line itself.
    while(line < token.source->bytes + token.source->length) {
        uint64_t remaining = (token.source->bytes + token.source->length) - line;
        codepoint_t c = utf8_getCodepoint(line, remaining);
        if(c == '\0' || c == '\n') { break; }
        int size = utf8_writeCodepoint(c, utf, 6);
        line += size;
        utf[size] = '\0';
        fprintf(out, "%.*s", size, utf);
    }
    fprintf(out, "\n");
}

typedef struct {
    const char* name;
    const char* string;
    bool        isBinaryOp;
    bool        isUnaryOp;
} OCTokenData;

static const OCTokenData _tokenData[] = {
    [ORBIT_TOK_LPAREN] = {"l_paren", "(", true, false},
    [ORBIT_TOK_RPAREN] = {"r_paren", ")", false, false},
    [ORBIT_TOK_LBRACE] = {"l_brace", "{", false, false},
    [ORBIT_TOK_RBRACE] = {"r_brace", "}", false, false},
    [ORBIT_TOK_LBRACKET] = {"l_bracket", "[", true, false},
    [ORBIT_TOK_RBRACKET] = {"r_bracket", "]", false, false},
    [ORBIT_TOK_PLUS] = {"plus", "+", true, false},
    [ORBIT_TOK_MINUS] = {"plus", "-", true, true},
    [ORBIT_TOK_SLASH] = {"slash", "/", true, false},
    [ORBIT_TOK_STAR] = {"star", "*", true, false},
    [ORBIT_TOK_STARSTAR] = {"starstar", "**", true, false},
    [ORBIT_TOK_PERCENT] = {"percent", "%%", true, false},
    [ORBIT_TOK_CARET] = {"caret", "^", true, false},
    [ORBIT_TOK_TILDE] = {"tilde", "~", false, true},
    [ORBIT_TOK_AMP] = {"amp", "&", true, false},
    [ORBIT_TOK_PIPE] = {"pipe", "|", true, false},
    [ORBIT_TOK_BANG] = {"exclaim", "!", false, true},
    [ORBIT_TOK_QUESTION] = {"question", "?", false, false},
    [ORBIT_TOK_LT] = {"less", "<", true, false},
    [ORBIT_TOK_GT] = {"greater", ">", true, false},
    [ORBIT_TOK_EQUALS] = {"equal", "=", true, false},
    [ORBIT_TOK_LTEQ] = {"less_equal", "<=", true, false},
    [ORBIT_TOK_GTEQ] = {"greater_equal", ">=", true, false},
    [ORBIT_TOK_EQEQ] = {"equal_equal", "==", true, false},
    [ORBIT_TOK_PLUSEQ] = {"plus_equal", "+=", true, false},
    [ORBIT_TOK_MINUSEQ] = {"minus_equal", "-=", true, false},
    [ORBIT_TOK_STAREQ] = {"star_equal", "*=", true, false},
    [ORBIT_TOK_SLASHEQ] = {"slash_equal", "/=", true, false},
    [ORBIT_TOK_BANGEQ] = {"exclaim_equal", "!=", true, false},
    [ORBIT_TOK_LTLT] = {"less_less", "<<", true, false},
    [ORBIT_TOK_GTGT] = {"greater_greater", ">>", true, false},
    [ORBIT_TOK_AMPAMP] = {"amp_amp", "&&", true, false},
    [ORBIT_TOK_PIPEPIPE] = {"pipe_pipe", "||", true, false},
    [ORBIT_TOK_SEMICOLON] = {"semicolon", ";", false, false},
    [ORBIT_TOK_NEWLINE] = {"newline", "\\n", false, false},
    [ORBIT_TOK_COLON] = {"colon", ":", false, false},
    [ORBIT_TOK_COMMA] = {"comma", ",", false, false},
    [ORBIT_TOK_DOT] = {"dot", ".", true, false},
    [ORBIT_TOK_ARROW] = {"arrow", "->", false, false},
    [ORBIT_TOK_INTEGER_LITERAL] = {"integer_constant", "integer constant", false, false},
    [ORBIT_TOK_FLOAT_LITERAL] = {"float_constant", "floating-point constant", false, false},
    [ORBIT_TOK_STRING_LITERAL] = {"string_constant", "string constant", false, false},
    [ORBIT_TOK_IDENTIFIER] = {"raw_identifier", "identifier", false, false},
    [ORBIT_TOK_FUN] = {"fun_kw", "fun", false, false},
    [ORBIT_TOK_VAR] = {"var_kw", "var", false, false},
    [ORBIT_TOK_CONST] = {"const_kw", "const", false, false},
    [ORBIT_TOK_MAYBE] = {"maybe_kw", "maybe", false, false},
    [ORBIT_TOK_TYPE] = {"type_kw", "type", false, false},
    [ORBIT_TOK_RETURN] = {"return_kw", "return", false, false},
    [ORBIT_TOK_FOR] = {"for_kw", "for", false, false},
    [ORBIT_TOK_WHILE] = {"while_kw", "while", false, false},
    [ORBIT_TOK_BREAK] = {"break_kw", "break", false, false},
    [ORBIT_TOK_IN] = {"in_kw", "in", false, false},
    [ORBIT_TOK_IF] = {"if_kw", "if", false, false},
    [ORBIT_TOK_ELSE] = {"else_kw", "else", false, false},
    [ORBIT_TOK_INIT] = {"init_kw", "init", false, false},
    [ORBIT_TOK_FAIL] = {"fail_kw", "fail", false, false},
    [ORBIT_TOK_RANGE] = {"range_kw", "range", false, false},
    [ORBIT_TOK_NUMBER] = {"number_kw", "Number", false, false},
    [ORBIT_TOK_BOOL] = {"bool_kw", "Bool", false, false},
    [ORBIT_TOK_STRING] = {"string_kw", "String", false, false},
    [ORBIT_TOK_NIL] = {"nil_kw", "Nil", false, false},
    [ORBIT_TOK_VOID] = {"void_kw", "Void", false, false},
    [ORBIT_TOK_ARRAY] = {"array_kw", "Array", false, false},
    [ORBIT_TOK_MAP] = {"map_kw", "Map", false, false},
    [ORBIT_TOK_ANY] = {"any_kw", "Any", false, false},
    [ORBIT_TOK_EOF] = {"end_of_file", "end of file", false, false},
    [ORBIT_TOK_INVALID] = {"invalid", "invalid token", false, false},
};

bool source_tokenEquals(OCToken* a, OCToken* b) {
    // TODO: null instance check
    if(a == b) { return true; }
    const char* strA = a->source->bytes + a->sourceLoc.offset;
    const char* strB = b->source->bytes + b->sourceLoc.offset;
    return a->source == b->source
        && a->kind == b->kind
        && a->length == b->length
        && ((a->sourceLoc.offset == b->sourceLoc.offset)
            || (strncmp(strA, strB, a->length) == 0));
}

const char* source_tokenName(OCTokenKind token) {
    if(token > ORBIT_TOK_INVALID) { token = ORBIT_TOK_INVALID; }
    return _tokenData[token].name;
}
const char* source_tokenString(OCTokenKind token) {
    if(token > ORBIT_TOK_INVALID) { token = ORBIT_TOK_INVALID; }
    return _tokenData[token].string;
}

bool source_isBinaryOp(OCTokenKind token) {
    if(token > ORBIT_TOK_INVALID) { return false; }
    return _tokenData[token].isBinaryOp;
}

bool source_isUnaryOp(OCTokenKind token) {
    if(token > ORBIT_TOK_INVALID) { return false; }
    return _tokenData[token].isUnaryOp;
}

typedef struct {
    OCTokenKind kind;
    int         precedence;
    bool        rightAssoc;
} OCOperator;

static OCOperator opTable[] = {
    {ORBIT_TOK_DOT,         110,    false},
    {ORBIT_TOK_LBRACKET,    110,    false},
    {ORBIT_TOK_LPAREN,      110,    false},
    {ORBIT_TOK_STARSTAR,    100,    true},
    {ORBIT_TOK_STAR,        90,     false},
    {ORBIT_TOK_SLASH,       90,     false},
    {ORBIT_TOK_PERCENT,     90,     false},
    {ORBIT_TOK_PLUS,        80,     false},
    {ORBIT_TOK_MINUS,       80,     false},
    {ORBIT_TOK_LTLT,        70,     false},
    {ORBIT_TOK_GTGT,        70,     false},
    {ORBIT_TOK_LT,          60,     false},
    {ORBIT_TOK_GT,          60,     false},
    {ORBIT_TOK_LTEQ,        60,     false},
    {ORBIT_TOK_GTEQ,        60,     false},
    {ORBIT_TOK_EQEQ,        50,     false},
    {ORBIT_TOK_BANGEQ,      50,     false},
    {ORBIT_TOK_AMP,         40,     false},
    {ORBIT_TOK_CARET,       30,     false},
    {ORBIT_TOK_PIPE,        20,     false},
    {ORBIT_TOK_AMPAMP,      10,     false},
    {ORBIT_TOK_PIPEPIPE,    9,      false},
    {ORBIT_TOK_EQUALS,      0,      false},
    {ORBIT_TOK_PLUSEQ,      0,      false},
    {ORBIT_TOK_MINUSEQ,     0,      false},
    {ORBIT_TOK_STAREQ,      0,      false},
    {ORBIT_TOK_SLASHEQ,     0,      false},
    {ORBIT_TOK_INVALID,     -1,     false},
};

int source_binaryPrecedence(OCTokenKind token) {
    if(!source_isBinaryOp(token)) { return -1; }
    for(int i = 0; opTable[i].kind != ORBIT_TOK_INVALID; ++i) {
        if(opTable[i].kind == token) { return opTable[i].precedence; }
    }
    return -1;
}

int source_binaryRightAssoc(OCTokenKind token) {
    if(!source_isBinaryOp(token)) { return false; }
    for(int i = 0; opTable[i].kind != ORBIT_TOK_INVALID; ++i) {
        if(opTable[i].kind == token) { return opTable[i].rightAssoc; }
    }
    return false;
}
