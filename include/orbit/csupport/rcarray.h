//===--------------------------------------------------------------------------------------------===
// orbit/utils/rcarray.h
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_rcarray_h
#define orbit_utils_rcarray_h

#include <stdint.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _ORCArray ORCArray;

struct _ORCArray {
    uint64_t        size;
    uint64_t        capacity;
    void**          data;
};

void orbitRcArrayDeinit(void* ref);
ORCArray* orbitRcArrayInit(ORCArray* array, uint64_t capacity);

void orbitRcArrayAppend(ORCArray* array, void* item);
void orbitRcArrayInsert(ORCArray* array, uint64_t index, void* item);

void orbitRcArrayRemove(ORCArray* array, uint64_t index);
void orbitRcArrayEmpty(ORCArray* array);

#endif /* orbit_utils_rcarray_h */
