//===--------------------------------------------------------------------------------------------===
// orbit/source/tokens.c
// This source is part of Orbit - Source Handling
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <string.h>
#include <orbit/source/tokens.h>
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
    [TOKEN_LPAREN] = {"l_paren", "(", true, false},
    [TOKEN_RPAREN] = {"r_paren", ")", false, false},
    [TOKEN_LBRACE] = {"l_brace", "{", false, false},
    [TOKEN_RBRACE] = {"r_brace", "}", false, false},
    [TOKEN_LBRACKET] = {"l_bracket", "[", true, false},
    [TOKEN_RBRACKET] = {"r_bracket", "]", false, false},
    [TOKEN_PLUS] = {"plus", "+", true, false},
    [TOKEN_MINUS] = {"plus", "-", true, true},
    [TOKEN_SLASH] = {"slash", "/", true, false},
    [TOKEN_STAR] = {"star", "*", true, false},
    [TOKEN_STARSTAR] = {"starstar", "**", true, false},
    [TOKEN_PERCENT] = {"percent", "%%", true, false},
    [TOKEN_CARET] = {"caret", "^", true, false},
    [TOKEN_TILDE] = {"tilde", "~", false, true},
    [TOKEN_AMP] = {"amp", "&", true, false},
    [TOKEN_PIPE] = {"pipe", "|", true, false},
    [TOKEN_BANG] = {"exclaim", "!", false, true},
    [TOKEN_QUESTION] = {"question", "?", false, false},
    [TOKEN_LT] = {"less", "<", true, false},
    [TOKEN_GT] = {"greater", ">", true, false},
    [TOKEN_EQUALS] = {"equal", "=", true, false},
    [TOKEN_LTEQ] = {"less_equal", "<=", true, false},
    [TOKEN_GTEQ] = {"greater_equal", ">=", true, false},
    [TOKEN_EQEQ] = {"equal_equal", "==", true, false},
    [TOKEN_PLUSEQ] = {"plus_equal", "+=", true, false},
    [TOKEN_MINUSEQ] = {"minus_equal", "-=", true, false},
    [TOKEN_STAREQ] = {"star_equal", "*=", true, false},
    [TOKEN_SLASHEQ] = {"slash_equal", "/=", true, false},
    [TOKEN_BANGEQ] = {"exclaim_equal", "!=", true, false},
    [TOKEN_LTLT] = {"less_less", "<<", true, false},
    [TOKEN_GTGT] = {"greater_greater", ">>", true, false},
    [TOKEN_AMPAMP] = {"amp_amp", "&&", true, false},
    [TOKEN_PIPEPIPE] = {"pipe_pipe", "||", true, false},
    [TOKEN_SEMICOLON] = {"semicolon", ";", false, false},
    [TOKEN_NEWLINE] = {"newline", "\\n", false, false},
    [TOKEN_COLON] = {"colon", ":", false, false},
    [TOKEN_COMMA] = {"comma", ",", false, false},
    [TOKEN_DOT] = {"dot", ".", true, false},
    [TOKEN_ARROW] = {"arrow", "->", false, false},
    [TOKEN_INTEGER_LITERAL] = {"integer_constant", "integer constant", false, false},
    [TOKEN_FLOAT_LITERAL] = {"float_constant", "floating-point constant", false, false},
    [TOKEN_STRING_LITERAL] = {"string_constant", "string constant", false, false},
    [TOKEN_IDENTIFIER] = {"raw_identifier", "identifier", false, false},
    [TOKEN_FUN] = {"fun_kw", "fun", false, false},
    [TOKEN_VAR] = {"var_kw", "var", false, false},
    [TOKEN_CONST] = {"const_kw", "const", false, false},
    [TOKEN_MAYBE] = {"maybe_kw", "maybe", false, false},
    [TOKEN_TYPE] = {"type_kw", "type", false, false},
    [TOKEN_RETURN] = {"return_kw", "return", false, false},
    [TOKEN_FOR] = {"for_kw", "for", false, false},
    [TOKEN_WHILE] = {"while_kw", "while", false, false},
    [TOKEN_BREAK] = {"break_kw", "break", false, false},
    [TOKEN_IN] = {"in_kw", "in", false, false},
    [TOKEN_IF] = {"if_kw", "if", false, false},
    [TOKEN_ELSE] = {"else_kw", "else", false, false},
    [TOKEN_INIT] = {"init_kw", "init", false, false},
    [TOKEN_FAIL] = {"fail_kw", "fail", false, false},
    [TOKEN_RANGE] = {"range_kw", "range", false, false},
    [TOKEN_NUMBER] = {"number_kw", "Number", false, false},
    [TOKEN_BOOL] = {"bool_kw", "Bool", false, false},
    [TOKEN_STRING] = {"string_kw", "String", false, false},
    [TOKEN_NIL] = {"nil_kw", "Nil", false, false},
    [TOKEN_VOID] = {"void_kw", "Void", false, false},
    [TOKEN_ARRAY] = {"array_kw", "Array", false, false},
    [TOKEN_MAP] = {"map_kw", "Map", false, false},
    [TOKEN_ANY] = {"any_kw", "Any", false, false},
    [TOKEN_EOF] = {"end_of_file", "end of file", false, false},
    [TOKEN_INVALID] = {"invalid", "invalid token", false, false},
};

