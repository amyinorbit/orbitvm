//
//  orbit_objfile.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-13.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include "orbit_objfile.h"
#include "orbit_pack.h"
#include "orbit_utils.h"

#define ARRAY_DEFAULT_CAPACITY 64

#define STR(string) strcpy(malloc(strlen(string)), (string))


static inline void array_init(ObjectArray* array) {
    array->size = 0;
    array->capacity = ARRAY_DEFAULT_CAPACITY;
    array->data = calloc(ARRAY_DEFAULT_CAPACITY, sizeof(void*));
}

static inline void array_ensure(ObjectArray* array) {
    if(array->size + 1 < array->capacity) return;
    array->capacity *= 2;
    array->data = realloc(array->data, sizeof(void*) * array->capacity);
}

static inline void array_deinit(ObjectArray* array) {
    array->capacity = 0;
    array->size = 0;
    free(array->data);
    array->data = NULL;
}

void orbit_objInit(ObjectFile* obj) {
    OASSERT(obj != NULL, "Null instance error");
    array_init(&obj->variables);
    array_init(&obj->classes);
    array_init(&obj->functions);
}

void orbit_objDeinit(ObjectFile* obj) {
    OASSERT(obj != NULL, "Null instance error");
    
    for(uint16_t i = 0; i < obj->variables.size; ++i) {
        free(obj->variables.data[i]);
    }
    
    for(uint16_t i = 0; i < obj->classes.size; ++i) {
        free(obj->classes.data[i]);
    }
        
    for(uint16_t i = 0; i < obj->functions.size; ++i) {
        free(((ObjectFn*)obj->functions.data[i])->name);
        free(obj->functions.data[i]);
    }
    
    array_deinit(&obj->variables);
    array_deinit(&obj->classes);
    array_deinit(&obj->functions);
}

ObjectFn* orbit_objFnNew(const char* name, uint16_t codeSize, uint8_t* code) {
    OASSERT(name != NULL, "Null instance error");
    
    ObjectFn* fn = malloc(sizeof(ObjectFn) + (codeSize * sizeof(uint8_t)));
    fn->name = STR(name);
    fn->parameterCount = 0;
    fn->requiredStack = 0;
    fn->byteCodeLength = codeSize;
    memcpy(fn->byteCode, code, codeSize);
    
    return fn;
}

void orbit_objAddFn(ObjectFile* obj, ObjectFn* function) {
    OASSERT(obj != NULL, "Null instance error");
    OASSERT(function != NULL, "Null instance error");
    array_ensure(&obj->functions);
    obj->functions.data[obj->functions.size++] = function;
}

uint8_t orbit_objFnAddCString(ObjectFn* fn, const char* string) {
    OASSERT(fn != NULL, "Null instance error");
    OASSERT(fn->constants.size < 255, "Constant pool overflow");
    
    fn->constants.entries[fn->constants.size].type = TYPE_STRING;
    fn->constants.entries[fn->constants.size].stringValue = STR(string);
        
    return fn->constants.size++;
}

uint8_t orbit_objFnAddCNumber(ObjectFn* fn, double number) {
    OASSERT(fn != NULL, "Null instance error");
    OASSERT(fn->constants.size < 255, "Constant pool overflow");
    
    fn->constants.entries[fn->constants.size].type = TYPE_NUMBER;
    fn->constants.entries[fn->constants.size].numValue = number;
        
    return fn->constants.size++;
}

void orbit_objAddClass(ObjectFile* obj, const char* name, uint16_t fieldCount) {
    OASSERT(obj != NULL, "Null instance error");
    OASSERT(name != NULL, "Null instance error");
    
    size_t length = strlen(name);
    array_ensure(&obj->classes);
    ObjectClass* class = malloc(sizeof(ObjectClass)
                                + (length * sizeof(uint8_t)) + 1);
    class->fieldCount = fieldCount;
    memcpy(class->name, name, length);
    class->name[length] = '\0';
    obj->classes.data[obj->classes.size++] = class;
}

void orbit_objAddVariable(ObjectFile* obj, const char* name) {
    array_ensure(&obj->variables);
    obj->variables.data[obj->variables.size++] = STR(name);
}



