//
//  orbit_compiler.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <parser/orbit_parser.h>

static char* loadSource(const char* path, uint64_t* length) {
    FILE* f = fopen(path, "r");
    if(!f) {
        *length = 0;
        return NULL; 
    }

    fseek(f, 0, SEEK_END);
    uint64_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* source = malloc((len+1) * sizeof(char));
    fread(source, sizeof(char), len, f);
    source[len] = '\0';
    fclose(f);
    
    *length = len;
    return source;
}

int main(int argc, const char** args) {
    
    if(argc < 2 || argc > 3) {
        fprintf(stderr, "usage: orbitc source_file [dump-tokens]\n");
        return -1;
    }
    
    bool compile = true;
    
    if(argc == 3) {
        compile = strcmp(args[2], "dump-tokens") != 0;
    }
    
    uint64_t length = 0;
    char* source = loadSource(args[1], &length);
    if(!source) {
        fprintf(stderr, "error opening `%s`\n", args[1]);
        return -1;
    }
    if(compile) {
        orbit_compile(args[1], source, length);
    } else {
        orbit_dumpTokens(args[1], source, length);
    }
    free(source);
    return 0;
}
