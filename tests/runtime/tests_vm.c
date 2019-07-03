//
//  tests-vm/tests-vm.c
//  Orbit - /Tests
//
//  Created by Amy Parent on 2016-12-24.
//  Copyright © 2016 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <string.h>
#include <orbit/rt2/value.h>
#include <orbit/rt2/value_string.h>
#include <orbit/rt2/vm.h>
#include <orbit/rt2/garbage.h>
#include <orbit/utils/pack.h>
#include <orbit/utils/hashing.h>
#include <unity.h>

OrbitGC gc;

void setUp(void) {
    orbit_gcInit(&gc);
}

void tearDown(void) {
    orbit_gcDeinit(&gc);
}

void pack_uint8(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    uint8_t in = 123;
    uint8_t out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_pack8(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbit_unpack8(f, &error);
    TEST_ASSERT_EQUAL(PACK_NOERROR, error);
    TEST_ASSERT_EQUAL_HEX8(in, out);

    fclose(f);
}

void pack_uint16(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    uint16_t in = 1231;
    uint16_t out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_pack16(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbit_unpack16(f, &error);
    TEST_ASSERT_EQUAL(PACK_NOERROR, error);
    TEST_ASSERT_EQUAL_HEX16(in, out);

    fclose(f);
}

void pack_uint32(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    uint32_t in = 123123;
    uint32_t out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_pack32(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbit_unpack32(f, &error);
    TEST_ASSERT_EQUAL(PACK_NOERROR, error);
    TEST_ASSERT_EQUAL_HEX32(in, out);

    fclose(f);
}

void pack_uint64(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    uint64_t in = 123;
    uint64_t out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_pack64(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbit_unpack64(f, &error);
    TEST_ASSERT_EQUAL(PACK_NOERROR, error);
    TEST_ASSERT_EQUAL_HEX64(in, out);

    fclose(f);
}

void pack_bytes(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    char* in = "Hello, World!";
    char out[32] = {'\0'};

    size_t len = strlen(in);

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_packBytes(f, (uint8_t*)in, len));
    fseek(f, 0, SEEK_SET);
    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_unpackBytes(f, (uint8_t*)out, len));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(in, out, len);
    fclose(f);
}

void pack_ieee754(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    double in = 123.456;
    double out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbit_packIEEE754(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbit_unpackIEEE754(f, &error);
    TEST_ASSERT_EQUAL(PACK_NOERROR, error);
    TEST_ASSERT_EQUAL(in, out);

    fclose(f);
}

void test_valueInt(void) {
    OrbitValue a = ORBIT_VALUE_INT(INT32_MAX);
    OrbitValue b = ORBIT_VALUE_INT(-1);
    
    TEST_ASSERT_TRUE(orbit_valueEquals(a, a));
    TEST_ASSERT_FALSE(orbit_valueEquals(a, b));
    
    TEST_ASSERT_EQUAL(ORBIT_AS_INT(a), INT32_MAX);
    TEST_ASSERT_EQUAL(ORBIT_AS_INT(b), -1);
}

void test_valueFloat(void) {
    OrbitValue a = ORBIT_VALUE_FLOAT(12.3456f);
    OrbitValue b = ORBIT_VALUE_FLOAT(65.4321f);
    
    TEST_ASSERT_TRUE(orbit_valueEquals(a, a));
    TEST_ASSERT_FALSE(orbit_valueEquals(a, b));
    
    TEST_ASSERT_EQUAL(12.3456f, ORBIT_AS_FLOAT(a));
    TEST_ASSERT_EQUAL(65.4321f, ORBIT_AS_FLOAT(b));
}

void test_valueString(void) {
    const char basicC[] = "Hello, world!";
    const char clustersC[] = "🏴󠁧󠁢󠁳󠁣󠁴󠁿🏳️‍🌈😁";
    
    OrbitValue basic = ORBIT_VALUE_REF(orbit_stringCopy(&gc, basicC, strlen(basicC)));
    OrbitValue clusters = ORBIT_VALUE_REF(orbit_stringCopy(&gc, clustersC, strlen(clustersC)));
    
    TEST_ASSERT_TRUE(ORBIT_IS_STRING(basic));
    TEST_ASSERT_TRUE(ORBIT_IS_STRING(clusters));
    
    TEST_ASSERT_EQUAL_INT32(13, ORBIT_AS_STRING(basic)->count);
    TEST_ASSERT_EQUAL_INT32(3, ORBIT_AS_STRING(clusters)->count);
    
    TEST_ASSERT_TRUE(orbit_valueEquals(basic, basic));
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(pack_uint8);
    RUN_TEST(pack_uint16);
    RUN_TEST(pack_uint32);
    RUN_TEST(pack_uint64);
    RUN_TEST(pack_bytes);
    RUN_TEST(pack_ieee754);
    RUN_TEST(test_valueInt);
    RUN_TEST(test_valueFloat);
    RUN_TEST(test_valueString);
    return UNITY_END();
}