static void orbit_objWriteString(FILE* out, const char* str) {
        uint16_t length = strlen(str);
        orbit_pack8(out, TYPE_STRING);
        orbit_pack16(out, length);
        orbit_packBytes(out, (uint8_t*)str, length);
}

static void orbit_objWriteConstant(FILE* out, CPoolEntry cst) {
    switch(cst.type) {
    case TYPE_NUMBER:
        orbit_pack8(out, TYPE_NUMBER);
        orbit_packIEEE754(out, cst.numValue);
        break;
        
    case TYPE_STRING:
        orbit_objWriteString(out, cst.stringValue);
        break;
        
    default:
        break;
    }
}

void orbit_objWriteFile(FILE* out, ObjectFile* obj) {
    static const char signature[4] = "OOFF";
    
    // write the signature and version number
    orbit_packBytes(out, (uint8_t*)signature, 4);
    orbit_pack16(out, 0x0001);
    
    // write the variable count
    orbit_pack16(out, obj->variables.size);
    
    // write all the variables
    for(uint16_t i = 0; i < obj->variables.size; ++i) {
        orbit_pack8(out, TYPE_VARIABLE);
        orbit_objWriteString(out, (const char*)obj->variables.data[i]);
    }
    
    // write the class count
    orbit_pack16(out, obj->classes.size);
    for(uint16_t i = 0; i < obj->classes.size; ++i) {
        ObjectClass* class = (ObjectClass*)obj->classes.data[i];
        
        orbit_pack8(out, TYPE_CLASS);
        orbit_objWriteString(out, class->name);
        orbit_pack16(out, class->fieldCount);
    }
    
    // write the function count
    orbit_pack16(out, obj->functions.size);
    for(uint16_t i = 0; i < obj->functions.size; ++i) {
        ObjectFn* fn = (ObjectFn*)obj->functions.data[i];
        
        orbit_pack8(out, TYPE_FUNCTION);
        orbit_objWriteString(out, fn->name);
        
        orbit_pack8(out, fn->parameterCount);
        orbit_pack16(out, fn->requiredStack);
        
        orbit_pack8(out, fn->constants.size);
        for(uint8_t i = 0; i < fn->constants.size; ++i) {
            orbit_objWriteConstant(out, fn->constants.entries[i]);
        }
        
        orbit_pack16(out, fn->byteCodeLength);
        orbit_packBytes(out, fn->byteCode, fn->byteCodeLength);
    }
}

    
#define OPCODE(x, _) CODE_##x,
    typedef enum {
#include "orbit_opcodes.h"    
    } Code;
#undef OPCODE

static void orbit_objDumpBytecode(uint8_t* bytecode, uint16_t length) {

#define OPCODE(x, _) [CODE_##x] = #x,
    static const char* mnemonics[] = {
#include "orbit_opcodes.h"        
    };
#undef OPCODE
    
#define OPCODE(x, y) [CODE_##x] = y,
    static const uint8_t params[] = {
#include "orbit_opcodes.h"        
    };
#undef OPCODE
    
    uint8_t opcode = 0;
    uint16_t param16 = 0;
    
    uint16_t index = 0;
    
    while(index < length) {
        opcode = bytecode[index++];

        printf("\t\t%2hu: %s", index, mnemonics[opcode]);
        
        switch(params[opcode]) {
        case 0:
            printf("\n");
            break;
            
        case 1:
            printf(" %2hhu\n", bytecode[index++]);
            break;
            
        case 2:
            param16 = bytecode[index++] << 8;
            param16 |= bytecode[index++];

            printf(" %hu\n", param16);
            break;
        default:
            break;
        }
    }
    
}

