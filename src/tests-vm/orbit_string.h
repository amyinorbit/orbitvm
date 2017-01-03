#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_utils.h>

void string_create(void) {
    GCString* string = orbit_gcStringNew(NULL, "Hello, world!");
    TEST_CHECK(string != NULL);
    TEST_CHECK(string->length == 13);
    TEST_CHECK(strcmp(string->data, "Hello, world!") == 0);
    DEALLOC(NULL, string);
}

void string_hash(void) {
    GCString* a = orbit_gcStringNew(NULL, "Hello");
    GCString* b = orbit_gcStringNew(NULL, "Hello");
    GCString* c = orbit_gcStringNew(NULL, "Goodbye!");
    
    TEST_CHECK(a != NULL);
    TEST_CHECK(b != NULL);
    TEST_CHECK(c != NULL);
    
    TEST_CHECK(a->hash == b->hash);
    TEST_CHECK(a->hash != c->hash);
    
    DEALLOC(NULL, a);
    DEALLOC(NULL, b);
    DEALLOC(NULL, c);
}
