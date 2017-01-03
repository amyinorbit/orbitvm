#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_utils.h>
#include <liborbit/orbit_vm.h>

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
