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
#include <orbit/rt2/map.h>
#include <orbit/utils/pack.h>
#include <orbit/utils/hashing.h>
#include <unity.h>

OrbitGC gc;

void setUp(void) {
    orbitGCInit(&gc);
}

void tearDown(void) {
    orbitGCDeinit(&gc);
}

void pack_uint8(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    uint8_t in = 123;
    uint8_t out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitPack8(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbitUnpack8(f, &error);
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

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitPack16(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbitUnpack16(f, &error);
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

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitPack32(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbitUnpack32(f, &error);
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

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitPack64(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbitUnpack64(f, &error);
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

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitPackBytes(f, (uint8_t*)in, len));
    fseek(f, 0, SEEK_SET);
    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitUnpackBytes(f, (uint8_t*)out, len));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(in, out, len);
    fclose(f);
}

void pack_ieee754(void) {
    FILE* f = fopen("/tmp/test", "w+");
    TEST_ASSERT_NOT_NULL(f);

    OrbitPackError error = PACK_NOERROR;
    double in = 123.456;
    double out = 0;

    TEST_ASSERT_EQUAL(PACK_NOERROR, orbitPackIEEE754(f, in));
    fseek(f, 0, SEEK_SET);
    out = orbitUnpackIEEE754(f, &error);
    TEST_ASSERT_EQUAL(PACK_NOERROR, error);
    TEST_ASSERT_EQUAL(in, out);

    fclose(f);
}

void test_valueInt(void) {
    OrbitValue a = ORBIT_VALUE_INT(INT32_MAX);
    OrbitValue b = ORBIT_VALUE_INT(-1);
    
    TEST_ASSERT_TRUE(orbitValueEquals(a, a));
    TEST_ASSERT_FALSE(orbitValueEquals(a, b));
    
    TEST_ASSERT_EQUAL(ORBIT_AS_INT(a), INT32_MAX);
    TEST_ASSERT_EQUAL(ORBIT_AS_INT(b), -1);
}

void test_valueFloat(void) {
    OrbitValue a = ORBIT_VALUE_FLOAT(12.3456f);
    OrbitValue b = ORBIT_VALUE_FLOAT(65.4321f);
    
    TEST_ASSERT_TRUE(orbitValueEquals(a, a));
    TEST_ASSERT_FALSE(orbitValueEquals(a, b));
    
    TEST_ASSERT_EQUAL(12.3456f, ORBIT_AS_FLOAT(a));
    TEST_ASSERT_EQUAL(65.4321f, ORBIT_AS_FLOAT(b));
}

void test_stringCount(void) {
    const char string[] = "Hi! 여보세요 🏴󠁧󠁢󠁳󠁣󠁴󠁿🏳️‍🌈😁";
    OrbitValue val = ORBIT_VALUE_REF(orbitStringCopy(&gc, string, strlen(string)));
    
    TEST_ASSERT_EQUAL(12, ORBIT_AS_STRING(val)->count);
}

void test_stringEquality(void) {
    OrbitValue a = ORBIT_VALUE_REF(orbitStringCopy(&gc, "Hello!", 6));
    OrbitValue b = ORBIT_VALUE_REF(orbitStringCopy(&gc, "Hello!", 6));
    
    TEST_ASSERT_TRUE(orbitValueEquals(a, b));
}

void test_stringConcat(void) {
    OrbitString* a = orbitStringCopy(&gc, "Hello!", 6);
    OrbitString* b = orbitStringCopy(&gc, " 👋", strlen(" 👋"));
    OrbitString* c = orbitStringConcat(&gc, a, b);
    
    TEST_ASSERT_EQUAL(8, c->count);
    TEST_ASSERT_EQUAL(strlen("Hello! 👋"), c->utf8count);
    TEST_ASSERT_EQUAL_HEX8_ARRAY("Hello! 👋", c->data, c->utf8count);
}

void test_gcRootsPush(void) {
    OrbitString* a = orbitStringCopy(&gc, "Hello!", 6);
    orbitGCPush(&gc, (OrbitObject*)a);
    TEST_ASSERT_EQUAL(1, gc.rootCount);
}

void test_gcRootsPop(void) {
    OrbitString* a = orbitStringCopy(&gc, "Hello!", 6);
    orbitGCPush(&gc, (OrbitObject*)a);
    orbitGCPop(&gc);
    TEST_ASSERT_EQUAL(0, gc.rootCount);
}

void test_gcRootRelease(void) {
    OrbitString* a = orbitStringCopy(&gc, "Hello!", 6);
    OrbitString* b = orbitStringCopy(&gc, "Hello!", 6);
    OrbitString* c = orbitStringCopy(&gc, "Hello!", 6);
    
    orbitGCPush(&gc, (OrbitObject*)a);
    orbitGCPush(&gc, (OrbitObject*)b);
    orbitGCPush(&gc, (OrbitObject*)c);
    orbitGCPush(&gc, (OrbitObject*)a);
    orbitGCPush(&gc, (OrbitObject*)a);
    
    orbitGCRelease(&gc, (OrbitObject*)b);
    
    TEST_ASSERT_EQUAL(4, gc.rootCount);
    TEST_ASSERT_EQUAL_HEX(a, gc.roots[0]);
    TEST_ASSERT_EQUAL_HEX(b, gc.roots[1]);
    TEST_ASSERT_EQUAL_HEX(a, gc.roots[2]);
    TEST_ASSERT_EQUAL_HEX(a, gc.roots[3]);
}

void test_mapBasic(void) {
    OrbitValue key = ORBIT_VALUE_REF(orbitStringCopy(&gc, "some key", 8));
    OrbitMap map;
    orbitMapInit(&map);
    
    TEST_ASSERT_EQUAL(0, map.count);
    orbitMapInsert(&gc, &map, (OrbitPair){key, ORBIT_VALUE_INT(1234)});
    TEST_ASSERT_EQUAL(1, map.count);
    OrbitValue value;
    
    bool success = orbitMapGet(&gc, &map, key, &value);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(ORBIT_IS_INT(value));
    TEST_ASSERT_EQUAL(1234, ORBIT_AS_INT(value));
    
    orbitMapInsert(&gc, &map, (OrbitPair){key, ORBIT_VALUE_FLOAT(123.4f)});
    
    success = orbitMapGet(&gc, &map, key, &value);
    TEST_ASSERT_EQUAL(1, map.count);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(ORBIT_IS_FLOAT(value));
    TEST_ASSERT_EQUAL(123.4f, ORBIT_AS_FLOAT(value));
    
    orbitMapRemove(&gc, &map, key);
    TEST_ASSERT_EQUAL(0, map.count);
    
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
    RUN_TEST(test_stringCount);
    RUN_TEST(test_stringEquality);
    RUN_TEST(test_stringConcat);
    RUN_TEST(test_gcRootsPush);
    RUN_TEST(test_gcRootsPop);
    RUN_TEST(test_mapBasic);
    return UNITY_END();
}
