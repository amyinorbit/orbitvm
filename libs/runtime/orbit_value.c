//
//  orbit_value.c
//  OrbitVM
//
//  Created by Amy Parent on 2016-12-26.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#include <string.h>
#include <utils/orbit_assert.h>
#include "orbit_value.h"
#include "orbit_rtutils.h"
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
    GCString* object = orbit_gcStringReserve(vm, length);
    
    memcpy(object->data, string, length);
    object->data[length] = '\0';
    orbit_gcStringComputeHash(object);
    return object;
}

GCString* orbit_gcStringReserve(OrbitVM* vm, size_t length) {
    OASSERT(vm != NULL, "Null instance error");
    
    GCString* object = ALLOC_FLEX(vm, GCString, char, length+1);
    orbit_objectInit(vm, (GCObject*)object, NULL);
    
    object->base.type = OBJ_STRING;
    object->hash = 0;
    object->length = length;
    memset(object->data, '\0', length);
    return object;
}

void orbit_gcStringComputeHash(GCString* string) {
    OASSERT(string != NULL, "Null instance error");
    string->hash = orbit_hashString(string->data, string->length);
}

GCInstance* orbit_gcInstanceNew(OrbitVM* vm, GCClass* class) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(class != NULL, "Null class error");
    
    GCInstance* object = ALLOC_FLEX(vm, GCInstance, GCValue, class->fieldCount);
    orbit_objectInit(vm, (GCObject*)object, class);
    object->base.type = OBJ_INSTANCE;
    return object;
}

GCClass* orbit_gcClassNew(OrbitVM* vm, GCString* name, uint16_t fieldCount) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(name != NULL, "Null instance error");
    
    GCClass* class = ALLOC(vm, GCClass);
    orbit_objectInit(vm, (GCObject*)class, NULL);
    class->base.type = OBJ_CLASS;
    class->name = name;
    class->super = NULL;
    class->fieldCount = fieldCount;
    class->methods = orbit_gcMapNew(vm);
    
    return class;
}

VMFunction* orbit_gcFunctionNew(OrbitVM* vm, uint16_t byteCodeLength) {
    OASSERT(vm != NULL, "Null instance error");
    
    VMFunction* function = ALLOC(vm, VMFunction);
    orbit_objectInit(vm, (GCObject*)function, NULL);
    function->base.type = OBJ_FUNCTION;
    function->type = FN_NATIVE;
    
    // By default the function lives in the wild
    function->module = NULL;
    function->native.byteCode = ALLOC_ARRAY(vm, uint8_t, byteCodeLength);
    function->native.byteCodeLength = byteCodeLength;
    
    function->arity = 0;
    function->localCount = 0;
    function->stackEffect = 0;
    
    return function;
}

// Creates a new foreign function
VMFunction* orbit_gcFunctionForeignNew(OrbitVM* vm, GCForeignFn ffi, uint8_t arity) {
    OASSERT(vm != NULL, "Null instance error");
    
    VMFunction* function = ALLOC(vm, VMFunction);
    orbit_objectInit(vm, (GCObject*)function, NULL);
    function->base.type = OBJ_FUNCTION;
    function->type = FN_FOREIGN;

    function->module = NULL;
    function->foreign = ffi;
    
    function->arity = arity;
    function->localCount = 0;
    function->stackEffect = 0;
    
    return function;
}

VMModule* orbit_gcModuleNew(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    
    VMModule* module = ALLOC(vm, VMModule);
    orbit_objectInit(vm, (GCObject*)module, NULL);
    module->base.type = OBJ_MODULE;
    
    module->constantCount = 0;
    module->constants = NULL;
    module->globalCount = 0;
    module->globals = NULL;
    
    return module;
}

VMTask* orbit_gcTaskNew(OrbitVM* vm, VMFunction* function) {
    
    VMTask* task = ALLOC(vm, VMTask);
    orbit_objectInit(vm, (GCObject*)task, NULL);
    task->base.type = OBJ_TASK;
    
    task->stack = ALLOC_ARRAY(vm, GCValue, 512);
    task->stackCapacity = 512;
    
    task->frames = ALLOC_ARRAY(vm, VMTask, 32);
    task->frameCapacity = 32;
    
    // Create the first frame
    task->frameCount = 1;
    VMCallFrame* frame = &task->frames[0];
    
    frame->task = task; // FIXME: not required? prob. not accesed
    frame->function = function;
    frame->ip = function->native.byteCode;
    frame->stackBase = task->stack;
    
    // Put the stack pointer where it should be, after the entry point's
    // locals table.
    task->sp = frame->stackBase + function->localCount;
    
    return task;
}

void orbit_gcDeallocate(OrbitVM* vm, GCObject* object) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(object != NULL, "Null instance error");
    
    switch(object->type) {
    case OBJ_CLASS:
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
        }
        break;
        
    case OBJ_MODULE:
        DEALLOC(vm, ((VMModule*)object)->constants);
        DEALLOC(vm, ((VMModule*)object)->globals);
        break;
        
    case OBJ_TASK:
        DEALLOC(vm, ((VMTask*)object)->stack);
        DEALLOC(vm, ((VMTask*)object)->frames);
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
        if(IS_NIL(oldData[i].key)) continue;
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
    GCString* stra = AS_STRING(a);
    GCString* strb = AS_STRING(b);
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
    uint32_t index = orbit_valueHash(key) & map->mask;
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
        index = (index + 1) & map->mask;
    } while(index != start);
    
    return insert;
}

GCMap* orbit_gcMapNew(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    
    GCMap* map = ALLOC(vm, GCMap);
    orbit_objectInit(vm, (GCObject*)map, NULL/* TODO: replace with Map class*/);
    map->base.type = OBJ_MAP;
    
    map->data = NULL;
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
    if(IS_NIL(slot->key)) {
        map->size += 1;
    }
    slot->key = key;
    slot->value = value;
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
    
    array->data = NULL;
    array->capacity = 0;
    array->size = 0;
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
