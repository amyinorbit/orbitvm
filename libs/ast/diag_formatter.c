//===--------------------------------------------------------------------------------------------===
// orbit/ast/diag_formatter.c - Implementation of the diagnostics token formatter
// This source is part of Orbit - AST
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <orbit/ast/diag.h>
#include <orbit/ast/type.h>
#include <orbit/csupport/string.h>

#include "diag_private.h"

static const char* scan(const char* str, const char* end, char value) {
    while(*str != value && str != end) {
        str += 1;
    }
    return str;
}

static bool isDigit(codepoint_t c) {
    return c >= '0' && c <= '9';
}

static bool modifierIs(const char* str, uint32_t length, const char* value) {
    if(length != strlen(value)) { return false; }
    return strncmp(str, value, length) == 0;
}

static void handleSelect(OCStringBuffer* buf, const char* arg, uint32_t length, int value) {
    assert(arg && "diagnostic format argument: no parameter");
    assert(length > 0 && "diagnostic format argument: no parameter");
    const char* end = arg + length;
    while(value) {
        const char* next = scan(arg, end, '|');
        assert(next != end && "diagnostic format argument: invalid selection index");
        value -= 1;
        arg = next+1;
    }
    
    end = scan(arg, end, '|');
    orbit_stringBufferAppendC(buf, arg, end-arg);
}

static void handleS(OCStringBuffer* buf, int value) {
    if(value == 1) { return; }
    orbit_stringBufferAppend(buf, 's');
}

static void handlePlural(OCStringBuffer* buf, const char* arg, uint32_t length, int value) {
    handleSelect(buf, arg, length, value == 1 ? 0 : 1);
}

// After a few attempts using rec-descent parsers, this is heavily based on clang's diagnostic
// formatter (lines ~700+ -> https://clang.llvm.org/doxygen/Basic_2Diagnostic_8cpp_source.html)
char* orbit_diagGetFormat(OrbitDiag* diag) {
    // TODO: probably cache this? Though we are unlikely to print diags more than once
    // TODO: We should be safe, but we should probably port all of this to codepoint_t
    
    OCStringBuffer buffer, *buf = &buffer;
    orbit_stringBufferInit(buf, 512);
    const char* fmt = diag->format;
    const char* end = diag->format + strlen(diag->format);
    
    while(fmt != end) {
                
        // If we aren't looking at a $, we can just copy until the first dollar (or the end).
        if(*fmt != '$') {
            const char* strEnd = scan(fmt, end, '$');
            orbit_stringBufferAppendC(buf, fmt, strEnd-fmt);
            fmt = strEnd;
            continue;
        }
        
        // Skip the dollar.
        fmt += 1;
        
        const char* modifier = NULL;
        const char* parameter = NULL;
        uint16_t modifierLength = 0;
        uint16_t parameterLength = 0;
        
        if(!isDigit(*fmt)) {
            modifier = fmt;
            while(*fmt >= 'a' && *fmt <= 'z') { fmt += 1; }
            modifierLength = fmt - modifier;
            
            if(*fmt == '{') {
                fmt += 1;
                parameter = fmt;
                fmt = scan(fmt, end, '}');
                assert(fmt != end && "diagnostic format string: unmatched '{'");
                parameterLength = fmt - parameter;
                fmt += 1;
            }
        }
        
        assert(isDigit(*fmt) && "diagnostic format argument: should be a digit");
        uint8_t argIdx = *fmt - '0';
        assert(argIdx < diag->paramCount && "diagnostic format argument: invalid index");
        OrbitDiagArg param = diag->params[argIdx];
        fmt += 1;
        
        switch(param.kind) {
        case ORBIT_DPK_INT:
            if(modifierIs(modifier, modifierLength, "s")) {
                handleS(buf, param.intValue);
            }
            else if(modifierIs(modifier, modifierLength, "plural")) {
                handlePlural(buf, parameter, parameterLength, param.intValue);
            }
            else if(modifierIs(modifier, modifierLength, "select")) {
                handleSelect(buf, parameter, parameterLength, param.intValue);
            }
            else {
                assert(!modifier && "diagnostic format argument: unknown modifier");
                char number[32];
                int numberLength = snprintf(number, 32, "%d", param.intValue);
                orbit_stringBufferAppendC(buf, number, numberLength);
            }
            break;
            
        case ORBIT_DPK_CSTRING:
            assert(!modifier && "diagnostic format argument: no string modifiers");
            orbit_stringBufferAppendC(buf, param.cstringValue, strlen(param.cstringValue));
            break;
            
        case ORBIT_DPK_STRING: {
                OCString* str = orbit_stringPoolGet(param.stringValue);
                assert(str && "diagnostic format argument: invalid string pool ID");
                assert(!modifier && "diagnostic format argument: no string modifiers");
                orbit_stringBufferAppendC(buf, str->data, str->length);
            }
            break;
        
        case ORBIT_DPK_TYPE:
            assert(!modifier && "diagnostic format argument: no type expression modifiers");
            orbit_astTypeString(buf, param.typeValue);
            break;
        }
    }
    
    return buffer.data; // Ownership of the allocated data string is transferred
}