void orbit_objDebugFile(FILE* in) {
    
#define GUARD(expr) if(!(expr)) goto eof
    
    printf("orbit object file parser\n");
    
    // Check the signature
    char signature[5] = {'\0'};
    uint16_t version = 0;
    
    GUARD(orbit_unpackBytes(in, (uint8_t*)signature, 4));
    GUARD(orbit_unpack16(in, &version));
    
    printf("signature: %4s\n", signature);
    printf("version: %04x\n\n", version);
    
    uint16_t count = 0;
    GUARD(orbit_unpack16(in, &count));
    fprintf(stderr, "---%hu global variables\n", count);
    
    for(uint16_t i = 0; i < count; ++i) {
        uint8_t type = 0;
        uint16_t length = 0;
        char* buffer = NULL;
        
        GUARD(orbit_unpack8(in, &type));
        if(type != TYPE_VARIABLE) goto mismatch;
        
        GUARD(orbit_unpack8(in, &type));
        if(type != TYPE_STRING) goto mismatch;
        
        GUARD(orbit_unpack16(in, &length));
        
        buffer = malloc(length+1);
        GUARD(orbit_unpackBytes(in, (uint8_t*)buffer, length));
        buffer[length] = '\0';
        printf("\t%2hu -> \"%s\"_%hu\n", i, buffer, length);
        free(buffer);
    }

    GUARD(orbit_unpack16(in, &count));
    fprintf(stderr, "\n---%hu user-defined classes\n", count);
    
    for(uint16_t i = 0; i < count; ++i) {
        uint8_t type = 0;
        uint16_t length = 0;
        uint16_t fields = 0;
        char* buffer = NULL;
        
        GUARD(orbit_unpack8(in, &type));
        if(type != TYPE_CLASS) goto mismatch;
        
        GUARD(orbit_unpack8(in, &type));
        if(type != TYPE_STRING) goto mismatch;
        GUARD(orbit_unpack16(in, &length));
        
        buffer = malloc(length+1);
        GUARD(orbit_unpackBytes(in, (uint8_t*)buffer, length));
        buffer[length] = '\0';
        
        GUARD(orbit_unpack16(in, &fields));
        
        printf("\t%2hu -> %s [fields: %hu]\n", i, buffer, fields);
        free(buffer);
    }
    
    GUARD(orbit_unpack16(in, &count));
    fprintf(stderr, "\n---%hu functions\n", count);
    
    for(uint16_t i = 0; i < count; ++i) {
        uint8_t type = 0, params = 0, ccount = 0;
        uint16_t length = 0, stack = 0;
        char* buffer = NULL;
        
        GUARD(orbit_unpack8(in, &type));
        if(type != TYPE_FUNCTION) goto mismatch;
        
        GUARD(orbit_unpack8(in, &type));
        if(type != TYPE_STRING) goto mismatch;
        GUARD(orbit_unpack16(in, &length));
        
        buffer = malloc(length+1);
        GUARD(orbit_unpackBytes(in, (uint8_t*)buffer, length));
        buffer[length] = '\0';
        
        GUARD(orbit_unpack8(in, &params));
        GUARD(orbit_unpack16(in, &stack));
        
        printf("\t%2hu -> %s [params: %hhu, stack: %hu]\n", i, buffer, params, stack);
        free(buffer);
        
        GUARD(orbit_unpack8(in, &ccount));
        printf("\t---%2hhu constants in pool\n", ccount);
        for(uint8_t i = 0; i < ccount; ++i) {
            uint8_t ctype = 0;
            uint16_t clength = 0;
            double cnumValue = 0;
        
            GUARD(orbit_unpack8(in, &ctype));
            switch(ctype) {
            case TYPE_NUMBER:
                GUARD(orbit_unpackIEEE754(in, &cnumValue));
                printf("\t\t%2hhu -> %lf\n", i, cnumValue);
                break;
                
            case TYPE_STRING:
                GUARD(orbit_unpack16(in, &clength));
                char* cbuffer = malloc(clength);
                GUARD(orbit_unpackBytes(in, (uint8_t*)cbuffer, clength));
                cbuffer[clength] = '\0';
                printf("\t\t%2hhu -> \"%s\"_%hu\n", i, cbuffer, clength);
                free(cbuffer);
                break;
                
            default:
                goto mismatch;
                break;
            }
            
        }
        
        
        GUARD(orbit_unpack16(in, &length));
        printf("\t---bytecode [%hu]\n", length);
        uint8_t* code = malloc(length * sizeof(uint8_t));
        
        GUARD(orbit_unpackBytes(in, code, length));
        
        orbit_objDumpBytecode(code, length);
        free(code);
    }
    printf("\n---done\n");
    
    return;
    
mismatch:
    fprintf(stderr, "type mismatch\n");
    return;
    
eof:
    fprintf(stderr, "unexpected end of file\n");
    return;
    
}