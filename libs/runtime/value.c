//===--------------------------------------------------------------------------------------------===
// orbit/runtime/value.c
// This source is part of Orbit - Runtime
//
// Created on 2016-12-26 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <string.h>
#include <orbit/utils/hashing.h>
#include <orbit/runtime/value.h>
#include <orbit/runtime/rtutils.h>
#include <orbit/runtime/vm.h>

// Initialises [object] as an instance of [class]. [class] can be NULL if the
// object being initialized is a class itself.
static void orbit_objectInit(OrbitVM* vm, OrbitGCObject* object, OrbitGCClass* class) {
    assert(vm != NULL && "Null instance error");
    assert(object != NULL && "Null instance error");
    
    object->class = class;
    object->mark = false;
    object->next = vm->gcHead;
    vm->gcHead = object;
}

OrbitGCString* orbit_gcStringNew(OrbitVM* vm, const char* string) {
    assert(vm != NULL && "Null instance error");
    
    size_t length = strlen(string);
    OrbitGCString* object = orbit_gcStringReserve(vm, length);
    
    memcpy(object->data, string, length);
    object->data[length] = '\0';
    orbit_gcStringComputeHash(object);
    return object;
}

OrbitGCString* orbit_gcStringReserve(OrbitVM* vm, size_t length) {
    assert(vm != NULL && "Null instance error");
    
    OrbitGCString* object = ALLOC_FLEX(vm, OrbitGCString, char, length+1);
    orbit_objectInit(vm, (OrbitGCObject*)object, NULL);
    
    object->base.kind = ORBIT_OBJK_STRING;
    object->hash = 0;
    object->length = length;
    memset(object->data, '\0', length);
    return object;
}

void orbit_gcStringComputeHash(OrbitGCString* string) {
    assert(string != NULL && "Null instance error");
    string->hash = orbit_hashString(string->data, string->length);
}

OrbitGCInstance* orbit_gcInstanceNew(OrbitVM* vm, OrbitGCClass* class) {
    assert(vm != NULL && "Null instance error");
    assert(class != NULL && "Null class error");
    
    OrbitGCInstance* object = ALLOC_FLEX(vm, OrbitGCInstance, OrbitValue, class->fieldCount);
    orbit_objectInit(vm, (OrbitGCObject*)object, class);
    object->base.kind = ORBIT_OBJK_INSTANCE;
    return object;
}

OrbitGCClass* orbit_gcClassNew(OrbitVM* vm, OrbitGCString* name, uint16_t fieldCount) {
    assert(vm != NULL && "Null instance error");
    assert(name != NULL && "Null instance error");
    
    OrbitGCClass* class = ALLOC(vm, OrbitGCClass);
    orbit_objectInit(vm, (OrbitGCObject*)class, NULL);
    class->base.kind = ORBIT_OBJK_CLASS;
    class->name = name;
    class->super = NULL;
    class->fieldCount = fieldCount;
    class->methods = orbit_gcMapNew(vm);
    
    return class;
}

