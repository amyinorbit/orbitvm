//
//  orbit_vtable.h
//  OrbitVM
//
//  Created by Cesar Parent on 2016-12-30.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#ifndef orbit_vtable_h
#define orbit_vtable_h

#include <stdint.h>
#include <stdbool.h>
#include "orbit_platforms.h"

typedef struct _VMFunction  VMFunction;
typedef struct _GCString    GCString;
typedef struct _OrbitVtable OrbitVtable;

#define VTABLE_DEFAULT_CAPACITY 32

// 
struct _OrbitVtable {
    uint32_t        mask;
    uint32_t        size;
    uint32_t        capacity;
    VMFunction**    data;
};

// Allocates memory for the [table]'s storage and initialises the fields.
void orbit_vtableInit(OrbitVtable* table);

// Deallocates memory used by [table]'s storage.
void orbit_vtableDeinit(OrbitVtable* table);

// Registers [function] in [table], growing [table]'s storage if required.
void orbit_vtableInsert(OrbitVtable* table, VMFunction* function);

// Looks up [selector] in [table] and returns a pointer to the function. If the
// function doesn't exist, returns [NULL].
VMFunction* orbit_vtableLookup(OrbitVtable* table, GCString* selector);

#endif /* orbit_vtable_h */
