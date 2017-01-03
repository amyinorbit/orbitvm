//
//  tests-vm/tests-vm.c
//  OrbitVM/Tests
//
//  Created by Cesar Parent on 2016-12-24.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <stdio.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_utils.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_vtable.h>
#include <liborbit/orbit_vm.h>

// #include "test_string.h"
// #include "test_double.h"
// #include "test_vtable.h"

#define DECL_TEST(test) {#test, test}
static OrbitVM vm;

void string_create(void) {
    orbit_vmInit(&vm);
    GCString* string = orbit_gcStringNew(&vm, "Hello, world!");
    TEST_CHECK(string != NULL);
    TEST_CHECK(string->length == 13);
    TEST_CHECK(strcmp(string->data, "Hello, world!") == 0);
    DEALLOC(&vm, string);
}

void string_hash(void) {
    orbit_vmInit(&vm);
    GCString* a = orbit_gcStringNew(&vm, "Hello");
    GCString* b = orbit_gcStringNew(&vm, "Hello");
    GCString* c = orbit_gcStringNew(&vm, "Goodbye!");
    
    TEST_CHECK(a != NULL);
    TEST_CHECK(b != NULL);
    TEST_CHECK(c != NULL);
    
    TEST_CHECK(a->hash == b->hash);
    TEST_CHECK(a->hash != c->hash);
    
    DEALLOC(&vm, a);
    DEALLOC(&vm, b);
    DEALLOC(&vm, c);
}

void double_hash(void) {
    TEST_CHECK(orbit_hashDouble(12345.6789) == orbit_hashDouble(12345.6789));
    TEST_CHECK(orbit_hashDouble(-123.456) != orbit_hashDouble(123.456));
    TEST_CHECK(orbit_hashDouble(0.0) != orbit_hashDouble(-0.0));
}

void vtable_create(void) {
    OrbitVtable table;
    orbit_vtableInit(&table);
    
    TEST_CHECK(table.data != NULL);
    TEST_CHECK(table.size == 0);
    TEST_CHECK(table.capacity == VTABLE_DEFAULT_CAPACITY);
    
    orbit_vtableDeinit(&table);
}

void vtable_insert_get(void) {
    orbit_vmInit(&vm);
    
    const char* signature = "doSomething(String;Int;MyObject)Void";
    
    OrbitVtable table;
    orbit_vtableInit(&table);
    TEST_CHECK(table.data != NULL);
    
    VMFunction fn;
    orbit_stringInitStatic(&fn.selector, signature);
    
    GCString* lookup = orbit_gcStringNew(&vm, signature);
    
    orbit_vtableInsert(&table, &fn);
    TEST_CHECK(table.data != NULL);
    TEST_CHECK(table.size == 1);
    
    VMFunction* fetch = orbit_vtableLookup(&table, lookup);
    TEST_CHECK(fetch != NULL);
    TEST_CHECK(fetch == &fn);
    
    DEALLOC(&vm, lookup);
    orbit_vtableDeinit(&table);
}

TEST_LIST = {
    DECL_TEST(string_create),
    DECL_TEST(string_hash),
    DECL_TEST(double_hash),
    DECL_TEST(vtable_create),
    DECL_TEST(vtable_insert_get),
    { 0 }
};