OrbitVMFunction* orbit_gcFunctionNew(OrbitVM* vm, uint16_t byteCodeLength) {
    assert(vm != NULL && "Null instance error");
    
    OrbitVMFunction* function = ALLOC(vm, OrbitVMFunction);
    orbit_objectInit(vm, (OrbitGCObject*)function, NULL);
    function->base.kind = ORBIT_OBJK_FUNCTION;
    function->kind = ORBIT_FK_NATIVE;
    
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
OrbitVMFunction* orbit_gcFunctionForeignNew(OrbitVM* vm, GCForeignFn ffi, uint8_t arity) {
    assert(vm != NULL && "Null instance error");
    
    OrbitVMFunction* function = ALLOC(vm, OrbitVMFunction);
    orbit_objectInit(vm, (OrbitGCObject*)function, NULL);
    function->base.kind = ORBIT_OBJK_FUNCTION;
    function->kind = ORBIT_FK_FOREIGN;

    function->module = NULL;
    function->foreign = ffi;
    
    function->arity = arity;
    function->localCount = 0;
    function->stackEffect = 0;
    
    return function;
}

OrbitVMModule* orbit_gcModuleNew(OrbitVM* vm) {
    assert(vm != NULL && "Null instance error");
    
    OrbitVMModule* module = ALLOC(vm, OrbitVMModule);
    orbit_objectInit(vm, (OrbitGCObject*)module, NULL);
    module->base.kind = ORBIT_OBJK_MODULE;
    
    module->constantCount = 0;
    module->constants = NULL;
    module->globalCount = 0;
    module->globals = NULL;
    
    return module;
}

OrbitVMTask* orbit_gcTaskNew(OrbitVM* vm, OrbitVMFunction* function) {
    
    OrbitVMTask* task = ALLOC(vm, OrbitVMTask);
    orbit_objectInit(vm, (OrbitGCObject*)task, NULL);
    task->base.kind = ORBIT_OBJK_TASK;
    
    task->stack = ALLOC_ARRAY(vm, OrbitValue, 512);
    task->stackCapacity = 512;
    
    task->frames = ALLOC_ARRAY(vm, OrbitVMTask, 32);
    task->frameCapacity = 32;
    
    // Create the first frame
    task->frameCount = 1;
    OrbitVMFrame* frame = &task->frames[0];
    
    frame->task = task; // FIXME: not required? prob. not accesed
    frame->function = function;
    frame->ip = function->native.byteCode;
    frame->stackBase = task->stack;
    
    // Put the stack pointer where it should be, after the entry point's
    // locals table.
    task->sp = frame->stackBase + function->localCount;
    
    return task;
}

void orbit_gcDeallocate(OrbitVM* vm, OrbitGCObject* object) {
    assert(vm != NULL && "Null instance error");
    assert(object != NULL && "Null instance error");
    
    switch(object->kind) {
    case ORBIT_OBJK_CLASS:
        break;
        
    case ORBIT_OBJK_INSTANCE:
        break;
        
    case ORBIT_OBJK_STRING:
        break;
        
    case ORBIT_OBJK_MAP:
        DEALLOC(vm, ((OrbitGCMap*)object)->data);
        break;
    
    case ORBIT_OBJK_ARRAY:
        DEALLOC(vm, ((OrbitGCArray*)object)->data);
        break;
        
    case ORBIT_OBJK_FUNCTION:
        if(((OrbitVMFunction*)object)->kind == ORBIT_FK_NATIVE) {
            DEALLOC(vm, ((OrbitVMFunction*)object)->native.byteCode);
        }
        break;
        
    case ORBIT_OBJK_MODULE:
        DEALLOC(vm, ((OrbitVMModule*)object)->constants);
        DEALLOC(vm, ((OrbitVMModule*)object)->globals);
        break;
        
    case ORBIT_OBJK_TASK:
        DEALLOC(vm, ((OrbitVMTask*)object)->stack);
        DEALLOC(vm, ((OrbitVMTask*)object)->frames);
        break;
    }
    DEALLOC(vm, object);
}

// MARK: - Map functions implementations

static inline uint32_t orbit_valueHash(OrbitValue value) {
    if(IS_NUM(value))
        return orbit_hashDouble(AS_NUM(value));
    if(IS_STRING(value))
        return AS_STRING(value)->hash;
    else
        return 0; // TODO: replace with pointer hash.
}

static void orbit_gcMapGrow(OrbitVM* vm, OrbitGCMap* map) {
    uint32_t oldCapacity = map->capacity;
    OrbitGCMapEntry* oldData = map->data;
    
    if(map->capacity == 0)
        map->capacity = GCMAP_DEFAULT_CAPACITY;
    else
        map->capacity = map->capacity << 1;
    
    map->data = ALLOC_ARRAY(vm, OrbitGCMapEntry, map->capacity);
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
static inline bool orbit_gcMapComp(OrbitValue a, OrbitValue b) {
    if(IS_NUM(a) && IS_NUM(b)) {
        return AS_NUM(a) == AS_NUM(b);
    }
    OrbitGCString* stra = AS_STRING(a);
    OrbitGCString* strb = AS_STRING(b);
    // Check for pointer equality first.
    return (stra == strb)
        || (stra->length == strb->length
            && stra->hash == strb->hash
            && memcmp(stra->data, strb->data, stra->length) == 0); 
}

// Find the entry in [map] keyed by [key], or return a pointer to the entry
// where a value should be inserted.
static OrbitGCMapEntry* orbit_gcMapFindSlot(OrbitGCMap* map, OrbitValue key) {
    OrbitGCMapEntry* insert = NULL;
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

OrbitGCMap* orbit_gcMapNew(OrbitVM* vm) {
    assert(vm != NULL && "Null instance error");
    
    OrbitGCMap* map = ALLOC(vm, OrbitGCMap);
    orbit_objectInit(vm, (OrbitGCObject*)map, NULL/* TODO: replace with Map class*/);
    map->base.kind = ORBIT_OBJK_MAP;
    
    map->data = NULL;
    map->capacity = 0;
    orbit_gcMapGrow(vm, map);
    
    return map;
}

void orbit_gcMapAdd(OrbitVM* vm, OrbitGCMap* map, OrbitValue key, OrbitValue value) {
    assert(vm != NULL && "Null instance error");
    assert(map != NULL && "Null instance error");
    assert(IS_NUM(key) || IS_STRING(key) && "Map keys must be primitives");
    
    if(map->size+1 > 0.75 * map->capacity) {
        orbit_gcMapGrow(vm, map);
    }
    
    OrbitGCMapEntry* slot = orbit_gcMapFindSlot(map, key);
    if(IS_NIL(slot->key)) {
        map->size += 1;
    }
    slot->key = key;
    slot->value = value;
}

bool orbit_gcMapGet(OrbitGCMap* map, OrbitValue key, OrbitValue* value) {
    assert(map != NULL && "Null instance error");
    assert(IS_NUM(key) || IS_STRING(key) && "Map keys must be primitives");
    
    OrbitGCMapEntry* slot = orbit_gcMapFindSlot(map, key);
    if(IS_NIL(slot->key)) {
        *value = VAL_NIL;
        return false;
    } else {
        *value = slot->value;
        return true;
    }
}

void orbit_gcMapRemove(OrbitVM* vm, OrbitGCMap* map, OrbitValue key) {
    assert(vm != NULL && "Null instance error");
    assert(map != NULL && "Null instance error");
    assert(IS_NUM(key) || IS_STRING(key) && "Map keys must be primitives");
    
    OrbitGCMapEntry* slot = orbit_gcMapFindSlot(map, key);
    if(IS_NIL(slot->key)) return;
    
    // Tombstone, so that entries pushed further by collisions can still be
    // found using gcMapFindSlot.
    slot->key = VAL_NIL;
    slot->value = VAL_TRUE;
    map->size -= 1;
}

// MARK: - Array functions implementation

static void orbit_gcArrayGrow(OrbitVM* vm, OrbitGCArray* array) {
    if(array->capacity == 0)
        array->capacity = GCARRAY_DEFAULT_CAPACITY;
    else
        array->capacity = array->capacity << 1;
    array->data = REALLOC_ARRAY(vm, array->data, OrbitValue, array->capacity);
}
//
// static inline void orbit_gcArrayShrink(OrbitVM* vm, OrbitGCArray* array) {
//     while(array->capacity >> 1 > array->size) {
//         array->capacity = array->capacity >> 1;
//     }
//     array->data = REALLOC_ARRAY(vm, array->data, OrbitValue, array->capacity);
// }

OrbitGCArray* orbit_gcArrayNew(OrbitVM* vm) {
    assert(vm != NULL && "Null instance error");
    
    OrbitGCArray* array = ALLOC(vm, OrbitGCArray);
    orbit_objectInit(vm, (OrbitGCObject*)array, NULL);
    array->base.kind = ORBIT_OBJK_ARRAY;
    
    array->data = NULL;
    array->capacity = 0;
    array->size = 0;
    orbit_gcArrayGrow(vm, array);
    
    return array;
}

void orbit_gcArrayAdd(OrbitVM* vm, OrbitGCArray* array, OrbitValue value) {
    assert(vm != NULL && "Null instance error");
    assert(array != NULL && "Null instance error");
    
    if(array->size + 1 > array->capacity) {
        orbit_gcArrayGrow(vm, array);
    }
    array->data[array->size++] = value;
}

bool orbit_gcArrayGet(OrbitGCArray* array, uint32_t index, OrbitValue* value) {
    assert(array != NULL && "Null instance error");
    
    if(index > array->size) {
        *value = VAL_NIL;
        return false;
    }
    *value = array->data[index];
    return true;
}

bool orbit_gcArrayRemove(OrbitVM* vm, OrbitGCArray* array, uint32_t index) {
    assert(vm != NULL && "Null instance error");
    assert(array != NULL && "Null instance error");
    
    if(index > array->size) return false;
    array->size -= 1;
    memmove(&array->data[index], &array->data[index+1],
            sizeof(OrbitValue)*(array->size - index));
    return true;
}
