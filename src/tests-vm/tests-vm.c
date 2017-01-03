//
//  tests-vm/tests-vm.c
//  OrbitVM/Tests
//
//  Created by Cesar Parent on 24/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_string.h"
#include "orbit_double.h"
#include "orbit_vtable.h"

#define DECL_TEST(test) {#test, test}

TEST_LIST = {
    DECL_TEST(string_create),
    DECL_TEST(string_hash),
    DECL_TEST(double_hash),
    DECL_TEST(vtable_create),
    DECL_TEST(vtable_insert_get),
    { 0 }
};