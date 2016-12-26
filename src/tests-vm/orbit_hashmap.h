#include <stdio.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_hashmap.h>

void hashmap_create(void) {
    HashMap map;
    orbit_hashmapInit(&map);
    TEST_CHECK(map.data != NULL);
    TEST_CHECK(map.size == 0);
    TEST_CHECK(map.capacity == ORBIT_MAP_CAPACITY);
    orbit_hashmapDeinit(&map);
}

void hashmap_insert(void) {
    
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

void hashmap_grow(void) {
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
