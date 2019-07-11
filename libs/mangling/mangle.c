//===--------------------------------------------------------------------------------------------===
// mangle.c - implementation of Orbit type name mangling
// This source is part of Orbit - Mangling
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
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
    orbitStringBufferAppendC(buffer, lenStr, lenLen);
    
    uint64_t idx = 0;
    while(idx < length) {
        codepoint_t c = utf8_getCodepoint(name + idx, length - idx);
        idx += utf8_codepointSize(c);
        
        if(c < 256) orbitStringBufferAppend(buffer, c);
        else {
            orbitStringBufferAppend(buffer, '%');
            char hex[10];
            uint64_t hexLen = snprintf(hex, 10, "%06X", c);
            orbitStringBufferAppendC(buffer, hex, hexLen);
        }
    }
}

static void orbitMangleList(OrbitAST* head, OCStringBuffer* buffer, codepoint_t start) {
    if(!head) return;
    orbitStringBufferAppend(buffer, start);
    orbitMangleType(head, buffer);
    while(head->next) {
        head = head->next;
        orbitStringBufferAppend(buffer, '_');
        orbitMangleType(head, buffer);
    }
    orbitStringBufferAppend(buffer, 'e');
}

void orbitMangleType(OrbitAST* type, OCStringBuffer* buffer) {
    if(!type) {
        orbitStringBufferAppend(buffer, 'v');
        return;
    }
    
    switch (type->kind) {
        case ORBIT_AST_TYPEEXPR_ANY:    orbitStringBufferAppendC(buffer, "N*", 2); break;
        case ORBIT_AST_TYPEEXPR_BOOL:   orbitStringBufferAppendC(buffer, "Nb", 2); break;
        case ORBIT_AST_TYPEEXPR_STRING: orbitStringBufferAppendC(buffer, "Ns", 2); break;
        case ORBIT_AST_TYPEEXPR_INT:    orbitStringBufferAppendC(buffer, "Nd", 2); break;
        case ORBIT_AST_TYPEEXPR_FLOAT:  orbitStringBufferAppendC(buffer, "Nf", 2); break;
        case ORBIT_AST_TYPEEXPR_VOID:   orbitStringBufferAppend(buffer, 'v');      break;
            
        case ORBIT_AST_TYPEEXPR_ARRAY:
            orbitStringBufferAppendC(buffer, "at", 2);
            orbitMangleType(type->typeExpr.arrayType.elementType, buffer);
            orbitStringBufferAppend(buffer, 'e');
            break;
        case ORBIT_AST_TYPEEXPR_MAP:
            orbitStringBufferAppendC(buffer, "mt", 2);
            orbitMangleType(type->typeExpr.mapType.keyType, buffer);
            orbitStringBufferAppend(buffer, '_');
            orbitMangleType(type->typeExpr.mapType.elementType, buffer);
            orbitStringBufferAppend(buffer, 'e');
            break;
            
        case ORBIT_AST_TYPEEXPR_FUNC:
            orbitStringBufferAppendC(buffer, "f", 1);
            orbitMangleList(type->typeExpr.funcType.params, buffer, 'p');
            orbitMangleType(type->typeExpr.funcType.returnType, buffer);
            break;
            
        case ORBIT_AST_TYPEEXPR_USER:
            orbitStringBufferAppend(buffer, 'U');
            OCString* name = orbitStringPoolGet(type->typeExpr.userType.symbol);
            _mangleNameLength(name->data, name->length, buffer);
            break;
        default:
            break;
    }
}

OCStringID orbitMangleFuncName(OrbitAST* decl) {
    OCStringBuffer buffer;
    orbitStringBufferInit(&buffer, 128);
    
    OCString* name = orbitStringPoolGet(decl->funcDecl.name);
    
    orbitStringBufferAppendC(&buffer, "_OF", 3);
    _mangleNameLength(name->data, name->length, &buffer);
    orbitMangleList(decl->type->typeExpr.funcType.params,&buffer, 'p');
    orbitMangleType(decl->type->typeExpr.funcType.returnType, &buffer);
    //orbitStringBufferAppend(&buffer, '_');
    
    OCStringID id = orbitStringBufferIntern(&buffer);
    orbitStringBufferDeinit(&buffer);
    return id;
}
