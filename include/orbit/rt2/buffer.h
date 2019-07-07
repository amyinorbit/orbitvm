//===--------------------------------------------------------------------------------------------===
// buffer.h - Template-ish interface for creating dynamic runtime buffers
// This source is part of Orbit
//
// Created on 2019-07-04 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_buffer_h
#define orbit_buffer_h
#include <orbit/common.h>
#include <orbit/rt2/value.h>

#define DECLARE_BUFFER(name, type)                                                                 \
    typedef struct {                                                                               \
        type* data;                                                                                \
        int32_t count;                                                                             \
        int32_t capacity;                                                                          \
    } Orbit##name##Buffer;                                                                         \
    void orbit_##name##BufferInit(Orbit##name##Buffer* buffer);                                    \
    void orbit_##name##BufferDeinit(OrbitGC* gc, Orbit##name##Buffer* buffer);                     \
    void orbit_##name##BufferFill(OrbitGC* gc, Orbit##name##Buffer* buffer, type data,             \
                                  int32_t count);                                                  \
    void orbit_##name##BufferWrite(OrbitGC* gc, Orbit##name##Buffer* buffer, type data)

// This should be used once for each type instantiation, somewhere in a .c file.
#define DEFINE_BUFFER(name, type)                                                                  \
    void orbit_##name##BufferInit(Orbit##name##Buffer* buffer) {                                   \
        buffer->data = NULL;                                                                       \
        buffer->capacity = 0;                                                                      \
        buffer->count = 0;                                                                         \
    }                                                                                              \
                                                                                                   \
    void orbit_##name##BufferDeinit(OrbitGC* gc, Orbit##name##Buffer* buffer) {                    \
        orbit_gcalloc(gc, buffer->data, buffer->capacity * sizeof(type), 0);                       \
        orbit_##name##BufferInit(buffer);                                                          \
    }                                                                                              \
                                                                                                   \
    void orbit_##name##BufferFill(OrbitGC* gc, Orbit##name##Buffer* buffer, type data,             \
                                  int count) {                                                     \
        if(buffer->capacity < buffer->count + count) {                                             \
            int oldCapacity = buffer->capacity;                                                    \
            while(buffer->capacity < buffer->count + count)                                        \
                buffer->capacity = ORBIT_GROW_CAPACITY(buffer->capacity);                          \
            buffer->data = (type*)orbit_gcalloc(gc, buffer->data, oldCapacity * sizeof(type),      \
                                                buffer->capacity * sizeof(type));                  \
        }                                                                                          \
                                                                                                   \
        for(int i = 0; i < count; i++) {                                                           \
            buffer->data[buffer->count++] = data;                                                  \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    void orbit_##name##BufferWrite(OrbitGC* gc, Orbit##name##Buffer* buffer, type data) {          \
        orbit_##name##BufferFill(gc, buffer, data, 1);                                             \
    }

DECLARE_BUFFER(Value, OrbitValue);
DECLARE_BUFFER(Byte, uint8_t);
DECLARE_BUFFER(Int, int32_t);
DECLARE_BUFFER(String, OrbitString*);

#endif
