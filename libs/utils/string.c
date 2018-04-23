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


static struct OCStringPool {
    uint64_t size;
    uint64_t capacity;
    void* data;
} StringPool;

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

void orbit_stringPoolInit(uint64_t capacity) {
    StringPool.capacity = capacity;
    StringPool.size = 0;
    StringPool.data = orbit_alloc(StringPool.capacity);
}

void orbit_stringPoolDeinit() {
    StringPool.capacity = 0;
    StringPool.size = 0;
    orbit_dealloc(StringPool.data);
}

bool orbit_stringEquals(OCString* a, const char* b, uint64_t length) {
    uint32_t hash = orbit_hashString(b, length);
    return hash == a->hash && length == a->length && strncmp(b, a->data, length) == 0;
}

OCString* orbit_stringPoolSearch(const char* data, uint64_t length) {
    uint32_t hash = orbit_hashString(data, length);
    uint64_t offset = 0;
    
    while(offset < StringPool.size) {
        OCString* str = (OCString*)(StringPool.data + offset);
        if(hash == str->hash && length == str->length && strncmp(data, str->data, length) == 0) {
            return str;
        }
        offset = str->next;
    }
    return NULL;
}

OCStringID orbit_stringIntern(const char* data, uint64_t length) {
    OCString* str = orbit_stringPoolSearch(data, length);
    if(str) { return (OCStringID)((void*)str - StringPool.data); }
    
    while(StringPool.size + length > StringPool.capacity) {
        StringPool.capacity *= 2;
        StringPool.data = orbit_realloc(StringPool.data, StringPool.capacity);
    }
    
    uint64_t id = StringPool.size;
    str = (OCString*)(StringPool.data + StringPool.size);
    StringPool.size += sizeof(OCString) + (length + 1) * sizeof(uint8_t);
    
    memcpy(str->data, data, length+1);
    str->data[length] = '\0';
    str->length = length;
    str->next = StringPool.size;
    str->hash = orbit_hashString(data, length);
    
    return id;
}

OCString* orbit_stringPoolGet(OCStringID id) {
    return (OCString*)(StringPool.data + id);
}

void orbit_stringBufferInit(OCStringBuffer* buffer, uint64_t capacity) {
    OASSERT(buffer != NULL, "Null instance error");
    
    buffer->data = ORBIT_ALLOC_ARRAY(char, capacity);
    buffer->data[0] = '\0';
    buffer->capacity = capacity;
    buffer->length = 0;
}

void orbit_stringBufferReset(OCStringBuffer* buffer) {
    OASSERT(buffer != NULL, "Null instance error");
    buffer->length = 0;
    buffer->data[0] = '\0';
}

void orbit_stringBufferDeinit(OCStringBuffer* buffer) {
    OASSERT(buffer != NULL, "Null instance error");
    orbit_dealloc(buffer->data);
    buffer->capacity = 0;
    buffer->length = 0;
}

static void _bufferReserve(OCStringBuffer* buffer, size_t newSize) {
    OASSERT(buffer != NULL, "Null instance error");
    
    if(newSize < buffer->capacity) { return; }
    while(newSize >= buffer->capacity) {
        buffer->capacity *= 2;
    }
    buffer->data = ORBIT_REALLOC_ARRAY(buffer->data, char, buffer->capacity);
}

void orbit_stringBufferAppend(OCStringBuffer* buffer, codepoint_t c) {
    OASSERT(buffer != NULL, "Null instance error");
    
    int8_t size = utf8_codepointSize(c);
    if(size < 0) { return; }
    
    _bufferReserve(buffer, buffer->length + size + 1);
    utf8_writeCodepoint(c, &buffer->data[buffer->length],
                           buffer->capacity - buffer->length);
    buffer->length += size;
    buffer->data[buffer->length] = '\0';
}

OCStringID orbit_stringBufferIntern(OCStringBuffer* buffer) {
    return orbit_stringIntern(buffer->data, buffer->length);
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

UTFConstString* orbit_cStringConstCopy(const char* cString, size_t length) {
    OASSERT(cString != NULL, "Null instance error");
    
    UTFConstString template = {
        .length = length,
        .hash = orbit_hashString(cString, length)
    };
    
    UTFConstString* copy = ORBIT_ALLOC_FLEX(UTFConstString, char, length + 1);
    memcpy(copy, &template, sizeof(UTFConstString));
    ORCINIT(copy, NULL);
    memcpy((char*)copy->data, cString, length);
    ((char*)copy->data)[copy->length] = '\0';
    return copy;
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
