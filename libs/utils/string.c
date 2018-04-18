//===--------------------------------------------------------------------------------------------===
// orbit/utils/string.c
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <orbit/utils/assert.h>
#include <orbit/utils/string.h>

uint32_t orbit_hashString(const char* string, size_t length) {
    OASSERT(string != NULL, "Null instance error");
    
    //Fowler-Noll-Vo 1a hash
    //http://create.stephan-brumme.com/fnv-hash/
    uint32_t hash = 0x811C9DC5;
    for(size_t i = 0; i < length; ++i) {
        hash = (hash ^ string[i]) * 0x01000193;
    }
    return hash;
}

void orbit_utfStringDeinit(void* ref) {
    OASSERT(ref != NULL, "Null instance error");
    UTFString* string = (UTFString*)ref;
    orbit_dealloc(string->data);
}

UTFString* orbit_utfStringInit(UTFString* string, const char* buffer, size_t length) {
    OASSERT(string != NULL, "Null instance error");
    OASSERT(buffer != NULL, "Null instance error");
    
    ORCINIT(string, &orbit_utfStringDeinit);
    string->data = ORBIT_ALLOC_ARRAY(char, length + 1);
    memcpy(string->data, buffer, length);
    string->data[length] = 0;
    string->capacity = length + 1;
    string->length = length;
    return string;
}

UTFString* orbit_utfStringInitWithCapacity(UTFString* string, uint64_t capacity) {
    OASSERT(string != NULL, "Null instance error");
    
    ORCINIT(string, &orbit_utfStringDeinit);
    string->data = ORBIT_ALLOC_ARRAY(char, capacity);
    string->capacity = capacity;
    string->length = 0;
    return string;
}

static void _stringReserve(UTFString* string, size_t newSize) {
    OASSERT(string != NULL, "Null instance error");
    
    if(newSize < string->capacity) { return; }
    while(newSize >= string->capacity) {
        string->capacity *= 2;
    }
    string->data = ORBIT_REALLOC_ARRAY(string->data, char, string->capacity);
}

void orbit_utfStringAppend(UTFString* string, codepoint_t c) {
    OASSERT(string != NULL, "Null instance error");
    
    int8_t size = utf8_codepointSize(c);
    if(size < 0) { return; }
    
    _stringReserve(string, string->length + size + 1);
    utf8_writeCodepoint(c, &string->data[string->length],
                           string->capacity - string->length);
    string->length += size;
    string->data[string->length] = '\0';
}

UTFConstString* orbit_utfStringConstCopy(UTFString* string) {
    OASSERT(string != NULL, "Null instance error");
    
    UTFConstString template = {
        .length = string->length,
        .hash = orbit_hashString(string->data, string->length)
    };
    
    UTFConstString* copy = ORBIT_ALLOC_FLEX(UTFConstString, char, string->length + 1);
    memcpy(copy, &template, sizeof(UTFConstString));
    ORCINIT(copy, NULL);
    memcpy((char*)copy->data, string->data, string->length);
    ((char*)copy->data)[copy->length] = '\0';
    
    return copy;
}