UTFConstString* source_stringFromToken(OCToken* token) {
    return orbit_cStringConstCopy(token->source->bytes + token->sourceLoc.offset, token->length);
}

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
    if(token > TOKEN_INVALID) { token = TOKEN_INVALID; }
    return _tokenData[token].name;
}
const char* source_tokenString(OCTokenKind token) {
    if(token > TOKEN_INVALID) { token = TOKEN_INVALID; }
    return _tokenData[token].string;
}

bool source_isBinaryOp(OCTokenKind token) {
    if(token > TOKEN_INVALID) { return false; }
    return _tokenData[token].isBinaryOp;
}

bool source_isUnaryOp(OCTokenKind token) {
    if(token > TOKEN_INVALID) { return false; }
    return _tokenData[token].isUnaryOp;
}

typedef struct {
    OCTokenKind kind;
    int         precedence;
    bool        rightAssoc;
} OCOperator;

static OCOperator opTable[] = {
    {TOKEN_DOT,         110,    false},
    {TOKEN_LBRACKET,    110,    false},
    {TOKEN_LPAREN,      110,    false},
    {TOKEN_STARSTAR,    100,    true},
    {TOKEN_STAR,        90,     false},
    {TOKEN_SLASH,       90,     false},
    {TOKEN_PERCENT,     90,     false},
    {TOKEN_PLUS,        80,     false},
    {TOKEN_MINUS,       80,     false},
    {TOKEN_LTLT,        70,     false},
    {TOKEN_GTGT,        70,     false},
    {TOKEN_LT,          60,     false},
    {TOKEN_GT,          60,     false},
    {TOKEN_LTEQ,        60,     false},
    {TOKEN_GTEQ,        60,     false},
    {TOKEN_EQEQ,        50,     false},
    {TOKEN_BANGEQ,      50,     false},
    {TOKEN_AMP,         40,     false},
    {TOKEN_CARET,       30,     false},
    {TOKEN_PIPE,        20,     false},
    {TOKEN_AMPAMP,      10,     false},
    {TOKEN_PIPEPIPE,    9,      false},
    {TOKEN_EQUALS,      0,      false},
    {TOKEN_PLUSEQ,      0,      false},
    {TOKEN_MINUSEQ,     0,      false},
    {TOKEN_STAREQ,      0,      false},
    {TOKEN_SLASHEQ,     0,      false},
    {TOKEN_INVALID,     -1,     false},
};

int source_binaryPrecedence(OCTokenKind token) {
    if(!source_isBinaryOp(token)) { return -1; }
    for(int i = 0; opTable[i].kind != TOKEN_INVALID; ++i) {
        if(opTable[i].kind == token) { return opTable[i].precedence; }
    }
    return -1;
}

int source_binaryRightAssoc(OCTokenKind token) {
    if(!source_isBinaryOp(token)) { return false; }
    for(int i = 0; opTable[i].kind != TOKEN_INVALID; ++i) {
        if(opTable[i].kind == token) { return opTable[i].rightAssoc; }
    }
    return false;
}
