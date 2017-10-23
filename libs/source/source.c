//
//  orbit/source/source.c
//  Orbit - Source Handling
//
//  Created by Amy Parent on 2017-10-23.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdlib.h>
#include <orbit/source/source.h>

/// Creates a source handler by opening the file at [path] and reading its bytes.
OCSource source_readFromPath(const char* path) {
    
    FILE* f = fopen(path, "r");
    if(!f) {
        // TODO: Crash Here
    }
    OCSource source = source_readFromFile(f);
    source.path = path;
    return source;
}

/// Creates a source handler by reading the bytes of [file].
OCSource source_readFromFile(FILE* file) {
    OCSource source;
    
    fseek(file, 0, SEEK_END);
    uint64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // TODO: replace with crash-on-fail malloc from utils
    char* bytes = malloc((length+1) * sizeof(char));
    fread(bytes, sizeof(char), length, file);
    bytes[length] = '\0';
    fclose(file);
    
    source.path = "";
    source.bytes = bytes;
    source.length = length;
    
    return source;
}

/// Deallocates the memory used to store the bytes in [source].
void source_close(OCSource* source) {
    
}
