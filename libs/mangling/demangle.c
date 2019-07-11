//===--------------------------------------------------------------------------------------------===
// demangle.c - implementation of the demangler for Orbit, usinga recursive descent parser
// This source is part of Orbit - Mangling
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdbool.h>
#include <orbit/ast/builders.h>
#include <orbit/utils/utf8.h>
#include <orbit/csupport/string.h>
#include <orbit/mangling/mangle.h>


#include <stdio.h>

typedef struct {
    const char* source;
    uint64_t    length;
    const char* current;
    bool        failed;
} OCMParser;

void fail(OCMParser* parser) {
    parser->failed = true;
}

char peek(OCMParser* parser) {
    if(parser->failed) { return '\0'; }
    if(!parser->current) { return '\0'; }
    return *parser->current;
}

char next(OCMParser* parser) {
    if(parser->failed) { return '\0'; }
    if(parser->current >= parser->source + parser->length) { return *parser->current; }
    parser->current += 1;
    return *parser->current;
}

bool nextIf(OCMParser* parser, char c) {
    if(parser->failed) { return false; }
    if(*parser->current == c) {
        next(parser);
        return true;
    }
    return false;
}

static void _recTypeList(OCMParser* p, OCStringBuffer* d, char start);
static void _recIdentifier(OCMParser* p, OCStringBuffer* d);

static void _recPrimitiveType(OCMParser* p, OCStringBuffer* d) {
    if(p->failed) { return; }
    switch(peek(p)) {
        case '*': orbitStringBufferAppendC(d, "Any", 3);       break;
        case 'b': orbitStringBufferAppendC(d, "Bool", 4);      break;
        case 'd': orbitStringBufferAppendC(d, "Int", 3);       break;
        case 'f': orbitStringBufferAppendC(d, "Float", 5);     break;
        case 'i': orbitStringBufferAppendC(d, "Int", 3);       break;
        case 's': orbitStringBufferAppendC(d, "String", 6);    break;
        default: fail(p); return;
    }
    next(p);
}

static void _recType(OCMParser* p, OCStringBuffer* d) {
    if(p->failed) { return; }
    switch(peek(p)) {
        case 'N':
            next(p);
            _recPrimitiveType(p, d);
            break;
            
        case 'U':
            next(p);
            _recIdentifier(p, d);
            break;
            
        case 'v':
            next(p);
            orbitStringBufferAppendC(d, "Void", 4);
            break;
            
        case 'f':
            next(p);
            orbitStringBufferAppend(d, '(');
            if(peek(p) == 'p') _recTypeList(p, d, 'p');
            orbitStringBufferAppendC(d, ") -> ", 5);
            _recType(p, d);
            break;
            
        case 'a':
            next(p);
            orbitStringBufferAppendC(d, "Array[", 6);
            _recTypeList(p, d, 't');
            orbitStringBufferAppend(d, ']');
            break;
            
        case 'm':
            next(p);
            orbitStringBufferAppendC(d, "Map[", 4);
            _recTypeList(p, d, 't');
            orbitStringBufferAppend(d, ']');
            break;
            
        default: fail(p); break;
    }
}

static void _recTypeList(OCMParser* p, OCStringBuffer* d, char start) {
    if(p->failed) { return; }
    if(!nextIf(p, start)) {
        fail(p);
        return;
    }
    _recType(p, d);
    while(peek(p) == '_') {
        next(p);
        orbitStringBufferAppendC(d, ", ", 2);
        _recType(p, d);
    }
    if(peek(p) == 'e') {
        next(p);
    } else {
        fail(p);
    }
}


static void _recIdentifier(OCMParser* p, OCStringBuffer* d) {
    if(p->failed) { return; }
    char c = peek(p);
    
    // We need to get the length first to know how much to read.
    uint64_t length = 0;
    while(c >= '0' && c <= '9') {
        length = length * 10 + (c - '0');
        c = next(p);
    }
    
    // Now we parse the actual identifier, and decode unicode characters when needed.
    codepoint_t point = -1;
    
    while(length) {
        if(c == '%') {
            // TODO: decode unicode scalar
            c = next(p);
            point = 0;
            for(int i = 0; i < 6; ++i) {
                if(c >= '0' && c <= '9') {
                    point = point * 16 + (c - '0');
                }
                else if(c >= 'A' && c <= 'F') {
                    point = point * 16 + (10 + (c - 'A'));
                }
                else {
                    fail(p);
                    return;
                }
                c = next(p);
            }
            orbitStringBufferAppend(d, point);
            length -= 7;
        } else {
            orbitStringBufferAppend(d, c);
            c = next(p);
            length -= 1;
        }
    }
}

static void _recFuncName(OCMParser* p, OCStringBuffer* d) {
    if(p->failed) { return; }
    if(!nextIf(p, 'F')) { fail(p); return; }
    _recIdentifier(p, d);
    while(peek(p) >= '0' && peek(p) <= '9') {
        orbitStringBufferAppend(d, '.');
        _recIdentifier(p, d);
    }
    orbitStringBufferAppend(d, '(');
    if(peek(p) == 'p') {
        _recTypeList(p, d, 'p');
    }
    orbitStringBufferAppendC(d, ") -> ", 5);
    _recType(p, d);
}

static void _recVariableName(OCMParser* p, OCStringBuffer* d) {
    if(p->failed) { return; }
    if(!nextIf(p, 'V')) { fail(p); return; }
    _recIdentifier(p, d);
}

OCStringID orbitDemangle(const char* mangledName, uint64_t length) {
    OCStringBuffer demangled;
    OCMParser parser = {
        .failed = false,
        .source = mangledName,
        .length = length,
        .current = mangledName,
    };
    orbitStringBufferInit(&demangled, 256);
    
    OCStringID id = orbitInvalidStringID;
    
    if(!nextIf(&parser, '_')) { goto failure; }
    if(!nextIf(&parser, 'O')) { goto failure; }
    
    switch(peek(&parser)) {
    case 'F':
        _recFuncName(&parser, &demangled);
        break;
        
    case 'V':
        _recVariableName(&parser, &demangled);
        break;
        
    default: break;
    }    
    
    if(parser.failed) { goto failure; }
    
    id = orbitStringBufferIntern(&demangled);
failure:
    orbitStringBufferDeinit(&demangled);
    return id;
    
}
