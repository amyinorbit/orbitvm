//
//  orbit/<#module#>/compiler.c
//  Orbit - 
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/ast.h>
#include <orbit/source/source.h>
#include <orbit/parser/parser.h>
#include <orbit/sema/type.h>

int main(int argc, const char** args) {
    
    if(argc < 2 || argc > 3) {
        fprintf(stderr, "usage: orbitc source_file [-dump-tokens] [-dump-ast]\n");
        return -1;
    }
    
    OCSource source = source_readFromPath(args[1]);
    
    if(argc == 2) {
        AST* ast = ORCRETAIN(orbit_parse(&source));
        ORCRELEASE(ast);
    }
    else {
        if(strcmp(args[2], "-dump-tokens") == 0) {
            orbit_dumpTokens(&source);
        }
        else if(strcmp(args[2], "-dump-ast") == 0) {
            AST* ast = ORCRETAIN(orbit_parse(&source));
            sema_runTypeAnalysis(ast);
            ast_print(stdout, ast);
            ORCRELEASE(ast);
        }
    }
    
    source_close(&source);
    return 0;
}
