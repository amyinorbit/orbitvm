#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_string.h>
#include <liborbit/orbit_vtable.h>

void vtable_create(void) {
    OrbitVtable table;
    orbit_vtableInit(&table);
    
    TEST_CHECK(table.data != NULL);
    TEST_CHECK(table.size == 0);
    TEST_CHECK(table.capacity == VTABLE_DEFAULT_CAPACITY);
    
    orbit_vtableDeinit(&table);
}

void vtable_insert_get(void) {
    
    const char* signature = "doSomething(String;Int;MyObject)Void";
    
    OrbitVtable table;
    orbit_vtableInit(&table);
    TEST_CHECK(table.data != NULL);
    
    VMFunction fn;
    orbit_stringInitStatic(&fn.selector, signature);
    
    GCString* lookup = orbit_gcStringNew(NULL, signature);
    
    orbit_vtableInsert(&table, &fn);
    TEST_CHECK(table.data != NULL);
    TEST_CHECK(table.size == 1);
    
    VMFunction* fetch = orbit_vtableLookup(&table, lookup);
    TEST_CHECK(fetch != NULL);
    TEST_CHECK(fetch == &fn);
    
    DEALLOC(NULL, lookup);
    orbit_vtableDeinit(&table);
}
