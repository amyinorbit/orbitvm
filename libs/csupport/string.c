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
#include <orbit/csupport/string.h>

const OCStringID orbit_invalidStringID = UINT64_MAX;

static struct OCStringPool {
    uint64_t size;
    uint64_t capacity;
    void* data;
} StringPool;

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
    if(id == orbit_invalidStringID) { return NULL; }
    return (OCString*)(StringPool.data + id);
}

void orbit_stringPoolDebug() {
    uint64_t offset = 0;
    while(offset < StringPool.size) {
        OCString* str = (OCString*)(StringPool.data + offset);
        fprintf(stderr, "[strpool: 0x%08lx]: \"%.*s\"\n", ((void*)str - StringPool.data), (int)str->length, str->data);
        offset = str->next;
    }
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

void orbit_stringBufferAppendP(OCStringBuffer* buffer, OCStringID id) {
    OASSERT(buffer != NULL, "Null instance error");
    OCString* str = orbit_stringPoolGet(id);
    if(!str) { return; }
    orbit_stringBufferAppendC(buffer, str->data, str->length);
}

void orbit_stringBufferAppendC(OCStringBuffer* buffer, const char* data, uint64_t length) {
    OASSERT(buffer != NULL, "Null instance error");
    _bufferReserve(buffer, buffer->length + length + 1);
    memcpy(buffer->data + buffer->length, data, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
}

OCStringID orbit_stringBufferIntern(OCStringBuffer* buffer) {
    return orbit_stringIntern(buffer->data, buffer->length);
}
