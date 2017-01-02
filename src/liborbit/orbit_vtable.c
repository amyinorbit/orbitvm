//
//  orbit_vtable.c
//  OrbitVM
//
//  Created by Cesar Parent on 30/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <string.h>
#include "orbit_vtable.h"
#include "orbit_value.h"
#include "orbit_utils.h"

#define VTABLE_CMP(sel, fun)                                            \
((sel)->data == (fun)->selector.data                                    \
    || ((sel)->length == (fun)->selector.length                         \
        && (sel)->hash == (fun)->selector.hash                          \
        && strcmp((sel)->data, (fun)->selector.data) == 0))

static void orbit_vtableGrow(OrbitVtable* table) {
    OASSERT(table != NULL, "Null instance error");
    
    VMFunction** oldData = table->data;
    uint32_t oldCapacity = table->capacity;
    
    if(oldCapacity > 0)
        table->capacity = oldCapacity << 1;
    else
        table->capacity = VTABLE_DEFAULT_CAPACITY;
    
    table->mask = table->capacity - 1;
    table->data = ALLOC_ARRAY(VMFunction*, table->capacity);
    memset(table->data, 0, table->capacity);
    
    for(uint32_t i = 0; i < oldCapacity; ++i) {
        if(oldData[i] == NULL) continue;
        uint32_t index = oldData[i]->selector.hash & table->mask;
        while(table->data[index]) {
            index = (index + 1) & table->mask;
        }
        table->data[index] = oldData[i];
        table->size += 1;
    }
    DEALLOC(oldData);
}

void orbit_vtableInit(OrbitVtable* table) {
    OASSERT(table != NULL, "Null instance error");
    
    table->capacity = VTABLE_DEFAULT_CAPACITY;
    table->mask = VTABLE_DEFAULT_CAPACITY - 1;
    table->size = 0;
    table->data = ALLOC_ARRAY(VMFunction*, VTABLE_DEFAULT_CAPACITY);
    memset(table->data, 0, VTABLE_DEFAULT_CAPACITY);
}

void orbit_vtableDeinit(OrbitVtable* table) {
    OASSERT(table != NULL, "Null instance error");
    
    if(table->data != NULL) {
        DEALLOC(table->data);
    }
}

void orbit_vtableInsert(OrbitVtable* table, VMFunction* function) {
    OASSERT(table != NULL, "Null instance error");
    OASSERT(function != NULL, "Null instance error");
    
    uint32_t index = function->selector.hash & table->mask;
    while(table->data[index]) {
        index = (index + 1) & table->mask;
    }
    table->data[index] = function;
    table->size += 1;
    
    if(table->size >= 0.75 * table->capacity) {
        orbit_vtableGrow(table);
    }
}

VMFunction* orbit_vtableLookup(OrbitVtable* table, VMString* selector) {
    OASSERT(table != NULL, "Null instance error");
    OASSERT(selector != NULL, "Null instance error");
    
    for(uint32_t index = selector->hash & table->mask;
        table->data[index] != NULL;
        index = (index +  1) & table->mask)
    {
        if(VTABLE_CMP(selector, table->data[index])) {
            return table->data[index];
        }
    }
    return NULL;
}
