//===--------------------------------------------------------------------------------------------===
// orbit/utils/string.h
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_string_h
#define orbit_utils_string_h

#include <stdint.h>
#include <orbit/utils/utf8.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _OCString OCString;
typedef uint64_t OCStringID;
typedef struct _OCStringBuffer OCStringBuffer;

extern const OCStringID orbit_invalidStringID;

struct _OCString {
    uint64_t        length;
    uint64_t        next;
    uint32_t        hash;
    char            data[0];
};

struct _OCStringBuffer {
    uint64_t        length;
    uint64_t        capacity;
    char*           data;
};

// Computes the FNV-1a hash of [string].
// This is O(n) complexity and should be used lightly.
uint32_t orbit_hashString(const char* string, size_t length);

void orbit_stringPoolInit(uint64_t capacity);
void orbit_stringPoolDeinit();

bool orbit_stringEquals(OCString* a, const char* b, uint64_t length);
OCStringID orbit_stringIntern(const char* data, uint64_t length);
OCString* orbit_stringPoolSearch(const char* data, uint64_t length);
OCString* orbit_stringPoolGet(OCStringID id);
void orbit_stringPoolDebug();

void orbit_stringBufferInit(OCStringBuffer* buffer, uint64_t capacity);
void orbit_stringBufferDeinit(OCStringBuffer* buffer);
void orbit_stringBufferReset(OCStringBuffer* buffer);

void orbit_stringBufferAppend(OCStringBuffer* buffer, codepoint_t codepoint);
void orbit_stringBufferAppendC(OCStringBuffer* buffer, char* data, uint64_t length);
OCStringID orbit_stringBufferIntern(OCStringBuffer* buffer);

#endif /* orbit_utils_string_h */
