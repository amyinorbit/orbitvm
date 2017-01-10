//
//  orbit_value.c
//  OrbitVM
//
//  Created by Cesar Parent on 2016-12-26.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <string.h>
#include "orbit_value.h"
#include "orbit_utils.h"
#include "orbit_vm.h"

// Initialises [object] as an instance of [class]. [class] can be NULL if the
// object being initialized is a class itself.
static void orbit_objectInit(OrbitVM* vm, GCObject* object, GCClass* class) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(object != NULL, "Null instance error");
    
    object->class = class;
    object->mark = false;
    object->next = vm->gcHead;
    vm->gcHead = object;
}

GCString* orbit_gcStringNew(OrbitVM* vm, const char* string) {
    OASSERT(vm != NULL, "Null instance error");
    
    size_t length = strlen(string);
    GCString* object = ALLOC_FLEX(vm, GCString, char, length+1);
    orbit_objectInit(vm, &object->base, NULL);
    
    object->base.type = OBJ_STRING;
    object->length = length;
    object->hash = orbit_hashString(string, length);
    memcpy(object->data, string, length);
    object->data[length] = '\0';
    return object;
}

GCInstance* orbit_gcInstanceNew(OrbitVM* vm, GCClass* class) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(class != NULL, "Null class error");
    
    GCInstance* object = ALLOC_FLEX(vm, GCInstance, GCValue, class->fieldCount);
    orbit_objectInit(vm, &object->base, class);
    object->base.type = OBJ_INSTANCE;
    return object;
}

GCClass* orbit_gcClassNew(OrbitVM* vm, const char* name, uint16_t fieldCount) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(name != NULL, "Null name error");
    
    GCClass* class = ALLOC(vm, GCClass);
    orbit_objectInit(vm, &class->base, NULL);
    class->base.type = OBJ_CLASS;
    class->super = NULL;
    class->fieldCount = fieldCount;
    orbit_stringInit(&class->name, name);
    
    return class;
}


VMFunction* orbit_gcFunctionNew(OrbitVM* vm, uint8_t* byteCode,
                                             uint16_t byteCodeLength,
                                             uint8_t constantCount) {
    OASSERT(vm != NULL, "Null instance error");
    
    VMFunction* function = ALLOC(vm, VMFunction);
    orbit_objectInit(vm, &function->base, NULL);
    function->base.type = OBJ_FUNCTION;
    function->type = FN_NATIVE;
    
    function->native.byteCode = ALLOC_ARRAY(vm, uint8_t, byteCodeLength);
    function->native.byteCodeLength = byteCodeLength;
    memcpy(function->native.byteCode, byteCode, byteCodeLength);
    
    function->native.constants = ALLOC_ARRAY(vm, GCValue, constantCount);
    function->native.constantCount = constantCount;
    
    return function;
}

void orbit_gcDeallocate(OrbitVM* vm, GCObject* object) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(object != NULL, "Null instance error");
    
    switch(object->type) {
    case OBJ_CLASS:
        orbit_stringDeinit(&((GCClass*)object)->name);
        break;
        
    case OBJ_INSTANCE:
        break;
        
    case OBJ_STRING:
        break;
        
    case OBJ_MAP:
        DEALLOC(vm, ((GCMap*)object)->data);
        break;
    
    case OBJ_ARRAY:
        DEALLOC(vm, ((GCArray*)object)->data);
        break;
        
    case OBJ_FUNCTION:
        if(((VMFunction*)object)->type == FN_NATIVE) {
            DEALLOC(vm, ((VMFunction*)object)->native.byteCode);
            DEALLOC(vm, ((VMFunction*)object)->native.constants);
        }
        break;
        
    case OBJ_CONTEXT:
        // TODO: implement context deallocation.
        break;
    }
    DEALLOC(vm, object);
}

// MARK: - Map functions implementations

static inline uint32_t orbit_valueHash(GCValue value) {
    if(IS_NUM(value))
        return orbit_hashDouble(AS_NUM(value));
    if(IS_STRING(value))
        return AS_STRING(value)->hash;
    else
        return 0; // TODO: replace with pointer hash.
}

static void orbit_gcMapGrow(OrbitVM* vm, GCMap* map) {
    uint32_t oldCapacity = map->capacity;
    GCMapEntry* oldData = map->data;
    
    if(map->capacity == 0)
        map->capacity = GCMAP_DEFAULT_CAPACITY;
    else
        map->capacity = map->capacity << 1;
    
    map->data = ALLOC_ARRAY(vm, GCMapEntry, map->capacity);
    map->size = 0;
    map->mask = map->capacity - 1;
    
    for(uint32_t i = 0; i < map->capacity; ++i) {
        map->data[i].key = VAL_NIL;
        map->data[i].value = VAL_FALSE;
    }
    
    for(uint32_t i = 0; i < oldCapacity; ++i) {
        orbit_gcMapAdd(vm, map, oldData[i].key, oldData[i].value);
    }
    
    DEALLOC(vm, oldData);
}

