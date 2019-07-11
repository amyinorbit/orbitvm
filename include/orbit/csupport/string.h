//===--------------------------------------------------------------------------------------------===
// orbit/utils/string.h
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
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

extern const OCStringID orbitInvalidStringID;

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

void orbitStringPoolInit(uint64_t capacity);
void orbitStringPoolDeinit();

bool orbitStringEquals(OCString* a, const char* b, uint64_t length);
OCStringID orbitStringIntern(const char* data, uint64_t length);
OCString* orbitStringPoolSearch(const char* data, uint64_t length);
OCString* orbitStringPoolGet(OCStringID id);
void orbitStringPoolDebug();

void orbitStringBufferInit(OCStringBuffer* buffer, uint64_t capacity);
void orbitStringBufferDeinit(OCStringBuffer* buffer);
void orbitStringBufferReset(OCStringBuffer* buffer);

void orbitStringBufferAppend(OCStringBuffer* buffer, codepoint_t codepoint);
void orbitStringBufferAppendP(OCStringBuffer* buffer, OCStringID id);
void orbitStringBufferAppendC(OCStringBuffer* buffer, const char* data, uint64_t length);
OCStringID orbitStringBufferIntern(OCStringBuffer* buffer);

#endif /* orbit_utils_string_h */
