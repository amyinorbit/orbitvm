#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_vtable.h>

void vtable_create(void) {
    OrbitVtable table;
    orbit_vtableInit(&table);
    
    TEST_CHECK(table.data != NULL);
    TEST_CHECK(table.size == 0);
    TEST_CHECK(table.capacity == VTABLE_DEFAULT_CAPACITY);
    
    orbit_vtableDeinit(&table);
}

void vtable_insert(void) {
    
    const char* signature = "doSomething(String;Int;MyObject)Void";
    
    VMFunction fn;
    fn.selector.data = signature;
    fn.selector.length = strlen(signature);
    fn.selector.hash = orbit_stringHash(signature, fn.selector.length);
    
    
    
    
    const char* test_key = "test_key";
    int data = 200;
    int* result = NULL;

    HashMap map;
    orbit_hashmapInit(&map);
    TEST_CHECK(map.data != NULL);
    
    orbit_hashmapInsert(&map, test_key, &data);
    TEST_CHECK(map.data != NULL);
    TEST_CHECK(map.size == 1);
    
    result = orbit_hashmapGet(&map, test_key);
    TEST_CHECK(result != NULL);
    TEST_CHECK(result == &data);
    
    orbit_hashmapDeinit(&map);
}

void vtable_grow(void) {
    const char* test_key = "test_key";
    int data = 200;

    HashMap map;
    orbit_hashmapInit(&map);
    TEST_CHECK(map.data != NULL);
    
    for(int i = 0; i < ORBIT_MAP_CAPACITY; ++i) {
        orbit_hashmapInsert(&map, test_key, &data);
    }
    TEST_CHECK(map.data != NULL);
    TEST_CHECK(map.size == ORBIT_MAP_CAPACITY);
    TEST_CHECK(map.capacity == ORBIT_MAP_CAPACITY*2);
    
    orbit_hashmapDeinit(&map);
}