// Custom equality check for map, we avoid unused cases (only number and string
// comparisons)
static inline bool orbit_gcMapComp(GCValue a, GCValue b) {
    if(IS_NUM(a) && IS_NUM(b)) {
        return AS_NUM(a) == AS_NUM(b);
    }
    GCString *stra = AS_STRING(a), *strb = AS_STRING(b);
    
    // Check for pointer equality first.
    return (stra == strb)
        || (stra->length == strb->length
            && stra->hash == strb->hash
            && memcmp(stra->data, strb->data, stra->length) == 0); 
}

// Find the entry in [map] keyed by [key], or return a pointer to the entry
// where a value should be inserted.
static GCMapEntry* orbit_gcMapFindSlot(GCMap* map, GCValue key) {
    GCMapEntry* insert = NULL;
    uint32_t index = orbit_valueHash(key);
    uint32_t start = index;
    
    do {
        if(IS_NIL(map->data[index].key)) {
            if(IS_FALSE(map->data[index].value)) {
                // Empty slot, we're done, the key is not in the map.
                return &map->data[index];
            } else {
                // Tombstone, so we need to keep searching. Keep a pointer
                // to the tombstone so that this can be returned for insertion.
                if(insert == NULL) insert = &map->data[index];
            }
        } else {
            if(orbit_gcMapComp(key, map->data[index].key)) {
                return &map->data[index];
            }
        }
        index = (index + 1) & map->capacity;
    } while(index != start);
    
    return insert;
}

GCMap* orbit_gcMapNew(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    
    GCMap* map = ALLOC(vm, GCMap);
    orbit_objectInit(vm, (GCObject*)map, NULL/* TODO: replace with Map class*/);
    map->base.type = OBJ_MAP;
    
    map->capacity = 0;
    orbit_gcMapGrow(vm, map);
    
    return map;
}

void orbit_gcMapAdd(OrbitVM* vm, GCMap* map, GCValue key, GCValue value) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(map != NULL, "Null instance error");
    OASSERT(IS_NUM(key) || IS_STRING(key), "Map keys must be primitives");
    
    if(map->size+1 > 0.75 * map->capacity) {
        orbit_gcMapGrow(vm, map);
    }
    
    GCMapEntry* slot = orbit_gcMapFindSlot(map, key);
    slot->value = value;
    map->size += 1;
}

bool orbit_gcMapGet(GCMap* map, GCValue key, GCValue* value) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(IS_NUM(key) || IS_STRING(key), "Map keys must be primitives");
    
    GCMapEntry* slot = orbit_gcMapFindSlot(map, key);
    if(IS_NIL(slot->key)) {
        *value = VAL_NIL;
        return false;
    } else {
        *value = slot->value;
        return true;
    }
}

void orbit_gcMapRemove(OrbitVM* vm, GCMap* map, GCValue key) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(map != NULL, "Null instance error");
    OASSERT(IS_NUM(key) || IS_STRING(key), "Map keys must be primitives");
    
    GCMapEntry* slot = orbit_gcMapFindSlot(map, key);
    if(IS_NIL(slot->key)) return;
    
    // Tombstone, so that entries pushed further by collisions can still be
    // found using gcMapFindSlot.
    slot->key = VAL_NIL;
    slot->value = VAL_TRUE;
    map->size -= 1;
}

// MARK: - Array functions implementation

static void orbit_gcArrayGrow(OrbitVM* vm, GCArray* array) {
    if(array->capacity == 0)
        array->capacity = GCARRAY_DEFAULT_CAPACITY;
    else
        array->capacity = array->capacity << 1;
    array->data = REALLOC_ARRAY(vm, array->data, GCValue, array->capacity);
}
//
// static inline void orbit_gcArrayShrink(OrbitVM* vm, GCArray* array) {
//     while(array->capacity >> 1 > array->size) {
//         array->capacity = array->capacity >> 1;
//     }
//     array->data = REALLOC_ARRAY(vm, array->data, GCValue, array->capacity);
// }

GCArray* orbit_gcArrayNew(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    
    GCArray* array = ALLOC(vm, GCArray);
    orbit_objectInit(vm, (GCObject*)array, NULL);
    array->base.type = OBJ_ARRAY;
    
    array->capacity = 0;
    orbit_gcArrayGrow(vm, array);
    
    return array;
}

void orbit_gcArrayAdd(OrbitVM* vm, GCArray* array, GCValue value) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(array != NULL, "Null instance error");
    
    if(array->size + 1 > array->capacity) {
        orbit_gcArrayGrow(vm, array);
    }
    array->data[array->size++] = value;
}

bool orbit_gcArrayGet(GCArray* array, uint32_t index, GCValue* value) {
    OASSERT(array != NULL, "Null instance error");
    
    if(index > array->size) {
        *value = VAL_NIL;
        return false;
    }
    *value = array->data[index];
    return true;
}

bool orbit_gcArrayRemove(OrbitVM* vm, GCArray* array, uint32_t index) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(array != NULL, "Null instance error");
    
    if(index > array->size) return false;
    array->size -= 1;
    memmove(&array->data[index], &array->data[index+1],
            sizeof(GCValue)*(array->size - index));
    return true;
}
