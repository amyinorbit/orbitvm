//
//  compiler_utils.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include "compiler_utils.h"

static const char* _ansiCodes[] = {
    [CLI_RESET]     = "\033[0m",
    [CLI_GREEN]     = "\033[32m",
    [CLI_RED]       = "\033[31m",
    [CLI_MAGENTA]   = "\033[35m",
    [CLI_BADCOLOR]  = "",
};

void orbit_setConsoleColor(FILE* out, CLIColor color) {
    // TODO: implement windows equivalent
    if(color > CLI_BADCOLOR) { return; }
    fprintf(out, "%s", _ansiCodes[color]);
}

void orbit_printSquigglies(FILE* out, uint64_t column, uint64_t length) {
    for(uint64_t i = 0; i < column; ++i) {
        fputc(' ', out);
    }
    orbit_setConsoleColor(out, CLI_GREEN);
    fputc('^', out);
    for(uint64_t i = 0; i < length-1; ++i) {
        fputc('~', out);
    }
    orbit_setConsoleColor(out, CLI_RESET);
    fputc('\n', out);
}

typedef struct {
    const char* name;
    const char* string;
    bool        isBinaryOp;
    bool        isUnaryOp;
} OCTokenData;

static const OCTokenData _tokenData[] = {
    [TOKEN_LPAREN] = {"l_paren", "(", false, false},
    [TOKEN_RPAREN] = {"r_paren", ")", false, false},
    [TOKEN_LBRACE] = {"l_brace", "{", false, false},
    [TOKEN_RBRACE] = {"r_brace", "}", false, false},
    [TOKEN_LBRACKET] = {"l_bracket", "[", false, false},
    [TOKEN_RBRACKET] = {"r_bracket", "]", false, false},
    [TOKEN_PLUS] = {"plus", "+", true, false},
    [TOKEN_MINUS] = {"plus", "-", true, true},
    [TOKEN_SLASH] = {"slash", "/", true, false},
    [TOKEN_STAR] = {"star", "*", true, false},
    [TOKEN_STARSTAR] = {"starstar", "**", true, false},
    [TOKEN_PERCENT] = {"percent", "%", true, false},
    [TOKEN_CARET] = {"carent", "^", true, false},
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
    [TOKEN_DOT] = {"dot", ".", false, false},
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
    [TOKEN_NIL] = {"nil_Kw", "Nil", false, false},
    [TOKEN_ANY] = {"any_kw", "Any", false, false},
    [TOKEN_EOF] = {"end_of_file", "end of file", false, false},
    [TOKEN_INVALID] = {"invalid", "invalid token", false, false},
};

const char* orbit_tokenName(OCTokenType token) {
    if(token > TOKEN_INVALID) { token = TOKEN_INVALID; }
    return _tokenData[token].name;
}
const char* orbit_tokenString(OCTokenType token) {
    if(token > TOKEN_INVALID) { token = TOKEN_INVALID; }
    return _tokenData[token].string;
}

bool orbit_isBinaryOp(OCTokenType token) {
    if(token > TOKEN_INVALID) { return false; }
    return _tokenData[token].isBinaryOp;
}

bool orbit_isUnaryOp(OCTokenType token) {
    if(token > TOKEN_INVALID) { return false; }
    return _tokenData[token].isUnaryOp;
}

typedef struct {
    OCTokenType kind;
    int         precedence;
    bool        rightAssoc;
} OCOperator;

static OCOperator opTable[] = {
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

int orbit_binaryPrecedence(OCTokenType token) {
    if(!orbit_isBinaryOp(token)) { return -1; }
    for(int i = 0; opTable[i].kind != TOKEN_INVALID; ++i) {
        if(opTable[i].kind == token) { return opTable[i].precedence; }
    }
    return -1;
}

int orbit_binaryRightAssoc(OCTokenType token) {
    if(!orbit_isBinaryOp(token)) { return false; }
    for(int i = 0; opTable[i].kind != TOKEN_INVALID; ++i) {
        if(opTable[i].kind == token) { return opTable[i].rightAssoc; }
    }
    return false;
}
