//
//  tests-vm/tests-vm.c
//  OrbitVM/Tests
//
//  Created by Cesar Parent on 2016-12-24.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <stdio.h>
#include <liborbit/orbit_utils.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_vtable.h>
#include <liborbit/orbit_vm.h>

#include "unity.h"

// #include "test_string.h"
// #include "test_double.h"
// #include "test_vtable.h"

static OrbitVM vm;

void gc_collect(void) {
    orbit_vmInit(&vm);
    TEST_ASSERT_EQUAL(vm.allocated, 0);
    
    GCString* string = orbit_gcStringNew(&vm, "Hello, world");
    TEST_ASSERT_EQUAL(vm.allocated, sizeof(GCString) + string->length+1);
    
    orbit_gcRun(&vm);
    
    TEST_ASSERT_EQUAL(vm.allocated, 0);
}

void gc_savestack(void) {
    orbit_vmInit(&vm);
    TEST_ASSERT_EQUAL(vm.allocated, 0);
    
    GCString* string = orbit_gcStringNew(&vm, "Hello, world");
    
    size_t size = sizeof(GCString) + string->length+1;
    TEST_ASSERT_EQUAL(vm.allocated, size);
    
    orbit_gcRetain(&vm, (GCObject*)string);
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(vm.allocated, size);
    orbit_gcRelease(&vm);
    orbit_gcRun(&vm);
    
    TEST_ASSERT_EQUAL(vm.allocated, 0);
}

void string_create(void) {
    orbit_vmInit(&vm);
    GCString* string = orbit_gcStringNew(&vm, "Hello, world!");
    
    TEST_ASSERT_NOT_NULL(string);
    TEST_ASSERT_EQUAL(string->length, 13);
    TEST_ASSERT_EQUAL_STRING("Hello, world!", string->data);
    
    DEALLOC(&vm, string);
}

void string_hash(void) {
    orbit_vmInit(&vm);
    GCString* a = orbit_gcStringNew(&vm, "Hello");
    GCString* b = orbit_gcStringNew(&vm, "Hello");
    GCString* c = orbit_gcStringNew(&vm, "Goodbye!");
    
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(c);
    
    TEST_ASSERT_EQUAL(a->hash, b->hash);
    TEST_ASSERT_NOT_EQUAL(a->hash, c->hash);
    
    DEALLOC(&vm, a);
    DEALLOC(&vm, b);
    DEALLOC(&vm, c);
}

void double_hash(void) {
    TEST_ASSERT_EQUAL(orbit_hashDouble(12345.6789), orbit_hashDouble(12345.6789));
    TEST_ASSERT_NOT_EQUAL(orbit_hashDouble(-123.456), orbit_hashDouble(123.456));
    TEST_ASSERT_NOT_EQUAL(orbit_hashDouble(0.0), orbit_hashDouble(-0.0));
}

void vtable_create(void) {
    OrbitVtable table;
    orbit_vtableInit(&table);
    
    TEST_ASSERT_NOT_NULL(table.data);
    TEST_ASSERT_EQUAL(table.size, 0);
    TEST_ASSERT_EQUAL(table.capacity, VTABLE_DEFAULT_CAPACITY);
    
    orbit_vtableDeinit(&table);
}

void vtable_insert_get(void) {
    orbit_vmInit(&vm);
    
    const char* signature = "doSomething(String;Int;MyObject)Void";
    
    OrbitVtable table;
    orbit_vtableInit(&table);
    TEST_ASSERT_NOT_NULL(table.data);
    
    VMFunction fn;
    orbit_stringInitStatic(&fn.selector, signature);
    
    GCString* lookup = orbit_gcStringNew(&vm, signature);
    
    orbit_vtableInsert(&table, &fn);
    TEST_ASSERT_NOT_NULL(table.data);
    TEST_ASSERT_EQUAL(table.size, 1);
    
    VMFunction* fetch = orbit_vtableLookup(&table, lookup);
    TEST_ASSERT_NOT_NULL(fetch);
    TEST_ASSERT_EQUAL(fetch, &fn);
    
    DEALLOC(&vm, lookup);
    orbit_vtableDeinit(&table);
}

