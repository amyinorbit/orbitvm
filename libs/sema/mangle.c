//===--------------------------------------------------------------------------------------------===
// mangle.c - implementation of Orbit type name mangling
// This source is part of Orbit - Sema
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <inttypes.h>
#include <orbit/sema/mangle.h>

static void sema_mangleNameLength(const char* name, uint64_t length, OCStringBuffer* buffer) {
    char lenStr[5];
    uint64_t lenNum = snprintf(lenStr, 5, "%" PRIu64, length);
    orbit_stringBufferAppendC(buffer, lenStr, lenNum);
    orbit_stringBufferAppendC(buffer, name, length);
}

static void sema_mangleList(AST* head, OCStringBuffer* buffer, codepoint_t start) {
    if(!head) return;
    orbit_stringBufferAppend(buffer, start);
    sema_mangleType(head, buffer);
    while(head->next) {
        head = head->next;
        orbit_stringBufferAppend(buffer, '_');
        sema_mangleType(head, buffer);
    }
    orbit_stringBufferAppend(buffer, 'e');
}

void sema_mangleType(AST* type, OCStringBuffer* buffer) {
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
            sema_mangleType(type->typeExpr.arrayType.elementType, buffer);
            orbit_stringBufferAppend(buffer, 'e');
            break;
        case AST_TYPEEXPR_MAP:
            orbit_stringBufferAppendC(buffer, "mt", 2);
            sema_mangleType(type->typeExpr.mapType.keyType, buffer);
            orbit_stringBufferAppend(buffer, '_');
            sema_mangleType(type->typeExpr.mapType.elementType, buffer);
            orbit_stringBufferAppend(buffer, 'e');
            break;
            
        case AST_TYPEEXPR_FUNC:
            orbit_stringBufferAppendC(buffer, "f", 1);
            sema_mangleType(type->typeExpr.funcType.returnType, buffer);
            sema_mangleList(type->typeExpr.funcType.params, buffer, 'p');
            break;
            
        case AST_TYPEEXPR_USER:
            orbit_stringBufferAppend(buffer, 'U');
            OCString* name = orbit_stringPoolGet(type->typeExpr.userType.symbol);
            sema_mangleNameLength(name->data, name->length, buffer);
            break;
        default:
            break;
    }
}

OCStringID sema_mangleFuncName(AST* decl) {
    OCStringBuffer buffer;
    orbit_stringBufferInit(&buffer, 128);
    
    OCString* name = orbit_stringPoolGet(decl->funcDecl.name);
    
    orbit_stringBufferAppendC(&buffer, "_O", 2);
    sema_mangleNameLength(name->data, name->length, &buffer);
    sema_mangleType(decl->type->typeExpr.funcType.returnType, &buffer);
    sema_mangleList(decl->type->typeExpr.funcType.params,&buffer, 'p');
    //orbit_stringBufferAppend(&buffer, '_');
    
    OCStringID id = orbit_stringBufferIntern(&buffer);
    orbit_stringBufferDeinit(&buffer);
    return id;
}
