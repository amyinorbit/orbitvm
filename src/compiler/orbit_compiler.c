//
//  orbit_compiler.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <string.h>
#include <orbit/orbit_utils.h>
#include <orbit/orbit_utf8.h>
#include "orbit_lexer.h"

int main(int argc, const char** args) {
    
    const char* source = "var 😀 = \"Hello, キルラキル!\"";
    uint64_t length = strlen(source);
    
    DBG("Compiling `%s`", source);
    
    OCLexer lex;
    
    lexer_init(&lex, source, length);
    
    codepoint_t point;
    
    while((point = lex.currentChar) > 0) {
        char utf[6];
        int8_t length = utf8_writeCodepoint(point, utf, 6);
        if(length > 0) {
            utf[length] = '\0';
            printf("char: %s\n", utf);
        }
        lexer_nextChar(&lex);
    }
    return 0;
}
