//===--------------------------------------------------------------------------------------------===
// mangle.c - implementation of Orbit type name mangling
// This source is part of Orbit - Mangling
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <inttypes.h>
#include <orbit/utils/utf8.h>
#include <orbit/mangling/mangle.h>

static uint64_t _encodedLength(const char* name, uint64_t length) {
    uint64_t idx = 0;
    uint64_t encodedLength = 0;
    while(idx < length) {
        codepoint_t c = utf8_getCodepoint(name + idx, length - idx);
        if(c < 0) { return 0; }
        
        idx += utf8_codepointSize(c);
        encodedLength += c < 256 ? 1 : 7;
    }
    return encodedLength;
}

static void _mangleNameLength(const char* name, uint64_t length, OCStringBuffer* buffer) {
    char lenStr[5];
    uint64_t encodedLength = _encodedLength(name, length);
    if(!encodedLength) return;
    
    uint64_t lenLen = snprintf(lenStr, 4, "%" PRIu64, encodedLength);
    orbit_stringBufferAppendC(buffer, lenStr, lenLen);
    
    uint64_t idx = 0;
    while(idx < length) {
        codepoint_t c = utf8_getCodepoint(name + idx, length - idx);
        idx += utf8_codepointSize(c);
        
        if(c < 256) orbit_stringBufferAppend(buffer, c);
        else {
            orbit_stringBufferAppend(buffer, '%');
            char hex[10];
            uint64_t hexLen = snprintf(hex, 10, "%06X", c);
            orbit_stringBufferAppendC(buffer, hex, hexLen);
        }
    }
}

static void orbit_mangleList(AST* head, OCStringBuffer* buffer, codepoint_t start) {
    if(!head) return;
    orbit_stringBufferAppend(buffer, start);
    orbit_mangleType(head, buffer);
    while(head->next) {
        head = head->next;
        orbit_stringBufferAppend(buffer, '_');
        orbit_mangleType(head, buffer);
    }
    orbit_stringBufferAppend(buffer, 'e');
}

void orbit_mangleType(AST* type, OCStringBuffer* buffer) {
    if(!type) {
        orbit_stringBufferAppend(buffer, 'v');
        return;
    }
    
    switch (type->kind) {
        case AST_TYPEEXPR_ANY:      orbit_stringBufferAppendC(buffer, "N*", 2); break;
        case AST_TYPEEXPR_BOOL:     orbit_stringBufferAppendC(buffer, "Nb", 2); break;
        case AST_TYPEEXPR_STRING:   orbit_stringBufferAppendC(buffer, "Ns", 2); break;
        case AST_TYPEEXPR_NUMBER:   orbit_stringBufferAppendC(buffer, "Nd", 2); break;
        case AST_TYPEEXPR_VOID:     orbit_stringBufferAppend(buffer, 'v');      break;
            
        case AST_TYPEEXPR_ARRAY:
            orbit_stringBufferAppendC(buffer, "at", 2);
            orbit_mangleType(type->typeExpr.arrayType.elementType, buffer);
            orbit_stringBufferAppend(buffer, 'e');
            break;
        case AST_TYPEEXPR_MAP:
            orbit_stringBufferAppendC(buffer, "mt", 2);
            orbit_mangleType(type->typeExpr.mapType.keyType, buffer);
            orbit_stringBufferAppend(buffer, '_');
            orbit_mangleType(type->typeExpr.mapType.elementType, buffer);
            orbit_stringBufferAppend(buffer, 'e');
            break;
            
        case AST_TYPEEXPR_FUNC:
            orbit_stringBufferAppendC(buffer, "f", 1);
            orbit_mangleList(type->typeExpr.funcType.params, buffer, 'p');
            orbit_mangleType(type->typeExpr.funcType.returnType, buffer);
            break;
            
        case AST_TYPEEXPR_USER:
            orbit_stringBufferAppend(buffer, 'U');
            OCString* name = orbit_stringPoolGet(type->typeExpr.userType.symbol);
            _mangleNameLength(name->data, name->length, buffer);
            break;
        default:
            break;
    }
}

OCStringID orbit_mangleFuncName(AST* decl) {
    OCStringBuffer buffer;
    orbit_stringBufferInit(&buffer, 128);
    
    OCString* name = orbit_stringPoolGet(decl->funcDecl.name);
    
    orbit_stringBufferAppendC(&buffer, "_O", 2);
    _mangleNameLength(name->data, name->length, &buffer);
    orbit_mangleList(decl->type->typeExpr.funcType.params,&buffer, 'p');
    orbit_mangleType(decl->type->typeExpr.funcType.returnType, &buffer);
    //orbit_stringBufferAppend(&buffer, '_');
    
    OCStringID id = orbit_stringBufferIntern(&buffer);
    orbit_stringBufferDeinit(&buffer);
    return id;
}
