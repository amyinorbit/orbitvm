//===--------------------------------------------------------------------------------------------===
// orbit/utils/string.c
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <assert.h>
#include <orbit/csupport/string.h>
#include <orbit/utils/hashing.h>

const OCStringID orbitInvalidStringID = UINT64_MAX;

static struct OCStringPool {
    uint64_t size;
    uint64_t capacity;
    void* data;
} StringPool;

void orbitStringPoolInit(uint64_t capacity) {
    StringPool.capacity = capacity;
    StringPool.size = 0;
    StringPool.data = ORBIT_ALLOC_ARRAY(char, StringPool.capacity);
}

void orbitStringPoolDeinit() {
    ORBIT_DEALLOC_ARRAY(StringPool.data, char, StringPool.capacity);
    StringPool.capacity = 0;
    StringPool.size = 0;
}

bool orbitStringEquals(OCString* a, const char* b, uint64_t length) {
    uint32_t hash = orbitHashString(b, length);
    return hash == a->hash && length == a->length && strncmp(b, a->data, length) == 0;
}

OCString* orbitStringPoolSearch(const char* data, uint64_t length) {
    uint32_t hash = orbitHashString(data, length);
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


OCStringID orbitStringIntern(const char* data, uint64_t length) {
    OCString* str = orbitStringPoolSearch(data, length);
    if(str) { return (OCStringID)((void*)str - StringPool.data); }
    
    if(StringPool.size + length >= StringPool.capacity) {
        uint64_t oldCapacity = StringPool.capacity;
        while(StringPool.size + length >= StringPool.capacity)
            StringPool.capacity = ORBIT_GROW_CAPACITY(StringPool.capacity);
        StringPool.data = ORBIT_REALLOC_ARRAY(StringPool.data, char, oldCapacity, StringPool.capacity);
    }
    
    uint64_t id = StringPool.size;
    str = (OCString*)(StringPool.data + StringPool.size);
    StringPool.size += sizeof(OCString) + (length + 1) * sizeof(uint8_t);
    
    memcpy(str->data, data, length+1);
    str->data[length] = '\0';
    str->length = length;
    str->next = StringPool.size;
    str->hash = orbitHashString(data, length);
    
    return id;
}

OCString* orbitStringPoolGet(OCStringID id) {
    if(id == orbitInvalidStringID) { return NULL; }
    return (OCString*)(StringPool.data + id);
}

void orbitStringPoolDebug() {
    uint64_t offset = 0;
    while(offset < StringPool.size) {
        OCString* str = (OCString*)(StringPool.data + offset);
        fprintf(stderr, "[strpool: 0x%08lx]: \"%.*s\"\n", ((void*)str - StringPool.data), (int)str->length, str->data);
        offset = str->next;
    }
}

void orbitStringBufferInit(OCStringBuffer* buffer, uint64_t capacity) {
    assert(buffer != NULL && "Null instance error");
    
    buffer->data = ORBIT_ALLOC_ARRAY(char, capacity);
    buffer->data[0] = '\0';
    buffer->capacity = capacity;
    buffer->length = 0;
}

void orbitStringBufferReset(OCStringBuffer* buffer) {
    assert(buffer != NULL && "Null instance error");
    buffer->length = 0;
    buffer->data[0] = '\0';
}

void orbitStringBufferDeinit(OCStringBuffer* buffer) {
    assert(buffer != NULL && "Null instance error");
    ORBIT_DEALLOC_ARRAY(buffer->data, char, buffer->capacity);
    buffer->capacity = 0;
    buffer->length = 0;
}

static void _bufferReserve(OCStringBuffer* buffer, size_t newSize) {
    assert(buffer != NULL && "Null instance error");
    
    if(newSize < buffer->capacity) { return; }
    
    size_t oldCapacity = buffer->capacity;
    while(newSize >= buffer->capacity)
        buffer->capacity = ORBIT_GROW_CAPACITY(buffer->capacity);
    buffer->data = ORBIT_REALLOC_ARRAY(buffer->data, char, oldCapacity, buffer->capacity);
}

void orbitStringBufferAppend(OCStringBuffer* buffer, codepoint_t c) {
    assert(buffer != NULL && "Null instance error");
    
    int8_t size = utf8_codepointSize(c);
    if(size < 0) { return; }
    
    _bufferReserve(buffer, buffer->length + size + 1);
    utf8_writeCodepoint(c, &buffer->data[buffer->length],
                           buffer->capacity - buffer->length);
    buffer->length += size;
    buffer->data[buffer->length] = '\0';
}

void orbitStringBufferAppendP(OCStringBuffer* buffer, OCStringID id) {
    assert(buffer != NULL && "Null instance error");
    OCString* str = orbitStringPoolGet(id);
    if(!str) { return; }
    orbitStringBufferAppendC(buffer, str->data, str->length);
}

void orbitStringBufferAppendC(OCStringBuffer* buffer, const char* data, uint64_t length) {
    assert(buffer != NULL && "Null instance error");
    _bufferReserve(buffer, buffer->length + length + 1);
    memcpy(buffer->data + buffer->length, data, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
}

OCStringID orbitStringBufferIntern(OCStringBuffer* buffer) {
    return orbitStringIntern(buffer->data, buffer->length);
}
