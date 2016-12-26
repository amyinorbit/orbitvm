//
//  tests-vm/tests-vm.c
//  OrbitVM/Tests
//
//  Created by Cesar Parent on 24/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_hashmap.h"
#include "orbit_string.h"

#define DECL_TEST(test) {#test, test}

TEST_LIST = {
    DECL_TEST(hashmap_create),
    DECL_TEST(hashmap_insert),
    DECL_TEST(hashmap_grow),
    DECL_TEST(string_create),
    DECL_TEST(string_hash),
    { 0 }
};