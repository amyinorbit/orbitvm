#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_utils.h>

void string_create(void) {
    VMString* string = orbit_vmStringNew(NULL, "Hello, world!");
    TEST_CHECK(string != NULL);
    TEST_CHECK(string->length == 13);
    TEST_CHECK(strcmp(string->data, "Hello, world!") == 0);
    DEALLOC(string);
}

void string_hash(void) {
    VMString* a = orbit_vmStringNew(NULL, "Hello");
    VMString* b = orbit_vmStringNew(NULL, "Hello");
    VMString* c = orbit_vmStringNew(NULL, "Goodbye!");
    
    TEST_CHECK(a != NULL);
    TEST_CHECK(b != NULL);
    TEST_CHECK(c != NULL);
    
    TEST_CHECK(a->hash == b->hash);
    TEST_CHECK(a->hash != c->hash);
    
    DEALLOC(a);
    DEALLOC(b);
    DEALLOC(c);
}
