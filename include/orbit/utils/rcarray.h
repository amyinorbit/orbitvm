//
//  orbit/utils/rcarray.h
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-11-21.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#ifndef orbit_utils_rcarray_h
#define orbit_utils_rcarray_h

#include <stdint.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _ORCArray ORCArray;

struct _ORCArray {
    uint64_t        size;
    uint64_t        capacity;
    void*           data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

ORCArray* orbit_rcArrayNew(uint64_t capacity);

void orbit_rcArrayAppend(ORCArray** array, void* item);
void orbit_rcArrayInsert(ORCArray** array, uint64_t index, void* item);

void orbit_rcArrayRemove(ORCArray** array, uint64_t index);
void orbit_rcArrayEmpty(ORCArray** array);

#endif /* orbit_utils_rcarray_h */
