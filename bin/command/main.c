#include <stdio.h>
#include <string.h>
#include <time.h>
#include <orbit/stdlib/stdlib.h>
#include <orbit/orbit.h>

int main(int argc, const char** argv) {
    
    if(argc < 2) {
        fprintf(stderr, "error: no input module file.\n");
        return -1;
    }
    
    OrbitVM* vm = orbit_vmNew();
    orbit_registerStandardLib(vm);
    if(!orbit_vmInvoke(vm, argv[1], "main")) {
        fprintf(stderr, "error: interpreter error\n");
    }
    orbit_vmDealloc(vm);
}