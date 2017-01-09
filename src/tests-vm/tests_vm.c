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

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(gc_collect);
    RUN_TEST(gc_savestack);
    RUN_TEST(string_create);
    RUN_TEST(string_hash);
    RUN_TEST(double_hash);
    RUN_TEST(vtable_create);
    RUN_TEST(vtable_insert_get);
    return UNITY_END();
}
