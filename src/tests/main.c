#include <stdio.h>
#include <orbit/orbit.h>

#include "../liborbit/orbit_hashmap.h"


// THE X-MACRO!
#define OPCODE(x) x,
enum opcodes_e {
#include "../liborbit/orbit_opcodes.h"
};
#undef OPCODE

int main(int argc, const char** args) {
    //OrbitVM* vm = orbit_createVM();
    //orbit_destroyVM(vm);
    
    // Let's take the Hash Map for a spin
    
    printf("creating hash map\n");
    HashMap* map = orbit_hashmapNew();
    
    int test1 = 1000;
    int test2 = 2000;
    
    const char* kTest1 = "test_1";
    const char* kTest2 = "test_2";
    
    map = orbit_hashmapInsert(map, kTest1, &test1);
    map = orbit_hashmapInsert(map, kTest2, &test2);
    map = orbit_hashmapInsert(map, kTest2, &test2);
    map = orbit_hashmapInsert(map, kTest2, &test2);
    map = orbit_hashmapInsert(map, kTest2, &test2);
    
    int* fetched = orbit_hashmapGet(map, kTest2);
    
    printf("t2: %d\n", *fetched);
    
    orbit_hashmapDealloc(map);
}