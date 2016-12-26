#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_utils.h>

void string_create(void) {
    VMString* string = orbit_stringNew(NULL, "Hello, world!");
    TEST_CHECK(string != NULL);
    TEST_CHECK(string->length == 13);
    TEST_CHECK(strcmp(string->data, "Hello, world!") == 0);
    DEALLOC(_, string);
}

void string_hash(void) {
    VMString* a = orbit_stringNew(NULL, "Hello");
    VMString* b = orbit_stringNew(NULL, "Hello");
    VMString* c = orbit_stringNew(NULL, "Goodbye!");
    
    TEST_CHECK(a != NULL);
    TEST_CHECK(b != NULL);
    TEST_CHECK(c != NULL);
    
    TEST_CHECK(a->hash == b->hash);
    TEST_CHECK(a->hash != c->hash);
    
    DEALLOC(_, a);
    DEALLOC(_, b);
    DEALLOC(_, c);
}
