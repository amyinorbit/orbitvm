//===--------------------------------------------------------------------------------------------===
// demangle.c - implementation of the demangler for Orbit, usinga recursive descent parser
// This source is part of Orbit - Sema
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdbool.h>
#include <orbit/utils/string.h>
#include <orbit/sema/mangle.h>

typedef struct {
    const char* source;
    const char* current;
    uint64_t    length;
} OCMParser;

char peek(OCMParser* parser) {
    if(!parser->current) { return '\0'; }
    return *parser->current;
}

char next(OCMParser* parser) {
    if(parser->current >= parser->source + parser->length) { return *parser->current; }
    parser->current += 1;
    return *parser->current;
}

bool nextIf(OCMParser* parser, char c) {
    if(*parser->current == c) {
        next(parser);
        return true;
    }
    return false;
}

char* sema_demangle(const char* mangledName, uint64_t length) {
    // TODO: implementation
    return NULL;
}
