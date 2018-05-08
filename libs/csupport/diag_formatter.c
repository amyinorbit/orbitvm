//===--------------------------------------------------------------------------------------------===
// orbit/csupport/diag_formatter.c - Implementation of the diagnostics token formatter
// This source is part of Orbit - Compiler Support
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <orbit/csupport/diag.h>
#include <orbit/utils/assert.h>

uint32_t orbit_printDiagS(OrbitDiagParam param, char* str, uint32_t size) {
    bool print = false;
    switch(param.kind) {
        case ORBIT_DPK_INT:     print = (param.intValue != 1);      break;
        case ORBIT_DPK_FLOAT:   print = (param.floatValue != 1.0);  break;
        case ORBIT_DPK_CSTRING:
        case ORBIT_DPK_STRING:  print = false;                      break;
    }
    if(print && size > 1) {
        putchar('s');
        str[0] = 's';
        return 1;
    }
    return 0;
}

uint32_t orbit_printDiagParam(OrbitDiagParam param, char* str, uint32_t size) {
    switch(param.kind) {
        case ORBIT_DPK_INT:
            return snprintf(str, size, "%d", param.intValue);
            
        case ORBIT_DPK_FLOAT:
            return snprintf(str, size, "%f", param.floatValue);
            
        case ORBIT_DPK_STRING: {
            OCString* p = orbit_stringPoolGet(param.stringValue);
            return snprintf(str, size, "%.*s", (int)p->length, p->data);
        }
        
        case ORBIT_DPK_CSTRING:
            return snprintf(str, size, "%s", param.cstringValue);
    }
}

uint32_t orbit_diagGetFormat(OrbitDiag* diag, char* str, uint32_t size) {
    
    const char* format = diag->format;
    uint32_t head = 0;
    
    #define CURRENT()   (*format)
    #define NEXT()      (*(format+1))
    #define INCRCHECK() {if(*(++format) == '\0') { break; }}
    #define ISDIGIT()   (*format >= '0' && *format <= '9')
    
    bool placeholderIsPlural = false;
    uint16_t placeholderIndex = 0;
    
    while(*format && head < size-1) {
        
        if(CURRENT() != '$') {
            str[head++] = CURRENT();
            INCRCHECK();
            continue;
        }
        INCRCHECK();
        
        placeholderIndex = 0;
        placeholderIsPlural = false;
        
        if(CURRENT() == 's') {
            placeholderIsPlural = true;
            INCRCHECK();
        }
        
        if(!ISDIGIT()) { break; }
        while(ISDIGIT()) {
            placeholderIndex = placeholderIndex * 10 + (CURRENT() - '0');
            INCRCHECK();
        }
        
        if(placeholderIsPlural) {
            head += orbit_printDiagS(diag->params[placeholderIndex], &str[head], (size-head));
        } else {
            head += orbit_printDiagParam(diag->params[placeholderIndex], &str[head], (size-head));
        }
    }
    
    #undef CURRENT
    #undef NEXT
    #undef INCRCHECK
    str[head++] = '\0';
    
    return head;
}
