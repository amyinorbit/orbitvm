#include <stdio.h>
#include <string.h>
#include <cutest/cutest.h>
#include <liborbit/orbit_value.h>
#include <liborbit/orbit_utils.h>

void double_hash(void) {
    TEST_CHECK(orbit_hashNumber(12345.6789) == orbit_hashNumber(12345.6789));
    TEST_CHECK(orbit_hashNumber(-123.456) != orbit_hashNumber(123.456));
    TEST_CHECK(orbit_hashNumber(0.0) != orbit_hashNumber(-0.0));
}