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
    
    if(argc != 2) {
        fprintf(stderr, "Invalid format\n");
        return -1;
    }
    
    FILE* f = fopen(args[1], "r");
    if(!f) {
        fprintf(stderr, "error opening `%s`\n", args[1]);
        return -1; 
    }

    fseek(f, 0, SEEK_END);
    uint64_t length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* source = malloc((length+1) * sizeof(char));
    fread(source, sizeof(char), length, f);
    source[length] = '\0';
    fclose(f);
    
    OCLexer lex;
    lexer_init(&lex, args[1], source, length);
    
    lexer_nextToken(&lex);
    while(lex.currentToken.type != TOKEN_EOF) {
        lexer_nextToken(&lex);
    }
    free(source);
    return 0;
}
