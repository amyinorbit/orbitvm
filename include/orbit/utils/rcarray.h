//===--------------------------------------------------------------------------------------------===
// orbit/utils/rcarray.h
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
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

void orbit_rcArrayDeinit(void* ref);
ORCArray* orbit_rcArrayInit(ORCArray* array, uint64_t capacity);

void orbit_rcArrayAppend(ORCArray* array, void* item);
void orbit_rcArrayInsert(ORCArray* array, uint64_t index, void* item);

void orbit_rcArrayRemove(ORCArray* array, uint64_t index);
void orbit_rcArrayEmpty(ORCArray* array);

#endif /* orbit_utils_rcarray_h */
