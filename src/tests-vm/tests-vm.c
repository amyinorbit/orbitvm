//
//  tests-vm/tests-vm.c
//  OrbitVM/Tests
//
//  Created by Cesar Parent on 24/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <stdio.h>
#include <cutest/cutest.h>
#include "../liborbit/orbit_hashmap.h"

void hashmap_create(void) {
    HashMap* map = orbit_hashmapNew();
    TEST_CHECK(map != NULL);
    TEST_CHECK(map->size == 0);
    TEST_CHECK(map->capacity == ORBIT_MAP_DEFAULT_SIZE);
}

void hashmap_insert(void) {
    
    const char* test_key = "test_key";
    int data = 200;
    int* result = NULL;
    
    HashMap* map = orbit_hashmapNew();
    TEST_CHECK(map != NULL);
    
    map = orbit_hashmapInsert(map, test_key, &data);
    TEST_CHECK(map != NULL);
    TEST_CHECK(map->size == 1);
    
    result = orbit_hashmapGet(map, test_key);
    TEST_CHECK(result != NULL);
    TEST_CHECK(result == &data);
}

void hashmap_grow(void) {
    const char* test_key = "test_key";
    int data = 200;
    
    HashMap* map = orbit_hashmapNew();
    TEST_CHECK(map != NULL);
    
    for(int i = 0; i < ORBIT_MAP_DEFAULT_SIZE; ++i) {
        map = orbit_hashmapInsert(map, test_key, &data);
    }
    TEST_CHECK(map != NULL);
    TEST_CHECK(map->size == ORBIT_MAP_DEFAULT_SIZE);
    TEST_CHECK(map->capacity == ORBIT_MAP_DEFAULT_SIZE*2);
}


TEST_LIST = {
    {"hashmap_create", hashmap_create},
    {"hashmap_insert", hashmap_insert},
    {"hashmap_grow", hashmap_grow},
    { 0 }
};