void gcarray_new(void) {
    orbit_vmInit(&vm);
    
    GCArray* array = orbit_gcArrayNew(&vm);
    
    TEST_ASSERT_NOT_NULL(array);
    TEST_ASSERT_NOT_NULL(array->data);
    TEST_ASSERT_EQUAL(0, array->size);
    TEST_ASSERT_EQUAL(GCARRAY_DEFAULT_CAPACITY, array->capacity);
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcarray_add(void) {
    orbit_vmInit(&vm);
    GCArray* array = orbit_gcArrayNew(&vm);
    orbit_gcArrayAdd(&vm, array, MAKE_NUM(123.456));
    
    TEST_ASSERT_EQUAL(1, array->size);
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcarray_get(void) {
    GCValue result;
    bool success = false;
    
    orbit_vmInit(&vm);
    GCArray* array = orbit_gcArrayNew(&vm);
    orbit_gcArrayAdd(&vm, array, MAKE_NUM(123.456));
    
    TEST_ASSERT_EQUAL(1, array->size);
    success = orbit_gcArrayGet(array, 0, &result);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(123.456, AS_NUM(result));
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcarray_remove(void) {
    GCValue result;
    bool success = false;
    
    orbit_vmInit(&vm);
    GCArray* array = orbit_gcArrayNew(&vm);
    orbit_gcArrayAdd(&vm, array, MAKE_NUM(123.456));
    orbit_gcArrayAdd(&vm, array, MAKE_NUM(-1));
    
    success = orbit_gcArrayRemove(&vm, array, 0);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(1, array->size);
    
    success = orbit_gcArrayGet(array, 0, &result);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(-1, AS_NUM(result));
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcarray_grow(void) {
    orbit_vmInit(&vm);
    GCArray* array = orbit_gcArrayNew(&vm);
    
    for(uint32_t i = 0; i <= GCARRAY_DEFAULT_CAPACITY; ++i) {
        orbit_gcArrayAdd(&vm, array, MAKE_NUM(i));
    }

    TEST_ASSERT_NOT_NULL(array->data);
    TEST_ASSERT_EQUAL(GCARRAY_DEFAULT_CAPACITY+1, array->size);
    TEST_ASSERT_EQUAL(2*GCARRAY_DEFAULT_CAPACITY, array->capacity);
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_new(void) {
    orbit_vmInit(&vm);
    
    GCMap* map = orbit_gcMapNew(&vm);
    
    TEST_ASSERT_NOT_NULL(map);
    TEST_ASSERT_NOT_NULL(map->data);
    TEST_ASSERT_EQUAL(0, map->size);
    TEST_ASSERT_EQUAL(GCMAP_DEFAULT_CAPACITY, map->capacity);
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_insert(void) {
    orbit_vmInit(&vm);
    
    GCValue key1 = MAKE_OBJECT(orbit_gcStringNew(&vm, "key1"));
    GCValue key2 = MAKE_NUM(123);
    GCMap* map = orbit_gcMapNew(&vm);
    
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(1000));
    orbit_gcMapAdd(&vm, map, key2, MAKE_NUM(-1000));
    TEST_ASSERT_EQUAL(2, map->size);
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_get(void) {
    orbit_vmInit(&vm);
    GCValue result;
    bool success = false;
    
    GCValue key1 = MAKE_OBJECT(orbit_gcStringNew(&vm, "key1"));
    GCValue key2 = MAKE_NUM(123);
    GCMap* map = orbit_gcMapNew(&vm);
    
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(1000));
    orbit_gcMapAdd(&vm, map, key2, MAKE_NUM(-1000));
    
    success = orbit_gcMapGet(map, key1, &result);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(1000, AS_NUM(result));
    
    success = orbit_gcMapGet(map, key2, &result);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(-1000, AS_NUM(result));
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_overwrite(void) {
    orbit_vmInit(&vm);
    GCValue result;
    bool success = false;
    
    GCValue key1 = MAKE_OBJECT(orbit_gcStringNew(&vm, "key1"));
    GCValue key2 = MAKE_NUM(123);
    GCMap* map = orbit_gcMapNew(&vm);
    
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(1000));
    orbit_gcMapAdd(&vm, map, key2, MAKE_NUM(-1000));
    TEST_ASSERT_EQUAL(2, map->size);
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(-5));
    
    TEST_ASSERT_EQUAL(2, map->size);
    success = orbit_gcMapGet(map, key1, &result);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(-5, AS_NUM(result));
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_remove(void) {
    orbit_vmInit(&vm);
    GCValue result;
    bool success = false;
    
    GCValue key1 = MAKE_OBJECT(orbit_gcStringNew(&vm, "key1"));
    GCValue key2 = MAKE_NUM(123);
    GCMap* map = orbit_gcMapNew(&vm);
    
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(1000));
    orbit_gcMapAdd(&vm, map, key2, MAKE_NUM(-1000));
    TEST_ASSERT_EQUAL(2, map->size);
    
    orbit_gcMapRemove(&vm, map, key1);
    TEST_ASSERT_EQUAL(1, map->size);
    
    success = orbit_gcMapGet(map, key1, &result);
    TEST_ASSERT_FALSE(success);
    TEST_ASSERT_TRUE(IS_NIL(result));
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_removeAdd(void) {
    orbit_vmInit(&vm);
    GCValue result;
    bool success = false;
    
    GCValue key1 = MAKE_OBJECT(orbit_gcStringNew(&vm, "key1"));
    GCValue key2 = MAKE_NUM(123);
    GCMap* map = orbit_gcMapNew(&vm);
    
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(1000));
    orbit_gcMapAdd(&vm, map, key2, MAKE_NUM(-1000));
    TEST_ASSERT_EQUAL(2, map->size);
    
    orbit_gcMapRemove(&vm, map, key1);
    TEST_ASSERT_EQUAL(1, map->size);
    
    success = orbit_gcMapGet(map, key1, &result);
    TEST_ASSERT_FALSE(success);
    TEST_ASSERT_TRUE(IS_NIL(result));
    
    orbit_gcMapAdd(&vm, map, key1, MAKE_NUM(123));
    TEST_ASSERT_EQUAL(2, map->size);
    
    success = orbit_gcMapGet(map, key1, &result);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(123, AS_NUM(result));
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

void gcmap_grow(void) {

    orbit_vmInit(&vm);
    GCMap* map = orbit_gcMapNew(&vm);
    
    for(uint32_t i = 0; i <= GCMAP_DEFAULT_CAPACITY; ++i) {
        orbit_gcMapAdd(&vm, map, MAKE_NUM(i), MAKE_NUM(i*1000));
    }
    
    TEST_ASSERT_NOT_NULL(map->data);
    TEST_ASSERT_EQUAL(GCMAP_DEFAULT_CAPACITY*2, map->capacity);
    
    GCValue result;
    bool success = false;
    for(uint32_t i = 0; i <= GCMAP_DEFAULT_CAPACITY; ++i) {
        success = orbit_gcMapGet(map, MAKE_NUM(i), &result);
        TEST_ASSERT_TRUE(success);
        TEST_ASSERT_EQUAL(i*1000, AS_NUM(result));
    }
    
    orbit_gcRun(&vm);
    TEST_ASSERT_EQUAL(0, vm.allocated);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(gc_collect);
    RUN_TEST(gc_savestack);
    RUN_TEST(string_create);
    RUN_TEST(string_hash);
    RUN_TEST(double_hash);
    
    RUN_TEST(vtable_create);
    RUN_TEST(vtable_insert_get);
    
    RUN_TEST(gcarray_new);
    RUN_TEST(gcarray_add);
    RUN_TEST(gcarray_get);
    RUN_TEST(gcarray_remove);
    RUN_TEST(gcarray_grow);
    
    RUN_TEST(gcmap_new);
    RUN_TEST(gcmap_insert);
    RUN_TEST(gcmap_get);
    RUN_TEST(gcmap_overwrite);
    RUN_TEST(gcmap_remove);
    RUN_TEST(gcmap_removeAdd);
    RUN_TEST(gcmap_grow);
    return UNITY_END();
}
