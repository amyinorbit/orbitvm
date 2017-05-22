//
//  compiler_utils.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include "compiler_utils.h"

typedef struct {
    const char* name;
    const char* string;
} OCTokenData;

static const OCTokenData _tokenData[] = {
    [TOKEN_LPAREN] = {"l_paren", "("},
    [TOKEN_RPAREN] = {"r_paren", ")"},
    [TOKEN_LBRACE] = {"l_brace", "{"},
    [TOKEN_RBRACE] = {"r_brace", "}"},
    [TOKEN_LBRACKET] = {"l_bracket", "["},
    [TOKEN_RBRACKET] = {"r_bracket", "]"},
    [TOKEN_PLUS] = {"plus", "+"},
    [TOKEN_MINUS] = {"plus", "-"},
    [TOKEN_SLASH] = {"slash", "/"},
    [TOKEN_STAR] = {"star", "*"},
    [TOKEN_STARSTAR] = {"starstar", "*"},
    [TOKEN_PERCENT] = {"percent", "%"},
    [TOKEN_CARET] = {"carent", "^"},
    [TOKEN_TILDE] = {"tilde", "~"},
    [TOKEN_AMP] = {"amp", "&"},
    [TOKEN_PIPE] = {"pipe", "|"},
    [TOKEN_BANG] = {"exclaim", "!"},
    [TOKEN_QUESTION] = {"question", "?"},
    [TOKEN_LT] = {"less", "<"},
    [TOKEN_GT] = {"greater", ">"},
    [TOKEN_EQUALS] = {"equal", "="},
    [TOKEN_LTEQ] = {"less_equal", "<="},
    [TOKEN_GTEQ] = {"greater_equal", ">="},
    [TOKEN_EQEQ] = {"equal_equal", "=="},
    [TOKEN_PLUSEQ] = {"plus_equal", "+="},
    [TOKEN_MINUSEQ] = {"minus_equal", "-="},
    [TOKEN_STAREQ] = {"star_equal", "*="},
    [TOKEN_SLASHEQ] = {"slash_equal", "/="},
    [TOKEN_BANGEQ] = {"exclaim_equal", "!="},
    [TOKEN_LTLT] = {"less_less", "<<"},
    [TOKEN_GTGT] = {"greater_greater", ">>"},
    [TOKEN_AMPAMP] = {"amp_amp", "&&"},
    [TOKEN_PIPEPIPE] = {"pipe_pipe", "||"},
    [TOKEN_SEMICOLON] = {"semicolon", ";"},
    [TOKEN_NEWLINE] = {"newline", "\\n"},
    [TOKEN_COLON] = {"colon", ":"},
    [TOKEN_COMMA] = {"comma", ","},
    [TOKEN_DOT] = {"dot", "."},
    [TOKEN_ARROW] = {"arrow", "->"},
    [TOKEN_INTEGER_LITERAL] = {"integer_constant", "integer constant"},
    [TOKEN_FLOAT_LITERAL] = {"float_constant", "floating-point constant"},
    [TOKEN_STRING_LITERAL] = {"string_constant", "string constant"},
    [TOKEN_IDENTIFIER] = {"raw_identifier", "identifier"},
    [TOKEN_FUN] = {"fun_kw", "fun"},
    [TOKEN_VAR] = {"var_kw", "var"},
    [TOKEN_CONST] = {"const_kw", "const"},
    [TOKEN_MAYBE] = {"maybe_kw", "maybe"},
    [TOKEN_TYPE] = {"type_kw", "type"},
    [TOKEN_RETURN] = {"return_kw", "return"},
    [TOKEN_FOR] = {"for_kw", "for"},
    [TOKEN_WHILE] = {"while_kw", "while"},
    [TOKEN_BREAK] = {"break_kw", "break"},
    [TOKEN_IN] = {"in_kw", "in"},
    [TOKEN_IF] = {"if_kw", "if"},
    [TOKEN_ELSE] = {"else_kw", "else"},
    [TOKEN_INIT] = {"init_kw", "init"},
    [TOKEN_FAIL] = {"fail_kw", "fail"},
    [TOKEN_RANGE] = {"range_kw", "range"},
    [TOKEN_NUMBER] = {"number_kw", "Number"},
    [TOKEN_BOOL] = {"bool_kw", "Bool"},
    [TOKEN_STRING] = {"string_kw", "String"},
    [TOKEN_NIL] = {"nil_Kw", "Nil"},
    [TOKEN_ANY] = {"any_kw", "Any"},
    [TOKEN_EOF] = {"end_of_file", "end of file"},
    [TOKEN_INVALID] = {"invalid", "invalid token"},
};

const char* orbit_tokenName(OCTokenType token) {
    if(token > TOKEN_INVALID) { token = TOKEN_INVALID; }
    return _tokenData[token].name;
}
const char* orbit_tokenString(OCTokenType token) {
    if(token > TOKEN_INVALID) { token = TOKEN_INVALID; }
    return _tokenData[token].string;
}

